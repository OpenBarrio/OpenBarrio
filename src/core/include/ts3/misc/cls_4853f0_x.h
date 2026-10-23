// TS3W.exe 0x004853F0..0x00489050 and 0x004A08E0..0x004A14C0 (worker unit cls_4853f0_x).
// Two unlabelled classes that are adjacent in the image:
//   * the DPF-style reader/loader block (group A, 0x4853F0..0x488E30),
//   * the query-expression AST and parser (group B, 0x4A08E0..0x4A14C0).
// The x86 game objects are handed to this code unchanged; TS3_X86_OFFSET records the
// x86-only layouts. Group A register-argument functions (a stream in ESI/EAX) are
// exposed here with explicit parameters; the harness/test supply x86 thunks.
#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::misc {

// ---------------------------------------------------------------------------
// Group A: packed-file reader (0x4853F0..0x488E30)
// ---------------------------------------------------------------------------

// TS3W.exe reader/stream object. The unit only touches these fields; the real object
// is larger (>= 0x8B4 bytes) and is owned by the game.
struct Stream {
    uint8_t* base;         // +0x000 NUL => bytes come through read()
    int32_t (__cdecl* read)(uintptr_t ctx, void* dst, int32_t pos, int32_t size);  // +0x004
    uintptr_t ctx;         // +0x008
    uint8_t inlineBuffer[0x208];  // +0x00C, read() destination for file-backed streams
    uint32_t capacity;     // +0x214
    uint32_t windowStart;  // +0x218
    uint32_t position;     // +0x21C
    uint32_t reserved220;  // +0x220
    uint32_t reserved224;  // +0x224
    uint32_t reserved228;  // +0x228
    void* errorSystem;     // +0x22C
};
TS3_X86_OFFSET(Stream, read, 0x004);
TS3_X86_OFFSET(Stream, inlineBuffer, 0x00C);
TS3_X86_OFFSET(Stream, capacity, 0x214);
TS3_X86_OFFSET(Stream, windowStart, 0x218);
TS3_X86_OFFSET(Stream, position, 0x21C);
TS3_X86_OFFSET(Stream, errorSystem, 0x22C);

// 20-byte block index built by 0x4859C0; +0xC is a (count+1)-dword offset table.
struct Block {
    void* pool;        // +0x00
    uint32_t offset;   // +0x04 file offset of the first entry
    uint8_t type;      // +0x08 entry reader kind (1..4), see 0x4859C0
    uint8_t pad09[3];
    uint32_t* entries;// +0x0C
    uint16_t count;    // +0x10
    uint8_t pad12[2];
};
TS3_X86_SIZE(Block, 20);
TS3_X86_OFFSET(Block, entries, 0x00C);
TS3_X86_OFFSET(Block, count, 0x010);

