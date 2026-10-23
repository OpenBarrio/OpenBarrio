// Unit clu_401260_SIGS (SIGS): the async loader's decompress-job entry helpers.
// See the header for the 0x00727950 re-export decision.
#include "ts3/sigs/clu_401260_SIGS.h"

#include "ts3/layout.h"
#include "ts3/thread/mutex.h"

namespace ts3::sigs {

namespace {

// The game's global ThreadTime at 0x00FB217C is 0xFFFFFFFF ("wait forever"); EA's
// Mutex::Lock only reads the value behind the pointer, so the x64 build uses the rewritten
// mutex with the same constant. Kept EA's actual pointer on x86 for byte-exact behavior.
// Kept out of MutexGuardCtor so no mutation site hides behind the preprocessor.
constexpr thread::ThreadTime kTimeoutNone = 0xFFFFFFFFu;

void LockGuardMutex(void* mutex)
{
#if defined(_M_IX86)
    const auto* timeout = reinterpret_cast<const thread::ThreadTime*>(static_cast<uintptr_t>(0x00FB217Cu));
    CallGameMethod<int, 0x004E16F0>(mutex, timeout);  // EA::Thread::Mutex::Lock(mutex, timeout)
#else
    static_cast<thread::Mutex*>(mutex)->Lock(kTimeoutNone);
#endif
}

template <class T> T Field(void* base, std::size_t offset)
{
    return *reinterpret_cast<T*>(static_cast<uint8_t*>(base) + offset);
}

}  // namespace

// TS3W.exe 0x004A3870, 25 bytes, __thiscall (this in ecx, one stack argument, ret 4).
void* MutexGuardCtor(void* self, void* mutex)
{
    // 0x004A3877
    *static_cast<void**>(self) = mutex;
    LockGuardMutex(mutex);
    return self;  // 0x004A3883: eax = esi
}

// TS3W.exe 0x00401260, 37 bytes, __thiscall (this in ecx, no stack arguments, ret 0).
void* AsyncIoCancel(void* self)
{
    // 0x00401263: the read object, once it exists, is returned as-is (nothing is cancelled).
    void* read = Field<void*>(self, 4);
    if (read)
        return read;
    // 0x0040126A: EA dereferences self+0x40 with no null check.
    uint8_t* holder = Field<uint8_t*>(self, 0x40);
    void* target = holder;
    if (holder[6]) {  // 0x0040126D
        void* singleton = CallGameFunction<void*, 0x00402160>();
        target = Field<void*>(singleton, 0x28);  // 0x00401278
    }
    // 0x0040127B
    return CallGameMethod<void*, 0x00401810>(target, self);
}

// TS3W.exe 0x00727950: already implemented and verified in the resource unit.
bool DecompressJob(resource::DecompressJobHost* host, const void* taskContext, uint32_t state)
{
    return resource::DecompressJob(host, taskContext, state);
}

}  // namespace ts3::sigs