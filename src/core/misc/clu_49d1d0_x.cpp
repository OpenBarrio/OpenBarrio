#include "ts3/misc/clu_49d1d0_x.h"
#include <initializer_list>

namespace ts3::misc {
// 0x004A3890
int EqualGuid(const void* first, const void* second)
{
    auto a = static_cast<const GuidWords*>(first); auto b = static_cast<const GuidWords*>(second);
    return a->words[0] == b->words[0] && a->words[2] == b->words[2] &&
           a->words[3] == b->words[3] && a->words[1] == b->words[1];
}
// 0x004A3CF0
bool __stdcall LessGuid(const void* first, const void* second)
{
    auto a = static_cast<const GuidWords*>(first); auto b = static_cast<const GuidWords*>(second);
    if (a->words[0] != b->words[0]) return a->words[0] < b->words[0];
    if (a->words[1] != b->words[1]) return a->words[1] < b->words[1];
    if (a->words[3] != b->words[3]) return a->words[3] < b->words[3];
    return a->words[2] < b->words[2];
}
// 0x004A11E0
NameNode* __stdcall FindName(NameNode* head, const char16_t* const* name, int)
{
    while (head) {
        const char16_t* a = head->name;
        const char16_t* b = *name;
        while (*b && *b == *a) { ++b; ++a; }
        if (*b == *a) return head;
        head = head->next;
    }
    return nullptr;
}
// 0x004A69F0
int32_t ValueOwner::GetValue() const { return value; }
// 0x00C4B800
int ResourceKey::Equals(const void* key) const
{
    auto other = static_cast<const ResourceKey*>(key);
    if (kind != other->kind) return 0;
    for (unsigned i : {0u, 2u, 3u, 1u, 4u, 6u, 7u, 5u})
        if (words[i] != other->words[i]) return 0;
    return EqualGuid(&guid, &other->guid);
}
// 0x00C4F670
ResourceIterator* ResourceMap::Find(ResourceIterator* out, const ResourceKey* key)
{
    // The original sign-extends the 16-bit kind before XOR, then uses unsigned division.
    const uint32_t hash = uint32_t(int32_t(int16_t(key->kind))) ^ key->words[0] ^ key->words[4];
    ResourceNode** bucket = buckets + hash % bucketCount;
    ResourceNode* node = *bucket;
    while (node && !key->Equals(&node->key)) node = node->next;
    if (!node) { bucket = buckets + bucketCount; node = *bucket; }
    *out = {node, bucket};
    return out;
}
// 0x00C50E80
uint32_t* ResourceMap::GetOrInsert(const ResourceKey* key)
{
    ResourceIterator it;
    Find(&it, key);
    if (it.node == buckets[bucketCount]) {
        struct Pair { ResourceKey key; uint32_t value; } pair{*key, 0};
        // EA clears only AL of the key pointer argument. The callee consumes the low
        // byte as a bool; retaining this shape matters for the original ABI.
        const uintptr_t hint = reinterpret_cast<uintptr_t>(key) & ~uintptr_t(255);
        CallGameMethod<void, 0x00C4F820>(this, &it, &pair, hint);
    }
    return &it.node->value;
}
// 0x00C517D0
uint32_t ResourceIds::Intern(const ResourceKey* key)
{
    ResourceIterator it;
    map.Find(&it, key);
    if (it.node != map.buckets[map.bucketCount]) return it.node->value;
    *map.GetOrInsert(key) = nextId;
    return nextId++;
}
}

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::misc {
// 0x004DAA70
bool ThreadState::Stop()
{
    if (!thread) return false;
    running = 0;
#if defined(_WIN32)
    SetEvent(event);
    WaitForSingleObject(thread, 0xFFFFFFFFu);
    CloseHandle(thread);
#else
    AbortNotRewritten("worker join on non-Windows");
#endif
    thread = nullptr;
    return true;
}
// 0x004DAB70
bool ThreadState::Close()
{
    if (!initialized) return false;
    initialized = 0;
    if (thread) Stop();
#if defined(_WIN32)
    if (event) { CloseHandle(event); event = nullptr; }
    if (extraEvent) { CloseHandle(extraEvent); extraEvent = nullptr; }
#else
    if (event || extraEvent) AbortNotRewritten("event close on non-Windows");
#endif
    return true;
}
// 0x004A4460
bool WorldWorker::Shutdown()
{
    if (active) {
        active = 0;
        worker.Stop(); worker.Close();
        CallGameMethod<void, 0x006A0850>(&list, list.count);
        list.next = &list.next; list.prev = &list.next;
        list.count = 0; list.flag = 0; list.extra = 0;
    }
    return true;
}
// 0x004A1280
int UiCleanup::Cleanup()
{
    while (links.prev != &links) {
        UiLink* node = links.prev;
        node->prev->next = &links;
        links.prev = node->prev;
        if (node->object) {
            void* object = node->object;
            node->object = nullptr;
            CallGameVirtual<void, 1>(static_cast<uint8_t*>(object) + sizeof(void*));
        }
    }
    if (storage && storage != inlineStorage) CallGameFunction<void, 0x004E3F70>(storage);
    return CallGameMethod<int, 0x004E87F0>(reinterpret_cast<uint8_t*>(this) + sizeof(void*));
}
// 0x004A4D40
void TreeOwner::DestroyTree(TreeNode* node)
{
    while (node) {
        DestroyTree(node->left);
        TreeNode* right = node->right;
        TreeItem* item = node->items.next;
        while (item != &node->items) {
            TreeItem* next = item->next;
            if (item->object) CallGameVirtual<void, 2>(item->object);
            CallGameFunction<void, 0x004E3F70>(item);
            item = next;
        }
        const uintptr_t span = (node->capacity - reinterpret_cast<uintptr_t>(node->data)) & ~uintptr_t(1);
        if (static_cast<intptr_t>(span) > 2 && node->data) CallGameFunction<void, 0x004E3F70>(node->data);
        CallGameVirtual<void, 3>(allocator, node, uint32_t(sizeof(TreeNode)));
        node = right;
    }
}
}

