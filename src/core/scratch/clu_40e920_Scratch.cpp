// TS3W.exe cluster unit clu_40e920_Scratch rewritten for x86/x64 (see the header for the
// per-function contracts). Every function keeps the original's address in the comment
// above it; dependencies that are not rewritten yet go through the bridge in layout.h.
// The game's static wide strings and pointer globals are referenced at their TS3W.exe
// addresses on x86 (they never change at runtime); on x64 the build only needs to
// compile, so those addresses resolve to the empty string (AbortNotRewritten covers the
// game calls).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "ts3/scratch/clu_40e920_Scratch.h"

#include <cstdarg>
#include <cstdint>
#include <cwchar>

#include "ts3/scratch/clu_408e20_Scratch.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"

namespace ts3::scratch {
namespace {

// ---- game data addresses (x86) ---------------------------------------------------------
#if defined(_M_IX86)
inline void* EmptyBegin() { return reinterpret_cast<void*>(0x011C7E54u); }
inline void* EmptyCap() { return reinterpret_cast<void*>(0x011C7E56u); }
inline const void* StrAt(uintptr_t address) { return reinterpret_cast<const void*>(address); }
inline const void* StrPtrAt(uintptr_t address)
{
    return *reinterpret_cast<const void* const*>(address);
}
#else
inline void* EmptyBegin() { return nullptr; }
inline void* EmptyCap() { return nullptr; }
inline const void* StrAt(uintptr_t) { return nullptr; }
inline const void* StrPtrAt(uintptr_t) { return nullptr; }
#endif

inline WideString EmptyWideStr(const void* allocName)
{
    return WideString{EmptyBegin(), EmptyBegin(), EmptyCap(), const_cast<void*>(allocName)};
}

// The game's "(cap - begin) & 0xFFFFFFFE > 2" ownership test used by its string helpers.
inline bool OwnsMasked(const WideString& text)
{
    const int32_t span = static_cast<int32_t>(reinterpret_cast<intptr_t>(text.cap) -
                                              reinterpret_cast<intptr_t>(text.begin));
    return (span & 0xFFFFFFFE) > 2 && text.begin != nullptr;
}
inline void FreeOwnedMasked(WideString& text)
{
    if (OwnsMasked(text))
        Free4E3F70(text.begin);
}

// Calls a __stdcall game function (the bridge templates only cover cdecl/thiscall).
template <class R, unsigned kAddress, class... Args>
R CallGameStdcall(Args... args)
{
#if defined(_M_IX86)
    using Function = R(__stdcall*)(Args...);
    return reinterpret_cast<Function>(static_cast<uintptr_t>(kAddress))(args...);
#else
    ((void)args, ...);
    AbortNotRewritten("a game stdcall function");
    return R{};
#endif
}

// The unit's own XML writer dispatchers (0x007DA190/0x007DA1D0/0x007DA1F0).
inline char OpenKey(void* writer, const void* key)
{
    return XmlOpenTag7DA190(writer, nullptr, key);
}
inline char WriteValue(void* writer, const void* value)
{
    return XmlValue7DA1D0(writer, nullptr, value);
}
inline char CloseKey(void* writer, const void* key)
{
    return XmlCloseTag7DA1F0(writer, nullptr, key);
}

// 0x004F3890: format the wide string at `src` into `out` (a game helper that returns the
// output pointer the game then reads its begin field from).
void EmitFormattedField(void* writer, const void* src)
{
    WideString out = EmptyWideStr(nullptr);
    WideString* formatted = CallGameFunction<WideString*, 0x004F3890>(
        &out, reinterpret_cast<const WideString*>(src));
    WriteValue(writer, *reinterpret_cast<void**>(formatted));
    FreeOwnedMasked(out);
}

// 0x011C7E55: the game's short empty-string cap (begin + 1) used by 0x009B2420's entry
// string. The plain (cap - begin) > 1 ownership test below therefore never frees it.
inline void* EmptyCapShort()
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x011C7E55u);
#else
    return nullptr;
#endif
}
inline WideString EmptyWideStrShort(const void* allocName)
{
    return WideString{EmptyBegin(), EmptyBegin(), EmptyCapShort(), const_cast<void*>(allocName)};
}

// The game's *unmasked* ownership test used by 0x009B2420 for its entry/lookup strings:
// (cap - begin) > 1, no 0xFFFFFFFE mask (unlike OwnsMasked above).
inline bool OwnsPlain(const WideString& text)
{
    const int32_t span = static_cast<int32_t>(reinterpret_cast<intptr_t>(text.cap) -
                                              reinterpret_cast<intptr_t>(text.begin));
    return span > 1 && text.begin != nullptr;
}

// The game's XML writer builder object used by 0x009C7A20: 0x007DBA60 constructs it and
// the 0x007DA1xx dispatchers read +0x0C (inner writer) and +0x48 (state word).
struct alignas(16) XmlBuilder {
    uint8_t bytes[0x50];
};

