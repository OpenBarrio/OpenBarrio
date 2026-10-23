#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::g11_detail {
struct Group { uint32_t* records{}; void** resolved{}; uint32_t count{}; };
struct Parser {
    // TS3W.exe 0x00726BA0: thiscall adapter for the blob parser.
    uint8_t Parse(void*,uint32_t,uint32_t);
    uint8_t *begin{}, *end{}, *capacity{};
    const char* name{};
    uint32_t strings{}, count{};
    void* storage{};
    uint8_t swapped{};
    uint8_t padding[7]{};
    Group groups[4]{};
};
TS3_X86_OFFSET(Parser,groups,0x24);
TS3_X86_SIZE(Parser,0x54);
struct StringRef { uint8_t* data; uint32_t size; };
struct Cache {
    void* vtable{}; void* database{};
    // TS3W.exe 0x0073CDF0: thiscall adapter for cache key removal.
    void ClearCache();
};
struct KeyList { uint32_t *begin{}, *end{}, *capacity{}; const char* name{}; };
struct Ops {
    void* (*allocate)(uint32_t);
    void (*copy)(Parser*,uint8_t*,uint8_t*,uint32_t);
    void (*extend)(Parser*,uint8_t*,uint32_t,uint8_t*);
    void (*keys)(void*,KeyList*,uint32_t);
    void (*remove)(void*,uint32_t*);
    void (*flush)(void*);
    void (*free)(void*);
};
// Optional native/test dispatch; null uses the original x86 dependencies.
extern const Ops* dispatch;
// TS3W.exe 0x00726BA0: parse an async resource blob, preserving in-place endian swaps and low-byte copy flag.
uint8_t Parse(Parser*,uint32_t*,uint32_t,uint32_t);
// TS3W.exe 0x0073CDF0: remove nonzero-type cache keys in reverse order, flush, then free the key list (void ABI).
void ClearCache(Cache*);
}

namespace ts3::resource::m03 {
// ABI-only non-owning view: both original entry points receive different objects.
struct g11 {
    // TS3W.exe 0x00726BA0: parser thiscall entry; fields are in g11_detail::Parser.
    uint8_t Parse(void*,uint32_t,uint32_t);
    // TS3W.exe 0x0073CDF0: cache thiscall entry; fields are in g11_detail::Cache.
    void ClearCache();
};
}
