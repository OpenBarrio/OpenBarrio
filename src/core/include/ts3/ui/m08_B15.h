#pragma once
#include "ts3/layout.h"
namespace ts3::ui::b15 {
struct Callback { void** vtable; void* target; uint32_t mode; uint8_t opaque[8]; uint32_t references; };
struct ControllerPrefix { void** primary; void** secondary; uint32_t mode; uint8_t opaque[4]; Callback callback; };
struct Descriptor { const void* table; void* object; uint32_t count; uint32_t type; };
TS3_X86_OFFSET(Callback, references, 0x14);
TS3_X86_OFFSET(ControllerPrefix, callback, 0x10);
TS3_X86_SIZE(Descriptor, 16);
// TS3W.exe 0x00515C40: adjusts the secondary window receiver and queries its interface.
void* __fastcall F515C40(void* self, void*, uint32_t id);
// TS3W.exe 0x00515C50: returns the window descriptor.
const void* __fastcall F515C50(void* self, void*);
// TS3W.exe 0x00515C60: returns the window type ID.
uint32_t __fastcall F515C60(void* self, void*);
// TS3W.exe 0x00515C70: adjusts the secondary window receiver and releases it.
uint32_t __fastcall F515C70(void* self, void*);
// TS3W.exe 0x00515EC0: queries the controller callback interface or delegates to its base.
void* __fastcall F515EC0(void* self, void*, uint32_t id);
// TS3W.exe 0x00515EF0: returns the controller type ID.
uint32_t __fastcall F515EF0(void* self, void*);
// TS3W.exe 0x00515F00: decrements callback references and invokes shutdown then release; early EAX is undefined.
uintptr_t __fastcall F515F00(Callback* self, void*);
// TS3W.exe 0x00515F40: returns the controller secondary type ID.
uint32_t __fastcall F515F40(void* self, void*);
// TS3W.exe 0x00515FC0: writes a descriptor and returns the virtual type ID.
uint32_t __fastcall F515FC0(void* self, void*, Descriptor* out);
// TS3W.exe 0x00515FF0: adjusts the secondary receiver and releases it.
uint32_t __fastcall F515FF0(void* self, void*);
// TS3W.exe 0x00516010: adjusts the secondary receiver and queries the base interface.
void* __fastcall F516010(void* self, void*, uint32_t id);
// TS3W.exe 0x00516020: adjusts the callback receiver and queries its controller.
void* __fastcall F516020(void* self, void*, uint32_t id);
// TS3W.exe 0x00516030: adjusts the callback receiver and delegates deleting destruction.
void* __fastcall F516030(void* self, void*, uint32_t flags);
}
