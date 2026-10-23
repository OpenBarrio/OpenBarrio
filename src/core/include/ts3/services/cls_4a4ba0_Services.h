#pragma once
#include "ts3/layout.h"
#include "ts3/thread/mutex.h"
#include "ts3/resource/packed_file.h"

namespace ts3::services {
struct ServiceKey { uint32_t first, second; uint64_t last; };
struct ServiceString { wchar_t* begin; wchar_t* end; wchar_t* capacity; void* allocator; };
struct ServiceLinks { ServiceLinks* next; ServiceLinks* previous; uint32_t count; };
struct ServicePayload { ServiceString path; uint32_t low, high; ServiceLinks links; };
struct ServiceValue { ServiceKey key; ServicePayload payload; };
struct ServiceNodeBase { ServiceNodeBase* right; ServiceNodeBase* left; ServiceNodeBase* parent; uint8_t color; };
struct ServiceNode { ServiceNodeBase base; ServiceValue value; };
struct ServiceInsertResult { ServiceNodeBase* node; uint8_t inserted; };
struct ServiceTree {
    uint32_t comparator;
    ServiceNodeBase anchor;
    uint32_t count;
    void* allocator;
    uint32_t flags;
    const char* name;
    // TS3W.exe 0x004A5140: removes one node and returns the successor through out.
    ServiceNodeBase** Erase(ServiceNodeBase** out, ServiceNode* node);
    // TS3W.exe 0x004A51A0: allocates and links a copied value at the insertion position.
    ServiceNodeBase** InsertAt(ServiceNodeBase** out, ServiceNodeBase* position, const ServiceValue* value, uint8_t forceLeft);
    // TS3W.exe 0x004A5410: inserts a unique key; preserves result padding.
    ServiceInsertResult* Insert(ServiceInsertResult* out, const ServiceValue* value, uintptr_t unused);
};
struct ResourceMan {
    void* const* vtable;
    void* const* secondaryVtable;
    volatile int32_t references;
    uint8_t active;
    uint8_t padding0D[3];
    void* allocator;
    uint8_t flag20;
    uint8_t padding15[3];
    void* state;
    ServiceString path;
    ServiceTree entryTree;
    ServiceTree recordTree;
    uint32_t generation, unknown78, unknown7C;
    thread::Mutex mutex;
    // TS3W.exe 0x004A5500: initializes the resource manager and optional UTF-16 path.
    ResourceMan* Initialize(const wchar_t* source, void* coreAllocator);
    // TS3W.exe 0x004A5620: releases state, both trees, mutex and path; keeps incidental EAX.
    uintptr_t Destroy();
    // TS3W.exe 0x004A4F10: reloads the DDF text file; signed name-byte parsing is retained.
    uint8_t LoadDefinitions();
    // TS3W.exe 0x004A6050: adds a file-backed record, retaining the temporary cleanup return.
    uintptr_t AddRecord(const ServiceKey* key, const wchar_t* filename, const void* source);
};
// TS3W.exe 0x004A4BA0: copies a tree value and its owning string/list.
ServiceValue* CopyValue(ServiceValue* destination, const ServiceValue* source);
// TS3W.exe 0x004A4CD0: constructs a tree value from separate key and payload.
ServiceValue* MakeValue(ServiceValue* destination, const ServiceKey* key, const ServicePayload* payload);
// TS3W.exe 0x004A6E00: obtains creation/modification timestamps under the package lock.
uint8_t GetTimes(resource::DatabasePackedFile* self, uint32_t* created, uint32_t* modified);
// TS3W.exe 0x004A6F10: checks the signed float free-space ratio under the package lock.
uint8_t ExceedsFreeRatio(resource::DatabasePackedFile* self);
// TS3W.exe 0x004A7030: uses the previously verified codec-choice implementation.
uint8_t ChooseWriteCodec(resource::DatabasePackedFile* self, const void* source, uint32_t size, void** output, uint32_t* outputSize, uint16_t* type);
TS3_X86_SIZE(ServiceString, 16);
TS3_X86_OFFSET(ServiceString, allocator, 12);
TS3_X86_SIZE(ServiceLinks, 12);
TS3_X86_OFFSET(ServiceLinks, count, 8);
TS3_X86_SIZE(ServicePayload, 36);
TS3_X86_OFFSET(ServicePayload, low, 16);
TS3_X86_OFFSET(ServicePayload, high, 20);
TS3_X86_SIZE(ServiceValue, 56);
TS3_X86_SIZE(ServiceNode, 72);
TS3_X86_OFFSET(ServiceNode, value, 16);
TS3_X86_SIZE(ServiceNodeBase, 16);
TS3_X86_OFFSET(ServiceNodeBase, parent, 8);
TS3_X86_OFFSET(ServiceNodeBase, color, 12);
TS3_X86_OFFSET(ServiceValue, payload, 16);
TS3_X86_OFFSET(ServicePayload, links, 24);
TS3_X86_OFFSET(ServiceTree, anchor, 4);
TS3_X86_OFFSET(ServiceTree, count, 20);
TS3_X86_OFFSET(ServiceTree, allocator, 24);
TS3_X86_SIZE(ServiceTree, 36);
TS3_X86_OFFSET(ResourceMan, active, 12);
TS3_X86_OFFSET(ResourceMan, allocator, 16);
TS3_X86_OFFSET(ResourceMan, state, 24);
TS3_X86_OFFSET(ResourceMan, path, 28);
TS3_X86_OFFSET(ResourceMan, entryTree, 44);
TS3_X86_OFFSET(ResourceMan, recordTree, 80);
TS3_X86_OFFSET(ResourceMan, mutex, 128);
TS3_X86_SIZE(ResourceMan, 176);
}
