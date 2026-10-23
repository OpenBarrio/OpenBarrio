// Rewrite of cluster unit clu_4b4250_Scratch (Scratch).
//
// The unit has no exported code of its own beyond the entry points below: everything it
// touches that is not rewritten yet goes through the bridge in ts3/layout.h. Two calls
// cannot go through the bridge because their callee pops the arguments (stdcall), so
// they are called through a local __stdcall pointer.
#include "ts3/scratch/clu_4b4250_Scratch.h"

#include <cstdio>
#include <cstring>

#include "ts3/layout.h"

#if defined(_M_IX86)
#include <intrin.h>
#endif

namespace ts3::scratch {
namespace {

// __stdcall game functions: CallGameFunction is __cdecl, so the caller would clean the
// arguments a second time. Both direct callers of 0x00992430 and 0x008A7BB0 let the
// callee pop the arguments.
template <class R, unsigned kAddress, class... Args>
R CallStd(Args... args)
{
#if defined(_M_IX86)
    using Function = R(__stdcall*)(Args...);
    return reinterpret_cast<Function>(static_cast<uintptr_t>(kAddress))(args...);
#else
    ((void)args, ...);
    AbortNotRewritten("a game function");
    return R{};
#endif
}

void Log898620(int a1, int a2, const wchar_t* channel, const wchar_t* message,
               const wchar_t* extra, int last)
{
    CallGameFunction<void, 0x00898620>(a1, a2, channel, message, extra, last);
}

// The game's "is the allocation worth freeing" test: (capEnd - begin) & ~1 > 2.
bool IsHeapAllocated(const String& s)
{
    return (((uintptr_t)s.cap - (uintptr_t)s.begin) & 0xFFFFFFFEu) > 2;
}
bool IsHeapAllocated(const WideString& s)
{
    return (((uintptr_t)s.cap - (uintptr_t)s.begin) & 0xFFFFFFFEu) > 2;
}
// 0x004E3F70: the game's free.
void Free4E3F70(const void* p)
{
    CallGameFunction<void, 0x004E3F70>(const_cast<void*>(p));
}

// 0x00898620's channel literals of this unit.
const wchar_t* const kLogNeedToPatch = L"ContentPatchRecord::NeedToPatch";
const wchar_t* const kLogInitTrial = L"ContentPatchRecord::InitTrialData";

// Allocator tags for the strings this unit builds.
const char* const kTagPatchFile = "Scratch/ContentPatchFile/patchfile";
const char* const kTagPatchDate = "Scratch/ContentPatchFile/patchDate";
const char* const kTagIgnorePath = "Scratch/ContentPatchFile/ignorePatchFilepath";
const char* const kTagTokens = "Scratch/SIGS/TestIgnoringContentPatcheFlag/tokens";

// 0x011C7E54: the game's empty-string singleton (the inline "NewFileName" buffer).
void SetEmptyString(String& s, const char* tag)
{
#if defined(_M_IX86)
    s.begin = reinterpret_cast<char*>(static_cast<uintptr_t>(0x011C7E54));
    s.cap = reinterpret_cast<char*>(static_cast<uintptr_t>(0x011C7E56));
#else
    s.begin = nullptr;
    s.cap = nullptr;
#endif
    s.cur = s.begin;
    s.tag = tag;
}
}  // namespace

// ---------------------------------------------------------------------------------------
// 0x004B4250
int ScratchBlockPair::FreeBlocks4B4250()
{
    int32_t spanB = static_cast<int32_t>(reinterpret_cast<uintptr_t>(endB) -
                                         reinterpret_cast<uintptr_t>(beginB));
    if (spanB > 1 && beginB) {
        CallGameVirtual<void, 3>(static_cast<ScratchBlockPair*>(this)->allocatorB, beginB, spanB);
    }

    int32_t spanA = static_cast<int32_t>(reinterpret_cast<uintptr_t>(endA) -
                                         reinterpret_cast<uintptr_t>(beginA));
    if (spanA > 1 && beginA) {
        return CallGameVirtual<int, 3>(allocatorA, beginA, spanA);
    }
    return spanA;
}

// ---------------------------------------------------------------------------------------
// 0x004B4460
uint32_t RecordNode::MapLookup4B4460(const char* key)
{
    // 0x004B4390 fills the out iterator: the sentinel (this+0x38) or -8 means "not found".
    void* node = nullptr;
    CallGameMethod<void*, 0x004B4390>(this, &node, key);

    uint32_t value = 0;
    const void* sentinel = reinterpret_cast<uint8_t*>(this) + 0x38;
    const void* end = reinterpret_cast<const void*>(static_cast<uintptr_t>(0xFFFFFFF8u));
    if (node != sentinel && node != end) {
        value = *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(node) + 0x20);
    }
    return value;
}

