// Rewrite of cluster unit clu_4a9b20_Scratch (Scratch download/export bookkeeping and the
// XML spreadsheet importer). Every entry point keeps the original TS3W.exe ABI (thiscall
// with callee cleanup, cdecl or stdcall); dependencies that are not rewritten go through
// ts3/layout.h, the two already-verified Scratch helpers are called directly.
#pragma once

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/scratch/clu_4b4170_Scratch.h"  // ScratchString/ScratchKey/ScratchPairs/ScratchKeys/ScratchContentRecord
#include "ts3/scratch/clu_4e3f70_Scratch.h"  // Free4E3F70

namespace ts3::scratch {

// ---------------------------------------------------------------------------------------
// 0x004A9B20 (this). The loader object owns a CRITICAL_SECTION at +0x270; the run is
// guarded by the byte at +0x38D and only happens while +0x14 is non-null.
struct ScratchLoaderHost {
    uint8_t reserved00[0x14];
    void* pendingWork;             // +0x14 gate
    uint8_t reserved18[0x270 - 0x18];
    uint8_t lock270[24];           // +0x270 CRITICAL_SECTION
    uint8_t reserved288[0x38D - 0x288];
    uint8_t running38D;            // +0x38D re-entrancy byte
    // 0x004A9B20, __thiscall, ret 0; returns the 4A9150 result or 0 when it did not run.
    uint8_t RunLocked();
};
TS3_X86_OFFSET(ScratchLoaderHost, pendingWork, 0x14);
TS3_X86_OFFSET(ScratchLoaderHost, lock270, 0x270);
TS3_X86_OFFSET(ScratchLoaderHost, running38D, 0x38D);

// ---------------------------------------------------------------------------------------
// 0x004AE820: bounded search of a byte span inside another span. Reads only, so this is the
// unit's only compare-safe entry point.
struct ScratchByteRange {
    void* begin;  // +0x00
    void* end;    // +0x04
    // 0x004AE820, __thiscall, ret 8. EA quirk: the length check is unsigned (offset+size
    // wraps), and a match ending exactly at end counts only when the needle is empty.
    int32_t FindFrom(const ScratchByteRange& needle, uint32_t offset) const;
    // 0x00876260, __thiscall, ret 0xC: copy min(capacity-left, count) bytes of `source`
    // over this span at `offset` via 0x004D2290; returns that call's result.
    int32_t OverwriteFrom(uint32_t offset, uint32_t count, const ScratchByteRange& source) const;
};
TS3_X86_SIZE(ScratchByteRange, 8);

// ---------------------------------------------------------------------------------------
// 0x005E6A30 / 0x005E7260 / 0x009A0180: the 232-byte download/export content record.
// Layout matches ScratchContentRecord (clu_4b4170_Scratch.h) exactly - 0x009A0100 is that
// record's verified Assign; this unit adds the two constructors and the destructor.
struct ScratchXmlRecord {
    uint32_t key[4];                 // +0x00  16-byte identity copied by the constructors
    ScratchString name10;            // +0x10  inline, cap differences of 1
    ScratchString desc20;            // +0x20  inline, cap differences of 1
    uint32_t category;               // +0x30
    ScratchString transient34;       // +0x34  built by 0x00500A40(this+0x34, 0)
    ScratchNamedRecord named;        // +0x44  label Scratch/DownloadContent/PresetStr
    uint32_t flags7C;                // +0x7C
    ScratchString path80;            // +0x80  label Scratch/DownloadContent/WorldFilePath
    ScratchPairs pairs90;            // +0x90  label Scratch/DownloadContent/NameList
    ScratchPairs pairsA0;            // +0xA0  label Scratch/DownloadContent/DescList
    uint8_t flagB0;                  // +0xB0
    uint8_t tailB1[3];               // +0xB1
    uint32_t fieldB4;                // +0xB4  never written by this unit's constructors
    uint32_t payloadB8[8];           // +0xB8  payloadB8[0] is a state word, [6] a pointer
    ScratchKeys keys;                // +0xD8  label Scratch/DownloadContent/ExportContentAddPackageIds
    // 0x005E6A30, __thiscall ctor, ret 4: identity from `key`, everything else preset.
    ScratchXmlRecord* InitFromKey(const uint32_t* key);
    // 0x009A0180, __thiscall ctor, ret 8: identity from `key` plus the source's copied
    // fields (+0x10/+0x20 strings, +0xB0 flag, +0x80.. via 0x00408EB0) and 0x009A0100.
    ScratchXmlRecord* InitCopy(const uint32_t* key, const ScratchXmlRecord& source);
    // 0x005E7260, __thiscall, ret 0: release every owned block; EAX contract = last free
    // result, or the value of the +0x10 begin pointer when the last test does not free.
    uintptr_t Destroy();
};
TS3_X86_OFFSET(ScratchXmlRecord, name10, 0x10);
TS3_X86_OFFSET(ScratchXmlRecord, desc20, 0x20);
TS3_X86_OFFSET(ScratchXmlRecord, category, 0x30);
TS3_X86_OFFSET(ScratchXmlRecord, transient34, 0x34);
TS3_X86_OFFSET(ScratchXmlRecord, named, 0x44);
TS3_X86_OFFSET(ScratchXmlRecord, flags7C, 0x7C);
TS3_X86_OFFSET(ScratchXmlRecord, path80, 0x80);
TS3_X86_OFFSET(ScratchXmlRecord, pairs90, 0x90);
TS3_X86_OFFSET(ScratchXmlRecord, pairsA0, 0xA0);
TS3_X86_OFFSET(ScratchXmlRecord, flagB0, 0xB0);
TS3_X86_OFFSET(ScratchXmlRecord, payloadB8, 0xB8);
TS3_X86_OFFSET(ScratchXmlRecord, keys, 0xD8);
TS3_X86_SIZE(ScratchXmlRecord, 232);

// ---------------------------------------------------------------------------------------
// 0x005E5550 / 0x005E5890: the 72-byte scratch download/export query descriptor.
struct ScratchExportDesc {
    uint32_t key[4];      // +0x00 zeroed by the ctor
    uint8_t flag10;       // +0x10 first ctor byte
    uint8_t flag11;       // +0x11 second ctor byte
    uint8_t pad12[2];     // +0x12
    ScratchString text14; // +0x14 label Scratch/DownloadContent/mOutputFilename
    ScratchString text24; // +0x24 label Scratch/DownloadContent/mExportDir
    ScratchString text34; // +0x34 built by 0x00500A40(this+0x34, 0)
    uint32_t tail44;      // +0x44
    // 0x005E5550, __thiscall ctor, ret 8: set both bytes, the two tagged inline strings and
    // the 0x00500A40 string; returns this.
    ScratchExportDesc* Construct(char a2, char a3);
    // 0x005E5890, __thiscall, ret 0: free the two tagged strings, largest difference first.
    uintptr_t Destroy();
};
TS3_X86_OFFSET(ScratchExportDesc, flag10, 0x10);
TS3_X86_OFFSET(ScratchExportDesc, flag11, 0x11);
TS3_X86_OFFSET(ScratchExportDesc, text14, 0x14);
TS3_X86_OFFSET(ScratchExportDesc, text24, 0x24);
TS3_X86_OFFSET(ScratchExportDesc, text34, 0x34);
TS3_X86_OFFSET(ScratchExportDesc, tail44, 0x44);
TS3_X86_SIZE(ScratchExportDesc, 72);

// ---------------------------------------------------------------------------------------
// 0x00985590 (this): release three ScratchStrings. EAX = last free result or the +0x00 begin.
struct ScratchStringTriple {
    ScratchString a, b, c;  // +0x00 / +0x10 / +0x20, each freed when cap-begin > 1
    uintptr_t Destroy();
};
TS3_X86_SIZE(ScratchStringTriple, 48);

// ---------------------------------------------------------------------------------------
// 0x0099E4B0 (this): append one two-word key to the vector at +0x8C, growing through
// 0x006202E0 when it is full. Returns the written element, 0 for an empty vector, or the
// grow call's result.
struct ScratchWordPair {
    uint32_t a, b;
};
TS3_X86_SIZE(ScratchWordPair, 8);
struct ScratchPairListHost {
    uint8_t reserved00[0x8C];
    ScratchWordPair* keyBegin;    // +0x8C
    ScratchWordPair* keyEnd;      // +0x90
    ScratchWordPair* keyCapacity; // +0x94
    const char* keyLabel;         // +0x98
    ScratchWordPair* AppendKey(uint32_t a, uint32_t b);
};
TS3_X86_OFFSET(ScratchPairListHost, keyBegin, 0x8C);
TS3_X86_OFFSET(ScratchPairListHost, keyEnd, 0x90);
TS3_X86_OFFSET(ScratchPairListHost, keyCapacity, 0x94);
TS3_X86_OFFSET(ScratchPairListHost, keyLabel, 0x98);

// ---------------------------------------------------------------------------------------
// 0x004B4880 (this = the intrusive list head): destroy every 56-byte node. The allocator
// used for the nodes is at this+0x08 (inside the owning object), each node has its own two
// (begin,end,allocator) triples at +0x08 and +0x20.
struct ScratchListRecord {
    ScratchListRecord* next;  // +0x00
    uint32_t pad04;
    uint32_t block08;       // +0x08
    uint32_t pad0C;
    uint32_t end10;         // +0x10
    void* allocator14;      // +0x14
    uint32_t pad18[2];
    uint32_t block20;       // +0x20
    uint32_t pad24;
    uint32_t end28;         // +0x28
    void* allocator2C;      // +0x2C
    uint32_t pad30[2];
};
TS3_X86_SIZE(ScratchListRecord, 56);

struct ScratchListHead {
    ScratchListHead* next;     // +0x00 sentinel self-link
    ScratchListHead* prev;     // +0x04
    void* nodeAllocator;       // +0x08 = owning object +0x40
    // 0x004B4880, __thiscall, ret 0; no return contract (callers ignore EAX).
    void DestroyNodes();
};
TS3_X86_OFFSET(ScratchListHead, nodeAllocator, 0x08);

// ---------------------------------------------------------------------------------------
// 0x004B4560 / 0x004B4AF0 / 0x004B4CD0 / the deleting destructors: the hash-map-like
// container. Blocks are (begin[,tag], capacityEnd, allocator) triples whose allocator's
// vtable slot 3 is the free.
struct ScratchContainer {
    void* vtable;            // +0x00 0x00FB3018
    uint32_t field04;
    uint32_t block08;        // +0x08  begin
    uint32_t pad0C;
    uint32_t end10;          // +0x10  capacity end
    void* allocator14;       // +0x14
    uint32_t pad18[3];
    void* list24;            // +0x24  intrusive list sentinel
    void* list28;            // +0x28
    void* allocator2C;       // +0x2C
    uint32_t pad30[2];
    uint32_t block38;        // +0x38
    uint32_t pad3C;
    uint32_t end40;          // +0x40
    void* allocator44;       // +0x44
    uint32_t pad48[2];
    uint32_t block50;        // +0x50
    uint32_t pad54;
    uint32_t end58;          // +0x58
    void* allocator5C;       // +0x5C
    uint32_t pad60[2];
    uint32_t block68;        // +0x68
    uint32_t pad6C;
    uint32_t end70;          // +0x70
    void* allocator74;       // +0x74
    uint32_t pad78[2];
    // 0x004B4560, __thiscall, ret 0; EAX = last free result or end10-block08.
    uintptr_t Destroy4B4560();
    // 0x004B4AF0, __thiscall, ret 0: free the +0x38/+0x50/+0x68 blocks, then Destroy4B4560.
    uintptr_t Destroy4B4AF0();
    // 0x004B4CD0, __thiscall, ret 0: 0x004B4290(this), destroy the list at +0x38, self-link it.
    void Reset4B4CD0();
    // 0x004B4B60, __thiscall deleting dtor, ret 4: Destroy4B4AF0, free(this) iff flags&1.
    ScratchContainer* Delete4B4B60(uint32_t flags);
    // 0x004B4C00, __thiscall deleting dtor, ret 4: free [block38,end40) through allocator44,
    // Destroy4B4560, free(this) iff flags&1.
    ScratchContainer* Delete4B4C00(uint32_t flags);
    // 0x004B4F80, __thiscall deleting dtor, ret 4: destroy the +0x38 node list, then
    // Destroy4B4560, free(this) iff flags&1.
    ScratchContainer* Delete4B4F80(uint32_t flags);
};
TS3_X86_OFFSET(ScratchContainer, block08, 0x08);
TS3_X86_OFFSET(ScratchContainer, end10, 0x10);
TS3_X86_OFFSET(ScratchContainer, allocator14, 0x14);
TS3_X86_OFFSET(ScratchContainer, list24, 0x24);
TS3_X86_OFFSET(ScratchContainer, allocator2C, 0x2C);
TS3_X86_OFFSET(ScratchContainer, block38, 0x38);
TS3_X86_OFFSET(ScratchContainer, end40, 0x40);
TS3_X86_OFFSET(ScratchContainer, allocator44, 0x44);
TS3_X86_OFFSET(ScratchContainer, block50, 0x50);
TS3_X86_OFFSET(ScratchContainer, end58, 0x58);
TS3_X86_OFFSET(ScratchContainer, allocator5C, 0x5C);
TS3_X86_OFFSET(ScratchContainer, block68, 0x68);
TS3_X86_OFFSET(ScratchContainer, end70, 0x70);
TS3_X86_OFFSET(ScratchContainer, allocator74, 0x74);
TS3_X86_SIZE(ScratchContainer, 0x7C + 4);

// ---------------------------------------------------------------------------------------
// 0x0087D770 / 0x0087D810: another scratch container with vtable 0x0102B99C, five tagged
// strings at +0x04/+0x14/+0x24/+0x34/+0x44 and a ScratchContainer at +0x54.
struct ScratchContainer87D {
    void* vtable;          // +0x00 0x0102B99C
    ScratchString s04;     // +0x04
    ScratchString s14;     // +0x14
    ScratchString s24;     // +0x24
    ScratchString s34;     // +0x34
    ScratchString s44;     // +0x44
    ScratchContainer container54;  // +0x54 (96 bytes in the game)
    // 0x0087D770, __thiscall, ret 0; EAX = last free result or the +0x04 begin.
    uintptr_t Destroy87D770();
    // 0x0087D810, __thiscall deleting dtor, ret 4: Destroy87D770, free(this) iff flags&1.
    ScratchContainer87D* Delete87D810(uint32_t flags);
};
TS3_X86_OFFSET(ScratchContainer87D, s04, 0x04);
TS3_X86_OFFSET(ScratchContainer87D, s14, 0x14);
TS3_X86_OFFSET(ScratchContainer87D, s24, 0x24);
TS3_X86_OFFSET(ScratchContainer87D, s34, 0x34);
TS3_X86_OFFSET(ScratchContainer87D, s44, 0x44);
TS3_X86_OFFSET(ScratchContainer87D, container54, 0x54);

// ---------------------------------------------------------------------------------------
// 0x005E8FA0 (this): export the record `record` through 0x009AF7C0, reporting the result on
// the global channel 0x0115B44 and dispatching two virtual callbacks.
struct ScratchExportSource {
    void* vtable;  // +0x00, slot 1 is called after a successful export
    // 0x005E8FA0, __thiscall, ret 8. Returns the 0x009AF7C0 low byte, 0 when record is
    // null or source is null. EA quirk: the value handed to 0x009AE400 keeps the upper 24
    // bits of the source pointer argument (only BL is overwritten).
    uint8_t Export(void* source, ScratchXmlRecord* record);
};
TS3_X86_SIZE(ScratchExportSource, 4);

// ---------------------------------------------------------------------------------------
// Concatenation helpers (cdecl): build a tagged string out of a C string and/or a raw span.
// 0x0088EC50: C string first, then the span at `tail` (only +0x00/+0x04/+0x0C are read).
ScratchString* __cdecl ConcatCStringSpan(ScratchString* out, const char* text, const ScratchString* tail);
// 0x009C65C0: the span first, then the C string.
ScratchString* __cdecl ConcatSpanCString(ScratchString* out, const ScratchString* head, const char* text);

// ---------------------------------------------------------------------------------------
// 0x009C1100, __stdcall, ret 0x1C: export the first element of the vector `items` (a pair
// of pointers) through a fresh record; 0 when the vector is empty or 0x0099B250 rejects it.
uint8_t __stdcall ExportFirstItem(uint32_t a1, const ScratchByteRange* items, uint32_t a3, uint32_t a4,
                                  uint32_t a5, uint32_t a6, uint32_t a7);

// ---------------------------------------------------------------------------------------
// XML spreadsheet helpers. Both are side-effecting (they build and free strings).
//
// 0x009CFA30, __thiscall, ret 0x14: from the tag name stored at +0x4C of `this`, build
// "<tag>" and "</tag>", locate both in the text buffer [textBegin, textEnd) (capacity
// textCap, freed at the end) and hand the element content to 0x004059F0 with `out` as
// `this`. Reproduces the original unsigned arithmetic, including the -1/-1 case where the
// subtraction wraps and 0x004059F0 is asked to copy from before the buffer.
struct ScratchXmlTagHost {
    uint8_t reserved00[0x4C];
    ScratchString tag4C;  // +0x4C

