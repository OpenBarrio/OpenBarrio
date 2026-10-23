// Cluster unit clu_c72760_x (subsystem misc): heap/introsort over fixed-size
// float-keyed records (48-byte and 80-byte strides), rewritten from TS3W.exe. Every
// function below is instruction-for-instruction faithful to its original, including
// the compare branches: wherever the original branches on comiss with jbe the rewrite
// tests `!(a > b)` (or just `a > b` as the continue condition), so unordered NaN keys
// take the same arms. Key moves in the original go through x87 fld/fstp (a bit-exact
// copy for every finite float, Inf and QNaN; an sNaN could be quieted by the x87 and
// the game never sorts on sNaN keys — the tests document that).
//
// EA quirks kept on purpose because they affect outputs:
//  - Partition48/80 scan unbounded: a pivot VALUE that appears nowhere in the range
//    runs off the array. The introsort median-of-three always passes a pointer to an
//    in-range element, whose own slot stops both scans (a NaN pivot stops them after
//    at most one step each and the crossing check is unsigned, so it still terminates).
//  - PartialSort80's element counter is ceil((end-middle)/80): a range that is not a
//    multiple of 80 bytes walks past `end` (game ranges are always aligned).
//  - PopRoot80 reads and writes [last-0x50] even when the range is empty (reads the
//    record before `first`); SortHeap80 only calls it for n >= 2.
//  - IntroSort48/80 leave a garbage EAX (low32(magic * byteSpan)) behind when a
//    partition loop runs out with depth != 0; depth < 0 returns the same garbage (or
//    n on the first iteration). SortHeap48 returns 1 and SortHeap80 returns 48 for
//    n >= 2 instead of the element count. MakeHeap48/80 return that garbage as their
//    result for n < 2 (low32(0x2AAAAAAB*span) resp. low32(0x66666667*span)).
//  - CopyFrom does not copy bytes +0x02..+0x0F, does not free the old destination
//    vector, and a source span of 1..3 bytes makes 0x0069DB60 allocate nothing while
//    the memmove still writes the span (game spans are multiples of 4).
//  - CopyRefElements walks [begin, end) while begin != end: an end below begin runs
//    away, and a null destination cell still advances — after one skipped element the
//    cursor is 16, which passes the null test, so the ORIGINAL faults writing 0x00000010
//    (the game never calls it with a null cell and more than one element).
#include "ts3/misc/clu_c72760_x.h"

#include <cstring>

