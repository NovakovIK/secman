//
// Created by ivan-novakov on 10/13/17.
//

#ifndef SECMAN_PRIORITYQUEUE_HPP
#define SECMAN_PRIORITYQUEUE_HPP

#include <utility>
#include "BinaryHeap.hpp"

template <typename P, typename V>
class PriorityQueue
{
public:
    using PriorityType = P;
    using ValueType = V;
    using PairType = std::pair<P, V>;

    PriorityQueue() : heap([](const PairType &a, const PairType &b) -> bool {return a.first < b.first;}) {}

    void insert(PriorityType priority, ValueType value)
    {
        heap.insert(std::make_pair(priority, value));
    }

    ValueType pop()
    {
        return heap.pop().second;
    }

    std::size_t size()
    {
        return heap.size();
    }

    bool empty()
    {
        return heap.empty();
    }

private:
    BinaryHeap<std::pair<P, V>> heap;
};


#endif //SECMAN_PRIORITYQUEUE_HPP
