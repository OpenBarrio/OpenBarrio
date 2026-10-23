#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b45 {
struct Variant { void* table; uint32_t reserved; uint16_t flags,type; uint32_t pad; union { void* indirect; uint32_t word; }; uint8_t tail[12]; };
struct Property { uint32_t key,pad; Variant value; };
struct Link { Link* next; Link* previous; };
struct Group {
    void* table; int32_t refs; void* base[8]; void* secondary;
    Link* previous; Link* next; uint32_t hash; uint8_t active; uint8_t padding[3];
    Property* begin; Property* end; Property* capacity; const char* label;
    uint32_t reserved4c; void* owner; Link sentinel; uint32_t reserved;
    Variant first,second; void* groups; void* groupsEnd; void* groupsCapacity; const char* groupsLabel;
};
struct String { char* begin; char* end; char* capacity; const char* label; };
struct WideString { uint16_t* begin; uint16_t* end; uint16_t* capacity; const char* label; };
struct Configuration {
    void* table; int32_t refs; uint8_t mutex[0x30]; void* services; void* allocator;
    uint32_t mapReserved; void* root; void* left; void* right; uint8_t flag; uint8_t pad[3]; uint32_t count; const char* label;
};
TS3_X86_SIZE(Variant,0x20); TS3_X86_SIZE(Property,0x28);
TS3_X86_OFFSET(Group,owner,0x50); TS3_X86_OFFSET(Group,first,0x60); TS3_X86_SIZE(Group,0xB0);
TS3_X86_OFFSET(Configuration,services,0x38); TS3_X86_OFFSET(Configuration,count,0x54);
// TS3W.exe 0x008A7900: Lower-bound search over unsigned property keys.
Property* __cdecl LowerBound(Property*,Property*,const uint32_t*);
// TS3W.exe 0x008A7950: Copy-construct a property range and advance the destination cursor.
Property** __cdecl CopyProperties(Property**,Property*,Property*,Property*);
// TS3W.exe 0x008A7A50: Obtain the resource name and hash it, freeing temporary string storage.
uint32_t __fastcall NameHash(void*,void*,void*,void*);
// TS3W.exe 0x008A7AE0: Unlink and destroy a resource group and its owned containers.
void __fastcall DestroyGroup(Group*,void*);
// TS3W.exe 0x008A7BF0: Construct an empty group, leaving unused payload bytes untouched.
Group* __fastcall ConstructGroup(Group*,void*);
// TS3W.exe 0x008A7CA0: Destroy and optionally free a group; returns the original pointer.
Group* __fastcall DeleteGroup(Group*,void*,uint32_t);
// TS3W.exe 0x008A7CC0: Read type-19 property data, honoring indirect storage and AL-only lookup success.
uint32_t __fastcall Get19(void*,void*,uint32_t);
// TS3W.exe 0x008A7D00: Read type-18 property data, honoring indirect storage and AL-only lookup success.
uint32_t __fastcall Get18(void*,void*,uint32_t);
// TS3W.exe 0x008A7D40: Read a big-endian byte-string length then its payload; preserves stale upper EAX.
uint32_t __cdecl ReadString(void*,String*);
// TS3W.exe 0x008A7DB0: Read a big-endian wide-string length then raw UTF16 bytes; preserves stale upper EAX.
uint32_t __cdecl ReadWideString(void*,WideString*);
// TS3W.exe 0x008A7E20: Construct configuration, allocator and service registration in observable order.
Configuration* __fastcall ConstructConfiguration(Configuration*,void*);
// TS3W.exe 0x008A7F50: Release configuration state and optionally free the object.
Configuration* __fastcall DeleteConfiguration(Configuration*,void*,uint32_t);
}
