#ifndef MAX_HEAP_HPP
#define MAX_HEAP_HPP

#include <vector>
#include <utility>
#include <stdexcept>

// Array-backed max heap. The comparator must return true when its first
// argument should rank ahead of its second argument.
template <typename T, typename HigherPriority>
class MaxHeap {
private:
    std::vector<T> items;
    HigherPriority higherPriority;

    void siftUp(size_t index) {
        while (index > 0) {
            const size_t parent = (index - 1) / 2;
            if (!higherPriority(items[index], items[parent])) break;
            std::swap(items[index], items[parent]);
            index = parent;
        }
    }

    void siftDown(size_t index) {
        while (true) {
            const size_t left = index * 2 + 1;
            const size_t right = left + 1;
            size_t best = index;
            if (left < items.size() && higherPriority(items[left], items[best])) best = left;
            if (right < items.size() && higherPriority(items[right], items[best])) best = right;
            if (best == index) break;
            std::swap(items[index], items[best]);
            index = best;
        }
    }

public:
    explicit MaxHeap(HigherPriority comparator = HigherPriority())
        : higherPriority(std::move(comparator)) {}

    bool empty() const { return items.empty(); }
    size_t size() const { return items.size(); }

    void insert(const T& value) {
        items.push_back(value);
        siftUp(items.size() - 1);
    }

    const T& top() const {
        if (empty()) throw std::out_of_range("Cannot inspect an empty MaxHeap.");
        return items.front();
    }

    T extractMax() {
        if (empty()) throw std::out_of_range("Cannot extract from an empty MaxHeap.");
        T result = std::move(items.front());
        if (items.size() == 1) {
            items.pop_back();
            return result;
        }
        items.front() = std::move(items.back());
        items.pop_back();
        siftDown(0);
        return result;
    }
};

#endif
