#include "ts3/audio/m06_B09.h"
#include <intrin.h>
#include <cstring>
namespace ts3::audio {
uint32_t B09Erase(B09Map* map, const uint32_t* key)
{
    const uint32_t before = map->count;
    B09Node** link = &map->buckets[*key % map->bucketCount];
    while (*link && (*link)->value.key != *key) link = &(*link)->next;
    while (*link && (*link)->value.key == *key) {
        B09Node* node = *link;
        *link = node->next;
        CallGameFunction<void, 0x004e3f70>(node);
        --map->count;
    }
    return before - map->count;
}
B09InsertResult* B09Insert(B09Map* map, B09InsertResult* result, const B09Record* value, uint32_t hint)
{
    (void)hint;
    uint32_t bucket = value->key % map->bucketCount;
    B09Node** link = &map->buckets[bucket];
    for (B09Node* node = *link; node; node = node->next) {
        if (node->value.key == value->key) {
            result->node = node; result->bucket = link; result->inserted = false;
            return result;
        }
    }
    struct Growth { bool grow; unsigned char padding[3]; uint32_t buckets; } growth;
    CallGameMethod<void, 0x004e09b0>(map->policy, &growth, map->bucketCount, map->count, uint32_t(1));
    auto* node = static_cast<B09Node*>(CallGameFunction<void*, 0x004e3f90>(
        uint32_t(sizeof(B09Node)), map->label, uint32_t(0), uint32_t(0),
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00f98980),
#else
        "audio map",
#endif
        uint32_t(0xe7)));
    if (node) node->value = *value;
    // EA dereferences a failed allocation here; do not turn this into a recoverable failure.
    node->next = nullptr;
    if (growth.grow) {
        bucket = value->key % growth.buckets;
        CallGameMethod<void, 0x00625a00>(map, growth.buckets);
    }
    node->next = map->buckets[bucket];
    map->buckets[bucket] = node;
    ++map->count;
    result->node = node; result->bucket = &map->buckets[bucket]; result->inserted = true;
    return result;
}
uint32_t B09ManagerErase(B09Manager* manager, uint32_t key)
{
    return B09Erase(&manager->records, &key);
}
void B09ManagerInsert(B09Manager* manager, uint32_t key, uint32_t value0, uint32_t value1)
{
    B09Record value; value.key = key; value.value0 = value0; value.value1 = value1;
    B09InsertResult result;
    B09Insert(&manager->records, &result, &value, key & 0xffffff00u);
}
namespace {
struct ModifierPair { uint32_t key; void* value; };
struct ModifierResult { B09ModifierNode* node; B09ModifierNode** bucket; bool inserted; };
}
void B09RegisterCallback(B09Manager* manager, uint32_t key, void* callback)
{
    ModifierPair pair{key, callback}; ModifierResult result;
    CallGameMethod<void, 0x00b34440>(&manager->callbacks, &result, &pair, key & 0xffffff00u);
}
uint8_t B09RegisterModifier(B09Manager* manager, uint32_t key, void* modifier)
{
    if (!modifier) return 0;
    ModifierPair pair{key, modifier}; ModifierResult result;
    CallGameMethod<void, 0x00b34440>(&manager->modifiers, &result, &pair,
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(modifier)) & 0xffffff00u);
    if (!result.inserted) return 0;
    CallGameVirtual<uintptr_t, 1>(modifier);
    return 1;
}
namespace {
struct Retained { void** vtable; volatile long references; };
}
B09TreeNode** B09TreeInsert(B09Tree* tree, B09TreeNode** output, B09TreeNode* position, const B09TreeValue* value, uint32_t forceLeft)
{
    const bool right = !(forceLeft & 0xff) && &position->links != &tree->anchor && value->key >= position->value.key;
    auto* node = static_cast<B09TreeNode*>(CallGameFunction<void*, 0x004e3f90>(
        uint32_t(sizeof(B09TreeNode)), tree->label, uint32_t(0), uint32_t(0),
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00f98980),
#else
        "audio tree",
#endif
        uint32_t(0xe7)));
    // EA checks the value subobject, not the allocation. Allocation failure is still fatal.
    node->value = *value;
    if (value->object) _InterlockedExchangeAdd(&static_cast<Retained*>(value->object)->references, 1);
    CallGameFunction<void, 0x004e0c40>(&node->links, &position->links, &tree->anchor, uint32_t(right));
    ++tree->count;
    *output = node;
    return output;
}
namespace {
void RetainSound(void* sound)
{
    if (sound) _InterlockedExchangeAdd(&static_cast<Retained*>(sound)->references, 1);
}
void ReleaseSound(void* sound)
{
    auto* object = static_cast<Retained*>(sound);
    if (object && _InterlockedExchangeAdd(&object->references, -1) == 1) {
        _InterlockedExchange(&object->references, 1);
        CallGameVirtual<void, 0>(object, uint32_t(1));
    }
}
}
void B09NotifySounds(B09Manager* manager, uint32_t kind, uintptr_t payload, uintptr_t arg2, uintptr_t arg3, void* event)
{
    struct EventView { unsigned char prefix[0xec]; uint32_t state; };
    if (kind == 0x03a129ee && event && static_cast<EventView*>(event)->state == 0 && manager->soundCount) {
        auto* node = manager->soundAnchor.left;
        while (node != &manager->soundAnchor) {
            void* sound = reinterpret_cast<B09TreeNode*>(node)->value.object;
            RetainSound(sound);
            CallGameMethod<void, 0x00871f30>(sound, payload);
            ReleaseSound(sound);
            node = CallGameFunction<misc::ResourceTreeNode*, 0x004e0b20>(node);
        }
    }
    CallGameMethod<void, 0x008bcae0>(manager, kind, payload, arg2, arg3, event);
}
namespace {
struct MapIterator { B09ModifierNode* node; B09ModifierNode** bucket; };
struct Box { unsigned char bytes[32]; };
}
void B09InvokeCallback(B09Manager* manager, uint32_t kind, uint32_t key, void* middle, uint32_t last)
{
    if (kind != 0x03a129ee) return;
    auto& map = manager->callbacks;
    MapIterator found;
    CallGameMethod<void, 0x00d38b00>(&map, &found, &key);
    if (found.node == map.buckets[map.bucketCount]) return;
    void* callback = found.node->modifier;
    MapIterator erased;
    CallGameMethod<void, 0x0066f320>(&map, &erased, found.node, found.bucket);
    if (!callback) return;
    void* scripting = CallGameFunction<void*, 0x007692b0>();
    if (!scripting) return;
    Box args[3], result;
    CallGameMethod<void, 0x004f6180>(&args[0], &key);
    CallGameMethod<void, 0x004f61e0>(&args[1], middle);
    CallGameMethod<void, 0x004f6180>(&args[2], &last);
    CallGameVirtual<void, 77>(scripting, &result, callback, args, uint32_t(3));
#if defined(_M_IX86)
    *reinterpret_cast<void**>(result.bytes) = reinterpret_cast<void*>(0x00fb96b4);
#endif
    if (result.bytes[8] & 4) CallGameMethod<void, 0x004f55a0>(&result, uint32_t(0));
#if defined(_M_IX86)
    // MSVC's vector destructor iterator uses stdcall (callee pops all four arguments).
    using DestroyArray = void(__stdcall*)(void*, uint32_t, uint32_t, void*);
    reinterpret_cast<DestroyArray>(0x00446710)(args, uint32_t(32), uint32_t(3), reinterpret_cast<void*>(0x004f5660));
#else
    NotRewritten("audio script argument destruction");
#endif
}
void B09DispatchEvent(B09Manager* manager, void* event)
{
    const char* name = CallGameMethod<const char*, 0x0089f9c0>(event, uint32_t(0x0347537e));
    if (!name || !manager->provider) return;
    uint32_t hash = CallGameFunction<uint32_t, 0x004fa590>(name, uint32_t(0x811c9dc5), uint32_t(1));
    uint32_t count = 0; uint32_t* ids = nullptr;
    if (!CallGameVirtual<uint8_t, 19>(manager->provider, hash, &count, &ids) || !count) {
        const char* underscore = std::strchr(name, '_');
        if (!underscore) return;
        char prefix[128];
        const size_t length = static_cast<size_t>(underscore-name)+1;
        // EA does not bound this prefix copy. Inputs over 127 bytes retain the original overflow hazard.
        std::memcpy(prefix, name, length); prefix[length] = 0;
        hash = CallGameFunction<uint32_t, 0x004fa590>(prefix, uint32_t(0x811c9dc5), uint32_t(1));
        if (!CallGameVirtual<uint8_t, 19>(manager->provider, hash, &count, &ids) || !count) return;
    }
    char result[128];
    CallGameFunction<uint32_t, 0x004ff320>(result, name, uint32_t(128));
    CallGameMethod<uint64_t, 0x0089fbd0>(event, uint32_t(0x0347537e));
    struct State { uint32_t first, second; uint8_t flag; } state{0,0,0};
    for (uint32_t i=0; i<count; ++i) {
        uint32_t key = ids[i]; MapIterator found;
        CallGameMethod<void, 0x00d38b00>(&manager->modifiers, &found, &key);
        if (found.node != manager->modifiers.buckets[manager->modifiers.bucketCount])
            CallGameVirtual<void, 4>(found.node->modifier, event, result, uint32_t(128), &state);
    }
    const uint64_t key = CallGameVirtual<uint64_t, 70>(manager, result);
    CallGameMethod<void, 0x0089fb50>(event, uint32_t(0x03475381), key);
}
uint8_t B09SelectSound(B09Manager* manager, const B09Query* query, B09Candidate** output)
{
    if (!output) return 0;
    auto owner = [](B09SoundLink* link) -> B09Candidate* {
        return link ? reinterpret_cast<B09Candidate*>(reinterpret_cast<unsigned char*>(link) - offsetof(B09SoundHead, link)) : nullptr;
    };
    B09Candidate* best = nullptr;
    float bestGain = 0.0f;
    float bestPriority = query->priority;
    auto* cursor = &manager->soundList.link;
    auto* end = manager->soundList.link.first;
    while (cursor != end) {
        auto* sound = owner(cursor->next);
        bool eligible = !CallGameMethod<uint8_t, 0x008ab4d0>(sound);
        if (eligible) eligible = !CallGameVirtual<uint8_t, 8>(sound, int32_t(-1), uint32_t(0));
        if (eligible) eligible = !CallGameMethod<uint8_t, 0x008ab4f0>(sound);
        if (eligible) eligible = !(CallGameMethod<float, 0x008ab510>(sound) > query->priority);
        if (eligible) eligible = !(sound->flags & 0x40000);
        if (eligible && query->type == 0xee8347b0)
            eligible = CallGameVirtual<uint64_t, 19>(sound) != query->key;
        if (eligible && reinterpret_cast<uintptr_t>(query->keyBegin) >= reinterpret_cast<uintptr_t>(query->keyEnd))
            eligible = CallGameVirtual<uint64_t, 19>(sound) == query->key;
        if (eligible && reinterpret_cast<uintptr_t>(query->idBegin) >= reinterpret_cast<uintptr_t>(query->idEnd))
            eligible = CallGameMethod<uint32_t, 0x008ab310>(sound) == query->id;
        if (eligible) {
            const float gain = CallGameMethod<float, 0x008ab320>(sound);
            bool replace = gain >= bestGain;
            if (!replace) replace = CallGameMethod<float, 0x008ab510>(sound) < bestPriority;
            if (replace) {
                bestGain = CallGameMethod<float, 0x008ab320>(sound);
                bestPriority = CallGameMethod<float, 0x008ab510>(sound);
                best = sound;
            }
        }
        cursor = cursor->next;
    }
    if (!best) return 0;
    *output = best;
    CallGameVirtual<uintptr_t, 0>(best);
    return 1;
}
uint8_t B09Shutdown(B09Manager* manager)
{
    if (manager->provider) {
        void* provider = manager->provider;
        manager->provider = nullptr;
        CallGameVirtual<void, 1>(provider);
    }
    if (manager->stream) CallGameMethod<void, 0x00c0fd60>(manager->stream);
    manager->pendingState = 0;
    auto& map = manager->modifiers;
    B09ModifierNode** bucket = map.buckets;
    while (!*bucket) ++bucket;
    B09ModifierNode* node = *bucket;
    B09ModifierNode* end = map.buckets[map.bucketCount];
    while (node != end) {
        CallGameVirtual<void, 2>(node->modifier);
        node = node->next;
        while (!node) node = *++bucket;
    }
    CallGameMethod<void, 0x007d3af0>(&map, map.buckets, map.bucketCount);
    map.count = 0;
    return CallGameMethod<uint8_t, 0x00868e40>(manager);
}
void B09UpdateSounds(B09Manager* manager, float first, float second)
{
#if defined(_M_IX86)
    struct Routing { unsigned char prefix[0x74]; void* target; };
    auto* routing = *reinterpret_cast<Routing**>(0x011ecbc4);
    if (routing && routing->target) CallGameMethod<void, 0x00d69940>(routing->target, manager->position);
#endif
    CallGameMethod<void, 0x00868e70>(manager, first, second);
    if (manager->soundCount) {
        auto* node = manager->soundAnchor.left;
        while (node != &manager->soundAnchor) {
            void* sound = reinterpret_cast<B09TreeNode*>(node)->value.object;
            RetainSound(sound);
            CallGameMethod<void, 0x00872e00>(sound, first, second);
            ReleaseSound(sound);
            node = CallGameFunction<misc::ResourceTreeNode*, 0x004e0b20>(node);
        }
    }
}
B09TreeNode** B09TreeErase(B09Tree* tree, B09TreeNode** output, B09TreeNode* node)
{
    --tree->count;
    auto* next = CallGameFunction<B09TreeNode*, 0x004e0b20>(&node->links);
    misc::EraseResourceTree(&node->links, &tree->anchor);
    auto* object = static_cast<Retained*>(node->value.object);
    if (object && _InterlockedExchangeAdd(&object->references, -1) == 1) {
        _InterlockedExchange(&object->references, 1);
        CallGameVirtual<void, 0>(object, uint32_t(1));
    }
    CallGameFunction<void, 0x004e3f70>(node);
    *output = next;
    return output;
}
}

