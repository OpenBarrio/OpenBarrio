// Rewritten cluster unit clu_4dbbd0_Scratch (Scratch subsystem): the TS3W.exe
// DownloadContent / world-archive scratch functions 0x004DBBD0 .. 0x00D15C50.
//
// Every function keeps its original address in the comment above it. Callees that are
// already rewritten are called directly (Free4E3F70, FormatWide0040E920, AssignWide408E20,
// SplitPath, NormalizePath, GetDirectoryManager); everything else goes through the
// layout.h bridge. Original calling conventions are preserved: __thiscall entries are
// declared __fastcall with a dummy second parameter (self in ECX, dummy in EDX, callee
// pops the stack arguments), cdecl/stdcall entries stay as declared.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
#include "ts3/resourcesystem/clu_727290_ResourceSystem.h"
#include "ts3/scratch/clu_40e920_Scratch.h"
#include "ts3/scratch/clu_4dbbd0_Scratch.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include "ts3/world/clu_4ec520_World.h"

namespace ts3::scratch {

namespace {

// A game global that holds a value of T at a fixed address (e.g. 0x0117E298 = the
// version string pointer, 0x011808B0/B4/B8 = the version store).
template <class T>
T GameGlobal(uintptr_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(address);
#else
    (void)address;
    NotRewritten("Scratch game global");
    return T{};
#endif
}

// A game rodata address used as a string/format constant.
template <class T>
T GameConstant(uintptr_t address)
{
#if defined(_M_IX86)
    return reinterpret_cast<T>(address);
#else
    (void)address;
    NotRewritten("Scratch game constant");
    return T{};
#endif
}

// The game's case table at 0x0114CB30: identity except 'A'..'Z' -> 'a'..'z' (read
// byte by byte, exactly like the sibling units of this subsystem).
inline uint8_t FoldByte(uint16_t character)
{
#if defined(_M_IX86)
    return reinterpret_cast<const uint8_t*>(uintptr_t{0x0114CB30})[character];
#else
    if (character >= u'A' && character <= u'Z')
        return static_cast<uint8_t>(character + 0x20);
    return static_cast<uint8_t>(character);
#endif
}

// The empty-string sentinels the game uses for these objects.
constexpr uintptr_t kEmptyWideBegin = 0x011C7E54;
constexpr uintptr_t kEmptyWideCap = 0x011C7E56;

void SetEmptyWideString(ScratchWideString* string, void* allocName)
{
    string->begin = reinterpret_cast<void*>(kEmptyWideBegin);
    string->end = reinterpret_cast<void*>(kEmptyWideBegin);
    string->cap = reinterpret_cast<void*>(kEmptyWideCap);
    string->allocName = allocName;
}

// Free `begin` when (cap - begin) & ~1 > 2 and begin is non-null, and return the free
// helper's EAX (the original always leaves the last call's EAX in EAX).
void* FreeHeapIfOwned(ScratchWideString* string, void* begin)
{
    const int32_t span = static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(string->cap) - reinterpret_cast<uintptr_t>(begin)) &
        static_cast<int32_t>(0xFFFFFFFEu);
    if (span > 2 && begin != nullptr)
        return Free4E3F70(begin);
    return begin;
}

// 0x00987D50/0x00987E40 family: 0x00986600 and 0x00D38B00 fill an 8-byte hash-map
// iterator {node, bucket}. The map header holds the bucket array at +4 and the sentinel
// bucket index at +8; a lookup that returns the sentinel node missed.
struct MapIterator {
    void* node;
    void* bucket;
};
TS3_X86_SIZE(MapIterator, 8);

void* MapSentinelNode(void* map)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(map);
    void* const* buckets = reinterpret_cast<void* const*>(
        *reinterpret_cast<const uintptr_t*>(base + 4));
    const uint32_t sentinelIndex = *reinterpret_cast<const uint32_t*>(base + 8);
    return buckets[sentinelIndex];
}

