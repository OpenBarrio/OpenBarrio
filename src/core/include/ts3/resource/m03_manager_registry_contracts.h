// Finite A04 boundary preparation, not registry algorithms or a complete hash ABI.
#pragma once
#include "ts3/resource/m03_manager_contracts.h"
#include "ts3/scratch/clu_408e20_Scratch.h"
namespace ts3::resource::m03::manager::registry {
// Opaque original hash-helper receiver. Only its allocator at x86 +1C is
// certified here; never allocate sizeof this view or infer manager placement.
struct KeyNameTable;
// Bounded used payload only: NOT KeyNameNode, a complete hash table, or an owner.
struct TypeNameTable;
struct TypeNameValue { uint32_t type; ts3::scratch::WideString name; };
struct TypeNameNode { TypeNameValue value; TypeNameNode* next; };
struct TypeNameResult {
    TypeNameNode* node;
    TypeNameNode** bucketCell;
    uint8_t inserted;
    uint8_t padding[sizeof(void*)-1];
};
TS3_X86_OFFSET(TypeNameValue,name,4);
TS3_X86_OFFSET(TypeNameNode,next,0x14);
TS3_X86_SIZE(TypeNameNode,0x18);
TS3_X86_OFFSET(TypeNameResult,inserted,8);
TS3_X86_SIZE(TypeNameResult,12);
struct KeyNameValue { Key key; ts3::scratch::WideString name; };
struct KeyNameNode { KeyNameValue value; KeyNameNode* next; uint32_t untouched; };
TS3_X86_OFFSET(KeyNameValue,name,0x10);
TS3_X86_OFFSET(KeyNameNode,next,0x20);
TS3_X86_SIZE(KeyNameNode,0x28);
// Canonical pointer/flags carrier only. The pointee's used inline count is +4
// on x86, NOT the adjusted DB reference interface from A02.
using InlineEntry=CallbackEntry;
struct NativeInlineEntry { NativeCallback* callback; uint32_t flags; };
struct NativeRegistry { void* identity=nullptr; };
struct NativeCalls {
    TypeNameResult* (*insertTypeName)(void*,NativeRegistry&,TypeNameResult*,const TypeNameValue*,uintptr_t)=nullptr;
    Factory* (*findFactory)(void*,ResourceManager*,uint32_t,uint32_t)=nullptr;
    void* context=nullptr;
    FilterNode** (*insertNode)(void*,FilterSet&,FilterNode**,FilterNode*,const FilterEntry*,uint32_t)=nullptr;
    CallbackVector* (*constructVector)(void*,CallbackVector*,uint32_t,const GameAllocator*)=nullptr;
    KeyNameNode* (*createNameNode)(void*,NativeRegistry&,const KeyNameValue*)=nullptr;
    NativeInlineEntry* (*assignInlineBackward)(void*,const NativeInlineEntry*,const NativeInlineEntry*,NativeInlineEntry*)=nullptr;
    NativeInlineEntry* (*assignInline)(void*,const NativeInlineEntry*,const NativeInlineEntry*,NativeInlineEntry*)=nullptr;
};
class NativeBoundary {
    NativeCalls calls_;
public:
    explicit NativeBoundary(NativeCalls calls):calls_(calls) {}
    TypeNameResult* InsertTypeName(NativeRegistry&,TypeNameResult*,const TypeNameValue*,uintptr_t opaqueWord) const;
    Factory* FindFactory(ResourceManager*,uint32_t type,uint32_t variant) const;
    FilterNode** InsertNode(FilterSet&,FilterNode**,FilterNode*,const FilterEntry*,uint32_t) const;
    CallbackVector* ConstructVector(CallbackVector*,uint32_t,const GameAllocator*) const;
    KeyNameNode* CreateNameNode(NativeRegistry&,const KeyNameValue*) const;
    NativeInlineEntry* AssignInlineBackward(const NativeInlineEntry*,const NativeInlineEntry*,NativeInlineEntry*) const;
    NativeInlineEntry* AssignInline(const NativeInlineEntry*,const NativeInlineEntry*,NativeInlineEntry*) const;
};
#if defined(_M_IX86)
// TS3W.exe 0x004B14E0: ECX type/name helper, (out,value,opaqueWord), ret12; returns out.
// Unsigned type lookup; miss allocates24 and copies WideString. Only inserted BYTE
// is written; output padding survives. Null node allocation is NOT safely handled.
// Preparation fixtures certify captured-policy no-growth ONLY; original stays unpatched.
TypeNameResult* InsertTypeName(TypeNameTable&,TypeNameResult*,const TypeNameValue*,uintptr_t opaqueWord);
// TS3W.exe 0x004B10E0: manager ECX, type/variant words, ret8; borrowed full pointer.
// Locks +1A0, looks up type through +78, walks node vector stride8. Variant -1
// bypasses primary slot12(type,variant); otherwise tests AL only. Normal exits unlock.
void* FindFactory(ResourceManager*,uint32_t type,uint32_t variant);
// TS3W.exe 0x004B0EC0: set ECX; out**, parent*, value*, flag dword; ret16.
// EAX == out; allocates 0x20, copies payload, links/rebalances, increments size.
// Only the flag's low byte and comparator AL select side; no duplicate check/null guard.
FilterNode** InsertNode(FilterSet&,FilterNode**,FilterNode*,const FilterEntry*,uint32_t);
// TS3W.exe 0x004B0F50: vector ECX; count, allocator*; ret8; returns self.
// Null allocation retains null begin/end with capacity word count*8.
CallbackVector* ConstructVector(CallbackVector*,uint32_t,const GameAllocator*);
// TS3W.exe 0x004B0FB0: hash-helper ECX; value*; ret4; returns allocated node.
// Copies Key, copy-constructs WideString via 00408EB0, clears next, leaves +24.
// Null allocation still writes [null+20]; not a safe failure return.
KeyNameNode* CreateNameNode(KeyNameTable&,const KeyNameValue*);
// TS3W.exe 0x004B1000: cdecl(first,last,dest), returns dest end (empty: dest).
// Different pointers: increment new+4, store pointer, decrement old+4; last old
// count resets to one before primary slot0(1). Flags copied AFTER destruction.
InlineEntry* AssignInline(const InlineEntry*,const InlineEntry*,InlineEntry*);
// TS3W.exe 0x004B1070: cdecl(first,last,destEnd), plain ret; backward stride8.
// Reads last-8/destEnd-8 before stepping. Inline +4 retain/store/release;
// last old resets to one before primary slot0(1), then full flags are read/copied.
// Returns destination begin; empty returns destEnd, including null.
InlineEntry* AssignInlineBackward(const InlineEntry*,const InlineEntry*,InlineEntry*);
#endif
}
namespace exehost::m03::manager::registry {
struct Result { unsigned long long cases,failures; };
Result RunNative(const char* label="m03_manager_registry native injected");
}
