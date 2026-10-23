#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource {
struct A11String { char16_t* begin; char16_t* end; char16_t* capacity; const char* tag; };
struct A11Entry { uint32_t first, second; A11String name; uintptr_t metadata[4]; };
struct A11Vector { A11Entry* begin; A11Entry* end; A11Entry* capacity; const char* tag; };
struct A11Directory { void** vtable; A11Vector entries; };
TS3_X86_SIZE(A11Entry, 40);
// TS3W.exe 0x0072ECD0: insert a directory record, preserving alias relocation and capacity doubling.
uintptr_t A11Insert(A11Vector*, A11Entry*, const A11Entry*);
// TS3W.exe 0x0072EE70: append a directory record, growing only when full.
uintptr_t A11Append(A11Vector*, const A11Entry*);
// TS3W.exe 0x0072EEC0: add a unique directory name under the directory mutex; compares with mode 5.
uint8_t A11AddDirectory(A11Directory*, const char16_t*, uint32_t, uint32_t);
// TS3W.exe 0x0072F4A0: deserialize the debug key/name map, retaining clearing-before-validation and partial insertions.
uint8_t A11LoadNames(void*, void*);
struct A11Factory { void** vtable; int32_t refs; void* allocator; uint32_t type; };
TS3_X86_OFFSET(A11Factory, type, 0xC);
struct A11Cursor { uintptr_t current, first, last; uintptr_t* node; };
TS3_X86_SIZE(A11Cursor, 16);
// TS3W.exe 0x0072F020: dispatch matching serialization and always close the stream; retains cleanup EAX upper bits.
uintptr_t A11Write(A11Factory*, void*, void*, uintptr_t, uint32_t);
// TS3W.exe 0x0072F050: report one supported type, writing it only when capacity is nonzero.
uint32_t A11Types(A11Factory*, uint32_t*, uint32_t);
// TS3W.exe 0x0072F070: create a resource only for the factory's type.
void* A11Create(A11Factory*, uint32_t);
// TS3W.exe 0x0072F090: create/addref/load a resource, closing failed streams; success inherits output pointer upper bits.
uintptr_t A11Load(A11Factory*, void*, void**, uintptr_t, uint32_t);
// TS3W.exe 0x0072F120: copy record key and dispatch matching load, always closing its stream.
uintptr_t A11Read(A11Factory*, void*, void*, uintptr_t, uint32_t);
// TS3W.exe 0x0072F190: construct the common factory and reset its refcount.
A11Factory* A11Construct(A11Factory*, uint32_t, void*);
// TS3W.exe 0x0072F1D0: exchange the main resource singleton (not an atomic exchange).
void* A11Exchange(void*);
// TS3W.exe 0x0072F770: advance a 16-element block deque cursor with EA signed page arithmetic.
A11Cursor* A11Advance(A11Cursor*, int32_t);
// TS3W.exe 0x0072F7E0: copy a deque cursor and subtract a wrapped signed displacement.
A11Cursor* A11Subtract(A11Cursor*, A11Cursor*, int32_t);
}

namespace ts3::resource {
#if defined(_M_IX86)
#define A11_HOOK __fastcall
#else
#define A11_HOOK
#endif
// TS3W.exe 0x0072ECD0: ABI replacement adapter for A11Insert.
uintptr_t A11_HOOK HookA11Insert(A11Vector* s, void*, A11Entry* p, const A11Entry* v);
// TS3W.exe 0x0072EE70: ABI replacement adapter for A11Append.
uintptr_t A11_HOOK HookA11Append(A11Vector* s, void*, const A11Entry* v);
// TS3W.exe 0x0072EEC0: ABI replacement adapter for A11AddDirectory.
uint8_t A11_HOOK HookA11AddDirectory(A11Directory* s, void*, const char16_t* n, uint32_t a, uint32_t b);
// TS3W.exe 0x0072F020: ABI replacement adapter for A11Write.
uintptr_t A11_HOOK HookA11Write(A11Factory* s, void*, void* r, void* v, uintptr_t c, uint32_t t);
// TS3W.exe 0x0072F050: ABI replacement adapter for A11Types.
uint32_t A11_HOOK HookA11Types(A11Factory* s, void*, uint32_t* o, uint32_t n);
// TS3W.exe 0x0072F070: ABI replacement adapter for A11Create.
void* A11_HOOK HookA11Create(A11Factory* s, void*, uint32_t t);
// TS3W.exe 0x0072F090: ABI replacement adapter for A11Load.
uintptr_t A11_HOOK HookA11Load(A11Factory* s, void*, void* v, void** o, uintptr_t c, uint32_t t);
// TS3W.exe 0x0072F120: ABI replacement adapter for A11Read.
uintptr_t A11_HOOK HookA11Read(A11Factory* s, void*, void* v, void* r, uintptr_t c, uint32_t t);
// TS3W.exe 0x0072F190: ABI replacement adapter for A11Construct.
A11Factory* A11_HOOK HookA11Construct(A11Factory* s, void*, uint32_t t, void* a);
// TS3W.exe 0x0072F1D0: ABI replacement adapter for A11Exchange.
void* HookA11Exchange(void* v);
// TS3W.exe 0x0072F4A0: ABI replacement adapter for A11LoadNames.
uint8_t A11_HOOK HookA11LoadNames(void* s, void*, void* v);
// TS3W.exe 0x0072F770: ABI replacement adapter for A11Advance.
A11Cursor* A11_HOOK HookA11Advance(A11Cursor* s, void*, int32_t n);
// TS3W.exe 0x0072F7E0: ABI replacement adapter for A11Subtract.
A11Cursor* A11_HOOK HookA11Subtract(A11Cursor* s, void*, A11Cursor* o, int32_t n);
}
