// cls_4a3a40_x.cpp - portable rewrite of the TS3W.exe functions grouped in this misc
// unit (29 functions / 2406 bytes). See the header for the layout of every object and
// the address of each function; this file keeps the original call order, the original
// calling conventions and the EA quirks that change the outputs.
//
// Functions that belong to other units are reached through the bridge in ts3/layout.h
// (CallGameFunction / CallGameMethod / CallGameVirtual); the only verified dependency
// used directly is 0x00732E90 (clu_727290_ResourceSystem TrimCacheTo).
#include "ts3/misc/cls_4a3a40_x.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/resourcesystem/clu_727290_ResourceSystem.h"

namespace ts3 {
namespace misc {

namespace {

// The game's vtable pointers and global slots this unit reads or writes.
constexpr uintptr_t kMapListVtable = 0x00FD9A6C;         // 0x0059AEB0
constexpr uintptr_t kMapListDerivedVtable = 0x00FDDb14;  // 0x0059CD60
constexpr uintptr_t kMapListDestroyVtable = 0x00FDDb4c;  // 0x0059D060
constexpr uintptr_t kBaseVtable = 0x00FB2CBC;            // 0x0059AEB0 / 0x00C64FE0
constexpr uintptr_t kKeyGlobalSlot = 0x011C78D0;         // 0x004A3A40
constexpr uintptr_t kMapListGlobalA = 0x011CE2D0;        // 0x0059AEB0
constexpr uintptr_t kMapListGlobalB = 0x011CE308;        // 0x0059D060

// Key-list tag strings and vtables used by 0x0059D1E0 and 0x00735C20.
constexpr uintptr_t kStrKeyListTag = 0x00FDDBA4;  // "ResourceLoad/NameGuidMap/KeyList"
constexpr uintptr_t kStrTempKeyListTag = 0x00FFE1DC;  // "Debug/KeyNameMap/TempKeyList"
constexpr uintptr_t kStrKeyNameMapTag = 0x00FFDB5C;   // "Debug/KeyNameMap"
constexpr uintptr_t kStrDatabaseListTag = 0x00FDDBC8;  // ".../DatabaseList"
constexpr uintptr_t kStrFrcListTag = 0x00FFE1C0;  // "ResourceSystem/FRCacheList"
constexpr uintptr_t kStrDatabaseKeyFilter = 0x00FFE360;  // "ResourceSystem/DatabaseKeyFilter"
constexpr uintptr_t kKeyFilterVtable = 0x00FD8248;
constexpr uintptr_t kDatabaseKeyFilterObj = 0x00FFE384;
constexpr uint32_t kKeyFilterId = 0xF3A38370u;
constexpr uint32_t kTempKeyFilterId = 0x0166038Cu;
constexpr uint32_t kRecordQueryId = 362355231u;  // 0x15991A1F

// The {begin, end, capacity, tag} view the game passes to collect a key list. Fields
// are pointers, never dwords, so the x64 build keeps them whole.
struct KeyVectorView {
    void* begin;         // +0x00
    void* end;           // +0x04
    void* capacity;      // +0x08
    const char* tag;     // +0x0C
};
TS3_X86_SIZE(KeyVectorView, 0x10);

// A {vtable, key} query filter (0x00FD8248 is a game filter vtable).
struct KeyFilter {
    void* vtable;  // +0x00
    uint32_t key;  // +0x04
};
TS3_X86_SIZE(KeyFilter, 0x8);

// 0x007368F0's three-field circular list head.
struct NodeListHead {
    void* next;        // +0x00
    void* prev;        // +0x04
    const char* tag;   // +0x08
};
TS3_X86_SIZE(NodeListHead, 0xC);

#if defined(_M_IX86)
void* GamePtr(uintptr_t value) { return reinterpret_cast<void*>(value); }
#endif

}  // namespace

// ---------------------------------------------------------------------------
// 0x004A3A40
// ---------------------------------------------------------------------------
uint32_t SwapKeyGlobal(uint32_t value)
{
#if defined(_M_IX86)
    uint32_t* const slot = reinterpret_cast<uint32_t*>(kKeyGlobalSlot);
    const uint32_t previous = *slot;
    *slot = value;
    return previous;
#else
    (void)value;
    AbortNotRewritten("ts3::misc::SwapKeyGlobal");
    return 0;
#endif
}

// ---------------------------------------------------------------------------
// 0x0059ADF0
// ---------------------------------------------------------------------------
uint8_t MaskScanKeyRange(const KeySpan* span, int32_t mode, uint32_t mask)
{
    const ResourceMapKey* current = span->first;
    const ResourceMapKey* const last = span->last;
    if (current == last) return 0;
    for (;;) {
        // EA tests `mode` first: a non-zero mode never compares the masked byte, so
        // every element misses.
        if (mode == 0 && (current->group & 0xFF000000u) == mask) return 1;
        current = reinterpret_cast<const ResourceMapKey*>(
            reinterpret_cast<const uint8_t*>(current) + sizeof(ResourceMapKey));
        if (current == last) return 0;
    }
}

// ---------------------------------------------------------------------------
// ResourceMapKey predicates (0x00996880, 0x009A7490, 0x009A74C0, 0x009A7C50,
// 0x009A7C70, 0x00AD4DF0, 0x00AD4ED0, 0x00AD96B0, 0x00AD9700)
// ---------------------------------------------------------------------------
bool ResourceMapKey::MatchesTypePair(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->type == instance && key->group == high;
}

bool ResourceMapKey::IsTypeExcluded3(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->instance != 0x05ED1226u && key->instance != 0xB4DD716Bu &&
           key->instance != 0x34E5247Cu;
}

bool ResourceMapKey::IsTypeExcluded8(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->instance != 0x05ED1226u && key->instance != 0x06B981EDu &&
           key->instance != 0x220557DAu && key->instance != 0xD063545Bu &&
           key->instance != 0x022B756Cu && key->instance != 0x35A33E29u &&
           key->instance != 0xB4DD716Bu && key->instance != 0x34E5247Cu;
}

bool ResourceMapKey::IsTypeNotExcluded1(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->instance != 0x0679B9B8u;
}

bool ResourceMapKey::IsTypeNotExcluded4(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->instance != 0x05ED1226u && key->instance != 0x0679B9B8u &&
           key->instance != 0xB4DD716Bu && key->instance != 0x34E5247Cu;
}

bool ResourceMapKey::MatchesTypePairEx2(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    if (key->type != instance || key->group != high) return false;
    return key->instance != 0x05ED1226u && key->instance != 0x34E5247Cu;
}

ResourceMapKey* ResourceMapKey::CopyPairWith(void* out_, const void* src_) const
{
    ResourceMapKey* out = static_cast<ResourceMapKey*>(out_);
    const ResourceMapKey* src = static_cast<const ResourceMapKey*>(src_);
    // The original copies src->instance/high first, then this->instance/high.
    out->instance = src->instance;
    out->high = src->high;
    out->type = instance;
    out->group = high;
    return out;
}

bool ResourceMapKey::MatchesTypePair6(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    if (key->type != instance || key->group != high) return false;
    const uint32_t value = key->high;
    if (value > 0x00B0C507u) {
        // EA subtracts 0x00BC1E4C and then 8; wrap-around is not possible because the
        // unsigned compare already routed the small values the other way.
        const uint32_t first = value - 0x00BC1E4Cu;
        if (first == 0) return true;
        return first - 8u == 0;
    }
    if (value == 0x00B0C507u || value == 0x001064CAu || value == 0x001DA7E9u ||
        value == 0x007E7555u)
        return true;
    return false;
}

bool ResourceMapKey::MatchesTypePairFire(const void* key_) const
{
    const ResourceMapKey* key = static_cast<const ResourceMapKey*>(key_);
    return key->type == instance && key->group == high && key->high == 0x001064CAu;
}

// ---------------------------------------------------------------------------
// ResourceMapList (0x0059AEB0, 0x0059B050, 0x0059CD60, 0x0059CFC0, 0x0059D060,
// 0x0059D1E0, 0x0059D340)
// ---------------------------------------------------------------------------
uint32_t ResourceMapList::VtableConstant()
{
    (void)this;
    return static_cast<uint32_t>(kMapListDerivedVtable);
}

int32_t ResourceMapList::DestructRefList()
{
#if defined(_M_IX86)
    vtable = GamePtr(kMapListVtable);
    CallGameFunction<void, 0x0044C390>();
    *reinterpret_cast<uint32_t*>(kMapListGlobalA) = 0;
    MapListNode* const sentinel = reinterpret_cast<MapListNode*>(&listNext);
    MapListNode* node = listNext;
    while (node != sentinel) {
        MapListNode* const current = node;
        node = node->next;
        CallGameFunction<uint32_t, 0x004E3F70>(current);
    }
    baseVtable = GamePtr(kBaseVtable);
    return CallGameMethod<int32_t, 0x00731CF0>(this);
#else
    AbortNotRewritten("ts3::misc::ResourceMapList::DestructRefList");
    return 0;
#endif
}

void* ResourceMapList::EraseKey(const ResourceMapKey* key)
{
#if defined(_M_IX86)
    MapListNode* const sentinel = reinterpret_cast<MapListNode*>(&listNext);
    MapListNode* node = listNext;
    if (node == sentinel) return const_cast<ResourceMapKey*>(key);
    for (;;) {
        // The original compares type, instance, high, group in that order.
        if (node->key.type == key->type && node->key.instance == key->instance &&
            node->key.high == key->high && node->key.group == key->group)
            break;
        node = node->next;
        if (node == sentinel) return const_cast<ResourceMapKey*>(key);
    }
    void* const object = node->object;
    if (object) CallGameVirtual<void, 1, void>(object);
    node->object = nullptr;
    MapListNode* const next = node->next;
    node->prev->next = node->next;
    next->prev = node->prev;
    return reinterpret_cast<void*>(
        static_cast<uintptr_t>(CallGameFunction<uint32_t, 0x004E3F70>(node)));
#else
    (void)key;
    AbortNotRewritten("ts3::misc::ResourceMapList::EraseKey");
    return nullptr;
#endif
}

int32_t ResourceMapList::ClearVector()
{
#if defined(_M_IX86)
    void* const vector = reinterpret_cast<uint8_t*>(this) + 0x10;
    const int32_t result = CallGameMethod<int32_t, 0x00C1C390>(vector, vecBegin, vecCursor);
    vecCursor = vecBegin;
    return result;
#else
    AbortNotRewritten("ts3::misc::ResourceMapList::ClearVector");
    return 0;
#endif
}

ResourceMapList* ResourceMapList::Destroy(int32_t flags)
{
#if defined(_M_IX86)
    vtable = GamePtr(kMapListDestroyVtable);
    *reinterpret_cast<uint32_t*>(kMapListGlobalB) = 0;
    void* const vector = reinterpret_cast<uint8_t*>(this) + 0x10;
    CallGameMethod<int32_t, 0x00C1C390>(vector, vecBegin, vecCursor);
    if (vecBegin) CallGameFunction<uint32_t, 0x004E3F70>(vecBegin);
    CallGameMethod<int32_t, 0x00731CF0>(this);
    if (flags & 1) CallGameFunction<uint32_t, 0x004E3F70>(this);
    return this;
#else
    (void)flags;
    AbortNotRewritten("ts3::misc::ResourceMapList::Destroy");
    return this;
#endif
}

int32_t ResourceMapList::AppendDatabase(void* database)
{
#if defined(_M_IX86)
    void* const manager = CallGameFunction<void*, 0x004AFD20>();
    if (!manager) return 0;

    KeyVectorView collected;
    collected.begin = nullptr;
    collected.end = nullptr;
    collected.capacity = nullptr;
    collected.tag = reinterpret_cast<const char*>(kStrKeyListTag);
    KeyFilter filter;
    filter.vtable = GamePtr(kKeyFilterVtable);
    filter.key = kKeyFilterId;

    // First argument is the output view (pushed last), second the filter.
    CallGameVirtual<void, 0x30 / 4, void>(database, &collected, &filter);

    if (collected.begin != collected.end) {
        uint8_t* current = static_cast<uint8_t*>(collected.begin);
        const uint8_t* const last = static_cast<const uint8_t*>(collected.end);
        for (;;) {
            ResourceMapKey key;
            std::memcpy(&key, current, sizeof key);
            void* iface = nullptr;
            // v10(manager, &key, &iface, 0, database, 0, 0)
            const uint8_t accepted = CallGameVirtual<uint8_t, 0x0C / 4, void>(
                manager, &key, &iface, 0, database, 0, 0);
            if (accepted) {
                void* record = nullptr;
                if (iface) {
                    record = CallGameVirtual<void*, 0x0C / 4, void>(iface, kRecordQueryId);
                    if (record) CallGameVirtual<void, 0, void>(record);
                }
                uint8_t* const cursor = static_cast<uint8_t*>(vecCursor);
                if (cursor >= static_cast<uint8_t*>(vecCapacity)) {
                    CallGameMethod<void, 0x0059D0C0>(
                        reinterpret_cast<uint8_t*>(this) + 0x10, cursor,
                        reinterpret_cast<uint8_t*>(record) + 40);
                } else {
                    vecCursor = cursor + 32;
                    if (cursor)
                        CallGameMethod<void, 0x0093C460>(
                            cursor, reinterpret_cast<uint8_t*>(record) + 40);
                }
                if (record) CallGameVirtual<void, 1, void>(record);
            }
            if (iface) CallGameVirtual<void, 1, void>(iface);
            current += sizeof(ResourceMapKey);
            if (current == last) break;
        }
    }
    // EA resets the filter's vtable to the base vtable before releasing the list (the
    // filter is a stack temporary, so this is only kept for fidelity).
    filter.vtable = GamePtr(kBaseVtable);
    if (!collected.begin) return 0;
    return static_cast<int32_t>(
        static_cast<uintptr_t>(CallGameFunction<uint32_t, 0x004E3F70>(collected.begin)));
#else
    (void)database;
    AbortNotRewritten("ts3::misc::ResourceMapList::AppendDatabase");
    return 0;
#endif
}

uint8_t ResourceMapList::LoadDatabaseKeys()
{
#if defined(_M_IX86)
    void* const manager = CallGameFunction<void*, 0x004AFD20>();
    if (!manager) return 0;

    if (!CallGameVirtual<int32_t, 0x2C / 4, void>(manager, kKeyFilterId, -1))
        CallGameFunction<void, 0x00935430>();

    void* const listOwner = CallGameFunction<void*, 0x00931020>();
    struct DatabaseListHead {
        void* next;       // +0x00
        void* prev;       // +0x04
        void* owner;      // +0x08
        void* spare;      // +0x0C
        const char* tag;  // +0x10
    };
    DatabaseListHead head;
    head.next = &head;
    head.prev = &head;
    head.owner = listOwner;
    head.spare = nullptr;
    head.tag = reinterpret_cast<const char*>(kStrDatabaseListTag);
    CallGameVirtual<void, 0x48 / 4, void>(manager, &head, 0);

    for (void* node = head.next; node != &head;) {
        void* const current = node;
        node = *reinterpret_cast<void**>(node);
        AppendDatabase(*reinterpret_cast<void**>(static_cast<uint8_t*>(current) + 8));
    }
    for (void* node = head.next; node != &head;) {
        void* const current = node;
        node = *reinterpret_cast<void**>(node);
        // The original re-reads the list head's owner field on every iteration.
        CallGameVirtual<void, 0x0C / 4, void>(head.owner, current, 12);
    }
    return 1;
#else
    AbortNotRewritten("ts3::misc::ResourceMapList::LoadDatabaseKeys");
    return 0;
#endif
}

// ---------------------------------------------------------------------------
// 0x005A3830
// ---------------------------------------------------------------------------
uint8_t KeyFieldOwner::SerializeFields(void* stream)
{
#if defined(_M_IX86)
    // The game's field reader: 0x18 bytes, stream pointer at +8 (set by 0x007D91B0),
    // flag dword at +0x10 and the accumulated ok byte at +0x14.
    alignas(8) uint8_t reader[0x18] = {};
    CallGameMethod<void, 0x007D91B0>(reader, stream, 1);

    uint8_t ok = CallGameFunction<uint8_t, 0x004DD700>(
        reader + 8, &f00, *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);
    ok = CallGameFunction<uint8_t, 0x004DD700>(reader + 8, &f04,
                                               *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);
    ok = CallGameFunction<uint8_t, 0x004DD700>(reader + 8, &f08,
                                               *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);
    ok = CallGameFunction<uint8_t, 0x004DD700>(reader + 8, &f0C,
                                               *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);
    // 0x007D7530 reads/fixes four dwords (offset +8, +0xC, +0 and +4).
    ok = CallGameFunction<uint8_t, 0x007D7530>(reader + 8, &f10,
                                               *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);
    ok = CallGameFunction<uint8_t, 0x007D7530>(reader + 8, &f20,
                                               *reinterpret_cast<uint32_t*>(reader + 0x10));
    CallGameMethod<uint8_t, 0x007D77C0>(reader, ok);

    const uint8_t result = reader[0x14];  // read before the teardown updates the flag
    CallGameMethod<int32_t, 0x007D7760>(reader);
    return result;
#else
    (void)stream;
    AbortNotRewritten("ts3::misc::KeyFieldOwner::SerializeFields");
    return 0;
#endif
}

// ---------------------------------------------------------------------------
// DatabaseShadowWatchers (0x00733E40, 0x00733E70, 0x00733EE0, 0x00733EF0,
// 0x00733F00, 0x00735C20, 0x00736600, 0x007368F0)
// ---------------------------------------------------------------------------
int32_t DatabaseShadowWatchers::NotifyShadow(void* target, int32_t value)
{
#if defined(_M_IX86)
    void* const notifier = CallGameMethod<void*, 0x00727280>(this);
    if (notifier) CallGameVirtual<void, 0x24 / 4, void>(notifier, target, value);
    return CallGameVirtual<int32_t, 0x40 / 4, void>(target, value);
#else
    (void)target;
    (void)value;
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::NotifyShadow");
    return 0;
#endif
}

int32_t DatabaseShadowWatchers::TrimCacheRefs()
{
#if defined(_M_IX86)
    int32_t result = 0;  // EA leaves the caller's EAX when both pointers are null
    if (cacheRefA) {
        result = static_cast<int32_t>(resourcesystem::TrimCacheTo(
            static_cast<resourcesystem::ResourceCache*>(cacheRefA),
            *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(cacheRefA) + 0x8C), nullptr,
            1));
    }
    if (cacheRefB) {
        return static_cast<int32_t>(resourcesystem::TrimCacheTo(
            static_cast<resourcesystem::ResourceCache*>(cacheRefB),
            *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(cacheRefB) + 0x8C), nullptr,
            1));
    }
    return result;
#else
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::TrimCacheRefs");
    return 0;
#endif
}

