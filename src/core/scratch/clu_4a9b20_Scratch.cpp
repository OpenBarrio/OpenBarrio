// Rewrite of cluster unit clu_4a9b20_Scratch (Scratch). Every entry point keeps the
// original TS3W.exe ABI; callees that are not rewritten yet go through the bridge in
// ts3/layout.h. Two Scratch helpers already rewritten elsewhere (0x004E3F70, 0x004EFBA0)
// and the record Assign 0x009A0100 are called directly.
#include "ts3/scratch/clu_4a9b20_Scratch.h"

#include <cstring>

#include "ts3/layout.h"

namespace ts3::scratch {
namespace {

// Game addresses of the analysis-relevant globals/strings (x64 has no game image).
inline void* GameAddr(uintptr_t address)
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address;
    return nullptr;
#endif
}

// The game's empty-string singleton: NewFileName at 0x011C7E54, its end at +1/+2.
constexpr uintptr_t kEmptyBegin = 0x011C7E54u;
constexpr uintptr_t kEmptyCap1 = 0x011C7E55u;
constexpr uintptr_t kEmptyCap2 = 0x011C7E56u;

// Allocator tag literals (pointers stored in the records).
constexpr uintptr_t kTagOutputFile = 0x00FE47D0u;         // Scratch/DownloadContent/mOutputFilename
constexpr uintptr_t kTagExportDir = 0x00FE47ACu;          // Scratch/DownloadContent/mExportDir
constexpr uintptr_t kTagContentName = 0x00FE4938u;        // .../ExportContentName
constexpr uintptr_t kTagContentDesc = 0x00FE4904u;        // .../ExportContentDescription
constexpr uintptr_t kTagPresetStr = 0x00FE4788u;          // .../PresetStr
constexpr uintptr_t kTagWorldFile = 0x00FE48DCu;          // .../WorldFilePath
constexpr uintptr_t kTagNameList = 0x00FE48B8u;           // .../NameList
constexpr uintptr_t kTagDescList = 0x00FE4894u;           // .../DescList
constexpr uintptr_t kTagPackageIds = 0x00FE4860u;         // .../ExportContentAddPackageIds

// One 0x011C7E54-based tagged string with a static label.
void SetInline(ScratchString& s, uintptr_t begin, uintptr_t cap, uintptr_t label)
{
    s.begin = GameAddr(begin);
    s.end = GameAddr(begin);
    s.capacity = GameAddr(cap);
    s.label = static_cast<const char*>(GameAddr(label));
}

void SetEmptyVector(ScratchPairs& v, uintptr_t label)
{
    v.begin = nullptr;
    v.end = nullptr;
    v.capacity = nullptr;
    v.label = static_cast<const char*>(GameAddr(label));
}

void SetEmptyKeys(ScratchKeys& v, uintptr_t label)
{
    v.begin = nullptr;
    v.end = nullptr;
    v.capacity = nullptr;
    v.label = static_cast<const char*>(GameAddr(label));
}

// (cap - begin) as the original's signed 32-bit subtraction.
int32_t SignedSpan(const void* begin, const void* cap)
{
    return static_cast<int32_t>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cap) -
                                                      reinterpret_cast<uintptr_t>(begin)));
}

// The game's "is this heap allocated" test: (span & mask) > limit && begin != 0.
bool HeapBlock(const ScratchString& s, uint32_t mask, int32_t limit)
{
    return (static_cast<uint32_t>(SignedSpan(s.begin, s.capacity)) & mask) > static_cast<uint32_t>(limit) &&
           s.begin != nullptr;
}

// Free one tagged string, returning EAX (free result, or the begin pointer when not freed).
uintptr_t FreeTagged(const ScratchString& s, uint32_t mask, int32_t limit)
{
    if (HeapBlock(s, mask, limit))
        return reinterpret_cast<uintptr_t>(Free4E3F70(s.begin));
    return reinterpret_cast<uintptr_t>(s.begin);
}

// __stdcall game functions (CallGameFunction is __cdecl): the callee pops its arguments.
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

// 0x004D2290: copy a byte span. It is a __thiscall whose ECX is the destination range
// object (0x00876260 leaves its own `this` there), followed by four stack arguments the
// callee pops (ret 0x10). The 0x00876260 caller does not clean them.
int32_t CopySpan4D2290(const ScratchByteRange* self, void* destination, void* destinationEnd,
                       void* source, void* sourceEnd)
{
    return CallGameMethod<int32_t, 0x004D2290>(const_cast<ScratchByteRange*>(self), destination,
                                               destinationEnd, source, sourceEnd);
}

