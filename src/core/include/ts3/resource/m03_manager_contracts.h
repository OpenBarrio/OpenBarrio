// Bounded used-field views, not a complete ResourceManager interface or rewrite.
#pragma once
#include "ts3/resource/m03_layouts.h"
#include "ts3/resource/m03_interfaces.h"
#include "ts3/eastl_game.h"
#include "ts3/misc/clu_4a6c30_x.h"
#include <stdexcept>
#include <EASTL/set.h>

namespace ts3::resource::m03::manager {
// 004B3575..358E initializes eight pointers/words; 004AFDF7..0E only
// admits SEVEN live entries, reserving the eighth pointer as a null sentinel.
struct Controls {
    void* pointers[8]{};
    uint16_t types[8]{};
    uint32_t policy=0;
    float factor=0;
};
TS3_X86_OFFSET(Controls,types,0x20);
TS3_X86_OFFSET(Controls,policy,0x30);
TS3_X86_OFFSET(Controls,factor,0x34);
TS3_X86_SIZE(Controls,0x38);
// Returns borrowed pointer storage, not a copy or ownership transfer.
void** PointerSlots(Controls&);

// Raw game objects only. A native service must NEVER be stored in this view.
struct RawCallback { const void* const* vtable; };
struct CallbackEntry { RawCallback* callback; uint32_t flags; };
struct DatabaseEntry { void* database; uint32_t flags; };
using CallbackVector = GameVector<CallbackEntry>;
using DatabaseVector = GameVector<DatabaseEntry>;
TS3_X86_SIZE(CallbackEntry,8);
TS3_X86_SIZE(DatabaseEntry,8);

// FISet helpers receive manager+0C (004B1C57), not manager+10.
// Header/links are the game's used tree view; ResourceKey remains canonical.
// 004B0960: node+10 is cdecl notification(database,key,context), +14
// context, +18 optional predicate (raw game slot1(key*) -> AL), +1C priority.
// These are NOT ResourceKey words. The native layout retains all three pointers.
using FilterNotify = void (*)(void* database,const Key* key,void* context);
struct KeyRange { const Key* first; const Key* last; };
TS3_X86_SIZE(KeyRange,8);
struct FilterEntry {
    FilterNotify notify=nullptr;
    void* context=nullptr;
    void* predicate=nullptr;
    int32_t priority=0;
};
TS3_X86_OFFSET(FilterEntry,context,4);
TS3_X86_OFFSET(FilterEntry,predicate,8);
TS3_X86_OFFSET(FilterEntry,priority,0x0c);
TS3_X86_SIZE(FilterEntry,0x10);
// Signed priority DESCENDING, then pointer words unsigned ascending. Native
// ordering uses uintptr_t, never truncating a native function/object pointer.
struct FilterLess {
    uint32_t unused=0;
    bool operator()(const FilterEntry&,const FilterEntry&) const;
};
using FilterNode = eastl::rbtree_node<FilterEntry>;
using FilterSet = eastl::set<FilterEntry,FilterLess,GameAllocator>;
TS3_X86_OFFSET(FilterNode,mValue,0x10);
TS3_X86_SIZE(FilterNode,0x20);
TS3_X86_OFFSET(FilterSet,mAnchor,4);
TS3_X86_OFFSET(FilterSet,mnSize,0x14);
TS3_X86_OFFSET(FilterSet,mAllocator,0x18);
TS3_X86_SIZE(FilterSet,0x24);
struct InsertResult { FilterNode* node; uint8_t inserted; };
TS3_X86_OFFSET(InsertResult,inserted,4);

// Native boundary state uses native pointers and an explicitly injected service.
// It is not a layout-compatible replacement for raw game vtables.
// Calls below inject WHOLE used helpers. They forward raw predicate words (only
// AL may be tested by users); they do not implement manager algorithms. In
// particular delegateDatabase hides x86's stack alias, and parseName's service
// receives native state identity, never a raw manager/vtable or context fallback.
// All returned objects and pointer-array storage are borrowed; callers own the
// service/state lifetime. Missing services throw before touching state.
struct NativeCallback { virtual ~NativeCallback()=default; };
struct NativeState { Controls controls; };
// Opaque native owner identity; no EA hash/node/string representation.
struct NativeRetirementOwner { void* identity=nullptr; };
struct KeyNameRetirementOwner; // distinct x86 manager+118 table receiver
struct TypeNameRetirementOwner; // x86 table receiver, not a complete object
struct NativeEntryCopyOwner { void* identity=nullptr; };
#if defined(_M_IX86)
// Complete 1780 value prefix; enclosing hash-node next word is NOT part of it.
struct EntryVectorValue { uint32_t hash; CallbackVector entries; };
TS3_X86_OFFSET(EntryVectorValue,entries,4);
TS3_X86_SIZE(EntryVectorValue,0x1c);
#endif
struct NativeCalls {
    void* context=nullptr;
    NativeState* (*singleton)(void*)=nullptr;
    uint32_t (*initialize)(void*,NativeState&)=nullptr;
    Database* (*delegateDatabase)(void*,NativeState&,const Key*)=nullptr;
    int32_t (*databaseLock)(void*,NativeState&,uint32_t)=nullptr;
    uint32_t (*control)(void*,NativeState&,uint32_t,void*,uint32_t)=nullptr;
    void* (*findControl)(void*,NativeState&,uint32_t)=nullptr;
    uint32_t (*readPolicy)(void*,NativeState&,float*)=nullptr;
    uint32_t (*setPolicy)(void*,NativeState&,uint32_t,float)=nullptr;
    uint8_t (*parseName)(void*,NativeState&,ts3::misc::ResourceSortKey*,const char16_t*,uint32_t,uint32_t)=nullptr;
    uint32_t (*containsDatabase)(void*,NativeState&,Database*)=nullptr;
    NativeCallback* (*findCallback)(void*,NativeState&,const uint32_t*)=nullptr;
    Database* (*findDatabase)(void*,NativeState&,const Key*,uint32_t*)=nullptr;
    int32_t (*notifyFilters)(void*,NativeState&,void*,const KeyRange*)=nullptr;
    FilterNode** (*findFilter)(void*,FilterSet&,FilterNode**,const FilterEntry*)=nullptr;
    FilterNode** (*eraseFilter)(void*,FilterSet&,FilterNode**,FilterNode*)=nullptr;
    InsertResult* (*insertFilter)(void*,FilterSet&,InsertResult*,const FilterEntry*,uint32_t)=nullptr;
    void (*retireKeyNames)(void*,NativeRetirementOwner&,void**,uint32_t)=nullptr;
    void (*retireTypeNames)(void*,NativeRetirementOwner&,void**,uint32_t)=nullptr;
    NativeEntryCopyOwner* (*copyEntryVector)(void*,NativeEntryCopyOwner&,const NativeEntryCopyOwner&)=nullptr;
};
class NativeBoundary {
    NativeCalls calls_;
public:
    explicit NativeBoundary(NativeCalls calls):calls_(calls) {}
    NativeEntryCopyOwner* CopyEntryVector(NativeEntryCopyOwner&,const NativeEntryCopyOwner&) const;
    void RetireKeyNames(NativeRetirementOwner&,void** buckets,uint32_t count) const;
    void RetireTypeNames(NativeRetirementOwner&,void** buckets,uint32_t count) const;
    NativeState* Singleton() const;
    uint32_t Initialize(NativeState&) const;
    Database* DelegateDatabase(NativeState&,const Key*) const;
    int32_t DatabaseLock(NativeState&,uint32_t) const;
    uint32_t Control(NativeState&,uint32_t,void*,uint32_t) const;
    void* FindControl(NativeState&,uint32_t) const;
    uint32_t ReadPolicy(NativeState&,float*) const;
    uint32_t SetPolicy(NativeState&,uint32_t,float) const;
    uint8_t ParseName(NativeState&,ts3::misc::ResourceSortKey*,const char16_t*,uint32_t,uint32_t) const;
    uint32_t ContainsDatabase(NativeState&,Database*) const;
    NativeCallback* FindCallback(NativeState&,const uint32_t*) const;
    Database* FindDatabase(NativeState&,const Key*,uint32_t*) const;
    int32_t NotifyFilters(NativeState&,void*,const KeyRange*) const;
    FilterNode** FindFilter(FilterSet&,FilterNode**,const FilterEntry*) const;
    FilterNode** EraseFilter(FilterSet&,FilterNode**,FilterNode*) const;
    InsertResult* InsertFilter(FilterSet&,InsertResult*,const FilterEntry*,uint32_t) const;
};
#if defined(_M_IX86)
// TS3W.exe 0x004B1780: ECX destination, source word, ret4/full destination;
// real 0F50 then 0430 retain on PRIMARY slot1; publishes copied end afterward.
EntryVectorValue* CopyEntryVector(EntryVectorValue*,const EntryVectorValue*);
// TS3W.exe 0x004B1710: ECX key/name owner, (buckets,count), ret8; void.
// Caches next+20, signed masked capacity gate, string before node40; delayed head clear.
void RetireKeyNames(KeyNameRetirementOwner&,void** buckets,uint32_t count);
// TS3W.exe 0x004B16A0: ECX type/name owner, (buckets,count), ret8; void.
// Caches next+14 before string/node callbacks; clears head after complete chain.
// Incidental EAX is not a result; 1710 is a distinct node family.
void RetireTypeNames(TypeNameRetirementOwner&,void** buckets,uint32_t count);
// All offsets below are into the manager primary view, never a native service.
Controls& ControlFields(ResourceManager*);
DatabaseVector& Databases(ResourceManager*);
CallbackVector& Callbacks(ResourceManager*);
FilterSet& FilterTree(ResourceManager*);
Mutex& DatabaseMutex(ResourceManager*);
Allocator*& ManagerAllocator(ResourceManager*);
// TS3W.exe 0x004AFDA0: slot16(key*, outFlags*) on unchanged manager ECX;
// ret4; full EAX borrowed database*. outFlags aliases the wrapper's STACK copy
// of key* (initial contents == key address), not the caller key or flags storage.
void* DelegateDatabase(ResourceManager*,const Key*);
// TS3W.exe 0x004AFF00: stores policy and nonzero factor, else 0.8f; ret8.
// +/-0 become 0.8f; negative and unordered NaN bits survive. EAX is policy with AH replaced
// by UCOMISS/LAHF flags (02 positive, 42 zero, 03 negative, 47 unordered).
// Uses ambient MXCSR: with DAZ a denormal also selects 0.8f/equal flags.
uint32_t SetPolicy(ResourceManager*,uint32_t,float);
// TS3W.exe 0x004AFF40: parser 004AECE0(out,UTF16,type,group,manager,null),
// cdecl helper / ret16 wrapper; only AL defined. Uses the canonical parser type,
// whose historic member names do NOT describe ResourceKey's memory word order.
uint8_t ParseName(ResourceManager*,ts3::misc::ResourceSortKey*,const char16_t*,uint32_t,uint32_t);
// TS3W.exe 0x004B0F50: ECX uninitialized 8-byte-entry vector; capacity,
// GameAllocator*; ret8; EAX == self even if allocation fails. Copies all three
// allocator words. Failure leaves begin/end null but capacity == count*8.
CallbackVector* ConstructEntryVector(CallbackVector*,uint32_t,const GameAllocator*);
// TS3W.exe 0x004B0070: ECX manager; stack type*, ret4; EAX borrowed callback*.
RawCallback* FindCallback(ResourceManager*,const uint32_t*);
// TS3W.exe 0x004AFFC0: ECX manager; key*, outFlags*; ret8; EAX borrowed
// database* (null on miss, outFlags untouched). Uses database slot13 with
// (key,null,1,6,1,0); only AL decides whether to stop. No byte-accounting claim.
void* FindDatabase(ResourceManager*,const Key*,uint32_t*);
// Callback slot6 (+18): ECX callback; type*, zero on stack, ret8; AL predicate.
uint32_t ProbeCallback(RawCallback*,const uint32_t*);
// TS3W.exe 0x004B0180: ECX manager+0C; out**, filter*; ret8; EAX == out.
FilterNode** FindFilter(FilterSet&,FilterNode**,const FilterEntry*);
// TS3W.exe 0x004B0DD0: ECX manager+0C; out**, node*; ret8; EAX == out.
FilterNode** EraseFilter(FilterSet&,FilterNode**,FilterNode*);
// TS3W.exe 0x004B13F0: ECX manager+0C; out*, filter*, flag dword; ret12;
// EAX == out. Only byte out+4 is written (padding is NOT initialized).
InsertResult* InsertFilter(FilterSet&,InsertResult*,const FilterEntry*,uint32_t);
// TS3W.exe 0x004AFDC0: ECX manager; low byte stack flag; ret4. Return
// inherited from mutex Lock/Unlock, not a normalized success boolean.
int DatabaseLock(ResourceManager*,uint32_t);
// TS3W.exe 0x004B0960: ECX manager; database*, range*; ret8. EAX is
// final mutex Unlock result, NOT a notification count or normalized boolean.
// Unlocks around predicate slot1 and cdecl FilterNotify, then reacquires.
int32_t NotifyFilters(ResourceManager*,void*,const KeyRange*);
#endif
} // namespace

namespace exehost { class ExeImage; }
namespace exehost::m03::manager {
struct Result { unsigned long long cases,failures; };
Result RunNative(const char* label="m03_manager_native");
}
