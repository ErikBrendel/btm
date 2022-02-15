#pragma once

#include "util.h"
#include <memory>
#include <vector>

using namespace std;

template<typename T, typename V=double, bool ascending=true>
class SortedLimitedList {
private:
    vector<pair<T, V>> container;
    unsigned int maxCapacity;

    // V cutoff_value = ascending ? numeric_limits<V>::max() : numeric_limits<V>::min();

public:
    explicit SortedLimitedList(unsigned int maxCapacity);

    void add(T element, V value);

    const vector<pair<T, V>>& get() const;

    [[nodiscard]] vector<T> getElements() const;

};

#include "SortedLimitedList.inl"