// 0x004B4290 (__thiscall, ret 0): destroy the +0x24 intrusive list of a hash-map object.
void DestroyHashList4B4290(void* self) { CallGameMethod<int, 0x004B4290>(self); }

// 0x00500A40 (__thiscall, ret 4): initialize the tagged string at `self` from a size tag.
void InitTaggedString500A40(void* self, int32_t tag) { CallGameMethod<int, 0x00500A40>(self, tag); }

// 0x00405AD0 (__thiscall, ret 4): copy-assign one tagged string.
void AssignTaggedString405AD0(ScratchString* destination, const ScratchString* source)
{
    CallGameMethod<int, 0x00405AD0>(destination, source);
}

// 0x00408EB0 (__thiscall, ret 4): copy-assign the 0x80-block (vector with label) of a record.
void AssignBlock408EB0(void* destination, const void* source)
{
    CallGameMethod<int, 0x00408EB0>(destination, source);
}

// 0x004058B0 (__thiscall, ret 4): reserve `count` bytes in a tagged string.
void Reserve4058B0(ScratchString* string, int32_t count)
{
    CallGameMethod<int, 0x004058B0>(string, count);
}

// 0x00405790 (__thiscall, ret 8): append the [begin,end) span to a tagged string.
void Append405790(ScratchString* string, const void* begin, const void* end)
{
    CallGameMethod<int, 0x00405790>(string, begin, end);
}

// 0x004B4880's allocator: vtable slot 3 (offset 0xC) frees a block.
uintptr_t FreeThrough(void* allocator, void* block, uint32_t size)
{
    return CallGameVirtual<uintptr_t, 3>(allocator, block, size);
}

struct AllocTriple {
    uint32_t begin;
    uint32_t tag;
    uint32_t end;
    void* allocator;
};

// Free a (begin, tag, end, allocator) block exactly like the original: signed span > 1 and
// a non-null begin; returns the free result or EAX before the call.
uintptr_t FreeTriple(const uint32_t* block)
{
    const int32_t span = static_cast<int32_t>(block[2] - block[0]);
    uintptr_t result = block[0];
    if (span > 1 && block[0] != 0)
        result = FreeThrough(*reinterpret_cast<void* const*>(block + 3),
                             reinterpret_cast<void*>(static_cast<uintptr_t>(block[0])),
                             static_cast<uint32_t>(span));
    return result;
}

// 0x005E6A30/0x009A0180 share the preset block up to (not including) the 0x500A40 string.
void InitRecordCommon(ScratchXmlRecord* r)
{
    SetInline(r->name10, kEmptyBegin, kEmptyCap1, kTagContentName);
    SetInline(r->desc20, kEmptyBegin, kEmptyCap1, kTagContentDesc);
    r->category = 0;
    InitTaggedString500A40(&r->transient34, 0);
    SetInline(r->named.name, kEmptyBegin, kEmptyCap2, kTagPresetStr);
    r->named.fields[0] = 0;
    r->named.fields[1] = 0xFFFFFFFFu;
    for (int i = 2; i < 10; ++i)
        r->named.fields[i] = 0;
    SetInline(r->path80, kEmptyBegin, kEmptyCap2, kTagWorldFile);
    SetEmptyVector(r->pairs90, kTagNameList);
    SetEmptyVector(r->pairsA0, kTagDescList);
    r->payloadB8[0] = 4;
    r->payloadB8[2] = 0;
    r->payloadB8[3] = 0;
    r->payloadB8[4] = 0;
    r->payloadB8[5] = 0;
    SetEmptyKeys(r->keys, kTagPackageIds);
}

}  // namespace

// ---------------------------------------------------------------------------------------
// 0x004A9B20
uint8_t ScratchLoaderHost::RunLocked()
{
    const uintptr_t lock = reinterpret_cast<uintptr_t>(lock270);
    CallGameMethod<int, 0x004E16F0>(reinterpret_cast<void*>(lock), GameAddr(0x00FB24F8));
    uint8_t result = 0;
    if (pendingWork != nullptr && running38D == 0) {
        running38D = 1;
        result = static_cast<uint8_t>(CallGameMethod<int, 0x004A9150>(this));
        running38D = 0;
    }
    CallGameMethod<int, 0x004E17B0>(reinterpret_cast<void*>(lock));
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x004AE820
int32_t ScratchByteRange::FindFrom(const ScratchByteRange& needle, uint32_t offset) const
{
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(begin));
    const uint32_t limit = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end));
    const uint32_t needleSize = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(needle.end) -
                                                      reinterpret_cast<uintptr_t>(needle.begin));
    if (needleSize + offset > limit - base)
        return -1;

    const uintptr_t found = CallGameFunction<uintptr_t, 0x004AE460>(
        reinterpret_cast<void*>(static_cast<uintptr_t>(base + offset)),
        reinterpret_cast<void*>(static_cast<uintptr_t>(limit)), needle.begin, needle.end);
    if (static_cast<uint32_t>(found) == limit && needleSize != 0)
        return -1;
    return static_cast<int32_t>(static_cast<uint32_t>(found) - base);
}

