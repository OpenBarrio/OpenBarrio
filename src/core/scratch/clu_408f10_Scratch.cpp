// TS3W.exe cluster unit clu_408f10_Scratch rewritten for x86/x64 (see the header for the
// per-function contracts). Every function keeps its original address in the comment above
// it; dependencies that are not rewritten yet go through the bridge in layout.h.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ts3/scratch/clu_408f10_Scratch.h"
#include "ts3/scratch/clu_408e20_Scratch.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"

#include <cstdint>
#include <cstring>

namespace ts3::scratch {
namespace {

// The game's static empty wide string, the generic one-past-end capacity sentinel, and the
// distinct 0x008A7870 literal (which is 0x011C7E55, see the disassembly).
#if defined(_M_IX86)
constexpr uintptr_t kWideEmpty = 0x011C7E54u;
constexpr uintptr_t kWideEmptyCap = 0x011C7E56u;
constexpr uintptr_t kEmptyNameCap = 0x011C7E55u;

// Log keys the manifest writer opens its records with.
constexpr uintptr_t kG1180A60 = 0x01180A60u;
constexpr uintptr_t kG1180A64 = 0x01180A64u;
constexpr uintptr_t kG1180A68 = 0x01180A68u;
constexpr uintptr_t kG1180A6C = 0x01180A6Cu;
constexpr uintptr_t kG1180A70 = 0x01180A70u;
constexpr uintptr_t kG1180A74 = 0x01180A74u;
constexpr uintptr_t kG1180A78 = 0x01180A78u;
constexpr uintptr_t kG1180A7C = 0x01180A7Cu;
constexpr uintptr_t kG1180A80 = 0x01180A80u;
constexpr uintptr_t kG1180A84 = 0x01180A84u;
constexpr uintptr_t kG1180A88 = 0x01180A88u;
constexpr uintptr_t kG1180A8C = 0x01180A8Cu;
constexpr uintptr_t kG1180A90 = 0x01180A90u;
constexpr uintptr_t kG1180A94 = 0x01180A94u;
constexpr uintptr_t kG1180A98 = 0x01180A98u;
constexpr uintptr_t kG1180A9C = 0x01180A9Cu;
constexpr uintptr_t kG1180AA0 = 0x01180AA0u;
constexpr uintptr_t kG1180AA4 = 0x01180AA4u;
constexpr uintptr_t kG1180AA8 = 0x01180AA8u;
constexpr uintptr_t kG1180AAC = 0x01180AACu;
constexpr uintptr_t kG1180AB0 = 0x01180AB0u;
constexpr uintptr_t kG1180AB4 = 0x01180AB4u;
constexpr uintptr_t kG1180AB8 = 0x01180AB8u;
constexpr uintptr_t kG1180ABC = 0x01180ABCu;

// The record logger 009B3A90 and 009B3820/009B3A10 drive.
constexpr uintptr_t kG1180AC0 = 0x01180AC0u;
constexpr uintptr_t kG1180AC4 = 0x01180AC4u;
constexpr uintptr_t kG1180AC8 = 0x01180AC8u;
constexpr uintptr_t kG1180ACC = 0x01180ACCu;
constexpr uintptr_t kG1180AD0 = 0x01180AD0u;
constexpr uintptr_t kG1180AD4 = 0x01180AD4u;
constexpr uintptr_t kG1180AE0 = 0x01180AE0u;
constexpr uintptr_t kG1180AE4 = 0x01180AE4u;
constexpr uintptr_t kG1180AE8 = 0x01180AE8u;
constexpr uintptr_t kG1180AEC = 0x01180AECu;
constexpr uintptr_t kG1180AF0 = 0x01180AF0u;
constexpr uintptr_t kG1180AF4 = 0x01180AF4u;

// String/format literals of the manifest writer and the walkers.
constexpr uintptr_t kStrLocalKeyList = 0x0104E1E4u;  // key descriptor value
constexpr uintptr_t kStrPkgTypeName = 0x0104ED80u;
constexpr uintptr_t kStrPkgSubTypeName = 0x0104ED50u;
constexpr uintptr_t kStrTrue = 0x00FC0C78u;   // L"true"
constexpr uintptr_t kStrFalse = 0x00FCBF94u;  // L"false"
constexpr uintptr_t kStrGameVersionName = 0x0104ED20u;
constexpr uintptr_t kStrGameVersionFmt = 0x00FB6880u;   // L"%hs"
constexpr uintptr_t kStrGameVersion = 0x00FCC700u;      // "0.0.0.32"
constexpr uintptr_t kStrCurTimeName = 0x0104ECF4u;
constexpr uintptr_t kStrTimeFmt = 0x0104A9B8u;
constexpr uintptr_t kStrGeneratedName = 0x0104ECC4u;
constexpr uintptr_t kStrGeneratedFmt = 0x0104EC9Cu;
constexpr uintptr_t kStrAssetVerName = 0x0104EC6Cu;
constexpr uintptr_t kStrAssetVerFmt = 0x00FC07D4u;  // L"%u"
constexpr uintptr_t kStrEpFlagsName = 0x0104EC40u;
constexpr uintptr_t kStrIntFmt = 0x0102B0C4u;   // L"%x"
constexpr uintptr_t kStrThumbnailName = 0x0104EC10u;
constexpr uintptr_t kStrKeyListName = 0x0104EBE4u;
constexpr uintptr_t kStrKeyDepName = 0x0104EBB8u;
constexpr uintptr_t kStrTempKeyName = 0x0104EB8Cu;
constexpr uintptr_t kStrPairFmt = 0x0104EB64u;
constexpr uintptr_t kStrBufferName = 0x0104FBA4u;
constexpr uintptr_t kStrElementName = 0x0104E378u;
constexpr uintptr_t kStrAttribName = 0x0104E354u;
constexpr uintptr_t kStrAttribValue = 0x0104E330u;
constexpr uintptr_t kStrParamName = 0x00FB9684u;
constexpr uintptr_t kStrSharedLibExport = 0x00FB946Cu;  // "EASharedLibraryStartupFunction"
constexpr uintptr_t kDefaultLoader = 0x01180EA0u;
#else
constexpr uintptr_t kWideEmpty = 0;
constexpr uintptr_t kWideEmptyCap = 0;
constexpr uintptr_t kEmptyNameCap = 0;
constexpr uintptr_t kG1180A60 = 0;
constexpr uintptr_t kG1180A64 = 0;
constexpr uintptr_t kG1180A68 = 0;
constexpr uintptr_t kG1180A6C = 0;
constexpr uintptr_t kG1180A70 = 0;
constexpr uintptr_t kG1180A74 = 0;
constexpr uintptr_t kG1180A78 = 0;
constexpr uintptr_t kG1180A7C = 0;
constexpr uintptr_t kG1180A80 = 0;
constexpr uintptr_t kG1180A84 = 0;
constexpr uintptr_t kG1180A88 = 0;
constexpr uintptr_t kG1180A8C = 0;
constexpr uintptr_t kG1180A90 = 0;
constexpr uintptr_t kG1180A94 = 0;
constexpr uintptr_t kG1180A98 = 0;
constexpr uintptr_t kG1180A9C = 0;
constexpr uintptr_t kG1180AA0 = 0;
constexpr uintptr_t kG1180AA4 = 0;
constexpr uintptr_t kG1180AA8 = 0;
constexpr uintptr_t kG1180AAC = 0;
constexpr uintptr_t kG1180AB0 = 0;
constexpr uintptr_t kG1180AB4 = 0;
constexpr uintptr_t kG1180AB8 = 0;
constexpr uintptr_t kG1180ABC = 0;
constexpr uintptr_t kG1180AC0 = 0;
constexpr uintptr_t kG1180AC4 = 0;
constexpr uintptr_t kG1180AC8 = 0;
constexpr uintptr_t kG1180ACC = 0;
constexpr uintptr_t kG1180AD0 = 0;
constexpr uintptr_t kG1180AD4 = 0;
constexpr uintptr_t kG1180AE0 = 0;
constexpr uintptr_t kG1180AE4 = 0;
constexpr uintptr_t kG1180AE8 = 0;
constexpr uintptr_t kG1180AEC = 0;
constexpr uintptr_t kG1180AF0 = 0;
constexpr uintptr_t kG1180AF4 = 0;
constexpr uintptr_t kStrLocalKeyList = 0;
constexpr uintptr_t kStrPkgTypeName = 0;
constexpr uintptr_t kStrPkgSubTypeName = 0;
constexpr uintptr_t kStrTrue = 0;
constexpr uintptr_t kStrFalse = 0;
constexpr uintptr_t kStrGameVersionName = 0;
constexpr uintptr_t kStrGameVersionFmt = 0;
constexpr uintptr_t kStrGameVersion = 0;
constexpr uintptr_t kStrCurTimeName = 0;
constexpr uintptr_t kStrTimeFmt = 0;
constexpr uintptr_t kStrGeneratedName = 0;
constexpr uintptr_t kStrGeneratedFmt = 0;
constexpr uintptr_t kStrAssetVerName = 0;
constexpr uintptr_t kStrAssetVerFmt = 0;
constexpr uintptr_t kStrEpFlagsName = 0;
constexpr uintptr_t kStrIntFmt = 0;
constexpr uintptr_t kStrThumbnailName = 0;
constexpr uintptr_t kStrKeyListName = 0;
constexpr uintptr_t kStrKeyDepName = 0;
constexpr uintptr_t kStrTempKeyName = 0;
constexpr uintptr_t kStrPairFmt = 0;
constexpr uintptr_t kStrBufferName = 0;
constexpr uintptr_t kStrElementName = 0;
constexpr uintptr_t kStrAttribName = 0;
constexpr uintptr_t kStrAttribValue = 0;
constexpr uintptr_t kStrParamName = 0;
constexpr uintptr_t kStrSharedLibExport = 0;
constexpr uintptr_t kDefaultLoader = 0;
#endif

// (cap - begin) & 0xFFFFFFFE, the game's "owns a heap buffer" test (see clu_408e20).
inline bool OwnsBuffer(const WideString& text)
{
    const intptr_t span =
        reinterpret_cast<intptr_t>(text.cap) - reinterpret_cast<intptr_t>(text.begin);
    return (static_cast<int32_t>(span) & static_cast<int32_t>(0xFFFFFFFEu)) > 2 &&
           text.begin != nullptr;
}

inline void ReleaseOwnedBuffer(WideString& text)
{
    if (OwnsBuffer(text))
        Free4E3F70(text.begin);
}

// The custom {begin, end, capacity, alloc name} records the manifest writer keeps.
struct KeyContainer {
    void* begin;
    void* end;
    void* cap;
    void* name;
};
TS3_X86_OFFSET(KeyContainer, begin, 0x00);
TS3_X86_OFFSET(KeyContainer, end, 0x04);
TS3_X86_OFFSET(KeyContainer, cap, 0x08);
TS3_X86_OFFSET(KeyContainer, name, 0x0C);
TS3_X86_SIZE(KeyContainer, 16);

inline void FreeRawContainer(KeyContainer& container)
{
    if (container.begin != nullptr)
        Free4E3F70(container.begin);
}

inline void DestroyWide(WideString* text)
{
    CallGameMethod<void, 0x004EF330>(text);
}

inline void DestroyWide405940(WideString* text)
{
    CallGameMethod<void, 0x00405940>(text);
}

// The game's static empty wide string with an allocator-name tag.
inline WideString EmptyWide(const void* allocName)
{
    WideString text{};
    text.begin = reinterpret_cast<void*>(kWideEmpty);
    text.end = reinterpret_cast<void*>(kWideEmpty);
    text.cap = reinterpret_cast<void*>(kWideEmptyCap);
    text.allocName = const_cast<void*>(allocName);
    return text;
}

// Reads one of the walker's global log keys. In x64 builds the game image is absent, so the
// bridges would abort if they are reached; a null value is enough to compile.
inline void* GlobalValue(uintptr_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(address);
#else
    (void)address;
    return nullptr;
#endif
}

// Logs one "enter / convert / log / free / leave" field of the record logger. Returns the
// leave call's result; the last block's value is the function result.
char LogWideField9B3A90(void* self, void* logger, size_t offset, uintptr_t key)
{
    CallGameMethod<char, 0x007DA190>(logger, GlobalValue(key));
    WideString converted{};
    CallGameFunction<WideString*, 0x004F3890>(&converted,
                                               static_cast<uint8_t*>(self) + offset);
    CallGameMethod<char, 0x007DA1D0>(logger, converted.begin);
    ReleaseOwnedBuffer(converted);
    return CallGameMethod<char, 0x007DA1F0>(logger, GlobalValue(key));
}

// Logs one field nested in a second record of the record logger.
char LogWideFieldNested9B3A90(void* self, void* logger, size_t offset, uintptr_t outer,
                              uintptr_t inner)
{
    CallGameMethod<char, 0x007DA190>(logger, GlobalValue(outer));
    LogWideField9B3A90(self, logger, offset, inner);
    return CallGameMethod<char, 0x007DA1F0>(logger, GlobalValue(outer));
}

// Logs one vector of 16-byte wide strings of the record logger.
char LogWideVector9B3A90(void* self, void* logger, size_t offset, uintptr_t enter,
                         uintptr_t sub, uintptr_t leave)
{
    CallGameMethod<char, 0x007DA190>(logger, GlobalValue(enter));
    uint8_t* element = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + offset);
    uint8_t* end =
        *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + offset + sizeof(void*));
    for (; element != end; element += sizeof(WideString)) {
        CallGameMethod<char, 0x007DA190>(logger, GlobalValue(sub));
        WideString converted{};
        CallGameFunction<WideString*, 0x004F3890>(&converted, element);
        CallGameMethod<char, 0x007DA1D0>(logger, converted.begin);
        ReleaseOwnedBuffer(converted);
        CallGameMethod<char, 0x007DA1F0>(logger, GlobalValue(sub));
    }
    return CallGameMethod<char, 0x007DA1F0>(logger, GlobalValue(leave));
}

}  // namespace

