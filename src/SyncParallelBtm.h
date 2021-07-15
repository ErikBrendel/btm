#pragma once

#include <vector>
#include "Btm.h"

#include "ThreadsafeQueue.h"
#include "ThreadSync1toN.h"

using namespace std;

using Biterm = pair<unsigned int, unsigned int>;

class SyncParallelBtm : public Btm {
public:
    unsigned int workerThreadCount = 3;
protected:
    void doGibbsIterations(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                           vector<unsigned int> &bitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                           vector<unsigned int> &topicToCount) const override;

private:
    void aggregator_thread_fn(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                              vector<unsigned int> &wordAndTopicToCount, vector<unsigned int> &topicToCount,
                              ThreadsafeQueue<tuple<unsigned int, unsigned int, unsigned int>> &bitermTopicUpdates,
                              ThreadSync1toN &threadSync) const;

    void worker_thread_fn(const vector<Biterm> &allMyBiterms, unsigned int vocabSize, unsigned int iterations,
                          vector<unsigned int> &myBitermToTopic, unsigned int myBitermOffset,
                          const vector<unsigned int> &wordAndTopicToCount,
                          const vector<unsigned int> &topicToCount,
                          ThreadsafeQueue<tuple<unsigned int, unsigned int, unsigned int>> &bitermTopicUpdates,
                          ThreadSync1toN &threadSync) const;
};