// 0x00876260
int32_t ScratchByteRange::OverwriteFrom(uint32_t offset, uint32_t count, const ScratchByteRange& source) const
{
    const uint32_t size = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(end) -
                                                reinterpret_cast<uintptr_t>(begin));
    const uint32_t left = size - offset;
    const uint32_t n = left < count ? left : count;
    uint8_t* const destination = static_cast<uint8_t*>(begin) + offset;
    return CopySpan4D2290(this, destination, destination + n, source.begin, source.end);
}

// ---------------------------------------------------------------------------------------
// 0x005E5550
ScratchExportDesc* ScratchExportDesc::Construct(char a2, char a3)
{
    flag11 = static_cast<uint8_t>(a3);
    flag10 = static_cast<uint8_t>(a2);
    key[0] = 0;
    key[1] = 0;
    key[2] = 0;
    key[3] = 0;
    SetInline(text14, kEmptyBegin, kEmptyCap2, kTagOutputFile);
    SetInline(text24, kEmptyBegin, kEmptyCap2, kTagExportDir);
    InitTaggedString500A40(&text34, 0);
    tail44 = 0;
    return this;
}

// 0x005E5890
uintptr_t ScratchExportDesc::Destroy()
{
    uintptr_t result = 0;
    if (HeapBlock(text24, 0xFFFFFFFEu, 2))
        result = reinterpret_cast<uintptr_t>(Free4E3F70(text24.begin));
    result = reinterpret_cast<uintptr_t>(text14.begin);
    if (HeapBlock(text14, 0xFFFFFFFEu, 2))
        result = reinterpret_cast<uintptr_t>(Free4E3F70(text14.begin));
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x005E6A30
ScratchXmlRecord* ScratchXmlRecord::InitFromKey(const uint32_t* sourceKey)
{
    key[0] = sourceKey[0];
    key[1] = sourceKey[1];
    key[2] = sourceKey[2];
    key[3] = sourceKey[3];
    InitRecordCommon(this);
    flags7C = 0;
    flagB0 = 1;
    return this;
}

// 0x009A0180
ScratchXmlRecord* ScratchXmlRecord::InitCopy(const uint32_t* sourceKey, const ScratchXmlRecord& source)
{
    key[0] = sourceKey[0];
    key[1] = sourceKey[1];
    key[2] = sourceKey[2];
    key[3] = sourceKey[3];
    AssignTaggedString405AD0(&name10, &source.name10);
    AssignTaggedString405AD0(&desc20, &source.desc20);
    InitTaggedString500A40(&transient34, 0);
    SetInline(named.name, kEmptyBegin, kEmptyCap2, kTagPresetStr);
    named.fields[0] = 0;
    named.fields[1] = 0xFFFFFFFFu;
    for (int i = 2; i < 10; ++i)
        named.fields[i] = 0;
    AssignBlock408EB0(&path80, &source.path80);
    SetEmptyVector(pairs90, kTagNameList);
    SetEmptyVector(pairsA0, kTagDescList);
    flagB0 = source.flagB0;
    payloadB8[0] = 4;
    payloadB8[2] = 0;
    payloadB8[3] = 0;
    payloadB8[4] = 0;
    payloadB8[5] = 0;
    SetEmptyKeys(keys, kTagPackageIds);
    // 0x009A0100 is already rewritten and verified; the layouts are identical.
    reinterpret_cast<ScratchContentRecord*>(this)->Assign(
        reinterpret_cast<const ScratchContentRecord*>(&source));
    return this;
}

// 0x005E7260
uintptr_t ScratchXmlRecord::Destroy()
{
    uintptr_t result = 0;
    if (keys.begin != nullptr)
        result = reinterpret_cast<uintptr_t>(Free4E3F70(keys.begin));

    FreeScratchPairs(pairsA0.begin, pairsA0.end);
    if (pairsA0.begin != nullptr)
        result = reinterpret_cast<uintptr_t>(Free4E3F70(pairsA0.begin));

    FreeScratchPairs(pairs90.begin, pairs90.end);
    if (pairs90.begin != nullptr)
        result = reinterpret_cast<uintptr_t>(Free4E3F70(pairs90.begin));

    result = FreeTagged(path80, 0xFFFFFFFEu, 2);
    result = FreeTagged(named.name, 0xFFFFFFFEu, 2);
    result = FreeTagged(desc20, 0xFFFFFFFFu, 1);
    return FreeTagged(name10, 0xFFFFFFFFu, 1);
}

// ---------------------------------------------------------------------------------------
// 0x00985590
uintptr_t ScratchStringTriple::Destroy()
{
    uintptr_t result = 0;
    result = FreeTagged(c, 0xFFFFFFFFu, 1);
    result = FreeTagged(b, 0xFFFFFFFFu, 1);
    return FreeTagged(a, 0xFFFFFFFFu, 1);
}

// ---------------------------------------------------------------------------------------
// 0x0099E4B0
ScratchWordPair* ScratchPairListHost::AppendKey(uint32_t a, uint32_t b)
{
    ScratchWordPair* const end = keyEnd;
    if (end >= keyCapacity) {
        ScratchWordPair pair{a, b};
        return reinterpret_cast<ScratchWordPair*>(CallGameFunction<uintptr_t, 0x006202E0>(end, &pair));
    }
    keyEnd = end + 1;
    if (end == nullptr)
        return nullptr;
    end->a = a;
    end->b = b;
    return end;
}

// ---------------------------------------------------------------------------------------
// 0x004B4880
void ScratchListHead::DestroyNodes()
{
    ScratchListRecord* node = reinterpret_cast<ScratchListRecord*>(next);
    while (reinterpret_cast<ScratchListHead*>(node) != this) {
        ScratchListRecord* const current = node;
        node = current->next;
        if (static_cast<int32_t>(current->end28 - current->block20) > 1 && current->block20 != 0)
            FreeThrough(current->allocator2C,
                        reinterpret_cast<void*>(static_cast<uintptr_t>(current->block20)),
                        current->end28 - current->block20);
        if (static_cast<int32_t>(current->end10 - current->block08) > 1 && current->block08 != 0)
            FreeThrough(current->allocator14,
                        reinterpret_cast<void*>(static_cast<uintptr_t>(current->block08)),
                        current->end10 - current->block08);
        FreeThrough(nodeAllocator, current, 56);
    }
}

// ---------------------------------------------------------------------------------------
// 0x004B4560
uintptr_t ScratchContainer::Destroy4B4560()
{
    vtable = GameAddr(0x00FB3018);
    DestroyHashList4B4290(this);

    ScratchListRecord* node = reinterpret_cast<ScratchListRecord*>(list24);
    ScratchListRecord* const sentinel = reinterpret_cast<ScratchListRecord*>(&list24);
    while (node != sentinel) {
        ScratchListRecord* const current = node;
        node = current->next;
        FreeThrough(allocator2C, current, 12);
    }

    const uint32_t span = end10 - block08;
    if (static_cast<int32_t>(span) > 1 && block08 != 0)
        return FreeThrough(allocator14, reinterpret_cast<void*>(static_cast<uintptr_t>(block08)), span);
    return span;
}

// 0x004B4AF0
uintptr_t ScratchContainer::Destroy4B4AF0()
{
    FreeTriple(&block68);
    FreeTriple(&block50);
    FreeTriple(&block38);
    return Destroy4B4560();
}

// 0x004B4CD0
void ScratchContainer::Reset4B4CD0()
{
    DestroyHashList4B4290(this);
    ScratchListHead* const head = reinterpret_cast<ScratchListHead*>(
        reinterpret_cast<uint8_t*>(this) + 0x38);
    head->DestroyNodes();
    // The original self-links the list head after destroying it.
    head->next = head;
    head->prev = head;
}

// 0x004B4B60
ScratchContainer* ScratchContainer::Delete4B4B60(uint32_t flags)
{
    Destroy4B4AF0();
    if ((flags & 1u) != 0)
        Free4E3F70(this);
    return this;
}

// 0x004B4C00
ScratchContainer* ScratchContainer::Delete4B4C00(uint32_t flags)
{
    if (static_cast<int32_t>(end40 - block38) > 1 && block38 != 0)
        FreeThrough(allocator44, reinterpret_cast<void*>(static_cast<uintptr_t>(block38)), end40 - block38);
    Destroy4B4560();
    if ((flags & 1u) != 0)
        Free4E3F70(this);
    return this;
}

// 0x004B4F80
ScratchContainer* ScratchContainer::Delete4B4F80(uint32_t flags)
{
    reinterpret_cast<ScratchListHead*>(reinterpret_cast<uint8_t*>(this) + 0x38)->DestroyNodes();
    Destroy4B4560();
    if ((flags & 1u) != 0)
        Free4E3F70(this);
    return this;
}

// ---------------------------------------------------------------------------------------
// 0x0087D770
uintptr_t ScratchContainer87D::Destroy87D770()
{
    vtable = GameAddr(0x0102B99C);
    container54.Destroy4B4560();
    uintptr_t result = 0;
    result = FreeTagged(s44, 0xFFFFFFFFu, 1);
    result = FreeTagged(s34, 0xFFFFFFFFu, 1);
    result = FreeTagged(s24, 0xFFFFFFFFu, 1);
    result = FreeTagged(s14, 0xFFFFFFFFu, 1);
    return FreeTagged(s04, 0xFFFFFFFFu, 1);
}

// 0x0087D810
ScratchContainer87D* ScratchContainer87D::Delete87D810(uint32_t flags)
{
    Destroy87D770();
    if ((flags & 1u) != 0)
        Free4E3F70(this);
    return this;
}

// ---------------------------------------------------------------------------------------
// 0x0088EC50
ScratchString* __cdecl ConcatCStringSpan(ScratchString* out, const char* text, const ScratchString* tail)
{
    const size_t length = std::strlen(text);
    const uint32_t total = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tail->end) -
                                                 reinterpret_cast<uintptr_t>(tail->begin) + length + 1);
    out->begin = nullptr;
    out->end = nullptr;
    out->capacity = nullptr;
    out->label = tail->label;
    Reserve4058B0(out, static_cast<int32_t>(total));
    *static_cast<uint8_t*>(out->end) = 0;
    Append405790(out, text, text + length);
    Append405790(out, tail->begin, tail->end);
    return out;
}