// ---------------------------------------------------------------------------------------
// 0x00408F10
WideString* __fastcall CopyWide408F10(WideString* self, void*, const WideString* other)
{
    if (other != self)
        AssignWide408E20(self, nullptr, other->begin, other->end);
    return self;
}

// ---------------------------------------------------------------------------------------
// 0x00409010
WideString* __fastcall AssignWideFromCStr409010(WideString* self, void*, const wchar_t* src)
{
    const wchar_t* cursor = src;
    while (*cursor)
        ++cursor;
    return static_cast<WideString*>(AssignWide408E20(self, nullptr, src, cursor));
}

// ---------------------------------------------------------------------------------------
// 0x004F2800
uint8_t __fastcall LoadSharedLibrary4F2800(void* self, void*, const wchar_t* path,
                                           uint8_t flag)
{
    if (path == nullptr)
        return 0;

    // The first gate is virtual slot 8 (no arguments); the update must be requested again
    // through slot 5 (argument 0) after the name has been assigned.
    if (CallGameVirtual<char, 8>(self) == 0)
        return 0;

    HMODULE module = ::LoadLibraryW(path);
    if (module == nullptr)
        return 0;

    // The startup export must exist; the module is dropped again right away.
    void* startup =
        reinterpret_cast<void*>(::GetProcAddress(module, reinterpret_cast<const char*>(
                                                        kStrSharedLibExport)));
    ::FreeLibrary(module);
    if (startup == nullptr)
        return 0;

    WideString* name = reinterpret_cast<WideString*>(static_cast<uint8_t*>(self) + 0x0C);
    WideString saved = EmptyWide(nullptr);
    CallGameMethod<WideString*, 0x00408EB0>(&saved, name);

    AssignWideFromCStr409010(name, nullptr, path);

    if (flag != 0 && CallGameVirtual<char, 5>(self, 0) == 0) {
        // Roll the failed update back and report the failure.
        CopyWide408F10(name, nullptr, &saved);
        DestroyWide(&saved);
        return 0;
    }

    ReleaseOwnedBuffer(saved);
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x00500830
uint64_t __fastcall GetValue500830(void* self, void*, int32_t index)
{
    const uint32_t* words = static_cast<const uint32_t*>(self);
    if (index == 0)
        return static_cast<uint64_t>(words[0]) | (static_cast<uint64_t>(words[1]) << 32);
    if (index == 1)
        return static_cast<uint64_t>(words[2]) | (static_cast<uint64_t>(words[3]) << 32);
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x007DA1B0
char __fastcall Dispatch7DA1B0(void* self, void*, int32_t a2, int32_t a3)
{
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) != 2)
        return 0;

    void* target = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x0C);
    return CallGameVirtual<char, 6>(target, a2, a3);
}