// 0x00594EC0: records are 20 bytes; free the pointer at +4 when
// ((*(record+12) - it) & ~1) > 2 and it is non-null.
void DestroyRecordRange(uintptr_t first, uintptr_t last)
{
    for (uintptr_t i = first; i < last; i += 20) {
        const int32_t value = *reinterpret_cast<const int32_t*>(i + 4);
        const int32_t span = (*reinterpret_cast<const int32_t*>(i + 12) - value) & 0xFFFFFFFE;
        if (span > 2 && value != 0)
            Free4E3F70(reinterpret_cast<void*>(static_cast<intptr_t>(value)));
    }
}

}  // namespace

// TS3W.exe 0x004DBBD0.
int32_t DeleteFile004DBBD0(const wchar_t* lpFileName)
{
    return DeleteFileW(lpFileName) != 0 ? 1 : 0;
}

// TS3W.exe 0x004DCFF0.
char EnsureDirectory004DCFF0(const wchar_t* lpFileName)
{
    if (lpFileName == nullptr || *lpFileName == 0) return 0;
    const DWORD attributes = GetFileAttributesW(lpFileName);
    if (attributes == 0xFFFFFFFFu || (attributes & 0x10u) == 0)
        return static_cast<char>(CallGameFunction<uint8_t, 0x004DCD40>(lpFileName));
    return 1;
}

// TS3W.exe 0x004ED5E0.
const wchar_t* __fastcall GetSplitPathExtension004ED5E0(const wchar_t* path, void* /*edx*/)
{
    const wchar_t* base = nullptr;
    const wchar_t* file = nullptr;
    const wchar_t* extension = nullptr;
    ts3::world::SplitPath(path, &base, &file, &extension);
    return extension;
}

// TS3W.exe 0x004EDA20.
char16_t* __fastcall ConstructWideFromPath004EDA20(char16_t* out, void* /*edx*/,
                                                   const char16_t* src)
{
    int32_t zero = 0;
    *out = 0;
    return ts3::resourcesystem::NormalizePath(
        out, src, reinterpret_cast<const char16_t*>(&zero), 5);
}

// TS3W.exe 0x004EDD80.
char EnsureTrailingSeparator004EDD80(ScratchWideString* string)
{
    const uintptr_t begin = reinterpret_cast<uintptr_t>(string->begin);
    const uintptr_t end = reinterpret_cast<uintptr_t>(string->end);
    if (begin != end) {
        const uint16_t last =
            *reinterpret_cast<const uint16_t*>(end - 2);
        if (last == 0x5C || last == 0x2F || last == 0x3A) return 0;
    }
    CallGameMethod<void, 0x00412CC0>(string, static_cast<int16_t>(0x5C));
    return 1;
}

// TS3W.exe 0x004EF330.
void* __fastcall DestroyString004EF330(ScratchWideString* self, void* /*edx*/)
{
    return FreeHeapIfOwned(self, self->begin);
}

// TS3W.exe 0x004FF6D0.
int32_t CompareWideFold004FF6D0(const char16_t* a, const char16_t* b)
{
    for (;;) {
        uint16_t foldedA = *a;
        if (foldedA < 0x100u) foldedA = FoldByte(foldedA);
        uint16_t foldedB = *b;
        if (foldedB < 0x100u) foldedB = FoldByte(foldedB);
        ++a;
        ++b;
        if (foldedA != foldedB)
            return static_cast<int32_t>(foldedA) - static_cast<int32_t>(foldedB);
        if (foldedA == 0) return 0;
    }
}

// TS3W.exe 0x00594EC0.
void __stdcall DestroyRecordArray00594EC0(uintptr_t first, uintptr_t last)
{
    DestroyRecordRange(first, last);
}

