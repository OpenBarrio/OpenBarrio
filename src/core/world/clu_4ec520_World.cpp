#include "ts3/world/clu_4ec520_World.h"

#include <cstdint>
#include <cstring>

namespace ts3::world {

namespace {

// The game's case table at 0x0114CB30 is the identity except 'A'..'Z' -> 'a'..'z'
// (verified byte by byte). Only characters below 0x100 are folded.
uint16_t FoldCase(uint16_t c)
{
    if (c >= u'A' && c <= u'Z') return static_cast<uint16_t>(c + 0x20);
    return c;
}

// The original uses `sar reg, 1`; C++'s signed >> is implementation-defined, this is not.
int32_t ShiftRight1(int32_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value);
    return static_cast<int32_t>((bits >> 1) | (bits & 0x80000000u));
}

int32_t ShiftRight2(int32_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value);
    return static_cast<int32_t>((bits >> 2) | (bits & 0xC0000000u));
}

// The address of the game's empty wide string (begin/end) and its one-character capacity
// bracket, plus the allocator object 0x0107916C the string helpers understand.
constexpr uintptr_t kEmptyWideStringBegin = 0x011C7E54;
constexpr uintptr_t kEmptyWideStringCapacity = 0x011C7E56;
constexpr uintptr_t kWideStringAllocator = 0x0107916C;

void FreeGameStringIfHeap(const WorldWideString& string)
{
    const int32_t span = static_cast<int32_t>(reinterpret_cast<uintptr_t>(string.capacity) -
                                              reinterpret_cast<uintptr_t>(string.begin)) &
                         static_cast<int32_t>(0xFFFFFFFEu);
    if (span > 2 && string.begin != nullptr) {
        CallGameFunction<void, 0x004E3F70>(string.begin);
    }
}

}  // namespace

int32_t ComparePathRanges(const wchar_t* firstBegin, const wchar_t* firstEnd,
                          const wchar_t* secondBegin, const wchar_t* secondEnd, int32_t mode)
{
    const auto* first = reinterpret_cast<const uint16_t*>(firstBegin);
    const auto* second = reinterpret_cast<const uint16_t*>(secondBegin);
    const int32_t firstLength = ShiftRight1(static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(firstEnd) - reinterpret_cast<uintptr_t>(firstBegin)));
    const int32_t secondLength = ShiftRight1(static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(secondEnd) - reinterpret_cast<uintptr_t>(secondBegin)));
    const int32_t length = firstLength > secondLength ? firstLength : secondLength;
    if (length == 0) return 0;

    uint32_t remaining = static_cast<uint32_t>(length);
    const uint16_t separator = (static_cast<uint32_t>(mode) - 3u) > 3u ? u'\\' : u'/';
    const bool fold = mode == 2 || mode == 3 || mode == 5 || mode == 6;

    for (;;) {
        uint16_t left = first[0];
        uint16_t right = second[0];
        ++first;
        ++second;
        if (fold) {
            left = FoldCase(left);
            right = FoldCase(right);
        }
        while (left != right) {
            if (left == u'\\' || left == separator) {
                left = 0xFFFFu;
            } else if (right == u'\\' || right == separator) {
                right = 0xFFFFu;
            } else {
                return static_cast<int32_t>(right) - static_cast<int32_t>(left);
            }
        }
        if (--remaining == 0) return 0;
    }
}

