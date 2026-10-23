#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b17 {
struct Modifier { void* vtable; uint32_t state; void* delegate; };
struct String { char16_t* begin; char16_t* end; char16_t* capacity; const void* label; };
struct SkillModifier { Modifier base; String name; };
TS3_X86_OFFSET(Modifier,state,4);
TS3_X86_OFFSET(Modifier,delegate,8);
TS3_X86_OFFSET(SkillModifier,name,12);
// TS3W.exe 0x0086C3F0: Install the target-name modifier table and clear its state.
Modifier* __fastcall ConstructTarget(Modifier*, void*);
// TS3W.exe 0x0086C520: Construct the delegate-backed name modifier without retaining the delegate.
Modifier* __fastcall ConstructName(Modifier*, void*, void* delegate);
// TS3W.exe 0x0086C500: Install the base table and optionally free the object, returning its old address.
void* __fastcall DestroyBase(Modifier*, void*, uint32_t flags);
// TS3W.exe 0x0086CB30: Release skill-name storage and optionally delete the object; capacity flag bit zero is ignored.
void* __fastcall DestroySkill(SkillModifier*, void*, uint32_t flags);
// TS3W.exe 0x0086CB80: Construct a delegate-backed modifier with an owned UTF16 skill name.
SkillModifier* __fastcall ConstructSkill(SkillModifier*, void*, void* delegate, const char16_t* name);
// TS3W.exe 0x0086C310: Append the actor's resolved modifier name after the separator.
uint32_t __fastcall ActorName(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t unused);
// TS3W.exe 0x0086C400: Append the target's resolved modifier name after the separator.
uint32_t __fastcall TargetName(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t unused);
// TS3W.exe 0x0086C540: Invoke a name delegate with actor and UTF16 output arguments.
uintptr_t __fastcall EvaluateName(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t unused);
// TS3W.exe 0x0086C710: Invoke a quality delegate and apply its optional numeric outputs, preserving duplicate property probes.
uintptr_t __fastcall EvaluateQuality(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t unused);
// TS3W.exe 0x0086CBE0: Invoke the skill-name delegate with an additional owned skill-name argument.
uintptr_t __fastcall EvaluateSkill(SkillModifier*, void*, void* event, char* output, uint32_t capacity, uint32_t unused);
// TS3W.exe 0x0086CDF0: Resolve a position's lot, room and clamped floor, with negative-floor fallback.
uintptr_t __cdecl ResolveRoom(const float* position, void** lot, uint16_t* room, int32_t* floor);
// TS3W.exe 0x0086CF40: Repeat a physics query past flagged interior hits; AL-only returns retain incidental pointer bits in upper EAX.
uint32_t __cdecl QueryInterior(const float* position, const float* endpoint, void* options, void* hit, uint32_t flags);
}
