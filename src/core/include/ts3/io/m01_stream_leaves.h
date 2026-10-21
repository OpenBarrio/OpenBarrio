#pragma once
#include "ts3/io/m01_streams.h"

namespace ts3::io::m01 {
// TS3W.exe 0x004559C0: return FileStream's type identifier.
uint32_t __fastcall FileType(void*, void*);
// TS3W.exe 0x004559F0: increment the non-atomic MemoryStream reference count, with wrapping arithmetic.
int32_t __fastcall MemoryAddRef(MemoryStream*, void*);
// TS3W.exe 0x00455A20: return MemoryStream's type identifier.
uint32_t __fastcall MemoryType(void*, void*);
// TS3W.exe 0x00455A30: empty the stream through SetData, returning its AL result.
bool __fastcall MemoryClose(MemoryStream*, void*);
// TS3W.exe 0x004DEA60: install the destroyed IStream vtable and optionally delete; return self.
void* __fastcall DestroyStreamBase(void*, void*, uint32_t flags);
// TS3W.exe 0x004DEBA0: construct a non-allocating SharedBuffer around external data.
SharedBuffer* __fastcall WrapBuffer(SharedBuffer*, void*, void* data, bool owns, allocator::ICoreAllocator*);
// TS3W.exe 0x004DEBE0: construct an owning SharedBuffer, retaining a null data allocation on failure.
SharedBuffer* __fastcall AllocateBuffer(SharedBuffer*, void*, uint32_t size, allocator::ICoreAllocator*, const char* name);
// TS3W.exe 0x004DF0C0: initialize a stream and attach only when both buffer and size are nonzero; leave option16 untouched.
MemoryStream* __fastcall ConstructShared(MemoryStream*, void*, SharedBuffer*, uint32_t size, const char* name);
// TS3W.exe 0x005BFB90: return zero state.
int32_t __fastcall MemoryState(void*, void*);
// TS3W.exe 0x006D8F20: reset the SharedBuffer vtable and optionally delete only the holder, never its payload.
SharedBuffer* __fastcall DestroyBuffer(SharedBuffer*, void*, uint32_t flags);
// TS3W.exe 0x0083F5F0: return read/write access flags (3).
int32_t __fastcall MemoryAccess(void*, void*);
// TS3W.exe 0x00D1AA10: return the stream's size field.
uint32_t __fastcall MemorySize(MemoryStream*, void*);
// TS3W.exe 0x00774DB0: return the StreamHost registration name.
const char* __fastcall BindingName(void*, void*);
// Native runtime boundary: the host must supply its translated binding table;
// an image pointer cannot be used in a native process. Not native-complete yet.
inline BindingEntry* nativeBindingTable = nullptr;
// TS3W.exe 0x00776650: store the corrected function/name table pointer and return the out-parameter.
BindingEntry** __fastcall BindingTable(void*, void*, BindingEntry** out);
}
