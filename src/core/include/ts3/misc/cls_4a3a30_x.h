#pragma once
#include "ts3/services/cls_4a4ba0_Services.h"

namespace ts3::misc {
// Native-pointer counterpart of the 176-byte x86 ResourceMan. The provider and
// callback slots are pointers (not the opaque dwords in the older services view).
struct ResourceManConfig {
    void* const* vtable;
    void* const* secondaryVtable;
    volatile int32_t references;
    uint8_t active, padding0D[3];
    void* allocator;
    uint8_t flag20, padding15[3];
    void* state;
    services::ServiceString path;
    services::ServiceTree entryTree, recordTree;
    uint32_t generation;
    void* provider;
    uintptr_t unknown7C;
    thread::Mutex mutex;
    // TS3W.exe 0x004A3A30: stores and returns the entry callback.
    uintptr_t SetEntryCallback(uintptr_t callback);
    // TS3W.exe 0x004A3BD0: obtains a named record's 64-bit file timestamp.
    uint64_t FileTime(const services::ServiceKey* key);
    // TS3W.exe 0x004A3C50: resolves a filename through the configured callback.
    uint8_t ResolveFile(const wchar_t* source, void* output);
    // TS3W.exe 0x004A3EA0: looks up a DDF name or parses a hexadecimal type.
    uint32_t TypeForName(const wchar_t* name);
    // TS3W.exe 0x004A3F50: returns the name associated with a DDF type.
    const wchar_t* NameForType(uint32_t type);
    // TS3W.exe 0x004A3FB0: registers/unregisters a provider, forwarding existing DDF names.
    uint8_t BindProvider(uint32_t add, void* value, uint32_t notify);
    // TS3W.exe 0x004A46D0: copies the path of an indexed record without bounds checking.
    uint8_t CopyRecordPath(const services::ServiceKey* key, wchar_t* out);
    // TS3W.exe 0x004A48D0: removes a record from the indexed DDF list.
    uint8_t RemoveRecord(void* record);
    // TS3W.exe 0x004A4DD0: registers a nonempty DDF name, stripping one leading dot.
    uint8_t RegisterType(const wchar_t* name, uint32_t type);
    // TS3W.exe 0x004A52A0: deletes an unused indexed file and erases its entry.
    uint8_t DeleteRecord(const services::ServiceKey* key);
    // TS3W.exe 0x004A5340: reloads definitions and scans the directory for entries.
    int32_t Scan(void* keys);
    // TS3W.exe 0x004A6100: updates an indexed file and optionally appends its resource key.
    int32_t UpdateFile(const wchar_t* source, void* keys);
};
TS3_X86_OFFSET(ResourceManConfig, provider, 0x78);
TS3_X86_OFFSET(ResourceManConfig, unknown7C, 0x7C);
TS3_X86_OFFSET(ResourceManConfig, mutex, 0x80);
TS3_X86_SIZE(ResourceManConfig, 176);
// TS3W.exe 0x004A84F0: secondary-interface deleting-destructor adjustor (ret 4).
void* DeletePackedSecondary(void* secondary, uint32_t flags);
}