const wchar_t* SplitPath(const wchar_t* path, const wchar_t** outBase,
                         const wchar_t** outFile, const wchar_t** outExt)
{
    const wchar_t* firstSeparator = nullptr;
    const wchar_t* lastSeparator = nullptr;
    const wchar_t* lastDot = nullptr;
    const wchar_t* cursor = path;

    if (*cursor != 0) {
        for (;;) {
            const wchar_t current = *cursor;
            if (current == L'\\' || current == L'/' || current == L':') {
                if (firstSeparator == nullptr) firstSeparator = cursor;
                lastSeparator = cursor;
                lastDot = nullptr;
            }
            const wchar_t next = cursor[1];
            if (current == L'.') lastDot = cursor;
            ++cursor;
            if (next == 0) break;
        }
    }

    // EAX on the paths below: the NUL position, or the UNC host end the scan stopped at.
    const wchar_t* end = cursor;
    *outBase = path;
    if (path[0] == L'\\' && path[1] == L'\\') {
        // UNC: skip "\\", then scan the host name up to the second backslash, '/', ':' or NUL.
        const wchar_t* host = path + 2;
        int backslashes = 0;
        uint16_t current = static_cast<uint16_t>(host[0]);
        for (;;) {
            if (current == 0) break;
            if (current == L'\\') {
                if (++backslashes == 2) break;
            } else if (current == L'/' || current == L':') {
                break;
            }
            current = static_cast<uint16_t>(host[1]);
            ++host;
        }
        end = host;
        *outBase = host;
        if (lastSeparator != nullptr &&
            reinterpret_cast<uintptr_t>(lastSeparator) < reinterpret_cast<uintptr_t>(host)) {
            // mov bx, [eax]; neg bx; sbb ebx, ebx; and ebx, eax -- keep the host end as the
            // new last separator, or drop it when the host name is empty.
            lastSeparator = *host != 0 ? host : nullptr;
        }
    } else if (firstSeparator != nullptr && firstSeparator[0] == L':') {
        ++firstSeparator;
        *outBase = firstSeparator;
    }

    *outFile = *outBase;
    if (lastSeparator != nullptr) *outFile = lastSeparator + 1;

    if (lastDot != nullptr) {
        *outExt = lastDot;
        // Original returns EAX = the outExt argument address on this path.
        return reinterpret_cast<const wchar_t*>(outExt);
    }
    *outExt = end;
    while (**outExt != 0) *outExt = *outExt + 1;
    return end;
}

uint32_t CopyFileNamePart(const wchar_t* path, wchar_t* dst, uint32_t dstCount)
{
    const wchar_t* base = nullptr;
    const wchar_t* file = nullptr;
    const wchar_t* ext = nullptr;
    SplitPath(path, &base, &file, &ext);
    (void)base;
    (void)ext;
    const int32_t characters = CallGameFunction<int32_t, 0x004FE0F0>(file);
    // Original: lea eax, [file + 2*len]; sub eax, file; sar eax, 1.
    const uint32_t length = static_cast<uint32_t>(2 * characters) >> 1;
    if (length < dstCount) {
        std::memcpy(dst, file, static_cast<size_t>(2) * length);
        dst[length] = 0;
    }
    return length;
}

uint32_t CopyDirectoryPart(const wchar_t* path, wchar_t* dst, uint32_t dstCount)
{
    const wchar_t* base = nullptr;
    const wchar_t* file = nullptr;
    const wchar_t* ext = nullptr;
    SplitPath(path, &base, &file, &ext);
    (void)base;
    (void)ext;
    const wchar_t* end = file;
    if (end == nullptr) end = path + CallGameFunction<int32_t, 0x004FE0F0>(path);
    const uint32_t length = static_cast<uint32_t>(
                                reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(path)) >>
                            1;
    if (length < dstCount) {
        std::memcpy(dst, path, static_cast<size_t>(2) * length);
        dst[length] = 0;
    }
    return length;
}

int32_t ComparePathParts(int32_t from, int32_t to, const wchar_t* lhs, const wchar_t* rhs,
                         int32_t mode, char foldFlag)
{
    // SplitPath fills [1]..[3]; slot [0] is the whole path and slot [4] stays null, exactly
    // like the zeroed fifth local in the original.
    const wchar_t* left[5];
    const wchar_t* right[5];
    left[0] = lhs;
    right[0] = rhs;
    left[4] = nullptr;
    right[4] = nullptr;
    SplitPath(lhs, &left[1], &left[2], &left[3]);
    SplitPath(rhs, &right[1], &right[2], &right[3]);

    if (foldFlag == 0) {
        return ComparePathRanges(left[from], left[to], right[from], right[to], mode);
    }
    if (from >= to) return 0;
    // EA quirk kept: the same part is re-compared until it differs, which never terminates
    // when the parts compare equal.
    int32_t result;
    do {
        result = ComparePathRanges(left[from], left[from + 1], right[from], right[from + 1], mode);
    } while (result == 0);
    return result;
}

