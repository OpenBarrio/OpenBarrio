#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m119 {
struct Node { uint32_t key; unsigned char payload[96]; Node* next; };
struct Table { void* opaque; Node** buckets; uint32_t bucketCount,count; unsigned char policy[12]; void* allocator; };
struct Result { Node* node; Node** bucket; uint8_t inserted; };
struct Input { uint32_t key; void* begin; void* end; };
struct LargeNode { uint32_t key; void* begin; void* end; void* capacity; uint32_t descriptor; unsigned char storage[0xf80]; LargeNode* next; };
TS3_X86_OFFSET(Node,next,0x64);
TS3_X86_OFFSET(Table,allocator,0x1c);
TS3_X86_OFFSET(LargeNode,next,0xf94);
#if defined(_M_IX86)
#define M119_CALL __fastcall
#else
#define M119_CALL
#endif
// TS3W.exe 0x008425D0: find or insert a keyed controller record, returning the output iterator.
Result* M119_CALL Insert(Table*, void*, Result*, const Input*, const void*);
// TS3W.exe 0x00842690: allocate and copy a large inline controller record; null allocation still faults.
LargeNode* M119_CALL CreateLarge(Table*, void*, const Input*);
}
