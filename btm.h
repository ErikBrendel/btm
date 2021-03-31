#pragma once

#include <vector>

using namespace std;

using Biterm = pair<unsigned int, unsigned int>;

class btm {
public:
    unsigned int topicCount = 3;
    double alpha = 1.0; // dirichlet prior probability for each topic
    double beta = 0.01; // dirichlet prior probability for topic-word-probabilities
    unsigned int maxTopWords = 10;
public:
    [[nodiscard]] pair<vector<vector<unsigned int>>, vector<double>>
    fitTransform(const vector<vector<Biterm>>& documentsBiterms, unsigned int vocabSize, unsigned int iterations);
private:
    [[nodiscard]] pair<vector<unsigned int>, vector<unsigned int>> gibbsSampling(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations) const;
};