// The loader object (0x489B30's EDI). Only the fields this unit touches.
struct Loader {
    void* pool;              // +0x000
    Stream* stream;          // +0x004
    uint32_t baseOffset;     // +0x008
    uint32_t entryCount;     // +0x00C
    uint8_t pad010[0xDC - 0x010];
    uint16_t wordDc;         // +0x0DC
    uint16_t wordDe;         // +0x0DE
    uint8_t pad0e0[0x210 - 0x0E0];   // +0x0E0
    uint32_t* entryTable;    // +0x210
    void* freeList214;       // +0x214
    void* freeList218;       // +0x218
    uint32_t field21c;       // +0x21C
    uint32_t pad220;         // +0x220
    int32_t low224;          // +0x224
    int32_t high228;         // +0x228
    int32_t span22c;         // +0x22C
    uint32_t recordSeed;     // +0x230 (0x564 in the decompile: 0x234)
    uint32_t field234;       // +0x234
    uint32_t index238;       // +0x238
    uint8_t pad23c[0x370 - 0x23C];
    uint8_t byte370;         // +0x370
    uint8_t byte371;         // +0x371
    uint8_t pad372[2];
    Block* block374;         // +0x374
    Block* block378;         // +0x378
    uint8_t pad37c[0x3A0 - 0x37C];
    uint32_t field3a0;       // +0x3A0
    uint8_t pad3a4[0x3B8 - 0x3A4];
    uint32_t field3b8;       // +0x3B8
    uint32_t field3bc;       // +0x3BC
    uint32_t field3c0;       // +0x3C0
    uint8_t pad3c4[0x3D0 - 0x3C4];
    void* free3d0;           // +0x3D0
    uint8_t pad3d4[0x4D8 - 0x3D4];
    uint32_t random4d8;      // +0x4D8
    uint32_t random4dc;      // +0x4DC
    uint32_t random4e0;      // +0x4E0
    uint32_t random4e4;      // +0x4E4
    uint8_t byte4e8;         // +0x4E8
    uint8_t pad4e9[3];
    Block* block4ec;         // +0x4EC
    Block* block4f0;         // +0x4F0
    uint32_t field4f4;       // +0x4F4
    Block* block4f8;         // +0x4F8
    uint32_t field4fc;       // +0x4FC
    uint32_t field500;       // +0x500
    uint32_t field504;       // +0x504
    uint32_t field508;       // +0x508
    Block* block50c;         // +0x50C
    uint32_t field510;       // +0x510
    uint32_t field514;       // +0x514
    uint16_t word518;        // +0x518
    uint16_t word51a;        // +0x51A
    uint32_t field51c;       // +0x51C
};
TS3_X86_OFFSET(Loader, stream, 0x004);
TS3_X86_OFFSET(Loader, entryCount, 0x00C);
TS3_X86_OFFSET(Loader, wordDc, 0x0DC);
TS3_X86_OFFSET(Loader, wordDe, 0x0DE);
TS3_X86_OFFSET(Loader, entryTable, 0x210);
TS3_X86_OFFSET(Loader, low224, 0x224);
TS3_X86_OFFSET(Loader, index238, 0x238);
TS3_X86_OFFSET(Loader, block374, 0x374);
TS3_X86_OFFSET(Loader, block378, 0x378);
TS3_X86_OFFSET(Loader, block4f8, 0x4F8);
TS3_X86_OFFSET(Loader, block50c, 0x50C);
TS3_X86_OFFSET(Loader, field3a0, 0x3A0);
TS3_X86_OFFSET(Loader, field51c, 0x51C);

// Output record of 0x485D90 (the loader's +0x37C object, ~0x16D bytes used).
struct Record {
    uint16_t word00, word02, word04, word06, word08;  // +0x00..+0x08
    uint16_t pad0a;
    uint32_t field0c;                                  // +0x0C
    uint32_t field10, field14, field18, field1c;       // +0x10..+0x1C
    uint32_t field20, field24, field28;                // +0x20..+0x28
    uint32_t field2c;                                  // +0x2C
    uint8_t byte30;                                    // +0x30
    uint8_t pad31[3];
    uint32_t field34, field38, field3c;                // +0x34..+0x3C
    uint32_t field40, field44, field48, field4c;       // +0x40..+0x4C
    uint32_t field50;                                  // +0x50
    void* field54;                                     // +0x54 (allocated array)
    uint32_t data[0x104 / 4];                          // +0x58 generic copy target
    uint32_t field15c, field160, field164, field168;   // +0x15C..+0x168
    uint8_t byte16c;                                   // +0x16C
};
TS3_X86_OFFSET(Record, field0c, 0x00C);
TS3_X86_OFFSET(Record, byte30, 0x030);
TS3_X86_OFFSET(Record, field54, 0x054);
TS3_X86_OFFSET(Record, field15c, 0x15C);
TS3_X86_OFFSET(Record, byte16c, 0x16C);

// TS3W.exe 0x004853F0: variable-length value read from the stream for tag `tag` (28/29 read
// 2/4 big-endian bytes, <=246 returns tag-139, 247..250 and 251..254 fixed encodings).
int32_t ReadStreamValue(Stream* stream, int32_t tag);
// TS3W.exe 0x004859C0: build a 20-byte block index from the stream; pool is the allocator.
Block* BuildBlockIndex(Stream* stream, void* pool);
// TS3W.exe 0x00485B20: read entry `index` of `block` (never reached for an out-of-range index).
void* ReadBlockEntry(int32_t index, Block* block, Stream* stream);
// TS3W.exe 0x00485B90: entry 0x238 of the loader's 0x374 block, but only when kind == 4.
void* LoaderEntryForKind(void* loaderRaw, int32_t unused, int16_t kind);
// TS3W.exe 0x00485BC0: rebuild the loader's 0x4FC/0x500/0x504/0x508 event index.
int32_t RebuildLoaderIndex(Loader* loader);
// TS3W.exe 0x00485D90: parse entry `index` of `block` into `out` (record table, fixed layout).
int32_t ParseLoaderRecord(int32_t index, Stream* stream, Block* block, Record* out);
// TS3W.exe 0x00488930: free every list the loader owns (and the loader itself).
int32_t DestroyLoader(Loader* loader);
// TS3W.exe 0x00488C90: shell-sort `count` 4-byte {u16 key; u16 value} records by key.
int32_t SortLoaderEntries(void* recordsRaw, int32_t count);
// TS3W.exe 0x00488E30: initialize the loader's random seeds and min/max span fields.
int32_t InitLoader(Loader* loader);