// TS3W.exe 0x0077F620.
int32_t NotifyHotInstall0077F620(void* src)
{
    const uintptr_t manager = CallGameFunction<uintptr_t, 0x007692C0>();
    if (manager == 0) return 0;
    uintptr_t created = 0;
    const uintptr_t message = CallGameFunction<uintptr_t, 0x004E3F90>(
        static_cast<uintptr_t>(24), GameConstant<const char*>(0x0100EA68),
        static_cast<uintptr_t>(0), static_cast<uintptr_t>(0), static_cast<uintptr_t>(0),
        static_cast<uintptr_t>(0));
    if (message != 0) {
        created = static_cast<uintptr_t>(
            CallGameMethod<int, 0x0077F5A0>(reinterpret_cast<void*>(message), src));
        if (created != 0)
            CallGameVirtual<void, 1>(reinterpret_cast<void*>(created));
    }
    const int32_t result = CallGameVirtual<int, 7>(
        reinterpret_cast<void*>(manager), static_cast<int>(0xE777FB0Du),
        static_cast<int32_t>(created), 0, 0);
    if (created != 0)
        return CallGameVirtual<int, 2>(reinterpret_cast<void*>(created));
    return result;
}

// TS3W.exe 0x00875950.
char __fastcall CopyFileRetry00875950(void* self, void* /*edx*/, wchar_t* lpFileName,
                                      uint8_t* outError, int64_t* deadline)
{
    const wchar_t* source =
        reinterpret_cast<const wchar_t*>(reinterpret_cast<uintptr_t>(self) + 0x268);
    for (;;) {
        Sleep(500);
        if (static_cast<uint8_t>(
                CallGameFunction<uint8_t, 0x004DBBF0>(source, lpFileName, static_cast<char>(1))) != 0)
            break;
        if (GetLastError() != 5) {
            CallGameFunction<void, 0x004DBBD0>(lpFileName);
            return 0;
        }
        *outError = static_cast<uint8_t>(
            CallGameFunction<int, 0x008992C0>(lpFileName, static_cast<int>(120), 0));
        int64_t now = 0;
        CallGameMethod<void, 0x004F9F40>(&now, static_cast<int32_t>(2));
        if (now - *deadline >= 120) {
            *outError = 1;
            return 0;
        }
    }
    const uint8_t ready = static_cast<uint8_t>(
        CallGameFunction<int, 0x008992C0>(lpFileName, static_cast<int>(120), 0));
    *outError = ready;
    if (ready == 0) return 1;
    CallGameFunction<void, 0x004DBBD0>(lpFileName);
    return 0;
}

// TS3W.exe 0x00987D50.
bool __fastcall IsEntryInTable00987D50(void* self, void* /*edx*/, int32_t key,
                                       int32_t, int32_t, int32_t)
{
    void* const map = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self) + 0x620);
    MapIterator iterator = {nullptr, nullptr};
    CallGameMethod<void, 0x00986600>(map, &iterator, &key);
    if (iterator.node == MapSentinelNode(map)) return false;
    const int32_t wanted = *reinterpret_cast<const int32_t*>(
        reinterpret_cast<uintptr_t>(iterator.node) + 0x10);
    if (wanted == 0) return false;
    const uintptr_t head = reinterpret_cast<uintptr_t>(self) + 0x0C;
    const uintptr_t first = *reinterpret_cast<const uintptr_t*>(head);
    if (first == head) return false;
    const uintptr_t node = first + 8;
    if (node == 0) return false;
    const uintptr_t begin = *reinterpret_cast<const uintptr_t*>(node + 4);
    const uintptr_t end = *reinterpret_cast<const uintptr_t*>(node + 8);
    for (uintptr_t it = begin; it != end; it += 4) {
        if (*reinterpret_cast<const int32_t*>(it) == wanted) return true;
    }
    return false;
}

// TS3W.exe 0x00987E40.
int32_t __fastcall LookupEntryField00987E40(void* self, void* /*edx*/, int32_t key)
{
    void* const map = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self) + 0x24);
    MapIterator iterator = {nullptr, nullptr};
    CallGameMethod<void, 0x00D38B00>(map, &iterator, &key);
    if (iterator.node == MapSentinelNode(map)) return 0;
    return *reinterpret_cast<const int32_t*>(
        reinterpret_cast<uintptr_t>(iterator.node) + 4);
}

