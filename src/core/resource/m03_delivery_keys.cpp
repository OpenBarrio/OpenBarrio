#include "ts3/resource/m03_delivery_keys.h"
#include "ts3/eastl_game.h"
#include <cstring>
#include <stdexcept>
#include <wchar.h>
#include "ts3/resource/m03_manager_refs_contracts.h"
#include "ts3/misc/clu_4a6c30_x.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
#include "ts3/resourcesystem/clu_4a3640_ResourceSystem.h"
#include "ts3/resourcesystem/clu_49e9e0_ResourceSystem.h"
#include "ts3/eatextutil/clu_4a0dc0_EATextUtil.h"

namespace ts3::resource {
namespace {
bool KeyLess(const ResourceKey& a, const ResourceKey& b)
{
    if (a.instanceLow != b.instanceLow) return a.instanceLow < b.instanceLow;
    if (a.instanceHigh != b.instanceHigh) return a.instanceHigh < b.instanceHigh;
    if (a.group != b.group) return a.group < b.group;
    return a.type < b.type;
}
}

uint32_t DeliveryKeysContains(const ResourceKey* first, const ResourceKey* last,
                              const ResourceKey* key)
{
    // C8CAB0 is a standard lower_bound: mid=count/2; advance by mid+1 only
    // when *mid < key, else retain first and halve count. It never writes inputs.
    // Test equality before subtracting: null/null is an allowed empty range.
    if (first == last) return 0;
    auto count = last - first;
    while (count > 0) {
        const auto half = count / 2;
        const auto* middle = first + half;
        if (KeyLess(*middle, *key)) {
            first = middle + 1;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    // With the sorted precondition lower_bound has already established !(*first<key).
    return first != last && !KeyLess(*key, *first) ? 1u : 0u;
}

uint32_t DeliveryKeySpan::Contains(const ResourceKey* key) const
{
    return DeliveryKeysContains(first, last, key);
}
DeliveryStringRecord* DeliveryRecordsCopyBackward(const DeliveryStringRecord* first,
    const DeliveryStringRecord* last, DeliveryStringRecord* outEnd)
{
    while (last != first) {
        --last;
        --outEnd;
        if (last != outEnd)
            scratch::AssignWide408E20(&outEnd->text, nullptr, last->text.begin, last->text.end);
        outEnd->value = last->value;
    }
    return outEnd;
}
namespace {
DeliveryStorage storage{};
const char* AllocationFile()
{
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00F98980u);
#else
    return "c:\\BuildAgent\\slc1-build110\\CMBuild\\Core\\include\\EASTL/allocator.h";
#endif
}
void* AllocateBytes(size_t bytes, const char* name)
{
    if (storage.allocate) return storage.allocate(bytes, name, 0, 0, AllocationFile(), 0xE7);
    return NamedAllocator(name).allocate(bytes, 0);
}
void ReleaseBytes(void* block)
{
    if (storage.release) storage.release(block);
    else NamedAllocator().deallocate(block, 0);
}
char16_t* EmptyText()
{
#if defined(_M_IX86)
    return reinterpret_cast<char16_t*>(0x011C7E54u);
#else
    static char16_t empty = 0;
    return &empty;
#endif
}
// Necessary uncredited construction algorithm from 735A20/408EB0/408830.
// Unlike assignment it copies +C, allocates exactly length+1, and always writes NUL.
void ConstructRecord(DeliveryStringRecord* out, const DeliveryStringRecord* source)
{
    out->text = {nullptr, nullptr, nullptr, source->text.allocName};
    const auto* first = static_cast<const char16_t*>(source->text.begin);
    const auto length = (reinterpret_cast<uintptr_t>(source->text.end) -
                         reinterpret_cast<uintptr_t>(first)) / 2;
    auto* buffer = length ? static_cast<char16_t*>(AllocateBytes(2*(length+1),
        static_cast<const char*>(out->text.allocName))) : EmptyText();
    out->text.begin = buffer;
    out->text.end = buffer;
    out->text.cap = buffer + length + 1;
    std::memmove(buffer, first, 2*length);
    out->text.end = buffer + length;
    buffer[length] = 0;
    out->value = source->value;
}
DeliveryStringRecord* ConstructRange(const DeliveryStringRecord* first,
    const DeliveryStringRecord* last, DeliveryStringRecord* out)
{
    for (; first != last; ++first, ++out)
        if (out) ConstructRecord(out, first);
    return out;
}
void DestroyRange(DeliveryStringRecord* first, DeliveryStringRecord* last)
{
    // 409040 walks forward and does not clear any record fields.
    for (; reinterpret_cast<uintptr_t>(first) < reinterpret_cast<uintptr_t>(last); ++first) {
        const auto bytes = static_cast<intptr_t>((reinterpret_cast<uintptr_t>(first->text.cap) -
            reinterpret_cast<uintptr_t>(first->text.begin)) & ~uintptr_t(1));
        if (bytes > 2 && first->text.begin) ReleaseBytes(first->text.begin);
    }
}
}
DeliveryStorage SetDeliveryStorage(DeliveryStorage binding)
{
    const auto old = storage;
    storage = binding;
    return old;
}
namespace {
#if !defined(_M_IX86)
// Native implementation of the one established base-table slot (9D6C70).
// This is a real typed global/callable boundary, not an EA address token.
DeliveryKeySpan* __fastcall DeleteDeliveryBase(DeliveryKeySpan* self, void*, uint32_t flags)
{
    self->identity = DeliveryKeyBaseIdentity();
    if (flags & 1u) ReleaseBytes(self);
    return self;
}
const DeliveryKeyBaseTable deliveryBaseTable{DeleteDeliveryBase};
#endif
}
const DeliveryKeyBaseTable* DeliveryKeyBaseIdentity()
{
#if defined(_M_IX86)
    return reinterpret_cast<const DeliveryKeyBaseTable*>(0x00FB2CBCu);
#else
    return &deliveryBaseTable;
#endif
}
DeliveryKeySpan* DeliveryKeyScalarDelete(DeliveryKeySpan* self, uint32_t flags)
{
    if (self->first) ReleaseBytes(const_cast<ResourceKey*>(self->first));
    self->identity = DeliveryKeyBaseIdentity();
    if (flags & 1u) ReleaseBytes(self);
    // Address identity only: no member access after the external self release.
    return self;
}
uint32_t DeliveryRecordVector::Insert(DeliveryStringRecord* position,
                                     const DeliveryStringRecord* source)
{
    auto* const oldEnd = end;
    if (oldEnd != capacity) {
        if (reinterpret_cast<uintptr_t>(source) >= reinterpret_cast<uintptr_t>(position) &&
            reinterpret_cast<uintptr_t>(source) < reinterpret_cast<uintptr_t>(oldEnd))
            ++source;
        if (oldEnd) ConstructRecord(oldEnd, oldEnd-1);
        DeliveryRecordsCopyBackward(position, end-1, end);
        if (source != position)
            scratch::AssignWide408E20(&position->text, nullptr, source->text.begin, source->text.end);
        const uint32_t result = source->value;
        position->value = result;
        end += 1;
        return result;
    }
    // Native record count, not a truncated pointer or a hard-coded x86 stride.
    const auto count = (reinterpret_cast<uintptr_t>(oldEnd) -
                        reinterpret_cast<uintptr_t>(begin)) / sizeof(DeliveryStringRecord);
    const uint32_t newCount = count ? 2*static_cast<uint32_t>(count) : 1;
    auto* const buffer = newCount ? static_cast<DeliveryStringRecord*>(
        AllocateBytes(sizeof(DeliveryStringRecord)*newCount, allocationName)) : nullptr;
    auto* cursor = ConstructRange(begin, position, buffer);
    if (cursor) ConstructRecord(cursor, source);
    cursor = ConstructRange(position, oldEnd, cursor+1);
    DestroyRange(begin, oldEnd);
    if (begin) ReleaseBytes(begin);
    // EA publishes end first, then begin, then capacity, after all old destruction.
    end = cursor;
    begin = buffer;
    capacity = buffer + newCount;
    return newCount;
}
} // namespace ts3::resource

namespace ts3::resource {
namespace {
DeliveryAdapterLeaves adapterLeaves{};
// Actual USED cleanup-vector fields; native pointers deliberately widen.
TS3_X86_OFFSET(misc::ResourceKeyVector, first, 0);
TS3_X86_OFFSET(misc::ResourceKeyVector, last, 4);
TS3_X86_OFFSET(misc::ResourceKeyVector, capacity, 8);
DeliveryKeyLookup* KeyManager()
{
    if (adapterLeaves.manager) return adapterLeaves.manager();
#if defined(_M_IX86)
    return CallGameFunction<DeliveryKeyLookup*, 0x004AFD20>();
#else
    throw std::logic_error("M03 delivery: missing key-manager leaf");
#endif
}
uint32_t HasKey(DeliveryKeyLookup* self, const ResourceKey* key)
{
    if (adapterLeaves.hasKey) return adapterLeaves.hasKey(self, key);
#if defined(_M_IX86)
    return CallGameVirtual<uint32_t, 17>(self, key);
#else
    throw std::logic_error("M03 delivery: missing key lookup leaf");
#endif
}
// Exact bounded constructor 7D80E0 and its small 7D7580 transformation.
// Evidence: 7D80E0..7D819D, 7D7580..7D760A, 54DD80..54DDAE.
// These are real algorithms, NOT an injectable whole-key conversion provider.
ResourceKey ConstructDeliveryKey(const ResourceKey& input)
{
    if (!(input.group & 0xf0000000u)) {
        auto* manager = KeyManager();
        if (manager && !HasKey(manager, &input)) {
            ResourceKey candidate = input;
            candidate.group ^= 0x08000000u;
            if (!input.instanceHigh && !(input.instanceLow & 0xf8000000u) &&
                (input.type == 0x736884f1u || input.type == 0x94c5d14au ||
                 input.type == 0x01d10f34u || input.type == 0x01661233u ||
                 input.type == 0x00ae6c67u))
                candidate.instanceLow ^= 0x08000000u;
            // 54DD80 returns AL=1 for different keys. Here group always differs.
            if (std::memcmp(&candidate, &input, sizeof(input)) != 0 &&
                HasKey(manager, &candidate)) return candidate;
        }
    }
    return input;
}
}
DeliveryAdapterLeaves SetDeliveryAdapterLeaves(DeliveryAdapterLeaves leaves)
{
    const auto old = adapterLeaves;
    adapterLeaves = leaves;
    return old;
}
uint32_t DeliveryRequest(DeliveryResourceService* self, const ResourceKey* input,
    void** out, uint32_t flags, void* database, void* factory, const ResourceKey* alias)
{
    const ResourceKey key = ConstructDeliveryKey(*input);
    if (adapterLeaves.request)
        return adapterLeaves.request(self, &key, out, flags, database, factory, alias);
#if defined(_M_IX86)
    return m03::manager::refs::DelegateRecord(reinterpret_cast<m03::ResourceManager*>(self),
        &key, out, flags, database, reinterpret_cast<m03::manager::RawCallback*>(factory), alias);
#else
    throw std::logic_error("M03 delivery: missing request leaf");
#endif
}
uint32_t DeliveryOperate(DeliveryResourceService* self, misc::ResourceKeyVector* vector,
    const void* filter, uint32_t flags)
{
    uint32_t result;
    if (adapterLeaves.operation) result = adapterLeaves.operation(self, vector, filter, flags);
    else {
#if defined(_M_IX86)
        result = CallGameMethod<uint32_t, 0x004B1AE0>(self, vector, filter, flags);
#else
        throw std::logic_error("M03 delivery: missing operation leaf");
#endif
    }
    if (result != 0 && vector != nullptr) {
        if (adapterLeaves.cleanup) return adapterLeaves.cleanup(vector);
#if defined(_M_IX86)
        return static_cast<uint32_t>(misc::KeySortUnique(vector));
#else
        throw std::logic_error("M03 delivery: missing cleanup leaf");
#endif
    }
    return result;
}
} // namespace ts3::resource

namespace ts3::resource {
namespace { DeliveryDestroyNodes destroyDeliveryNodes = nullptr; }
DeliveryDestroyNodes SetDeliveryDestroyNodes(DeliveryDestroyNodes binding)
{
    const auto old = destroyDeliveryNodes;
    destroyDeliveryNodes = binding;
    return old;
}
void DeliveryCleanup(DeliveryCleanupStorage* self)
{
    // Both arguments are snapshots BEFORE the external call. That call can
    // reenter/mutate the receiver: the later count and pointer must be reloaded.
    const uint32_t count = self->bucketCount;
    void* const buckets = self->buckets;
    if (destroyDeliveryNodes) destroyDeliveryNodes(self, buckets, count);
    else {
#if defined(_M_IX86)
        CallGameMethod<void, 0x007F3790>(self, buckets, count);
#else
        throw std::logic_error("M03 delivery: missing node-destruction leaf");
#endif
    }
    const bool release = self->bucketCount > 1u;
    self->size = 0;
    if (release) ReleaseBytes(self->buckets);
}
} // namespace ts3::resource

namespace ts3::resource {
namespace { DeliveryShadowSlot2 deliveryShadowSlot2 = nullptr; }
DeliveryShadowSlot2 SetDeliveryShadowSlot2(DeliveryShadowSlot2 leaf)
{
    const auto old = deliveryShadowSlot2;
    deliveryShadowSlot2 = leaf;
    return old;
}
uint32_t DeliveryShadowDispatch(DeliveryShadowOwner* owner, const void* source)
{
    // The real body loads [[owner]+8] and keeps ECX (receiver) and the original
    // stack argument untouched, pushing the literal word zero after it. Only the
    // actual external slot-2 implementation is crossed; the right-to-left argument
    // order is preserved by the thiscall adapter.
    if (deliveryShadowSlot2) return deliveryShadowSlot2(owner, source, 0u);
#if defined(_M_IX86)
    return CallGameVirtual<uint32_t, 2>(owner, source, 0u);
#else
    throw std::logic_error("M03 delivery: missing shadow slot-2 leaf");
#endif
}
void* DeliveryShadowAnchor(const DeliveryShadowOwner* owner)
{
    // Pointer-sized cell returned raw: no null check and no width narrowing.
    return owner->anchor;
}
} // namespace ts3::resource

namespace ts3::resource {
namespace { DeliveryPathPrefix deliveryPathPrefix = nullptr; }
DeliveryPathPrefix SetDeliveryPathPrefix(DeliveryPathPrefix binding)
{
    const auto old = deliveryPathPrefix;
    deliveryPathPrefix = binding;
    return old;
}
void* DeliveryPathFirstValue(DeliveryPathOwner* owner, const char16_t* path)
{
    // The original pushes the literal mode word 5 at each helper call site.
    constexpr int kPathPrefixMode = 5;
    // Both ends are snapshotted before the first external call. The helper (or a
    // reentrant caller reached from it) may reassign the receiver's vector cells,
    // but EA keeps traversing this snapshot and only compares against it.
    const auto* first = owner->begin;
    const auto* const last = owner->end;
    void* selected = nullptr;
    while (first != last) {
        // The record's path pointer is reloaded on every iteration, and only AL of
        // the helper word is tested: high garbage in EAX must not match.
        const auto* prefix = reinterpret_cast<const char16_t*>(first->text.begin);
        uint32_t accepted;
        if (deliveryPathPrefix) accepted = deliveryPathPrefix(prefix, path, kPathPrefixMode);
        else {
#if defined(_M_IX86)
            accepted = ts3::resourcesystem::HasPathPrefix(prefix, path, kPathPrefixMode) ? 1u : 0u;
#else
            throw std::logic_error("M03 delivery: missing path-prefix leaf");
#endif
        }
        if ((accepted & 0xffu) != 0u) {
            // Loaded AFTER the helper: its side effects are observable here.
            selected = first->payload;
            break;
        }
        ++first;
    }
    return selected;
}
} // namespace ts3::resource

namespace ts3::resource {
namespace {
// The two genuine allocation tags the subject's body initializes: the temporary
// string uses the image's "ResourceSystem" tag and the local path record uses
// "ResourceSystem/PathDdfInfo". Native hosts use real static string identities; a
// raw image address is never dereferenced outside x86.
const char* PathTempAllocName()
{
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00FFE3A0u);
#else
    return "ResourceSystem";
#endif
}
const char* PathRecordAllocName()
{
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00FFE0F8u);
#else
    return "ResourceSystem/PathDdfInfo";
#endif
}
char16_t* EmptyTextCap()
{
    return reinterpret_cast<char16_t*>(reinterpret_cast<uintptr_t>(EmptyText()) + 2);
}
// The subject's exactly-once ownership test for the two locals: masked signed extent
// greater than two AND a non-null begin. Release goes through the unit's established
// byte-release binding (default canonical NamedAllocator / 004E3F70).
void ReleaseWideIfOwned(const scratch::WideString& text)
{
    const auto extent = static_cast<intptr_t>(
        (reinterpret_cast<uintptr_t>(text.cap) - reinterpret_cast<uintptr_t>(text.begin)) & ~uintptr_t(1));
    if (extent > 2 && text.begin != nullptr) ReleaseBytes(text.begin);
}
#if !defined(_M_IX86)
// 0x00408830's growth contract for a NUL-terminated source, native-executable:
// scan the terminator, allocate exactly 2*(length+1) bytes with the string's own
// +0xC tag, copy, NUL-terminate; an empty source keeps the static sentinel.
void ConstructWideFromTextNative(scratch::WideString* out, const char16_t* source)
{
    size_t length = 0;
    while (source[length] != 0) ++length;
    char16_t* buffer = length
        ? static_cast<char16_t*>(AllocateBytes(2 * (length + 1), static_cast<const char*>(out->allocName)))
        : EmptyText();
    out->begin = buffer;
    out->end = buffer;
    out->cap = buffer + length + 1;
    std::memmove(buffer, source, 2 * length);
    out->end = buffer + length;
    buffer[length] = 0;
}
// 0x00408B90's append contract, native-executable: keep the current characters,
// append [first,last), writing the terminator at the new end. The in-capacity path
// shifts the appended tail and writes through the existing terminator; the growth
// path allocates 2*(count+1) with the destination's own +0xC tag, where count is
// max(needed, oldCapacityMinusOne <= 8 ? 8 : 2*oldCapacityMinusOne), copies both
// parts, releases the old owned buffer under the masked-extent test and then
// publishes begin/end/cap.
void AppendWideRangeNative(scratch::WideString* out, const char16_t* first, const char16_t* last)
{
    if (first == last) return;
    const intptr_t appended =
        (reinterpret_cast<intptr_t>(last) - reinterpret_cast<intptr_t>(first)) >> 1;
    const intptr_t kept =
        (reinterpret_cast<intptr_t>(out->end) - reinterpret_cast<intptr_t>(out->begin)) >> 1;
    const intptr_t capacityMinusOne =
        ((reinterpret_cast<intptr_t>(out->cap) - reinterpret_cast<intptr_t>(out->begin)) >> 1) - 1;
    const intptr_t needed = kept + appended;
    if (needed <= capacityMinusOne) {
        std::memmove(reinterpret_cast<uint8_t*>(out->end) + 2, first + 1,
                     static_cast<size_t>(2 * (appended - 1)));
        *reinterpret_cast<char16_t*>(reinterpret_cast<uintptr_t>(out->end) + 2 * appended) = 0;
        *static_cast<char16_t*>(out->end) = first[0];
        out->end = reinterpret_cast<uint8_t*>(out->end) + 2 * appended;
        return;
    }
    const intptr_t growth = capacityMinusOne <= 8 ? 8 : 2 * capacityMinusOne;
    const intptr_t count = growth < needed ? needed : growth;
    auto* buffer = static_cast<char16_t*>(
        AllocateBytes(2 * (count + 1), static_cast<const char*>(out->allocName)));
    std::memmove(buffer, out->begin, static_cast<size_t>(2 * kept));
    char16_t* newEnd = buffer + kept;
    std::memmove(newEnd, first, static_cast<size_t>(2 * appended));
    newEnd[appended] = 0;
    ReleaseWideIfOwned(*out);
    out->begin = buffer;
    out->end = newEnd + appended;
    out->cap = buffer + count + 1;
}
// 0x00408E20's assign-range contract, native-executable: the length compare is
// unsigned; a longer source memmoves the kept prefix and appends the remainder
// through the 408B90 contract, a shorter one memmoves, moves the terminator and
// shrinks `end` by the truncated character count. Returns nothing; the caller owns
// the destination.
void AssignWideRangeNative(scratch::WideString* out, const char16_t* first, const char16_t* last)
{
    const intptr_t srcChars =
        (reinterpret_cast<intptr_t>(last) - reinterpret_cast<intptr_t>(first)) >> 1;
    const intptr_t dstChars =
        (reinterpret_cast<intptr_t>(out->end) - reinterpret_cast<intptr_t>(out->begin)) >> 1;
    if (static_cast<uint32_t>(srcChars) > static_cast<uint32_t>(dstChars)) {
        std::memmove(out->begin, first, static_cast<size_t>(2 * dstChars));
        const intptr_t kept =
            (reinterpret_cast<intptr_t>(out->end) - reinterpret_cast<intptr_t>(out->begin)) >> 1;
        AppendWideRangeNative(out, first + kept, last);
        return;
    }
    const intptr_t bytes = 2 * srcChars;
    std::memmove(out->begin, first, static_cast<size_t>(bytes));
    uint8_t* newEnd = static_cast<uint8_t*>(out->begin) + bytes;
    uint8_t* oldEnd = static_cast<uint8_t*>(out->end);
    if (newEnd != oldEnd) {
        std::memmove(newEnd, oldEnd, 2);
        out->end = oldEnd - 2 * ((oldEnd - newEnd) >> 1);
    }
}
#endif
// Genuine host implementations of the three external seams. x86 executes the real
// image code / the verified canonical rewrite; x64 uses the native-executable
// equivalents above, so an unresolved game address is never crossed silently.
void HostConstructText(scratch::WideString* out, const char16_t* source)
{
#if defined(_M_IX86)
    CallGameMethod<void, 0x0040B700>(out, source);
#else
    ConstructWideFromTextNative(out, source);
#endif
}
void HostAssignRange(scratch::WideString* out, const char16_t* first, const char16_t* last)
{
#if defined(_M_IX86)
    (void)scratch::AssignWide408E20(out, nullptr, first, last);
#else
    AssignWideRangeNative(out, first, last);
#endif
}
void HostConstructRecordText(DeliveryPathRecord* out, const DeliveryPathRecord* source)
{
#if defined(_M_IX86)
    CallGameMethod<void, 0x00408EB0>(out, source);
#else
    ConstructRecord(reinterpret_cast<DeliveryStringRecord*>(out),
                    reinterpret_cast<const DeliveryStringRecord*>(source));
#endif
}
DeliveryPathAppendLeaves appendLeaves{};
}
DeliveryPathAppendLeaves DefaultDeliveryPathAppendLeaves()
{
    return {&HostConstructText, &HostAssignRange, &HostConstructRecordText};
}
DeliveryPathAppendLeaves SetDeliveryPathAppendLeaves(DeliveryPathAppendLeaves leaves)
{
    const auto old = appendLeaves;
    appendLeaves = leaves;
    return old;
}
uint32_t DeliveryPathAppendRecord(DeliveryPathOwner* owner, const char16_t* path)
{
    const auto leaves = appendLeaves;
    // 1. Temporary string from the argument (0x0040B700's contract). The +0xC tag is
    //    in place before the call, exactly like the original local initialization.
    scratch::WideString temporary{};
    temporary.allocName = const_cast<char*>(PathTempAllocName());
    if (leaves.constructText) leaves.constructText(&temporary, path);
    else HostConstructText(&temporary, path);
    // 2. Local path record: the empty sentinel words, the element tag and payload 0.
    DeliveryPathRecord local{};
    local.text.begin = EmptyText();
    local.text.end = EmptyText();
    local.text.cap = EmptyTextCap();
    local.text.allocName = const_cast<char*>(PathRecordAllocName());
    local.payload = nullptr;
    if (leaves.assignRange)
        leaves.assignRange(&local.text, static_cast<const char16_t*>(temporary.begin),
                           static_cast<const char16_t*>(temporary.end));
    else
        HostAssignRange(&local.text, static_cast<const char16_t*>(temporary.begin),
                        static_cast<const char16_t*>(temporary.end));
    // 3. First temporary lifetime ends before any vector work.
    ReleaseWideIfOwned(temporary);
    // 4. Append at the vector end: UNSIGNED position < capacity, end published BEFORE
    //    the constructor, null position skips construction, payload read AFTER it.
    DeliveryPathRecord* position = owner->end;
    if (reinterpret_cast<uintptr_t>(position) < reinterpret_cast<uintptr_t>(owner->capacity)) {
        owner->end = reinterpret_cast<DeliveryPathRecord*>(
            reinterpret_cast<uintptr_t>(position) + sizeof(DeliveryPathRecord));
        if (position != nullptr) {
            if (leaves.constructRecordText) leaves.constructRecordText(position, &local);
            else HostConstructRecordText(position, &local);
            position->payload = local.payload;
        }
    } else {
        // The real 0x00737800 on the same +0x724 begin/end/capacity/tag vector; the
        // record types share the begin/end/capacity/tag layout and the native record
        // stride (asserted below), so the credited production is reused unchanged.
        auto* vector = reinterpret_cast<DeliveryRecordVector*>(&owner->begin);
        const auto oldBegin = owner->begin;
        const auto oldCount = static_cast<size_t>(
            (reinterpret_cast<uintptr_t>(position) - reinterpret_cast<uintptr_t>(oldBegin)) /
            sizeof(DeliveryPathRecord));
#if !defined(_M_IX86)
        // 0x00737800 carries each record's identity across the growth as the original's
        // 4-byte cell word, but this host's identity is a full pointer (the header
        // requires widening it, never truncating it), so the source identities are kept
        // here and re-stored full-width after the reused insert. A zero count allocates
        // nothing; the snapshot is released again before the local record is freed.
        void** identities = nullptr;
        if (oldCount != 0)
            identities = static_cast<void**>(AllocateBytes(sizeof(void*) * oldCount,
                const_cast<char*>(owner->allocationName)));
        for (size_t i = 0; i < oldCount; ++i) identities[i] = oldBegin[i].payload;
#endif
        vector->Insert(reinterpret_cast<DeliveryStringRecord*>(position),
                       reinterpret_cast<const DeliveryStringRecord*>(&local));
#if !defined(_M_IX86)
        for (size_t i = 0; i < oldCount; ++i) owner->begin[i].payload = identities[i];
        if (identities != nullptr) ReleaseBytes(identities);
        // The appended record's identity is this body's own contract: the reused
        // insert only stored the low cell word of the local pointer.
        (owner->end - 1)->payload = local.payload;
#endif
    }
    // 5. Reload the CURRENT cells after the branch and save the index before cleanup.
    const auto begin = reinterpret_cast<uintptr_t>(owner->begin);
    const auto end = reinterpret_cast<uintptr_t>(owner->end);
    const auto count = static_cast<intptr_t>(end - begin) /
                       static_cast<intptr_t>(sizeof(DeliveryPathRecord));
    const uint32_t index = static_cast<uint32_t>(count - 1);
    // 6. Final local lifetime, then the SAVED index (never the release residue).
    ReleaseWideIfOwned(local.text);
    return index;
}
static_assert(sizeof(DeliveryStringRecord) == sizeof(DeliveryPathRecord),
    "the reused 737800 vector view needs one element stride for both record types");