namespace ts3::misc {
namespace {

constexpr int kStride48 = 48;
constexpr int kStride80 = 80;
constexpr int kMove48 = 0x30;  // what a 48-record move copies: key + [0x10,0x30)
constexpr int kMove80 = 0x4C;  // what an 80-record move copies: [0,0x4C)

float Key48(const char* record)
{
    float key;
    std::memcpy(&key, record, sizeof key);
    return key;
}

float Key80(const char* record)
{
    float key;
    std::memcpy(&key, record + 0x44, sizeof key);
    return key;
}

// The originals move a 48-record's key with fld/fstp and its tail with movaps;
// bytes +0x04..+0x0F stay with the array slot and are never copied.
void Move48(char* dst, const char* src)
{
    std::memcpy(dst, src, 4);
    std::memcpy(dst + 0x10, src + 0x10, 0x20);
}

// The 80-record move covers [0,0x4C); bytes +0x4C..+0x4F stay with the slot.
void Move80(char* dst, const char* src)
{
    std::memcpy(dst, src, kMove80);
}

char* At48(char* base, int index)
{
    return base + static_cast<std::ptrdiff_t>(index) * kStride48;
}

const char* At48(const char* base, int index)
{
    return base + static_cast<std::ptrdiff_t>(index) * kStride48;
}

char* At80(char* base, int index)
{
    return base + static_cast<std::ptrdiff_t>(index) * kStride80;
}

const char* At80(const char* base, int index)
{
    return base + static_cast<std::ptrdiff_t>(index) * kStride80;
}

// The garbage EAX the division idiom leaves behind (low32 of a 32x32 multiply).
char* Junk48(std::ptrdiff_t byteSpan)
{
    const uint32_t bits = 0x2AAAAAABu * static_cast<uint32_t>(static_cast<int32_t>(byteSpan));
    return reinterpret_cast<char*>(static_cast<uintptr_t>(bits));
}

char* Junk80(std::ptrdiff_t byteSpan)
{
    const uint32_t bits = 0x66666667u * static_cast<uint32_t>(static_cast<int32_t>(byteSpan));
    return reinterpret_cast<char*>(static_cast<uintptr_t>(bits));
}

int32_t Junk48Int(std::ptrdiff_t byteSpan)
{
    return static_cast<int32_t>(0x2AAAAAABu * static_cast<uint32_t>(static_cast<int32_t>(byteSpan)));
}

int32_t Junk80Int(std::ptrdiff_t byteSpan)
{
    return static_cast<int32_t>(0x66666667u * static_cast<uint32_t>(static_cast<int32_t>(byteSpan)));
}

}  // namespace

// ---------------------------------------------------------------------------
// 48-stride family
// ---------------------------------------------------------------------------

// TS3W.exe 0x00C72760
char* PushUp48(char* base, int top, int hole, const char* value)
{
    int parent = (hole - 1) >> 1;
    while (hole > top && Key48(value) > Key48(At48(base, parent))) {
        Move48(At48(base, hole), At48(base, parent));
        hole = parent;
        parent = (hole - 1) >> 1;
    }
    char* dst = At48(base, hole);
    Move48(dst, value);
    return dst;
}

// TS3W.exe 0x00C74510
char* SiftDown48(char* base, int top, int count, int hole, const char* value)
{
    int right = hole * 2 + 2;
    while (right < count) {
        // The left child wins only when strictly greater than the right one
        // (comiss left,right; jbe keeps the right child).
        if (Key48(At48(base, right - 1)) > Key48(At48(base, right)))
            --right;
        Move48(At48(base, hole), At48(base, right));
        hole = right;
        right = hole * 2 + 2;
    }
    if (right == count) {  // lone left child at count-1
        Move48(At48(base, hole), At48(base, count - 1));
        hole = count - 1;
    }
    return PushUp48(base, top, hole, value);
}

// TS3W.exe 0x00C756E0
char* MakeHeap48(char* first, char* last)
{
    const int32_t span = static_cast<int32_t>(last - first);
    const int n = span / kStride48;
    if (n < 2)
        return Junk48(span);  // garbage EAX, kept
    int node = ((n - 2) >> 1) + 1;
    char* result = nullptr;
    for (;;) {
        --node;
        alignas(16) char buffer[kMove48] = {};
        const char* record = At48(first, node);
        std::memcpy(buffer, record, 4);
        std::memcpy(buffer + 0x10, record + 0x10, 0x20);
        result = SiftDown48(first, node, n, node, buffer);
        if (node == 0)
            break;
    }
    return result;
}

// TS3W.exe 0x00C75770
int SortHeap48(char* first, char* last)
{
    const int n = static_cast<int32_t>(last - first) / kStride48;
    if (n <= 1)
        return n;
    int remaining = n;
    char* heapEnd = last;
    for (;;) {
        char* tailRecord = heapEnd - kStride48;
        alignas(16) char buffer[kMove48];
        std::memcpy(buffer, tailRecord, 4);
        std::memcpy(buffer + 0x10, tailRecord + 0x10, 0x20);
        std::memcpy(tailRecord, first, 4);  // root key -> last slot (fld/fstp)
        std::memcpy(tailRecord + 0x10, first + 0x10, 0x20);
        SiftDown48(first, 0, remaining - 1, 0, buffer);
        heapEnd -= kStride48;
        --remaining;
        if (remaining <= 1)
            break;
    }
    return remaining;  // 1 for n >= 2 (the original's loop exit value)
}

// TS3W.exe 0x00C77B30
char* Partition48(char* first, char* last, const char* pivot)
{
    const float pivotKey = Key48(pivot);
    char* i = first;
    char* j = last;
    for (;;) {
        float iKey;
        for (;;) {
            iKey = Key48(i);
            if (!(pivotKey > iKey))  // comiss pivot,elem; jbe stops the scan
                break;
            i += kStride48;
        }
        float jKey;
        for (;;) {  // steps back once before testing (reads [last-0x30] first)
            j -= kStride48;
            jKey = Key48(j);
            if (!(jKey > pivotKey))  // comiss elem,pivot; ja continues
                break;
        }
        if (reinterpret_cast<uintptr_t>(i) >= reinterpret_cast<uintptr_t>(j))  // unsigned jae
            return i;
        char tail[0x20];
        std::memcpy(tail, i + 0x10, sizeof tail);
        std::memcpy(i, j, 4);  // i.key = j.key
        std::memcpy(i + 0x10, j + 0x10, 0x20);
        std::memcpy(j, &iKey, 4);  // j.key = the key the left scan loaded
        std::memcpy(j + 0x10, tail, sizeof tail);
        i += kStride48;
    }
}

// TS3W.exe 0x00C77BA0
int PartialSort48(char* first, char* middle, char* end)
{
    MakeHeap48(first, middle);
    char* element = middle;
    while (reinterpret_cast<uintptr_t>(element) < reinterpret_cast<uintptr_t>(end)) {  // unsigned jb
        const float elementKey = Key48(element);
        const float rootKey = Key48(first);
        if (rootKey > elementKey) {  // comiss root,element; jbe skips
            alignas(16) char buffer[kMove48];
            std::memcpy(buffer, &elementKey, 4);
            std::memcpy(buffer + 0x10, element + 0x10, 0x20);
            std::memcpy(element, &rootKey, 4);  // element takes the root
            std::memcpy(element + 0x10, first + 0x10, 0x20);
            const int count = static_cast<int32_t>(middle - first) / kStride48;
            SiftDown48(first, 0, count, 0, buffer);
        }
        element += kStride48;
    }
    return SortHeap48(first, middle);
}

// TS3W.exe 0x00C78350 (entry point)
int IntroSort48(char* first, char* last, int depth)
{
    int n = static_cast<int32_t>(last - first) / kStride48;
    int result = n;  // EAX after the entry division (returned when depth != 0)
    for (;;) {
        if (n <= 28) {
            if (depth != 0)
                return result;
            return PartialSort48(first, last, last);
        }
        if (depth <= 0) {
            if (depth != 0)
                return result;  // garbage EAX after a partition loop, kept
            return PartialSort48(first, last, last);
        }
        n = static_cast<int32_t>(last - first) / kStride48;  // recomputed each round
        const float firstKey = Key48(first);
        const float lastKey = Key48(last - kStride48);
        const int mid = n / 2;
        const char* midRecord = At48(first, mid);
        const float midKey = Key48(midRecord);
        const char* lastRecord = last - kStride48;
        const char* pivot;
        if (midKey > firstKey) {
            if (lastKey > midKey)
                pivot = midRecord;
            else if (lastKey > firstKey)
                pivot = lastRecord;
            else
                pivot = first;
        } else {
            if (lastKey > firstKey)
                pivot = first;
            else if (lastKey > midKey)
                pivot = lastRecord;
            else
                pivot = midRecord;
        }
        char* split = Partition48(first, last, pivot);
        --depth;
        IntroSort48(split, last, depth);  // recursion result ignored
        last = split;
        result = Junk48Int(split - first);  // garbage EAX, kept
        n = static_cast<int32_t>(split - first) / kStride48;
    }
}

// ---------------------------------------------------------------------------
// 80-stride family
// ---------------------------------------------------------------------------

// TS3W.exe 0x00C72850
char* PushUp80(char* base, int top, int hole, const char* value)
{
    int parent = (hole - 1) >> 1;
    while (hole > top && Key80(value) > Key80(At80(base, parent))) {
        Move80(At80(base, hole), At80(base, parent));
        hole = parent;
        parent = (hole - 1) >> 1;
    }
    char* dst = At80(base, hole);
    Move80(dst, value);
    return dst;
}

// TS3W.exe 0x00C72920
char* SwapRecords80(char* a, char* b)
{
    char tmp[kMove80];
    std::memcpy(tmp, a, kMove80);
    std::memcpy(a, b, kMove80);
    std::memcpy(b, tmp, kMove80);
    return a;  // EAX still holds the first argument
}

// TS3W.exe 0x00C745C0
char* SiftDown80(char* base, int top, int count, int hole, const char* value)
{
    int right = hole * 2 + 2;
    while (right < count) {
        if (Key80(At80(base, right - 1)) > Key80(At80(base, right)))
            --right;
        Move80(At80(base, hole), At80(base, right));
        hole = right;
        right = hole * 2 + 2;
    }
    if (right == count) {
        Move80(At80(base, hole), At80(base, count - 1));
        hole = count - 1;
    }
    return PushUp80(base, top, hole, value);
}

// TS3W.exe 0x00C746C0
char* PopRoot80(char* first, char* last)
{
    alignas(16) char buffer[kMove80];
    std::memcpy(buffer, last - kStride80, kMove80);  // save the last record
    std::memcpy(last - kStride80, first, kMove80);   // root takes its slot
    const int count = static_cast<int32_t>(last - first) / kStride80 - 1;
    return SiftDown80(first, 0, count, 0, buffer);
}

// TS3W.exe 0x00C75820
char* MakeHeap80(char* first, char* last)
{
    const int32_t span = static_cast<int32_t>(last - first);
    const int n = span / kStride80;
    if (n < 2)
        return Junk80(span);  // garbage EAX, kept
    int node = ((n - 2) >> 1) + 1;
    char* result = nullptr;
    for (;;) {
        --node;
        alignas(16) char buffer[kMove80];
        std::memcpy(buffer, At80(first, node), kMove80);
        result = SiftDown80(first, node, n, node, buffer);
        if (node == 0)
            break;
    }
    return result;
}

// TS3W.exe 0x00C758D0
int SortHeap80(char* first, char* last)
{
    const int32_t span0 = static_cast<int32_t>(last - first);
    const int n = span0 / kStride80;
    if (n <= 1)
        return n;
    int32_t span = span0;
    char* heapEnd = last;
    for (;;) {
        PopRoot80(first, heapEnd);
        span -= kStride80;
        heapEnd -= kStride80;
        if (span / kStride80 > 1)
            continue;
        break;
    }
    return Junk80Int(span);  // low32(0x66666667 * span) = 48 for aligned ranges
}

// TS3W.exe 0x00C77C40
char* Partition80(char* first, char* last, const char* pivot)
{
    const float pivotKey = Key80(pivot);
    char* i = first;
    char* j = last;
    for (;;) {
        float iKey;
        for (;;) {
            iKey = Key80(i);
            if (!(pivotKey > iKey))
                break;
            i += kStride80;
        }
        float jKey;
        for (;;) {
            j -= kStride80;
            jKey = Key80(j);
            if (!(jKey > pivotKey))
                break;
        }
        if (reinterpret_cast<uintptr_t>(i) >= reinterpret_cast<uintptr_t>(j))
            return i;
        SwapRecords80(i, j);  // the original calls 0x00C72920 here
        i += kStride80;
    }
}

// TS3W.exe 0x00C77CA0
int PartialSort80(char* first, char* middle, char* end)
{
    MakeHeap80(first, middle);
    uint32_t visits = 0;
    if (reinterpret_cast<uintptr_t>(middle) < reinterpret_cast<uintptr_t>(end)) {
        const uint32_t span = static_cast<uint32_t>(static_cast<int32_t>(end - middle));
        visits = (span - 1u) / kStride80 + 1u;  // ceil(span / 80), EA counter
    }
    char* element = middle;
    for (uint32_t v = 0; v < visits; ++v, element += kStride80) {
        const float elementKey = Key80(element);
        const float rootKey = Key80(first);
        if (rootKey > elementKey) {
            alignas(16) char buffer[kMove80];
            std::memcpy(buffer, element, kMove80);  // full element, read first
            std::memcpy(element, first, kMove80);   // root takes its place
            const int count = static_cast<int32_t>(middle - first) / kStride80;
            SiftDown80(first, 0, count, 0, buffer);
        }
    }
    return SortHeap80(first, middle);
}

// TS3W.exe 0x00C78430 (entry point)
int IntroSort80(char* first, char* last, int depth)
{
    int n = static_cast<int32_t>(last - first) / kStride80;
    int result = n;
    for (;;) {
        if (n <= 28) {
            if (depth != 0)
                return result;
            return PartialSort80(first, last, last);
        }
        if (depth <= 0) {
            if (depth != 0)
                return result;
            return PartialSort80(first, last, last);
        }
        n = static_cast<int32_t>(last - first) / kStride80;
        const float firstKey = Key80(first);
        const float lastKey = Key80(last - kStride80);
        const int mid = n / 2;
        const char* midRecord = At80(first, mid);
        const float midKey = Key80(midRecord);
        const char* lastRecord = last - kStride80;
        const char* pivot;
        if (midKey > firstKey) {
            if (lastKey > midKey)
                pivot = midRecord;
            else if (lastKey > firstKey)
                pivot = lastRecord;
            else
                pivot = first;
        } else {
            if (lastKey > firstKey)
                pivot = first;
            else if (lastKey > midKey)
                pivot = lastRecord;
            else
                pivot = midRecord;
        }
        char* split = Partition80(first, last, pivot);
        --depth;
        IntroSort80(split, last, depth);  // recursion result ignored
        last = split;
        result = Junk80Int(split - first);
        n = static_cast<int32_t>(split - first) / kStride80;
    }
}

// ---------------------------------------------------------------------------
// Standalone entry points
// ---------------------------------------------------------------------------

// TS3W.exe 0x00C7D700 (__thiscall, ret 4)
VecCopyObject* VecCopyObject::CopyFrom(const VecCopyObject* src)
{
    byte00 = src->byte00;
    byte01 = src->byte01;
    std::memcpy(body, src->body, 0x40);  // bytes +0x02..+0x0F are NOT copied
    const uint32_t span = static_cast<uint32_t>(
        static_cast<int32_t>(reinterpret_cast<uintptr_t>(src->vector.end) -
                             reinterpret_cast<uintptr_t>(src->vector.begin)));
    const int32_t count = static_cast<int32_t>(span) >> 2;
    // Re-creates the destination vector (context dword + allocation) through the
    // game's 0x0069DB60, then copies the elements. x64 has no game code to call.
    CallGameMethod<void, 0x69DB60>(&vector, count, &src->vector.context);
    void* const moved = std::memmove(vector.begin, src->vector.begin, static_cast<size_t>(span));
    vector.end = reinterpret_cast<uint32_t*>(static_cast<char*>(moved) +
                                             (static_cast<int32_t>(span) >> 2) * 4);
    return this;
}

// TS3W.exe 0x00C7F140 (entry point, cdecl)
char* CopyRefElements(char* cursor, const char* begin, const char* end, uint32_t init)
{
    uint32_t* cell = reinterpret_cast<uint32_t*>(cursor);
    *cell = init;
    const char* element = begin;
    while (element != end) {  // equality only: end < begin runs away (EA hazard)
        uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(*cell));
        if (dst) {
            const uint32_t* fields = reinterpret_cast<const uint32_t*>(element);
            const uint32_t ref0 = fields[0];
            const uint32_t ref1 = fields[1];
            dst[0] = ref0;
            if (ref0)  // virtual slot 0 of the game object, no stack arguments
                CallGameVirtual<void, 0>(reinterpret_cast<void*>(static_cast<uintptr_t>(ref0)));
            dst[1] = ref1;
            if (ref1)
                CallGameVirtual<void, 0>(reinterpret_cast<void*>(static_cast<uintptr_t>(ref1)));
            dst[2] = fields[2];
            dst[3] = fields[3];
        }
        *cell += 16;  // advances even through a null cell
        element += 16;
    }
    return cursor;
}

}  // namespace ts3::misc
