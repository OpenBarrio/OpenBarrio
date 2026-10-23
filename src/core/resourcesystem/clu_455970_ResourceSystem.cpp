#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
#include <direct.h>

namespace ts3::resourcesystem {
// 0x004DA940, thiscall, ret 4; actual function size is 132, not the brief's 9.
ShadowWatcher* ShadowWatcher::Initialize(void* suppliedAllocator)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void*>(uintptr_t{0x00FB82F8});
#else
    NotRewritten("ShadowWatcher original vtable");
#endif
    active = 0;
    allocator = suppliedAllocator ? suppliedAllocator : CallGameFunction<void*, 0x004D9AD0>();
    state0 = 0;
    mode = 4;
    for (auto& word : state) word = 0;
    flag = 0;
    path[0] = 0;
    return this;
}

// 0x004A4740, thiscall, ret 0.
DirectoryDefinition* DirectoryDefinition::Initialize()
{
    state = 0;
    watcher.Initialize(nullptr);
    sentinel.next = &sentinel;
    sentinel.previous = &sentinel;
    count = 0;
    flag = 0;
    for (auto& byte : padding) byte = 0;
    value = nullptr;
#if defined(_M_IX86)
    label = reinterpret_cast<const char*>(uintptr_t{0x00FB2328});
#else
    label = "ResourceSystem/ResourceMan/DDF/DDFS";
#endif
    path[0] = 0;
    return this;
}

// 0x0072EA70, thiscall, ret 8; success is defined in AL only.
bool DirectoryMappings::FindResourceMapping(void* query, uint32_t* key)
{
    void* manager = CallGameFunction<void*, 0x004AFD20>();
    if (!manager) return false;
    ResourceList list;
    list.allocator = CallGameFunction<void*, 0x00931020>();
    list.next = list.previous = reinterpret_cast<ResourceListNode*>(&list);
    list.count = 0;
#if defined(_M_IX86)
    list.label = reinterpret_cast<const char*>(uintptr_t{0x00FFDA88});
    void* mutex = reinterpret_cast<void*>(uintptr_t{0x011D3C98});
    const void* lockName = reinterpret_cast<const void*>(uintptr_t{0x00FFDA44});
#else
    list.label = "ResourceSystem/DirectoryManager/dbList";
    void* mutex = nullptr;
    const void* lockName = nullptr;
#endif
    if (!CallGameVirtual<int, 18>(manager, &list, query)) {
        CallGameMethod<void, 0x0054A290>(&list);
        return false;
    }
    CallGameMethod<void, 0x004E16F0>(mutex, lockName);
    bool found = false;
    for (auto node = list.next; node != reinterpret_cast<ResourceListNode*>(&list); node = node->next) {
        const auto path = CallGameVirtual<const char16_t*, 10>(node->resource);
        if (!path) continue;
        for (auto entry = begin; entry != end; ++entry) {
            PathBuffer resourcePath, mappingPath;
            CallGameMethod<PathBuffer*, 0x004EDA20>(&resourcePath, path);
            CallGameMethod<PathBuffer*, 0x004EDA20>(&mappingPath, entry->path);
            resourcePath.RemoveFilename();
            mappingPath.RemoveFilename();
            auto resourceData = CallGameMethod<char16_t*, 0x00F626F0>(&resourcePath);
            auto mappingData = CallGameMethod<char16_t*, 0x00F626F0>(&mappingPath);
            if (HasPathPrefix(mappingData, resourceData, 5)) {
                key[1] = entry->keyHigh;
                key[0] = entry->keyLow;
                found = true;
                break; // EA continues with later resources and may overwrite the key.
            }
        }
    }
    CallGameMethod<void, 0x004E17B0>(mutex);
    auto node = list.next;
    while (node != reinterpret_cast<ResourceListNode*>(&list)) {
        auto freed = node;
        node = node->next;
        CallGameVirtual<void, 3>(list.allocator, freed, static_cast<uint32_t>(sizeof(ResourceListNode)));
    }
    return found;
}