// TS3W.exe 0x009886E0. For a null list the original returns whatever EAX held at entry
// (no caller reads it); this rewrite returns 0 there.
int32_t __fastcall FreeNodeList009886E0(void* self, void* /*edx*/, ScratchListNode* node)
{
    int32_t result = 0;
    ScratchListNode* current = node;
    while (current != nullptr) {
        FreeNodeList009886E0(self, nullptr, current->child);
        ScratchListNode* next = current->next;
        if (current->owned != nullptr)
            Free4E3F70(current->owned);
        result = static_cast<int32_t>(
            reinterpret_cast<uintptr_t>(Free4E3F70(current)));
        current = next;
    }
    return result;
}

// TS3W.exe 0x009893C0.
char __fastcall EnsureTempDbCache009893C0(void* self, void* /*edx*/)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    const uintptr_t list = base + 0x0C;
    if (*reinterpret_cast<const uintptr_t*>(list) != list ||
        *reinterpret_cast<const int32_t*>(base + 0xD8) != 0)
        return 1;
    void* const directory = *reinterpret_cast<void* const*>(base + 0xE0);
    if (static_cast<uint8_t>(CallGameFunction<uint8_t, 0x004DCD40>(directory)) == 0)
        return 0;
    ScratchWideString path;
    SetEmptyWideString(&path, GameConstant<void*>(0x01046FBC));
    CallGameFunction<int, 0x0040E920>(
        &path, GameConstant<const wchar_t*>(0x0101A6F0), directory,
        GameGlobal<void*>(0x0117E298));
    void* const text = path.begin;
    if (static_cast<uint8_t>(CallGameFunction<uint8_t, 0x004DBBA0>(text)) != 0)
        CallGameFunction<void, 0x004DBBD0>(text);
    const char result = static_cast<char>(
        CallGameMethod<int, 0x00987350>(self, &path, static_cast<int32_t>(1)));
    FreeHeapIfOwned(&path, text);
    return result;
}

// TS3W.exe 0x0098AE00. The map wipe after +0x6C is cleared is dead code in the original
// (the second test re-reads the just-zeroed word), so EAX always ends as the +0x50
// vector begin; the block is kept for shape.
void* __fastcall ResetContentVectors0098AE00(int32_t* self, void* /*edx*/)
{
    if (self[0] != self[1]) self[1] = self[0];
    if (self[4] != self[5]) {
        CallGameMethod<void, 0x0073ECC0>(self + 4, self[4], self[5]);
        self[5] = self[4];
    }
    if (self[8] != self[9]) {
        DestroyRecordRange(static_cast<uintptr_t>(static_cast<uint32_t>(self[8])),
                           static_cast<uintptr_t>(static_cast<uint32_t>(self[9])));
        self[9] = self[8];
    }
    if (self[12] != self[13]) self[13] = self[12];
    if (self[16] != self[17]) self[17] = self[16];
    if (self[20] != self[21]) {
        CallGameMethod<void, 0x0073ECC0>(self + 20, self[20], self[21]);
        self[21] = self[20];
    }
    void* result = reinterpret_cast<void*>(static_cast<uintptr_t>(self[20]));
    if (self[27] != 0) {
        CallGameMethod<void, 0x0071F3B0>(self + 24, self[25], self[26]);
        self[27] = 0;
        if (self[27] != 0) {
            uint8_t* const map = reinterpret_cast<uint8_t*>(self) + 0x80;
            *reinterpret_cast<uintptr_t*>(map + 4) =
                reinterpret_cast<uintptr_t>(map + 4);
            *reinterpret_cast<uintptr_t*>(map + 8) =
                reinterpret_cast<uintptr_t>(map + 4);
            *reinterpret_cast<uintptr_t*>(map + 12) = 0;
            map[0x10] = 0;
            *reinterpret_cast<uintptr_t*>(map + 0x14) = 0;
            return map + 4;
        }
    }
    return result;
}

