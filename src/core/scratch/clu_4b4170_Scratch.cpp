#include <initializer_list>
#include "ts3/scratch/clu_4b4170_Scratch.h"

namespace ts3::scratch {
namespace {
uintptr_t Address(const void* p) { return reinterpret_cast<uintptr_t>(p); }
uintptr_t Free(void* p) { return CallGameFunction<uintptr_t, 0x004E3F70>(p); }
// Compare a signed pointer-width span; do not subtract unrelated C++ pointers.
intptr_t Span(const void* end, const void* begin) {
    return static_cast<intptr_t>(Address(end) - Address(begin));
}
uintptr_t FreeWide(ScratchString& s) {
    const intptr_t span = Span(s.capacity, s.begin) & ~intptr_t(1);
    if (span > 2 && s.begin) return Free(s.begin);
    return Address(s.begin);
}
}

// 0x004B4170. Capture allocator BEFORE the destructor (which may change the object).
uintptr_t ScratchOwnedObject::Release() {
    void* saved = allocator;
    CallGameVirtual<void, 0>(this, uint32_t(0));
    return CallGameVirtual<uintptr_t, 3>(saved, this, uint32_t(0));
}

// 0x004EFBA0. Empty-range EAX has no standalone return contract; use end as
// the seed because all in-unit wrappers load it into EAX immediately before calling.
uintptr_t __stdcall FreeScratchPairs(ScratchPair* begin, const ScratchPair* end) {
    uintptr_t result = Address(end);
    for (auto p = begin; Address(p) < Address(end); ++p) {
        FreeWide(p->second);
        result = FreeWide(p->first);
    }
    return result;
}
// 0x008A7BB0. Capacity subtraction is SIGNED in the assembly (jle).
uintptr_t __stdcall FreeScratchStrings(ScratchString* begin, const ScratchString* end) {
    uintptr_t result = Address(end);
    for (auto p = begin; Address(p) < Address(end); ++p) {
        result = Address(p->begin);
        if (Span(p->capacity, p->begin) > 1 && p->begin) result = Free(p->begin);
    }
    return result;
}
// 0x009B4280.
uintptr_t __stdcall FreeScratchTraits(ScratchTrait* begin, const ScratchTrait* end) {
    uintptr_t result = Address(end);
    for (auto p = begin; Address(p) < Address(end); ++p) {
        FreeWide(p->text);
        result = Address(p->value);
        if (p->value) result = Free(p->value);
    }
    return result;
}
// 0x009B42D0.
uintptr_t __stdcall FreeScratchOutfits(ScratchOutfit* begin, const ScratchOutfit* end) {
    uintptr_t result = Address(end);
    for (auto p = begin; Address(p) < Address(end); ++p) {
        FreeWide(p->strings.second);
        result = FreeWide(p->strings.first);
    }
    return result;
}
// 0x009B3600. This function invokes arbitrary virtual code; it is NOT compare-safe.
uint8_t __stdcall OpenScratchResource(void* resource) {
    if (!resource) return 0;
#if defined(_M_IX86)
    return static_cast<uint8_t>(CallGameVirtual<uint32_t, 13>(resource,
        reinterpret_cast<void*>(uintptr_t(0x01180EA0)), uint32_t(0), uint32_t(1),
        uint32_t(6), uint32_t(1), uint32_t(0)));
#else
    AbortNotRewritten("OpenScratchResource game constant");
    return 0;
#endif
}
// 0x009B3630. end is decremented AFTER each destructor, not before it.
uintptr_t ScratchObjects::Clear() {
    uintptr_t result = Address(begin);
    while (begin != end) {
        auto object = (end - 1)->object;
        result = Address(object);
        if (object) result = object->Release();
        --end;
    }
    return result;
}
// 0x009B39F0.
uintptr_t ScratchObjects::Destroy() {
    uintptr_t result = Clear();
    if (begin) return Free(begin);
    return result;
}
// 0x009B52B0. Deliberately don't special-case self-copy: Clear empties the source.
void* ScratchObjects::Copy(const ScratchObjects* source) {
    Clear();
    void* result = const_cast<ScratchObjects*>(source);
    auto p = source->begin;
    const auto last = source->end;
    for (; p != last; ++p) {
        ScratchOwnedObject* clone = p->object ? CallGameVirtual<ScratchOwnedObject*, 1>(p->object) : nullptr;
        ScratchObjectEntry item{p->key, clone};
        auto destination = end;
        result = destination;
        if (Address(destination) < Address(capacity)) {
            end = reinterpret_cast<ScratchObjectEntry*>(Address(destination) + sizeof(ScratchObjectEntry));
            if (destination) *destination = item;
        } else {
            result = CallGameMethod<void*, 0x006202E0>(this, destination, &item);
        }
    }
    return result;
}
// 0x009B5F50.
ScratchObjects* ScratchObjects::Assign(const ScratchObjects* source) {
    Copy(source);
    return this;
}

namespace {
template<class T> uint32_t Count(T* end, T* begin) {
    // The machine subtracts addresses, arithmetic-shifts, then compares UNSIGNED.
    const auto span = Span(end, begin);
    const auto stride = static_cast<intptr_t>(sizeof(T));
    return static_cast<uint32_t>(span / stride - (span < 0 && span % stride != 0 ? 1 : 0));
}
template<class T> T* Advance(T* p, uint32_t count) {
    return reinterpret_cast<T*>(Address(p) + uintptr_t(count) * sizeof(T));
}
void AssignWide(ScratchString* to, const ScratchString* from) {
    if (to != from) CallGameMethod<void, 0x00408E20>(to, from->begin, from->end);
}
}
// 0x0099D6B0.
ScratchPair* CopyScratchPairs(const ScratchPair* begin, const ScratchPair* end, ScratchPair* destination) {
    for (; begin != end; ++begin, ++destination) {
        AssignWide(&destination->first, &begin->first);
        AssignWide(&destination->second, &begin->second);
    }
    return destination;
}
// 0x0099CB60.
ScratchNamedRecord* ScratchNamedRecord::Assign(const ScratchNamedRecord* source) {
    AssignWide(&name, &source->name);
    for (unsigned i=0; i<10; ++i) fields[i] = source->fields[i];
    return this;
}
// 0x004F0750.
ScratchPair* ScratchPairs::Reserve(uint32_t count) {
    auto result = begin;
    if (count > Count(capacity, begin)) {
        auto replacement = CallGameMethod<ScratchPair*, 0x004F06A0>(this, count, begin, end);
        FreeScratchPairs(begin, end);
        if (begin) Free(begin);
        result = Advance(replacement, Count(end, begin));
        begin = replacement;
        end = result;
        capacity = Advance(replacement, count);
    }
    return result;
}
// 0x009A0010.
ScratchPairs* ScratchPairs::Assign(const ScratchPairs* source) {
    if (source == this) return this;
    const auto count = Count(source->end, source->begin);
    if (count > Count(capacity, begin)) {
        auto replacement = CallGameMethod<ScratchPair*, 0x004F06A0>(this, count, source->begin, source->end);
        FreeScratchPairs(begin, end);
        if (begin) Free(begin);
        end = capacity = Advance(replacement, count);
        begin = replacement;
    } else if (count > Count(end, begin)) {
        CopyScratchPairs(source->begin, Advance(source->begin, Count(end, begin)), begin);
        ScratchPair* constructed = nullptr;
        CallGameFunction<void, 0x004EFC00>(&constructed,
            Advance(source->begin, Count(end, begin)), source->end, end, source);
        end = Advance(begin, count);
    } else {
        auto tail = CopyScratchPairs(source->begin, source->end, begin);
        FreeScratchPairs(tail, end);
        end = Advance(begin, count);
    }
    return this;
}
// 0x007DE2C0.
ScratchKeys* ScratchKeys::Assign(const ScratchKeys* source) {
    if (source == this) return this;
    const auto count = Count(source->end, source->begin);
    if (count > Count(capacity, begin)) {
        auto replacement = CallGameMethod<ScratchKey*, 0x007DE0A0>(this, count, source->begin, source->end);
        if (begin) Free(begin);
        end = capacity = Advance(replacement, count);
        begin = replacement;
    } else if (count > Count(end, begin)) {
        CallGameFunction<void, 0x007DD8D0>(source->begin, Advance(source->begin, Count(end, begin)), begin);
        ScratchKey* constructed = nullptr;
        CallGameFunction<void, 0x007DDC50>(&constructed,
            Advance(source->begin, Count(end, begin)), source->end, end, source);
        end = Advance(begin, count);
    } else {
        CallGameFunction<void, 0x007DD8D0>(source->begin, source->end, begin);
        end = Advance(begin, count);
    }
    return this;
}

// 0x009A0100. 0x500310 remains another unit's key-copy method, called through the bridge.
ScratchKey* ScratchContentRecord::Assign(const ScratchContentRecord* source) {
    named.Assign(&source->named);
    flags = source->flags;
    for (unsigned i=0; i<8; ++i) payload[i] = source->payload[i];
    category = source->category;
    pairs90.Assign(&source->pairs90);
    pairsA0.Assign(&source->pairsA0);
    keys.Assign(&source->keys);
    return CallGameMethod<ScratchKey*, 0x00500310>(&key, &source->key);
}
// 0x009B04E0.
ScratchVersion* ScratchVersion::Parse(const char* text) {
    ScratchString temporary;
    auto converted = CallGameFunction<ScratchString*, 0x004F38C0>(&temporary, text, int32_t(-1), uint32_t(0));
    CallGameMethod<void, 0x009B01D0>(this, converted->begin);
    if (Span(temporary.capacity, temporary.begin) > 1 && temporary.begin) Free(temporary.begin);
    return this;
}
// 0x009B0530. Read and write the game's actual cached bytes, not process-local statics.
uint8_t IsScratchZeroVersion() {
#if defined(_M_IX86)
    auto& initialized = *reinterpret_cast<uint8_t*>(uintptr_t(0x011E96C1));
    auto& cached = *reinterpret_cast<uint8_t*>(uintptr_t(0x011E96C0));
    if (initialized) return cached;
    auto text = CallGameFunction<const char*, 0x0058A900>(
        reinterpret_cast<const void*>(uintptr_t(0x0104C490)),
        reinterpret_cast<const void*>(uintptr_t(0x0104C4A0)),
        reinterpret_cast<const void*>(uintptr_t(0x0104C4E4)));
    ScratchVersion version;
    version.Parse(text);
    initialized = 1;
    const uint8_t result = (version.parts[0] == 0 && version.parts[1] == 0 && version.parts[2] == 0 && version.parts[3] == 0) ? 1 : 0;
    cached = result;
    return result;
#else
    AbortNotRewritten("IsScratchZeroVersion game globals");
    return 0;
#endif
}

namespace {
uintptr_t FreeNarrow(ScratchString& s) {
    if (Span(s.capacity, s.begin) > 1 && s.begin) return Free(s.begin);
    return Address(s.begin);
}
void InitMetadataString(ScratchString& s, unsigned index) {
#if defined(_M_IX86)
    static const uintptr_t labels[] = {0x0104E748,0x0104E70C,0x0104E6D8,0x0104E6A0,
        0x0104E66C,0x0104E600,0x0104E5C8,0x0104E44C,0x0104E590,0x0104E41C};
    s = {reinterpret_cast<void*>(uintptr_t(0x011C7E54)),reinterpret_cast<void*>(uintptr_t(0x011C7E54)),
        reinterpret_cast<void*>(uintptr_t(0x011C7E56)),reinterpret_cast<const char*>(labels[index])};
#else
    static char16_t empty[1] = {};
    static const char* labels[] = {
        "Scratch/DownloadContent/Manifest/MetaData/expirationDuration", "Scratch/DownloadContent/Manifest/MetaData/sendToFriendID",
        "Scratch/DownloadContent/Manifest/MetaData/lotName", "Scratch/DownloadContent/Manifest/MetaData/householdName",
        "Scratch/DownloadContent/Manifest/MetaData/actorName", "Scratch/DownloadContent/Manifest/MetaData/matCategory",
        "Scratch/DownloadContent/Manifest/MetaData/patternType", "Scratch/DownloadContent/Manifest/MetaData/name",
        "Scratch/DownloadContent/Manifest/MetaData/description", "Scratch/DownloadContent/Manifest/MetaData/bio"};
    s = {empty,empty,empty+1,labels[index]};
#endif
}
void SetMetadataLabels(ScratchMetadata& m) {
#if defined(_M_IX86)
    m.friends.label = reinterpret_cast<const char*>(uintptr_t(0x0104E638));
    m.traitIds.label = reinterpret_cast<const char*>(uintptr_t(0x0104E55C));
    m.traits.label = reinterpret_cast<const char*>(uintptr_t(0x0104E528));
    m.outfits.label = reinterpret_cast<const char*>(uintptr_t(0x0104E4EC));
    m.householdMembers.label = reinterpret_cast<const char*>(uintptr_t(0x0104E4B4));
    m.validMetaTags.label = reinterpret_cast<const char*>(uintptr_t(0x0104E47C));
#else
    m.friends.label = "Scratch/DownloadContent/Manifest/MetaData/friends";
    m.traitIds.label = "Scratch/DownloadContent/Manifest/MetaData/traits";
    m.traits.label = "Scratch/DownloadContent/Manifest/Metadata/outfits";
    m.outfits.label = "Scratch/DownloadContent/Manifest/MetaData/householdMembers";
    m.householdMembers.label = "Scratch/DownloadContent/Manifest/Metadata/lotThumbnails";
    m.validMetaTags.label = "Scratch/DownloadContent/Manifest/MetaData/validMetaTags";
#endif
}
}
// 0x007685D0.
uintptr_t ScratchStrings::Destroy() {
    auto result = FreeScratchStrings(begin, end);
    if (begin) return Free(begin);
    return result;
}
// 0x009B48F0.
uintptr_t ScratchPairs::Clear() {
    auto result = FreeScratchPairs(begin, end);
    end = begin;
    return result;
}
// 0x009A80A0. Empty range leaves EAX unspecified; populated ranges return the last free/first pointer.
uintptr_t __stdcall FreeScratchDownloadParts(ScratchDownloadPart* begin, const ScratchDownloadPart* end) {
    uintptr_t result = 0;
    for (auto p=begin; Address(p)<Address(end); ++p) {
        p->firstEnd = p->first;
        p->secondEnd = p->second;
        if (p->second) Free(p->second);
        result = Address(p->first);
        if (p->first) result = Free(p->first);
    }
    return result;
}
// 0x009B4990.
uintptr_t ScratchMetadata::Clear() {
    traitIds.end = traitIds.begin;
    friends.end = friends.begin;
    validMetaTags.end = validMetaTags.begin;
    FreeScratchOutfits(outfits.begin, outfits.end);
    outfits.end = outfits.begin;
    CallGameMethod<void, 0x0073ECC0>(&householdMembers, householdMembers.begin, householdMembers.end);
    householdMembers.end = householdMembers.begin;
    auto result = FreeScratchTraits(traits.begin, traits.end);
    traits.end = traits.begin;
    return result;
}
// 0x009B4D30. In particular, +46 and all opaque ranges remain untouched.
ScratchMetadata* ScratchMetadata::Initialize() {
    for (auto& p : initial) p=nullptr;
    for (unsigned i=0;i<10;++i) if (i!=2) flags2C[i]=0;
    InitMetadataString(expiration,0); InitMetadataString(sendToFriend,1);
    for (auto& b : flags58) b=0;
    flag69=flagB0=0; valueCC=0;
    InitMetadataString(lotName,2); valuesE0[0]=valuesE0[1]=0;
    InitMetadataString(householdName,3); value100=value114=0;
    InitMetadataString(actorName,4); values128[0]=values128[1]=0; flag130=0;
    friends.begin=friends.end=friends.capacity=nullptr; value144=0;
    CallGameMethod<void, 0x00500A40>(&key148, uint32_t(0));
    for (auto& v : values158) v=0;
    InitMetadataString(matCategory,5); InitMetadataString(patternType,6); InitMetadataString(name,7);
    InitMetadataString(description,8); InitMetadataString(bio,9); value1B8=0;
    traitIds.begin=traitIds.end=traitIds.capacity=nullptr;
    traits.begin=traits.end=traits.capacity=nullptr;
    outfits.begin=outfits.end=outfits.capacity=nullptr;
    householdMembers.begin=householdMembers.end=householdMembers.capacity=nullptr;
    validMetaTags.begin=validMetaTags.end=validMetaTags.capacity=nullptr;
    SetMetadataLabels(*this);
    Clear();
    return this;
}
// 0x009B4FA0. Repeated empty-range cleanups are intentional EA behavior.
uintptr_t ScratchMetadata::Destroy() {
    Clear();
    if (validMetaTags.begin) Free(validMetaTags.begin);
    CallGameMethod<void, 0x0073ECC0>(&householdMembers, householdMembers.begin, householdMembers.end);
    if (householdMembers.begin) Free(householdMembers.begin);
    FreeScratchOutfits(outfits.begin,outfits.end);
    if (outfits.begin) Free(outfits.begin);
    FreeScratchTraits(traits.begin,traits.end);
    if (traits.begin) Free(traits.begin);
    if (traitIds.begin) Free(traitIds.begin);
    FreeWide(bio); FreeWide(description); FreeWide(name); FreeWide(patternType); FreeWide(matCategory);
    if (friends.begin) Free(friends.begin);
    FreeWide(actorName); FreeWide(householdName); FreeWide(lotName); FreeWide(sendToFriend);
    return FreeWide(expiration);
}
// 0x009B5590. All live elements are removed before freeing the vectors themselves.
uintptr_t ScratchManifest::Destroy() {
    while (parts.begin!=parts.end) {
        auto p=--parts.end;
        FreeScratchDownloadParts(p,p+1);
    }
    for (auto v : {&texts120,&texts130,&texts140})
        while (v->begin!=v->end) { auto p=--v->end; FreeNarrow(*p); }
    metadata.Destroy();
    texts140.Destroy(); texts130.Destroy(); texts120.Destroy();
    for (unsigned i=8;i>0;--i) FreeNarrow(textA0[i-1]);
    objects.Destroy();
    FreeScratchPairs(pairs6C.begin,pairs6C.end); if (pairs6C.begin) Free(pairs6C.begin);
    FreeScratchPairs(pairs5C.begin,pairs5C.end); if (pairs5C.begin) Free(pairs5C.begin);
    FreeScratchDownloadParts(parts.begin,parts.end); if (parts.begin) Free(parts.begin);
    FreeNarrow(description);
    return FreeNarrow(name);
}
// 0x00988890. All four stack words are copied, not just the low byte Hex-Rays names a2.
void* ScratchPendingKeys::Append(uint32_t a,uint32_t b,uint32_t c,uint32_t d) {
    ScratchKey value{{a,b,c,d}};
    auto destination=pending.end;
    if (Address(destination)>=Address(pending.capacity))
        return CallGameMethod<void*, 0x00892790>(&pending,destination,&value);
    pending.end=reinterpret_cast<ScratchKey*>(Address(destination)+sizeof(ScratchKey));
    if (destination) return CallGameMethod<ScratchKey*, 0x00500310>(destination,&value);
    return &pending;
}
// 0x00A3C120. EAX is uninitialized on the inactive path, so there is no return-value API.
ScratchKey* ScratchKeyState::Complete(uint32_t a,uint32_t b,uint32_t c,uint32_t d) {
    if (state==3) {
        ScratchKey value{{a,b,c,d}};
        auto result=CallGameMethod<ScratchKey*, 0x00500310>(&key,&value);
        state=4;
        return result;
    }
    return nullptr; // original inactive path has indeterminate EAX, not a return-value contract
}
// 0x009C79B0.
ScratchDownloadItem* ScratchDownloadItem::Initialize() {
#if defined(_M_IX86)
    vtable=reinterpret_cast<void*>(uintptr_t(0x0104FFE4));
#else
    vtable=nullptr;
#endif
    CallGameMethod<void, 0x00500A40>(&key,uint32_t(0));
    value18=nullptr; for (auto& v:values1C) v=0;
    path.capacity=dependency238; path.end=path.begin=inlinePath; inlinePath[0]=0;
    CallGameMethod<void, 0x004EC290>(dependency238);
    metadata.Initialize(); value858=value85C=0; flag860=0; value864=0;
    return this;
}
// 0x009C85D0. Growth returns the temporary metadata destructor's EAX, not the new item.
uintptr_t ScratchDownloadItems::Append() {
    auto destination=end;
    if (Address(destination)<Address(capacity)) {
        end=reinterpret_cast<ScratchDownloadItem*>(Address(destination)+sizeof(ScratchDownloadItem));
        if (destination) return Address(destination->Initialize());
        return Address(end);
    }
    ScratchDownloadItem temporary;
    temporary.Initialize();
    CallGameMethod<void, 0x009C8490>(this,destination,&temporary);
    return temporary.metadata.Destroy();
}
// 0x009C8C80.
uintptr_t ScratchDownloadQueue::Append(void* input,uint8_t flag,uint32_t value) {
    items.Append();
    auto item=items.end-1;
    CallGameMethod<void, 0x009C69C0>(item,input);
    auto result=CallGameMethod<uintptr_t, 0x009C5890>(item);
    item->value85C=value; item->flag860=flag;
    return result;
}
// 0x0098B9A0. Lookup iterator is two machine words; only its node is read here.
uint8_t ScratchContentManager::Lookup(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint16_t* version) {
#if defined(_M_IX86)
    const void* lockName=reinterpret_cast<const void*>(uintptr_t(0x010468AC));
#else
    const void* lockName=nullptr;
#endif
    CallGameMethod<void, 0x004E16F0>(mutex,lockName);
    if (version) *version=0;
    alignas(void*) uint8_t record[152];
    CallGameMethod<void, 0x00984B60>(record);
    ScratchKey key{{a,b,c,d}};
    struct { ScratchCacheNode* node; uintptr_t extra; } iterator;
    CallGameMethod<void, 0x00986600>(&cache,&iterator,&key);
    uint8_t result;
    if (cache.buckets[cache.count]!=iterator.node && iterator.node->value) {
        CallGameMethod<void, 0x00987C20>(record,iterator.node->value);
        result=1;
        if (version) {
            uint16_t copied;
            // The loaded cache record's generation word is exactly +120.
            copied=static_cast<uint16_t>(record[120] | uint16_t(record[121])<<8);
            *version=copied;
        }
    } else {
        void* provider=nullptr;
#if defined(_M_IX86)
        provider=*reinterpret_cast<void**>(uintptr_t(0x011E96A4));
#endif
        result=provider ? CallGameMethod<uint8_t, 0x009A37F0>(provider,&key) : 0;
        if (!result) result=CallGameFunction<uint8_t, 0x009A2720>(&key,static_cast<void*>(nullptr));
    }
    CallGameMethod<void, 0x00986350>(record);
    CallGameMethod<void, 0x004E17B0>(mutex);
    return result;
}
// 0x0098F630. Helpers are bridged at their observed ABI, not the unreliable esp-frame decompile.
int32_t ScratchContentManager::Import(void* resource,ScratchKey* key,ScratchImportContext* context) {
    if (state==1) return 4;
    if (!context) return 1;
    ScratchManifest manifest;
    CallGameMethod<void, 0x009B5330>(&manifest);
    auto finish=[&](int32_t result) { manifest.Destroy(); return result; };
    if (!OpenScratchResource(resource)) return finish(10);
    if (!CallGameMethod<uint8_t, 0x009BE7C0>(&manifest,resource,uint32_t(0))) return finish(4096);
    const ScratchKey manifestKey=manifest.key;
    CallGameMethod<ScratchKey*, 0x00500310>(key,&manifestKey);
    const ScratchKey inputKey=manifestKey;
    auto appendKey=[&] {
        ScratchKey value;
        CallGameMethod<void, 0x00983370>(&manifest,&value);
        context->Append(value.words[0],value.words[1],value.words[2],value.words[3]);
    };
    if (!IsScratchZeroVersion() && !context->flag9C && CallGameMethod<uint8_t, 0x0099BBE0>(this,resource)) {
        appendKey(); return finish(4096);
    }
    auto service=[]() -> void* {
#if defined(_M_IX86)
        auto manager=*reinterpret_cast<void**>(uintptr_t(0x011EADA0));
        auto stream=CallGameMethod<void*, 0x00580420>(manager);
        return CallGameMethod<void*, 0x00D1AA10>(stream);
#else
        AbortNotRewritten("Import game singleton"); return nullptr;
#endif
    };
    auto serviceState=service();
    if (CallGameMethod<uint8_t, 0x00A3C0B0>(serviceState) && manifest.words00[0]!=0x04F88964) {
        appendKey(); return finish(4096);
    }
    if (!CallGameFunction<uint8_t, 0x0099B5A0>(manifest.reserved98[1],uint32_t(0),uint32_t(0))) return finish(12);
    auto notify=[&] {
        auto target=static_cast<ScratchKeyState*>(service());
        target->Complete(inputKey.words[0],inputKey.words[1],inputKey.words[2],inputKey.words[3]);
    };
    uint16_t version=0;
    if (Lookup(inputKey.words[0],inputKey.words[1],inputKey.words[2],inputKey.words[3],&version)) {
        if (version>=manifest.version) {
            if (manifest.words00[0]==0x04F88964) {
                auto cas=CallGameFunction<void*, 0x004AFD20>();
                if (cas && !CallGameVirtual<uint8_t,14>(cas,resource))
                    CallGameVirtual<void,13>(cas,uint32_t(1),resource,uint32_t(0));
                notify();
            }
            return finish(4098);
        }
        CallGameMethod<void, 0x00988850>(context,inputKey.words[0],inputKey.words[1],inputKey.words[2],inputKey.words[3]);
    }
    int32_t status=CallGameMethod<int32_t, 0x009B9210>(&manifest);
    const uint32_t kind=manifest.words00[0];
    const uint32_t auxiliary=manifest.words00[1];
    const uintptr_t metadata=CallGameMethod<uintptr_t, 0x009B3790>(&manifest.metadata);
    if (kind==0x0B3A415E) return finish(CallGameFunction<int32_t, 0x009AC0A0>(this,kind,resource,&manifest,context));
    if (kind==0x04F88964) {
        if (special && CallGameMethod<uint8_t, 0x009CC390>(special,this,resource,&manifest,context)) { notify(); return finish(status); }
        return finish(3);
    }
    if (kind==0x06BC1F92) {
        auto found=CallGameFunction<ScratchKey*, 0x00984370>(context->resourceBegin,context->resourceEnd,&inputKey);
        return finish(CallGameFunction<int32_t, 0x009ACBD0>(this,resource,&manifest,context,uint8_t(found!=context->resourceEnd)));
    }
    struct Remap { uintptr_t words[8]; };
    auto makeMap=[&](Remap& map) {
#if defined(_M_IX86)
        const char* label=reinterpret_cast<const char*>(uintptr_t(0x01047E48));
#else
        const char* label="Scratch/DownloadContent/ContentManager/KeyRemapHashMap";
#endif
        CallGameMethod<void, 0x00AA3730>(&map,&label);
        CallGameMethod<void, 0x009B58B0>(&manifest,&map);
    };
    if (kind==0x436FEE4C) {
        auto result=CallGameFunction<int32_t, 0x009AC0A0>(this,kind,resource,&manifest,context);
        if (result>=4096) {
            Remap map; makeMap(map);
            auto ok=CallGameMethod<uint8_t, 0x0098DF80>(this,resource,&map,&manifest,context);
            CallGameMethod<void, 0x007D6630>(&map);
            if (!ok) return finish(0);
        }
        return finish(result);
    }
    // Preserve EA's odd test: null root is NOT rejected; only root == -8 is.
    auto handlerRef=reinterpret_cast<void**>(Address(handlers)+2*sizeof(void*));
    if (!handlerRef) return finish(2);
    Remap map; makeMap(map);
    struct DatabaseVector { void* begin; void* end; void* capacity; const char* label; };
    struct DatabaseAdapter { void* vtable; DatabaseVector values; } adapter{nullptr,{nullptr,nullptr,nullptr,nullptr}};
    TS3_X86_OFFSET(DatabaseAdapter, values, 4);
    auto& database=adapter.values;
#if defined(_M_IX86)
    database.label=reinterpret_cast<const char*>(uintptr_t(0x01047E28));
    adapter.vtable=reinterpret_cast<void*>(uintptr_t(0x010468F0));
#endif
    uint8_t success=0;
    if (CallGameVirtual<uint32_t,12>(resource,&database,&adapter)) {
        success=CallGameMethod<uint8_t, 0x0099F3F0>(this,&database,resource,*handlerRef,&map,
            inputKey.words[0],inputKey.words[1],inputKey.words[2],inputKey.words[3]);
        if (!success) status=8;
        else if (!CallGameMethod<uint8_t, 0x0098B5E0>(this,handlerRef,&manifest,kind,auxiliary,metadata)) { success=0; status=0; }
    } else status=0;
#if defined(_M_IX86)
    adapter.vtable=reinterpret_cast<void*>(uintptr_t(0x00FB2CBC));
#endif
    if (database.begin) Free(database.begin);
    CallGameMethod<void, 0x007D6630>(&map);
    if (!success) return finish(status);
    for (auto entry=manifest.objects.begin; entry!=manifest.objects.end; ++entry) {
        auto handler=CallGameMethod<void*, 0x00987E40>(this,entry->key);
        if (handler) {
            ScratchKey value=manifestKey;
            auto result=CallGameVirtual<int32_t,0>(handler,this,&value,entry->object);
            if (result<4096) return finish(result);
            if (result==4097) status=4097;
        }
    }
    // The first four failures are deliberately not returned: EA collapses them to 8.
    if (status==4096
        && CallGameMethod<int32_t, 0x0098D6A0>(this,key,resource,context)==4096
        && CallGameMethod<int32_t, 0x00985120>(this,key,resource,context)==4096
        && CallGameMethod<int32_t, 0x0098E7B0>(this,key,resource,context)==4096
        && CallGameMethod<int32_t, 0x0098D4F0>(this,key,resource,context)==4096) {
        status=CallGameMethod<int32_t, 0x0098D5A0>(this,key,resource,context);
        if (status==4096) {
            appendKey();
            auto notifications=CallGameFunction<void*, 0x0059CD50>();
            CallGameMethod<void, 0x0059D1E0>(notifications,resource);
            return finish(status);
        }
    }
    return finish(8);
}
// 0x009C8460. This assigned address has assembly but no IDA function record.
ScratchDownloadItem* ScratchDownloadItem::Delete(uint32_t flags) {
#if defined(_M_IX86)
    vtable=reinterpret_cast<void*>(uintptr_t(0x0104FFE4));
#else
    vtable=nullptr;
#endif
    metadata.Destroy();
    if (flags & 1) Free(this);
    return this;
}
}
