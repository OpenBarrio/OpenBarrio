#pragma once

#include <cstddef>
#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
#define TS3_MISC_FASTCALL __fastcall
#define TS3_MISC_STDCALL __stdcall
#define TS3_MISC_CDECL __cdecl
#else
#define TS3_MISC_FASTCALL
#define TS3_MISC_STDCALL
#define TS3_MISC_CDECL
#endif

namespace ts3::misc {

// Cluster unit clu_4210a0_x. Every original address is named in the declaration comment;
// x86-only bodies abort on x64 through ts3::AbortNotRewritten, so the header stays a
// portable declaration of the game ABI.

// TS3W.exe 0x004210A0: tail thunk that forwards its single argument to 0x0041E8A0.
int32_t TS3_MISC_CDECL Dispatch4210A0(int32_t a1);
// TS3W.exe 0x004210B0: writes a script field; returns the object, the value or the
// context dispatcher's result depending on the index (EA quirk kept).
int32_t TS3_MISC_CDECL SetField4210B0(int32_t obj, int32_t index, int32_t value);
// TS3W.exe 0x004213B0: state-machine step (states 205/206/207/210) returning 1 when the
// secondary dispatcher reported an error, else 0.
int32_t TS3_MISC_CDECL Step4213B0(int32_t obj);
// TS3W.exe 0x004215E0: reads one block through the object's callback into a caller
// scratch slot, advances the position and returns the callback's out value (null on the
// overrun path).
int32_t* TS3_MISC_CDECL ReadBlock4215E0(int32_t obj, int32_t a2, int32_t a3);
// TS3W.exe 0x00421930: lazily allocates the object's 0x28-byte sub-object and fills its
// six handler pointers; returns the sub-object.
int32_t TS3_MISC_CDECL InitSub421930(int32_t obj, int32_t a2);
// TS3W.exe 0x0042D5E0: this-in-ECX with one stack out-buffer and caller-cleaned stack;
// writes out[0..2] and returns the third channel (255 on clamp). The rewrite takes its
// source in ECX too (fastcall) but pops the stack argument itself, so its only caller is
// the rewrite of 0x0042D700 and never the caller-cleaned original.
int32_t TS3_MISC_FASTCALL Shade42D5E0(const uint8_t* src, void* edx, uint8_t* out);
// TS3W.exe 0x0042D700: dst in EAX, src in EDX plus three stack arguments (private; only
// 0x0042DDA0 calls it), packed-pixel conversion. The rewrite spells the register inputs
// as ordinary parameters: nothing but the rewrite of 0x0042DDA0 calls it. On the mode==4
// path the original leaves its scratch dword's address in EAX with only the low byte
// replaced by the third channel; the rewrite returns that same shape from its own scratch
// dword, so only the low byte is comparable.
int32_t TS3_MISC_CDECL ConvertPixels42D700(uint8_t* dst, const char* src, int32_t size,
                                          int32_t mode, int32_t flag);
// TS3W.exe 0x0042DDA0: builds a 0x1D0-byte scripting context on the stack under a
// setjmp3 frame, allocates its buffer and converts the block rows; returns 1, or 0 after
// a longjmp.
int32_t TS3_MISC_STDCALL ConvertRegion42DDA0(int32_t a1, int32_t a2, uint8_t* a3, int32_t a4);
// TS3W.exe 0x004303C0: memcpy(dst, src, count << 7) (the shift wraps like EA's shl).
void* TS3_MISC_CDECL CopyShifted4303C0(const void* src, void* dst, int32_t count);
// TS3W.exe 0x00436630: memcpy(dst, src, size); the first argument is unused.
void* TS3_MISC_CDECL CopyPacked436630(int32_t unused, void* dst, const void* src, size_t size);
// TS3W.exe 0x004961F0: allocates the 572-byte stream object under an optional setjmp3
// frame and initializes it; returns the object (0 after a longjmp).
int32_t TS3_MISC_CDECL CreateStream4961F0(int32_t a1, int32_t a2, int32_t a3, int32_t* a4);
// TS3W.exe 0x004DE950: this-in-ECX + two stack args (ret 8); copies up to len bytes out
// of the window and returns the number copied.
int32_t TS3_MISC_FASTCALL ReadWindow4DE950(void* self, void* edx, void* dst, int32_t len);
// TS3W.exe 0x004DE990: this-in-ECX + two stack args (ret 8); copies into the window,
// grows its high-water mark and returns whether the whole len was copied.
int32_t TS3_MISC_FASTCALL WriteWindow4DE990(void* self, void* edx, const void* src, int32_t len);
// TS3W.exe 0x004E93E0: this-in-ECX + two stack args (ret 8); puts the source as a
// NUL-terminated UTF-16 clipboard string; returns the success byte.
uint8_t TS3_MISC_FASTCALL SetClipboardText4E93E0(void* self, void* edx, const void* src, int32_t len);
// TS3W.exe 0x004FAAD0: bounded UTF-8 aware write into the state's buffer; returns the
// bytes written (0 when the state is already overrun).
uint32_t TS3_MISC_CDECL WriteUtf8Bounded4FAAD0(const uint8_t* src, uint32_t size, int32_t state);
// TS3W.exe 0x006D5390: stdcall (ret 8) appends the source vector's dwords to the
// destination vector and returns the appended-to address.
void* TS3_MISC_STDCALL AppendVector6D5390(void* dstVec, const void* srcVec);
// TS3W.exe 0x00C2FD50: stdcall (ret 8) reads the MRTR record `index` of the renderer
// global 0x011D7530 into a 16-byte out buffer and returns it.
void* TS3_MISC_STDCALL ReadRecordC2FD50(void* out, int32_t index);
// TS3W.exe 0x00E7A300: copies one object element into the caller's buffer; the element
// stride comes from 0x00E6B690.
void* TS3_MISC_CDECL CopyElementE7A300(int32_t obj, int32_t count, void* src);
// TS3W.exe 0x00E7A330: copies a whole row of the object's inline array from another
// object; the element stride comes from 0x00E6B690.
void* TS3_MISC_CDECL CopyRowE7A330(int32_t a1, int32_t a2);
// TS3W.exe 0x00EB9040: memcpy(a3 + 2*a4 + 16, a1 + 2*a2 + 8, 2*a5).
void* TS3_MISC_CDECL CopySpanEB9040(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5);
// TS3W.exe 0x00EB9400: memcpy(dst, src, 2*count) for u16 elements.
void* TS3_MISC_CDECL CopyU16EB9400(const void* src, void* dst, int32_t count);
// TS3W.exe 0x00F1D6D0: reads a 16-byte header plus an optional u16-length payload from
// the current cursor; false when the remaining/needed sizes are inconsistent.
uint8_t TS3_MISC_CDECL ReadChunkF1D6D0(int32_t* srcPtr, int32_t* avail, int32_t size, void* headerOut,
                                       void* dst, int32_t cap);
// TS3W.exe 0x00F4B720: allocation hook driving the object's +0x4B4/+0x4B8 callbacks (or
// malloc/free); returns the new block (0 on the free-only path).
int32_t TS3_MISC_CDECL AllocHookF4B720(int32_t obj, void* src, int32_t size, int32_t copySize);
// TS3W.exe 0x00F7E8A6: the MSVCR80 memcpy import thunk (jmp through IAT 0x00F95654).
void* TS3_MISC_CDECL MemcpyF7E8A6(void* dst, const void* src, size_t size);
// TS3W.exe 0x00F7EE92: the MSVCR80 _setjmp3 import thunk (jmp through IAT 0x00F956D8).
int32_t TS3_MISC_CDECL Setjmp3F7EE92(void* env, int32_t count);

// The protected scripting-slot wrappers (0x00DA2780 .. 0x00DE3A60). a1 is never read,
// a2 is the error out, a3 the context whose +0x10 slot holds the active setjmp frame,
// a4 the result out; the handles/values are the remaining arguments.
int32_t TS3_MISC_CDECL SlotValueDA2780(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDA3110(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDAA260(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDAF9D0(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDAFD60(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDB2BB0(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDB5880(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDB73D0(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6, int32_t a7);
int32_t TS3_MISC_CDECL SlotValueDBB270(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDBE9E0(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDC2380(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDC4320(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDD3190(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);
int32_t TS3_MISC_CDECL SlotValueDE1760(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5);
int32_t TS3_MISC_CDECL SlotValueDE3A60(int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6);

}  // namespace ts3::misc