uint8_t DatabaseShadowWatchers::SetFlag(int32_t value)
{
    flag = static_cast<uint8_t>(value);
    return static_cast<uint8_t>(value);
}

int32_t DatabaseShadowWatchers::InitKeyMapMember()
{
#if defined(_M_IX86)
    return CallGameMethod<int32_t, 0x00731CA0>(reinterpret_cast<uint8_t*>(this) + 0x768);
#else
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::InitKeyMapMember");
    return 0;
#endif
}

int32_t DatabaseShadowWatchers::DestroyKeyMapMember()
{
#if defined(_M_IX86)
    return CallGameMethod<int32_t, 0x00730270>(reinterpret_cast<uint8_t*>(this) + 0x768);
#else
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::DestroyKeyMapMember");
    return 0;
#endif
}

void* DatabaseShadowWatchers::BuildTempKeyList()
{
#if defined(_M_IX86)
    void* const result = static_cast<void*>(&tempMapNext);
    if (!mapBuilt) return result;
    if (tempMapNext != result) return result;

    void* const manager = CallGameFunction<void*, 0x004AFD20>();
    KeyVectorView collected;
    collected.begin = nullptr;
    collected.end = nullptr;
    collected.capacity = nullptr;
    collected.tag = reinterpret_cast<const char*>(kStrTempKeyListTag);
    KeyFilter filter;
    filter.vtable = GamePtr(kKeyFilterVtable);
    filter.key = kTempKeyFilterId;
    // v7(manager, &collected, &filter, 0)
    CallGameVirtual<void, 0x20 / 4, void>(manager, &collected, &filter, 0);
    CallGameFunction<int32_t, 0x004AFCD0>(&collected);

    uint8_t* current = static_cast<uint8_t*>(collected.begin);
    const uint8_t* const last = static_cast<const uint8_t*>(collected.end);
    while (current != last) {
        void* const node = CallGameVirtual<void*, 0x44 / 4, void>(manager, current);
        if (node) {
            void* record = nullptr;
            const uint8_t accepted = CallGameVirtual<uint8_t, 0x34 / 4, void>(
                node, current, &record, 1, 6, 1, 0);
            if (accepted) {
                void* const memory = CallGameFunction<void*, 0x004E3F90>(
                    72, reinterpret_cast<const char*>(kStrKeyNameMapTag), 0, 0, 0, 0);
                void* const allocated =
                    memory ? CallGameMethod<void*, 0x0072F690>(memory) : nullptr;
                void* const value = CallGameVirtual<void*, 0x18 / 4, void>(record);
                CallGameMethod<void, 0x0072F4A0>(allocated, value);
                CallGameMethod<void, 0x008AE460>(result, &allocated);
                CallGameVirtual<void, 0x20 / 4, void>(record);
            }
            // The original only calls the record's slot 2 when the slot 0x34 call
            // actually produced a record (accepted entries always do).
            if (record) CallGameVirtual<void, 0x08 / 4, void>(record);
        }
        current += sizeof(ResourceMapKey);
    }
    if (collected.begin) CallGameFunction<uint32_t, 0x004E3F70>(collected.begin);
    return result;
#else
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::BuildTempKeyList");
    return nullptr;
#endif
}