// ---------------------------------------------------------------------------------------
// 0x007DA210
char __fastcall ResetState7DA210(void* self, void*)
{
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) != 2)
        return 0;

    *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) = 0;
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x007DA2D0
char __fastcall Dispatch7DA2D0(void* self, void*, int32_t a2, const void* a3)
{
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) != 2)
        return 0;

    void* target = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x0C);
    return CallGameVirtual<char, 6>(target, a2, *static_cast<const int32_t*>(a3));
}

// ---------------------------------------------------------------------------------------
// 0x007DA840
char __fastcall ConvertAndAssign7DA840(void* self, void*, int32_t value, WideString* dst)
{
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) != 1)
        return 0;

    void* source = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x08);
    const char* text = CallGameMethod<const char*, 0x00471070>(source, value);

    // The dispatcher may have been retargeted while converting; that skips only the
    // extra [obj+8] test, the assignment still happens.
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) == 1) {
        void* again = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x08);
        if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(again) + 0x08) != 0)
            return 0;
    }
    if (text == nullptr)
        return 0;

    AssignWideFromCStr409010(dst, nullptr, reinterpret_cast<const wchar_t*>(text));
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x007DBAF0
char __fastcall AppendConverted7DBAF0(void* self, void*, const WideString* text)
{
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) != 2)
        return 0;

    const uint8_t* begin = static_cast<const uint8_t*>(text->begin);
    const uint8_t* end = static_cast<const uint8_t*>(text->end);
    const int32_t length = static_cast<int32_t>(end - begin) >> 1;

    // EA's clamp: pass 0 while the count is positive, the negative count as itself.
    const int32_t count = length <= 0 ? length : 0;
    const int32_t checked = CallGameFunction<int32_t, 0x004E33D0>(
        begin, reinterpret_cast<const void*>(kStrParamName), count);
    if (checked == 0 && length == 0)
        return 0;

    uint8_t* vector = static_cast<uint8_t*>(self) + 0x14;
    uint8_t* slot = *reinterpret_cast<uint8_t**>(vector + 0x04);   // end
    uint8_t* limit = *reinterpret_cast<uint8_t**>(vector + 0x08);  // capacity end
    if (slot >= limit) {
        CallGameMethod<void*, 0x0051D700>(vector, slot, const_cast<WideString*>(text));
    } else {
        *reinterpret_cast<uint8_t**>(vector + 0x04) = slot + 0x10;
        if (slot != nullptr)
            CallGameMethod<WideString*, 0x00408EB0>(slot, const_cast<WideString*>(text));
    }

    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x48) == 1) {
        void* other = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x08);
        if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(other) + 0x08) != 0)
            return 0;
    }
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x008A7870
WideString* __fastcall InitName8A7870(WideString* self, void*, const void** src)
{
    self->begin = reinterpret_cast<void*>(kWideEmpty);
    self->end = reinterpret_cast<void*>(kWideEmpty);
    self->cap = reinterpret_cast<void*>(kEmptyNameCap);
    self->allocName = const_cast<void*>(*src);
    return self;
}