// 0x009C65C0
ScratchString* __cdecl ConcatSpanCString(ScratchString* out, const ScratchString* head, const char* text)
{
    const size_t length = std::strlen(text);
    const uint32_t total = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(head->end) -
                                                 reinterpret_cast<uintptr_t>(head->begin) + length + 1);
    out->begin = nullptr;
    out->end = nullptr;
    out->capacity = nullptr;
    out->label = head->label;
    Reserve4058B0(out, static_cast<int32_t>(total));
    *static_cast<uint8_t*>(out->end) = 0;
    Append405790(out, head->begin, head->end);
    Append405790(out, text, text + length);
    return out;
}

// ---------------------------------------------------------------------------------------
// 0x005E8FA0
uint8_t ScratchExportSource::Export(void* source, ScratchXmlRecord* record)
{
    if (record == nullptr)
        return 0;
    if (source == nullptr) {
        CallGameFunction<int, 0x009AE400>(*static_cast<void**>(GameAddr(0x0115B44)), 0);
        record->Destroy();
        Free4E3F70(record);
        return 0;
    }

    ScratchExportDesc desc;
    desc.Construct(0, 1);
    record->payloadB8[0] = 5;
    record->payloadB8[6] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source) + 0x30);

    const uint8_t result =
        static_cast<uint8_t>(CallGameFunction<int, 0x009AF7C0>(record, &desc, 0));
    // EA quirk: only BL of the argument slot is overwritten, so the upper 24 bits of the
    // source pointer argument survive into the 0x009AE400 call.
    const uint32_t reported =
        (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source)) & 0xFFFFFF00u) | result;
    CallGameFunction<int, 0x009AE400>(*static_cast<void**>(GameAddr(0x0115B44)), reported);
    CallGameVirtual<int, 1>(this);
    desc.Destroy();
    CallGameVirtual<int, 2>(source);
    record->Destroy();
    Free4E3F70(record);
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x009C1100
uint8_t __stdcall ExportFirstItem(uint32_t, const ScratchByteRange* items, uint32_t, uint32_t, uint32_t,
                                  uint32_t, uint32_t)
{
    void* const first = items->begin;
    if (first == items->end)
        return 0;
    if (CallGameFunction<uint8_t, 0x0099B250>(static_cast<uint8_t*>(first) + 8) == 0)
        return 0;

    ScratchXmlRecord record;
    record.InitFromKey(static_cast<const uint32_t*>(first));
    ScratchExportDesc desc;
    desc.Construct(0, 1);
    const uint8_t result =
        static_cast<uint8_t>(CallGameFunction<int, 0x009AF7C0>(&record, &desc, 0));
    desc.Destroy();
    record.Destroy();
    return result;
}