#if defined(_M_IX86)
// Game-ABI entries used as the harness replace symbols for the register-argument entry
// points (they read ESI/EAX/ECX and leave the stack arguments to the game's caller).
Block* __cdecl BuildBlockIndexEntry(void);
int32_t __cdecl ParseLoaderRecordEntry(void);
int32_t __cdecl SortLoaderEntriesEntry(void);
int32_t __cdecl InitLoaderEntry(void);
#endif

// ---------------------------------------------------------------------------
// Group B: query-expression AST (0x4A08E0..0x4A14C0)
// ---------------------------------------------------------------------------
// Every node starts with the game's class descriptor pointer; descriptor[1] is the
// node's Test() (virtual slot 1, `bool thiscall(void*, int32_t)`).
struct QueryNode {
    void* klass;  // +0x00
    // TS3W.exe 0x004A0B30: store class descriptor 0x00FB1F3C at +0 and return this.
    QueryNode* SetClass(int32_t unused);
};
struct QueryNotNode : QueryNode { QueryNode* child; };                   // 0xFB1F6C
struct QueryAndNode : QueryNode { QueryNode* left; QueryNode* right; };  // 0xFB1F74
struct QueryOrNode : QueryNode { QueryNode* left; QueryNode* right; };   // 0xFB1F7C
struct QueryWordNode : QueryNode { const char16_t* word; };              // 0xFB1F84

// Open-addressed string->node map (this of 0x4A1340/0x4A14C0 helpers).
struct QueryMap {
    uint32_t reserved0;  // +0x00
    void** buckets;      // +0x04
    uint32_t count;      // +0x08
};

// Token cursor the parser walks (16 bytes).
struct QueryCursor {
    const char16_t** token;  // +0x00
    const char16_t** end;    // +0x04
    void* arena;             // +0x08 bump arena {+8 limit, +0xC cursor, +0x10 high}
    int32_t error;           // +0x0C 0xD06B0006 / 0xD06B0007 on failure
};

// TS3W.exe 0x004A0980: true when the current token equals `keyword`, consuming it.
bool MatchQueryToken(QueryCursor* cursor, const char16_t* keyword);
// TS3W.exe 0x004A0B40: parse "not", a parenthesized or-chain, or a word node.
QueryNode* ParseQueryPrimary(QueryCursor* cursor);
// TS3W.exe 0x004A0CA0: parse primary ("and" primary)*.
QueryNode* ParseQueryAnd(QueryCursor* cursor);
// TS3W.exe 0x004A0D30: parse and-chain ("or" and-chain)*.
QueryNode* ParseQueryOr(QueryCursor* cursor);
// TS3W.exe 0x004A1340: FNV-1 lookup of *key in map; writes {node, bucket slot} to out.
void** QueryMapFind(QueryMap* map, void** out, const char16_t** key);
// TS3W.exe 0x004A14C0: slot 1 of the word node (class 0xFB1F84) - is `word` in map?
bool QueryWordNodeTest(QueryWordNode* node, QueryMap* map);
// TS3W.exe 0x004A08E0: slot 1 of the not node (class 0xFB1F6C).
bool QueryNotNodeTest(QueryNotNode* node, int32_t value);
// TS3W.exe 0x004A0900: slot 1 of the and node (class 0xFB1F74).
bool QueryAndNodeTest(QueryAndNode* node, int32_t value);
// TS3W.exe 0x004A0940: slot 1 of the or node (class 0xFB1F7C).
bool QueryOrNodeTest(QueryOrNode* node, int32_t value);

}  // namespace ts3::misc