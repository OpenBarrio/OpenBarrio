// eastl::sort exactly as compiled into TS3W.exe 1.67.2 (an introsort: quicksort with a
// median-of-three pivot, heap sort when the depth limit runs out, insertion sort for the
// final pass). Unstable like the original, and with the same tie order, so results match
// the game's even for equal keys. Transcribed from the decompiled instantiations
// (0x004ABFB0 / 0x004ABA10 / 0x004AB2F0 / 0x004A9CF0 / 0x004A9D50 / 0x004AB360 and the heap
// helpers 0x004AA910 / 0x004AA340 / 0x004A9F70 / 0x004AA970). Portable: x86 and x64.
#pragma once

#include <cstddef>
#include <utility>

namespace ts3::eastl_compat {

namespace detail {

inline constexpr std::ptrdiff_t kQuickSortLimit = 28;

// Number of significant bits of n (the original's "log2" loop).
inline int BitCount(std::ptrdiff_t n)
{
    int bits = 0;
    for (; n != 0; n >>= 1)
        ++bits;
    return bits;
}

// Pivot choice from first, middle, last-1 (exact comparison order of the original).
template <class T, class Less>
T* MedianOfThree(T* first, T* last, Less less)
{
    T* a = first;
    T* b = first + (last - first) / 2;
    T* c = last - 1;
    T* low = b;   // the smaller of a, b ...
    T* high = a;  // ... and the larger
    if (less(*a, *b)) {
        low = a;
        high = b;
    }
    if (!less(*high, *c)) {  // c <= high
        high = low;
        if (less(*low, *c))
            high = c;
    }
    return high;
}

template <class T, class Less>
T* Partition(T* first, T* last, const T& pivot, Less less)
{
    for (;;) {
        while (less(*first, pivot))
            ++first;
        do
            --last;
        while (less(pivot, *last));
        if (last <= first)
            return first;
        std::swap(*first, *last);
        ++first;
    }
}

template <class T, class Less>
void PushHeap(T* first, std::ptrdiff_t top, std::ptrdiff_t position, const T& value, Less less)
{
    while (top < position) {
        const std::ptrdiff_t parent = (position - 1) >> 1;
        if (!less(first[parent], value))
            break;
        first[position] = first[parent];
        position = parent;
    }
    first[position] = value;
}

template <class T, class Less>
void AdjustHeap(T* first, std::ptrdiff_t top, std::ptrdiff_t size, std::ptrdiff_t position, const T& value, Less less)
{
    std::ptrdiff_t child = position * 2 + 2;
    while (child < size) {
        if (less(first[child], first[child - 1]))
            --child;
        first[position] = first[child];
        position = child;
        child = position * 2 + 2;
    }
    if (child == size) {
        first[position] = first[child - 1];
        position = child - 1;
    }
    PushHeap(first, top, position, value, less);
}

template <class T, class Less>
void HeapSort(T* first, T* last, Less less)
{
    const std::ptrdiff_t n = last - first;
    if (n > 1) {  // make_heap
        for (std::ptrdiff_t parent = (n - 2) / 2 + 1; parent-- != 0;) {
            const T value = first[parent];
            AdjustHeap(first, parent, n, parent, value, less);
        }
    }
    for (std::ptrdiff_t len = n; len > 1; --len) {  // sort_heap
        const T value = first[len - 1];
        first[len - 1] = first[0];
        AdjustHeap(first, 0, len - 1, 0, value, less);
    }
}

template <class T, class Less>
void QuickSortImpl(T* first, T* last, int depth, Less less)
{
    while (last - first > kQuickSortLimit && depth > 0) {
        const T pivot = *MedianOfThree(first, last, less);
        T* mid = Partition(first, last, pivot, less);
        --depth;
        QuickSortImpl(mid, last, depth, less);
        last = mid;
    }
    if (depth == 0)
        HeapSort(first, last, less);
}

template <class T, class Less>
void InsertionSort(T* first, T* last, Less less)
{
    if (first == last)
        return;
    for (T* i = first + 1; i != last; ++i) {
        const T value = *i;
        T* j = i;
        while (j != first && less(value, j[-1])) {
            *j = j[-1];
            --j;
        }
        *j = value;
    }
}

// Unguarded: relies on a smaller-or-equal element before `first`.
template <class T, class Less>
void InsertionSortUnguarded(T* first, T* last, Less less)
{
    for (; first != last; ++first) {
        const T value = *first;
        T* j = first;
        while (less(value, j[-1])) {
            *j = j[-1];
            --j;
        }
        *j = value;
    }
}

}  // namespace detail

template <class T, class Less>
void Sort(T* first, T* last, Less less)
{
    if (first == last)
        return;
    const std::ptrdiff_t n = last - first;
    detail::QuickSortImpl(first, last, detail::BitCount(n) * 2 - 2, less);
    if (n > detail::kQuickSortLimit) {
        detail::InsertionSort(first, first + detail::kQuickSortLimit, less);
        detail::InsertionSortUnguarded(first + detail::kQuickSortLimit, last, less);
    } else {
        detail::InsertionSort(first, last, less);
    }
}

}  // namespace ts3::eastl_compat
