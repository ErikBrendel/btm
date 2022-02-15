
#include "SortedLimitedList.h"

#include <algorithm>

template<typename T, typename V, bool ascending>
SortedLimitedList<T, V, ascending>::SortedLimitedList(unsigned int maxCapacity) :
        maxCapacity{maxCapacity} {
    container.reserve(maxCapacity + 1);
}

template<typename T, typename V, bool ascending>
void SortedLimitedList<T, V, ascending>::add(T element, V value) {
    // TODO update and use cutoff_value to throw away values that can immediately be thrown away, speeding up the data structure?
    container.emplace_back(element, value);
    sort(all(container), [](const pair<T, V>& a, const pair<T, V>& b) {
        return ascending ? a.second < b.second : b.second < a.second;
    });
    if (container.size() > maxCapacity) {
        container.pop_back();
    }
}

template<typename T, typename V, bool ascending>
const vector<pair<T, V>>& SortedLimitedList<T, V, ascending>::get() const {
    return container;
}

template<typename T, typename V, bool ascending>
vector<T> SortedLimitedList<T, V, ascending>::getElements() const {
    vector<T> result(container.size());
    rep(i, container.size()) {
        result[i] = container[i].first;
    }
    return result;
}