namespace ts3::audio {
uint8_t __fastcall HookB09SelectSound(B09Manager* manager, void*, const B09Query* query, B09Candidate** output)
{
    return B09SelectSound(manager, query, output);
}
uint32_t __fastcall HookB09Erase(B09Map* map, void*, const uint32_t* key)
{
    return B09Erase(map, key);
}
B09InsertResult* __fastcall HookB09Insert(B09Map* map, void*, B09InsertResult* result, const B09Record* value, uint32_t hint)
{
    return B09Insert(map, result, value, hint);
}
uint8_t __fastcall HookB09Shutdown(B09Manager* manager, void*)
{
    return B09Shutdown(manager);
}
void __fastcall HookB09InvokeCallback(B09Manager* manager, void*, uint32_t kind, uint32_t key, void* middle, uint32_t last)
{
    return B09InvokeCallback(manager, kind, key, middle, last);
}
void __fastcall HookB09NotifySounds(B09Manager* manager, void*, uint32_t kind, uintptr_t payload, uintptr_t arg2, uintptr_t arg3, void* event)
{
    return B09NotifySounds(manager, kind, payload, arg2, arg3, event);
}
uint32_t __fastcall HookB09ManagerErase(B09Manager* manager, void*, uint32_t key)
{
    return B09ManagerErase(manager, key);
}
void __fastcall HookB09UpdateSounds(B09Manager* manager, void*, float first, float second)
{
    return B09UpdateSounds(manager, first, second);
}
void __fastcall HookB09ManagerInsert(B09Manager* manager, void*, uint32_t key, uint32_t value0, uint32_t value1)
{
    return B09ManagerInsert(manager, key, value0, value1);
}
void __fastcall HookB09RegisterCallback(B09Manager* manager, void*, uint32_t key, void* callback)
{
    return B09RegisterCallback(manager, key, callback);
}
void __fastcall HookB09DispatchEvent(B09Manager* manager, void*, void* event)
{
    return B09DispatchEvent(manager, event);
}
B09TreeNode** __fastcall HookB09TreeInsert(B09Tree* tree, void*, B09TreeNode** output, B09TreeNode* position, const B09TreeValue* value, uint32_t forceLeft)
{
    return B09TreeInsert(tree, output, position, value, forceLeft);
}
uint8_t __fastcall HookB09RegisterModifier(B09Manager* manager, void*, uint32_t key, void* modifier)
{
    return B09RegisterModifier(manager, key, modifier);
}
B09TreeNode** __fastcall HookB09TreeErase(B09Tree* tree, void*, B09TreeNode** output, B09TreeNode* node)
{
    return B09TreeErase(tree, output, node);
}
}
