#pragma once
#include "ts3/layout.h"

namespace ts3::scratch {
// Four-word string storage: capacity is a tagged end pointer, not a byte count.
struct ScratchString { void* begin; void* end; void* capacity; const char* label; };
struct ScratchPair { ScratchString first, second; };
struct ScratchTrait { void* value; void* end; void* capacity; uint32_t reserved; ScratchString text; };
struct ScratchOutfit { ScratchPair strings; uint32_t extra[3]; };
TS3_X86_SIZE(ScratchString, 16);
TS3_X86_SIZE(ScratchPair, 32);
TS3_X86_SIZE(ScratchTrait, 32);
TS3_X86_SIZE(ScratchOutfit, 44);
TS3_X86_OFFSET(ScratchString, capacity, 8);
TS3_X86_OFFSET(ScratchTrait, text, 16);

struct ScratchOwnedObject {
    void** vtable;
    uint32_t reserved[4];
    void* allocator;
    // TS3W.exe 0x004B4170: destroy without deleting, then free with the saved allocator.
    uintptr_t Release();
};
TS3_X86_OFFSET(ScratchOwnedObject, allocator, 20);
struct ScratchObjectEntry { uint32_t key; ScratchOwnedObject* object; };
TS3_X86_SIZE(ScratchObjectEntry, 8);
struct ScratchObjects {
    ScratchObjectEntry* begin;
    ScratchObjectEntry* end;
    ScratchObjectEntry* capacity;
    // TS3W.exe 0x009B3630: destroy entries backwards, leaving storage allocated.
    uintptr_t Clear();
    // TS3W.exe 0x009B39F0: clear and release storage; fields are not reset.
    uintptr_t Destroy();
    // TS3W.exe 0x009B52B0: clear, clone each source object and append its key.
    void* Copy(const ScratchObjects* source);
    // TS3W.exe 0x009B5F50: copy wrapper returning this.
    ScratchObjects* Assign(const ScratchObjects* source);
};
TS3_X86_SIZE(ScratchObjects, 12);
TS3_X86_OFFSET(ScratchObjects, end, 4);
TS3_X86_OFFSET(ScratchObjects, capacity, 8);
// TS3W.exe 0x004EFBA0: destroy pairs using signed tagged-capacity thresholds.
uintptr_t __stdcall FreeScratchPairs(ScratchPair* begin, const ScratchPair* end);
// TS3W.exe 0x008A7BB0: destroy narrow strings with signed capacity difference > 1.
uintptr_t __stdcall FreeScratchStrings(ScratchString* begin, const ScratchString* end);
// TS3W.exe 0x009B4280: destroy traits, string first, then unconditional nonnull value.
uintptr_t __stdcall FreeScratchTraits(ScratchTrait* begin, const ScratchTrait* end);
// TS3W.exe 0x009B42D0: destroy outfit pairs with a 44-byte x86 stride.
uintptr_t __stdcall FreeScratchOutfits(ScratchOutfit* begin, const ScratchOutfit* end);
struct ScratchPairs {
    ScratchPair* begin; ScratchPair* end; ScratchPair* capacity; const char* label;
    // TS3W.exe 0x004F0750: reserve pair storage, returning old begin on no growth.
    ScratchPair* Reserve(uint32_t count);
    // TS3W.exe 0x009B48F0: destroy live pairs and reset end to begin.
    uintptr_t Clear();
    // TS3W.exe 0x009A0010: assign pairs, reusing storage where possible.
    ScratchPairs* Assign(const ScratchPairs* source);
};
TS3_X86_SIZE(ScratchPairs, 16);
struct ScratchKey { uint32_t words[4]; };
struct ScratchKeys {
    ScratchKey* begin; ScratchKey* end; ScratchKey* capacity; const char* label;
    // TS3W.exe 0x007DE2C0: assign 16-byte keys, reusing storage where possible.
    ScratchKeys* Assign(const ScratchKeys* source);
};
TS3_X86_SIZE(ScratchKeys, 16);
struct ScratchNamedRecord {
    ScratchString name;
    uint32_t fields[10];
    // TS3W.exe 0x0099CB60: assign name, then copy the ten payload words.
    ScratchNamedRecord* Assign(const ScratchNamedRecord* source);
};
TS3_X86_OFFSET(ScratchNamedRecord, fields, 16);
TS3_X86_SIZE(ScratchNamedRecord, 56);
// TS3W.exe 0x0099D6B0: assign both strings in each pair; exact self-copy is skipped.
ScratchPair* CopyScratchPairs(const ScratchPair* begin, const ScratchPair* end, ScratchPair* destination);
struct ScratchContentRecord {
    uint8_t opaque00[48];
    uint32_t category;
    ScratchKey key;
    ScratchNamedRecord named;
    uint32_t flags;
    uint8_t opaque80[16];
    ScratchPairs pairs90, pairsA0;
    uint8_t opaqueB0[8];
    uint32_t payload[8];
    ScratchKeys keys;
    // TS3W.exe 0x009A0100: assign selected content fields and nested vectors.
    ScratchKey* Assign(const ScratchContentRecord* source);
};
TS3_X86_OFFSET(ScratchContentRecord, key, 52);
TS3_X86_OFFSET(ScratchContentRecord, named, 68);
TS3_X86_OFFSET(ScratchContentRecord, flags, 124);
TS3_X86_OFFSET(ScratchContentRecord, pairs90, 144);
TS3_X86_OFFSET(ScratchContentRecord, pairsA0, 160);
TS3_X86_OFFSET(ScratchContentRecord, payload, 184);
TS3_X86_OFFSET(ScratchContentRecord, keys, 216);
TS3_X86_SIZE(ScratchContentRecord, 232);
struct ScratchVersion {
    uint32_t parts[4];
    // TS3W.exe 0x009B04E0: convert version text, parse, release temporary string.
    ScratchVersion* Parse(const char* text);
};
// TS3W.exe 0x009B0530: cache whether the configured game version is all zero.
uint8_t IsScratchZeroVersion();
// TS3W.exe 0x009B3600: null-safe resource-open virtual dispatch; only AL is meaningful.
uint8_t __stdcall OpenScratchResource(void* resource);

template<class T> struct ScratchVector { T* begin; T* end; T* capacity; const char* label; };
struct ScratchStrings : ScratchVector<ScratchString> {
    // TS3W.exe 0x007685D0: destroy strings, then free their vector storage.
    uintptr_t Destroy();
};
struct ScratchDownloadPart {
    uint8_t opaque00[20];
    void* first; void* firstEnd; void* firstCapacity; uint32_t reserved20;
    void* second; void* secondEnd; void* secondCapacity; uint32_t reserved30;
};
TS3_X86_OFFSET(ScratchDownloadPart, first, 20);
TS3_X86_OFFSET(ScratchDownloadPart, second, 36);
TS3_X86_SIZE(ScratchDownloadPart, 52);
// TS3W.exe 0x009A80A0: reset both ends to begin and release both buffers.
uintptr_t __stdcall FreeScratchDownloadParts(ScratchDownloadPart* begin, const ScratchDownloadPart* end);
struct ScratchMetadata {
    void* initial[3]; uint8_t opaque0C[32]; uint8_t flags2C[10]; uint8_t pad36[2];
    ScratchString expiration, sendToFriend;
    uint8_t flags58[3]; uint8_t opaque5B[14]; uint8_t flag69;
    uint8_t opaque6A[70]; uint8_t flagB0; uint8_t opaqueB1[27];
    uint32_t valueCC; ScratchString lotName; uint32_t valuesE0[2];
    ScratchString householdName; uint8_t opaqueF8[8]; uint32_t value100;
    uint8_t opaque104[16]; uint32_t value114; ScratchString actorName;
    uint32_t values128[2]; uint8_t flag130; uint8_t pad131[3];
    ScratchVector<uint32_t> friends; uint32_t value144; ScratchKey key148;
    uint32_t values158[4];
    ScratchString matCategory, patternType, name, description, bio;
    uint32_t value1B8;
    ScratchVector<uint32_t> traitIds;
    ScratchVector<ScratchTrait> traits;
    ScratchVector<ScratchOutfit> outfits;
    ScratchVector<uint8_t> householdMembers;
    ScratchVector<uint32_t> validMetaTags;
    // TS3W.exe 0x009B4990: clear live metadata vectors without releasing storage.
    uintptr_t Clear();
    // TS3W.exe 0x009B4D30: initialize metadata, preserving all unspecified bytes.
    ScratchMetadata* Initialize();
    // TS3W.exe 0x009B4FA0: clear metadata and release owned vectors and strings.
    uintptr_t Destroy();
};
TS3_X86_SIZE(ScratchMetadata, 524);
TS3_X86_OFFSET(ScratchMetadata, expiration, 56);
TS3_X86_OFFSET(ScratchMetadata, lotName, 208);
TS3_X86_OFFSET(ScratchMetadata, actorName, 280);
TS3_X86_OFFSET(ScratchMetadata, friends, 308);
TS3_X86_OFFSET(ScratchMetadata, key148, 328);
TS3_X86_OFFSET(ScratchMetadata, matCategory, 360);
TS3_X86_OFFSET(ScratchMetadata, traitIds, 444);
TS3_X86_OFFSET(ScratchMetadata, traits, 460);
TS3_X86_OFFSET(ScratchMetadata, outfits, 476);
TS3_X86_OFFSET(ScratchMetadata, householdMembers, 492);
TS3_X86_OFFSET(ScratchMetadata, validMetaTags, 508);
struct ScratchManifest {
    uint32_t words00[2]; ScratchString name, description;
    ScratchKey key; ScratchVector<ScratchDownloadPart> parts;
    uint8_t opaque48[18]; uint16_t version; ScratchPairs pairs5C, pairs6C;
    uint8_t opaque7C[16]; ScratchObjects objects; uint32_t reserved98[2];
    ScratchString textA0[8]; ScratchStrings texts120, texts130, texts140;
    ScratchMetadata metadata;
    // TS3W.exe 0x009B5590: reverse-destroy live parts/strings, then release all storage.
    uintptr_t Destroy();
};
TS3_X86_OFFSET(ScratchManifest, parts, 56);
TS3_X86_OFFSET(ScratchManifest, pairs5C, 92);
TS3_X86_OFFSET(ScratchManifest, objects, 140);
TS3_X86_OFFSET(ScratchManifest, textA0, 160);
TS3_X86_OFFSET(ScratchManifest, texts120, 288);
TS3_X86_OFFSET(ScratchManifest, metadata, 336);
struct ScratchPendingKeys {
    uint8_t opaque00[48]; ScratchKey* resourceBegin; ScratchKey* resourceEnd; uint8_t opaque38[8]; ScratchKeys pending;
    // TS3W.exe 0x00988890: append a 16-byte key, growing the vector when full.
    void* Append(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
};
TS3_X86_OFFSET(ScratchPendingKeys, pending, 64);
struct ScratchKeyState {
    uint32_t words00[2]; uint32_t state; ScratchKey key;
    // TS3W.exe 0x00A3C120: copy the key and transition state 3 to 4; no defined return otherwise.
    ScratchKey* Complete(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
};
TS3_X86_OFFSET(ScratchKeyState, state, 8);
TS3_X86_OFFSET(ScratchKeyState, key, 12);
struct ScratchDownloadItem {
    void* vtable; uint32_t reserved04; ScratchKey key;
    void* value18; uint32_t values1C[3];
    ScratchString path; uint16_t inlinePath[256];
    uint8_t dependency238[1040]; ScratchMetadata metadata;
    uint32_t opaque854; uint32_t value858, value85C; uint8_t flag860; uint8_t pad861[3]; uint32_t value864;
    // TS3W.exe 0x009C79B0: initialize an item with inline path and nested metadata.
    ScratchDownloadItem* Initialize();
    // TS3W.exe 0x009C8460: scalar deleting destructor; only flags bit 0 controls freeing.
    ScratchDownloadItem* Delete(uint32_t flags);
};
TS3_X86_OFFSET(ScratchDownloadItem, path, 40);
TS3_X86_OFFSET(ScratchDownloadItem, inlinePath, 56);
TS3_X86_OFFSET(ScratchDownloadItem, dependency238, 568);
TS3_X86_OFFSET(ScratchDownloadItem, metadata, 1608);
TS3_X86_OFFSET(ScratchDownloadItem, value858, 2136);
TS3_X86_SIZE(ScratchDownloadItem, 2152);
struct ScratchDownloadItems : ScratchVector<ScratchDownloadItem> {
    // TS3W.exe 0x009C85D0: append a freshly initialized item, growing if necessary.
    uintptr_t Append();
};
struct ScratchDownloadQueue {
    uint32_t reserved00; ScratchDownloadItems items;
    // TS3W.exe 0x009C8C80: append an item, process input, and set item flags.
    uintptr_t Append(void* input, uint8_t flag, uint32_t value);
};
TS3_X86_OFFSET(ScratchDownloadQueue, items, 4);
struct ScratchCacheNode { ScratchKey key; void* value; };
TS3_X86_OFFSET(ScratchCacheNode, value, 16);
struct ScratchCacheTable { void* reserved; ScratchCacheNode** buckets; uint32_t count; };
struct ScratchImportContext : ScratchPendingKeys { uint8_t opaque50[76]; uint8_t flag9C; };
TS3_X86_OFFSET(ScratchImportContext, flag9C, 156);
struct ScratchContentManager {
    uint8_t opaque00[12]; void* handlers;
    uint8_t opaque10[200]; uint32_t state;
    uint8_t opaqueDC[1188]; uint8_t mutex[24]; uint8_t opaque598[136];
    ScratchCacheTable cache; uint8_t opaque62C[56]; void* special;
    // TS3W.exe 0x0098B9A0: lookup cached content under mutex, falling back to installed providers.
    uint8_t Lookup(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint16_t* version);
    // TS3W.exe 0x0098F630: import content, dispatching category/provider handlers and preserving status quirks.
    int32_t Import(void* resource, ScratchKey* key, ScratchImportContext* context);
};
TS3_X86_OFFSET(ScratchContentManager, state, 216);
TS3_X86_OFFSET(ScratchContentManager, mutex, 1408);
TS3_X86_OFFSET(ScratchContentManager, cache, 1568);
TS3_X86_OFFSET(ScratchContentManager, special, 1636);
}
