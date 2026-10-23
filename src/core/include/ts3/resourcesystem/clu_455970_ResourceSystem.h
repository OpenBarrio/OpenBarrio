#pragma once
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::resourcesystem {
struct ShadowWatcher {
    void* vtable;
    uint8_t active;
    uint8_t reserved[3];
    void* allocator;
    char16_t path[256];
    uintptr_t state0;
    uint32_t mode;
    uintptr_t state[11];
    uint8_t flag;
    uint8_t tail[3];
    // TS3W.exe 0x004DA940: initialize selected fields, preserving untouched bytes.
    ShadowWatcher* Initialize(void* suppliedAllocator);
};
TS3_X86_OFFSET(ShadowWatcher, allocator, 8);
TS3_X86_OFFSET(ShadowWatcher, path, 12);
TS3_X86_OFFSET(ShadowWatcher, state0, 524);
TS3_X86_OFFSET(ShadowWatcher, mode, 528);
TS3_X86_OFFSET(ShadowWatcher, state, 532);
TS3_X86_OFFSET(ShadowWatcher, flag, 576);
TS3_X86_SIZE(ShadowWatcher, 580);
struct DirectoryDefinition {
    char16_t path[256];
    uint32_t state;
    ShadowWatcher watcher;
    uint32_t reserved;
    struct Link { Link* next; Link* previous; } sentinel;
    uint32_t count;
    uint8_t flag;
    uint8_t padding[3];
    void* value;
    const char* label;
    // TS3W.exe 0x004A4740: initialize the watcher and embedded list sentinel.
    DirectoryDefinition* Initialize();
};
TS3_X86_OFFSET(DirectoryDefinition, state, 512);
TS3_X86_OFFSET(DirectoryDefinition, watcher, 516);
TS3_X86_OFFSET(DirectoryDefinition, sentinel, 1100);
TS3_X86_OFFSET(DirectoryDefinition, count, 1108);
TS3_X86_OFFSET(DirectoryDefinition, flag, 1112);
TS3_X86_OFFSET(DirectoryDefinition, value, 1116);
TS3_X86_OFFSET(DirectoryDefinition, label, 1120);
TS3_X86_SIZE(DirectoryDefinition, 1124);
struct DirectoryMapping {
    uint32_t keyLow, keyHigh;
    const char16_t* path;
    uintptr_t opaque[7];
};
TS3_X86_OFFSET(DirectoryMapping, path, 8);
TS3_X86_SIZE(DirectoryMapping, 40);
struct ResourceListNode {
    ResourceListNode* next;
    ResourceListNode* previous;
    void* resource;
};
struct ResourceList {
    ResourceListNode* next;
    ResourceListNode* previous;
    void* allocator;
    uint32_t count;
    const char* label;
};
TS3_X86_SIZE(ResourceListNode, 12);
TS3_X86_SIZE(ResourceList, 20);
struct DirectoryMappings {
    void* vtable;
    DirectoryMapping* begin;
    DirectoryMapping* end;
    // TS3W.exe 0x0072EA70: find each resource's first directory mapping; the last matching resource wins.
    bool FindResourceMapping(void* query, uint32_t* key);
};
TS3_X86_OFFSET(DirectoryMappings, begin, 4);
TS3_X86_OFFSET(DirectoryMappings, end, 8);
struct ShadowDatabase {
    uintptr_t opaqueHeader[2];
    void* databaseVtable;
    void* referenceVtable;
    uintptr_t opaqueBody[62];
};
TS3_X86_OFFSET(ShadowDatabase, databaseVtable, 8);
TS3_X86_OFFSET(ShadowDatabase, referenceVtable, 12);
TS3_X86_SIZE(ShadowDatabase, 264);
struct ShadowDirectoryManager {
    void* vtable;
    uintptr_t opaqueHeader;
    void* databaseManagerVtable;
    uintptr_t opaqueBeforeWatchers[122];
    struct Watchers { ShadowWatcher** begin; ShadowWatcher** end; ShadowWatcher** capacity; } watchers;
    uintptr_t opaqueBeforeMutex[6];
    uintptr_t mutex[6];
    uintptr_t opaqueBeforeCollection[34];
    uintptr_t collection[336];
    void* databaseContext;
    // TS3W.exe 0x00737950: open/register a shadow database and ensure a directory watcher exists.
    void* OpenShadowDatabase(const char16_t* path, int flags);
};
TS3_X86_OFFSET(ShadowDirectoryManager, databaseManagerVtable, 8);
TS3_X86_OFFSET(ShadowDirectoryManager, watchers, 500);
TS3_X86_OFFSET(ShadowDirectoryManager, mutex, 536);
TS3_X86_OFFSET(ShadowDirectoryManager, collection, 696);
TS3_X86_OFFSET(ShadowDirectoryManager, databaseContext, 2040);
// TS3W.exe 0x00455970: classify a full-width integer as a path separator.
int IsPathSeparator(int character);
// TS3W.exe 0x004ECCB0: resolve a base directory; capacity is ignored by EA.
char16_t* MakeAbsolutePath(char16_t* output, const char16_t* path, uint32_t capacity, int mode);
// TS3W.exe 0x004ECE50: normalize separators and dot components into a fixed 260-word buffer.
char16_t* NormalizePath(char16_t* output, const char16_t* path, const char16_t* base, int mode);
// TS3W.exe 0x004EC690: remove one trailing UTF-16 separator; -1 means measure the string.
bool RemoveTrailingSeparator(char16_t* path, int length);
// TS3W.exe 0x004ECAE0: emit reversed path components into 260-code-unit slots; preserves EA's length-minus-one quirk.
uint32_t SplitPathReverse(const char16_t* path, char16_t* slots, uint32_t capacity);
// TS3W.exe 0x004ED480: compare a path prefix on component boundaries, using the game's byte folding table.
bool HasPathPrefix(const char16_t* prefix, const char16_t* path, int mode);
// TS3W.exe 0x004ED980: expose the original path scanner's three pointers and return the third.
char16_t* ScanPathParts(char16_t* path, char16_t** root, char16_t** filename, char16_t** extension);
struct PathBuffer {
    char16_t data[260];
    // TS3W.exe 0x004EDA50: truncate at the scanner's filename pointer; retain its incidental EAX return.
    uintptr_t RemoveFilename();
};
TS3_X86_OFFSET(PathBuffer, data, 0);
TS3_X86_SIZE(PathBuffer, 520);
} // namespace ts3::resourcesystem
