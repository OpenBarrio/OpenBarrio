#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::g04_detail {
struct Key { uint32_t word[4]; };
struct Node { Node* next; Node* prev; Key key; void* resource; };
struct Locale {
    void* vtable;
    uint8_t base[12];
    void* allocator;
    uint32_t reserved;
    uint32_t primaryLow,primaryTag,secondaryLow,secondaryTag;
    char16_t name[6];
    void* map;
    uint32_t type,regionId;
    char16_t region[4];
    Node* first; Node* last;
    uint32_t tag,count;
};
TS3_X86_OFFSET(Locale,primaryLow,0x18);
TS3_X86_OFFSET(Locale,map,0x34);
TS3_X86_OFFSET(Locale,first,0x48);
TS3_X86_OFFSET(Node,resource,0x18);
// TS3W.exe 0x0059B190: refresh one locale table, preserving tag filtering and reference ordering; only AL is defined.
bool Refresh(Locale* self,const Key* key);
// TS3W.exe 0x0059BFF0: initialize locale aliases, preferences, factory and callback; preserves cleanup-derived upper EAX.
uint32_t Start(Locale* self);
}

namespace ts3::resource {
// Empty member facade for the registration tool's thiscall adapter; no extra state.
struct g04 : g04_detail::Locale {
    // TS3W.exe 0x0059B190: refresh a locale table through the typed implementation; only AL is defined.
    bool Refresh(const void* key);
    // TS3W.exe 0x0059BFF0: initialize localization through the typed implementation, retaining cleanup-derived EAX.
    uint32_t Start();
};
}
