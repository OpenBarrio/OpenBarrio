// cls_4a38c0_x.cpp - portable rewrite of the TS3W.exe ResourceMan / DDFRecord / ResourceBase
// family (misc/, 26 entry points at 0x004A38C0..0x004A662B, 0x0072AE00 and 0x00739D90).
//
// Every call into a unit that is not rewritten (and into the already verified clu_4e4230 /
// cls_4a4ba0 helpers, so the differential test can patch the game addresses) goes through the
// bridge in ts3/layout.h.  Unit-internal calls (the destructor thunks and the DDFRecord
// resetter) are direct.  The `this` convention of each entry point comes from the vtables:
// primary 0x00FB2420 (object) and 0x00FB2410 (object+4).
#include "ts3/misc/cls_4a38c0_x.h"

#include <cstdint>
#include <cstring>

#include <intrin.h>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {
namespace {

// The game's vtables written by the destructors and the base queryInterface constants.
constexpr uintptr_t kVtableResourceMan = 0x00FB2420;
constexpr uintptr_t kVtableDDFRecord = 0x00FB22C0;
constexpr uintptr_t kVtableFileObject = 0x00FB2270;
constexpr uintptr_t kVtableBaseObject = 0x00FB84D0;
constexpr uintptr_t kVtableBaseSecondary = 0x00FA1B78;
constexpr uintptr_t kVtableBaseSecondaryDead = 0x00FB21D8;

// The allocator name string at 0x00FB24C0 ("ResourceSystem/ResourceMan/DDFRecord").
constexpr char kRecordAllocName[] = "ResourceSystem/ResourceMan/DDFRecord";

// The game passes the address of the global kTimeoutNone at 0x00FB21D4; the value is the same
// 0xFFFFFFFF constant, so a local copy keeps the behavior (the tests patch the mutex calls).
constexpr uint32_t kTimeoutNone = 0xFFFFFFFFu;

#if defined(_M_IX86)
#define TS3_THIS_CALL __thiscall
#else
#define TS3_THIS_CALL
#endif

// Calls a virtual method of a game object by byte-free slot index (x86 __thiscall).
template <class R, class... Args>
R VirtualCall(void* object, int slot, Args... args) {
    using Method = R(TS3_THIS_CALL*)(void*, Args...);
    void* const* const* vtable = reinterpret_cast<void* const* const*>(object);
    return reinterpret_cast<Method>((*vtable)[slot])(object, args...);
}

void LockMutex(void* mutex) {
    CallGameMethod<int32_t, 0x004E16F0>(static_cast<unsigned char*>(mutex), &kTimeoutNone);
}

void UnlockMutex(void* mutex) {
    CallGameMethod<int32_t, 0x004E17B0>(static_cast<unsigned char*>(mutex));
}

// Resets one std::_Tree sentinel to the empty state, in the original store order.
void ResetTreeSentinel(services::ServiceTree* tree) {
    services::ServiceNodeBase* anchor = &tree->anchor;
    anchor->left = anchor;
    anchor->parent = nullptr;
    anchor->color = 0;
    tree->count = 0;
    anchor->right = anchor;
}

}  // namespace

void* ResourceBaseQueryInterface(ResourceBase* self, uint32_t id) {
    // 0x004A38C0: the original instruction sequence = self unless the adjusted
    // pointer is 4.
    if (id != 0xEE3F516E) return nullptr;
    const uintptr_t adjusted = reinterpret_cast<uintptr_t>(self);
    return (adjusted - 4) != 0 ? self : nullptr;
}

void* DDFRecordKeyAddress(DDFRecord* self) {
    // 0x004A38E0
    return reinterpret_cast<unsigned char*>(self) + 0x230;
}

int32_t DDFRecordSetKey(DDFRecord* self, const uint32_t* key) {
    // 0x004A38F0: four dword copies; the return value is the last dword read.
    self->key[0] = key[0];
    self->key[1] = key[1];
    self->key[2] = key[2];
    self->key[3] = key[3];
    return static_cast<int32_t>(key[3]);
}

void* DDFRecordIndex(DDFRecord* self) {
    // 0x004A3920
    return self->index;
}

uint32_t DDFRecordReset(DDFRecord* self) {
    // 0x004A3930: the flagless path only defines AL=1; the flagged path returns the whole
    // virtual result.
    if (self->flag244 == 0) return 1;
    void* index = self->index;
    return static_cast<uint32_t>(VirtualCall<int32_t>(index, 15, self));
}

