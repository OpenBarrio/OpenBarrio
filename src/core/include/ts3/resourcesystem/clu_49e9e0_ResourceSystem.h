#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resourcesystem {
// Resource configuration selection: three pointer fields must grow on native x64.
// The last two dwords contain packed counters/flags; copies do not take ownership.
struct PatternParameters { const char16_t* directory; const char16_t* filename; uint32_t counts; uint32_t flags; };
struct Words20 { const void* descriptor; PatternParameters parameters; };
TS3_X86_SIZE(PatternParameters, 16);
TS3_X86_OFFSET(Words20, parameters, 4);
TS3_X86_SIZE(Words20, 20);
// TS3W.exe 0x004A0A40: copy five-word records backwards, returning the destination begin.
Words20* Copy20Backward(const Words20* first, const Words20* last, Words20* end);
// TS3W.exe 0x00C99A90: copy five-word records forwards, preserving sequential overlapping writes.
Words20* Copy20Forward(const Words20* first, const Words20* last, Words20* out);
// TS3W.exe 0x00C99BF0: construct a range and publish its end; returns the output parameter itself.
Words20** Construct20Range(Words20** end, const Words20* first, const Words20* last, Words20* out);
// TS3W.exe 0x004A0AC0: copy through the first separator, or append a backslash; always writes a terminator.
uint32_t CopyPathComponent(const char16_t* input, char16_t* output, uint32_t capacity);
struct PatternVector {
    Words20* begin; Words20* end; Words20* capacity;
    void* allocator; uint32_t flags; const char* name;
    // TS3W.exe 0x0049E9E0: insert one pattern, preserving alias handling and branch-dependent return.
    uintptr_t Insert(Words20* position, const Words20* value);
    // TS3W.exe 0x004A1760: insert a pattern range; the fourth stack argument is unused.
    Words20* InsertRange(Words20* position, const Words20* first, const Words20* last, int unused);
};
TS3_X86_SIZE(PatternVector, 24);
TS3_X86_OFFSET(PatternVector, allocator, 12);
struct PatternLists {
    void* reserved; PatternVector primary; PatternVector secondary; bool dirty;
    // TS3W.exe 0x0049EDE0: append a primary pattern and mark the list dirty.
    uintptr_t AppendPrimary(const Words20* value);
    // TS3W.exe 0x0049EE40: append a secondary record with at most four payload fields.
    uintptr_t AppendSecondary(const void* descriptor, const PatternParameters* values, uint32_t count);
};
TS3_X86_OFFSET(PatternLists, primary, 4);
TS3_X86_OFFSET(PatternLists, secondary, 28);
TS3_X86_OFFSET(PatternLists, dirty, 52);
// TS3W.exe 0x004ED300: normalize two UTF-16 paths and compare them under the requested mode.
int32_t CompareResourcePaths(const char16_t* first, const char16_t* second, int32_t mode);
struct ResourceArena;
struct PatternStringPool;
// TS3W.exe 0x004A0E80: copy UTF-16 into an arena at this+4; allocation failure is unchecked.
char16_t* CopyPatternString(PatternStringPool* self, const char16_t* text);
}
namespace ts3::resourcesystem {
struct RemovalTreeNode { RemovalTreeNode *left, *right, *parent; uint32_t key; void* resource; };
struct RemovalKeyList { void *begin, *end, *capacity; const char* category; };
struct ResourceRemovalManager {
    uint8_t prefix[512]; void* observer; uint8_t middle[584]; RemovalTreeNode tree;
    // TS3W.exe 0x004A4090: remove selected or all records and notify the optional key observer.
    uintptr_t Remove(uintptr_t unused0, const char16_t* path, uintptr_t selection, uintptr_t unused1);
};
TS3_X86_OFFSET(RemovalTreeNode, resource, 16);
TS3_X86_OFFSET(ResourceRemovalManager, observer, 512);
TS3_X86_OFFSET(ResourceRemovalManager, tree, 1100);
TS3_X86_SIZE(RemovalKeyList, 16);
}
#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
namespace ts3::resourcesystem {
struct ConfigurationPattern {
    uint32_t type, value, argument, extra;
    uint8_t flag, allowMissing, consumed, reserved;
};
struct ConfigurationOutput {
    ConfigurationOutput *next, *previous;
    uint32_t value, inherited, argument, extra;
    uint8_t flag, allowMissing; uint8_t padding[2];
    char16_t* path; void* context; void* attached;
};
using ConfigurationError = void(*)(int32_t, const char16_t*, uint32_t, uint32_t, void*);
struct ResourceConfiguration {
    void* allocator; ResourceArena arena; uint32_t reserved;
    PathVector* configFiles; char16_t path[260]; uint8_t middle[152];
    uint32_t pathLength, inherited, depth;
    ConfigurationOutput *first, *last;
    void* context; ConfigurationError onError; void* errorContext; int32_t error;
    // TS3W.exe 0x004A2A70: expand configuration selections, recursing over literal and wildcard paths.
    bool Expand(PatternVector* patterns);
private:
    void AddOutput(ConfigurationPattern* pattern, const char16_t* filename);
    void PathOverflow();
};
TS3_X86_SIZE(ConfigurationPattern, 20);
TS3_X86_SIZE(ConfigurationOutput, 40);
TS3_X86_OFFSET(ConfigurationOutput, path, 28);
TS3_X86_OFFSET(ResourceConfiguration, arena, 4);
TS3_X86_OFFSET(ResourceConfiguration, configFiles, 40);
TS3_X86_OFFSET(ResourceConfiguration, path, 44);
TS3_X86_OFFSET(ResourceConfiguration, pathLength, 716);
TS3_X86_OFFSET(ResourceConfiguration, first, 728);
TS3_X86_OFFSET(ResourceConfiguration, error, 748);
TS3_X86_SIZE(ResourceConfiguration, 752);
}
