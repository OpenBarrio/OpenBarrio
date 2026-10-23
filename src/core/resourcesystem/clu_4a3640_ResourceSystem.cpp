#include "ts3/resourcesystem/clu_4a3640_ResourceSystem.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
#include "ts3/cas/clu_4a6710_CAS.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include "ts3/io/file_stream.h"
#include "ts3/services/cls_4a4ba0_Services.h"
#include <algorithm>
#include <cstring>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <direct.h>
#endif
namespace ts3::resourcesystem {
namespace {
uint32_t Length(const U16* s) { const U16* p = s; while (*p) ++p; return static_cast<uint32_t>(p - s); }
bool Slash(U16 c) { return c == u'\\' || c == u'/'; }
void Copy(U16* out, const U16* in) { do { *out++ = *in; } while (*in++); }
}
// 0x004DBF60
bool IsDirectory(const U16* path)
{
    if (!path || !*path) return false;
#if defined(_WIN32)
    const auto attributes = GetFileAttributesW(reinterpret_cast<const wchar_t*>(path));
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    AbortNotRewritten("Win32 GetFileAttributesW"); return false;
#endif
}
// 0x004DFF70
bool AppendDirectorySlash(U16* path, uint32_t capacity)
{
    const uint32_t n = Length(path);
    if (n && Slash(path[n - 1])) return false;
    if (n + 2 > capacity) return false;
    path[n] = u'\\'; path[n + 1] = 0;
    return true;
}
// 0x004EC640
bool AppendPathSlash(U16* path, int32_t length)
{
    const int32_t n = length == -1 ? static_cast<int32_t>(Length(path)) : length;
    if (n && (Slash(path[n - 1]) || path[n - 1] == u':')) return false;
    path[n] = u'\\'; path[n + 1] = 0;
    return true;
}
// 0x004DBFB0
uint32_t CurrentDirectory(U16* out, int32_t capacity)
{
#if defined(_WIN32)
    if (_wgetcwd(reinterpret_cast<wchar_t*>(out), 255)) {
        AppendDirectorySlash(out, static_cast<uint32_t>(capacity));
        return Length(out);
    }
#else
    (void)capacity; AbortNotRewritten("Win32 _wgetcwd");
#endif
    *out = 0; return 0;
}
// 0x004D9AF0
FindCursor* FindNext(FindCursor* cursor)
{
    if (!cursor) return nullptr;
#if defined(_WIN32)
    WIN32_FIND_DATAW data;
    if (!FindNextFileW(cursor->handle, &data)) return nullptr;
    CallGameFunction<void, 0x004DF4F0>(cursor->name, reinterpret_cast<const U16*>(data.cFileName), 260);
    cursor->directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    if (cursor->directory) AppendDirectorySlash(cursor->name, 260);
    return cursor;
#else
    AbortNotRewritten("Win32 FindNextFileW"); return nullptr;
#endif
}
// 0x004EC6D0
int32_t JoinPath(U16* out, const U16* base, const U16* suffix)
{
    U16* p = out;
    if (base) {
        const bool nonempty = *base != 0;
        do { *p++ = *base; } while (*base++);
        --p;
        if (nonempty && (p == out || (!Slash(p[-1]) && p[-1] != u':'))) *p++ = u'\\';
    }
    if (suffix) { do { *p++ = *suffix; } while (*suffix++); --p; }
    else *p = 0;
    return static_cast<int32_t>(p - out);
}
// 0x004FE2E0
const U16* FindSubstring(const U16* text, const U16* needle)
{
    if (!*needle) return text;
    for (; *text; ++text) {
        if (*text != *needle) continue;
        const U16* s = text; const U16* n = needle;
        while (*s == *n && *s) { ++s; ++n; }
        if (!*n) return text;
    }
    return nullptr;
}
// 0x004ED1A0
bool IsAbsolute(const U16* path, int32_t kind)
{
    if (static_cast<uint32_t>(kind) - 3u <= 3u || kind == 10) {
        // Exact alphabetic ranges of 0x0114CA30 & 0xC0 (not locale-dependent).
        const uint32_t c = *path;
        const bool alpha = (c >= 65 && c <= 90) || (c >= 97 && c <= 122) ||
                           (c >= 192 && c <= 246) || (c >= 248 && c <= 255);
        if (!(alpha && FindSubstring(path, u":\\")) && !(path[0] == u'\\' && path[1] == u'\\')) return false;
        if (FindSubstring(path, u"/.")) return false;
        return !FindSubstring(path, u"\\.");
    }
    if (*path != u'\\') return false;
    return !FindSubstring(path, u"/.");
}
// 0x004ED2A0
U16* ResolvePath(U16* out, const U16* path, const U16* base, int32_t kind)
{
    if (!CallGameFunction<uint8_t, 0x004ED110>(path, 5))
        return NormalizePath(out, path, base, kind);
    if (CallGameFunction<uint32_t, 0x004FE0F0>(path) >= 259) return nullptr;
    CallGameFunction<void, 0x004FDD00>(out, path);
    return out;
}
// 0x007CCFC0
uint8_t MakeAbsolute(PathString* path, const U16* base)
{
    if (path->begin == path->end) return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(path->begin));
    if (IsAbsolute(path->begin, 5)) return 1;
    U16 buffer[260];
    if (!ResolvePath(buffer, path->begin, base, 5)) return 0;
    return CallGameMethod<uint8_t, 0x00409010>(path, buffer);
}
// 0x004AA5E0
TreeNode** Tree::Erase(TreeNode** out, TreeNode* node)
{
    --count;
    TreeNode* next = CallGameFunction<TreeNode*, 0x004E0B20>(node);
    CallGameFunction<void, 0x004E0E20>(node, &anchor);
    scratch::Free4E3F70(node);
    *out = next;
    return out;
}
// 0x004A4A70
InsertResult* Tree::Insert(InsertResult* out, const PathString* path, uint32_t ignored)
{
    (void)ignored;
    TreeNode* parent = &anchor;
    TreeNode* node = anchor.parent;
    bool less = true;
    while (node) {
        const auto& key = static_cast<PathNode*>(node)->path;
        const auto n = path->end - path->begin;
        const auto m = key.end - key.begin;
        int comparison = 0;
        for (ptrdiff_t i = 0; i < (std::min)(n, m); ++i) {
            if (path->begin[i] != key.begin[i]) { comparison = path->begin[i] < key.begin[i] ? -1 : 1; break; }
        }
        if (!comparison) comparison = n < m ? -1 : n > m;
        less = comparison < 0;
        parent = node;
        node = less ? node->left : node->right;
    }
    TreeNode* candidate = parent;
    bool insert = false;
    if (less) {
        if (parent == anchor.left) insert = true;
        else candidate = CallGameFunction<TreeNode*, 0x004E0B60>(parent);
    }
    if (!insert) {
        const auto& key = static_cast<PathNode*>(candidate)->path;
        insert = CallGameFunction<int32_t, 0x00446B50>(key.begin, key.end, path->begin, path->end) < 0;
    }
    if (insert) {
        TreeNode* result;
        CallGameMethod<void, 0x004A47D0>(this, &result, parent, path, 0);
        out->node = result; out->inserted = 1;
    } else { out->node = candidate; out->inserted = 0; }
    return out;
}
// 0x007EDC10
InsertResult* Tree::InsertAdapter(InsertResult* out, const PathString* path) { Insert(out, path, 0); return out; }
// 0x004A4930
int32_t Tracker::Track(void* resource, uint32_t add)
{
    const uint32_t before = resources.count;
    if (resource) {
        TreeNode* found[2];
        CallGameMethod<void, 0x0061BA60>(&resources, found, &resource);
        if (static_cast<uint8_t>(add)) {
            if (found[0] == &resources.anchor) {
                add &= 0xFFFFFF00u; // the original clears only AL's argument byte
                CallGameMethod<void, 0x006A1BE0>(&resources, found, &resource, add);
            }
        } else if (found[0] != &resources.anchor) { TreeNode* next; resources.Erase(&next, found[0]); }
    }
    return before != resources.count;
}
// 0x004A43C0
int32_t DirectoryScan::Open(const U16* input)
{
    if (!*path && input && IsDirectory(input)) {
        Copy(path, input); // EA has no capacity check
        if (!manager) manager = CallGameFunction<void*, 0x004AFD20>();
        CallGameMethod<void, 0x004AFD80>(stream);
#if defined(_M_IX86)
        const auto callback = reinterpret_cast<void*>(uintptr_t(0x004A4210));
#else
        void* callback = nullptr;
#endif
        CallGameMethod<void, 0x004DAA10>(stream, callback, this);
        CallGameMethod<void, 0x004DA9E0>(stream, input);
        CallGameMethod<void, 0x004DAA30>(stream, 5);
        CallGameMethod<void, 0x004DAA50>(stream, 5);
        CallGameMethod<void, 0x004DB290>(stream);
    }
    return *path != 0;
}
// 0x004ACD70: do not duplicate the already verified EASTL rewrite.
bool FreeSpaceAdapter::MarkUsed(int32_t offset, int32_t size)
{ return reinterpret_cast<resource::FreeSpaceMap*>(this)->MarkUsed(offset, size); }
// 0x00CA9900
bool RemoveFlags(Tree* tree, const uint8_t* key, int32_t delta, uint8_t eraseAll)
{
    uint8_t packed[4] = {key[0], key[1], static_cast<uint8_t>(static_cast<uint32_t>(key[2]) + static_cast<uint32_t>(delta)), 0};
    TreeNode* node;
    CallGameMethod<void, 0x00D20E90>(tree, &node, packed);
    if (node == &tree->anchor) return false;
    if (eraseAll) { TreeNode* next; tree->Erase(&next, node); return true; }
    // Packed key payload follows the pointer-sized node header.
    auto* flags = reinterpret_cast<uint8_t*>(node + 1) + 3;
    if (!(*flags & key[3])) return false;
    *flags &= static_cast<uint8_t>(~key[3]);
    if (!*flags) { TreeNode* next; tree->Erase(&next, node); }
    return true;
}
// 0x004A3640
int32_t Config::Load(const U16* path, const U16* source)
{
    error = 0;
    U16 cwd[260], resolved[260];
    CurrentDirectory(cwd, 260);
    if (!ResolvePath(resolved, path, cwd, 5)) {
        error = static_cast<int32_t>(0xD06B0001u);
        if (errorCallback) errorCallback(error, resolved, 0, 0, errorContext);
        return error;
    }
    // EA retains this stack address in the queue after returning; preserve that bug.
    U16* value = resolved;
    if (pathsEnd < pathsCapacity) { auto slot = pathsEnd++; if (slot) *slot = resolved; }
    else CallGameMethod<void, 0x00B7B1E0>(&pathsBegin, pathsEnd, &value);
    struct Entries { uint8_t* begin; uint8_t* end; uint8_t* capacity; void* allocator; uint32_t flags; const char* name; };
    auto release = [](Entries& entries) {
        if (entries.begin) {
            const auto bytes = entries.capacity - entries.begin;
            CallGameVirtual<void, 3>(entries.allocator, entries.begin, static_cast<int32_t>(20 * (bytes / 20)));
        }
    };
    while (nextPath < static_cast<uint32_t>(pathsEnd - pathsBegin)) {
        cas::CopyUtf16(reinterpret_cast<uint16_t*>(directory), reinterpret_cast<const uint16_t*>(pathsBegin[nextPath++]), 260);
        AppendPathSlash(directory, -1);
        directoryLength = Length(directory);
        Entries entries{nullptr, nullptr, nullptr, allocator, 0, "ResourceSystem/ResourceConfig/Empty"};
        uint8_t flag;
        if (source && !CallGameMethod<uint8_t, 0x004A1BD0>(this, source, &entries, &flag)) { release(entries); return -1; }
        // The original snapshots begin/capacity/allocator before the consuming call.
        Entries owned = entries;
        CallGameMethod<void, 0x004A2A70>(this, &entries);
        release(owned);
    }
    return error;
}
namespace {
void InitTree(Tree& tree)
{
    tree.comparator = 0;
    tree.anchor = {&tree.anchor, &tree.anchor, nullptr, 0};
    tree.count = 0; tree.name = "ResourceSystem/ResourceMan/PathSet";
}
void ReleaseString(PathString& s)
{
    // EA's single-character static empty buffer must not be freed.
    if (s.begin && reinterpret_cast<uintptr_t>(s.capacity) - reinterpret_cast<uintptr_t>(s.begin) > 2)
        scratch::Free4E3F70(s.begin);
}
void CopyString(PathString& dest, const PathString& source)
{
    const auto n = (reinterpret_cast<uintptr_t>(source.end) - reinterpret_cast<uintptr_t>(source.begin)) / sizeof(U16);
    dest = {nullptr, nullptr, nullptr, source.name};
    CallGameMethod<void, 0x00408830>(&dest, static_cast<int32_t>(n + 1));
    if (n) std::memmove(dest.begin, source.begin, n * sizeof(U16));
    dest.end = dest.begin + n; *dest.end = 0;
}
}
// 0x004A56B0. Stack string/tree lifetimes follow the assembly, not the inaccurate
// esp-framed decompile (in particular, wcslwr's input is not null).
int32_t MountDirectories(const U16* path, void* manager, int32_t flags, PathFilter filter, void* context, int32_t priority)
{
    if (!manager) { manager = CallGameFunction<void*, 0x004AFD20>(); if (!manager) return 0; }
    if (!IsDirectory(path)) return 0;
    Tree seen, paths; InitTree(seen); InitTree(paths);
    U16 source[260], filename[260];
    JoinPath(source, path, u"DDFList.txt");
    io::FileStream stream;
    stream.Construct(reinterpret_cast<const wchar_t*>(source));
    int32_t count = 0;
    auto add = [&]() {
        PathString lower{nullptr, nullptr, nullptr, "ResourceSystem/ResourceMan/DDF"};
        CallGameMethod<void, 0x0040B700>(&lower, source);
#if defined(_WIN32)
        _wcslwr(reinterpret_cast<wchar_t*>(lower.begin));
#else
        AbortNotRewritten("game CRT wcslwr");
#endif
        PathString lookup;
        CopyString(lookup, lower);
        TreeNode* found;
        CallGameMethod<void, 0x004EFDC0>(&seen, &found, &lookup);
        const bool accept = found == &seen.anchor && (!filter || filter(source, context));
        ReleaseString(lookup);
        if (accept) {
            PathString key; CopyString(key, lower);
            InsertResult result;
            seen.Insert(&result, &key, 0); ReleaseString(key);
            PathString original{nullptr, nullptr, nullptr, "ResourceSystem/ResourceMan/DDF"};
            CallGameMethod<void, 0x0040B700>(&original, source);
            PathString copy; CopyString(copy, original);
            paths.Insert(&result, &copy, 0); ReleaseString(copy); ReleaseString(original);
            ++count;
        }
        ReleaseString(lower);
    };
    if (stream.Open(1, 6, 1, 0)) {
        char line[260];
        for (uint32_t n = CallGameFunction<uint32_t, 0x004DD970>(&stream, line, 260); n < 0xFFFFFFFEu;
             n = CallGameFunction<uint32_t, 0x004DD970>(&stream, line, 260)) {
            if (!n || line[0] == ';' || (line[0] == '/' && line[1] == '/')) continue;
            uint32_t i = 0;
            for (; i < 259 && line[i]; ++i) filename[i] = static_cast<uint8_t>(line[i]);
            filename[i] = 0;
            ResolvePath(source, filename, path, 5); AppendPathSlash(source, -1); add();
        }
    }
    struct List {
        List* next; List* previous; void* allocator; uint32_t count; const char* name;
    };
    List entries;
    entries.next = entries.previous = &entries;
    entries.allocator = CallGameFunction<void*, 0x004D9AD0>();
    entries.count = 0; entries.name = "ResourceSystem/ResourceMan/EntryList";
    void* enumerationContext[4] = {};
    CallGameMethod<void, 0x004DA3C0>(enumerationContext, path, &entries, nullptr, 1, 0x100000);
    // Nodes have next/prev/allocator followed by the UTF-16 name pointer.
    struct Entry { Entry* next; Entry* previous; void* allocator; U16* name; };
    for (auto* entry = reinterpret_cast<Entry*>(entries.next); entry != reinterpret_cast<Entry*>(&entries); entry = entry->next) {
        if (*entry->name == u'.') continue;
        ResolvePath(source, entry->name, path, 5); AppendPathSlash(source, -1);
        JoinPath(filename, source, u"DDFMap.txt");
        if (CallGameFunction<uint8_t, 0x004DBBA0>(filename)) add();
    }
#if defined(_M_IX86)
    auto* tracker = *reinterpret_cast<Tracker**>(uintptr_t(0x011C78D0));
#else
    Tracker* tracker = nullptr;
#endif
    for (TreeNode* node = paths.anchor.left; node != &paths.anchor;
         node = CallGameFunction<TreeNode*, 0x004E0B20>(node)) {
        auto* directory = static_cast<PathNode*>(node)->path.begin;
        if (!IsDirectory(directory)) continue;
        void* memory = CallGameFunction<void*, 0x004E41F0>(176, "ResourceSystem/ResourceMan/DDF", 0, 0, 0, 0);
        if (!memory) continue;
        void* mount = static_cast<services::ResourceMan*>(memory)->Initialize(reinterpret_cast<const wchar_t*>(directory), nullptr);
        if (!mount) continue;
        CallGameVirtual<void, 1>(mount);
        CallGameVirtual<void, 6>(mount, flags, 6, 0);
        CallGameVirtual<void, 13>(manager, 1, mount, priority);
        if (tracker) tracker->Track(mount, 1);
    }
    CallGameMethod<void, 0x00456CC0>(&entries);
    stream.Destruct();
    CallGameMethod<void, 0x005A0A30>(&seen, seen.anchor.parent);
    CallGameMethod<void, 0x005A0A30>(&paths, paths.anchor.parent);
    return count;
}
}
