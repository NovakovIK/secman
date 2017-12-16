#ifndef SECMAN_PRIORITYQUEUE_HPP
#define SECMAN_PRIORITYQUEUE_HPP


#include <queue>


template <typename Key, typename Value>
class PriorityQueue
{
public:
    using Pair = std::pair<Key, Value>;

    void insert(const Key &key, const Value &value)
    {
        queue.emplace(key, value);
    }

    const Pair & top() const
    {
        return queue.top();
    }

    Pair pop()
    {
        auto t = top();
        queue.pop();
        return t;
    }

private:
    template <typename Pair>
    class FirstLess
    {
    public:
        bool operator() (const Pair &left, const Pair &right)
        {
            return left.first < right.first;
        }
    };

    std::priority_queue<Pair, std::vector<Pair>, FirstLess<Pair>> queue;
};


#endif