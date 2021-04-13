#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class CountVectorizer {
public:
    unsigned int vocabSize;
private:
    unordered_map<string, int> word_to_index;
    vector<string>  index_to_word;
public:
    vector<vector<unsigned int>> fitTransform(const vector<vector<string>>& documentWords, unsigned int maxVocabSize);
    string getWord(unsigned int index) const;
};