// ---------------------------------------------------------------------------------------
// 0x009CFA30
ScratchString* ScratchXmlTagHost::ReplaceXmlAttribute(ScratchString* out, uint32_t textBegin,
                                                      uint32_t textEnd, uint32_t textCap,
                                                      int32_t value)
{
    ScratchString open;
    ConcatCStringSpan(&open, "<", &tag4C);
    ScratchString openClose;
    ConcatSpanCString(&openClose, &open, ">");
    FreeTagged(open, 0xFFFFFFFFu, 1);

    ScratchString close;
    ConcatCStringSpan(&close, "</", &tag4C);
    ScratchString closeClose;
    ConcatSpanCString(&closeClose, &close, ">");
    FreeTagged(close, 0xFFFFFFFFu, 1);

    const uint32_t openCloseLength = static_cast<uint32_t>(SignedSpan(openClose.begin, openClose.end));
    const uint32_t textLength = static_cast<uint32_t>(SignedSpan(
        reinterpret_cast<const void*>(textBegin), reinterpret_cast<const void*>(textEnd)));
    // Both searches mirror the original: a length mismatch or the (found == end && length != 0)
    // sentinel of 0x004AE460 leaves the position at -1.
    int32_t positionOpen = -1;
    if (openCloseLength <= textLength) {
        const uintptr_t found = CallGameFunction<uintptr_t, 0x004AE460>(
            textBegin, textEnd, openClose.begin, openClose.end);
        if (!(found == textEnd && openCloseLength != 0))
            positionOpen = static_cast<int32_t>(static_cast<uint32_t>(found - textBegin));
    }
    const uint32_t closeCloseLength = static_cast<uint32_t>(SignedSpan(closeClose.begin, closeClose.end));
    int32_t positionClose = -1;
    if (closeCloseLength <= textLength) {
        const uintptr_t found = CallGameFunction<uintptr_t, 0x004AE460>(
            textBegin, textEnd, closeClose.begin, closeClose.end);
        if (!(found == textEnd && closeCloseLength != 0))
            positionClose = static_cast<int32_t>(static_cast<uint32_t>(found - textBegin));
    }

    // All arithmetic is unsigned in the original, so the not-found (-1) case wraps instead of
    // being clamped; 0x004059F0 then copies what the buffer happens to hold.
    const uint32_t contentOffset = openCloseLength + static_cast<uint32_t>(positionOpen);
    const uint32_t remaining = textLength - contentOffset;
    const uint32_t contentLength =
        static_cast<uint32_t>(positionClose) - static_cast<uint32_t>(positionOpen) - openCloseLength;
    const uint32_t count = remaining < contentLength ? remaining : contentLength;
    CallGameMethod<int, 0x004059F0>(out, reinterpret_cast<void*>(textBegin + contentOffset),
                                    reinterpret_cast<void*>(textBegin + contentOffset + count), &value);

    FreeTagged(closeClose, 0xFFFFFFFFu, 1);
    FreeTagged(openClose, 0xFFFFFFFFu, 1);
    ScratchString text;
    text.begin = reinterpret_cast<void*>(textBegin);
    text.end = reinterpret_cast<void*>(textEnd);
    text.capacity = reinterpret_cast<void*>(textCap);
    text.label = nullptr;
    FreeTagged(text, 0xFFFFFFFFu, 1);
    return out;
}