// ---------------------------------------------------------------------------------------
// 0x0093D2C0
int32_t __fastcall CopyRange93D2C0(void* self, void*, int32_t a1, int32_t a2, int32_t a3)
{
    // The original duplicates the range end into the fourth argument and keeps the incoming
    // ECX as the callee's this pointer.
    return CallGameMethod<int32_t, 0x006C8370>(self, a1, a2, a3, a3);
}

// ---------------------------------------------------------------------------------------
// 0x009B3820
char __cdecl WalkRecord9B3820(void* record, void* logger)
{
    uint8_t* node = static_cast<uint8_t*>(record);
    if (node == nullptr || *reinterpret_cast<int32_t*>(node + 0x04) != 2)
        return 0;

    WideString element{};
    CallGameFunction<WideString*, 0x004F3750>(
        &element, *reinterpret_cast<void**>(node + 0x08),
        *reinterpret_cast<int32_t*>(node + 0x0C) - *reinterpret_cast<int32_t*>(node + 0x08),
        reinterpret_cast<const char*>(kStrElementName));

    // The walker enters and leaves the element under its converted name (the 4F3750
    // output's begin pointer), not under a global key; the name is freed afterwards.
    if (CallGameMethod<char, 0x007DA190>(logger, element.begin) == 0) {
        ReleaseOwnedBuffer(element);
        return 0;
    }

    // The record's attributes hang off the circular list rooted at node+0x38.
    for (uint8_t* attribute = *reinterpret_cast<uint8_t**>(node + 0x38);
         attribute != node + 0x38; attribute = *reinterpret_cast<uint8_t**>(attribute)) {
        WideString attributeName{};
        WideString attributeValue{};
        CallGameFunction<WideString*, 0x004F3750>(
            &attributeName, *reinterpret_cast<void**>(attribute + 0x08),
            *reinterpret_cast<int32_t*>(attribute + 0x0C) -
                *reinterpret_cast<int32_t*>(attribute + 0x08),
            reinterpret_cast<const char*>(kStrAttribName));
        CallGameFunction<WideString*, 0x004F3750>(
            &attributeValue, *reinterpret_cast<void**>(attribute + 0x20),
            *reinterpret_cast<int32_t*>(attribute + 0x24) -
                *reinterpret_cast<int32_t*>(attribute + 0x20),
            reinterpret_cast<const char*>(kStrAttribValue));

        const char logged = Dispatch7DA1B0(
            logger, nullptr, reinterpret_cast<int32_t>(attributeName.begin),
            reinterpret_cast<int32_t>(attributeValue.begin));
        ReleaseOwnedBuffer(attributeName);
        ReleaseOwnedBuffer(attributeValue);
        if (logged == 0) {
            ReleaseOwnedBuffer(element);
            return 0;
        }
    }

    // Children hang off the circular list rooted at node+0x24.
    for (uint8_t* child = *reinterpret_cast<uint8_t**>(node + 0x24); child != node + 0x24;
         child = *reinterpret_cast<uint8_t**>(child)) {
        if (WalkRecord9B3820(*reinterpret_cast<void**>(child + 0x08), logger) == 0) {
            ReleaseOwnedBuffer(element);
            return 0;
        }
    }

    const char left = CallGameMethod<char, 0x007DA1F0>(logger, element.begin);
    ReleaseOwnedBuffer(element);
    return left;
}