// ---------------------------------------------------------------------------------------
// 0x004F99E0
int DayPair::Compare86400(const DayPair& other, uint8_t divide, uint8_t modulo) const
{
    int64_t a = (static_cast<int64_t>(hi) << 32) | lo;
    int64_t b = (static_cast<int64_t>(other.hi) << 32) | other.lo;

    if (divide) {
        if (!modulo) {
            a /= 86400;
            b /= 86400;
        }
    } else if (modulo) {
        a %= 86400;
        b %= 86400;
    }

    if (a > b) return 1;
    if (a < b) return -1;
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x00882240
void* RefHolder::CopyRef4_882240(void* out)
{
    *static_cast<void**>(out) = target;
    if (target) {
        CallGameVirtual<void, 1>(target);
    }
    return out;
}

// ---------------------------------------------------------------------------------------
// 0x00992080
//
// EA quirk kept: the converted path string is built in the frame slot that is also the
// tag of the wide path, so appending after the conversion grows with the overwritten tag.
uint8_t PatchFile::NeedToPatch992080(const void* const* services)
{
    if (!initialized) {
        Log898620(16, 99, kLogNeedToPatch, L"not initialized", nullptr, 0);
        return 0;
    }

    WideString widePath;
    widePath.begin = nullptr;
    widePath.cur = nullptr;
    widePath.cap = nullptr;
    widePath.tag = kTagPatchFile;
    CallGameMethod<void, 0x0040B700>(
        &widePath, *reinterpret_cast<const wchar_t* const*>(services));

    WideString converted;
    // 0x004F3890 takes the narrow String object, not its begin pointer (an earlier
    // revision passed path.begin and crashed inside the conversion).  Its frame slot
    // (E-0x10) does not overlap widePath's tag (E-0x14), so the tag set above stays.
    CallGameFunction<void, 0x004F3890>(&converted, &path);
    CallGameMethod<void, 0x00408B90>(&widePath, converted.begin, converted.cur);
    if (IsHeapAllocated(converted) && converted.begin) Free4E3F70(converted.begin);

    if (!CallGameFunction<uint8_t, 0x004DBBA0>(widePath.begin)) {
        Log898620(16, 99, kLogNeedToPatch, L"file does not exists", widePath.begin, 0);
        if (IsHeapAllocated(widePath) && widePath.begin) Free4E3F70(widePath.begin);
        return 1;
    }
    if (CallGameFunction<uint32_t, 0x004DBC60>(widePath.begin) != fileSize) {
        Log898620(16, 99, kLogNeedToPatch, L"file sizes do not match", nullptr, 0);
        if (IsHeapAllocated(widePath) && widePath.begin) Free4E3F70(widePath.begin);
        return 1;
    }
    if (CallGameFunction<int, 0x00991E50>(&widePath) != fnv) {
        Log898620(16, 99, kLogNeedToPatch, L"fnv1 do not match", nullptr, 0);
        if (IsHeapAllocated(widePath) && widePath.begin) Free4E3F70(widePath.begin);
        return 1;
    }
    Log898620(16, 99, kLogNeedToPatch, L"no patch needed", nullptr, 0);
    if (IsHeapAllocated(widePath) && widePath.begin) Free4E3F70(widePath.begin);
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x00993970
//
// EA quirks kept: a failed FileStream allocation is dereferenced without a null check,
// and the token-array element-count test is ((cur - begin) & 0xFFFFFFF0) >= 48.
uint8_t ContentPatchGroup::IgnorePatchFile993970()
{
    String source;
    SetEmptyString(source, kTagIgnorePath);
    CallGameFunction<void, 0x00992430>(&source);  // append DontBugMeUntilNextPatch (cdecl)

    const wchar_t* widePath = reinterpret_cast<const wchar_t*>(source.begin);
    if (!CallGameFunction<uint8_t, 0x004DBBA0>(widePath)) {
        if (IsHeapAllocated(source) && source.begin) Free4E3F70(source.begin);
        return 0;
    }

    void* stream = CallGameFunction<void*, 0x004E3F90>(
        552, "Services/SimpleFileDownload/pFile", 0);
    if (stream) {
        stream = CallGameMethod<void*, 0x004DB970>(stream, widePath);  // FileStream(path)
        if (stream) CallGameVirtual<void, 1>(stream);                  // AddRef
    }
    // No null check on stream: the original dereferences it (kept for fidelity).
    if (CallGameVirtual<uint8_t, 19>(stream, 1, 3, 1, 0)) {
        uint32_t size = CallGameFunction<uint32_t, 0x004DBC60>(widePath);
        if (size > 32) size = 32;

        String contents;
        SetEmptyString(contents, kTagPatchDate);
        contents.cap = reinterpret_cast<char*>(static_cast<uintptr_t>(0x011C7E55));
        CallGameMethod<void, 0x00409BB0>(&contents, size);  // reserve
        CallGameMethod<void, 0x00409F00>(&contents, size);  // set length
        CallGameVirtual<uint32_t, 12>(stream, contents.begin, size);  // Read(buf, size)

        StringArray tokens;
        tokens.begin = nullptr;
        tokens.cur = nullptr;
        tokens.cap = nullptr;
        tokens.tag = kTagTokens;
        CallGameFunction<void, 0x0089A3C0>(&contents, &tokens, "/");

        if ((((uintptr_t)tokens.cur - (uintptr_t)tokens.begin) & 0xFFFFFFF0u) >= 48) {
            String* elements = tokens.begin;
            uint32_t day = CallGameFunction<int, 0x004FFA00>(elements[2].begin, 0, 10);
            uint32_t month = CallGameFunction<int, 0x004FFA00>(elements[1].begin, 0, 10);
            uint32_t year = CallGameFunction<int, 0x004FFA00>(elements[0].begin, 0, 10);
            uint32_t dateValue[4] = {0, 0, 0, 0};
            CallGameMethod<uint32_t, 0x004F9D70>(dateValue, year, month, day, 0, 0, 0);
            DayPair parsed;
            parsed.lo = dateValue[0];
            parsed.hi = dateValue[1];
            if (parsed.Compare86400(date, 1, 1) == 0) {
                CallGameMethod<void, 0x007685D0>(&tokens);
                if (IsHeapAllocated(contents) && contents.begin) Free4E3F70(contents.begin);
                CallGameVirtual<void, 2>(stream);
                if (IsHeapAllocated(source) && source.begin) Free4E3F70(source.begin);
                return 1;
            }
        }

        CallStd<void, 0x008A7BB0>(tokens.begin, tokens.cur);
        if (tokens.begin) Free4E3F70(tokens.begin);
        if (IsHeapAllocated(contents) && contents.begin) Free4E3F70(contents.begin);
    }

    CallGameVirtual<void, 2>(stream);
    if (IsHeapAllocated(source) && source.begin) Free4E3F70(source.begin);
    return 0;
}

// ---------------------------------------------------------------------------------------
// 0x009941D0
uint8_t ContentPatchGroup::PatchCheck9941D0()
{
    // The original returns 0 through two early-outs; keeping them on one result variable
    // leaves no unobservable return path for the mutation checks.
    uint8_t result = 0;
    if (file && !IgnorePatchFile993970()) {
        int32_t service = CallGameFunction<int, 0x00984550>();
        const void* const* installDir = reinterpret_cast<const void* const*>(
            reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(service)) + 0x140);
        result = file->NeedToPatch992080(installDir);
    }
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x00994C40
//
// EA quirk kept: the global at 0x11D405C is dereferenced at +0x6C even when it is null.
uint8_t ContentPatchGroup::InitTrialData994C40()
{
    Log898620(16, 99, kLogInitTrial, nullptr, nullptr, 0);
    if (!CallGameMethod<uint8_t, 0x00994200>(this)) {
        Log898620(16, 99, kLogInitTrial, L"Could not ", nullptr, 0);
        return 0;
    }
    if (versionUrl[0] == versionUrl[1]) {
        Log898620(16, 99, kLogInitTrial, L"empty mContentPatchVersionUrl", nullptr, 0);
        return 0;
    }
    if (baseUrl[0] == baseUrl[1]) {
        Log898620(16, 99, kLogInitTrial, L"empty mContentPatchBaseUrl", nullptr, 0);
        return 0;
    }
    if (versionFilename[0] == versionFilename[1]) {
        Log898620(16, 99, kLogInitTrial, L"empty mContentPatchVersionFilename", nullptr, 0);
        return 0;
    }

    const char* tag = reinterpret_cast<const char*>(static_cast<uintptr_t>(0x01046D84));
    int32_t service = CallGameFunction<int, 0x00984550>();
    const wchar_t* installDir = *reinterpret_cast<const wchar_t* const*>(
        reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(service)) + 0x358);
    WideString widePath;
    CallGameMethod<void, 0x0040BAD0>(&widePath, installDir, &tag);

    WideString converted;
    CallGameFunction<void, 0x004F3890>(&converted,
                                       reinterpret_cast<const void*>(versionFilename));
    CallGameMethod<void, 0x00408B90>(&widePath, converted.begin, converted.cur);
    if (IsHeapAllocated(converted) && converted.begin) Free4E3F70(converted.begin);

    void* ref = nullptr;
#if defined(_M_IX86)
    uintptr_t globalRef =
        *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(0x011D405C));
    void* holder = globalRef ? reinterpret_cast<void*>(globalRef - 12) : nullptr;
    ref = *reinterpret_cast<void**>(static_cast<uint8_t*>(holder) + 0x6C);  // null deref
    if (ref) {
        _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(
                                    static_cast<uint8_t*>(ref) + 4), 1);
    }