// 0x00737950, thiscall, ret 8. Allocation failures intentionally retain EA's null dereference.
void* ShadowDirectoryManager::OpenShadowDatabase(const char16_t* path, int flags)
{
    char16_t filename[260];
    CallGameFunction<uintptr_t, 0x004FF380>(filename, path, 260);
    RemoveTrailingSeparator(filename, 260); // EA passes capacity, not measured length.
    if (!CallGameFunction<bool, 0x004DBBA0>(filename)) return nullptr;
    if (auto existing = CallGameMethod<void*, 0x00734220>(this, path)) return existing;
#if defined(_M_IX86)
    const char* dbLabel = reinterpret_cast<const char*>(uintptr_t{0x00FFE4C0});
    const char* watcherLabel = reinterpret_cast<const char*>(uintptr_t{0x00FFE4A0});
    const void* lockLabel = reinterpret_cast<const void*>(uintptr_t{0x00FFDFC4});
    const void* callback = reinterpret_cast<const void*>(uintptr_t{0x007376F0});
#else
    const char* dbLabel = "ResourceSystem/ShadowedDBPF";
    const char* watcherLabel = "ResourceSystem/ShadowWatcher";
    const void* lockLabel = nullptr;
    const void* callback = nullptr;
#endif
    ShadowDatabase* database = nullptr;
    auto memory = CallGameFunction<ShadowDatabase*, 0x004E41F0>(static_cast<uint32_t>(sizeof(ShadowDatabase)), dbLabel, 0, 0, 0, 0);
    if (memory) {
        database = CallGameMethod<ShadowDatabase*, 0x007342F0>(memory);
        if (database) CallGameVirtual<void, 0>(&database->referenceVtable);
    }
    auto interface = &database->databaseVtable;
    CallGameMethod<void, 0x0072CA70>(interface, databaseContext);
    if (!CallGameVirtual<bool, 1>(interface)) {
        CallGameVirtual<void, 1>(&database->referenceVtable);
        return nullptr;
    }
    CallGameVirtual<void, 11>(interface, filename);
    if (!CallGameVirtual<bool, 13>(&databaseManagerVtable, 1, interface, flags)) {
        CallGameVirtual<void, 2>(interface);
        CallGameVirtual<void, 1>(&database->referenceVtable);
        return nullptr;
    }
    CallGameMethod<void, 0x004E16F0>(mutex, lockLabel);
    auto value = database;
    CallGameMethod<void, 0x005A4470>(collection, &value);
    auto watcher = watchers.begin;
    for (; watcher != watchers.end; ++watcher) {
        auto prefix = CallGameMethod<const char16_t*, 0x004DA9D0>(*watcher);
        if (HasPathPrefix(prefix, filename, 5)) break;
    }
    if (watcher == watchers.end) {
        auto allocated = CallGameFunction<ShadowWatcher*, 0x004E3FB0>(static_cast<uint32_t>(sizeof(ShadowWatcher)), 4, 0, watcherLabel, 0, 0, 0, 0);
        ShadowWatcher* created = allocated ? allocated->Initialize(nullptr) : nullptr;
        if (watchers.end < watchers.capacity) {
            auto destination = watchers.end++;
            if (destination) *destination = created;
        } else {
            CallGameMethod<void, 0x008621C0>(&watchers, watchers.end, &created);
        }
        char16_t *root, *file, *extension;
        ScanPathParts(filename, &root, &file, &extension);
        *file = 0;
        CallGameMethod<bool, 0x004DA9E0>(created, filename);
        CallGameMethod<void, 0x004DAA10>(created, callback, this);
        CallGameMethod<bool, 0x004DAA30>(created, 20);
        CallGameMethod<void, 0x004DB290>(created);
    }
    CallGameMethod<void, 0x004E17B0>(mutex);
    CallGameVirtual<void, 1>(&database->referenceVtable);
    return interface;
}

// 0x00455970, cdecl, full EAX result.
int IsPathSeparator(int character)
{
    return character == 92 || character == 47 || character == 58;
}

