// Frozen M01 views. Native pointer fields deliberately widen on x64; image addresses
// and slot offsets below describe TS3W.exe, not serialized native structures.
#pragma once
#include "ts3/io/memory_stream.h"
#include "ts3/io/file_stream.h"

namespace ts3::io::m01 {
struct MemoryStreamLayout {
    MemoryStream stream;
    int32_t option16; // SetOption(16), x86 +28; allocation is 0x2c, not 0x28.
};
TS3_X86_OFFSET(MemoryStream, vtable, 0);
TS3_X86_OFFSET(MemoryStream, buffer, 4);
TS3_X86_OFFSET(MemoryStream, name, 8);
TS3_X86_OFFSET(MemoryStream, refCount, 0xc);
TS3_X86_OFFSET(MemoryStream, capacity, 0x14);
TS3_X86_OFFSET(MemoryStream, growthFactor, 0x20);
TS3_X86_OFFSET(MemoryStreamLayout, option16, 0x28);
TS3_X86_SIZE(MemoryStreamLayout, 0x2c);
TS3_X86_OFFSET(SharedBuffer, allocator, 4);
TS3_X86_OFFSET(SharedBuffer, refCount, 12);
TS3_X86_OFFSET(SharedBuffer, ownsData, 16);
struct BindingObject { void* const* vtable; };
struct BindingEntry { void* function; const char* name; }; // NOT name/function!
TS3_X86_SIZE(BindingObject, 4);
TS3_X86_OFFSET(BindingEntry, name, 4);
inline constexpr uintptr_t kMemoryVtable = 0x00FB5BCC, kFileVtable = 0x00FB8300;
inline constexpr uintptr_t kBindingVtable = 0x0100C73C, kBindingTable = 0x01161898;
inline constexpr uintptr_t kCacheHandle = 0x011D6838, kCacheStream = 0x011D683C;
inline constexpr uintptr_t kScriptingSingleton = 0x011D4BBC;
inline constexpr uint32_t kHandleTag = 0x60;
namespace entry {
inline constexpr uintptr_t WriteInt=0x774E90, WriteInt64=0x774F30, WriteFloat=0x774FC0,
    WriteDouble=0x775040, WriteDecimal=0x7750C0, WriteByteArray=0x775160,
    WriteCharUTF8=0x775230, WriteCharArrayUTF8=0x775CE0, WriteStringUTF8=0x775E10,
    ReadInt=0x7752C0, ReadInt64=0x775350, ReadFloat=0x7753E0, ReadDouble=0x775470,
    ReadDecimal=0x775510, ReadCharUTF8=0x7755C0, ReadStringUTF8=0x776180,
    ReadByteArray=0x7756D0, ReadCharArrayUTF8=0x776420, Create=0x7757B0,
    CreateFromBuffer=0x775830, Close=0x775910, CloseTarget=0x774E30,
    Flush=0xC0FD60, GetSize=0x775920, GetPosition=0x775990, SetPosition=0x775A00,
    CopyStream=0x775A80, ToArray=0x775BA0;
}
// Binding entry points are cdecl; class originals use ECX. Fastcall adapters
// reserve EDX and preserve the original callee-pop stack arguments on x86.
using Factory = uint32_t(__cdecl*)();
using HandleEntry = int32_t(__cdecl*)(uint32_t);
using Read = size_type(__fastcall*)(void*, void*, void*, size_type);
using Seek = uint8_t(__fastcall*)(void*, void*, off_type, PositionType);
using Position = off_type(__fastcall*)(void*, void*, PositionType);
namespace service {
// Derived from 774E00/774E30, 775830/775BA0, 775CE0/775E10/776180.
inline constexpr unsigned RegisterBindings=18, StringData=22, StringLength=23,
    NewString=24, Buffer=28, CharBuffer=31, NewArray=32, ReleaseManaged=37,
    RegisterHandle=50, Lookup=51, Remove=54;
// 775CA0 returns directly after slot 18: the fixture supplies deterministic
// EAX for that inherited return (no invented bool/void semantic contract).
using RegisterBindingsFn = uint32_t(__fastcall*)(void*, void*, BindingObject*);
using Register = uint32_t(__fastcall*)(void*, void*, void*, uint32_t tag);
using LookupFn = void*(__fastcall*)(void*, void*, uint32_t handle);
using BufferFn = void(__fastcall*)(void*, void*, uint32_t managed, void** data, int32_t* length);
using NewArrayFn = void(__fastcall*)(void*, void*, uint32_t* result, int32_t length);
using NewStringFn = uint32_t(__fastcall*)(void*, void*, const wchar_t*, int32_t length);
using StringDataFn = const wchar_t*(__fastcall*)(void*, void*, uint32_t managed);
// 775E5F pre-pushes the converter's flags=0, NOT a second slot-23
// argument. Slot 23 consumes only the managed handle (ret 4 on x86).
using StringLengthFn = int32_t(__fastcall*)(void*, void*, uint32_t managed);
using ReleaseFn = void(__fastcall*)(void*, void*, uint32_t managed);
}
struct CacheSnapshot { uint32_t handle; void* stream; };
// Explicit cells make the same API usable by native fixtures, without truncating pointers.
inline CacheSnapshot SnapshotCache(uint32_t& handle, void*& stream) { return {handle, stream}; }
inline void RestoreCache(uint32_t& handle, void*& stream, CacheSnapshot saved) {
    handle=saved.handle; stream=saved.stream;
}
inline void ResetCache(uint32_t& handle, void*& stream) { RestoreCache(handle, stream, {0,nullptr}); }
} // namespace ts3::io::m01