// ---------------------------------------------------------------------------------------
// 0x009CFBD0
uint8_t __stdcall XmlTagMatches(ScratchString tag, ScratchString text)
{
    ScratchString open;
    ConcatCStringSpan(&open, "<", &tag);
    ScratchString openClose;
    ConcatSpanCString(&openClose, &open, ">");
    FreeTagged(open, 0xFFFFFFFFu, 1);

    ScratchString close;
    ConcatCStringSpan(&close, "</", &tag);
    ScratchString closeClose;
    ConcatSpanCString(&closeClose, &close, ">");
    FreeTagged(close, 0xFFFFFFFFu, 1);

    const uint32_t textLength = static_cast<uint32_t>(SignedSpan(text.begin, text.end));
    uint8_t result = 0;

    const uint32_t openCloseLength = static_cast<uint32_t>(SignedSpan(openClose.begin, openClose.end));
    uintptr_t foundOpen = reinterpret_cast<uintptr_t>(text.end);
    const bool openSearch = openCloseLength <= textLength;
    if (openSearch)
        foundOpen = CallGameFunction<uintptr_t, 0x004AE460>(text.begin, text.end, openClose.begin,
                                                           openClose.end);
    const bool openMissing =
        !openSearch ||
        (foundOpen == reinterpret_cast<uintptr_t>(text.end) && openCloseLength != 0) ||
        static_cast<int32_t>(static_cast<uint32_t>(
            foundOpen - reinterpret_cast<uintptr_t>(text.begin))) == -1;
    if (!openMissing) {
        const uint32_t closeCloseLength =
            static_cast<uint32_t>(SignedSpan(closeClose.begin, closeClose.end));
        uintptr_t foundClose = reinterpret_cast<uintptr_t>(text.end);
        const bool closeSearch = closeCloseLength <= textLength;
        if (closeSearch)
            foundClose = CallGameFunction<uintptr_t, 0x004AE460>(text.begin, text.end, closeClose.begin,
                                                                closeClose.end);
        const bool closeMissing =
            !closeSearch ||
            (foundClose == reinterpret_cast<uintptr_t>(text.end) && closeCloseLength != 0) ||
            static_cast<int32_t>(static_cast<uint32_t>(
                foundClose - reinterpret_cast<uintptr_t>(text.begin))) == -1;
        if (!closeMissing)
            result = 1;
    }

    FreeTagged(closeClose, 0xFFFFFFFFu, 1);
    FreeTagged(openClose, 0xFFFFFFFFu, 1);
    FreeTagged(tag, 0xFFFFFFFFu, 1);
    FreeTagged(text, 0xFFFFFFFFu, 1);
    return result;
}