int32_t CompareWorldPaths(const wchar_t* lhs, const wchar_t* rhs, char foldFlag, int32_t mode)
{
    return ComparePathParts(0, 4, lhs, rhs, mode, foldFlag);
}

void** CopyRefElements(void** cursor, void** begin, void** end, void* first)
{
    *cursor = static_cast<void**>(first);
    while (begin != end) {
        void** current = reinterpret_cast<void**>(*cursor);
        if (current != nullptr) {
            void* value = *begin;
            *current = value;
            if (value != nullptr) CallGameVirtual<void, 1>(value);
        }
        // Advances even through a null cursor, so a null first counts the range.
        *cursor = reinterpret_cast<void**>(reinterpret_cast<char*>(*cursor) + 4);
        ++begin;
    }
    return cursor;
}

int32_t ReleaseRefElements(void** begin, void** end)
{
    int32_t result = 0;
    while (reinterpret_cast<uintptr_t>(begin) < reinterpret_cast<uintptr_t>(end)) {
        void* value = *begin;
        if (value != nullptr) result = CallGameVirtual<int32_t, 2>(value);
        ++begin;
    }
    return result;
}

WorldResourceCopy* WorldResourceCopy::CopyFrom(const void* source)
{
    const auto* src = static_cast<const uint8_t*>(source);
    std::memcpy(header, src, 0x30);
    auto* vector = reinterpret_cast<void**>(&elements);
    const int32_t count = ShiftRight2(static_cast<int32_t>(
        *reinterpret_cast<const int32_t*>(src + 0x34) -
        *reinterpret_cast<const int32_t*>(src + 0x30)));
    CallGameMethod<void, 0x0069DB60>(vector, count, const_cast<uint8_t*>(src + 0x3C));
    void* cursor;
    CopyRefElements(&cursor, *reinterpret_cast<void** const*>(src + 0x30),
                    *reinterpret_cast<void** const*>(src + 0x34), vector[0]);
    elementsEnd = static_cast<void**>(cursor);
    return this;
}

int32_t WorldRefVector::Insert(void** pos, void** value)
{
    void** currentEnd = end;
    if (currentEnd != capacity) {
        void** insertAt = pos;
        void** element = value;
        if (reinterpret_cast<uintptr_t>(element) >= reinterpret_cast<uintptr_t>(insertAt) &&
            reinterpret_cast<uintptr_t>(element) < reinterpret_cast<uintptr_t>(currentEnd)) {
            ++element;
        }
        if (currentEnd != nullptr) {
            void* last = currentEnd[-1];
            *currentEnd = last;
            if (last != nullptr) CallGameVirtual<void, 1>(last);
        }
        int32_t result = CallGameFunction<int32_t, 0x006B05F0>(insertAt, currentEnd - 1, currentEnd);
        void* newValue = *element;
        void* oldValue = *insertAt;
        if (newValue != oldValue) {
            if (newValue != nullptr) result = CallGameVirtual<int32_t, 1>(newValue);
            *insertAt = newValue;
            if (oldValue != nullptr) result = CallGameVirtual<int32_t, 2>(oldValue);
        }
        end = currentEnd + 1;
        return result;
    }

    // Growth path: EA re-creates the vector through the counting form of 0x4F22E0 with a
    // null base, so the copy runs at byte offsets (4*n) instead of an allocated buffer, and
    // the result stores begin = null. Kept bit for bit.
    void** oldBegin = begin;
    const int32_t oldCount = ShiftRight2(static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(currentEnd) - reinterpret_cast<uintptr_t>(oldBegin)));
    int32_t newCount = 2 * oldCount;
    if (oldCount == 0) newCount = 1;
    void** savedPos = pos;
    void* cursor = nullptr;
    CopyRefElements(&cursor, oldBegin, pos, nullptr);
    void* afterPrefix = cursor;
    if (afterPrefix != nullptr) {
        void* newValue = *value;
        *reinterpret_cast<void**>(afterPrefix) = newValue;
        if (newValue != nullptr) CallGameVirtual<void, 1>(newValue);
    }
    CopyRefElements(&cursor, savedPos, currentEnd, static_cast<char*>(afterPrefix) + 4);
    void* result = cursor;
    ReleaseRefElements(begin, currentEnd);
    begin = nullptr;
    end = static_cast<void**>(result);
    capacity = reinterpret_cast<void**>(static_cast<uintptr_t>(4) * static_cast<uintptr_t>(newCount));
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(result));
}

