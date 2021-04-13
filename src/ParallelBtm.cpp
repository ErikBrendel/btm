#include <memory>
#include <iostream>
#include <thread>

#include "ParallelBtm.h"
#include "util.h"


#define RANDOM_SEED 42
std::mt19937 my_parallel_btm_randomEngine(RANDOM_SEED);

#define THREAD_SAFE false
#if THREAD_SAFE
#include <mutex>
mutex wordAndTopicToCount_mutex;
mutex topicToCount_mutex;
#define ts(x) x
#else
#define ts(x)
#endif


void ParallelBtm::worker_thread_fn(const vector<Biterm> &allMyBiterms, unsigned int vocabSize, unsigned int iterations,
                                   vector<unsigned int> &myBitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                                   vector<unsigned int> &topicToCount) const {

    // TODO this seems to work badly, lets try IWS instead:
    //  https://stanford.edu/~rezab/classes/cme323/S16/projects_reports/ehrenberg.pdf
    //  maybe here are too much memory collisions?

    double betaTimesVocabSize = beta * (double) vocabSize;
    rep(it, iterations) {
        cout << "Iteration " << it << endl;
        rep(i, allMyBiterms.size()) {
            const auto &biterm = allMyBiterms[i];
            // remove this biterm from its old topic
            ts(wordAndTopicToCount_mutex.lock();)
            wordAndTopicToCount[biterm.first * topicCount + myBitermToTopic[i]]--;
            wordAndTopicToCount[biterm.second * topicCount + myBitermToTopic[i]]--;
            ts(wordAndTopicToCount_mutex.unlock();)
            ts(topicToCount_mutex.lock();)
            auto oldTopicToCount = topicToCount[myBitermToTopic[i]];
            topicToCount[myBitermToTopic[i]]--;
            auto newTopicToCount = topicToCount[myBitermToTopic[i]];
            ts(topicToCount_mutex.unlock();)
            if (oldTopicToCount != newTopicToCount + 1) {
                cout << "Clash: From " << oldTopicToCount << " to " << newTopicToCount << endl;
            }

            // See equation 4 in the short text BTM paper
            vector<double> topicProbabilities(topicCount); // P_z, but not normalized, as the c++ random lib does that
            rep(t, topicCount) {
                ts(wordAndTopicToCount_mutex.lock();)
                topicProbabilities[t] = (topicToCount[t] + alpha)
                                        * (wordAndTopicToCount[biterm.first * topicCount + t] + beta)
                                        * (wordAndTopicToCount[biterm.second * topicCount + t] + beta)
                                        / square(2 * topicToCount[t] + betaTimesVocabSize);
                ts(wordAndTopicToCount_mutex.unlock();)
            }
            // convert list of topic probabilities to discrete random distribution object
            discrete_distribution<> discrete_dist(all(topicProbabilities));
            myBitermToTopic[i] = discrete_dist(my_parallel_btm_randomEngine); // assign a new topic

            // register this biterm to its new topic
            ts(wordAndTopicToCount_mutex.lock();)
            wordAndTopicToCount[biterm.first * topicCount + myBitermToTopic[i]]++;
            wordAndTopicToCount[biterm.second * topicCount + myBitermToTopic[i]]++;
            ts(wordAndTopicToCount_mutex.unlock();)
            ts(topicToCount_mutex.lock();)
            topicToCount[myBitermToTopic[i]]++;
            ts(topicToCount_mutex.unlock();)
        }
    }
}


void ParallelBtm::doGibbsIterations(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                                    vector<unsigned int> &bitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                                    vector<unsigned int> &topicToCount) const {

    vector<thread> threads;
    threads.reserve(workerThreadCount);
    unsigned int bitermsPerThread = ceil(allBiterms.size() / (double) workerThreadCount);
    vector<vector<Biterm>> allBitermsSplit(workerThreadCount);
    vector<vector<unsigned int>> bitermToTopicSplit(workerThreadCount);
    rep(th, workerThreadCount) {
        auto bitermCount = th == workerThreadCount - 1 ? (allBiterms.size() - (workerThreadCount-1) * bitermsPerThread) : bitermsPerThread;
        allBitermsSplit[th].resize(bitermCount);
        copy(allBiterms.begin() + th * bitermsPerThread,
             allBiterms.begin() + th * bitermCount + bitermCount,
             allBitermsSplit[th].begin());
        bitermToTopicSplit[th].resize(bitermCount);
        copy(bitermToTopic.begin() + th * bitermsPerThread,
             bitermToTopic.begin() + th * bitermCount + bitermCount,
             bitermToTopicSplit[th].begin());
        //thread foo(worker_thread_fn, allBitermsSplit[th], vocabSize, iterations, ref(bitermToTopicSplit[th]), ref(wordAndTopicToCount), ref(topicToCount));
        threads.emplace_back(&ParallelBtm::worker_thread_fn, this, allBitermsSplit[th], vocabSize, iterations,
                             ref(bitermToTopicSplit[th]), ref(wordAndTopicToCount), ref(topicToCount));
    }
    rep(th, workerThreadCount) {
        threads[th].join();
    }
}