static_assert(offsetof(DeliveryStringRecord, value) == offsetof(DeliveryPathRecord, payload),
    "the reused 737800 vector view needs the identity cell at the same offset");
} // namespace ts3::resource

namespace ts3::resource {
namespace {
DeliveryHashLeaves hashLeaves{};
// Mode-0 plain FNV-1a over a NUL-terminated UTF-16 string: the original instruction sequence
// then the original instruction sequence per unit, exactly the 0x004FA640 mode-0 loop (multiply, advance,
// xor, reload). Only mode 0 is native-executable: modes 1..3 read the game's
// 0x114CB30/0x114CC30 folding tables, which do not exist in this build.
uint32_t HashModeZero(const char16_t* text, uint32_t seed)
{
    uint32_t hash = seed;
    for (uint32_t c = static_cast<uint32_t>(*text); c != 0;
         c = static_cast<uint32_t>(*++text))
        hash = hash * 0x01000193u ^ c;
    return hash;
}
}
DeliveryHashLeaves SetDeliveryHashLeaves(DeliveryHashLeaves leaves)
{
    const auto old = hashLeaves;
    hashLeaves = leaves;
    return old;
}
void* DeliveryHashPathLookup(DeliveryHashOwner* owner, const char16_t* path)
{
    constexpr int32_t kPathMode = 5;
    constexpr uint32_t kHashSeed = 0x811C9DC5u;
    constexpr uint32_t kTimeoutNone = 0xFFFFFFFFu;

#if !defined(_M_IX86)
    // A native host has no game image: the three operations without a genuinely
    // native-executable canonical implementation must be bound up front, so a
    // missing leaf cannot leave the receiver locked or half-mutated.
    if (!hashLeaves.resolvePath || !hashLeaves.comparePaths || !hashLeaves.recordPath)
        throw std::logic_error("M03 delivery: missing hash-lookup leaf");
#endif

    // 260 UTF-16 units, proven by 004ED2A0's own contract (004FE0F0 >= 259 is
    // rejected before any copy) and the sibling comparison helper's char16_t[260]
    // locals; the 0x20C stack reserve is 4 bytes of saved hash plus this 520-byte
    // buffer, not a wider string.
    char16_t text[260];

    // 1. Normalization. The returned pointer is deliberately discarded: EA hashes
    //    whatever the helper left in the buffer, failed resolve included, and never
    //    repairs it. Only the external operation crosses the seam.
    if (hashLeaves.resolvePath) (void)hashLeaves.resolvePath(text, path, nullptr, kPathMode);
    else {
#if defined(_M_IX86)
        (void)resourcesystem::ResolvePath(text, path, nullptr, kPathMode);
#else
        throw std::logic_error("M03 delivery: missing path-resolve leaf");
#endif
    }

    // 2. Literal mode check; only AL decides the lowercase call, exactly like the
    //    original's the original instruction sequence after the original instruction sequence.
    uint32_t modeWord;
    if (hashLeaves.isTextMode) modeWord = hashLeaves.isTextMode(kPathMode);
    else modeWord = eatextutil::IsTextMode004EC3C0(kPathMode) ? 1u : 0u;
    if ((modeWord & 0xffu) == 0u) _wcslwr(reinterpret_cast<wchar_t*>(text));

    // 3. Full 32-bit hash of the scratch-derived string, never of the original
    //    argument, with the literal seed and mode.
    uint32_t hash;
    if (hashLeaves.hashText) hash = hashLeaves.hashText(text, kHashSeed, 0);
    else {
#if defined(_M_IX86)
        hash = CallGameFunction<uint32_t, 0x004FA640>(text, kHashSeed, 0);
#else
        hash = HashModeZero(text, kHashSeed);
#endif
    }

    // 4. Lock, and only after the lock snapshot the two vector cells. A lock seam
    //    that rewrites them is still observed by the snapshot.
    thread::Mutex* const mutex = &owner->mutex;
    const uint32_t timeout = kTimeoutNone;
    if (hashLeaves.lockMutex) (void)hashLeaves.lockMutex(mutex, &timeout);
    else {
#if defined(_M_IX86)
        CallGameMethod<int32_t, 0x004E16F0>(mutex, &timeout);
#else
        mutex->Lock(timeout);
#endif
    }
    DeliveryHashRecord* first = owner->begin;
    DeliveryHashRecord* const last = owner->end;

    // 5. First-match loop over the SNAPSHOTTED range: the full 32-bit hash gates
    //    every virtual/compare call, and the comparison result is tested as the
    //    full 32-bit EAX, never AL only.
    void* selected = nullptr;
    while (first != last) {
        if (first->hash == hash) {
            void* const object = first->object;
            const char16_t* recordText;
            if (hashLeaves.recordPath) recordText = hashLeaves.recordPath(object);
            else {
#if defined(_M_IX86)
                recordText = CallGameVirtual<const char16_t*, 10>(object);
#else
                throw std::logic_error("M03 delivery: missing record-path leaf");
#endif
            }
            int32_t comparison;
            if (hashLeaves.comparePaths)
                comparison = hashLeaves.comparePaths(path, recordText, kPathMode);
            else {
#if defined(_M_IX86)
                comparison = resourcesystem::CompareResourcePaths(path, recordText, kPathMode);
#else
                throw std::logic_error("M03 delivery: missing path-compare leaf");
#endif
            }
            if (comparison == 0) {
                // Reloaded after the compare (callbacks may have replaced the cell)
                // and saved before the unlock; the unlock result is never returned.
                selected = first->object;
                break;
            }
        }
        ++first;
    }
    if (hashLeaves.unlockMutex) (void)hashLeaves.unlockMutex(mutex);
    else {
#if defined(_M_IX86)
        CallGameMethod<int32_t, 0x004E17B0>(mutex);
#else
        mutex->Unlock();
#endif
    }
    return selected;
}
} // namespace ts3::resource