// TS3W.exe 0x0098C000.
int32_t __fastcall BuildWorldPath0098C000(void* self, void* /*edx*/, ScratchWideString* out,
                                          int32_t mode)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    SetEmptyWideString(out, GameConstant<void*>(0x01047200));
    for (int32_t index = 0;; ++index) {
        void* const extension = mode == 1 ? GameGlobal<void*>(0x0117E2A4)
                                          : GameGlobal<void*>(0x0117E2A0);
        CallGameFunction<int, 0x0040E920>(
            out, GameConstant<const wchar_t*>(0x010471B8),
            *reinterpret_cast<void* const*>(base + 0xE0),
            GameGlobal<void*>(0x0117E29C), static_cast<intptr_t>(index), extension);
        if (static_cast<uint8_t>(
                CallGameFunction<uint8_t, 0x004DBBA0>(out->begin)) == 0)
            break;
    }
    const char16_t* const text = static_cast<const char16_t*>(out->begin);
    const char16_t* scan = text;
    while (*scan != 0) ++scan;
    const int32_t assigned = CallGameMethod<int, 0x00408E20>(out, text, scan);
    FreeHeapIfOwned(out, out->begin);
    return assigned;
}

// TS3W.exe 0x0098D430.
int32_t __fastcall GetWorldDir0098D430(int32_t* self, void* /*edx*/, int32_t a2)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    if (*reinterpret_cast<const int32_t*>(base + 0xD8) != 0 ||
        static_cast<uint8_t>(
            CallGameFunction<uint8_t, 0x004DCD40>(
                *reinterpret_cast<void* const*>(base + 0xE0))) == 0)
        return 0;
    ScratchWideString path;
    SetEmptyWideString(&path, GameConstant<void*>(0x01047200));
    BuildWorldPath0098C000(self, nullptr, &path, a2);
    if (static_cast<uint8_t>(
            CallGameMethod<int, 0x00987350>(self, &path, a2)) == 0) {
        FreeHeapIfOwned(&path, path.begin);
        return 0;
    }
    const int32_t result =
        *reinterpret_cast<const int32_t*>(base + 0x10) + 8;
    FreeHeapIfOwned(&path, path.begin);
    return result;
}

// TS3W.exe 0x0098DD00. The walk starts at the SECOND list node: the first node is the
// world entry the single-node case resolves through 0x0098D430 (EA quirk kept).
int32_t __fastcall FindContent0098DD00(int32_t* self, void* /*edx*/, int32_t,
                                       int32_t a3)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    const uintptr_t head = base + 0x0C;
    if (*reinterpret_cast<const uintptr_t*>(head) == head) EnsureTempDbCache009893C0(self, nullptr);
    int32_t count = 0;
    for (uintptr_t node = *reinterpret_cast<const uintptr_t*>(head); node != head;
         node = *reinterpret_cast<const uintptr_t*>(node))
        ++count;
    if (count == 1) return GetWorldDir0098D430(self, nullptr, a3);
    for (uintptr_t node = *reinterpret_cast<const uintptr_t*>(
             *reinterpret_cast<const uintptr_t*>(head));
         node != head; node = *reinterpret_cast<const uintptr_t*>(node)) {
        if (a3 != *reinterpret_cast<const int32_t*>(node + 0x1C)) continue;
        void* const object = *reinterpret_cast<void* const*>(node + 8);
        void* const path = reinterpret_cast<void*>(
            CallGameVirtual<uintptr_t, 10>(object));
        if (static_cast<uint32_t>(CallGameFunction<int, 0x004DBC60>(path)) <
            static_cast<uint32_t>(*reinterpret_cast<const int32_t*>(base + 0xDC))) {
            CallGameVirtual<void, 9>(object);
            if (static_cast<uint32_t>(CallGameFunction<int, 0x004DBC60>(path)) <
                static_cast<uint32_t>(*reinterpret_cast<const int32_t*>(base + 0xDC)))
                return static_cast<int32_t>(node + 8);
        }
    }
    return GetWorldDir0098D430(self, nullptr, a3);
}