#endif

    RefHolder download;
    download.padding00[0] = 0;
    download.padding00[1] = 0;
    download.padding00[2] = 0;
    download.target = nullptr;
    CallGameMethod<void, 0x00882BC0>(&download, &ref);
    CallGameMethod<void, 0x007460F0>(&ref);

    void* urlPointer = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(versionUrl));
    if (CallGameMethod<uint8_t, 0x0087A330>(&download, urlPointer, &widePath, 0, 0, 0, 0)) {
        void* stream = nullptr;
        void* out = download.CopyRef4_882240(&stream);
        int parsed = CallGameMethod<int, 0x009947B0>(this, *static_cast<void**>(out));
        if (stream) CallGameVirtual<void, 2>(stream);
        if (parsed) {
            int32_t parsedVersion[4] = {0, 0, 0, 0};
            ConfigVersion9B0910(parsedVersion);
            GameVersion* versionObject = reinterpret_cast<GameVersion*>(parsedVersion);
            GameVersion* recordVersion = reinterpret_cast<GameVersion*>(version);
            if (CallGameMethod<uint8_t, 0x009B0150>(versionObject, recordVersion)) {
                if (PatchCheck9941D0()) {
                    CallGameMethod<void, 0x00993C50>(this);
                    CallGameFunction<void, 0x00874310>(11);
                } else {
                    Log898620(16, 99, kLogInitTrial, L"not needed ", nullptr, 0);
                }
            } else {
                CallGameFunction<void, 0x00874310>(13);
                Log898620(16, 99, kLogInitTrial, L"failed game version check", nullptr, 0);
            }
        } else {
            Log898620(16, 99, kLogInitTrial, L"failed to parse memory stream", nullptr, 0);
        }
    } else {
        Log898620(16, 99, kLogInitTrial, L"failed to download content patch", nullptr, 0);
    }

    CallGameMethod<void, 0x0088FAC0>(&download);
    if (IsHeapAllocated(widePath) && widePath.begin) Free4E3F70(widePath.begin);
    return 1;
}

