#pragma once
#include "ts3/layout.h"
#include "ts3/thread/mutex.h"
namespace ts3::audio::b44 {
struct Value { void* table; uintptr_t reserved; uint16_t flags, type; uint32_t padding; union { unsigned char bytes[12]; void* pointer; } data; uint32_t tail; };
struct Entry { uint32_t key; uint32_t padding; Value value; };
struct View { void* table; unsigned char gap[0x24]; void* parent; };
struct Owner { unsigned char prefix[8]; ts3::thread::Mutex mutex; void* resource; void* subscription; };
struct Label { const char* begin; const char* end; const char* capacity; const char* name; };
TS3_X86_SIZE(Value,0x20);
TS3_X86_SIZE(Entry,0x28);
TS3_X86_OFFSET(View,parent,0x28);
TS3_X86_OFFSET(Owner,resource,0x38);
// TS3W.exe 0x008A75D0: Match the local key or delegate to the parent.
bool __fastcall Matches(View*, void*, uint32_t, uint32_t);
// Getter and Clear contracts are AL-only booleans; upper EAX is incidental.
// TS3W.exe 0x008A7610: Read type 1 without changing output on failure.
bool __fastcall GetByte(View*, void*, uint32_t, void*);
// TS3W.exe 0x008A7660: Read type 9.
bool __fastcall GetInt(View*, void*, uint32_t, void*);
// TS3W.exe 0x008A76B0: Read type 10.
bool __fastcall GetWord(View*, void*, uint32_t, void*);
// TS3W.exe 0x008A7700: Read type 13 through x87, preserving signaling-NaN quieting.
bool __fastcall GetFloat(View*, void*, uint32_t, void*);
// TS3W.exe 0x008A7750: Clear resources while holding the original lock.
bool __fastcall Clear(Owner*, void*);
// TS3W.exe 0x008A77B0: Destroy a range of configuration entries.
void __stdcall Destroy(Entry*, Entry*);
// TS3W.exe 0x008A77F0: Assign a range forwards.
Entry* __cdecl Forward(Entry*, Entry*, Entry*);
// TS3W.exe 0x008A7830: Assign a range backwards.
Entry* __cdecl Backward(Entry*, Entry*, Entry*);
// TS3W.exe 0x008A78A0: Initialize the resource configuration allocation label.
Label* __fastcall InitLabel(Label*, void*);
// TS3W.exe 0x008A78C0: Return the configuration class identifier.
uint32_t __fastcall Type(void*, void*);
// TS3W.exe 0x008A78D0: Copy construct a configuration entry.
Entry* __fastcall Copy(Entry*, void*, const Entry*);
}
