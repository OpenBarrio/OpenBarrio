#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::d05 {
struct Key { uint32_t words[4]; };
struct KeyList { Key* begin; Key* end; Key* capacity; const char* name; };
struct Predicate { void* vtable; void* context; };
struct Database { void* vtable; void* refVtable; };
TS3_X86_SIZE(KeyList,16);
TS3_X86_SIZE(Predicate,8);
// TS3W.exe 0x007D8030: mounts a database, retaining/releasing its secondary ref view; null allocation still faults.
uint8_t Mount(const char16_t* path,uint32_t flags);
// TS3W.exe 0x007D80E0: resolves an untagged missing key through its candidate, copying to out in dword order.
Key* Resolve(Key* out,const Key* key);
// TS3W.exe 0x007D9060: enumerates filtered keys, dispatches the main service and frees the temporary key list.
uint8_t Enumerate(void* database,void* predicateContext,void* argument);
// TS3W.exe 0x007E3EB0: returns minus one.
int32_t Unsupported(void* self);
// TS3W.exe 0x0089F6C0: returns the dword at object offset 0x0C.
uint32_t Field0C(const void* self);
// TS3W.exe 0x009388C0: returns the service identifier object's address, not its first dword.
const uint64_t* ServiceId(void* self);
// TS3W.exe 0x0093DDF0: returns true in AL and ignores both stack arguments; upper EAX is undefined.
uint8_t Observe(void* self,void* first,void* second);
// TS3W.exe 0x0093ECC0: returns the dword at object offset 0x18.
uint32_t Field18(const void* self);
// TS3W.exe 0x009D6C70: installs the base vtable, optionally core-frees on flag bit zero, and returns self.
void* Destroy(void* self,uint32_t flags);
// TS3W.exe 0x00A47040: returns the dword at object offset 0x74.
uint32_t Field74(const void* self);
// TS3W.exe 0x00A47050: returns the dword at object offset 0x78.
uint32_t Field78(const void* self);
// TS3W.exe 0x00C93970: returns the dword at object offset 0x08.
uint32_t Field08(const void* self);
// TS3W.exe 0x00C94530: returns the embedded storage address at object offset 0xD8.
void* Storage(void* self);
// TS3W.exe 0x00F81560: constructs the locale mutex then registers F91910, returning atexit's status.
int InitLocaleMutex();
// TS3W.exe 0x00F83B90: constructs the directory mutex then registers F925D0, returning atexit's status.
int InitDirectoryMutex();
#if defined(_M_IX86)
inline int32_t __fastcall HookUnsupported(void* s,void*) { return Unsupported(s); }
inline uint32_t __fastcall HookField0C(void* s,void*) { return Field0C(s); }
inline const uint64_t* __fastcall HookServiceId(void* s,void*) { return ServiceId(s); }
inline uint8_t __fastcall HookObserve(void* s,void*,void* a,void* b) { return Observe(s,a,b); }
inline uint32_t __fastcall HookField18(void* s,void*) { return Field18(s); }
inline void* __fastcall HookDestroy(void* s,void*,uint32_t f) { return Destroy(s,f); }
inline uint32_t __fastcall HookField74(void* s,void*) { return Field74(s); }
inline uint32_t __fastcall HookField78(void* s,void*) { return Field78(s); }
inline uint32_t __fastcall HookField08(void* s,void*) { return Field08(s); }
inline void* __fastcall HookStorage(void* s,void*) { return Storage(s); }
#endif
}
