#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include <cstddef>
namespace ts3::audio {
struct B06String { char16_t text[250]; };
struct B06Strings {
    B06String *begin, *end, *capacity;
    const char* label;
    // TS3W.exe 0x00862F80: Insert a 500-byte string, preserving aliased input across growth.
    void Insert(void* position, const void* value);
    // TS3W.exe 0x00863290: Append one fixed string, publishing end before the copy.
    void Append(const void* value);
};
struct B06Node { uint32_t key[2]; void* layer; uint32_t hash; B06Node* next; };
struct B06Map {
    uint32_t policy;
    B06Node** buckets;
    uint32_t bucketCount, size;
    float load, growth;
    uint32_t allocator;
    const char* label;
};
struct B06Nodes { void *begin, *end; const char* label; int32_t capacity; };
struct B06Ambience {
    float state[4];
    void* resource;
    B06Map map;
    B06Nodes nodes;
    uint8_t inlineNodes[220];
    uint32_t untouched120;
    void* extra;
    uint8_t loaded;
    // TS3W.exe 0x00862E00: Attach the selected layer interface before activating the layer.
    bool Attach(void* target, uint32_t low, uint32_t high);
    // TS3W.exe 0x00862E60: Reload a layer between stop/start resource-key commands.
    bool Reload(uint32_t low, uint32_t high);
    // TS3W.exe 0x008630D0: Resolve, construct, initialize and retain a typed ambience layer.
    bool AddLayer(void* system, const void* name);
    // TS3W.exe 0x008632D0: Initialize ambience resources and load the configured layer names.
    bool Initialize(void* system);
    // TS3W.exe 0x00863580: Deactivate and detach every layer, then clear the map.
    bool Clear(void* target);
    // TS3W.exe 0x00863620: Dispose node storage, hash buckets and the held resource in order.
    void Dispose();
    // TS3W.exe 0x00863680: Initialize the ambience map and inline node vector, leaving padding intact.
    B06Ambience* Construct();
};
TS3_X86_OFFSET(B06Ambience, map, 0x14);
TS3_X86_OFFSET(B06Ambience, nodes, 0x34);
TS3_X86_OFFSET(B06Ambience, loaded, 0x128);
// TS3W.exe 0x00863710: Submit a keyed float command, or return zero without a command system.
uint32_t B06SetFloat(uint32_t key, float value);
// TS3W.exe 0x008637A0: Handle the matching settings event, interpreting bool variants by their low byte.
struct B06SettingEvent { bool Handle(uint32_t id, void* data); };
// TS3W.exe 0x008638F0: Submit an un-clamped sound float and flags command.
uint32_t B06SoundFloat(uint32_t sound, uint32_t key, float value, uint32_t flags);
}