uint8_t ResourceManDeactivate(services::ResourceMan* self) {
    // 0x004A3950: the virtual call's result is discarded; AL=1 is always returned.
    if (self->active != 0) {
        void* const* vtable = *reinterpret_cast<void* const* const*>(self);
        using Method = int32_t(TS3_THIS_CALL*)(void*);
        self->active = 0;
        reinterpret_cast<Method>(vtable[7])(self);
    }
    return 1;
}

void* ResourceManQueryInterface(services::ResourceMan* self, uint32_t id) {
    // 0x004A3970: this is the secondary subobject (object+4).
    if (id == 0x0226A1DE) return reinterpret_cast<unsigned char*>(self) - 4;
    if (id != 0xEE3F516E) return nullptr;
    const uintptr_t adjusted = reinterpret_cast<uintptr_t>(self);
    return (adjusted - 4) != 0 ? self : nullptr;
}

uint32_t ResourceManClassId(services::ResourceMan* self) {
    // 0x004A39A0
    (void)self;
    return 0x34728492u;
}

int32_t ResourceManLockMutex(services::ResourceMan* self, uint32_t lock) {
    // 0x004A39B0: mutex at +0x80; a byte test of the argument selects lock or unlock.
    unsigned char* mutex = reinterpret_cast<unsigned char*>(self) + 0x80;
    if ((lock & 0xFF) != 0) {
        return CallGameMethod<int32_t, 0x004E16F0>(mutex, &kTimeoutNone);
    }
    return CallGameMethod<int32_t, 0x004E17B0>(mutex);
}

ResourceBase* ResourceBaseDestroySecondary(ResourceBase* self, uint32_t flags) {
    // 0x004A3A90
    return ResourceBaseDestroy(reinterpret_cast<ResourceBase*>(reinterpret_cast<unsigned char*>(self) - 4), flags);
}

int32_t ResourceManRelease(services::ResourceMan* self) {
    // 0x004A3AA0: this is the secondary subobject (object+4); the reference count is at +4
    // from there (object+8).  At zero the count is pinned to 1 and the primary destructor is
    // called with flag 1, but only when the object pointer is non-null.
    auto* counter = reinterpret_cast<volatile long*>(reinterpret_cast<unsigned char*>(self) + 4);
    const int32_t remaining = _InterlockedDecrement(counter);
    if (remaining == 0) {
        _InterlockedExchange(counter, 1);
        void* object = reinterpret_cast<unsigned char*>(self) - 4;
        if (object != nullptr) {
            void* const* vtable = *reinterpret_cast<void* const* const*>(object);
            using Method = void(TS3_THIS_CALL*)(void*, int32_t);
            reinterpret_cast<Method>(vtable[0])(object, 1);
        }
    }
    return remaining;
}

uint8_t ResourceManOpen(services::ResourceMan* self, uint32_t flags, int32_t mode, uint32_t unused) {
    // 0x004A3AD0: three stack arguments, ret 0xC.  Mode 6 is remapped from the flag bits and
    // only the flag's low byte is tested.
    (void)unused;
    if (self->active == 0) return 0;
    // Mode 6 is remapped from the flag bits before the mode check.
    if (mode == 6) {
        mode = (flags & 2) != 0 ? (((flags & 1) != 0) ? 4 : 2) : 3;
    }
    if ((flags & 2) == 0 && mode != 3) return 0;
    const wchar_t* path = self->path.begin;
    uint8_t result = static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DBF60>(path));
    switch (mode) {
        case 1:
            if (result != 0) return 0;
            result = static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DCD40>(
                const_cast<wchar_t*>(path)));
            if (result == 0) return 0;
            break;
        case 2:
        case 4:
            if (result == 0) {
                result = static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DCD40>(
                    const_cast<wchar_t*>(path)));
                if (result == 0) return 0;
            }
            break;
        case 3:
            if (result == 0) return 0;
            break;
        case 5:
            if (result == 0) return 0;
            if (static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DCC00>(path, 1)) == 0) return 0;
            if (static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DCD40>(
                    const_cast<wchar_t*>(path))) == 0) {
                return 0;
            }
            break;
        default:
            return 0;
    }
    if (self->flag20 == 0) {
        result = 1;
    } else {
        result = static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DBF60>(path));
        if (result != 0) result = 1;
    }
    if (flags == 0) return 0;
    if (result == 0) return 0;
    const uintptr_t state = reinterpret_cast<uintptr_t>(self->state);
    if (state != 0 && state != flags) return 0;
    void* const* vtable = *reinterpret_cast<void* const* const*>(self);
    using Method = int32_t(TS3_THIS_CALL*)(void*, int32_t);
    self->state = reinterpret_cast<void*>(static_cast<uintptr_t>(flags));
    reinterpret_cast<Method>(vtable[23])(self, 0);
    return 1;
}