// TS3W.exe 0x0098FD70.
char __fastcall CommitWorldPackage0098FD70(int32_t* self, void* /*edx*/, int32_t a2)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    const uintptr_t head = base + 0x0C;
    if (*reinterpret_cast<const uintptr_t*>(head) == head) return 0;
    int32_t count = 0;
    for (uintptr_t node = *reinterpret_cast<const uintptr_t*>(head); node != head;
         node = *reinterpret_cast<const uintptr_t*>(node))
        ++count;
    const uintptr_t first = *reinterpret_cast<const uintptr_t*>(head) + 8;
    int32_t* record = nullptr;
    if (count == 1) {
        const int32_t value = GetWorldDir0098D430(self, nullptr, a2);
        if (value == 0) return 0;
        record = reinterpret_cast<int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(value)));
    } else {
        const int32_t value = FindContent0098DD00(
            self, nullptr, *reinterpret_cast<const int32_t*>(first), a2);
        if (value == 0) return 0;
        record = reinterpret_cast<int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(value)));
    }
    CallGameFunction<int, 0x007D8F00>(
        reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(*record))),
        *reinterpret_cast<void* const*>(first), nullptr, static_cast<char>(1), nullptr,
        nullptr);
    int32_t dest = 0;
    CallGameFunction<int32_t*, 0x00D15C50>(
        &dest, *reinterpret_cast<const int32_t*>(first + 4),
        *reinterpret_cast<const int32_t*>(first + 8),
        static_cast<int32_t>(reinterpret_cast<uintptr_t>(record + 1)));
    CallGameMethod<void, 0x00985700>(self, record);
    *reinterpret_cast<uintptr_t*>(first + 8) = *reinterpret_cast<uintptr_t*>(first + 4);
    CallGameMethod<void, 0x00989490>(self);
    return 1;
}

// TS3W.exe 0x0099C040.
bool VerifyContent0099C040(int32_t* vector)
{
    if (vector[0] == vector[1]) return true;
    void* manager = ts3::resourcesystem::GetDirectoryManager();
    if (manager == nullptr) return false;
    uint32_t zeroKey[4] = {};
    CallGameMethod<void, 0x00500A30>(zeroKey);
    for (uintptr_t element = static_cast<uintptr_t>(static_cast<uint32_t>(vector[0]));
         element != static_cast<uintptr_t>(static_cast<uint32_t>(vector[1]));
         element += 16) {
        if (static_cast<uint8_t>(CallGameFunction<uint8_t, 0x00500AC0>(
                reinterpret_cast<const void*>(element), zeroKey)) != 0)
            continue;
        if (static_cast<uint8_t>(CallGameMethod<int, 0x0072E7F0>(
                manager, reinterpret_cast<void*>(element))) == 0)
            continue;
        if (static_cast<uint8_t>(CallGameMethod<int, 0x0072E8A0>(
                manager, reinterpret_cast<void*>(element))) == 0)
            return false;
    }
    return true;
}

// TS3W.exe 0x0099C140.
bool VerifyContentWrapper0099C140(int32_t object)
{
    if (object == 0) return false;
    return VerifyContent0099C040(
        reinterpret_cast<int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(object)) + 0xBC));
}

// TS3W.exe 0x009A90B0.
int32_t BuildInstalledWorldPath009A90B0(int32_t out)
{
    void* const service = reinterpret_cast<void*>(
        CallGameFunction<uintptr_t, 0x0072F1C0>());
    const int32_t version = CallGameFunction<int, 0x0058A900>(
        GameGlobal<void*>(0x011808B0), GameGlobal<void*>(0x011808B4),
        GameGlobal<void*>(0x011808B8));
    const wchar_t* path = CallGameVirtual<const wchar_t*, 9>(service, version);
    return CallGameFunction<int, 0x0040E920>(
        out, GameConstant<const wchar_t*>(0x00FD83A4), path, version);
}

