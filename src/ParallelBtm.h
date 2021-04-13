#pragma once

#include <vector>
#include "Btm.h"

using namespace std;

using Biterm = pair<unsigned int, unsigned int>;

class ParallelBtm : public Btm {
    unsigned int workerThreadCount = 4;
protected:
    void doGibbsIterations(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations,
                           vector<unsigned int> &bitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                           vector<unsigned int> &topicToCount) const override;

private:
    void worker_thread_fn(const vector<Biterm> &allMyBiterms, unsigned int vocabSize, unsigned int iterations,
                          vector<unsigned int> &myBitermToTopic, vector<unsigned int> &wordAndTopicToCount,
                          vector<unsigned int> &topicToCount) const;
};