ResourceBase* ResourceBaseDestroy(ResourceBase* self, uint32_t flags) {
    // 0x004A3D30: the +4 store happens twice in the original; the first one is dead.
    self->secondaryVtable = reinterpret_cast<void* const*>(kVtableBaseSecondaryDead);
    self->secondaryVtable = reinterpret_cast<void* const*>(kVtableBaseSecondary);
    self->vtable = reinterpret_cast<void* const*>(kVtableBaseObject);
    if ((flags & 1) != 0) {
        CallGameFunction<void, 0x004E4230>(self);
    }
    return self;
}

DDFRecord* DDFRecordDestroySecondary(DDFRecord* self, uint32_t flags) {
    // 0x004A3D60
    return DDFRecordDestroy(reinterpret_cast<DDFRecord*>(reinterpret_cast<unsigned char*>(self) - 8), flags);
}

uint32_t DDFRecordResetSecondary(DDFRecord* self) {
    // 0x004A3D70
    return DDFRecordReset(reinterpret_cast<DDFRecord*>(reinterpret_cast<unsigned char*>(self) - 8));
}

DDFRecord* DDFRecordDestroy(DDFRecord* self, uint32_t flags) {
    // 0x004A4330: vtables, FileObject subobject destruction, base vtable, optional free.
    unsigned char* fileObject = reinterpret_cast<unsigned char*>(self) + 8;
    self->vtable = reinterpret_cast<void* const*>(kVtableDDFRecord);
    self->fileObject = reinterpret_cast<void* const*>(kVtableFileObject);
    CallGameMethod<void, 0x004DB9D0>(fileObject);
    self->vtable = reinterpret_cast<void* const*>(kVtableBaseObject);
    if ((flags & 1) != 0) {
        CallGameFunction<void, 0x004E4230>(self);
    }
    return self;
}

uint8_t ResourceManSetPath(services::ResourceMan* self, const wchar_t* path) {
    // 0x004A4370: assign + append a separator only while no state is installed.
    if (self->state != nullptr) return 0;
    size_t length = 0;
    while (path[length] != L'\0') ++length;
    CallGameMethod<void, 0x00408E20>(&self->path, path, path + length);
    CallGameFunction<int32_t, 0x004EDD80>(&self->path);
    return self->state == nullptr ? 1 : 0;
}

uint8_t ResourceManPurgeRecords(services::ResourceMan* self) {
    // 0x004A44C0: under the mutex, release records whose link list holds exactly one entry.
    unsigned char* mutex = reinterpret_cast<unsigned char*>(self) + 0x80;
    LockMutex(mutex);
    unsigned char* sentinel = reinterpret_cast<unsigned char*>(self) + 0x54;
    unsigned char* node = *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(self) + 0x58);
    while (node != sentinel) {
        unsigned char* links = node + 0x38;
        unsigned char* item = *reinterpret_cast<unsigned char**>(links);
        int32_t entries = 0;
        if (item != links) {
            do {
                item = *reinterpret_cast<unsigned char**>(item);
                ++entries;
            } while (item != links);
        }
        if (entries == 1) {
            DDFRecord* record = *reinterpret_cast<DDFRecord**>(
                *reinterpret_cast<unsigned char**>(links) + 8);
            void* resolved = VirtualCall<void*>(record, 6);
            if ((VirtualCall<uint8_t>(resolved, 4) & 2) != 0) {
                VirtualCall<void>(resolved, 13);
            }
        }
        node = CallGameFunction<unsigned char*, 0x004E0B20>(node);
    }
    UnlockMutex(mutex);
    return 1;
}

int32_t ResourceManGetRefCount(services::ResourceMan* self, const services::ServiceKey* key) {
    // 0x004A4540: finds the key and counts the link list at node+0x38.
    services::ServiceNode* node = nullptr;
    services::ServiceNode** out =
        CallGameMethod<services::ServiceNode**, 0x0057A7D0>(&self->recordTree, &node, key);
    node = *out;
    if (node == reinterpret_cast<services::ServiceNode*>(&self->recordTree.anchor)) return 0;
    unsigned char* links = reinterpret_cast<unsigned char*>(node) + 0x38;
    unsigned char* item = *reinterpret_cast<unsigned char**>(links);
    int32_t count = 0;
    while (item != links) {
        item = *reinterpret_cast<unsigned char**>(item);
        ++count;
    }
    return count;
}