    ScratchString* ReplaceXmlAttribute(ScratchString* out, uint32_t textBegin, uint32_t textEnd,
                                       uint32_t textCap, int32_t value);
};
TS3_X86_OFFSET(ScratchXmlTagHost, tag4C, 0x4C);

// 0x009CFBD0, __stdcall, ret 0x20: true when `text` contains both "<tag>" and "</tag>".
// Both strings are passed by value (two 16-byte aggregates = 8 stack slots); the callee
// frees the tag buffer and the text buffer it was handed.
uint8_t __stdcall XmlTagMatches(ScratchString tag, ScratchString text);

// 0x009A02B0 (the 0x009A02CE manifest entry is this function's body from +0x1E on: it is
// only reachable through the jump at 0x009A02C2 inside the same 0x19C-byte frame, so the
// whole function is the entry point). Enumerates the source record's keys through the
// container returned by 0x004AFD20 (its virtual slot 8 receives the 0x0099E680 key), copies
// each 16-byte key into a 232-byte record, formats
// "%d:%08x:%08x:%I64x"(0, key+8, key+0xC, key) through 0x00590900 and appends the pair
// (0x004FA590(global 0x01180808, 0x811C9DC5, 0), 0x004B5440(map)) to the pair host at
// *out. Returns 0 when 0x00931020 or 0x004AFD20 yields nothing, or when 0x009AEB70 rejects
// an item; 1 otherwise.
uint8_t __cdecl ExportStringTables(ScratchXmlRecord* source, void* out);

// Not rewritten yet (see the unit status report):
//   0x009D0D00, __thiscall, ret 0: exports the whole spreadsheet document. 1724 bytes of
//              string parsing over the object at `this`, not attempted in this batch.

}  // namespace ts3::scratch