#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b02 {
struct PoolNode { PoolNode* next; void* payload[2]; };
struct Pool { Pool* next; Pool* previous; PoolNode* free; PoolNode storage[64]; };
struct Registry {
    unsigned char workspace[0x4000];
    uintptr_t heap[9];
    Pool pool;
    uintptr_t opaque4330[3];
    void* current;
};
TS3_X86_OFFSET(Registry,pool,0x4024);
TS3_X86_OFFSET(Registry,current,0x433c);
struct Key { uint32_t words[4]; };
struct Reader { int32_t position; uintptr_t opaque04,opaque08; unsigned char* end; };
TS3_X86_OFFSET(Reader,end,0xc);
struct Clip {
    unsigned char opaque00[0x28];
    unsigned char bindings[5][11][16];
    Key primary,secondary;
    void* primaryResource;
    void* secondaryResource;
    float first,second;
    uint8_t flag0,flag1;
};
TS3_X86_OFFSET(Clip,primary,0x398);
TS3_X86_OFFSET(Clip,primaryResource,0x3b8);
TS3_X86_OFFSET(Clip,flag0,0x3c8);
// TS3W.exe 0x0054C530: register a factory pair; only AL is replaced in the hash result.
uintptr_t RegisterFactory(uintptr_t key, uintptr_t value);
// TS3W.exe 0x0054C570: forward evaluation to the current node's slot 2.
uintptr_t Evaluate(Registry* self, uintptr_t argument0, uintptr_t argument1);
// TS3W.exe 0x0054C5C0: set the current evaluator and return it.
void* SetCurrent(Registry* self, void* node);
// TS3W.exe 0x0054C5D0: return the current evaluator.
void* GetCurrent(Registry* self);
// TS3W.exe 0x0054C670: initialize the embedded list and fixed node pool.
Pool* InitPool(Pool* self);
// TS3W.exe 0x0054C720: construct the registry's heap, list and current-node state.
Registry* InitRegistry(Registry* self);
// TS3W.exe 0x0054C810: return the evaluator class identifier.
uint32_t TypeC810(void* self);
// TS3W.exe 0x0054C820: query this evaluator or its base identifier.
void* QueryC820(void* self, uint32_t id);
// TS3W.exe 0x0054C8B0: copy aligned payload16 to offset 16 and return the input.
void* SetPayload(void* self, const void* source);
// TS3W.exe 0x0054CD00: return the additive evaluator class identifier.
uint32_t TypeCD00(void* self);
// TS3W.exe 0x0054CD10: query this additive evaluator or its base identifier.
void* QueryCD10(void* self, uint32_t id);
// TS3W.exe 0x0054CEB0: return the evaluator class identifier.
uint32_t TypeCEB0(void* self);
// TS3W.exe 0x0054DB90: read a key in word order 2,3,0,1, preserving failed reads.
uintptr_t ReadKey(Reader* self, Key* key);
// TS3W.exe 0x0054DF50: set the secondary key and acquire before releasing; zero key retains the resource.
uintptr_t SetSecondary(Clip* self, const Key* key);
// TS3W.exe 0x0054E260: load sparse bindings and keys; encoded dimensions are inclusive and unchecked.
uintptr_t LoadClip(Clip* self, Reader* reader);
}

#if defined(_M_IX86)
namespace ts3::anim::b02 {
// TS3W.exe 0x0054C570: ABI adapter for Evaluate.
uintptr_t __fastcall HookEvaluate(Registry* self, void*, uintptr_t a, uintptr_t b);
// TS3W.exe 0x0054C5C0: ABI adapter for SetCurrent.
void* __fastcall HookSetCurrent(Registry* self, void*, void* node);
// TS3W.exe 0x0054C5D0: ABI adapter for GetCurrent.
void* __fastcall HookGetCurrent(Registry* self, void*);
// TS3W.exe 0x0054C670: ABI adapter for InitPool.
Pool* __fastcall HookInitPool(Pool* self, void*);
// TS3W.exe 0x0054C720: ABI adapter for InitRegistry.
Registry* __fastcall HookInitRegistry(Registry* self, void*);
// TS3W.exe 0x0054C810: ABI adapter for TypeC810.
uint32_t __fastcall HookTypeC810(void* self, void*);
// TS3W.exe 0x0054C820: ABI adapter for QueryC820.
void* __fastcall HookQueryC820(void* self, void*, uint32_t id);
// TS3W.exe 0x0054C8B0: ABI adapter for SetPayload.
void* __fastcall HookSetPayload(void* self, void*, const void* source);
// TS3W.exe 0x0054CD00: ABI adapter for TypeCD00.
uint32_t __fastcall HookTypeCD00(void* self, void*);
// TS3W.exe 0x0054CD10: ABI adapter for QueryCD10.
void* __fastcall HookQueryCD10(void* self, void*, uint32_t id);
// TS3W.exe 0x0054CEB0: ABI adapter for TypeCEB0.
uint32_t __fastcall HookTypeCEB0(void* self, void*);
// TS3W.exe 0x0054DB90: ABI adapter for ReadKey.
uintptr_t __fastcall HookReadKey(Reader* self, void*, Key* key);
// TS3W.exe 0x0054DF50: ABI adapter for SetSecondary.
uintptr_t __fastcall HookSetSecondary(Clip* self, void*, const Key* key);
// TS3W.exe 0x0054E260: ABI adapter for LoadClip.
uintptr_t __fastcall HookLoadClip(Clip* self, void*, Reader* reader);
}
#endif