int32_t ResourceManListEntries(services::ResourceMan* self, void* vector, void* filter) {
    // 0x004A4C10: appends the accepted 16-byte keys and returns the accept count.
    unsigned char* mutex = reinterpret_cast<unsigned char*>(self) + 0x80;
    LockMutex(mutex);
    int32_t count = 0;
    unsigned char* sentinel = reinterpret_cast<unsigned char*>(self) + 0x54;
    unsigned char* node = *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(self) + 0x58);
    if (node != sentinel) {
        do {
            unsigned char* value = node + 0x10;
            bool accepted = true;
            if (filter != nullptr) {
                accepted = (VirtualCall<uint8_t>(filter, 1, value) & 0xFF) != 0;
            }
            if (accepted) {
                if (vector != nullptr) {
                    unsigned char* end = *reinterpret_cast<unsigned char**>(
                        reinterpret_cast<unsigned char*>(vector) + 4);
                    unsigned char* capacity = *reinterpret_cast<unsigned char**>(
                        reinterpret_cast<unsigned char*>(vector) + 8);
                    if (end >= capacity) {
                        CallGameMethod<void, 0x006D3810>(vector, end, value);
                    } else {
                        *reinterpret_cast<unsigned char**>(reinterpret_cast<unsigned char*>(vector) + 4) =
                            end + 16;
                        if (end != nullptr) {
                            std::memcpy(end, value, 16);
                        }
                    }
                }
                ++count;
            }
            node = CallGameFunction<unsigned char*, 0x004E0B20>(node);
        } while (node != sentinel);
    }
    UnlockMutex(mutex);
    return count;
}

uint8_t ResourceManClear(services::ResourceMan* self) {
    // 0x004A5240: clears both trees and the state, resetting both sentinels.
    CallGameMethod<void, 0x004A4610>(self);
    services::ServiceTree* tree = &self->recordTree;
    CallGameMethod<void, 0x004A4D40>(tree, tree->anchor.parent);
    ResetTreeSentinel(tree);
    self->state = nullptr;
    tree = &self->entryTree;
    CallGameMethod<void, 0x004A4A10>(tree, tree->anchor.parent);
    ResetTreeSentinel(tree);
    return 1;
}

services::ResourceMan* ResourceManDestroySecondary(services::ResourceMan* self, uint32_t flags) {
    // 0x004A5610
    return ResourceManDestroy(reinterpret_cast<services::ResourceMan*>(
                                  reinterpret_cast<unsigned char*>(self) - 4),
                              flags);
}

services::ResourceMan* ResourceManDestroy(services::ResourceMan* self, uint32_t flags) {
    // 0x004A6030: ResourceMan::Destroy (0x004A5620) then the optional free.
    CallGameMethod<void, 0x004A5620>(self);
    if ((flags & 1) != 0) {
        CallGameFunction<void, 0x004E4230>(self);
    }
    return self;
}

