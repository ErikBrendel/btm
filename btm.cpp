#include <memory>
#include <iostream>
#include "btm.h"

#include "SortedLimitedList.h"

#include "util.h"

// https://github.com/markoarnauto/biterm/blob/master/biterm/btm.py

#define RANDOM_SEED 42
std::mt19937 my_btm_randomEngine(RANDOM_SEED);
/**
 * @param upperBound exclusive
 */
unsigned int randRange(unsigned int upperBound) {
    return std::uniform_int_distribution<int>(0, upperBound-1)(my_btm_randomEngine);
}


pair<vector<vector<unsigned int>>, vector<double>>
btm::fitTransform(const vector<vector<Biterm>> &documentsBiterms, unsigned int vocabSize, unsigned int iterations) {
    vector<Biterm> allBiterms;
    for (const auto &biterms: documentsBiterms) {
        allBiterms.insert(allBiterms.end(), all(biterms));
    }
    auto [topicToCount, wordAndTopicToCount] // n_z, n_wz
        = gibbsSampling(allBiterms, vocabSize, iterations);

    // TODO convert n_wz and n_z to phi_wz and theta_z
    //  use those two to transform the list of input documents to the P_dz matrix (docTop)
    //  https://numpy.org/doc/stable/user/basics.broadcasting.html
    //  https://github.com/jcapde/Biterm/blob/master/Biterm_sampler.py#L97
    //  https://github.com/markoarnauto/biterm/blob/master/biterm/btm.py#L61

    vector<double> phi_wz(vocabSize * topicCount);
    vector<double> theta_z(topicCount);
    double betaTimesVocabSize = beta * (double)vocabSize;
    rep(t, topicCount) {
        // marko: self.theta_z = (n_z + self.alpha) / (n_z + self.alpha).sum()
        // jcapde: thetaz = (Nz + alpha)/(B + num_topics*alpha)
        theta_z[t] = (topicToCount[t] + alpha) / (allBiterms.size() + topicCount*alpha);
        rep(word, vocabSize) {
            // marko: self.phi_wz = (self.nwz + self.beta) / np.array([(self.nwz + self.beta).sum(axis=0)] * len(self.V))
            // jcapde: phiwz = (Nwz + beta) / np.tile((Nwz.sum(axis=0)+V*beta),(V,1))

            // it seems like "Sum_w(n_wz)" equals "2 * n_z"
            phi_wz[word * topicCount + t] = (wordAndTopicToCount[word * topicCount + t] + beta) / (2 * topicToCount[t] + betaTimesVocabSize);
        }
    }

    vector<double> documentToTopicProbabilities(documentsBiterms.size() * topicCount); // P_zd

    rep(d, documentsBiterms.size()) {
        // since we exhaustively collect all biterms, also the duplicates, P_bd is always 1 over the amount of biterms in that document
        const auto& docBiterms = documentsBiterms[d];
        double p_bd = 1.0 / docBiterms.size();
        vector<double> p_zd(topicCount);  // TODO remove and use documentToTopicProbabilities directly
        rep(b, docBiterms.size()) {
            vector<double> p_zb(topicCount);
            double p_zb_sum = 0;
            rep(t, topicCount) {
                p_zb[t] = topicToCount[t] * wordAndTopicToCount[docBiterms[b].first * topicCount + t] * wordAndTopicToCount[docBiterms[b].second * topicCount + t];
                p_zb_sum += p_zb[t];
            }
            rep(t, topicCount) {
                p_zb[t] /= p_zb_sum;
            }
            rep(t, topicCount) {
                p_zd[t] += p_zb[t] * p_bd; // TODO move multiplication outwards
            }
        }
        rep(t, topicCount) {
            documentToTopicProbabilities[d * topicCount + t] = p_zd[t];
        }
    }

    vector<vector<unsigned int>> topWordsPerTopic;
    topWordsPerTopic.reserve(topicCount);
    rep(t, topicCount) {
        SortedLimitedList<unsigned int, unsigned int, false> sortList(maxTopWords);
        rep(w, vocabSize) {
            sortList.add(w, wordAndTopicToCount[w * topicCount + t]);
        }
        topWordsPerTopic.push_back(sortList.getElements());
    }

    return pair(topWordsPerTopic, documentToTopicProbabilities);
}

pair<vector<unsigned int>, vector<unsigned int>> btm::gibbsSampling(const vector<Biterm> &allBiterms, unsigned int vocabSize, unsigned int iterations) const {
    auto bitermCount = allBiterms.size();

    vector<unsigned int> bitermToTopic(bitermCount);  // Z
    vector<unsigned int> wordAndTopicToCount(vocabSize * topicCount); // N_wz
    vector<unsigned int> topicToCount(topicCount); // N_z

    // first, assign one of the topics randomly to each biterm
    rep(i, allBiterms.size()) {
        auto topic = randRange(topicCount);
        const auto &biterm = allBiterms[i];
        wordAndTopicToCount[biterm.first * topicCount + topic]++;
        wordAndTopicToCount[biterm.second * topicCount + topic]++;
        topicToCount[topic]++;
        bitermToTopic[i] = topic;
    }

    // perform gibbs iterations!
    double betaTimesVocabSize = beta * (double)vocabSize;
    rep(_, iterations) {
        std::cout << "Iteration " << _ << std::endl;
        rep(i, allBiterms.size()) {
            const auto &biterm = allBiterms[i];
            // remove this biterm from its old topic
            wordAndTopicToCount[biterm.first * topicCount + bitermToTopic[i]]--;
            wordAndTopicToCount[biterm.second * topicCount + bitermToTopic[i]]--;
            topicToCount[bitermToTopic[i]]--;

            // See equation 4 in the short text BTM paper
            vector<double> topicProbabilities(topicCount); // P_z, but not normalized, as the c++ random lib does that
            rep(t, topicCount) {
                topicProbabilities[t] = (topicToCount[t] + alpha)
                                        * (wordAndTopicToCount[biterm.first * topicCount + t] + beta)
                                        * (wordAndTopicToCount[biterm.second * topicCount + t] + beta)
                                        / square(2 * topicToCount[t] + betaTimesVocabSize);
            }
            // convert list of topic probabilities to discrete random distribution object
            std::discrete_distribution<> discrete_dist(all(topicProbabilities));
            bitermToTopic[i] = discrete_dist(my_btm_randomEngine); // assign a new topic

            // register this biterm to its new topic
            wordAndTopicToCount[biterm.first * topicCount + bitermToTopic[i]]++;
            wordAndTopicToCount[biterm.second * topicCount + bitermToTopic[i]]++;
            topicToCount[bitermToTopic[i]]++;
        }
    }

    return pair(topicToCount, wordAndTopicToCount);
}