// ---------------------------------------------------------------------------------------
// 0x009B3A10
char __fastcall WalkRecords9B3A10(void* self, void*, void* logger)
{
    if (CallGameMethod<char, 0x007DA190>(logger, GlobalValue(kG1180A80)) == 0)
        return 0;

    uint8_t* record = *reinterpret_cast<uint8_t**>(self);
    uint8_t* end = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x04);
    for (; record != end; record += 2 * sizeof(void*)) {
        if (WalkRecord9B3820(*reinterpret_cast<void**>(record + 0x04), logger) == 0)
            return 0;
    }

    return CallGameMethod<char, 0x007DA1F0>(logger, GlobalValue(kG1180A80));
}

// ---------------------------------------------------------------------------------------
// 0x009B3A90
int32_t __fastcall LogRecord9B3A90(void* self, void*, void* logger)
{
    LogWideField9B3A90(self, logger, 0xA0, kG1180AC0);
    LogWideField9B3A90(self, logger, 0xB0, kG1180AC4);
    LogWideField9B3A90(self, logger, 0xF0, kG1180AC8);
    LogWideField9B3A90(self, logger, 0x100, kG1180ACC);
    LogWideVector9B3A90(self, logger, 0x120, kG1180AD0, kG1180AD4, kG1180AD0);
    LogWideFieldNested9B3A90(self, logger, 0xC0, kG1180AA4, kG1180AE0);
    LogWideFieldNested9B3A90(self, logger, 0xD0, kG1180AB0, kG1180AE0);
    LogWideVector9B3A90(self, logger, 0x130, kG1180AE4, kG1180AE0, kG1180AE4);
    LogWideFieldNested9B3A90(self, logger, 0x110, kG1180AE8, kG1180AE0);
    LogWideVector9B3A90(self, logger, 0x140, kG1180AEC, kG1180AF0, kG1180AEC);
    return LogWideField9B3A90(self, logger, 0xE0, kG1180AF4);
}

