#include <memory>
#include <iostream>
#include <thread>
#include <atomic>

#include "SyncParallelBtm.h"
#include "util.h"


#define RANDOM_SEED 42
std::mt19937 my_sync_parallel_btm_randomEngine(RANDOM_SEED);


void
SyncParallelBtm::aggregator_thread_fn(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                                      vector<unsigned int> &wordAndTopicToCount, vector<unsigned int> &topicToCount,
                                      ThreadsafeQueue<tuple<unsigned int, unsigned int, unsigned int>> &bitermTopicUpdates,
                                      ThreadSync1toN &threadSync) const {
    auto startTime = timeNow();
    rep(it, iterations) {
        threadSync.mainThreadWait();
        // cout << "Iteration " << it << " had " << bitermTopicUpdates.size() << " updates" << endl;
        while (!bitermTopicUpdates.isEmpty()) {
            const auto&[i, oldTopic, newTopic] = bitermTopicUpdates.get();
            const auto &biterm = allBiterms[i];

            // remove this biterm from its old topic
            wordAndTopicToCount[biterm.first * topicCount + oldTopic]--;
            wordAndTopicToCount[biterm.second * topicCount + oldTopic]--;
            topicToCount[oldTopic]--;

            // register this biterm to its new topic
            wordAndTopicToCount[biterm.first * topicCount + newTopic]++;
            wordAndTopicToCount[biterm.second * topicCount + newTopic]++;
            topicToCount[newTopic]++;
        }

        //topic,coherence,iteration,threadCount
        auto elapsedTime = timeSince(startTime);
        if (elapsedTime > 500) {
            cout << "#progress " << it << " of " << iterations << endl;
            startTime = timeNow();
        }
        // printTopicCoherences("," + to_string(it) + "," + to_string(elapsedTime) + "," + to_string(workerThreadCount), vocabSize, wordAndTopicToCount);

        threadSync.mainThreadDone();
    }
}

void
SyncParallelBtm::worker_thread_fn(const vector<Biterm> &allMyBiterms, unsigned int vocabSize, unsigned int iterations,
                                  vector<unsigned int> &myBitermToTopic, unsigned int myBitermOffset,
                                  const vector<unsigned int> &wordAndTopicToCount,
                                  const vector<unsigned int> &topicToCount,
                                  ThreadsafeQueue<tuple<unsigned int, unsigned int, unsigned int>> &bitermTopicUpdates,
                                  ThreadSync1toN &threadSync) const {

    double betaTimesVocabSize = beta * (double) vocabSize;
    vector<double> topicProbabilities(topicCount); // P_z, but not normalized, as the c++ random lib does that
    rep(it, iterations) {
        threadSync.workerThreadWait();
        //cout << " Worker Iteration " << it << endl;
        vector<tuple<unsigned int, unsigned int, unsigned int>> localTaskCache;
        rep(i, allMyBiterms.size()) {
            const auto &biterm = allMyBiterms[i];
            // See equation 4 in the short text BTM paper
            rep(t, topicCount) {
                topicProbabilities[t] = (topicToCount[t] + alpha)
                                        * (wordAndTopicToCount[biterm.first * topicCount + t] + beta)
                                        * (wordAndTopicToCount[biterm.second * topicCount + t] + beta)
                                        / square(2 * topicToCount[t] + betaTimesVocabSize);
            }
            // convert list of topic probabilities to discrete random distribution object
            discrete_distribution<> discrete_dist(all(topicProbabilities));
            unsigned int newTopic = discrete_dist(my_sync_parallel_btm_randomEngine);
            if (newTopic != myBitermToTopic[i]) {
                localTaskCache.emplace_back(i + myBitermOffset, myBitermToTopic[i], newTopic);
                myBitermToTopic[i] = newTopic;
            }
        }
        {
            std::scoped_lock<std::mutex> lock(bitermTopicUpdates.queue_access_mutex);
            for (const auto& t: localTaskCache) {
                bitermTopicUpdates.container.push(t);
            }
        }
        threadSync.workerThreadDone();
    }
}


void
SyncParallelBtm::doGibbsIterations(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                                   vector<unsigned int> &bitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                                   vector<unsigned int> &topicToCount) const {

    ThreadsafeQueue<tuple<unsigned int, unsigned int, unsigned int>> bitermTopicUpdates;
    ThreadSync1toN threadSync(workerThreadCount);


    thread aggregator(&SyncParallelBtm::aggregator_thread_fn, this,
                      cref(allBiterms), vocabSize, iterations,
                      ref(wordAndTopicToCount), ref(topicToCount),
                      ref(bitermTopicUpdates), ref(threadSync));

    std::this_thread::sleep_for(1000ms);

    vector<thread> workerThreads;
    workerThreads.reserve(workerThreadCount);
    unsigned int bitermsPerThread = ceil(allBiterms.size() / (double) workerThreadCount);
    vector<vector<Biterm>> allBitermsSplit(workerThreadCount);
    vector<vector<unsigned int>> bitermToTopicSplit(workerThreadCount);
    rep(th, workerThreadCount) {
        auto bitermCount =
                th == workerThreadCount - 1 ? (allBiterms.size() - (workerThreadCount - 1) * bitermsPerThread)
                                            : bitermsPerThread;
        allBitermsSplit[th].resize(bitermCount);
        unsigned int bitermOffset = th * bitermsPerThread;
        copy(allBiterms.begin() + bitermOffset,
             allBiterms.begin() + th * bitermCount + bitermCount,
             allBitermsSplit[th].begin());
        bitermToTopicSplit[th].resize(bitermCount);
        copy(bitermToTopic.begin() + bitermOffset,
             bitermToTopic.begin() + th * bitermCount + bitermCount,
             bitermToTopicSplit[th].begin());
        workerThreads.emplace_back(&SyncParallelBtm::worker_thread_fn, this,
                                   cref(allBitermsSplit[th]), vocabSize, iterations,
                                   ref(bitermToTopicSplit[th]), bitermOffset,
                                   ref(wordAndTopicToCount), ref(topicToCount),
                                   ref(bitermTopicUpdates), ref(threadSync));
    }

    rep(th, workerThreadCount) {
        workerThreads[th].join();
    }
    aggregator.join();

    // TODO after that do some annealing thing to fine-tune the topics?
}
