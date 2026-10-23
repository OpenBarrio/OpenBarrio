#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m121 {
struct Entry {
 uint32_t *begin,*end,*capacity; const char* descriptor; uint32_t* inlineBase;
 uint32_t storage,tail;
};
struct Entries {
 Entry *begin,*end,*capacity; const char* descriptor; Entry* inlineBase;
 // TS3W.exe 0x00844450: Insert a style entry, preserving alias adjustment and capacity-valued growth return.
 uintptr_t Insert(void* position,void* source);
 // TS3W.exe 0x008447A0: Append an empty exclusion vector; growth returns the temporary's dangling begin pointer.
 uintptr_t Append();
};
struct LargeValue { unsigned char *begin,*end,*capacity; const char* descriptor; unsigned char storage[3968]; };
struct SmallValue { unsigned char *begin,*end,*capacity; const char* descriptor; unsigned char storage[80]; };
struct LargeNode { uint32_t key; LargeValue value; LargeNode* next; };
struct Result { LargeNode* node; LargeNode** bucket; uint8_t inserted; };
struct LargeTable {
 void* allocator; LargeNode** buckets; uint32_t bucketCount,count; uint32_t policy[2];
 // TS3W.exe 0x00844380: Find or insert a large keyed node, rehashing after node allocation.
 Result* Insert(void* result,const void* pair,uintptr_t hint);
 // TS3W.exe 0x00844870: Find or create a large style range and destroy temporary entries in order.
 LargeValue* Get(const void* key);
};
struct SmallTable {
 void* allocator; void** buckets; uint32_t bucketCount,count; uint32_t policy[2];
 // TS3W.exe 0x008446C0: Find or create a small keyed range and return its value subobject.
 SmallValue* Get(const void* key);
};
struct Style {
 unsigned char prefix[0x6c]; SmallTable table;
 // TS3W.exe 0x00844820: Replace the destination range unless aliased; only AL is the boolean result.
 bool Copy(uint32_t key,void* destination);
};
TS3_X86_SIZE(Entry,28);
TS3_X86_OFFSET(LargeNode,next,3988);
TS3_X86_OFFSET(Style,table,0x6c);
}
