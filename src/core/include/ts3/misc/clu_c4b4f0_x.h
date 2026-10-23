#pragma once

#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
#define TS3_MISC_FASTCALL __fastcall
#define TS3_MISC_STDCALL __stdcall
#define TS3_MISC_CDECL __cdecl
#else
#define TS3_MISC_FASTCALL
#define TS3_MISC_STDCALL
#define TS3_MISC_CDECL
#endif

namespace ts3::misc {

// Opaque 2076-byte page streams used by the resource hash table.
uint32_t TS3_MISC_CDECL C4B4F0(const uintptr_t* a, const uintptr_t* b);
void* TS3_MISC_FASTCALL C4B540(void* self, void* edx, int32_t delta);
void* TS3_MISC_FASTCALL C4B5C0(void* self, void* edx, void* out, int32_t delta);
void* TS3_MISC_CDECL C4B670(void* begin, void* end, const void* value);
int32_t TS3_MISC_CDECL C4B730(uintptr_t head, const uintptr_t* bucketArray, uintptr_t end);
uint32_t TS3_MISC_FASTCALL C4B9E0(const uintptr_t* self, void* edx);
void* TS3_MISC_FASTCALL C4BA30(void* self, void* edx, void* out, int32_t delta);
void* TS3_MISC_FASTCALL C4BA80(void* self, void* edx, void* out, const uint8_t* key);
int32_t TS3_MISC_FASTCALL C4C1B0(void* self, void* edx, const uint32_t* value);
int32_t TS3_MISC_FASTCALL C4C850(void* self, void* edx, int32_t index);
uintptr_t TS3_MISC_FASTCALL C4C8C0(void* self, void* edx, int32_t index, int32_t field);
uintptr_t TS3_MISC_STDCALL C4D120(uintptr_t node, const uint8_t* key, int32_t unused);
void TS3_MISC_FASTCALL C4D170(void* self, void* edx, uintptr_t* buckets, uint32_t count);
uintptr_t TS3_MISC_STDCALL C4D1B0(uintptr_t node, const uint8_t* key, int32_t unused);
void* TS3_MISC_FASTCALL C4D540(void* self, void* edx, int32_t a, int32_t b);
int32_t TS3_MISC_FASTCALL C4D610(void* self, void* edx);
void* TS3_MISC_FASTCALL C4DDC0(void* self, void* edx, const void* source);
void* TS3_MISC_FASTCALL C4E5B0(void* self, void* edx, void* out, const int8_t* key);
uintptr_t TS3_MISC_STDCALL C4EA80(uintptr_t node, const float* key, int32_t unused);
uint32_t* TS3_MISC_FASTCALL C50200(void* self, void* edx, int32_t id);

} // namespace ts3::misc

#undef TS3_MISC_FASTCALL
#undef TS3_MISC_STDCALL
#undef TS3_MISC_CDECL