// 0x004ECCB0, cdecl, four stack arguments (the third is unused).
char16_t* MakeAbsolutePath(char16_t* output, const char16_t* path, uint32_t capacity, int mode)
{
    (void)capacity;
    char16_t base[522];
    base[0] = 0;
    if (static_cast<uint32_t>(mode) - 3 > 3) {
        CallGameFunction<uintptr_t, 0x004DBFB0>(base, 260);
    } else {
        int drive;
        if (path[0] && path[1] && IsPathSeparator(path[0]) && IsPathSeparator(path[1])) {
            base[0] = base[1] = u'\\'; base[2] = 0;
            return NormalizePath(output, path + 2, base, mode);
        }
        const auto letter = path[0] && path[1] == u':'
            ? CallGameFunction<uint16_t, 0x004EC3F0>(static_cast<int>(path[0])) : uint16_t{0};
        if (static_cast<uint32_t>(letter) - u'a' <= 25) {
            drive = letter - u'a';
            path += 2;
        } else {
            drive = _getdrive() - 1;
        }
        if (IsPathSeparator(*path) && drive >= 0) {
            base[0] = drive < 26 ? static_cast<char16_t>(drive + u'a') : u'c';
            base[1] = u':'; base[2] = u'\\'; base[3] = 0;
            return NormalizePath(output, path + 1, base, mode);
        }
        if (!_wgetdcwd(drive + 1, reinterpret_cast<wchar_t*>(base), 521))
            CallGameFunction<uintptr_t, 0x004DBFB0>(base, 260);
    }
    const auto length = CallGameFunction<int, 0x004FE0F0>(base);
    // EA assumes a nonempty current directory and inspects base[length - 1].
    if (!IsPathSeparator(base[length - 1])) {
        base[length] = u'\\'; base[length + 1] = 0;
    }
    return NormalizePath(output, path, base, mode);
}

// 0x004ECE50, cdecl. Overflow preserves the written prefix and forces word 259 to zero.
char16_t* NormalizePath(char16_t* output, const char16_t* path, const char16_t* base, int mode)
{
    if (!path) return nullptr;
    if (!base) return MakeAbsolutePath(output, path, 260, mode);
    char16_t* out = output;
    char16_t* const limit = output + 260;
    const char16_t* begin = base;
    const char16_t* end = base + CallGameFunction<int, 0x004FE0F0>(base);
    char16_t *root, *filename, *extension;
    if (begin == end) {
        begin = path;
        ScanPathParts(const_cast<char16_t*>(path), &root, &filename, &extension);
        end = root;
        if (IsPathSeparator(*end)) ++end;
        path = end;
    }
    if (begin != end) {
        do {
            char16_t c = *begin++;
            if (c == u'/') c = u'\\';
            if (out == limit) goto overflow;
            *out++ = c;
        } while (begin != end);
        if (!IsPathSeparator(out[-1])) {
            if (out == limit) goto overflow;
            *out++ = u'\\';
        }
    }
    if (out == limit) goto overflow;
    *out = 0;
    ScanPathParts(output, &root, &filename, &extension);
    {
        const bool relativeRoot = root == output;
        bool unresolvedParent = relativeRoot;
        if (IsPathSeparator(*root)) ++root;
        bool separator = true;
        for (;;) {
            char16_t c = *path++;
            if (IsPathSeparator(c)) {
                if (separator) continue;
                separator = true;
                if (c == u'/') c = u'\\';
            } else {
                if (c != u'.') {
                    unresolvedParent = false;
                } else if (separator) {
                    const char16_t next = *path;
                    if (!next || IsPathSeparator(next)) {
                        if (out > output && IsPathSeparator(out[-1])) continue;
                    } else if (next == u'.' && (!path[1] || IsPathSeparator(path[1])) && !unresolvedParent) {
                        int separators = 0;
                        char16_t* i = out;
                        for (; i > root; --i) {
                            if (IsPathSeparator(i[-1]) && ++separators > 1) break;
                        }
                        // Preserve an existing unresolved parent component, including stale-buffer reads.
                        if (!(i[0] == u'.' && i[1] == u'.' && (!i[2] || IsPathSeparator(i[2])))) {
                            out = i;
                            if (i == root) unresolvedParent = relativeRoot;
                            ++path;
                            continue;
                        }
                    }
                }
                separator = false;
            }
            if (out == limit) goto overflow;
            *out++ = c;
            if (!c) return output;
        }
    }
overflow:
    output[259] = 0;
    return nullptr;
}

