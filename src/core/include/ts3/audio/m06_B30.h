#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b30 {
struct Queue {
 uint8_t* begin; uint8_t* end; uint8_t* storageEnd; const char* label;
 void* transient[6]; uint32_t unknown28;
 uint8_t* cursor; uint8_t* read; uint8_t* record; uint8_t* length; uint8_t* count;
 uint32_t unknown40, capacity, items, unknown4c, limit;
#if !defined(_M_IX86)
 void (*nativeResize)(Queue*,uint32_t)=nullptr;
#endif
};
TS3_X86_OFFSET(Queue,cursor,0x2c);
TS3_X86_OFFSET(Queue,limit,0x50);
// TS3W.exe 0x0089FD60: resize storage and relocate five cursors using signed offsets.
uintptr_t __fastcall F89FD60(Queue*,void*,uint32_t);
// TS3W.exe 0x0089FEB0: append aligned bytes, retaining the short-payload four-byte read quirk.
uintptr_t __fastcall F89FEB0(Queue*,void*,const void*,uint32_t);
// TS3W.exe 0x0089FFC0: serialize a type-zero four-byte parameter and increment record count.
uintptr_t __fastcall F89FFC0(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A0020: serialize a type-one four-byte parameter.
uintptr_t __fastcall F8A0020(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A0080: serialize a type-two eight-byte parameter.
uintptr_t __fastcall F8A0080(Queue*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x008A00E0: serialize a type-four four-byte parameter.
uintptr_t __fastcall F8A00E0(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A0140: resize and reset the queue; AL becomes one, upper EAX retains base bits.
uintptr_t __fastcall F8A0140(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A01A0: roll back an unfinished record and start a new record.
uintptr_t __fastcall F8A01A0(Queue*,void*,uint32_t);
// TS3W.exe 0x008A0210: round a float through x87 and serialize type zero.
uintptr_t __fastcall F8A0210(Queue*,void*,uint32_t,float);
// TS3W.exe 0x008A0230: tail-forward to the type-one serializer (verified thunk twin pattern).
uintptr_t __fastcall F8A0230(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A0240: forward an eight-byte parameter to the type-two serializer.
uintptr_t __fastcall F8A0240(Queue*,void*,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x008A0260: serialize length-plus-terminator bytes as type three.
uintptr_t __fastcall F8A0260(Queue*,void*,uint32_t,const char*,uint32_t);
// TS3W.exe 0x008A02E0: tail-forward to the type-four serializer (verified thunk twin pattern).
uintptr_t __fastcall F8A02E0(Queue*,void*,uint32_t,uint32_t);
// TS3W.exe 0x008A02F0: serialize a caller-sized type-five payload.
uintptr_t __fastcall F8A02F0(Queue*,void*,uint32_t,const void*,uint32_t);
// TS3W.exe 0x008A0350: serialize three consecutive four-byte words as type six.
uintptr_t __fastcall F8A0350(Queue*,void*,uint32_t,const void*);
}