// Wide-string strlen for the game's wchar_t buffers (kept in-word to match the original
// scans; the game reads 16-bit units).
inline const wchar_t* WideEnd(const wchar_t* text)
{
    const wchar_t* cursor = text;
    while (*cursor)
        ++cursor;
    return cursor;
}

}  // namespace

// ---------------------------------------------------------------------------------------
// 0x0040E920.
WideString* __cdecl FormatWide0040E920(WideString* self, const wchar_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    self->end = self->begin;
    CallGameMethod<void, 0x0040D400>(self, fmt, args);
    va_end(args);
    return self;
}

// ---------------------------------------------------------------------------------------
// 0x00500AF0.
int __cdecl CompareRecords00500AF0(const void* a, const void* b)
{
    const uint32_t* x = static_cast<const uint32_t*>(a);
    const uint32_t* y = static_cast<const uint32_t*>(b);
    return x[0] != y[0] || x[1] != y[1] || x[2] != y[2] || x[3] != y[3];
}

// ---------------------------------------------------------------------------------------
// 0x00501D70: the 0x00501B10 call is (buffer, &cursor, seed); the generator writes the
// generated bytes at `buffer` and the end pointer into the second argument.
void* __stdcall GeneratePackageId501D70(uint16_t* out, uint16_t** endOut, int seed)
{
    uint8_t buf[132];
    uint8_t* cursor = buf;
    CallGameStdcall<void, 0x00501B10>(buf, &cursor, seed);
    for (const uint8_t* p = buf; p < cursor; ++out)
        *out = *p++;
    if (endOut)
        *endOut = out;
    *out = 0;
    return endOut;
}

