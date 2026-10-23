// Finite A02 boundaries only: original bridges, not manager algorithms/hooks.
#pragma once
#include "ts3/resource/m03_manager_contracts.h"
namespace ts3::resource::m03::manager::refs {
// Native reference receiver is explicitly distinct from the database primary
// object. This is NOT the x86 +4 layout and never carries a raw game vtable.
struct DatabaseReference {};
// Non-owning identity plus an independent receiver token: host padding/offsets
// are deliberately unrelated to game +4. This is not a manager/DB interface.
struct NativeDatabase { void* identity=nullptr; DatabaseReference reference; };
using NativeCallback = manager::NativeCallback;
// Identity only: no raw EA layout or production hash/ownership policy.
struct NativeBucketOwner {};
struct RecordResource; // opaque original input: used key bytes at+8..17 only
struct NativeRecordResource {}; // independent host identity, never an EA view
struct CallbackBuckets; // used original receiver: manager+78, not a complete manager
struct TypeNameBuckets; // original manager+F0; distinct from callback buckets
struct NativeCalls {
    void* context=nullptr;
    // Whole 004B0810 operation, not an emulated manager/record algorithm.
    uint32_t (*delegateRecord)(void*,manager::NativeState&,const Key*,void**,uint32_t,NativeDatabase*,NativeCallback*,const Key*)=nullptr;
    uint32_t (*callback7)(void*,NativeCallback&,const Key*,uint32_t)=nullptr;
    uint32_t (*callback10)(void*,NativeCallback&,void*)=nullptr;
    uint32_t (*retainDatabase)(void*,DatabaseReference&)=nullptr;
    uint32_t (*releaseDatabase)(void*,DatabaseReference&)=nullptr;
    uint32_t (*retainCallback)(void*,NativeCallback&)=nullptr;
    uint32_t (*releaseCallback)(void*,NativeCallback&)=nullptr;
    // Appended to preserve all prior aggregate positions. Whole operation only.
    void (*rehashBuckets)(void*,NativeBucketOwner&,uint32_t)=nullptr;
    void (*rehashTypeNameBuckets)(void*,NativeBucketOwner&,uint32_t)=nullptr;
    uint8_t (*applyRecord)(void*,manager::NativeState&,NativeRecordResource&,uint32_t,NativeDatabase*,NativeCallback*,const Key*)=nullptr;
    // Whole 004B0480 boundary; supplied native policy does NOT certify an EA cache.
    uint32_t (*resolveCachedRecord)(void*,manager::NativeState&,uint32_t,const Key*,void**,uint32_t,NativeDatabase*,NativeCallback*,const Key*)=nullptr;
};
// No fallback to mapped addresses, generic success implementation or ownership
// assumption. Raw results survive unchanged; selection callers decide AL usage.
class NativeBoundary {
    NativeCalls calls_;
public:
    explicit NativeBoundary(NativeCalls calls):calls_(calls) {}
    uint32_t ResolveCachedRecord(manager::NativeState&,uint32_t,const Key*,void**,uint32_t,NativeDatabase*,NativeCallback*,const Key*) const;
    uint8_t ApplyRecord(manager::NativeState&,NativeRecordResource&,uint32_t,NativeDatabase*,NativeCallback*,const Key*) const;
    void RehashBuckets(NativeBucketOwner&,uint32_t) const;
    void RehashTypeNameBuckets(NativeBucketOwner&,uint32_t) const;
    uint32_t DelegateRecord(manager::NativeState&,const Key*,void**,uint32_t,NativeDatabase*,NativeCallback*,const Key*) const;
    uint32_t Callback7(NativeCallback&,const Key*,uint32_t) const;
    uint32_t Callback10(NativeCallback&,void*) const;
    uint32_t RetainDatabase(NativeDatabase&) const;
    uint32_t ReleaseDatabase(NativeDatabase&) const;
    uint32_t RetainCallback(NativeCallback&) const;
    uint32_t ReleaseCallback(NativeCallback&) const;
};
#if defined(_M_IX86)
// TS3W.exe 0x004B0480: manager ECX, seven words, ret28; opaque whole operation.
// Cached slot22/slot6 test AL; early hit sets AL1 over unlock upper EAX.
uint32_t ResolveCachedRecord(ResourceManager*,uint32_t,const Key*,void**,uint32_t,void*,RawCallback*,const Key*);
// TS3W.exe 0x004B0840: manager ECX, (resource,word,db,factory,keyOverride), ret20.
// Copies override or resource+8 key; primary DB13(3,2,1,0), factory10;
// caches AL across primary record2 release. Upper EAX is incidental.
uint8_t ApplyRecord(ResourceManager*,RecordResource*,uint32_t,void*,RawCallback*,const Key*);
// TS3W.exe 0x004B0280: ECX callback buckets, positive count word, ret4;
// allocator slots2/3; next+1C; free before publication. EAX is incidental,
// NOT a semantic result. No null-allocation guard; never probe failure here.
void RehashBuckets(CallbackBuckets*,uint32_t);
// TS3W.exe 0x004B0330: ECX type/name buckets, count word, ret4; next+14.
// Slot2 allocates, slot3 frees oldCount>1 before publication; EAX incidental.
// No allocation-null/zero-count guard; expose void, not allocator return.
void RehashTypeNameBuckets(TypeNameBuckets*,uint32_t);
// TS3W.exe 0x004B0810: unchanged manager ECX, six words, ret24; forwards
// (0,key,out,flags,database,factory,alias) to 004B0480, full EAX unchanged.
// out is resource identity, not the temporary record; only callee AL is status.
uint32_t DelegateRecord(ResourceManager*,const Key*,void**,uint32_t,void*,RawCallback*,const Key*);
// TS3W.exe 0x00736610: upstream six-word ret24 wrapper; converts key through
// 007D80E0 into a stack-local Key, then calls 004B0810; full EAX unchanged.
uint32_t ConvertDelegateRecord(ResourceManager*,const Key*,void**,uint32_t,void*,RawCallback*,const Key*);
// TS3W.exe 0x004B00C0: manager ECX; key*, flag dword; ret8. Lookup
// slot6(key+8,0), selected slot7(key,flag) raw EAX; fallback only if flag AL.
// Fallback tests AL and changes ONLY AL to 0/1, leaving upper EAX incidental.
uint32_t Delegate7(ResourceManager*,const Key*,uint32_t);
// TS3W.exe 0x004B0130: manager ECX; callback*, argument; ret8. Non-null
// callback slot10(argument), else broadcast in vector order (no AL short cut).
// Full EAX is last slot return; empty broadcast EAX is manager itself.
uint32_t Delegate10(ResourceManager*,RawCallback*,void*);
// TS3W.exe 0x004B0220: ignored ECX; [first,last) DB entries; ret8;
// each non-null calls db+4 slot1 (release); entries/flags unchanged. EAX incidental.
uint32_t ReleaseDatabases(DatabaseEntry*,DatabaseEntry*);
// TS3W.exe 0x004B0250: ignored ECX; [first,last) callback entries; ret8;
// each non-null calls PRIMARY slot2 (release); entries unchanged. EAX incidental.
uint32_t ReleaseCallbacks(CallbackEntry*,CallbackEntry*);
// TS3W.exe 0x004B03E0: cdecl(outCursor**,first,last,dest), returns outCursor;
// writes pointer, retains via db+4 slot0, THEN copies flags; advances cursor.
DatabaseEntry** CopyDatabases(DatabaseEntry**,const DatabaseEntry*,const DatabaseEntry*,DatabaseEntry*);
// TS3W.exe 0x004B0430: cdecl(outCursor**,first,last,dest), returns outCursor;
// writes pointer, retains via PRIMARY slot1, THEN copies flags; advances cursor.
CallbackEntry** CopyCallbacks(CallbackEntry**,const CallbackEntry*,const CallbackEntry*,CallbackEntry*);
// Direct virtual boundaries used by the above originals; no ownership inference
// beyond the positively probed paired acquisition/release calls.
uint32_t RetainDatabase(void*);
uint32_t ReleaseDatabase(void*);
uint32_t RetainCallback(RawCallback*);
uint32_t ReleaseCallback(RawCallback*);
uint32_t Callback7(RawCallback*,const Key*,uint32_t);
uint32_t Callback10(RawCallback*,void*);
#endif
}
namespace exehost::m03::manager::refs {
struct Result { unsigned long long cases,failures; };
Result RunNative(const char* label="m03_manager_refs native injected");
}
