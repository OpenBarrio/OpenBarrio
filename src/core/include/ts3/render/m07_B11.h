#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render {
// Sparse secondary interface: offsets are relative to the original object's +08.
struct B11Interface {
    void* vtable;
    uint32_t registration4, registration8;
    uint32_t flags, width, height, format;
    void* handle;
};
struct B11Buffer { void* vtable; uint32_t refs; B11Interface iface; };
TS3_X86_OFFSET(B11Buffer,iface,8);
TS3_X86_OFFSET(B11Buffer,iface.handle,0x24);
struct B11Rect { int32_t left,top,right,bottom; };
// TS3W.exe 0x00617C10: forward to buffer cleanup (617960).
void B11Cleanup(B11Buffer*);
// TS3W.exe 0x00617C20: adopt a surface and derive its descriptor (617A70).
void B11Adopt(B11Buffer*,void*,uint32_t);
// TS3W.exe 0x00617C30: create the surface selected by format and flags; AL return.
bool B11Create(B11Interface*);
// TS3W.exe 0x00617CB0: destroy the base buffer interfaces without freeing storage.
void B11Destroy(B11Buffer*);
// TS3W.exe 0x00617DE0: copy/stretch surfaces with signed rectangle checks; AL return.
bool B11Copy(B11Buffer*,const B11Rect*,B11Buffer*,const B11Rect*,uint8_t);
// TS3W.exe 0x00617F70: adjust secondary receiver by -08 and delete derived buffer.
B11Buffer* B11DeleteAdjusted(B11Interface*,uint32_t);
// TS3W.exe 0x00617F80: compute system-memory bytes with wrapping multiplication.
uint32_t B11SystemBytes(B11Interface*);
// TS3W.exe 0x00617FB0: compute video-memory bytes with wrapping multiplication.
uint32_t B11VideoBytes(B11Interface*);
// TS3W.exe 0x00617FE0: initialize base buffer, leaving unspecified fields untouched.
B11Buffer* B11Construct(B11Buffer*);
// TS3W.exe 0x00618020: destroy base buffer and optionally free it (flags bit zero).
B11Buffer* B11Delete(B11Buffer*,uint32_t);
// TS3W.exe 0x00618060: initialize derived buffer and its adjusted interface.
B11Buffer* B11ConstructDerived(B11Buffer*);
// TS3W.exe 0x006180A0: clean derived then base buffer (two cleanup calls), optionally free.
B11Buffer* B11DeleteDerived(B11Buffer*,uint32_t);
// TS3W.exe 0x00619FD0: destroy vertex-format interfaces without freeing storage.
void B11FormatDestroy(m07::VertexFormat*);
// TS3W.exe 0x0061A000: adjust vertex-format secondary receiver by -08 and delete.
void* B11FormatDeleteAdjusted(void*,uint32_t);
}

#if defined(_M_IX86)
namespace ts3::render {
// TS3W.exe 0x00617C10: ABI adapter for B11Cleanup.
inline void __fastcall Hook00617C10(B11Buffer* p, void*) { return B11Cleanup(p); }
// TS3W.exe 0x00617C20: ABI adapter for B11Adopt.
inline void __fastcall Hook00617C20(B11Buffer* p, void*, void* s, uint32_t t) { return B11Adopt(p,s,t); }
// TS3W.exe 0x00617C30: ABI adapter for B11Create.
inline bool __fastcall Hook00617C30(B11Interface* p, void*) { return B11Create(p); }
// TS3W.exe 0x00617CB0: ABI adapter for B11Destroy.
inline void __fastcall Hook00617CB0(B11Buffer* p, void*) { return B11Destroy(p); }
// TS3W.exe 0x00617DE0: ABI adapter for B11Copy.
inline bool __fastcall Hook00617DE0(B11Buffer* p, void*, const B11Rect* d, B11Buffer* s, const B11Rect* r, uint8_t f) { return B11Copy(p,d,s,r,f); }
// TS3W.exe 0x00617F70: ABI adapter for B11DeleteAdjusted.
inline B11Buffer* __fastcall Hook00617F70(B11Interface* p, void*, uint32_t f) { return B11DeleteAdjusted(p,f); }
// TS3W.exe 0x00617F80: ABI adapter for B11SystemBytes.
inline uint32_t __fastcall Hook00617F80(B11Interface* p, void*) { return B11SystemBytes(p); }
// TS3W.exe 0x00617FB0: ABI adapter for B11VideoBytes.
inline uint32_t __fastcall Hook00617FB0(B11Interface* p, void*) { return B11VideoBytes(p); }
// TS3W.exe 0x00617FE0: ABI adapter for B11Construct.
inline B11Buffer* __fastcall Hook00617FE0(B11Buffer* p, void*) { return B11Construct(p); }
// TS3W.exe 0x00618020: ABI adapter for B11Delete.
inline B11Buffer* __fastcall Hook00618020(B11Buffer* p, void*, uint32_t f) { return B11Delete(p,f); }
// TS3W.exe 0x00618060: ABI adapter for B11ConstructDerived.
inline B11Buffer* __fastcall Hook00618060(B11Buffer* p, void*) { return B11ConstructDerived(p); }
// TS3W.exe 0x006180A0: ABI adapter for B11DeleteDerived.
inline B11Buffer* __fastcall Hook006180A0(B11Buffer* p, void*, uint32_t f) { return B11DeleteDerived(p,f); }
// TS3W.exe 0x00619FD0: ABI adapter for B11FormatDestroy.
inline void __fastcall Hook00619FD0(m07::VertexFormat* p, void*) { return B11FormatDestroy(p); }
// TS3W.exe 0x0061A000: ABI adapter for B11FormatDeleteAdjusted.
inline void* __fastcall Hook0061A000(void* p, void*, uint32_t f) { return B11FormatDeleteAdjusted(p,f); }
}
#endif
