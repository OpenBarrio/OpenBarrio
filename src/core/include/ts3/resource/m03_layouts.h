// Canonical owners only: no copied/guessed resource layouts. Additional recovered
// layouts remain a blocker to freezing this public API. No new game functions.
#pragma once
#include "ts3/resource/compressed_index.h"
#include "ts3/resource/databases.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resource/packed_record.h"
#include "ts3/resource/packed_stream.h"
#include "ts3/allocator/core_allocator.h"
#include "ts3/thread/mutex.h"
#include "ts3/resourcesystem/clu_727290_ResourceSystem.h"

namespace ts3::resource::m03 {
// Evidence: 004ADE60 record key +10, stream +28; canonical packed_record.h
// owns the native-pointer fields and x86 offset checks. Never use uint32_t as
// pointer storage when constructing independent native worlds.
using Key = ts3::resource::ResourceKey;
using Allocator = ts3::allocator::ICoreAllocator;
using Mutex = ts3::thread::Mutex;
using PackageRecord = ts3::resource::PackedFileRecord;
using DirectoryManager = ts3::resourcesystem::DirectoryManager;
using ResourceCache = ts3::resourcesystem::ResourceCache;
using AsyncContainer = ts3::resourcesystem::ResourceDirContainer;
// 00726270 returns a 16-byte array element (shl eax,4); 00726480
// passes it to record slot5 then copies all four words to ChildResource key+8.
// 00726430 returns the same type, with its independent fallback at 011D3C70.
// These are ResourceKey values, NOT Context objects or native pointer words.
using ContextFallbackKey = Key;
static_assert(sizeof(ContextFallbackKey)==16);
// Resource slot +28 in AsyncFactory stores pointer | needsChildStream. 007260F0
// masks bit0 before Release; 00726480 clears it BEFORE child allocation, including
// failure paths. Encoding/decoding never transfers or frees ownership.
template<class T> class TaggedResourcePointer {
    uintptr_t bits_=0;
public:
    TaggedResourcePointer(T* pointer,bool needsChildStream)
        :bits_(reinterpret_cast<uintptr_t>(pointer) | uintptr_t(needsChildStream)) {}
    T* Get() const { return reinterpret_cast<T*>(bits_ & ~uintptr_t(1)); }
    bool NeedsChildStream() const { return (bits_ & 1)!=0; }
    void ClearChildTag() { bits_ &= ~uintptr_t(1); }
};
// Bounded PRIMARY PREFIX only, not an allocation-size declaration. 004B33BC,
// 0x004B33C2
// separately proven subobject/this-adjust views. The x86 complete object spans
// 0x1D0; never allocate/copy it using sizeof(this partial prefix).
struct ResourceManager {
    void* const* vtable;
    uint8_t initialized;
    uint8_t reserved05[3];
    Allocator* allocator;
};
TS3_X86_OFFSET(ResourceManager,initialized,4);
TS3_X86_OFFSET(ResourceManager,allocator,8);
}
