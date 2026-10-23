#pragma once
#include "ts3/resource/m03_A01.h"
namespace ts3::scratch { struct WideString; }
namespace ts3::resource::m03::manager::registry { struct TypeNameNode; struct KeyNameNode; }
namespace ts3::resource::m03::a05 {
using TypeNode=manager::registry::TypeNameNode;
using KeyNode=manager::registry::KeyNameNode;
using String=ts3::scratch::WideString;
template<class Node> struct Table {
    uint32_t policy; Node** buckets; uint32_t bucketCount,size;
    uint8_t reserved10[12]; Allocator* allocator; uint8_t reserved20[8];
};
template<class Node> struct Iterator { Node* node; Node** bucket; };
struct Manager {
    a01::Manager prefix;
    Table<TypeNode> typeNames;
    Table<KeyNode> keyNames;
    Mutex nameMutex;
};
struct RawVector {
    manager::CallbackEntry *begin=nullptr,*end=nullptr,*capacity=nullptr;
    GameAllocator allocator{"A05"};
};
struct Value { uint32_t hash; RawVector entries; };
struct ListNode { ListNode *next,*prev; void* value; };
struct DatabaseList { ListNode *next,*prev; Allocator* allocator; uint32_t flags; const char* name; };
struct ResourceList { ListNode *next,*prev; const char* name; };
struct KeyVector { Key *begin,*end,*capacity; };
TS3_X86_OFFSET(Manager,typeNames,0xf0);
TS3_X86_OFFSET(Manager,keyNames,0x118);
TS3_X86_OFFSET(Manager,nameMutex,0x140);
TS3_X86_OFFSET(Table<TypeNode>,allocator,0x1c);
TS3_X86_SIZE(Table<TypeNode>,0x28);
TS3_X86_SIZE(RawVector,0x18);
// TS3W.exe 0x004B16A0: retires type-name chains, caching next before callbacks; signed capacity gate kept.
void RetireTypes(Table<TypeNode>*,TypeNode**,uint32_t);
// TS3W.exe 0x004B1710: retires key-name chains, string before node, clearing each bucket last.
void RetireKeys(Table<KeyNode>*,KeyNode**,uint32_t);
// TS3W.exe 0x004B1780: copy-constructs a hash/vector value and returns destination.
Value* CopyValue(Value*,const Value*);
// TS3W.exe 0x004B17E0: collects borrowed databases under lock; only probe AL selects matches.
uint32_t CollectDatabases(Manager*,DatabaseList*,const Key*);
// TS3W.exe 0x004B18A0: resolves a key name through provider then cache; preserves helper/unlock EAX.
uint32_t GetName(Manager*,const Key*,String*);
// TS3W.exe 0x004B1960: constructs a hash/vector value from independent hash and vector inputs.
Value* MakeValue(Value*,const uint32_t*,const RawVector*);
// TS3W.exe 0x004B19C0: erases a type-name iterator, scans to next nonempty bucket without bounds check.
Iterator<TypeNode>* EraseType(Table<TypeNode>*,Iterator<TypeNode>*,Iterator<TypeNode>);
// TS3W.exe 0x004B1A50: erases a key-name iterator, preserving iterator-by-value ABI.
Iterator<KeyNode>* EraseKey(Table<KeyNode>*,Iterator<KeyNode>*,Iterator<KeyNode>);
// TS3W.exe 0x004B1AE0: gathers keys from databases or callback resources; releases temporary resource list last.
uint32_t CollectKeys(Manager*,KeyVector*,void*,uint32_t);
// TS3W.exe 0x004B1C20: adds/removes filters; only AL of flag and result is semantic.
uint8_t ControlFilter(Manager*,uint32_t,manager::FilterNotify,void*,void*,int32_t);
// TS3W.exe 0x004B1CD0: erases a cached key name; retains unlock upper EAX and replaces AL.
uint32_t RemoveKey(Manager*,const Key*);
// TS3W.exe 0x004B1D50: erases a cached type name; retains unlock upper EAX and replaces AL.
uint32_t RemoveType(Manager*,uint32_t);
#if defined(_M_IX86)
inline void __fastcall HookRetireTypes(Table<TypeNode>* s,void*,TypeNode** b,uint32_t n) { return RetireTypes(s,b,n); }
inline void __fastcall HookRetireKeys(Table<KeyNode>* s,void*,KeyNode** b,uint32_t n) { return RetireKeys(s,b,n); }
inline Value* __fastcall HookCopyValue(Value* s,void*,const Value* v) { return CopyValue(s,v); }
inline Value* __fastcall HookMakeValue(Value* s,void*,const uint32_t* h,const RawVector* v) { return MakeValue(s,h,v); }
inline Iterator<TypeNode>* __fastcall HookEraseType(Table<TypeNode>* s,void*,Iterator<TypeNode>* o,Iterator<TypeNode> i) { return EraseType(s,o,i); }
inline Iterator<KeyNode>* __fastcall HookEraseKey(Table<KeyNode>* s,void*,Iterator<KeyNode>* o,Iterator<KeyNode> i) { return EraseKey(s,o,i); }
inline uint32_t __fastcall HookCollectDatabases(Manager* s,void*,DatabaseList* l,const Key* k) { return CollectDatabases(s,l,k); }
inline uint32_t __fastcall HookGetName(Manager* s,void*,const Key* k,String* o) { return GetName(s,k,o); }
inline uint32_t __fastcall HookCollectKeys(Manager* s,void*,KeyVector* o,void* f,uint32_t mode) { return CollectKeys(s,o,f,mode); }
inline uint8_t __fastcall HookControlFilter(Manager* s,void*,uint32_t add,manager::FilterNotify n,void* c,void* p,int32_t priority) { return ControlFilter(s,add,n,c,p,priority); }
inline uint32_t __fastcall HookRemoveKey(Manager* s,void*,const Key* k) { return RemoveKey(s,k); }
inline uint32_t __fastcall HookRemoveType(Manager* s,void*,uint32_t t) { return RemoveType(s,t); }
#endif
}