// TS3W.exe 0x009A9970.
int32_t CommitWorldFile009A9970(const wchar_t* lpFileName, char a2)
{
    if (lpFileName == nullptr ||
        static_cast<uint8_t>(CallGameFunction<uint8_t, 0x004DBBA0>(lpFileName)) == 0)
        return 1;
    if (a2 == 0) {
        if (static_cast<uint8_t>(CallGameFunction<uint8_t, 0x004DBBD0>(lpFileName)) != 0)
            return 4096;
        return 1;
    }
    char16_t itemName[520];
    ConstructWideFromPath004EDA20(itemName, nullptr,
                                  reinterpret_cast<const char16_t*>(lpFileName));
    ScratchWideString source;
    SetEmptyWideString(&source, GameConstant<void*>(0x0104B890));
    ScratchWideString target;
    SetEmptyWideString(&target, GameConstant<void*>(0x0104B8E0));
    BuildInstalledWorldPath009A90B0(
        static_cast<int32_t>(reinterpret_cast<uintptr_t>(&source)));
    void* const sourcePath = source.begin;
    if (EnsureDirectory004DCFF0(static_cast<const wchar_t*>(sourcePath)) == 0) {
        CallGameFunction<void, 0x004DBBD0>(lpFileName);
        FreeHeapIfOwned(&source, source.begin);
        return 1;
    }
    wchar_t formatted[256];
    CallGameMethod<void, 0x004ED670>(itemName, formatted, static_cast<int32_t>(256));
    CallGameFunction<int, 0x0040E920>(&target, GameConstant<const wchar_t*>(0x0104B874),
                                      sourcePath, formatted);
    if (static_cast<uint8_t>(
            CallGameFunction<uint8_t, 0x004DBBA0>(target.begin)) != 0) {
        CallGameFunction<void, 0x004DBBD0>(lpFileName);
        FreeHeapIfOwned(&source, source.begin);
        FreeHeapIfOwned(&target, target.begin);
        return 15;
    }
    if (static_cast<uint8_t>(CallGameFunction<uint8_t, 0x004DBBF0>(
            lpFileName, target.begin, static_cast<char>(1))) != 0) {
        FreeHeapIfOwned(&source, source.begin);
        FreeHeapIfOwned(&target, target.begin);
        return 4096;
    }
    CallGameFunction<void, 0x004DBBD0>(lpFileName);
    FreeHeapIfOwned(&source, source.begin);
    FreeHeapIfOwned(&target, target.begin);
    return 1;
}

// TS3W.exe 0x009B00D0.
bool CompareTimestamps009B00D0(const void* aRaw, const void* bRaw)
{
    const uint32_t* a = static_cast<const uint32_t*>(aRaw);
    const uint32_t* b = static_cast<const uint32_t*>(bRaw);
    const bool aZero = a[0] == 0 && a[1] == 0 && a[2] == 0 && a[3] == 0;
    const bool bZero = b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 0;
    if (aZero || bZero) return true;
    return a[3] / 1000u >= b[3] / 1000u;
}

// TS3W.exe 0x009B0150.
bool __fastcall CompareTimestamps009B0150(const void* self, void* /*edx*/, const void* b)
{
    return CompareTimestamps009B00D0(self, b);
}