namespace {

// 0x009A02B0's map object (the 0x004B4F30/0x004B5440/0x004B4560 target): the prefix through
// +0x2C mirrors ScratchContainer (0x004B4560 walks only that far), then the exporter's own
// one-node list at +0x38 and the 0x0099E680/container-key area at +0x4C.
struct ScratchExportMap1 {
    void* vtable;        // +0x00
    uint32_t field04;    // +0x04
    uint32_t block08;    // +0x08
    uint32_t pad0C;
    uint32_t end10;      // +0x10
    void* allocator14;   // +0x14
    uint32_t pad18[3];
    void* list24;        // +0x24 (the list 0x004B4560 destroys)
    void* list28;        // +0x28
    void* allocator2C;   // +0x2C
    uint32_t pad30[2];
    uint32_t listNext;   // +0x38 (list sentinel 0x004B4880 is pointed at)
    uint32_t listTail;   // +0x3C
    void* allocator;     // +0x40 (vtable slot 2 allocates the 0x38-byte node)
    uint32_t slot44;     // +0x44 (third allocation argument)
    uint32_t slot48;     // +0x48 (second allocation argument)
    uint8_t key[16];     // +0x4C (0x0099E680 and virtual slot 8 use this)
};

// 0x009A02B0's pair-block object (the 0x004B48F0 target): two (begin, pad, end, allocator)
// blocks 0x18 bytes apart; each is freed through its allocator's virtual slot 3.
struct ScratchExportBlock {
    void* begin;     // +0x00
    uint32_t pad04;
    void* end;       // +0x08
    void* allocator; // +0x0C
};
struct ScratchExportMap2 {
    ScratchExportBlock blockA;  // +0x00
    uint32_t pad10[2];          // +0x10
    ScratchExportBlock blockB;  // +0x18
};

uintptr_t FreeExportBlock(const ScratchExportBlock& block)
{
    const int32_t span = static_cast<int32_t>(static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(block.end) - reinterpret_cast<uintptr_t>(block.begin)));
    uintptr_t result = reinterpret_cast<uintptr_t>(block.begin);
    if (span > 1 && block.begin != nullptr)
        result = FreeThrough(block.allocator, block.begin, static_cast<uint32_t>(span));
    return result;
}

// The original loads the pointer *stored at* 0x01180808 (the pair table) and 0x0118080C
// (the 0x004B48F0 table), it does not pass the addresses of those globals.
void* LoadExportTable(uintptr_t address) { return *reinterpret_cast<void**>(GameAddr(address)); }

}  // namespace

