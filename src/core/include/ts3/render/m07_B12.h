#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::m07::b12 {
// Batch-local refinement: scaffold opaque links and format storage are pointers.
struct Links { void* table; Links* next; Links* previous; };
struct Element { uint16_t stream,offset; uint8_t type,method,usage,index; };
struct Format { void* primary; uint32_t refs; Links secondary; void* declaration; Element* elements; int32_t count; uint32_t* strides; };
struct Stream { void* primary; uint32_t refs; Links secondary; void* buffer; uint32_t size,flags; };
struct Cache { void* opaque00; void** buckets; uint32_t bucketCount; uint8_t opaque0c[0x14]; const char* tag; };
TS3_X86_OFFSET(Format,declaration,0x14); TS3_X86_OFFSET(Format,strides,0x20);
TS3_X86_OFFSET(Stream,flags,0x1c); TS3_X86_OFFSET(Cache,tag,0x20);
// TS3W.exe 0x0061A010: copy elements, compute maximum stream strides and create a D3D declaration.
uint8_t __fastcall CreateFormat(Format*,void*,const Element*,int32_t);
// TS3W.exe 0x0061A1C0: create a vertex buffer; preserve partially written fields on failure.
uint8_t __fastcall CreateStream(Stream*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x0061A220: unregister and release a vertex stream.
uint32_t __fastcall ReleaseStream(Stream*,void*);
// TS3W.exe 0x0061A240: lock, copy and unlock a bounded vertex stream, ignoring unlock failure.
uint8_t __fastcall Upload(Stream*,void*,const void*,uint32_t);
// TS3W.exe 0x0061A2A0: lazily create the adjusted stream; only AL is changed on the cached path.
uint32_t __fastcall PollStream(Links*,void*);
// TS3W.exe 0x0061A2F0: release dynamic streams through an adjusted receiver; otherwise preserve EAX.
uint32_t __fastcall NotifyStream(Links*,void*);
// TS3W.exe 0x0061A300: initialize vertex format interfaces, leaving stride pointer untouched.
Format* __fastcall ConstructFormat(Format*,void*);
// TS3W.exe 0x0061A340: destroy a vertex format and optionally free it.
void* __fastcall DeleteFormat(Format*,void*,uint32_t);
// TS3W.exe 0x0061A380: initialize stream interfaces, leaving flags untouched.
Stream* __fastcall ConstructStream(Stream*,void*);
// TS3W.exe 0x0061A3C0: adjust the secondary stream receiver and invoke deleting destruction.
void* __fastcall DeleteAdjusted(Links*,void*,uint32_t);
// TS3W.exe 0x0061A3D0: destroy a stream and restore its base resource table.
uint32_t __fastcall DestructStream(Stream*,void*);
// TS3W.exe 0x0061A410: destroy a stream and optionally free it (00616F70 twin).
void* __fastcall DeleteStream(Stream*,void*,uint32_t);
// TS3W.exe 0x0061A9B0: look up or create a retained cached vertex format.
uint8_t __fastcall GetFormat(Cache*,void*,const Element*,int32_t,Format**);
// TS3W.exe 0x0061AC10: construct the vertex format cache and install its allocation tag.
Cache* __fastcall ConstructCache(Cache*,void*);
}
