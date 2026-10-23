#pragma once
#include "ts3/layout.h"
#include "ts3/thread/mutex.h"
namespace ts3::resource::m03::g16 {
struct Link { Link* next; Link* prev; };
struct Node { Link link; uint32_t unused; uint8_t flag; uint8_t pad[3]; void (__cdecl* callback)(void*); uintptr_t (__cdecl* step)(void*,uint32_t); void* context; };
struct Queue { void* vtable; uint32_t unused,count; Link pending; uint32_t reserved; thread::Mutex mutex; Link active; };
TS3_X86_OFFSET(Queue,pending,0x0c);
TS3_X86_OFFSET(Queue,mutex,0x18);
TS3_X86_OFFSET(Queue,active,0x48);
TS3_X86_OFFSET(Node,callback,0x10);
struct Config { void* ranges; const char16_t* section; uint32_t length; uint8_t valid; };
// TS3W.exe 0x0072A530: free active nodes, destroy mutex, then free pending nodes; do not invoke callbacks.
uintptr_t __fastcall Destroy(Queue*,void*);
// TS3W.exe 0x0072A730: swap queues and run up to count completed callbacks within the deadline; step returns a replacement function pointer.
uint8_t __fastcall Run(Queue*,void*,int32_t count,uint32_t timeout);
// TS3W.exe 0x0072ACC0: matching config section parses and inserts a range; invalid parsing only clears the sticky valid flag, always returns AL=1.
uint8_t __cdecl Parse(const char16_t* section,const char16_t* value,Config*);
}
