#pragma once
#include "ts3/resource/compressed_index.h"
#include "ts3/layout.h"
namespace ts3::resourcesystem {
using resource::ResourceKey;
struct CopyRecordInfo { uint32_t offset, size, memorySize, compression; };
struct CopyKeyList { ResourceKey* begin; ResourceKey* end; ResourceKey* capacity; const char* tag; };
TS3_X86_SIZE(CopyRecordInfo, 16);
TS3_X86_OFFSET(CopyKeyList, end, 4);
TS3_X86_OFFSET(CopyKeyList, capacity, 8);
TS3_X86_OFFSET(CopyKeyList, tag, 12);
TS3_X86_SIZE(CopyKeyList, 16);
// All twelve original functions are cdecl. The five record helpers take five args;
// batches and the public dispatcher take six. Database/mapper interfaces remain opaque.
bool CopyRecordMemoryToFile(void*, void*, const ResourceKey*, bool, void*);
bool CopyRecordStreamToFile(void*, void*, const ResourceKey*, bool, void*);
bool CopyRecordFileToFile(void*, void*, const ResourceKey*, bool, void*);
bool CopyRecordFileToMemory(void*, void*, const ResourceKey*, bool, void*);
bool CopyRecordStreamToMemory(void*, void*, const ResourceKey*, bool, void*);
uint32_t CopyMemoryToFile(void*, void*, void*, bool, void*, uint32_t*);
uint32_t CopyStreamToFile(void*, void*, void*, bool, void*, uint32_t*);
uint32_t CopyFileToFile(void*, void*, void*, bool, void*, uint32_t*);
uint32_t CopyFileToMemory(void*, void*, void*, bool, void*, uint32_t*);
uint32_t CopyMemoryToMemory(void*, void*, void*, bool, void*, uint32_t*);
uint32_t CopyStreamToMemory(void*, void*, void*, bool, void*, uint32_t*);
// 0x007D8F00: destination, source, enumeration filter, delete-after-success, key mapper,
// optional failed-record count. Null/unsupported inputs leave the failure count alone.
uint32_t CopyDatabaseRecords(void*, void*, void*, bool, void*, uint32_t*);
}
