#pragma once

#include <cstdint>

namespace ts3::usertool {

// TS3W.exe 0x00BFB990: __thiscall (this, a2..a6), ret 0x14. C1 constructor of
// UserTool/AppendRoofWallLocator/AddDiagonalShedWallLocators: forwards the five arguments unchanged
// to the base constructor 0x00BFADB0, then installs vtable 0x01073240 and returns this.
void* __fastcall AddDiagonalShedWallLocatorsCtor(void* self, void* edx, void* a2, void* a3, void* a4,
                                                 void* a5, void* a6);

// TS3W.exe 0x00C035A0: __thiscall (this, mode), ret 4. Vtable slot: calls
// 0x00BFE0B0(this, 0, mode, scratch) with a 16-byte UNINITIALIZED stack block and returns its
// result unchanged. No callers in the image; reached only through the class vtable.
int32_t __fastcall AddDiagonalShedWallLocatorsGetWallLocatorData(void* self, void* edx, int32_t mode);

// TS3W.exe 0x00CC0D70: __thiscall (locator, record, gridX), ret 8 - only the argument names match
// the disassembly; `locator` is the intrusive-refcounted locator that 0x00CC1D60 resolved, and
// record is that same object re-read as a locator record (0x00CC0D70 is the second switch's
// type 12/14/25 target). Allocates a new 228 (record types 12/25) or 244 (type 14) byte locator
// from the game allocator, constructs it through 0x00BFADB0 / 0x00BFAED0 / 0x00BFB990, hands it to
// 0x00A48900, makes the placement call through the locator's own vtable slot 1 and releases it.
// Other record types return (record[0x2c] - 25) and touch nothing else.
int32_t __fastcall AppendRoofWallLocator(void* locator, void* edx, const void* record, int32_t gridX);

// TS3W.exe 0x00CC1D60: __thiscall (handlerBlock, id), ret 4. Resolves the object through
// 0x00B974B0 / 0x00B971F0 (handler = handlerBlock[0xc]), dispatches by its type through the two
// jump tables at 0x00CC1EF4/0x00CC1F00 and 0x00CC1F1C/0x00CC1F28 and releases it. handlerBlock is
// the four-dword block {a4 (used as the locator vtable), a5, a6, a7 (the handler)} that the two
// drivers below build from their arguments. EAX on return is an artifact both drivers discard.
void __fastcall ProcessWallLocator(void* handlerBlock, void* edx, uint32_t id);

// TS3W.exe 0x00CC1F40: __cdecl (out, first, last, a4, a5, a6, a7). Runs 0x00CC1D60 for every dword
// element of [first,last) and fills the 16-byte result {0x0107DDFC, a5, a6, a7}; returns out.
void* __cdecl ProcessRoofLocatorRange(void* out, const uint32_t* first, const uint32_t* last,
                                      void* a4, void* a5, void* a6, void* a7);

// TS3W.exe 0x00CC1F90: __cdecl (out, first, last, a4, a5, a6, a7, a8). Same over an intrusive list
// stepped by 0x004E0B20, reading node+0x10; fills the 20-byte result
// {0x0107DE04, a5, a6, a7, a8}; returns out.
void* __cdecl ProcessRoofLocatorList(void* out, uintptr_t first, uintptr_t last, void* a4, void* a5,
                                     void* a6, void* a7, void* a8);

// Native-pointer layouts of the temporary EA containers (x86 offsets checked in the source).
struct RoofIds { uint32_t* first; uint32_t* last; uint32_t* capacity; };
struct RoofRects { float* first; float* last; float* capacity; const char* name; };
// TS3W.exe 0x00CC4EA0: collect roof rectangles and insert overlapping candidate ids into the set.
RoofIds* __cdecl CollectRoofOverlaps(void* tool, RoofIds* roofs, const RoofIds* candidates,
                                    void* selected, RoofRects* rectangles);
// TS3W.exe 0x00CC68B0: validate and apply roof placement, retaining EA's cleanup order.
uint8_t __cdecl PlaceRoofs(void* tool, RoofIds* roofs, const RoofIds* candidates, void* remove,
                           void* walls, void* placement, uint32_t mode, uint32_t flags,
                           uint32_t extra);

}  // namespace ts3::usertool
