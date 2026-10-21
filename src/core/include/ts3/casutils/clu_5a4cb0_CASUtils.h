// CASUtils script-bridge cluster from TS3W.exe 0x005A4CB0..0x00774E00 (40 functions).
//
// Every 0x005A* entry is a __cdecl script bridge: arg1 is a script handle whose high byte
// must be 0x35; the Scripting singleton (0x007692B0, returns the pointer stored at 0x011D4BBC) resolves
// it through a vtable lookup (slot 51 / offset 0xCC, slot 54 / 0xD8, or slot 50 / 0xC8) and
// the bridge then reads or writes one field of the returned game object. The setters are
// the only bridges that change game state.
#pragma once

#include <cstdint>

namespace ts3::casutils {

// 0x007692B0, __cdecl, ret 0. The Scripting singleton getter (the pointer stored at 0x011D4BBC),
// returned as the raw EAX dword (a pointer value in practice).
uint32_t GetScriptingSingleton7692B0();

// 0x005A4CB0, __cdecl, ret 0. Reads the float at object+0x268, 0.0f without a 0x35 handle.
float SimBuilderSetCurlPixelRadius5A4CB0(uint32_t handle);
// 0x005A4CF0, __cdecl, ret 0. Reads the float at object+0x26C, 0.0f without a 0x35 handle.
float SimBuilderGetFurMap5A4CF0(uint32_t handle);
// 0x005A4D30, __cdecl, ret 0. Reads the float at object+0x7C, 0.0f without a 0x35 handle.
float SimOutfitSetMorphWeightD5A4D30(uint32_t handle);
// 0x005A4DE0, __cdecl, ret 0. Reads the float at object+0x80, 0.0f without a 0x35 handle.
float SimOutfitGetFurMap5A4DE0(uint32_t handle);
// 0x005A4E20, __cdecl, ret 0. Writes the float at object+0x7C. EA leaves EAX = the object
// pointer, or the masked handle on the tag-mismatch path; both are returned.
void* SimOutfitGetCurlPixelRadius5A4E20(uint32_t handle, float value);
// 0x005A5120, __cdecl, ret 0. Looks the object up, then returns AL = 1 unconditionally.
uint8_t CreatePartSearch5A5120(uint32_t handle);
// 0x005A5790, __cdecl, ret 0. Resets the object's animation through the private helper
// 0x005E0190 (cached handle at +0x1EC, created object at +0x1F0); 0 without an object.
uint32_t SimBuilderResetAnimation5A5790(uint32_t handle);
// 0x005A57D0, __cdecl, ret 0. Reads the dword at object+0x64, 0 without a 0x35 handle.
uint32_t SimBuilderGetModelHandle5A57D0(uint32_t handle);
// 0x005A5960, __cdecl, ret 0. Reads the float at object+0x50, 0.0f without a 0x35 handle.
float SimBuilderSetMorphWeightD5A5960(uint32_t handle);
// 0x005A59A0, __cdecl, ret 0. Reads the float at object+0x54, 0.0f without a 0x35 handle.
float SimBuilderGetMorphWeightA5A59A0(uint32_t handle);
// 0x005A59E0, __cdecl, ret 0. Reads the float at object+0x58, 0.0f without a 0x35 handle.
float SimBuilderGetMorphWeightB5A59E0(uint32_t handle);
// 0x005A5A20, __cdecl, ret 0. Reads the float at object+0x5C, 0.0f without a 0x35 handle.
float SimBuilderGetMorphWeightC5A5A20(uint32_t handle);
// 0x005A5EA0, __cdecl, ret 0. Returns EAX: the low byte is the object+0x69 byte, the
// upper 24 bits keep whatever the object pointer or the AND result left there, because
// the original only writes the low byte. 0 when the lookup misses.
uint32_t SimBuilderSetSelectionIndex5A5EA0(uint32_t handle);
// 0x005A5F30, __cdecl, ret 0. Writes the byte at object+0x6B; EAX contract as 0x005A4E20.
void* SimBuilderSetUseCompression5A5F30(uint32_t handle, uint8_t value);
// 0x005A5F70, __cdecl, ret 0. EAX contract as 0x005A5EA0, object+0x6A.
uint32_t SimBuilderSetCacheAtlases5A5F70(uint32_t handle);
// 0x005A5FB0, __cdecl, ret 0. Writes the byte at object+0x6A; EAX contract as 0x005A4E20.
void* SimBuilderGetWaitForTextures5A5FB0(uint32_t handle, uint8_t value);
// 0x005A6030, __cdecl, ret 0. EAX contract as 0x005A5EA0, object+0x204.
uint32_t SimBuilderGetIsDirty5A6030(uint32_t handle);
// 0x005A6220, __cdecl, ret 0. Tail-calls the game method 0x005E10B0 on the object (game
// unit clu_56dc40_CAS); the masked handle on a bad tag, 0 on a lookup miss.
uint32_t SimBuilderGetPickDataFromBodyTypeAndRegion5A6220(uint32_t handle);
// 0x005A62B0, __cdecl, ret 0. Reads the dword at object+0x38, 0 without a 0x35 handle.
uint32_t FacialBlendGetName5A62B0(uint32_t handle);
// 0x005A63B0, __cdecl, ret 0. EAX contract as 0x005A5EA0, object+0x50.
uint32_t BlendUnitGetLocalizationKey5A63B0(uint32_t handle);
// 0x005A63F0, __cdecl, ret 0. Reads the dword at object+0x60, 0 without a 0x35 handle.
uint32_t BlendUnitIsBidirectional5A63F0(uint32_t handle);
// 0x005A64A0, __cdecl, ret 0. Reads the dword at object+0x54, 0 without a 0x35 handle.
uint32_t BlendUnitGetBlendKeyAtIndex5A64A0(uint32_t handle);
// 0x005A64E0, __cdecl, ret 0. Reads the dword at object+0x58, 0 without a 0x35 handle.
uint32_t BlendUnitGetRegion5A64E0(uint32_t handle);
// 0x005A6520, __cdecl, ret 0. Reads the dword at object+0x5C, 0 without a 0x35 handle.
uint32_t BlendUnitGetCategory5A6520(uint32_t handle);
// 0x005A6560, __cdecl, ret 0. Reads the dword at object+0x3C, 0 without a 0x35 handle.
uint32_t CompoundBlendGetAgeGender5A6560(uint32_t handle);
// 0x005A65A0, __cdecl, ret 0. Looks the object up through vtable slot 54 (not 51) and
// tail-calls its vtable slot 1; the masked handle on a bad tag, 0 on a lookup miss.
uint32_t FacialBlendGetRegionFlags5A65A0(uint32_t handle);
// 0x005A69D0, __cdecl, ret 0. Reads the float at object+0x50, 0.0f without a 0x35 handle.
float SimOutfitGetSkinToneKey5A69D0(uint32_t handle);
// 0x005A6AC0, __cdecl, ret 0. Reads the float at object+0x68, 0.0f without a 0x35 handle.
float SimOutfitGetRigKey5A6AC0(uint32_t handle);
// 0x005A6B00, __cdecl, ret 0. Writes the float at object+0x68; EAX contract as 0x005A4E20.
void* SimOutfitGetMorphWeightD5A6B00(uint32_t handle, float value);
// 0x005A6B40, __cdecl, ret 0. Reads the float at object+0x6C, 0.0f without a 0x35 handle.
float SimOutfitGetMorphWeightA5A6B40(uint32_t handle);
// 0x005A6B80, __cdecl, ret 0. Writes the float at object+0x6C; EAX contract as 0x005A4E20.
void* SimOutfitSetMorphWeightA5A6B80(uint32_t handle, float value);
// 0x005A6BC0, __cdecl, ret 0. Reads the float at object+0x70, 0.0f without a 0x35 handle.
float SimOutfitGetMorphWeightB5A6BC0(uint32_t handle);
// 0x005A6C00, __cdecl, ret 0. Writes the float at object+0x70; EAX contract as 0x005A4E20.
void* SimOutfitSetMorphWeightB5A6C00(uint32_t handle, float value);
// 0x005A6C40, __cdecl, ret 0. Reads the float at object+0x74, 0.0f without a 0x35 handle.
float SimOutfitGetMorphWeightC5A6C40(uint32_t handle);
// 0x005A6C80, __cdecl, ret 0. Writes the float at object+0x74; EAX contract as 0x005A4E20.
void* SimOutfitSetMorphWeightC5A6C80(uint32_t handle, float value);

// 0x005E0190, __thiscall (this in ecx), ret 0. Private helper of 0x005A5790; not a public
// entry point. Disassemblers group it with 0x005A5790, but it is a separate function.
uint32_t GetCachedScriptObject5E0190(void* self);

// 0x0076A4B0, __thiscall (this in ecx), ret 8. Calls the singleton getter and returns EAX
// with AL forced to 1: the upper 24 bits still hold the manager pointer.
uint32_t __fastcall ScriptingReturnsTrue76A4B0(void* self, void* reservedEdx, int32_t a, int32_t b);

// 0x0076CCA0, __cdecl, ret 0. manager->vtbl[95]() then ->vtbl[7]() (no null check, like
// the original: a null first result would fault).
uint32_t CheckPickData76CCA0();
// 0x00774E00, __cdecl, ret 0. Allocates a 0x60-byte script object through manager
// vtbl slot 50, then calls arg->vtbl[1]() and returns the allocated object (or 0).
void* CreateAndNotify774E00(void* arg);

}  // namespace ts3::casutils