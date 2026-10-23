#pragma once
#include "ts3/layout.h"
#include "ts3/resource/m03_C04.h"
namespace ts3::resource::m03::g09 {
// STBL table view. Only the fields these two entries touch are named; the padding
// fields exist so the x86 offsets stay asserted against the game layout.
using Node=c04::Node;
using Hash=c04::Hash;
struct Table {
    void* const* vtable;      // +00
    uint8_t reserved04[0x10];
    uint32_t flags;           // +14 bit 0x02000000 gates the key predicate
    uint8_t reserved18[0x08];
    uint32_t size20;          // +20 0x00731CC0 stores the computed size here when +24 is zero
    uint32_t capacity24;      // +24
    Hash hash;                // +28 7919 buckets, sentinel +7BE4, count +7BE8
    uint32_t last;            // +7BEC copied from the local filter hash, unspecified in the original
    void* nodes;              // +7BF0 24-byte node array
    void* strings;            // +7BF4 UTF16 string buffer
};
TS3_X86_OFFSET(Table,flags,0x14);
TS3_X86_OFFSET(Table,size20,0x20);
TS3_X86_OFFSET(Table,capacity24,0x24);
TS3_X86_OFFSET(Table,hash,0x28);
TS3_X86_OFFSET(Table,last,0x7bec);
TS3_X86_OFFSET(Table,nodes,0x7bf0);
TS3_X86_OFFSET(Table,strings,0x7bf4);
TS3_X86_SIZE(Table,0x7bf8);
// TS3W.exe 0x0059FA20: two-pass STBL parse of `count` entries into fresh string/node buffers, then store the
// computed data size through 0x00731CC0; size arithmetic wraps at 32 bits. EA quirks kept: the read status is
// zero-extended before the compare against 0xFFFFFFFF, so both 0xFF abort branches are dead - a failed length
// read keeps the pre-zeroed 0 and still reads text, a failed string read still inserts an empty-text node, and
// only a failed key read skips an entry, without consuming stream bytes.
// (rewrite ABI: __fastcall with an unused edx formal, since the original is __thiscall ret 0x10)
uintptr_t __fastcall ParseEntries(Table* self,void*,void* stream,uint32_t countLow,uint32_t countHigh,uint32_t encoding);
// TS3W.exe 0x0059FD70: read src, keep the entries accepted by the flag-0x02000000 predicate, install the fresh
// hash/node buffer (freeing the old nodes, copying the local hash including its unspecified `last` dword, leaving
// the string buffer alone) and return 0x0059F860(self,dst)!=0. EA quirks kept: null src/dst return 0 first, the
// filter pointer is dereferenced unchecked, and the allocated node buffer leaks when a key is rejected or the read
// fails, while the old buffer is always freed before the write is attempted.
// (rewrite ABI: __fastcall with an unused edx formal, since the original is __thiscall ret 0xC)
uint8_t __fastcall FilterWrite(Table* self,void*,void* src,void* dst,const void* filter);
}