// TS3W.exe 0x009C6800.
char __fastcall SaveSims3Pack009C6800(void* self, void* /*edx*/, int16_t a2,
                                      wchar_t* src)
{
    const uintptr_t base = reinterpret_cast<uintptr_t>(self);
    if (*reinterpret_cast<const int32_t*>(base + 0xB4) != 1 ||
        *reinterpret_cast<const uint8_t*>(base + 0x114) == 0 ||
        *reinterpret_cast<const int32_t*>(base + 0x110) == 0 ||
        *reinterpret_cast<const int32_t*>(base + 0x118) == 0)
        return 0;
    if (EnsureDirectory004DCFF0(src) == 0) return 0;
    void* const item = reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(
        CallGameMethod<int, 0x009C5EE0>(self, static_cast<int32_t>(a2)))));
    if (item == nullptr) return 0;
    ScratchWideString path;
    SetEmptyWideString(&path, GameConstant<void*>(0x0104FBD4));
    CallGameMethod<void, 0x00408890>(&path, GameConstant<void*>(0x0104FBD4));
    CallGameMethod<void, 0x00409010>(&path, src);
    EnsureTrailingSeparator004EDD80(&path);
    CallGameMethod<void, 0x004F1F50>(
        &path, *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(item) + 0x28));
    uint8_t stream[552] = {};
    CallGameMethod<void, 0x004DB970>(stream, path.begin);
    if (static_cast<uint8_t>(CallGameMethod<int, 0x004DB4C0>(stream, 3, 2, 1, 0)) == 0) {
        CallGameMethod<void, 0x004DB9D0>(stream);
        FreeHeapIfOwned(&path, path.begin);
        return 0;
    }
    const char result = static_cast<char>(
        CallGameMethod<int, 0x009C6760>(self, static_cast<int32_t>(a2), stream));
    CallGameMethod<void, 0x004DB600>(stream);
    CallGameMethod<void, 0x004DB9D0>(stream);
    FreeHeapIfOwned(&path, path.begin);
    return result;
}

// TS3W.exe 0x009CA860.
int32_t* ConstructCallback009CA860(int32_t* dest, int32_t a2, int32_t a3, int32_t a4,
                                   int32_t a5, void* callback, int32_t a7)
{
    if (a2 != a3) {
        const uintptr_t target = static_cast<uintptr_t>(static_cast<uint32_t>(a4 + a7));
#if defined(_M_IX86)
        using Callback = void(__thiscall*)(uintptr_t, uintptr_t);
        for (uintptr_t element = static_cast<uintptr_t>(static_cast<uint32_t>(a2));
             element != static_cast<uintptr_t>(static_cast<uint32_t>(a3)); element += 16)
            reinterpret_cast<Callback>(callback)(target, element);
#endif
    }
    dest[0] = a4;
    dest[1] = a5;
    dest[2] = static_cast<int32_t>(reinterpret_cast<uintptr_t>(callback));
    dest[3] = a7;
    return dest;
}

// TS3W.exe 0x009CAE60. The original builds the context in the argument slots it just
// pushed and reads its second word from an uninitialized stack slot (EA bug); this
// rewrite zeroes that word. Both early exits also leave EAX untouched, so the original
// returns whatever the caller had there (an EA bug); this rewrite returns null.
void* __fastcall CallbackSetup009CAE60(int32_t* self, void* /*edx*/, int32_t* a2)
{
    if (a2 == nullptr) return nullptr;
    const int32_t context = self[15];
    if (context == 0) return nullptr;
    int32_t built[4] = {context, 0, static_cast<int32_t>(0x0051B880), 0};
    ConstructCallback009CA860(built, a2[0], a2[1], context, built[1],
                              reinterpret_cast<void*>(uintptr_t{0x0051B880}), 0);
    return built;
}

// TS3W.exe 0x00D15C50.
int32_t* AppendRecords00D15C50(int32_t* dest, int32_t a2, int32_t a3, int32_t a4)
{
    const uintptr_t vector = static_cast<uintptr_t>(static_cast<uint32_t>(a4));
    if (a2 != a3) {
        for (uintptr_t element = static_cast<uintptr_t>(static_cast<uint32_t>(a2));
             element != static_cast<uintptr_t>(static_cast<uint32_t>(a3));
             element += 4) {
            const uintptr_t current = *reinterpret_cast<uintptr_t*>(vector + 4);
            if (current >= *reinterpret_cast<uintptr_t*>(vector + 8)) {
                CallGameMethod<void, 0x008621C0>(
                    reinterpret_cast<void*>(vector),
                    reinterpret_cast<void*>(current),
                    reinterpret_cast<void*>(element));
            } else {
                *reinterpret_cast<uintptr_t*>(vector + 4) = current + 4;
                if (current != 0)
                    *reinterpret_cast<int32_t*>(current) =
                        *reinterpret_cast<const int32_t*>(element);
            }
        }
    }
    *dest = a4;
    return dest;
}

}  // namespace ts3::scratch