uint8_t DatabaseShadowWatchers::GetFlag() const
{
    return flag;
}

int32_t DatabaseShadowWatchers::FilterDatabases(void* target)
{
#if defined(_M_IX86)
    KeyVectorView collected;
    collected.begin = nullptr;
    collected.end = nullptr;
    collected.capacity = nullptr;
    collected.tag = reinterpret_cast<const char*>(kStrDatabaseKeyFilter);
    void* const filter = GamePtr(kDatabaseKeyFilterObj);
    CallGameVirtual<void, 0x30 / 4, void>(target, &collected, nullptr);
    CallGameFunction<void, 0x004AFC60>(collected.begin, collected.end);

    NodeListHead list;
    list.next = &list;
    list.prev = &list;
    list.tag = reinterpret_cast<const char*>(kStrFrcListTag);
    // EA reads the slot from [this+8] and passes this+8 as `this`: the field holds the
    // vtable pointer of an object embedded at +8 (not a separate object pointer).
    CallGameVirtual<void, 0x64 / 4, void>(reinterpret_cast<uint8_t*>(this) + 8, &list);

    for (void* node = list.next; node != &list;) {
        void* const current = node;
        node = *reinterpret_cast<void**>(node);
        void* const object = *reinterpret_cast<void**>(static_cast<uint8_t*>(current) + 8);
        CallGameVirtual<void, 0x28 / 4, void>(object, &filter);
    }
    for (void* node = list.next; node != &list;) {
        void* const current = node;
        node = *reinterpret_cast<void**>(node);
        CallGameFunction<uint32_t, 0x004E3F70>(current);
    }
    if (!collected.begin) return 0;
    return static_cast<int32_t>(
        static_cast<uintptr_t>(CallGameFunction<uint32_t, 0x004E3F70>(collected.begin)));
#else
    (void)target;
    AbortNotRewritten("ts3::misc::DatabaseShadowWatchers::FilterDatabases");
    return 0;
#endif
}