#include <cmath>
#include <cstring>
namespace ts3::misc {
namespace {
constexpr uint32_t Press = 0x5ece0000, Lift = 0x5ece0001, Cancel = 0x5ece0002;
float BitsFloat(uint32_t bits) { float v; std::memcpy(&v, &bits, 4); return v; }
}
// 0x0049D1D0
TriggerEvent* TriggerDispatcher::FindCode(uint32_t code, int kind, int device)
{
    for (uint32_t i = 0; i < count; ++i)
        if (events[i].code == code && events[i].kind == kind && events[i].device == device) return events + i;
    return nullptr;
}
// 0x0049D220
TriggerEvent* TriggerDispatcher::FindResult(uintptr_t result, int kind, int device, uint32_t flags)
{
    for (uint32_t i = 0; i < count; ++i)
        if (events[i].result == result && events[i].kind == kind && events[i].device == device && (events[i].flags & flags)) return events + i;
    return nullptr;
}
// 0x0049D280
uintptr_t TriggerDispatcher::Remove(TriggerEvent* event, uint32_t message)
{
    TriggerEvent saved;
    std::memcpy(&saved, event, sizeof saved);
    TriggerEvent* last = events + --count;
    if (last > event) std::memcpy(event, last, sizeof *event);
    if (saved.owner && saved.owner->callback && !(saved.flags & 0x2000))
        saved.owner->callback(message, &saved, saved.owner->context);
    uintptr_t result = saved.linked;
    if (result && !(saved.flags & 0x1000)) {
        TriggerEvent* found = FindResult(result, saved.kind, saved.device, 0x1000);
        result = reinterpret_cast<uintptr_t>(found);
        if (found) result = Remove(found, message);
    }
    return result;
}
// 0x0049D330
uintptr_t TriggerDispatcher::Release(TriggerEvent* event)
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < count; ++i)
        if (events[i].result == event->result && events[i].owner == event->owner && events[i].code == event->code) ++n;
    event->duplicate = n > 1;
    return Remove(event, Lift);
}
// 0x0049D390
uintptr_t TriggerDispatcher::Clear(int kind, int device)
{
    uintptr_t result = 0; // EAX is unspecified for an empty table (not an observable return contract).
    for (uint32_t i = count; i; --i) {
        result = uintptr_t(kind);
        if ((kind == 7 || events[i-1].kind == kind) && (device == -2 || events[i-1].device == device))
            result = Remove(events + i-1, Cancel);
    }
    return result;
}
// 0x0049D860
uintptr_t TriggerDispatcher::Detach(Trigger* trigger)
{
    Trigger* sentinel = reinterpret_cast<Trigger*>(this);
    Trigger* current = next;
    uintptr_t result = reinterpret_cast<uintptr_t>(current);
    while (current != sentinel) {
        if (current == trigger) {
            result = reinterpret_cast<uintptr_t>(trigger->prev);
            trigger->prev->next = trigger->next;
            trigger->next->prev = trigger->prev;
            break;
        }
        current = current->next;
        result = reinterpret_cast<uintptr_t>(current);
    }
    for (uint32_t i = 0; i < count;) {
        if (events[i].owner == trigger) result = Remove(events + i, Cancel);
        else ++i;
    }
    return result;
}
// 0x0049DAF0
Trigger* TriggerDispatcher::Resolve(TriggerEvent* event, uintptr_t* result, uint32_t* flags, uintptr_t* linked)
{
    Trigger* sentinel = reinterpret_cast<Trigger*>(this);
    Trigger* first = next;
    Trigger* cursor = sentinel;
    if (cursor == first) return nullptr;
    do {
        Trigger* candidate = cursor->prev;
        if (candidate->resolver && (candidate->device == event->device || candidate->device == -2)) {
            *result = CallGameVirtual<uintptr_t, 1>(candidate->resolver, event, flags, linked);
            if (*result) return candidate;
        }
        if (candidate->flags & 1) break;
        cursor = cursor->prev;
    } while (cursor != first);
    return nullptr;
}
// 0x0049DB70
Trigger* TriggerDispatcher::ResolveAxis(TriggerEvent* event, uintptr_t* result, uint32_t* flags)
{
    Trigger* sentinel = reinterpret_cast<Trigger*>(this);
    Trigger* first = next;
    Trigger* cursor = sentinel;
    if (cursor == first) return nullptr;
    do {
        Trigger* candidate = cursor->prev;
        if (candidate->resolver && (candidate->device == event->device || candidate->device == -2)) {
            *result = CallGameVirtual<uintptr_t, 2>(candidate->resolver, event, flags);
            if (*result) return candidate;
        }
        if (candidate->flags & 1) break;
        cursor = cursor->prev;
    } while (cursor != first);
    return nullptr;
}
// 0x0049DBE0
uintptr_t TriggerDispatcher::Dispatch(TriggerEvent* event, int active)
{
    if (!(uint32_t(int32_t(event->kind)) & mask)) return 0;
    if (!uint8_t(active)) {
        TriggerEvent* found = FindCode(event->code, event->kind, event->device);
        if (!found) return 0;
        Release(found);
        return found->result; // EA reads AFTER swap-removal, hence possibly another event's result.
    }
    if (count >= 32) return 0;
    Trigger* owner = event->owner;
    uintptr_t result = event->result, linked = event->linked;
    uint32_t flags = event->flags;
    if (!owner || !result) owner = Resolve(event, &result, &flags, &linked);
    if (!owner || !owner->callback) return 0;
    TriggerEvent* added = events + count;
    std::memcpy(added, event, sizeof *added);
    added->duplicate = 0;
    for (uint32_t i = 0; i < count; ++i) {
        auto& old = events[i];
        if (old.code == event->code && old.kind == event->kind && old.device == event->device) return result;
        if (old.result == result && old.owner == owner && old.kind == event->kind && old.device == event->device) added->duplicate = 1;
    }
    added->owner = owner; added->result = result; added->linked = linked; added->flags = uint16_t(flags);
    if (flags & 0x800) added->x = -0.0f - added->x;
    ++count;
    if (!(flags & 0x2000)) owner->callback(Press, added, owner->context);
    if (linked && !(flags & 0x1000) && CallGameVirtual<uint8_t, 3>(owner->resolver, linked, this)) {
        TriggerEvent follow{};
        follow.owner = owner; follow.result = linked; follow.linked = linked;
        follow.kind = added->kind; follow.device = added->device; follow.flags = 0x1000;
        result = Dispatch(&follow, 1);
    }
    return result;
}
// 0x0049DE20
uintptr_t TriggerDispatcher::Axis(uint32_t code, int device, uint32_t kind, float value, float threshold, float hysteresis, uint8_t* state)
{
    if (!(kind & mask)) return 0;
    TriggerEvent event{};
    event.code = code; event.kind = int8_t(kind); event.device = int8_t(device); event.x = value;
    uint32_t flags = uint32_t(device);
    uintptr_t result = 0;
    float inner = threshold - hysteresis;
    Trigger* owner = ResolveAxis(&event, &result, &flags);
    if (owner && owner->callback) {
        if ((flags & 1) && value > -0.0f - inner) return result;
        event.owner = owner; event.result = result; event.flags = uint16_t(flags);
        if (value > inner || -0.0f - inner > value) {
            if (flags & 0x800) event.x = -0.0f - event.x;
            *state = 1; owner->callback(Press, &event, owner->context);
        } else if (*state == 1) {
            *state = 0; event.x = 0; owner->callback(Lift, &event, owner->context);
        }
        return result;
    }
    result = 0;
    if (*state == 1) {
        if (value > inner) return 0;
        *state = 0; result = Dispatch(&event, 0);
    } else if (*state == 255) {
        if (-0.0f - inner > value) return 0;
        *state = 0; event.flags = 1; result = Dispatch(&event, 0);
    }
    if (value > threshold) { *state = 1; event.flags = 0; return Dispatch(&event, 1); }
    if (-0.0f - threshold > value) { *state = 255; event.flags = 1; return Dispatch(&event, 1); }
    return result;
}
// 0x0049E050
uintptr_t TriggerDispatcher::Stick(uint32_t code, int device, uint32_t kind, float x, float y, float deadzone, uint8_t* state)
{
    if (!(kind & mask)) return 0;
    TriggerEvent event{};
    event.code = code; event.kind = int8_t(kind); event.device = int8_t(device);
    uintptr_t result = 0; uint32_t flags = code;
    Trigger* owner = ResolveAxis(&event, &result, &flags);
    if (!owner || !owner->callback) return 0;
    event.owner = owner; event.result = result;
    float length = x*x + y*y;
    if (!(length >= deadzone*deadzone)) {
        if (*state == 1) { *state = 0; owner->callback(Lift, &event, owner->context); }
    } else {
        length = std::sqrt(length);
        float scale = 1.0f / ((1.0f - deadzone)*length);
        event.x = ((length-deadzone)*x)*scale;
        event.y = ((length-deadzone)*y)*scale;
        // EA tests (-1 > v), so unordered values are retained, unlike fmax.
        event.x = -1.0f > event.x ? -1.0f : event.x;
        event.x = event.x > 1.0f ? 1.0f : event.x;
        event.y = -1.0f > event.y ? -1.0f : event.y;
        event.y = event.y > 1.0f ? 1.0f : event.y;
        *state = 1; owner->callback(Press, &event, owner->context);
    }
    return result;
}
// 0x0049E230
uintptr_t TriggerDispatcher::Motion(int device, uint32_t kind, uint32_t x, uint32_t y, uint32_t flags)
{
    if (!(kind & mask)) return 0;
    // EA initializes only the first 30 x86 bytes: the trailing word is stack residue.
    // Do not manufacture a deterministic tail that callbacks/the active table never had.
    TriggerEvent event;
    std::memset(&event, 0, offsetof(TriggerEvent, tail));
    event.kind = int8_t(kind); event.device = int8_t(device); event.flags = uint16_t(flags);
    event.code = 1100; event.x = BitsFloat(x); event.y = BitsFloat(y);
    uintptr_t result = flags;
    Trigger* owner = ResolveAxis(&event, &result, &kind);
    if (!owner || !owner->callback) return 0;
    event.owner = owner; event.result = result;
    owner->callback(Press, &event, owner->context);
    return result;
}
// 0x0049E2F0
uintptr_t TriggerDispatcher::Text(uint32_t code, int device, uint32_t kind, uint32_t flags)
{
    if (!(kind & mask)) return 0;
    // EA initializes only the first 30 x86 bytes: the trailing word is stack residue.
    // Do not manufacture a deterministic tail that callbacks/the active table never had.
    TriggerEvent event;
    std::memset(&event, 0, offsetof(TriggerEvent, tail));
    event.kind = int8_t(kind); event.device = int8_t(device); event.flags = uint16_t(flags);
    event.code = 254; event.x = 1.0f;
    uintptr_t result = kind, linked = uintptr_t(device);
    Trigger* owner = Resolve(&event, &result, &flags, &linked);
    if (!owner || !owner->callback) return 0;
    event.code = code; event.owner = owner; event.result = result; event.linked = linked;
    if (!(flags & 0x2000)) owner->callback(Press, &event, owner->context);
    return result;
}
// 0x0049E700
uintptr_t TriggerDispatcher::Digital(uint32_t code, int device, uint32_t kind, int active, uint32_t flags)
{
    if (!(kind & mask)) return 0;
    // EA initializes only the first 30 x86 bytes: the trailing word is stack residue.
    // Do not manufacture a deterministic tail that callbacks/the active table never had.
    TriggerEvent event;
    std::memset(&event, 0, offsetof(TriggerEvent, tail));
    event.code = code; event.kind = int8_t(kind); event.device = int8_t(device);
    event.x = uint8_t(active) ? 1.0f : 0.0f; event.flags = uint16_t(flags);
    return Dispatch(&event, active);
}
}

