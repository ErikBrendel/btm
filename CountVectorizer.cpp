//
// Created by erik on 29.03.21.
//

#include "CountVectorizer.h"

#include "util.h"

#include <algorithm>
#include <iostream>

vector<vector<unsigned int>>
CountVectorizer::fitTransform(const vector<vector<string>> &documentWords, unsigned int maxVocabSize) {
    auto nDocs = documentWords.size();
    // fit
    unordered_map<string, unsigned int> word_freq;
    for (const auto &doc: documentWords) {
        for (const auto &word: doc) {
            auto found = word_freq.find(word);
            if (found == word_freq.end()) {
                word_freq.emplace(word, 1);
            } else {
                found->second++;
            }
        }
    }

    // TODO min-df and max-df and such filtering stuff

    vector<pair<string, unsigned int>> word_freq_sorted;
    word_freq_sorted.reserve(word_freq.size());
    for (const auto &entry: word_freq) {
        word_freq_sorted.emplace_back(entry);
    }
    // todo correct order?
    sort(all(word_freq_sorted), [](const pair<string, unsigned int> &a, const pair<string, unsigned int> &b) {
        return b.second < a.second;
    });

    vocabSize = min(maxVocabSize, (unsigned int) word_freq_sorted.size());
    std::cout << "Total amount of words: " << word_freq_sorted.size() << ", vocab size: " << vocabSize << endl;
    index_to_word.resize(vocabSize);
    word_to_index.reserve(vocabSize);
    rep(i, vocabSize) {
        string word = word_freq_sorted[i].first;
        index_to_word[i] = word;
        word_to_index.emplace(word, i);
    }

    // transform
    vector<vector<unsigned int>> result;
    result.resize(nDocs);
    rep(d, nDocs) {
        result[d].resize(vocabSize);
        for (const auto &word: documentWords[d]) {
            auto index = word_to_index.find(word);
            if (index != word_to_index.end()) {
                result[d][index->second]++;
            }
        }
    }

    return result;
}

string CountVectorizer::getWord(unsigned int index) const {
    return index_to_word[index];
}
