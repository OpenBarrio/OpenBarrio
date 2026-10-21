#pragma once
#include "ts3/layout.h"
namespace ts3::scratch {
// These functions preserve allocator EAX, including cleanup paths that do not free.
// TS3W.exe 0x004E3F70: null-safe free through the allocator at 0x011CB864.
void* Free4E3F70(void* block);
struct DeletingObject {
    void* vtable;
    // TS3W.exe 0x004E8B80: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete004E8B80(uint32_t flags);
    // TS3W.exe 0x004F27E0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete004F27E0(uint32_t flags);
    // TS3W.exe 0x004F28D0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete004F28D0(uint32_t flags);
    // TS3W.exe 0x00502D50: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00502D50(uint32_t flags);
    // TS3W.exe 0x00502DC0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00502DC0(uint32_t flags);
    // TS3W.exe 0x00502DE0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00502DE0(uint32_t flags);
    // TS3W.exe 0x00505D80: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00505D80(uint32_t flags);
    // TS3W.exe 0x00576C90: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00576C90(uint32_t flags);
    // TS3W.exe 0x0058DA90: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0058DA90(uint32_t flags);
    // TS3W.exe 0x0058F7E0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0058F7E0(uint32_t flags);
    // TS3W.exe 0x00599140: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00599140(uint32_t flags);
    // TS3W.exe 0x0059FF90: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0059FF90(uint32_t flags);
    // TS3W.exe 0x0060B990: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0060B990(uint32_t flags);
    // TS3W.exe 0x00613F60: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00613F60(uint32_t flags);
    // TS3W.exe 0x006D45D0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete006D45D0(uint32_t flags);
    // TS3W.exe 0x0070BE90: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0070BE90(uint32_t flags);
    // TS3W.exe 0x00728740: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00728740(uint32_t flags);
    // TS3W.exe 0x0076AD90: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0076AD90(uint32_t flags);
    // TS3W.exe 0x007B2300: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete007B2300(uint32_t flags);
    // TS3W.exe 0x007C8C20: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete007C8C20(uint32_t flags);
    // TS3W.exe 0x007CBAD0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete007CBAD0(uint32_t flags);
    // TS3W.exe 0x007DA2B0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete007DA2B0(uint32_t flags);
    // TS3W.exe 0x007FE330: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete007FE330(uint32_t flags);
    // TS3W.exe 0x00800070: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00800070(uint32_t flags);
    // TS3W.exe 0x0081CCB0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete0081CCB0(uint32_t flags);
    // TS3W.exe 0x00877350: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00877350(uint32_t flags);
    // TS3W.exe 0x009C75C0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009C75C0(uint32_t flags);
    // TS3W.exe 0x009D7610: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009D7610(uint32_t flags);
    // TS3W.exe 0x009D7630: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009D7630(uint32_t flags);
    // TS3W.exe 0x009EF360: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009EF360(uint32_t flags);
    // TS3W.exe 0x009EFFF0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009EFFF0(uint32_t flags);
    // TS3W.exe 0x009F1FC0: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete009F1FC0(uint32_t flags);
    // TS3W.exe 0x00C25B20: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00C25B20(uint32_t flags);
    // TS3W.exe 0x00EED600: destroy, optionally free when flags bit 0 is set; return this.
    void* Delete00EED600(uint32_t flags);
};
TS3_X86_OFFSET(DeletingObject, vtable, 0);
TS3_X86_SIZE(DeletingObject, 4);
struct ScratchSpan { void* begin; void* end; void* limit; };
TS3_X86_OFFSET(ScratchSpan, limit, 8);
TS3_X86_SIZE(ScratchSpan, 12);
struct Cleanup87F5A0 { uint8_t prefix[0x20]; ScratchSpan span;
// TS3W.exe 0x0087F5A0: free begin if signed byte capacity, with bit 0 cleared, exceeds two.
void* Release(); };
TS3_X86_OFFSET(Cleanup87F5A0, span, 0x20);
TS3_X86_SIZE(Cleanup87F5A0, 0x2C);
struct Cleanup883A80 { void* prefix; ScratchSpan span;
// TS3W.exe 0x00883A80: same masked capacity cleanup at offset four.
void* Release(); };
TS3_X86_OFFSET(Cleanup883A80, span, 4);
TS3_X86_SIZE(Cleanup883A80, 16);
struct CleanupC16FD0 { void* block;
// TS3W.exe 0x00C16FD0: free the stored pointer without clearing it.
void* Release(); };
TS3_X86_SIZE(CleanupC16FD0, 4);
struct ExternalStorage { void* prefix; void* block; void* unused[3]; void* internal; };
TS3_X86_OFFSET(ExternalStorage, block, 4);
TS3_X86_OFFSET(ExternalStorage, internal, 0x14);
TS3_X86_SIZE(ExternalStorage, 0x18);
#if defined(_M_IX86)
#define TS3_SCRATCH_STDCALL __stdcall
#else
#define TS3_SCRATCH_STDCALL
#endif
// TS3W.exe 0x00945440: stdcall ret 4, frees when signed byte capacity exceeds one.
void* TS3_SCRATCH_STDCALL Release945440(ScratchSpan* span);
// TS3W.exe 0x00945A30: stdcall ret 4, frees non-null storage unless it equals the inline buffer.
void* TS3_SCRATCH_STDCALL Release945A30(ExternalStorage* storage);
} // namespace ts3::scratch