namespace ts3::misc {
namespace {
struct InputLock {
    void* lock;
    explicit InputLock(void* value) : lock(value) {
#if defined(_WIN32)
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(lock));
#else
        AbortNotRewritten("input critical section");
#endif
    }
    ~InputLock() {
#if defined(_WIN32)
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(lock));
#endif
    }
};
float InputSetting(size_t offset) {
#if defined(_M_IX86)
    return *reinterpret_cast<float*>(*reinterpret_cast<uint8_t**>(0x011D8C44) + offset);
#else
    (void)offset; AbortNotRewritten("input settings global"); return 0;
#endif
}
uint32_t FloatBits(float v) { uint32_t bits; std::memcpy(&bits, &v, 4); return bits; }
}
// 0x007F0010
bool TriggerWinProc::MouseDown(void* window, int, int, uint32_t code, uint32_t flags)
{
    bool handled;
    { InputLock guard(lock); handled = dispatcher->Digital(code, 0, 2, 1, flags) != 0; }
    if (!handled) return false;
    void* manager = CallGameVirtual<void*, 5>(window);
    if (CallGameVirtual<void*, 24>(manager, 1) == window) return false;
    manager = CallGameVirtual<void*, 5>(window);
    CallGameVirtual<void, 25>(manager, 1, window, 0);
    captured = 1;
    return true;
}
// 0x007F00A0
bool TriggerWinProc::Focus(void*, int mode, int)
{
    if (mode == 0 || mode == 1) { InputLock guard(lock); dispatcher->Clear(mode ? 2 : 4, -2); }
    return false;
}
// 0x007F0100
bool TriggerWinProc::KeyDown(void*, int, uint32_t code, uint32_t flags)
{
    InputLock guard(lock); return dispatcher->Digital(code, 0, 4, 1, flags) != 0;
}
// 0x007F0150
bool TriggerWinProc::KeyUp(void*, int, uint32_t code, uint32_t flags)
{
    InputLock guard(lock); return dispatcher->Digital(code, 0, 4, 0, flags) != 0;
}
// 0x007F01A0
bool TriggerWinProc::Character(void*, int, uint32_t code)
{
    InputLock guard(lock); return dispatcher->Text(uint16_t(code), 0, 4, 0) != 0;
}
// 0x007F01F0
bool TriggerWinProc::MouseUp(void* window, int, int, uint32_t code, uint32_t flags)
{
    if (captured) {
        captured = 0;
        void* manager = CallGameVirtual<void*, 5>(window);
        CallGameVirtual<void, 26>(manager, 1, window, 0);
    }
    InputLock guard(lock); return dispatcher->Digital(code, 0, 2, 0, flags) != 0;
}
// 0x007F0260
bool TriggerWinProc::MouseMove(void*, float x, float y, uint32_t flags)
{
    InputLock guard(lock); dispatcher->Motion(0, 2, FloatBits(x), FloatBits(y), flags); return false;
}
// 0x007F02A0
bool TriggerWinProc::Wheel(void*, int, int, int, int ticks)
{
    float threshold = InputSetting(996), hysteresis = InputSetting(1000);
    InputLock guard(lock);
    return dispatcher->Axis(1101, 0, 2, float(ticks)*0.0083333338f, threshold, hysteresis, &wheelState) != 0;
}
// 0x007F0330
bool TriggerWinProc::ButtonDown(void*, int device, uint32_t code)
{
    InputLock guard(lock); return dispatcher->Digital(code, device-2, 1, 1, 0) != 0;
}
// 0x007F0380
bool TriggerWinProc::ButtonUp(void*, int device, uint32_t code)
{
    InputLock guard(lock); return dispatcher->Digital(code, device-2, 1, 0, 0) != 0;
}
// 0x007F03D0
bool TriggerWinProc::PadStick(void*, int device, uint32_t code, float x, float y)
{
    float deadzone = InputSetting(984);
    InputLock guard(lock);
    auto state = reinterpret_cast<uint8_t*>(this) + (offsetof(TriggerWinProc, wheelState)-72) + 4*device + code - 2034;
    return dispatcher->Stick(code, device-2, 1, x, y, deadzone, state) != 0;
}
// 0x007F0450
bool TriggerWinProc::PadAxis(void*, int device, uint32_t code, float value)
{
    float threshold = InputSetting(988), hysteresis = InputSetting(992);
    InputLock guard(lock);
    auto state = reinterpret_cast<uint8_t*>(this) + (offsetof(TriggerWinProc, wheelState)-72) + 8*device + code - 1976;
    return dispatcher->Axis(code, device-2, 1, value, threshold, hysteresis, state) != 0;
}
}