// ---------------------------------------------------------------------------
// IndexedVector (0x00C64FB0, 0x00C64FE0)
// ---------------------------------------------------------------------------
bool IndexedVector::ContainsDifferent(void* key)
{
#if defined(_M_IX86)
    // The original passes ECX = this+4 (the id-array owner) to 0x0072DAD0, which reads
    // the count back from [ecx+8]; the call is a thiscall with the 2 stack arguments.
    uint32_t local[2] = {0, 0};
    const uint32_t id = ids[count];
    const uint32_t value = *static_cast<uint32_t*>(
        CallGameMethod<void*, 0x0072DAD0>(reinterpret_cast<uint8_t*>(this) + 4, local, key));
    return value != id;
#else
    (void)key;
    AbortNotRewritten("ts3::misc::IndexedVector::ContainsDifferent");
    return false;
#endif
}

IndexedVector* IndexedVector::Destroy(int32_t flags)
{
#if defined(_M_IX86)
    void* const holder = reinterpret_cast<uint8_t*>(this) + 4;
    CallGameMethod<void, 0x006258F0>(holder, ids, count);
    pad10 = 0;
    if (count > 1) CallGameFunction<uint32_t, 0x004E3F70>(ids);
    vtable = GamePtr(kBaseVtable);
    if (flags & 1) CallGameFunction<uint32_t, 0x004E3F70>(this);
    return this;
#else
    (void)flags;
    AbortNotRewritten("ts3::misc::IndexedVector::Destroy");
    return this;
#endif
}

}  // namespace misc
}  // namespace ts3