// 0x009A02B0 (covers the 0x009A02CE manifest entry, see the header).
uint8_t __cdecl ExportStringTables(ScratchXmlRecord* source, void* out)
{
    void* const context = CallGameFunction<void*, 0x00931020>();
    if (context == nullptr)
        return 0;

    // The enumeration vector: a tagged string 0x004AFD20's container slot 8 fills in place.
    // At the 0x009A02CE jump EAX is 0 (the prologue zeroes it), so all three pointers
    // start null and only the label is preset.
    ScratchString enumerated;
    enumerated.begin = nullptr;
    enumerated.end = nullptr;
    enumerated.capacity = nullptr;
    enumerated.label = static_cast<const char*>(GameAddr(0x0104ACA8u));

    ScratchString formatted;
    SetInline(formatted, kEmptyBegin, kEmptyCap1, 0x0104AC78u);

    void* const container = CallGameFunction<void*, 0x004AFD20>();
    if (container == nullptr) {
        // The cleanup frees the (still null) enumeration vector begin and returns 0.
        if (enumerated.begin != nullptr)
            Free4E3F70(enumerated.begin);
        return 0;
    }

    ScratchExportMap1 map1;
    ScratchExportMap2 map2;
    std::memset(&map1, 0, sizeof map1);
    std::memset(&map2, 0, sizeof map2);

    CallGameMethod<int, 0x0099E680>(&map1.key[0], source);
    CallGameVirtual<int, 8>(container, &enumerated, &map1.key[0], 0u);

    uint32_t* item = static_cast<uint32_t*>(enumerated.begin);
    uint32_t* const itemEnd = static_cast<uint32_t*>(enumerated.end);
    ScratchPairListHost* const host = *static_cast<ScratchPairListHost* const*>(out);

    uint8_t result = 1;
    if (item != itemEnd) {
        for (;;) {
            ScratchXmlRecord record;
            std::memset(&record, 0, sizeof record);
            record.InitCopy(item, *source);
            if (CallGameFunction<uint8_t, 0x009AEB70>(&record, out, 0u) == 0) {
                record.Destroy();
                result = 0;
                break;
            }
            // The 64-bit argument is item[0] (low) + item[1] (high).
            CallGameFunction<int, 0x00590900>(&formatted, "%d:%08x:%08x:%I64x", 0, item[2],
                                              item[3], *reinterpret_cast<const uint64_t*>(item));
            CallGameMethod<int, 0x004B4F30>(&map1, LoadExportTable(0x01180808u), 0u, context);
            void* const bucket = CallGameMethod<void*, 0x004B48F0>(
                &map2, LoadExportTable(0x0118080Cu), formatted.begin, context);
            uint32_t* const node = CallGameVirtual<uint32_t*, 2>(
                map1.allocator, 0x38u, map1.slot48, map1.slot44);
            if (node != reinterpret_cast<uint32_t*>(0xFFFFFFF8u))
                CallGameMethod<int, 0x004B45F0>(node + 2, bucket);
            node[0] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&map1.listNext));
            node[1] = map1.listTail;
            *reinterpret_cast<uint32_t**>(map1.listTail) = node;
            map1.listTail = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(node));

            FreeExportBlock(map2.blockB);  // 0x004B48F0 fills +0x18 first, freed first
            FreeExportBlock(map2.blockA);

            const uintptr_t mapResult = CallGameMethod<uintptr_t, 0x004B5440>(&map1);
            uintptr_t hash = 0;
            void* const hashTable = LoadExportTable(0x01180808u);
            if (hashTable != nullptr)
                hash = CallGameFunction<uintptr_t, 0x004FA590>(hashTable, 0x811C9DC5u, 0u);
            host->AppendKey(static_cast<uint32_t>(hash), static_cast<uint32_t>(mapResult));

            reinterpret_cast<ScratchListHead*>(&map1.listNext)->DestroyNodes();
            reinterpret_cast<ScratchContainer*>(&map1)->Destroy4B4560();
            record.Destroy();

            item += 4;
            if (item == itemEnd)
                break;
        }
    }
    // Both exits free the formatted string when its capacity span is still > 1 and the
    // enumeration buffer whenever its begin pointer is set.
    if (SignedSpan(formatted.begin, formatted.capacity) > 1 && formatted.begin != nullptr)
        Free4E3F70(formatted.begin);
    if (enumerated.begin != nullptr)
        Free4E3F70(enumerated.begin);
    return result;
}

}  // namespace ts3::scratch