void WorldObjNameSource::BuildObjName(const wchar_t* path, const wchar_t* appendPart,
                                      const wchar_t* suffixPart, void* outString)
{
    if (path == nullptr || appendPart == nullptr || suffixPart == nullptr) return;

    auto* out = static_cast<WorldWideString*>(outString);
    const wchar_t* base = nullptr;
    const wchar_t* file = nullptr;
    const wchar_t* ext = nullptr;
    SplitPath(path, &base, &file, &ext);
    (void)base;

    WorldWideString suffix;
    WorldWideString head;
    WorldWideString formatted;
    const WorldWideString empty{reinterpret_cast<void*>(kEmptyWideStringBegin),
                                reinterpret_cast<void*>(kEmptyWideStringBegin),
                                reinterpret_cast<void*>(kEmptyWideStringCapacity),
                                reinterpret_cast<void*>(kWideStringAllocator)};
    suffix = empty;
    head = empty;
    // The original leaves this helper object uninitialized and the callee fills it in; the
    // empty string keeps the rewrite deterministic without changing the observable calls.
    formatted = empty;

    CallGameMethod<void, 0x00408E20>(out, path, file);
    CallGameMethod<void, 0x00408B90>(out, file, ext);
    CallGameFunction<void, 0x004EDD80>(out);
    CallGameMethod<void, 0x004F1F50>(out, appendPart);
    CallGameFunction<void, 0x004EDD80>(out);
    if (partBegin == partEnd) {
        CallGameMethod<void, 0x00408B90>(&suffix, file, ext);
    } else {
        CallGameMethod<void, 0x00408B90>(&suffix, partBegin, partEnd);
    }
    CallGameMethod<void, 0x004F1F50>(&head, reinterpret_cast<const wchar_t*>(uintptr_t{0x00FAA258}));
    CallGameFunction<void, 0x004AEFE0>(&head,
                                       reinterpret_cast<const wchar_t*>(uintptr_t{0x0101A800}),
                                       valueLow, valueHigh);
    CallGameMethod<void, 0x004F1F50>(&head, suffixPart);

    const int32_t headLength = ShiftRight1(static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(head.end) - reinterpret_cast<uintptr_t>(head.begin)));
    const int32_t width = 40 - headLength;
    void* limited = CallGameMethod<void*, 0x007304F0>(&suffix, &formatted, width);
    if (limited != static_cast<void*>(&suffix)) {
        auto** range = static_cast<void**>(limited);
        CallGameMethod<void, 0x00408E20>(&suffix, range[0], range[1]);
    }
    FreeGameStringIfHeap(formatted);
    CallGameMethod<void, 0x00408B90>(out, suffix.begin, suffix.end);
    CallGameMethod<void, 0x00408B90>(out, head.begin, head.end);
    FreeGameStringIfHeap(head);
    FreeGameStringIfHeap(suffix);
}

}  // namespace ts3::world