// 0x004EC690, cdecl, only AL is defined.
bool RemoveTrailingSeparator(char16_t* path, int length)
{
    if (length == -1)
        length = CallGameFunction<int, 0x004FE0F0>(path);
    if (!length || !IsPathSeparator(path[length - 1]))
        return false;
    path[length - 1] = 0;
    return true;
}

// 0x004ECAE0, cdecl. No allocation; output is a flat array of 260-word slots.
uint32_t SplitPathReverse(const char16_t* path, char16_t* slots, uint32_t capacity)
{
    const uint32_t length = CallGameFunction<uint32_t, 0x004FE0F0>(path);
    if (!length) return 0;
    uint32_t end = length - 1; // EA drops the last character even without a trailing separator.
    int cursor = static_cast<int>(end);
    if (IsPathSeparator(path[cursor])) --cursor;
    if (!slots) capacity = UINT32_MAX;
    uint32_t count = 0;
    for (; cursor >= 0 && capacity > count + 1; --cursor) {
        uint32_t size;
        const char16_t* begin;
        if (IsPathSeparator(path[cursor])) {
            size = end - static_cast<uint32_t>(cursor);
            begin = path + cursor + 1;
            end = static_cast<uint32_t>(cursor);
        } else if (!cursor) {
            size = end;
            begin = path;
            end = 0;
        } else {
            continue;
        }
        char16_t* out = slots ? slots + 260 * count : nullptr;
        // Copy occurs BEFORE rejecting an oversized component (observable partial output).
        if (out) CallGameFunction<char16_t*, 0x004FDD80>(out, begin, size);
        if (size - 1 > 254) return 0;
        if (out) {
            if (IsPathSeparator(out[size - 1])) out[size] = 0;
            else { out[size] = 92; out[size + 1] = 0; }
        }
        ++count;
    }
    if (!end || capacity <= count) return count;
    if (end >= 256) return 0;
    if (slots) {
        char16_t* out = slots + 260 * count;
        CallGameFunction<char16_t*, 0x004FDD80>(out, path, end);
        if (IsPathSeparator(out[end - 1])) out[end] = 0;
        else { out[end] = 92; out[end + 1] = 0; }
    }
    return count + 1;
}

static char16_t FoldByte(char16_t c)
{
    if (c >= 256) return c;
#if defined(_M_IX86)
    return reinterpret_cast<const uint8_t*>(uintptr_t{0x0114CB30})[c];
#else
    NotRewritten("ResourceSystem game byte folding table");
#endif
}

// 0x004ED480, cdecl, AL only. Empty prefix succeeds even for a null path.
bool HasPathPrefix(const char16_t* prefix, const char16_t* path, int mode)
{
    if (!*prefix) return true;
    const bool exact = mode != 2 && mode != 3 && mode != 5 && mode != 6;
    do {
        if (!*path) return false;
        if (IsPathSeparator(*prefix)) {
            if (!IsPathSeparator(*path)) return false;
        } else if (exact) {
            if (*prefix != *path) return false;
        } else if (FoldByte(*prefix) != FoldByte(*path)) {
            return false;
        }
        ++prefix;
        ++path;
    } while (*prefix);
    return IsPathSeparator(prefix[-1]) || !*path || IsPathSeparator(*path);
}

// 0x004ED980, cdecl. Local temporaries preserve scanner output even when outputs alias.
char16_t* ScanPathParts(char16_t* path, char16_t** root, char16_t** filename, char16_t** extension)
{
    char16_t *r, *f, *e;
    CallGameFunction<uintptr_t, 0x004EC8D0>(path, &r, &f, &e);
    *root = r;
    *filename = f;
    *extension = e;
    return e;
}

// 0x004EDA50, thiscall, ret 0. No normalization or extra null checks.
uintptr_t PathBuffer::RemoveFilename()
{
    char16_t *r, *f, *e;
    const uintptr_t result = CallGameFunction<uintptr_t, 0x004EC8D0>(data, &r, &f, &e);
    *f = 0;
    return result;
}
} // namespace ts3::resourcesystem