uint8_t ResourceManOpenRecord(services::ResourceMan* self, const services::ServiceKey* key,
                              DDFRecord** out, uint32_t flags, int32_t mode, uint32_t unused,
                              int32_t* times) {
    // 0x004A62B0: the record open/register machine.  All tree, file and allocator calls go
    // through the bridge, so the differential test patches one address for both runs.
    (void)unused;
    unsigned char* mutex = reinterpret_cast<unsigned char*>(self) + 0x80;
    LockMutex(mutex);

    if (self->active == 0) {
        UnlockMutex(mutex);
        return 0;
    }

    unsigned char* sentinel = reinterpret_cast<unsigned char*>(self) + 0x54;
    unsigned char* node = nullptr;
    CallGameMethod<unsigned char**, 0x0057A7D0>(&self->recordTree, &node, key);
    uint8_t found = node != sentinel ? 1 : 0;

    wchar_t fileName[260];
    if (found != 0 && times != nullptr) {
        CallGameFunction<int32_t, 0x004EC6D0>(
            fileName, self->path.begin,
            *reinterpret_cast<const wchar_t**>(node + 0x20));
        // The original zeroes +0xC and +0 before the stamp call and then mirrors it at +4/+8.
        times[3] = 0;
        times[0] = 0;
        const int32_t stamp = CallGameFunction<int32_t, 0x004DBC60>(fileName);
        times[1] = stamp;
        times[2] = stamp;
    }

    if (out == nullptr) {
        UnlockMutex(mutex);
        return found;
    }
    if (flags == 0 ||
        (reinterpret_cast<uintptr_t>(self->state) & flags) != flags) {
        UnlockMutex(mutex);
        return 0;
    }

    unsigned char source[0x200];
    const uint8_t probed = VirtualCall<uint8_t>(self, 26, key, source);
    if (probed == 0) {
        UnlockMutex(mutex);
        CallGameMethod<void, 0x004A3DE0>(self, key, source);
        LockMutex(mutex);
    }
    CallGameFunction<int32_t, 0x004EC6D0>(fileName, self->path.begin,
                                          reinterpret_cast<const wchar_t*>(source));

    if (found == 0 &&
        static_cast<uint8_t>(CallGameFunction<int32_t, 0x004DBBA0>(fileName)) != 0) {
        CallGameMethod<void, 0x004A6050>(self, key, fileName, source);
        CallGameMethod<unsigned char**, 0x0057A7D0>(&self->recordTree, &node, key);
        found = 1;
    }

    int32_t recordMode = mode;
    if (mode == 6) {
        recordMode = (flags & 2) != 0 ? (((flags & 1) != 0) ? 4 : 2) : 3;
        mode = recordMode;
    }
    if (found != 0) {
        if (recordMode == 1) {
            UnlockMutex(mutex);
            return 0;
        }
    } else {
        if (recordMode == 3 || recordMode == 5) {
            UnlockMutex(mutex);
            return 0;
        }
        if (recordMode == 1 || recordMode == 2 || recordMode == 4) {
            const uint8_t readable = static_cast<uint8_t>(
                CallGameFunction<int32_t, 0x004DBB20>(fileName, recordMode == 2 ? 1 : 0));
            if (readable == 0) {
                UnlockMutex(mutex);
                return 0;
            }
            CallGameMethod<void, 0x004A6050>(self, key, fileName, source);
            CallGameMethod<unsigned char**, 0x0057A7D0>(&self->recordTree, &node, key);
        }
    }

    const int32_t payloadFlags = CallGameMethod<int32_t, 0x004A3D80>(self, node + 0x20);
    if (payloadFlags != 0 && ((flags | payloadFlags) & 2) != 0) {
        UnlockMutex(mutex);
        return 0;
    }

    void* memory = CallGameFunction<void*, 0x004E3DB0>(0x248, self->allocator, kRecordAllocName);
    DDFRecord* record = nullptr;
    if (memory != nullptr) {
        record = CallGameMethod<DDFRecord*, 0x004A42C0>(memory, self, key);
    }
    VirtualCall<void>(record, 1);
    unsigned char* fileObject = reinterpret_cast<unsigned char*>(record) + 8;
    VirtualCall<void>(fileObject, 15, fileName);
    const uint8_t opened = VirtualCall<uint8_t>(fileObject, 19, flags, 3, 1, 0);
    if (opened == 0) {
        VirtualCall<void>(record, 2);
        UnlockMutex(mutex);
        return 0;
    }
    if (mode == 2 || mode == 5) {
        VirtualCall<void>(fileObject, 8, 0);
    }
    record->flag244 = 1;

    // The link list header lives inside the found tree node at +0x38; 0x005C9100 overwrites
    // the node local with the record, so the list head/count are read first.
    unsigned char* payload = node + 0x20;
    RecordListLink* first = *reinterpret_cast<RecordListLink**>(payload + 0x18);
    const uint32_t linkCount = *reinterpret_cast<uint32_t*>(payload + 0x20);
    CallGameMethod<void, 0x005C9100>(&node, record);

    RecordListLink* link = static_cast<RecordListLink*>(
        CallGameFunction<void*, 0x004E3F90>(12, linkCount, 0u, 0u, 0xF98980u, 0xE7u));
    DDFRecord* listValue = reinterpret_cast<DDFRecord*>(node);
    link->value = listValue;
    if (listValue != nullptr) {
        VirtualCall<void>(listValue, 1);
    }
    link->next = first;
    link->previous = first->previous;
    first->previous->next = link;
    first->previous = link;
    if (listValue != nullptr) {
        VirtualCall<void>(listValue, 2);
    }
    *out = record;
    UnlockMutex(mutex);
    return 1;
}

uint8_t ResourceManActivate(services::ResourceMan* self) {
    // 0x0072AE00
    if (self->active == 0) self->active = 1;
    return 1;
}

void* DDFRecordFileObject(DDFRecord* self) {
    // 0x00739D90: object+8 or null. Single return so every mutation of the value (the
    // null, the +8, the comparison) changes the observable result.
    return self == nullptr ? nullptr : reinterpret_cast<unsigned char*>(self) + 8;
}

}  // namespace misc
}  // namespace ts3