// ---------------------------------------------------------------------------------------
// 0x00994EC0
uint8_t __cdecl InitTrialDataGlobal994EC0()
{
#if defined(_M_IX86)
    uint32_t global = *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(0x011E9640));
#else
    uint32_t global = 0;
#endif
    uint8_t result = 0;
    if (global && global != 8) {
        result = reinterpret_cast<ContentPatchGroup*>(
                     static_cast<uintptr_t>(global - 8))->InitTrialData994C40();
    }
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x0099F0E0
int __cdecl SplitTokens99F0E0(String* a1, TokenArray* a2, const char* a3)
{
    const auto setEnd = [a3]() { return a3 + std::strlen(a3); };

    char* begin = a1->begin;
    char* cur = a1->cur;
    const char* first = CallGameFunction<char*, 0x0045CFA0>(begin, cur, a3, setEnd());
    int32_t current = (first == cur) ? -1 : static_cast<int32_t>(first - begin);
    int32_t separator = CallGameMethod<int, 0x00405990>(a1, a3, static_cast<uint32_t>(current));
    int32_t result = separator;

    for (;;) {
        // Both -1: the original returns the leftover separator (-1), not the product.
        if (separator == -1 && current == -1) return separator;

        // The token is [current, current + min(separator - current, length - current));
        // when the separator was not found the difference wraps and the rest of the
        // string is the token (both compares are unsigned, as in the original).
        {
            char* tokenBegin = a1->begin;
            char* tokenCur = a1->cur;
            uint32_t total = static_cast<uint32_t>(tokenCur - tokenBegin);
            uint32_t fromSeparator = static_cast<uint32_t>(separator - current);
            uint32_t remaining = total - static_cast<uint32_t>(current);
            uint32_t length = remaining < fromSeparator ? remaining : fromSeparator;
            const char* tokenStart = tokenBegin + static_cast<uint32_t>(current);
            const char* tokenEnd = tokenStart + length;

            String element;
            char inlineBuffer[256];
            element.begin = inlineBuffer;
            element.cur = inlineBuffer;
            element.cap = inlineBuffer + 255;
            element.tag = nullptr;
            inlineBuffer[0] = 0;
            CallGameMethod<void, 0x0099C9C0>(&element, tokenStart, tokenEnd);

            TokenElement* slot = a2->cur;
            if (slot >= a2->cap) {
                CallGameMethod<void, 0x0099EEB0>(a2, slot, &element);
            } else {
                a2->cur = slot + 1;
                if (slot) {
                    char* buffer = slot->inlineBuffer;
                    slot->text.begin = buffer;
                    slot->text.cur = buffer;
                    slot->text.cap = buffer + 255;
                    *buffer = 0;
                    CallGameMethod<void, 0x0099C9C0>(&slot->text, tokenStart, tokenEnd);
                }
            }
        }

        // (0x78787879 * byteSize) / 272 is the element count; the low half of the
        // product is the 4-element return value (EA quirk kept).
        const uint32_t byteSize = static_cast<uint32_t>(
            reinterpret_cast<uint8_t*>(a2->cur) - reinterpret_cast<uint8_t*>(a2->begin));
        result = static_cast<int32_t>(0x78787879u * byteSize);
        if (byteSize / kString272Stride == kString272Capacity) return result;

        // The next token starts at the first non-separator at/after the separator that
        // ended the previous token.
        {
            char* searchBegin = a1->begin;
            char* searchCur = a1->cur;
            uint32_t total = static_cast<uint32_t>(searchCur - searchBegin);
            if (static_cast<uint32_t>(separator) > total) {
                current = -1;
            } else {
                const char* found = CallGameFunction<char*, 0x0045CFA0>(
                    searchBegin + static_cast<uint32_t>(separator), searchCur, a3, setEnd());
                current = (found == searchCur) ? -1 : static_cast<int32_t>(found - searchBegin);
            }
        }

        // The next separator is the first set byte at/after that start.
        {
            char* scanBegin = a1->begin;
            char* scanCur = a1->cur;
            uint32_t total = static_cast<uint32_t>(scanCur - scanBegin);
            if (static_cast<uint32_t>(current) >= total) {
                separator = -1;
                continue;
            }
            char* scan = scanBegin + static_cast<uint32_t>(current);
            const char* end = setEnd();
            while (scan != scanCur) {
                const char* inSet = a3;
                while (inSet != end && *inSet != *scan) ++inSet;
                if (inSet != end) break;
                ++scan;
            }
            separator = (scan == scanCur) ? -1 : static_cast<int32_t>(scan - scanBegin);
        }
    }
}