// ---------------------------------------------------------------------------------------
// 0x009B8250
char __fastcall BuildManifest9B8250(void* self, void*, void* a2)
{
    // All locals are declared up front so the failure gotos below stay valid.
    uint8_t logger[76] = {};
    WideString packageType = EmptyWide(nullptr);
    WideString packageSubType = EmptyWide(nullptr);
    WideString gameVersion = EmptyWide(nullptr);
    WideString currentTime = EmptyWide(nullptr);
    WideString assetVersion = EmptyWide(nullptr);
    WideString epFlags = EmptyWide(nullptr);
    WideString thumbnail = EmptyWide(nullptr);
    WideString generated = EmptyWide(nullptr);
    KeyContainer keyList = {nullptr, nullptr, nullptr, nullptr};
    KeyContainer keyDependencies = {nullptr, nullptr, nullptr, nullptr};
    uint8_t timeObject[8] = {};
    int32_t keyIndex = 0;
    wchar_t* convertedEnd = nullptr;
    wchar_t converted[130] = {};

    if (a2 == nullptr)
        return 0;

    CallGameMethod<void, 0x007DBA60>(&logger);
    if (CallGameMethod<char, 0x007DA230>(&logger, a2, 1) == 0 ||
        CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A60)) == 0)
        goto done_success_early;

    CallGameMethod<void, 0x007DB350>(&logger, GlobalValue(kG1180A88),
                                     reinterpret_cast<void*>(kStrLocalKeyList), 1);

    packageType = EmptyWide(reinterpret_cast<void*>(kStrPkgTypeName));
    {
        const char* convertedName = CallGameFunction<const char*, 0x0099AE70>(
            *reinterpret_cast<uint32_t*>(self),
            *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(self) + 0x04));
        AssignWideFromCStr409010(&packageType, nullptr,
                                 reinterpret_cast<const wchar_t*>(convertedName));
    }
    Dispatch7DA2D0(&logger, nullptr, reinterpret_cast<int32_t>(GlobalValue(kG1180A90)),
                   &packageType);

    packageSubType = EmptyWide(reinterpret_cast<void*>(kStrPkgSubTypeName));
    CallGameFunction<void*, 0x0040E920>(&packageSubType,
                                        reinterpret_cast<const wchar_t*>(kStrIntFmt),
                                        *reinterpret_cast<uint32_t*>(
                                            static_cast<uint8_t*>(self) + 0x04));
    Dispatch7DA2D0(&logger, nullptr, reinterpret_cast<int32_t>(GlobalValue(kG1180A94)),
                   &packageSubType);

    Dispatch7DA1B0(&logger, nullptr, reinterpret_cast<int32_t>(GlobalValue(kG1180A9C)),
                   *reinterpret_cast<uint8_t*>(static_cast<uint8_t*>(self) + 0x58) != 0
                       ? static_cast<int32_t>(kStrTrue)
                       : static_cast<int32_t>(kStrFalse));

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A98)) == 0)
        goto fail_freed_early;

    gameVersion = EmptyWide(reinterpret_cast<void*>(kStrGameVersionName));
    CallGameFunction<void*, 0x0040E920>(
        &gameVersion, reinterpret_cast<const wchar_t*>(kStrGameVersionFmt),
        reinterpret_cast<const char*>(kStrGameVersion));
    CallGameMethod<char, 0x007DA1D0>(&logger, gameVersion.begin);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A98));

    if (*reinterpret_cast<uint32_t*>(self) == 0xC4FB4F1u ||
        *reinterpret_cast<uint32_t*>(self) == 0xC4FB4F9u)
        LogRecord9B3A90(self, nullptr, &logger);

    CallGameMethod<void, 0x004F9F40>(timeObject, 2);
    currentTime = EmptyWide(reinterpret_cast<void*>(kStrCurTimeName));
    {
        const int32_t field10 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 10);
        const int32_t field9 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 9);
        const int32_t field8 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 8);
        const int32_t field1 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 1);
        const int32_t field6 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 6);
        const int32_t field2 = CallGameMethod<int32_t, 0x004F9AF0>(timeObject, 2);
        CallGameFunction<void*, 0x0040E920>(&currentTime,
                                            reinterpret_cast<const wchar_t*>(kStrTimeFmt),
                                            field2, field6, field1, field8, field9, field10);
    }
    CallGameMethod<void*, 0x00501D70>(static_cast<uint8_t*>(self) + 0x28, converted,
                                      &convertedEnd, 16);

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A70)) == 0)
        goto fail_late;
    CallGameMethod<char, 0x007DA1D0>(&logger, currentTime.begin);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A70));

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A64)) == 0)
        goto fail_late;
    CallGameMethod<char, 0x007DA1D0>(&logger, converted);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A64));

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A68)) == 0)
        goto fail_late;
    if (*reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x08) ==
        *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x0C)) {
        const char* name = reinterpret_cast<const char*>(kStrGeneratedName);
        generated = EmptyWide(nullptr);
        CallGameMethod<WideString*, 0x00408890>(&generated, &name);
        const uint64_t low = GetValue500830(static_cast<uint8_t*>(self) + 0x28, nullptr, 0);
        const uint64_t high = GetValue500830(static_cast<uint8_t*>(self) + 0x28, nullptr, 1);
        CallGameFunction<void*, 0x0040E920>(
            &generated, reinterpret_cast<const wchar_t*>(kStrGeneratedFmt),
            packageType.begin, high, low);
    } else {
        generated = EmptyWide(nullptr);
        CallGameFunction<WideString*, 0x004F3890>(
            &generated, static_cast<uint8_t*>(self) + 0x08);
    }
    CallGameMethod<char, 0x007DA1D0>(&logger, generated.begin);
    DestroyWide(&generated);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A68));

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A6C)) == 0)
        goto fail_late;
    if (*reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x18) ==
        *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x1C))
        CallGameMethod<int32_t, 0x00405B30>(
            static_cast<uint8_t*>(nullptr),
            *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x08));
    {
        WideString part = EmptyWide(nullptr);
        CallGameFunction<WideString*, 0x004F3890>(&part,
                                                  static_cast<uint8_t*>(self) + 0x18);
        CallGameMethod<char, 0x007DA1D0>(&logger, part.begin);
        DestroyWide(&part);
    }
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A6C));

    CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AA0));
    {
        const char* name = reinterpret_cast<const char*>(kStrAssetVerName);
        assetVersion = EmptyWide(nullptr);
        CallGameMethod<WideString*, 0x00408890>(&assetVersion, &name);
        CallGameFunction<void*, 0x0040E920>(
            &assetVersion, reinterpret_cast<const wchar_t*>(kStrAssetVerFmt),
            static_cast<uint32_t>(*reinterpret_cast<uint16_t*>(
                static_cast<uint8_t*>(self) + 0x5A)));
        CallGameMethod<char, 0x007DA1D0>(&logger, assetVersion.begin);
        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AA0));
    }
    CallGameMethod<int32_t, 0x009B01A0>(static_cast<uint8_t*>(self) + 0x7C, &assetVersion);
    CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AB8));
    CallGameMethod<char, 0x007DA1D0>(&logger, assetVersion.begin);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AB8));

    CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180ABC));
    {
        const char* name = reinterpret_cast<const char*>(kStrEpFlagsName);
        epFlags = EmptyWide(nullptr);
        CallGameMethod<WideString*, 0x00408890>(&epFlags, &name);
        CallGameFunction<void*, 0x0040E920>(
            &epFlags, reinterpret_cast<const wchar_t*>(kStrIntFmt),
            *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(self) + 0x9C));
        CallGameMethod<char, 0x007DA1D0>(&logger, epFlags.begin);
        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180ABC));
    }

    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A74)) == 0)
        goto fail_late_no_thumbnail;

    {
        const void* name = reinterpret_cast<const void*>(kStrThumbnailName);
        thumbnail = EmptyWide(nullptr);
        InitName8A7870(&thumbnail, nullptr, &name);
    }
    CallGameFunction<void*, 0x0099E770>(static_cast<uint8_t*>(self) + 0x48, &thumbnail);
    {
        WideString thumbnailText = EmptyWide(nullptr);
        CallGameFunction<WideString*, 0x004F3890>(&thumbnailText, &thumbnail);
        CallGameMethod<char, 0x007DA1D0>(&logger, thumbnailText.begin);
        DestroyWide(&thumbnailText);
    }
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A74));

    if (*reinterpret_cast<uint32_t*>(self) != 0xC4FB4F1u &&
        *reinterpret_cast<uint32_t*>(self) != 0xC4FB4F9u) {
        // The two 32-byte dependency records.
        if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AA4)) == 0)
            goto fail_thumbnail;
        for (uint8_t* element =
                 *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x5C);
             element != *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x60);
             element += 0x20) {
            if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AA8)) == 0)
                goto fail_thumbnail;
            Dispatch7DA1B0(&logger, nullptr,
                           reinterpret_cast<int32_t>(GlobalValue(kG1180AAC)),
                           *reinterpret_cast<int32_t*>(element));
            CallGameMethod<void, 0x007DA880>(&logger);
            AppendConverted7DBAF0(&logger, nullptr,
                                  reinterpret_cast<WideString*>(element + 0x10));
            CallGameMethod<void, 0x007DA970>(&logger);
            CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AA8));
        }
        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AA4));

        if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AB0)) == 0)
            goto fail_thumbnail;
        for (uint8_t* element =
                 *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x6C);
             element != *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x70);
             element += 0x20) {
            if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180AB4)) == 0)
                goto fail_thumbnail;
            Dispatch7DA1B0(&logger, nullptr,
                           reinterpret_cast<int32_t>(GlobalValue(kG1180AAC)),
                           *reinterpret_cast<int32_t*>(element));
            CallGameMethod<void, 0x007DA880>(&logger);
            AppendConverted7DBAF0(&logger, nullptr,
                                  reinterpret_cast<WideString*>(element + 0x10));
            CallGameMethod<void, 0x007DA970>(&logger);
            CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AB4));
        }
        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180AB0));
    }

    if (WalkRecords9B3A10(static_cast<uint8_t*>(self) + 0x8C, nullptr, &logger) == 0)
        goto fail_thumbnail;
    if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A78)) == 0)
        goto fail_thumbnail;

    // The package's key list and the matching dependency list.
    {
        const int32_t recordSpan = *reinterpret_cast<int32_t*>(
                                       static_cast<uint8_t*>(self) + 0x3C) -
                                   *reinterpret_cast<int32_t*>(
                                       static_cast<uint8_t*>(self) + 0x38);
        uint32_t recordCount = static_cast<uint32_t>(recordSpan / 52);
        uint8_t* keyListRecord = static_cast<uint8_t*>(self) + 0x38;
        keyList = KeyContainer{nullptr, nullptr, nullptr,
                               reinterpret_cast<void*>(kStrKeyListName)};
        keyDependencies = KeyContainer{nullptr, nullptr, nullptr,
                                       reinterpret_cast<void*>(kStrKeyDepName)};
        int32_t elementCount = 0;
        const void* position = nullptr;
        keyIndex = 0;

        for (; static_cast<uint32_t>(keyIndex) < recordCount; ++keyIndex) {
            uint8_t* element =
                keyListRecord + 52 * static_cast<size_t>(keyIndex);
            const int32_t count = (*reinterpret_cast<int32_t*>(element + 0x18) -
                                   *reinterpret_cast<int32_t*>(element + 0x14)) >> 4;
            CopyRange93D2C0(&keyList, nullptr, reinterpret_cast<int32_t>(position),
                            *reinterpret_cast<int32_t*>(element + 0x14),
                            *reinterpret_cast<int32_t*>(element + 0x18));
            position = keyList.end;
            CallGameMethod<int32_t, 0x007AAB20>(&keyDependencies, 0, count, &keyIndex);

            if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A64)) == 0)
                goto fail_keys;
            CallGameMethod<void*, 0x00501D70>(element + 0x04, converted, &convertedEnd, 16);
            CallGameMethod<char, 0x007DA1D0>(&logger, converted);
            CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A64));
        }
        keyIndex = static_cast<int32_t>(recordCount);
        elementCount = static_cast<int32_t>(
            (static_cast<uint8_t*>(keyList.end) - static_cast<uint8_t*>(keyList.begin)) >> 4);

        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A78));
        if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A7C)) == 0)
            goto fail_keys;

        if (elementCount !=
            static_cast<int32_t>((static_cast<uint8_t*>(keyDependencies.end) -
                                  static_cast<uint8_t*>(keyDependencies.begin)) >>
                                 2))
            goto fail_keys;

        {
            for (uint32_t index = 0; index < static_cast<uint32_t>(elementCount); ++index) {
                if (CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A84)) == 0)
                    goto fail_keys;
                const uint32_t* keyElement = reinterpret_cast<const uint32_t*>(
                    static_cast<uint8_t*>(keyList.begin) + 16 * index);
                const char* name = reinterpret_cast<const char*>(kStrTempKeyName);
                generated = EmptyWide(nullptr);
                CallGameMethod<WideString*, 0x00408890>(&generated, &name);
                CallGameFunction<void*, 0x0040E920>(
                    &generated, reinterpret_cast<const wchar_t*>(kStrPairFmt),
                    static_cast<uint32_t>(*reinterpret_cast<int32_t*>(
                        static_cast<uint8_t*>(keyDependencies.begin) + 4 * index)),
                    keyElement[2], keyElement[3], keyElement[0], keyElement[1]);
                CallGameMethod<char, 0x007DA1D0>(&logger, generated.begin);
                CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A84));
                DestroyWide(&generated);
            }
        }
        CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A7C));
    }

    CallGameMethod<char, 0x007DA190>(&logger, GlobalValue(kG1180A8C));
    CallGameMethod<int32_t, 0x009B6230>(static_cast<uint8_t*>(self) + 0x150,
                                        *reinterpret_cast<int32_t*>(self), &logger);
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A8C));
    CallGameMethod<char, 0x007DA1F0>(&logger, GlobalValue(kG1180A60));

    FreeRawContainer(keyDependencies);
    FreeRawContainer(keyList);
    DestroyWide405940(&thumbnail);
    DestroyWide(&epFlags);
    DestroyWide(&assetVersion);
    DestroyWide(&currentTime);
    DestroyWide(&gameVersion);
    DestroyWide(&packageSubType);
    DestroyWide(&packageType);

