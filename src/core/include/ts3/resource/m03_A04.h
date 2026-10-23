#pragma once
#include "ts3/resource/m03_manager_contracts.h"
namespace ts3::resource::m03::manager::registry {
struct KeyNameNode; struct KeyNameValue; struct TypeNameNode; struct TypeNameValue; struct TypeNameResult; using InlineEntry=CallbackEntry;
}
namespace ts3::resource::a04_detail {
using namespace m03::manager;
using namespace m03::manager::registry;
struct TreeNode { TreeNode *right,*left,*parent; uint32_t color; FilterEntry value; };
// Anchor has no value: payload overlaps the size and allocator in the real set.
struct TreeView { uint32_t compare; TreeNode *right,*left,*parent; uint32_t color,size; GameAllocator allocator = GameAllocator(); };
TS3_X86_OFFSET(TreeView,size,0x14);
struct VectorView { InlineEntry *begin,*end,*capacity; GameAllocator allocator = GameAllocator(); };
struct NameTable { uint32_t unused; void** buckets; uint32_t count,size; uint32_t policy[3]; GameAllocator allocator = GameAllocator(); };
TS3_X86_OFFSET(NameTable,allocator,0x1c);
struct KeyResult { KeyNameNode* node; KeyNameNode** bucketCell; uint8_t inserted; };
struct ListNode { ListNode *next,*prev; void* value; };
struct List { ListNode *next,*prev; const char* name; };
// TS3W.exe 0x004B0EC0: allocate and link a filter node; flag and comparator use AL only.
TreeNode** InsertNode(TreeView*,TreeNode**,TreeNode*,const FilterEntry*,uint32_t);
// TS3W.exe 0x004B0F50: construct vector storage, retaining nonzero capacity on allocation failure.
VectorView* ConstructVector(VectorView*,uint32_t,const GameAllocator*);
// TS3W.exe 0x004B0FB0: allocate key/name node; null allocation still faults and tail is untouched.
KeyNameNode* CreateNameNode(NameTable*,const KeyNameValue*);
// TS3W.exe 0x004B1000: forward assignment retains/stores/releases then reads flags.
InlineEntry* AssignInline(const InlineEntry*,const InlineEntry*,InlineEntry*);
// TS3W.exe 0x004B1070: backward assignment, last reference resets to one before deletion.
InlineEntry* AssignInlineBackward(const InlineEntry*,const InlineEntry*,InlineEntry*);
// TS3W.exe 0x004B10E0: locked factory lookup, variant -1 bypasses predicate; tests AL.
void* FindFactory(m03::ResourceManager*,uint32_t,uint32_t);
// TS3W.exe 0x004B1180: append borrowed factories to a list in hash iteration order.
int CollectFactories(m03::ResourceManager*,List*,uint32_t);
// TS3W.exe 0x004B1300: count callbacks and optionally append borrowed pointers without locking.
int CollectCallbacks(m03::ResourceManager*,List*);
// TS3W.exe 0x004B1380: locked type-name hash lookup returning its value word.
void* FindTypeName(m03::ResourceManager*,uint32_t);
// TS3W.exe 0x004B13F0: unique filter insertion, preserving output padding.
InsertResult* InsertFilter(TreeView*,InsertResult*,const FilterEntry*,uint32_t);
// TS3W.exe 0x004B14E0: unique unsigned type/name insertion with prime-policy rehash.
TypeNameResult* InsertTypeName(NameTable*,TypeNameResult*,const TypeNameValue*,uint32_t);
// TS3W.exe 0x004B15D0: unique key/name insertion; hashes word0 XOR word3.
KeyResult* InsertKeyName(NameTable*,KeyResult*,const KeyNameValue*,uint32_t);
}

// ABI-only adapters: no state. The manifest expects actual thiscall members.
namespace ts3::resource {
#if defined(_M_IX86)
struct a04 {
    uint32_t InsertNode(void*,void*,void*,uint32_t);
    uint32_t ConstructVector(uint32_t,void*);
    uint32_t CreateNameNode(void*);
    static uint32_t AssignInline(void*,void*,void*);
    static uint32_t AssignInlineBackward(void*,void*,void*);
    uint32_t FindFactory(uint32_t,uint32_t);
    uint32_t CollectFactories(void*,uint32_t);
    uint32_t CollectCallbacks(void*);
    uint32_t FindTypeName(uint32_t);
    uint32_t InsertFilter(void*,void*,uint32_t);
    uint32_t InsertTypeName(void*,void*,uint32_t);
    uint32_t InsertKeyName(void*,void*,uint32_t);
};
#endif
}
