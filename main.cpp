#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include "Btm.h"
#include "ParallelBtm.h"
#include "SyncParallelBtm.h"
#include "CountVectorizer.h"
#include "SortedLimitedList.h"
#include "util.h"
#include "argparse.h"

using namespace std;

/**
 * get the list of indices that are not zero in this vector
 */
vector<unsigned int> nonzero(const vector<unsigned int> &data) {
    vector<unsigned int> result;
    rep(i, data.size()) {
        if (data[i] != 0) result.emplace_back(i);
    }
    return result;
}

/**
 * get the list of indices that are not zero in this vector
 */
unsigned int count_nonzero(const vector<unsigned int> &data) {
    unsigned int result = 0;
    for (unsigned int elem : data) {
        if (elem != 0) result++;
    }
    return result;
}

vector<vector<Biterm>> vec2Biterms(const vector<vector<unsigned int>> &docWord) {
    unsigned int nDocs = docWord.size();

    vector<vector<Biterm>> result;
    result.resize(nDocs);
    rep(d, nDocs) {
        auto words = nonzero(docWord[d]);
        rep(i1, words.size()) {
            rep(i2, i1) {
                // TODO this does not reflect the amount of times that a word occurs, and also not self-pairs if any
                result[d].emplace_back(words[i2], words[i1]);
            }
        }
    }

    return result;
}


vector<vector<string>> readInputDocuments() {
    /*vector<vector<string>> documents = {
            {"this", "is", "a", "test", "is", "nice"},
            {"test", "some", "more"},
            {"doc3", "is", "different"},
    };*/

    vector<vector<string>> documents;

    string line, word;
    while (getline(cin, line)) {
        documents.emplace_back();
        stringstream ss(line);
        while (ss >> word) {
            documents.back().push_back(word);
        }
    }
    return documents;
}

int main(int argc, const char* argv[]) {
    auto threadCount = thread::hardware_concurrency();
    if (threadCount <= 0) threadCount = 4;
    SyncParallelBtm model;
    model.workerThreadCount = getArg("workerThreadCount", threadCount - 1);
    model.topicCount = getArg("topicCount", model.topicCount);
    model.maxTopWords = getArg("maxTopWords", model.maxTopWords);
    auto maxTopDocuments = getArg("maxTopDocuments", 5);
    auto maxVocabSize = getArg("maxVocabSize", 1000);
    auto iterations = getArg("iterations", 200);


    vector<vector<string>> documents = readInputDocuments();

    CountVectorizer vec;

    // for each document: for each vocab: how often does it appear
    vector<vector<unsigned int>> X = vec.fitTransform(documents, maxVocabSize);

    // for each document: list of occurring biterms, which are indices according to the countVectorizer
    vector<vector<Biterm>> documentsBiterms = vec2Biterms(X);


    model.X = X;
    auto[topWordsPerTopic, documentToTopicProbabilities] = model.fitTransform(documentsBiterms, vec.vocabSize, iterations); // called P_wz and P_zd

    cout << endl << "Top documents per topic:" << endl;
    rep(t, model.topicCount) {
        cout << "Topic " << t << ":";
        for (const unsigned int word: topWordsPerTopic[t]) {
            cout << " " << vec.getWord(word);
        }
        cout << endl;

        double coherence = 0; // C_z
        for (unsigned int m = 1; m < topWordsPerTopic.size(); m++) {
            for (unsigned int l = 0; l < m; l++) {
                // topWordsPerTopic is V_z
                double D_vmvl = 1; // add 1 to smooth out and not take log of zero
                double D_l = 0;
                for (const auto &doc: X) {
                    if (doc[topWordsPerTopic[t][l]] > 0) {
                        D_l++;
                        if (doc[topWordsPerTopic[t][m]] > 0) {
                            D_vmvl++;
                        }
                    }
                }
                if (D_l > 0) {
                    coherence += log(D_vmvl / D_l);
                }
            }
        }
        cout << "->Coherence: " << coherence << endl;
        SortedLimitedList<unsigned int, double, false> docSorter(maxTopDocuments);
        rep(d, documents.size()) {
            docSorter.add(d, documentToTopicProbabilities[d * model.topicCount + t]);
        }
        for (const unsigned int docId: docSorter.getElements()) {
            cout << "  ";
            for (const auto &word: documents[docId]) {
                cout << " " << word;
            }
            cout << endl;
        }
        cout << endl;
    }

    // return the doc-top matrix
    rep(d, documents.size()) {
        cout << "#doctop ";
        rep(t, model.topicCount) {
            if (t != 0) cout << ",";
            cout << documentToTopicProbabilities[d * model.topicCount + t];
        }
        cout << endl;
    }

    return 0;
}