done_success_early:
    CallGameMethod<char, 0x007DA210>(&logger);
    CallGameMethod<void, 0x007DB860>(&logger);
    return 1;

fail_freed_early:
    ReleaseOwnedBuffer(packageSubType);
    ReleaseOwnedBuffer(packageType);
    CallGameMethod<void, 0x007DB860>(&logger);
    return 0;

fail_keys:
    FreeRawContainer(keyDependencies);
    FreeRawContainer(keyList);
fail_thumbnail:
    DestroyWide405940(&thumbnail);
fail_late_no_thumbnail:
    DestroyWide(&epFlags);
    DestroyWide(&assetVersion);
fail_late:
    DestroyWide(&currentTime);
    DestroyWide(&gameVersion);
    DestroyWide(&packageSubType);
    DestroyWide(&packageType);
    CallGameMethod<void, 0x007DB860>(&logger);
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x009B95A0
char __fastcall CreateAndLoad9B95A0(void* self, void*, void* loader, void* source)
{
    if (loader == nullptr)
        return 0;

    void* input = source != nullptr ? source : reinterpret_cast<void*>(kDefaultLoader);
    void* produced = nullptr;
    char result = 0;
    if (CallGameVirtual<char, 13>(loader, input, &produced, 2, 2, 1, 0) != 0) {
        void* object = CallGameVirtual<void*, 6>(produced);
        result = BuildManifest9B8250(self, nullptr, object);
        CallGameVirtual<void, 13>(object);
        CallGameVirtual<void, 8>(produced);
    }
    if (produced != nullptr)
        CallGameVirtual<void, 2>(produced);
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x009C6630
char __fastcall WriteBuffer9C6630(void* self, void*, const void* src, void* target,
                                  uint32_t length)
{
    if (target == nullptr)
        return 0;
    if (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0xB4) != 1)
        return 0;
    if (*reinterpret_cast<uint8_t*>(static_cast<uint8_t*>(self) + 0x114) == 0)
        return 0;

    void* container = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x110);
    if (container == nullptr)
        return 0;
    int32_t offset = *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x118);
    if (offset == 0)
        return 0;
    if (src == nullptr)
        return 0;

    // The seek argument is a plain file position: the buffer offset plus the record's own
    // +0x24 field, added as 32-bit (not a pointer into the record).
    const uint32_t position =
        static_cast<uint32_t>(offset) +
        *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(src) + 0x24);
    if (CallGameVirtual<char, 10>(container,
                                  reinterpret_cast<void*>(static_cast<uintptr_t>(position)),
                                  0) == 0)
        return 0;

    WideString scratch = EmptyWide(nullptr);
    {
        const void* name = reinterpret_cast<const void*>(kStrBufferName);
        InitName8A7870(&scratch, nullptr, &name);
    }
    CallGameMethod<void*, 0x00409BB0>(&scratch, 0x1000);
    if (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(scratch.cap) -
                              reinterpret_cast<uintptr_t>(scratch.begin) - 1) < 0x1000) {
        DestroyWide405940(&scratch);
        return 0;
    }

    // Every chunk is written from the start of the scratch buffer (the original never
    // advances the pointer).
    uint8_t* buffer = static_cast<uint8_t*>(scratch.begin);
    uint32_t remaining = length;
    while (remaining != 0) {
        const uint32_t chunk = remaining > 0x1000 ? 0x1000 : remaining;
        if (CallGameVirtual<int32_t, 12>(container, buffer, chunk) == 0)
            goto fail;
        if (CallGameVirtual<char, 14>(target, buffer, chunk) == 0)
            goto fail;
        remaining -= chunk;
    }
    DestroyWide405940(&scratch);
    return 1;

fail:
    DestroyWide405940(&scratch);
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x009C6760
char __fastcall WriteIndexed9C6760(void* self, void*, uint16_t index, void* target)
{
    const int32_t count =
        (*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x08) -
         *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + 0x04)) /
        2152;
    if (static_cast<uint32_t>(index) >= static_cast<uint32_t>(count))
        // The original reads the element length from absolute address 0x20 (xor esi,esi;
        // mov eax,[esi+0x20]) and faults. volatile keeps that load (and the fault) against
        // the optimizer, which would otherwise drop it: its only use is a length argument
        // that 0x009C6630 never reads on the null-source path.
        return WriteBuffer9C6630(self, nullptr, nullptr, target,
                                 *reinterpret_cast<volatile uint32_t*>(0x20));

    uint8_t* element = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(self) + 0x04) +
                       2152 * static_cast<uint32_t>(index);
    return WriteBuffer9C6630(self, nullptr, element, target,
                             *reinterpret_cast<uint32_t*>(element + 0x20));
}

}  // namespace ts3::scratch