// ---------------------------------------------------------------------------------------
// 0x009B01D0
int GameVersion::ParseVersion9B01D0(const char* src)
{
    char textBuffer[256];
    String text;
    text.begin = textBuffer;
    text.cur = textBuffer;
    text.cap = textBuffer + 255;
    text.tag = nullptr;
    textBuffer[0] = 0;
    CallGameMethod<void, 0x0099C9C0>(&text, src, src + std::strlen(src));

    uint8_t elementStorage[272 * 5];
    std::memset(elementStorage, 0, sizeof(elementStorage));
    TokenArray elements;
    elements.begin = reinterpret_cast<TokenElement*>(elementStorage);
    elements.cur = elements.begin;
    elements.cap = reinterpret_cast<TokenElement*>(elementStorage + sizeof(elementStorage));
    SplitTokens99F0E0(&text, &elements, ".");

    int count = static_cast<int32_t>(elements.cur - elements.begin);
    int result = count;
    if (count == 4) {
        TokenElement* tokens = elements.begin;
        if (static_cast<uint32_t>(tokens[3].text.cur - tokens[3].text.begin) >= 5) {
            value[0] = CallGameFunction<int, 0x004FF9E0>(tokens[0].text.begin, 0, 10);
            value[1] = CallGameFunction<int, 0x004FF9E0>(tokens[1].text.begin, 0, 10);
            value[2] = CallGameFunction<int, 0x004FF9E0>(tokens[2].text.begin, 0, 10);
            const char* last = tokens[3].text.begin;
            if (tokens[3].text.cur - last == 5) {
                uint32_t v = static_cast<uint32_t>(
                    CallGameFunction<int, 0x004FF9E0>(last, 0, 10));
                result = static_cast<int>(v + 10 * (v / 100 + 10 * (v / 10000) - v / 10));
            } else {
                result = CallGameFunction<int, 0x004FF9E0>(last, 0, 10);
            }
            value[3] = result;
        } else {
            value[0] = 0;
            value[1] = 0;
            value[2] = 0;
            value[3] = 0;
        }
    } else {
        value[0] = 0;
        value[1] = 0;
        value[2] = 0;
        value[3] = 0;
    }
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x009B04C0
GameVersion* GameVersion::AssignVersion9B04C0(const char* src)
{
    ParseVersion9B01D0(src);
    return this;
}

// ---------------------------------------------------------------------------------------
// 0x009B0910
//
// EA quirk kept: 0x004F38C0 produces a wide buffer in the frame slot, but 0x009B01D0
// reads it as a narrow string, so strlen stops after the first character; the configured
// build number is then folded into value[3].
int32_t* __cdecl ConfigVersion9B0910(int32_t* a1)
{
    String config;
    config.begin = nullptr;
    config.cur = nullptr;
    config.cap = nullptr;
    config.tag = nullptr;

    const char* value = CallGameFunction<const char*, 0x0058A900>(
        reinterpret_cast<const wchar_t*>(static_cast<uintptr_t>(0x0104C490)),
        reinterpret_cast<const wchar_t*>(static_cast<uintptr_t>(0x0104C4A0)), L"0.0.0.0");
    CallGameFunction<void, 0x004F38C0>(&config, value, -1, 0);
    reinterpret_cast<GameVersion*>(a1)->ParseVersion9B01D0(config.begin);
    // The original tests the 16-byte slot's second pointer against its first one.
    if (((uintptr_t)config.cur - (uintptr_t)config.begin) > 1 && config.begin) {
        Free4E3F70(config.begin);
    }

    if (CallGameFunction<uint8_t, 0x009B0530>()) {
        int32_t build = CallGameFunction<int, 0x0058ABA0>(
            reinterpret_cast<const wchar_t*>(static_cast<uintptr_t>(0x0104C490)),
            reinterpret_cast<const wchar_t*>(static_cast<uintptr_t>(0x0104C524)), 0);
        a1[3] = 1000 * build + a1[3] % 1000;
    } else {
        int32_t fileVersion[4] = {0, 0, 0, 0};
        CallGameFunction<void, 0x009B05A0>(fileVersion);
        a1[3] = a1[3] % 1000 + 1000 * static_cast<int32_t>(fileVersion[3] / 1000);
    }
    return a1;
}

// ---------------------------------------------------------------------------------------
// 0x009C2D60
void* RecordTreeWalker::WalkRecordTree9C2D60(int32_t a2, int32_t a3)
{
    uint8_t* nodeRoot = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(a2));
    uint8_t* list = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(a3));
    uint32_t* sentinel = reinterpret_cast<uint32_t*>(nodeRoot + 0x24);
    uint32_t* node = reinterpret_cast<uint32_t*>(*sentinel);

    char textBuffer[256];
    String text;
    text.begin = textBuffer;
    text.cur = textBuffer;
    text.cap = textBuffer + 255;
    text.tag = nullptr;
    textBuffer[0] = 0;

    uint8_t elementStorage[272 * 5];
    std::memset(elementStorage, 0, sizeof(elementStorage));
    TokenArray elements;
    elements.begin = reinterpret_cast<TokenElement*>(elementStorage);
    elements.cur = elements.begin;
    elements.cap = reinterpret_cast<TokenElement*>(elementStorage + sizeof(elementStorage));

    void* result = text.cap;  // empty list: the original returns the frame's cap pointer

    if (node == sentinel) return result;

    do {
        RecordNode* value = reinterpret_cast<RecordNode*>(static_cast<uintptr_t>(node[2]));
        if (value->kind == 2) {
            // The key is chosen by comparing the value object's own name (offset +8) with
            // "value"; the map lookups on 0x004B4460 return the char* record text.
            const char* key = CallGameMethod<int, 0x009C1F70>(&value->name, "value") == 0
                                  ? "value"
                                  : "reskey";
            const char* found = reinterpret_cast<const char*>(static_cast<uintptr_t>(
                value->MapLookup4B4460(key)));
            if (found) {
                if (text.begin != found) {
                    if (text.begin != text.cur) {
                        *text.begin = 0;
                        text.cur = text.begin;
                    }
                    CallGameMethod<void, 0x0099C9C0>(&text, found, found + std::strlen(found));
                }
                elements.cur = elements.begin;
                SplitTokens99F0E0(&text, &elements, ":");
                if (CallGameMethod<int, 0x009C1F70>(&elements.begin[0].text, "key") == 0) {
                    uint32_t x = 0, y = 0;
                    uint32_t z[2] = {0, 0};
                    // x/y come from tokens 1/2 ("%x"), z from token 3 ("%I64x"); the
                    // original ignores the sscanf return values (kept).
                    std::sscanf(elements.begin[1].text.begin, "%x", &x);
                    std::sscanf(elements.begin[2].text.begin, "%x", &y);
                    std::sscanf(elements.begin[3].text.begin, "%I64x", z);
                    uint32_t* record = reinterpret_cast<uint32_t*>(list + 4);
                    uint32_t* cur = reinterpret_cast<uint32_t*>(*record);
                    if (cur >= reinterpret_cast<uint32_t*>(*(record + 1))) {
                        CallGameMethod<void, 0x006D3810>(list, cur, z);
                    } else {
                        *record = reinterpret_cast<uint32_t>(cur) + 16;
                        if (cur) {
                            cur[0] = z[0];
                            cur[1] = z[1];
                            cur[2] = x;
                            cur[3] = y;
                        }
                    }
                }
            }
            // The original recurses into the value object (its +0x24 is the child list).
            result = WalkRecordTree9C2D60(
                static_cast<int32_t>(reinterpret_cast<uintptr_t>(value)), a3);
        }
        node = reinterpret_cast<uint32_t*>(*node);
    } while (node != sentinel);

    return result;
}

}  // namespace ts3::scratch