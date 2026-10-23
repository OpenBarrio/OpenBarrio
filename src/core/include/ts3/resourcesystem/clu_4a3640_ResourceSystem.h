#pragma once
#include <cstdint>
#include "ts3/layout.h"
namespace ts3::resource { struct FreeSpaceMap; }
namespace ts3::resourcesystem {
using U16 = char16_t;
struct PathString { U16* begin; U16* end; U16* capacity; const char* name; };
struct TreeNode { TreeNode* right; TreeNode* left; TreeNode* parent; uint32_t color; };
struct PathNode : TreeNode { PathString path; };
struct InsertResult { TreeNode* node; uint8_t inserted; };
struct Tree {
    uint32_t comparator;
    TreeNode anchor;
    uint32_t count;
    const char* name;
    // TS3W.exe 0x004AA5E0: erases a node and returns the successor iterator through out.
    TreeNode** Erase(TreeNode** out, TreeNode* node);
    // TS3W.exe 0x004A4A70: inserts a UTF-16 path if absent; the last argument is ignored.
    InsertResult* Insert(InsertResult* out, const PathString* path, uint32_t ignored);
    // TS3W.exe 0x007EDC10: path-set insertion adapter.
    InsertResult* InsertAdapter(InsertResult* out, const PathString* path);
};
struct DirectoryScan {
    U16 path[256];
    void* manager;
    alignas(void*) uint8_t stream[552]; // opaque, owned by the unreconstructed stream dependency
    // TS3W.exe 0x004A43C0: initializes the directory scanner once.
    int32_t Open(const U16* path);
};
struct FindCursor {
    U16 name[260];
    uint8_t directory;
    uint8_t unknown209[1043];
    void* handle;
};
struct Tracker {
    uint8_t unknown000[1096];
    Tree resources;
    // TS3W.exe 0x004A4930: adds/removes a resource and reports a change in tree size.
    int32_t Track(void* resource, uint32_t add);
};
using ErrorCallback = void (*)(int32_t, const U16*, uint32_t, uint32_t, void*);
struct Config {
    void* allocator;
    uint8_t unknown004[40];
    U16 directory[260];
    U16** pathsBegin; U16** pathsEnd; U16** pathsCapacity;
    uint8_t unknown240[136];
    uint32_t nextPath; uint32_t directoryLength;
    uint8_t unknown2d0[20];
    ErrorCallback errorCallback; void* errorContext; int32_t error;
    // TS3W.exe 0x004A3640: resolves a base path and processes the pending configuration paths.
    int32_t Load(const U16* path, const U16* source);
};
TS3_X86_SIZE(PathString, 16);
TS3_X86_SIZE(TreeNode, 16);
TS3_X86_OFFSET(PathNode, path, 16);
TS3_X86_OFFSET(Tree, anchor, 4);
TS3_X86_OFFSET(Tree, count, 20);
TS3_X86_OFFSET(DirectoryScan, manager, 512);
TS3_X86_OFFSET(DirectoryScan, stream, 516);
TS3_X86_OFFSET(FindCursor, directory, 520);
TS3_X86_OFFSET(FindCursor, handle, 1564);
TS3_X86_OFFSET(Tracker, resources, 1096);
TS3_X86_OFFSET(Config, pathsBegin, 564);
TS3_X86_OFFSET(Config, nextPath, 712);
TS3_X86_OFFSET(Config, error, 748);
// TS3W.exe 0x004DBF60: returns whether a nonempty path names a directory.
bool IsDirectory(const U16* path);
// TS3W.exe 0x004D9AF0: advances a Win32 find cursor, adding a slash to directory names.
FindCursor* FindNext(FindCursor* cursor);
// TS3W.exe 0x004DBFB0: obtains the current directory with the original fixed 255-character limit.
uint32_t CurrentDirectory(U16* out, int32_t capacity);
// TS3W.exe 0x004DFF70: appends a backslash when missing and when capacity permits (colon is not a slash).
bool AppendDirectorySlash(U16* path, uint32_t capacity);
// TS3W.exe 0x004EC640: appends a backslash unless the path ends in slash, backslash or colon.
bool AppendPathSlash(U16* path, int32_t length);
// TS3W.exe 0x004EC6D0: concatenates nullable path components with a separator, without a capacity check.
int32_t JoinPath(U16* out, const U16* base, const U16* suffix);
// TS3W.exe 0x004FE2E0: finds the first UTF-16 substring, including an empty needle.
const U16* FindSubstring(const U16* text, const U16* needle);
// TS3W.exe 0x004ED1A0: checks absolute-path syntax, retaining the game's embedded-colon and dot-prefix quirks.
bool IsAbsolute(const U16* path, int32_t kind);
// TS3W.exe 0x004ED2A0: resolves a relative path; detection always uses kind 5, not the caller's kind.
U16* ResolvePath(U16* out, const U16* path, const U16* base, int32_t kind);
// TS3W.exe 0x007CCFC0: resolves a string in place; an empty string returns the pointer's low byte.
uint8_t MakeAbsolute(PathString* path, const U16* base);
// TS3W.exe 0x004ACD70: delegates to the already verified free-space-map implementation.
struct FreeSpaceAdapter { bool MarkUsed(int32_t offset, int32_t size); };
// TS3W.exe 0x00CA9900: removes a flag from the packed three-byte key, optionally erasing the entire node.
bool RemoveFlags(Tree* tree, const uint8_t* key, int32_t delta, uint8_t eraseAll);
using PathFilter = uint8_t (*)(U16*, void*);
// TS3W.exe 0x004A56B0: enumerates DDFList and DDFMap directories, filters them and mounts each unique path.
int32_t MountDirectories(const U16* path, void* manager, int32_t flags, PathFilter filter, void* context, int32_t priority);
// ABI-neutral adapters for hookgen's untyped buffer arguments. Read-only UTF-16
// inputs stay live (ptr0 metadata means untracked pointer, not a short byte snapshot).
inline bool HookIsDirectory(void* p) { return IsDirectory(static_cast<const U16*>(p)); }
inline void* HookFindSubstring(void* text, void* needle) { return const_cast<U16*>(FindSubstring(static_cast<const U16*>(text), static_cast<const U16*>(needle))); }
inline bool HookIsAbsolute(void* p, int32_t kind) { return IsAbsolute(static_cast<const U16*>(p), kind); }
}
