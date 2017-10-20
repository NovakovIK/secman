//
// Created by ivan-novakov on 10/13/17.
//

#ifndef SECMAN_BINARYHEAP_HPP
#define SECMAN_BINARYHEAP_HPP

#include <functional>

template <typename T>
class  BinaryHeap
{
public:
    explicit BinaryHeap(std::function<bool(const T&, const T&)> comparator=std::less<T>::operator()) : compare(comparator) {}

    void insert(T value)
    {
        array.push_back(value);

        for (std::size_t p, i = array.size() - 1; i != 0; i = p)
        {
            p = parent(i);
            if (!compare(array[i], array[p]))
                break;

            std::swap(array[p], array[i]);
        }
    }

    T pop()
    {
        T value = array[0];
        array[0] = array.back();;
        array.pop_back();

        std::size_t i = 0;
        while (true)
        {
            if ((left(i) >= array.size() || !compare(array[left(i)], array[i])) &&
                (right(i) >= array.size() || !compare(array[right(i)], array[i])))
                break;

            else if (right(i) == array.size() || compare(array[left(i)], array[right(i)]))
            {
                std::swap(array[i], array[left(i)]);
                i = left(i);
            }
            else
            {
                std::swap(array[i], array[right(i)]);
                i = right(i);
            }
        }

        return value;
    }

    bool empty()
    {
        return array.empty();
    }

    std::size_t size()
    {
        return array.size();
    }

private:
    std::vector<T> array;
    std::function<bool(const T&, const T&)> compare;

    inline std::size_t parent(std::size_t i) {return (i-1) / 2;}
    inline std::size_t left(std::size_t i)   {return i * 2 + 1;}
    inline std::size_t right(std::size_t i)  {return i * 2 + 2;}
};

#endif //SECMAN_BINARYHEAP_HPP