// ---------------------------------------------------------------------------------------
// 0x0058A900.
int __cdecl ResolveVersion58A900(void* src, void* a2, int fallback)
{
    int found = 0;
    void* store = CallGameFunction<void*, 0x0058B880>();
    if (!store)
        return fallback;
    const char ok = CallGameMethod<char, 0x0058C380>(store, src, a2, &found);
    const int result = found;
    if (!ok)
        return fallback;
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x007DA190.
char __fastcall XmlOpenTag7DA190(void* self, void*, const void* key)
{
    uint8_t* base = static_cast<uint8_t*>(self);
    if (*reinterpret_cast<uint32_t*>(base + 0x48) != 2)
        return 0;
    void* inner = *reinterpret_cast<void**>(base + 0x0C);
    return CallGameVirtual<char, 2>(inner, key);
}

// ---------------------------------------------------------------------------------------
// 0x007DA1D0.
char __fastcall XmlValue7DA1D0(void* self, void*, const void* value)
{
    uint8_t* base = static_cast<uint8_t*>(self);
    if (*reinterpret_cast<uint32_t*>(base + 0x48) != 2)
        return 0;
    void* inner = *reinterpret_cast<void**>(base + 0x0C);
    return CallGameVirtual<char, 8>(inner, value);
}

// ---------------------------------------------------------------------------------------
// 0x007DA1F0.
char __fastcall XmlCloseTag7DA1F0(void* self, void*, const void* key)
{
    uint8_t* base = static_cast<uint8_t*>(self);
    if (*reinterpret_cast<uint32_t*>(base + 0x48) != 2)
        return 0;
    void* inner = *reinterpret_cast<void**>(base + 0x0C);
    return CallGameVirtual<char, 3>(inner, key);
}

// ---------------------------------------------------------------------------------------
// 0x0087C960.
int __stdcall FormatLauncherId87C960(void* self, WideString* out)
{
    if (out->begin != out->end) {
        *static_cast<uint16_t*>(out->begin) = 0;
        out->end = out->begin;
    }
    uint8_t* base = static_cast<uint8_t*>(self);
    const wchar_t* text = *reinterpret_cast<const wchar_t**>(base + 0x88);
    if (!text)
        text = *reinterpret_cast<const wchar_t**>(base + 0x98);
    if (text) {
        const size_t length = std::wcslen(text);
        for (size_t i = 0; i < length; ++i) {
            const uint16_t c = static_cast<uint16_t>(text[i]);
#if defined(_M_IX86)
            const uint8_t kind = *reinterpret_cast<const uint8_t*>(0x0114CA30u + c);
#else
            const uint8_t kind = 0;
#endif
            if ((c < 0x100u && (kind & 0xD0) != 0) || c == 32)
                CallGameMethod<void, 0x00412CC0>(out, c);
        }
    }
    const int32_t span = static_cast<int32_t>(reinterpret_cast<intptr_t>(out->end) -
                                              reinterpret_cast<intptr_t>(out->begin));
    if (out->begin == out->end || (span & 0xFFFFFFFE) == 0) {
        const int64_t id = CallGameFunction<int64_t, 0x007CFC00>(0);
        FormatWide0040E920(out, static_cast<const wchar_t*>(StrAt(0x0102B76Cu)), id);
    }
    const wchar_t* suffix = static_cast<const wchar_t*>(StrAt(0x00FCBC44u));
    const wchar_t* cursor = suffix;
    while (*cursor)
        ++cursor;
    return CallGameMethod<int, 0x00408B90>(out, suffix, cursor);
}

// ---------------------------------------------------------------------------------------
// 0x0099AE70: exact decision tree from the disassembly (pointer globals 0x01180810..
// 0x01180858, 0x0104A66C for a miss).
const wchar_t* __cdecl ContentTypeName99AE70(int hash)
{
#if defined(_M_IX86)
    const auto name = [](uintptr_t address) -> const wchar_t* {
        return *reinterpret_cast<const wchar_t* const*>(address);
    };
    const wchar_t* const unknown = reinterpret_cast<const wchar_t*>(0x0104A66Cu);
#else
    const auto name = [](uintptr_t) -> const wchar_t* { return nullptr; };
    const wchar_t* const unknown = L"unknown";
#endif
    if (hash > 0x063261DA) {
        if (hash > static_cast<int>(0xC4FB4F1u)) {
            if (hash > 0x3A65AF29) {
                if (hash == 0x436FEE4C)
                    return name(0x01180814u);  // lot
                return unknown;
            }
            if (hash == 0x3A65AF29)
                return name(0x01180834u);  // pethaircolorinfo
            if (hash == static_cast<int>(0xC4FB4F9u))
                return name(0x01180854u);  // passportchallenge
            if (hash == 0x2F7D0004)
                return name(0x01180840u);  // screenShot
            return unknown;
        }
        if (hash == static_cast<int>(0xC4FB4F1u))
            return name(0x01180850u);  // dynamicchallenge
        if (hash > 0x06BC1F92) {
            if (hash == 0x0B3A415E)
                return name(0x01180818u);  // household
            return unknown;
        }
        if (hash == 0x06BC1F92)
            return name(0x01180844u);  // world
        if (hash == 0x063261F0)
            return name(0x01180830u);  // cascolorinfo
        if (hash == 0x06326213)
            return name(0x0118082Cu);  // objectcolorinfo
        return unknown;
    }
    if (hash == 0x063261DA)
        return name(0x01180828u);  // haircolorinfo
    if (hash > 0x034AEECB) {
        if (hash == 0x04F88964)
            return name(0x01180810u);  // sim
        if (hash == 0x0354796A)
            return name(0x01180848u);  // skin
        if (hash == 0x044AE110)
            return name(0x01180838u);  // preset
        if (hash == 0x05F8F5D9)
            return name(0x01180820u);  // object
        return unknown;
    }
    if (hash == 0x034AEECB)
        return name(0x0118081Cu);  // CASpart
    if (hash > 0x025ED6F4) {
        if (hash == 0x0341ACC9)
            return name(0x01180824u);  // pattern
        return unknown;
    }
    if (hash == 0x025ED6F4)
        return name(0x0118084Cu);  // outfit
    if (hash == static_cast<int>(0xB1CC1AF6u))
        return name(0x0118083Cu);  // video
    if (hash == static_cast<int>(0xDEA2951Cu))
        return name(0x01180858u);  // coatset
    return unknown;
}

// ---------------------------------------------------------------------------------------
// 0x009B59F0.
char __fastcall WriteOutfitCategory9B59F0(void* self, void*, void* writer)
{
    uint32_t* record = static_cast<uint32_t*>(self);
    const void* groupKey = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(record) + 0x10);
    OpenKey(writer, groupKey);
    uint32_t* cursor = *reinterpret_cast<uint32_t**>(record);
    uint32_t* end = *reinterpret_cast<uint32_t**>(reinterpret_cast<uint8_t*>(record) + 4);
    if (cursor != end) {
        do {
            WideString text = EmptyWideStr(StrAt(0x0104EAA0u));
            const uint64_t hash =
                static_cast<uint64_t>(cursor[0]) | (static_cast<uint64_t>(cursor[1]) << 32);
            FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0104EAE4u)), hash);
            OpenKey(writer, StrPtrAt(0x01180B50u));
            WriteValue(writer, text.begin);
            CloseKey(writer, StrPtrAt(0x01180B50u));
            FreeOwnedMasked(text);
            cursor += 2;
        } while (cursor != end);
    }
    return CloseKey(writer, groupKey);
}

