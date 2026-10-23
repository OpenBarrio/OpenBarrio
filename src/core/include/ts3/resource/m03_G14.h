#pragma once
#include "ts3/layout.h"
#include "ts3/resource/m03_A06.h"
#include "ts3/resource/m03_A07.h"
namespace ts3::resource::m03::g14 {
struct Load {
 void* vtable; uint32_t reserved04[3]; uint32_t key[4]; uint32_t reserved20;
 void* record; void* resource; void* factory; uint32_t reserved30,offset,size,bytes;
 uint8_t reserved40[8]; a07::Request request; int32_t refs;
 uint8_t forceCopy,flag95,flag96,mode,flag98,pending,cancelled,wait,flag9c,flag9d,failed,pad9f;
 uint32_t reservedA0; void* requestOwner; void* job; void* child; int64_t timestamp;
 void* next; void* previous; a06::Context context;
};
struct Loader {
 uint8_t reserved00[0x40]; uint8_t mutex[0x30]; uint32_t reserved70;
 int32_t active,bytes;
};
struct Completion { uint8_t reserved00[0x10]; Load* load; };
struct Driver {
 void* vtable; void* path; uint32_t reserved08[2]; const char* prefix;
 uint8_t pool[0x24]; uint8_t mutex[0x30];
};
struct Handle { uint32_t offset,size; void* file; void* database; };
TS3_X86_OFFSET(Load,context,0xc0);
TS3_X86_OFFSET(Load,failed,0x9e);
TS3_X86_OFFSET(Loader,active,0x74);
TS3_X86_OFFSET(Driver,mutex,0x38);
// TS3W.exe 0x00728220: destroy context/request, release three owners, optionally free, return self.
Load* __fastcall DeleteLoad(Load*,void*,uint32_t);
// TS3W.exe 0x0072A150: finish an async read, copy/decode when required, and schedule its children; AL only.
uint8_t __fastcall Complete(Loader*,void*,Completion*,uint32_t);
// TS3W.exe 0x0072E520: open a database or fallback file and allocate a stream handle; stale output is retained on early failure.
Handle* __fastcall Open(Driver*,void*,const char*,uint32_t,void**);
// TS3W.exe 0x007338F0: destroy the change monitor and core-free only for flags bit zero, returning self.
void* __fastcall DeleteMonitor(void*,void*,uint32_t);
}
