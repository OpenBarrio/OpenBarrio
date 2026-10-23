// Bounded A10 caller contracts; no hooks, algorithm/address credit or full driver layout.
#pragma once
#include "ts3/resource/m03_layouts.h"
#include "ts3/resourcesystem/clu_4a3640_ResourceSystem.h"

namespace ts3::resource::m03::catalog_driver {
// 004016F0 allocates 0x14 on x86. This byte-name node is NOT PathString (UTF16).
struct NameNode { NameNode* next; char name[16]; };
// Used base prefix only: never a complete driver allocation-size declaration.
struct DriverView { const void* table; NameNode* first; NameNode* last; uint32_t count; };
TS3_X86_SIZE(NameNode,0x14);
TS3_X86_OFFSET(DriverView,first,4);
TS3_X86_OFFSET(DriverView,last,8);
TS3_X86_OFFSET(DriverView,count,0xc);
// Whole-operation native providers. These are host services, never game vtables.
// All receivers, inputs and returned identities are forwarded without ownership changes.
// The provider owns any borrowed-input/callback lifetime and must drain before teardown.
struct NameAdapter {
    void* context=nullptr;
    DriverView* (*construct)(void*,DriverView*,const char*)=nullptr;
};
struct OpenAdapter {
    void* context=nullptr;
    void* (*open)(void*,DriverView*,const char*,uint32_t,uint32_t)=nullptr;
};
struct PoolAdapter {
    void* context=nullptr;
    uint32_t (*configure)(void*,void*)=nullptr;
};
// Whole-operation E2C0 native boundary: no portable EAX result or layout policy.
struct PublicationAdapter {
    void* context=nullptr;
    void (*publish)(void*,void* completeDriver,void* record)=nullptr;
};
// Whole-operation lifetime boundaries; provider owns all native lifecycle policy.
struct DriverConstructionAdapter {
    void* context=nullptr;
    void* (*construct)(void*,void* completeDriver,const char* name,void* argument)=nullptr;
};
struct DriverDestructionAdapter {
    void* context=nullptr;
    void* (*destroy)(void*,void* completeDriver,uint32_t flags)=nullptr;
};
// Whole callback operation only: provider owns all native lifetime policy.
struct WrapperCallbackAdapter {
    void* context=nullptr;
    void (*callback)(void*,void* wrapper)=nullptr;
};
// Whole lower free boundary; service is an opaque native identity, never a game vtable.
struct AllocatorFreeAdapter {
    void* context=nullptr;
    void* service=nullptr;
    uintptr_t (*free)(void*,void* service,void* wrapper)=nullptr;
};
uintptr_t FreeWrapperStorage(void* wrapper,const AllocatorFreeAdapter&);
void ReleaseWrapper(void* wrapper,const WrapperCallbackAdapter&);
void* ConstructDriver(void*,const char*,void*,const DriverConstructionAdapter&);
void* DestroyDriver(void*,uint32_t,const DriverDestructionAdapter&);
void PublishRecord(void* completeDriver,void* record,const PublicationAdapter&);
// Native whole-operation forwarding only; missing providers throw, never use x86 addresses.
DriverView* ConstructName(DriverView*,const char*,const NameAdapter&);
void* Open(DriverView*,const char*,uint32_t,uint32_t,const OpenAdapter&);
uint32_t ConfigurePool(void* completeDriver,const PoolAdapter&);
#if defined(_M_IX86)
// TS3W.exe 0x00401F10: cdecl pointer, slot3(pointer,0) ret8; full incidental EAX.
uintptr_t FreeWrapperStorage(void* wrapper);
// TS3W.exe 0x0072E200: cdecl nullable wrapper, caller pops4; incidental EAX.
void ReleaseWrapper(void* wrapper);
// TS3W.exe 0x0072E240: complete receiver, name/opaque argument, ret8, full self.
void* ConstructDriver(void*,const char*,void*);
// TS3W.exe 0x0072E470: complete receiver, flags ret4, low-byte bit0 free, saved self.
void* DestroyDriver(void*,uint32_t);
// TS3W.exe 0x0072E2C0: ECX driver, nullable record stack argument, ret4; EAX incidental.
void PublishRecord(void* completeDriver,void* record);
// TS3W.exe 0x0072E220: original ECX driver, name stack arg, ret4, full EAX self.
DriverView* ConstructName(DriverView*,const char*);
// TS3W.exe 0x0072E370: original ECX driver, path/flags/unused, ret12, pointer/zero.
void* Open(DriverView*,const char*,uint32_t,uint32_t);
// TS3W.exe 0x0072E290: original ECX complete driver, no stack args, full residual EAX.
uint32_t ConfigurePool(void* completeDriver);
#endif
}
namespace exehost { class ExeImage; }
namespace exehost::m03::catalog_driver {
struct Result { unsigned long long cases,failures; };
Result RunNative();
}