// ---------------------------------------------------------------------------------------
// 0x009B5AC0.
char __fastcall WriteManifestValue9B5AC0(void* self, void*, void* writer)
{
    WideString text = EmptyWideStr(StrAt(0x0104EAF8u));
    OpenKey(writer, StrPtrAt(0x01180E38u));
    OpenKey(writer, StrPtrAt(0x01180B98u));
    WriteValue(writer, *reinterpret_cast<void**>(self));
    CloseKey(writer, StrPtrAt(0x01180B98u));
    OpenKey(writer, StrPtrAt(0x01180C50u));
    WriteValue(writer, *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(self) + 0x10));
    CloseKey(writer, StrPtrAt(0x01180C50u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                       *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(self) + 0x20));
    OpenKey(writer, StrPtrAt(0x01180C08u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrPtrAt(0x01180C08u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                       *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(self) + 0x28));
    OpenKey(writer, StrPtrAt(0x01180C18u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrPtrAt(0x01180C18u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                       *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(self) + 0x24));
    OpenKey(writer, StrPtrAt(0x01180C10u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrPtrAt(0x01180C10u));
    CloseKey(writer, StrPtrAt(0x01180E38u));
    FreeOwnedMasked(text);
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x009C11E0.
void* __cdecl AssignPackageId9C11E0(WideString* out, void* unused)
{
    (void)unused;
    uint16_t buffer[130];
    uint16_t* end = nullptr;
    GeneratePackageId501D70(buffer, &end, 16);
    uint16_t* cursor = buffer;
    while (*cursor)
        ++cursor;
    return AssignWide408E20(out, nullptr, buffer, cursor);
}

// ---------------------------------------------------------------------------------------
// 0x009C5960.
char __stdcall CopyStreamToStream9C5960(void* source, void* sink)
{
    if (!source || !sink)
        return 0;
    uint8_t buffer[0x10000];
    while (CallGameVirtual<uint32_t, 11>(source) != 0) {
        const int read = CallGameVirtual<int, 12>(source, buffer, 0x10000);
        if (read > 0)
            CallGameVirtual<void, 14>(sink, buffer, read);
    }
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x009C6120.
char __fastcall WriteChallengeFields9C6120(void* self, void*, void* writer)
{
    uint8_t* record = static_cast<uint8_t*>(self);
    OpenKey(writer, StrAt(0x0104FB7Cu));
    EmitFormattedField(writer, record + 0x124);
    CloseKey(writer, StrAt(0x0104FB7Cu));

    OpenKey(writer, StrAt(0x0104FB58u));
    EmitFormattedField(writer, record + 0x134);
    CloseKey(writer, StrAt(0x0104FB58u));

    OpenKey(writer, StrAt(0x0104FB38u));
    EmitFormattedField(writer, record + 0x174);
    CloseKey(writer, StrAt(0x0104FB38u));

    OpenKey(writer, StrAt(0x0104FB18u));
    EmitFormattedField(writer, record + 0x184);
    CloseKey(writer, StrAt(0x0104FB18u));

    OpenKey(writer, StrAt(0x0104FAF4u));
    for (uint8_t* element = *reinterpret_cast<uint8_t**>(record + 0x1A4);
         element != *reinterpret_cast<uint8_t**>(record + 0x1A8); element += 16) {
        OpenKey(writer, StrAt(0x0104DD40u));
        EmitFormattedField(writer, element);
        CloseKey(writer, StrAt(0x0104DD40u));
    }
    CloseKey(writer, StrAt(0x0104FAF4u));

    OpenKey(writer, StrAt(0x0104FAD4u));
    OpenKey(writer, StrAt(0x0104DCE4u));
    EmitFormattedField(writer, record + 0x144);
    CloseKey(writer, StrAt(0x0104DCE4u));
    CloseKey(writer, StrAt(0x0104FAD4u));

    OpenKey(writer, StrAt(0x0104FAA8u));
    OpenKey(writer, StrAt(0x0104DCE4u));
    EmitFormattedField(writer, record + 0x154);
    CloseKey(writer, StrAt(0x0104DCE4u));
    CloseKey(writer, StrAt(0x0104FAA8u));

    OpenKey(writer, StrAt(0x0104FA9Cu));
    for (uint8_t* element = *reinterpret_cast<uint8_t**>(record + 0x1B4);
         element != *reinterpret_cast<uint8_t**>(record + 0x1B8); element += 16) {
        OpenKey(writer, StrAt(0x0104DCE4u));
        EmitFormattedField(writer, element);
        CloseKey(writer, StrAt(0x0104DCE4u));
    }
    CloseKey(writer, StrAt(0x0104FA9Cu));

    OpenKey(writer, StrAt(0x0104FA94u));
    OpenKey(writer, StrAt(0x0104DCE4u));
    EmitFormattedField(writer, record + 0x194);
    CloseKey(writer, StrAt(0x0104DCE4u));
    CloseKey(writer, StrAt(0x0104FA94u));

    OpenKey(writer, StrAt(0x0104FA80u));
    for (uint8_t* element = *reinterpret_cast<uint8_t**>(record + 0x1C4);
         element != *reinterpret_cast<uint8_t**>(record + 0x1C8); element += 16) {
        OpenKey(writer, StrAt(0x0104DCA8u));
        EmitFormattedField(writer, element);
        CloseKey(writer, StrAt(0x0104DCA8u));
    }
    CloseKey(writer, StrAt(0x0104FA80u));

    OpenKey(writer, StrAt(0x0104FA60u));
    EmitFormattedField(writer, record + 0x164);
    return CloseKey(writer, StrAt(0x0104FA60u));
}

// ---------------------------------------------------------------------------------------
// 0x009B2420: when the record vector at +0x28/+0x2C is non-empty, format the launcher
// "entryStr" key from the pair (a3 != 0, a2), look the translated entry up through the
// game's 0x004F39F0 (returning its out string, assigned into the short-empty entry string
// via 0x00405A50), then write the entry into a TS3Pack-style stream opened from the file
// name at +0x28. Returns 1 only when the stream opens and the write runs.
char __fastcall WriteLauncherEntry9B2420(void* self, void*, int a2, char a3)
{
    uint8_t* base = static_cast<uint8_t*>(self);
    if (*reinterpret_cast<void**>(base + 0x28) == *reinterpret_cast<void**>(base + 0x2C))
        return 0;

    WideString key = EmptyWideStr(StrAt(0x0104CA54u));
    FormatWide0040E920(&key, static_cast<const wchar_t*>(StrAt(0x0104CA48u)), a3 != 0, a2);

    WideString found = EmptyWideStr(StrAt(0x0104CA14u));
    WideString entry = EmptyWideStrShort(StrAt(0x0104CA14u));
    CallGameFunction<void*, 0x004F39F0>(&found, &key);
    CallGameMethod<void, 0x00405A50>(&entry, found.begin, found.end);
    if (OwnsPlain(found))
        Free4E3F70(found.begin);

    alignas(16) uint8_t stream[0x228];
    CallGameMethod<char, 0x004DB970>(stream, *reinterpret_cast<const wchar_t**>(base + 0x28));
    if (!CallGameMethod<char, 0x004DB4C0>(stream, 2, 4, 1, 0)) {
        CallGameMethod<void, 0x004DB9D0>(stream);
        if (OwnsPlain(entry))
            Free4E3F70(entry.begin);
        if (OwnsMasked(key))
            Free4E3F70(key.begin);
        return 0;
    }
    CallGameMethod<char, 0x004DB7D0>(stream, static_cast<long long>(0x200000000LL));
    const int32_t size = static_cast<int32_t>(reinterpret_cast<intptr_t>(entry.end) -
                                              reinterpret_cast<intptr_t>(entry.begin));
    CallGameFunction<void, 0x004DDF50>(stream, entry.begin, size, 3);
    CallGameMethod<void, 0x004DB600>(stream);
    CallGameMethod<void, 0x004DB9D0>(stream);
    if (OwnsPlain(entry))
        Free4E3F70(entry.begin);
    if (OwnsMasked(key))
        Free4E3F70(key.begin);
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x009C7670: emit one packaged-file record as XML through the writer. Fields: +0x18/+0x1C
// (64-bit Crc), +0x20 (Length), +0x28 (Offset), +0x858 (content-type hash), +0x85C
// (thumbnail state), +0x864 (EPFlags), +8 (package-id record copied by 0x00500310, a dead
// store), +0x648 (metatags object). Always returns 1.
char __stdcall WritePackagedFileXml9C7670(void* record, void* writer, int offset)
{
    uint8_t* base = static_cast<uint8_t*>(record);
    OpenKey(writer, StrAt(0x0104FFA0u));
    WideString text = EmptyWideStr(StrAt(0x0104FF68u));
    OpenKey(writer, StrAt(0x0104FF94u));
    WriteValue(writer, *reinterpret_cast<void**>(base + 0x28));
    CloseKey(writer, StrAt(0x0104FF94u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x00FBF574u)),
                       *reinterpret_cast<uint32_t*>(base + 0x20));
    OpenKey(writer, StrAt(0x0104FF58u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrAt(0x0104FF58u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x00FBF574u)),
                       static_cast<uint32_t>(offset));
    OpenKey(writer, StrAt(0x0104FF48u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrAt(0x0104FF48u));
    const uint64_t crc = static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(base + 0x18)) |
                         (static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(base + 0x1C)) << 32);
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0104EAE4u)), crc);
    OpenKey(writer, StrAt(0x0104FF40u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrAt(0x0104FF40u));
    {
        // 0x00500310 copies the 16-byte record at +8 into a scratch local that is never
        // read again; the copy is kept because the original performs it.
        uint8_t scratch[16];
        CallGameMethod<void, 0x00500310>(scratch, base + 8);
    }
    uint16_t packageId[130];
    uint16_t* packageEnd = nullptr;
    GeneratePackageId501D70(packageId, &packageEnd, 16);
    OpenKey(writer, StrAt(0x0104FF34u));
    WriteValue(writer, packageId);
    CloseKey(writer, StrAt(0x0104FF34u));
    OpenKey(writer, StrAt(0x0104FF1Cu));
    WriteValue(writer, ContentTypeName99AE70(*reinterpret_cast<int*>(base + 0x858)));
    CloseKey(writer, StrAt(0x0104FF1Cu));
    const uint32_t thumbnail = *reinterpret_cast<uint32_t*>(base + 0x85C);
    if (thumbnail != 0) {
        OpenKey(writer, StrAt(0x0104FF08u));
        CloseKey(writer, StrAt(0x0104FF08u));
        if (thumbnail == 2) {
            OpenKey(writer, StrAt(0x0104FEE0u));
            CloseKey(writer, StrAt(0x0104FEE0u));
        }
    }
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                       *reinterpret_cast<uint32_t*>(base + 0x864));
    OpenKey(writer, StrAt(0x0104FED0u));
    WriteValue(writer, text.begin);
    CloseKey(writer, StrAt(0x0104FED0u));
    OpenKey(writer, StrAt(0x0104DF54u));
    CallGameMethod<int, 0x009B6230>(base + 0x648,
                                    static_cast<int>(*reinterpret_cast<uint32_t*>(base + 0x858)),
                                    writer);
    CloseKey(writer, StrAt(0x0104DF54u));
    CloseKey(writer, StrAt(0x0104FFA0u));
    if (OwnsMasked(text))
        Free4E3F70(text.begin);
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x009C7A20: write the whole Sims3Pack manifest (type/version/package id/dependencies/
// localized strings/records) through the XML builder into `sink`. Returns 0 when the sink
// is null or rejects the builder; otherwise 1, or 0 after a failing record write (the sink
// position is then restored). EA quirk kept: the two challenge hashes take the
// 0x009C6120 path, everything else the localized path.
char __fastcall WriteSims3PackXml9C7A20(void* self, void*, void* sink)
{
    if (!sink)
        return 0;
    uint8_t* base = static_cast<uint8_t*>(self);
    XmlBuilder builder;
    CallGameMethod<void, 0x007DBA60>(&builder);
    const uint32_t typeHash = *reinterpret_cast<uint32_t*>(base + 0xA8);
    const bool challenge = typeHash == 0xC4FB4F1u || typeHash == 0xC4FB4F9u;
    const void* rootKey = challenge ? StrAt(0x0104FE78u) : StrAt(0x010500D4u);
    char result = 1;

    const uint32_t position = CallGameVirtual<uint32_t, 9>(sink, 0);
    if (!CallGameMethod<char, 0x007DA230>(&builder, sink, 1)) {
        CallGameVirtual<void, 10>(sink, position, 0);
        CallGameMethod<int, 0x007DB860>(&builder);
        return 0;
    }
    OpenKey(&builder, rootKey);
    WideString text = EmptyWideStr(StrAt(0x0104FEA4u));

    const wchar_t* typeName = ContentTypeName99AE70(*reinterpret_cast<int*>(base + 0xA8));
    AssignWide408E20(&text, nullptr, typeName, WideEnd(typeName));
    CallGameMethod<int, 0x007DA2D0>(&builder, StrAt(0x00FB8A18u), &text);
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                       *reinterpret_cast<uint32_t*>(base + 0xAC));
    CallGameMethod<int, 0x007DA2D0>(&builder, StrAt(0x010500C4u), &text);
    if (*reinterpret_cast<uint32_t*>(base + 0xB0) != 0) {
        FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0102B0C4u)),
                           *reinterpret_cast<uint32_t*>(base + 0xB0));
        CallGameMethod<int, 0x007DA2D0>(&builder, StrAt(0x010500B0u), &text);
    }
    OpenKey(&builder, StrAt(0x0104FE58u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x0104FE4Cu)), 1, 5);
    WriteValue(&builder, text.begin);
    CloseKey(&builder, StrAt(0x0104FE58u));
    OpenKey(&builder, StrAt(0x0104FE34u));
    FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x00FB6880u)),
                       const_cast<void*>(StrAt(0x00FCC700u)));
    WriteValue(&builder, text.begin);
    CloseKey(&builder, StrAt(0x0104FE34u));
    OpenKey(&builder, StrAt(0x0104C4A0u));
    {
        const int version = ResolveVersion58A900(const_cast<void*>(StrAt(0x0104C490u)),
                                                 const_cast<void*>(StrAt(0x0104C4A0u)),
                                                 static_cast<int>(reinterpret_cast<intptr_t>(
                                                     StrAt(0x0104C4E4u))));
        const wchar_t* versionText = reinterpret_cast<const wchar_t*>(static_cast<intptr_t>(version));
        AssignWide408E20(&text, nullptr, versionText, WideEnd(versionText));
    }
    WriteValue(&builder, text.begin);
    CloseKey(&builder, StrAt(0x0104C4A0u));
    if (*reinterpret_cast<void**>(base + 0x88) != *reinterpret_cast<void**>(base + 0x8C)) {
        OpenKey(&builder, StrAt(0x0104FE1Cu));
        WriteValue(&builder, *reinterpret_cast<void**>(base + 0x88));
        CloseKey(&builder, StrAt(0x0104FE1Cu));
    }
    if (*reinterpret_cast<void**>(base + 0x98) != *reinterpret_cast<void**>(base + 0x9C)) {
        OpenKey(&builder, StrAt(0x0104FE04u));
        WriteValue(&builder, *reinterpret_cast<void**>(base + 0x98));
        CloseKey(&builder, StrAt(0x0104FE04u));
    }

    WideString packageId = EmptyWideStr(StrAt(0x0104FDD0u));
    CallGameMethod<int, 0x00500A40>(&packageId, 0);
    if (CompareRecords00500AF0(&packageId, base + 0x100)) {
        AssignPackageId9C11E0(&packageId, base + 0x100);
        OpenKey(&builder, StrAt(0x0104FDBCu));
        WriteValue(&builder, packageId.begin);
        CloseKey(&builder, StrAt(0x0104FDBCu));
        if (OwnsMasked(packageId))
            Free4E3F70(packageId.begin);
    }
    OpenKey(&builder, StrAt(0x0104FDB0u));
    {
        void* localized = CallGameFunction<void*, 0x0099E500>(&text, 0);
        WriteValue(&builder, localized);
    }
    CloseKey(&builder, StrAt(0x0104FDB0u));

    if (!challenge) {
        OpenKey(&builder, StrAt(0x01050094u));
        FormatWide0040E920(&text, static_cast<const wchar_t*>(StrAt(0x00FC07D4u)),
                           static_cast<uint32_t>(*reinterpret_cast<uint16_t*>(base + 0xCC)));
        WriteValue(&builder, text.begin);
        CloseKey(&builder, StrAt(0x01050094u));
        OpenKey(&builder, StrAt(0x01050078u));
        CallGameMethod<int, 0x009B01A0>(base + 0xF0, &text);
        WriteValue(&builder, text.begin);
        CloseKey(&builder, StrAt(0x01050078u));
        OpenKey(&builder, StrAt(0x0105005Cu));
        for (uint8_t* dependency = *reinterpret_cast<uint8_t**>(base + 0xBC);
             dependency != *reinterpret_cast<uint8_t**>(base + 0xC0); dependency += 16) {
            uint16_t generated[130];
            uint16_t* generatedEnd = nullptr;
            GeneratePackageId501D70(generated, &generatedEnd, 16);
            OpenKey(&builder, StrAt(0x01050044u));
            WriteValue(&builder, generated);
            CloseKey(&builder, StrAt(0x01050044u));
        }
        CloseKey(&builder, StrAt(0x0105005Cu));
    }

    if (challenge) {
        WriteChallengeFields9C6120(base, nullptr, &builder);
    } else {
        OpenKey(&builder, StrAt(0x0104FAD4u));
        for (uint8_t* element = *reinterpret_cast<uint8_t**>(base + 0xD0);
             element != *reinterpret_cast<uint8_t**>(base + 0xD4); element += 0x20) {
            OpenKey(&builder, StrAt(0x01050028u));
            CallGameMethod<int, 0x007DA1B0>(&builder, StrAt(0x01050014u),
                                            *reinterpret_cast<void**>(element));
            CallGameMethod<int, 0x007DA880>(&builder);
            CallGameMethod<int, 0x007DBAF0>(&builder, element + 0x10);
            CallGameMethod<int, 0x007DA970>(&builder);
            CloseKey(&builder, StrAt(0x01050028u));
        }
        CloseKey(&builder, StrAt(0x0104FAD4u));
        OpenKey(&builder, StrAt(0x0104FAA8u));
        for (uint8_t* element = *reinterpret_cast<uint8_t**>(base + 0xE0);
             element != *reinterpret_cast<uint8_t**>(base + 0xE4); element += 0x20) {
            OpenKey(&builder, StrAt(0x0104FFE8u));
            CallGameMethod<int, 0x007DA1B0>(&builder, StrAt(0x01050014u),
                                            *reinterpret_cast<void**>(element));
            CallGameMethod<int, 0x007DA880>(&builder);
            CallGameMethod<int, 0x007DBAF0>(&builder, element + 0x10);
            CallGameMethod<int, 0x007DA970>(&builder);
            CloseKey(&builder, StrAt(0x0104FFE8u));
        }
        CloseKey(&builder, StrAt(0x0104FAA8u));
    }

    uint8_t* record = *reinterpret_cast<uint8_t**>(base + 4);
    uint8_t* recordEnd = *reinterpret_cast<uint8_t**>(base + 8);
    int32_t recordOffset = 0;
    if (record != recordEnd) {
        while (true) {
            result = WritePackagedFileXml9C7670(record, &builder, recordOffset);
            if (!result)
                break;
            recordOffset += *reinterpret_cast<int32_t*>(record + 0x20);
            record += 0x868;
            if (record == recordEnd)
                break;
        }
        if (!result)
            CallGameVirtual<void, 10>(sink, position, 0);
    }
    CloseKey(&builder, rootKey);
    CallGameMethod<int, 0x007DA210>(&builder);
    if (OwnsMasked(text))
        Free4E3F70(text.begin);
    CallGameMethod<int, 0x007DB860>(&builder);
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x009C80D0: save the Sims3Pack described by `this` to `fileName`. When the object's
// +0xB4 state is not 2 it returns 0. Opens a TS3Pack file stream, creates a memory stream
// (0x004E3F90/0x004DF0C0) only when the named resource exists, writes the XML manifest
// through 0x009C7A20, then copies either the attached stream at +0x120 or each record's
// file (0x009C59E0) into the package. 0x004DBBD0 deletes the file on failure.
char __fastcall SaveSims3Pack9C80D0(void* self, void*, const wchar_t* fileName)
{
    uint8_t* base = static_cast<uint8_t*>(self);
    if (*reinterpret_cast<uint32_t*>(base + 0xB4) != 2)
        return 0;

    alignas(16) uint8_t stream[0x228];
    CallGameMethod<char, 0x004DB970>(stream, fileName);
    if (!CallGameMethod<char, 0x004DB4C0>(stream, 2, 2, 1, 0)) {
        CallGameMethod<void, 0x004DB9D0>(stream);
        return 0;
    }
    CallGameFunction<void, 0x004DDE30>(stream, StrAt(0x0104F9B4u), 7, 1);
    CallGameFunction<void, 0x004DDA90>(stream, 1);
    CallGameFunction<void, 0x004DDA90>(stream, 5);
    void* memoryStream = nullptr;
    {
        void* resource = CallGameFunction<void*, 0x004E3F90>(0x2C, StrAt(0x0102B454u), 0, 0, 0, 0);
        if (resource) {
            memoryStream = CallGameMethod<void*, 0x004DF0C0>(resource, 0, 0, 0);
            if (memoryStream)
                CallGameVirtual<void, 1>(memoryStream);
        }
    }
    CallGameMethod<void, 0x004DEAC0>(memoryStream, 1, 1.0f);
    if (!WriteSims3PackXml9C7A20(base, nullptr, memoryStream)) {
        CallGameMethod<void, 0x004DB600>(stream);
        CallGameFunction<int, 0x004DBBD0>(fileName);
        if (memoryStream)
            CallGameVirtual<void, 2>(memoryStream);
        CallGameMethod<void, 0x004DB9D0>(stream);
        return 0;
    }
    CallGameVirtual<void, 10>(memoryStream, 0, 0);
    const uint32_t memorySize = CallGameVirtual<uint32_t, 11>(memoryStream);
    CallGameFunction<void, 0x004DDBD0>(stream, memorySize, 1);
    {
        void** memory = static_cast<void**>(memoryStream);
        const void* buffer = memory[1] ? *reinterpret_cast<const void**>(
                                            static_cast<uint8_t*>(memory[1]) + 8)
                                       : nullptr;
        CallGameMethod<int, 0x004DB8A0>(stream, buffer, memorySize);
    }

    void* attached = *reinterpret_cast<void**>(base + 0x120);
    uint8_t* record = *reinterpret_cast<uint8_t**>(base + 4);
    uint8_t* recordEnd = *reinterpret_cast<uint8_t**>(base + 8);
    char copied = 1;
    if (attached) {
        CallGameVirtual<void, 10>(attached, 0, 0);
        copied = CopyStreamToStream9C5960(attached, stream);
        wchar_t attachedName[260];
        CallGameVirtual<void, 17>(attached, attachedName, 260);
        CallGameVirtual<void, 6>(attached);
        if (attached) {
            *reinterpret_cast<void**>(base + 0x120) = nullptr;
            CallGameVirtual<void, 2>(attached);
        }
        CallGameFunction<int, 0x004DBBD0>(attachedName);
    } else if (record != recordEnd) {
        while (true) {
            uint8_t* node = record + 0x238;
            const wchar_t* name =
                static_cast<const wchar_t*>(CallGameMethod<void*, 0x00F626F0>(node));
            copied = CallGameStdcall<char, 0x009C59E0>(name, stream);
            if (*reinterpret_cast<uint8_t*>(node + 0x628)) {
                const wchar_t* again =
                    static_cast<const wchar_t*>(CallGameMethod<void*, 0x00F626F0>(node));
                CallGameFunction<int, 0x004DBBD0>(again);
            }
            if (!copied)
                break;
            record += 0x868;
            if (record == recordEnd)
                break;
        }
    }
    CallGameMethod<void, 0x004DB600>(stream);
    if (!copied)
        CallGameFunction<int, 0x004DBBD0>(fileName);
    CallGameVirtual<void, 2>(memoryStream);
    CallGameMethod<void, 0x004DB9D0>(stream);
    return copied;
}

}  // namespace ts3::scratch