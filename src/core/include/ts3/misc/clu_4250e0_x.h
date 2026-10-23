#pragma once
// Cluster unit clu_4250e0_x (misc subsystem): TS3W.exe 0x004250E0, 0x00425380,
// 0x00425540, 0x00425C20, 0x00425D30, 0x00425DE0, 0x00425F10, 0x00426370, 0x00426B20,
// 0x00426EF0, 0x00428140, 0x004281F0, 0x00428450, 0x004287B0 (14 functions, 4041 bytes).
// An IJG-style JPEG decoder cluster: APP0/JFIF and APP14/Adobe marker parsers, the DHT
// (Huffman table) and DRI (restart interval) segment parsers, unknown-marker skip,
// entropy skip-to-next-marker, SOI verify, restart processing, pass control, YCbCr ->
// RGB conversion and the output-pipeline row consume functions. The unit never calls
// into other units directly: every outside operation goes through a pointer the master
// carries (error mgr, source mgr, method tables), so no layout.h bridge calls appear.
// All 14 are cdecl and return 0; 0x00425540 takes its argument IN ESI (the caller at
// 0x004260BA keeps the master in ESI and pushes nothing). EA bugs that affect outputs
// are kept and marked "EA:".
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

struct JpegMaster;

// ---- the objects the master points at ---------------------------------------

// Error manager (*master): message code at +0x14 with args at +0x18/+0x1C (APP0/APP14
// also park parsed header words at +0x18..+0x24) before the call. Fatal paths call
// errorExit and IGNORE its return, so a fake that returns lets both sides continue
// identically; warnings call emitMessage with the level the original pushes.
struct JpegErrorMgr {
    int32_t (__cdecl* errorExit)(JpegMaster* master);                 // +0x00
    int32_t (__cdecl* emitMessage)(JpegMaster* master, int32_t level); // +0x04
    uint8_t pad08[0x14 - 0x08];
    int32_t code;   // +0x14
    int32_t arg1;   // +0x18
    int32_t arg2;   // +0x1C
    int32_t arg3;   // +0x20
    int32_t arg4;   // +0x24
};
TS3_X86_OFFSET(JpegErrorMgr, code, 0x14);
TS3_X86_OFFSET(JpegErrorMgr, arg4, 0x24);

// Byte source (master+0x14): next/remaining are a read cursor the parsers load into
// registers and write back - each function has its own store order (and some paths
// return 0 WITHOUT storing; 0x00425DE0 stores before its inner refill instead), which
// the rewrites reproduce instruction for instruction. fillInputBuffer returns 0 on
// failure -> the parser returns 0. skipInputData / markerHandler are called with the
// master and (count / restart number).
struct JpegSourceMgr {
    const uint8_t* next;            // +0x00
    uint32_t remaining;             // +0x04
    uint8_t pad08[0x0C - 0x08];
    int32_t (__cdecl* fillInputBuffer)(JpegMaster* master);              // +0x0C
    int32_t (__cdecl* skipInputData)(JpegMaster* master, int32_t count); // +0x10
    int32_t (__cdecl* markerHandler)(JpegMaster* master, int32_t restartNum); // +0x14
};
TS3_X86_OFFSET(JpegSourceMgr, fillInputBuffer, 0x0C);
TS3_X86_OFFSET(JpegSourceMgr, skipInputData, 0x10);
TS3_X86_OFFSET(JpegSourceMgr, markerHandler, 0x14);

// Output manager (master+4): only the request-rows slot at +0x1C is used, by
// 0x00428140 / 0x004281F0 when the row group is exhausted.
struct JpegOutputMgr {
    uint8_t pad00[0x1C];
    int32_t (__cdecl* requestRows)(JpegMaster* master, int32_t a, int32_t b, int32_t c,
                                   int32_t wholeGroup);  // +0x1C
};
TS3_X86_OFFSET(JpegOutputMgr, requestRows, 0x1C);

// Pass-control state (master+0x1A0), used by 0x00426B20. begun (+8) selects the
// "already started" branch; method54/method58 are method OBJECTS the branch installs
// into master+0x1C8 (they replace the default one for later calls in the same pass).
struct JpegPassState {
    uint8_t pad00[0x08];
    int32_t begun;      // +0x08
    int32_t rangeStart; // +0x0C  copied to resultState+0x0C
    int32_t field10;    // +0x10  skip the put step when nonzero
    void* method54;     // +0x14  -> master+0x1C8 when flag64 path (a1+100)
    void* method58;     // +0x18  -> master+0x1C8 when restart-scan path (a1+5C && a1+6C)
};
TS3_X86_OFFSET(JpegPassState, begun, 0x08);
TS3_X86_OFFSET(JpegPassState, method58, 0x18);

// Entropy/scan state (master+0x1B4), used by 0x00425DE0 / 0x00426370.
struct JpegEntropyState {
    uint8_t pad00[0x58];
    int32_t restartNum;  // +0x58  RST index; becomes (restartNum + 1) & 7
    int32_t skipCount;   // +0x5C  non-marker bytes skipped (warn 112 clears it)
};
TS3_X86_OFFSET(JpegEntropyState, restartNum, 0x58);
TS3_X86_OFFSET(JpegEntropyState, skipCount, 0x5C);

// Color-convert state (master+0x1AC): 0x00426B20 calls slot0 as (master, 0/2/3) while
// 0x00428140 / 0x004281F0 treat the same object as state (fields at +8..+0x18).
struct JpegCConvertState {
    int32_t (__cdecl* consume)(JpegMaster* master, int32_t phase);  // +0x00
    uint8_t pad04[0x08 - 0x04];
    int32_t groupSize;   // +0x08  v7[2]: passed to requestRows
    int32_t rowBuf;      // +0x0C  v7[3]: output row pointer (from requestRows)
    int32_t groupRows;   // +0x10  v7[4]: rows per group; reset point / return value
    int32_t groupTop;    // +0x14  v7[5]: advances by groupRows at the end of a group
    int32_t consumed;    // +0x18  v7[6]: rows consumed so far; 0 -> requestRows
};
TS3_X86_OFFSET(JpegCConvertState, groupSize, 0x08);
TS3_X86_OFFSET(JpegCConvertState, consumed, 0x18);

// Color-convert / upsample object (master+0x1C0): slot0 is 0x00426B20's put step,
// slot1 (+4) 0x00428140's converter, dwords +0x10..+0x1C the four YCbCr->RGB lookup
// tables of 0x00426EF0, +0x34..+0x3C the per-component method slots and +0x64.. the
// per-component row strides of 0x00428450, and +0x5C/+0x60 0x00428450's row counters.
struct JpegColorCvt {
    int32_t (__cdecl* put)(JpegMaster* master);                     // +0x00
    int32_t (__cdecl* colorConvert)(JpegMaster* master, int32_t a2, int32_t a3, int32_t a4,
                                    int32_t rowPtr, int32_t* consumedOut, int32_t groupRows); // +0x04
    uint8_t pad08[0x10 - 0x08];
    const uint32_t* lutCrR;  // +0x10  (v5[4])
    const uint32_t* lutCbG;  // +0x14  (v5[5])
    const uint32_t* lutCrG;  // +0x18  (v5[6])
    const uint32_t* lutCbB;  // +0x1C  (v5[7])
    uint8_t pad20[0x34 - 0x20];
    int32_t (__cdecl* compMethod0)(JpegMaster*, int32_t, int32_t, int32_t);  // +0x34 (0x00428450)
    int32_t (__cdecl* compMethod1)(JpegMaster*, int32_t, int32_t, int32_t);  // +0x38
    int32_t (__cdecl* compMethod2)(JpegMaster*, int32_t, int32_t, int32_t);  // +0x3C
    uint8_t pad40[0x5C - 0x40];
    int32_t outRow;         // +0x5C  rows of the current group already copied
    int32_t pendingRows;    // +0x60  rows left to upsample into the output group
    int32_t stride0;        // +0x64  per-component line stride (row * stride)
    int32_t stride1;        // +0x68
    int32_t stride2;        // +0x6C
};
TS3_X86_OFFSET(JpegColorCvt, lutCrR, 0x10);
TS3_X86_OFFSET(JpegColorCvt, outRow, 0x5C);
TS3_X86_OFFSET(JpegColorCvt, stride0, 0x64);

// Put/output object (master+0x1C4): slot0 is 0x00426B20's conditional prepare step,
// slot1 (+4) 0x00428450's row upsample-to-output call.
struct JpegPut {
    int32_t (__cdecl* prepare)(JpegMaster* master);                  // +0x00
    int32_t (__cdecl* putRows)(JpegMaster* master, int32_t slots, int32_t srcRow,
                               int32_t dst, int32_t count);           // +0x04
};
TS3_X86_OFFSET(JpegPut, putRows, 0x04);

// Output-row copy object (master+0x1C8): slot0 is 0x00426B20's finish step (the pass
// branch may REPLACE this whole object with passState.method54/method58), slot1 (+4)
// 0x00428140 / 0x004281F0's row copy with (master, src, dst, count).
struct JpegPutRows {
    int32_t (__cdecl* finish)(JpegMaster* master, int32_t phase);    // +0x00
    int32_t (__cdecl* copyRows)(JpegMaster* master, int32_t src, int32_t dst, int32_t count); // +0x04
};
TS3_X86_OFFSET(JpegPutRows, copyRows, 0x04);

// Method object behind master+0x1A4: slot0 called as (master, int) by 0x00426B20.
struct JpegMethod420 {
    int32_t (__cdecl* start)(JpegMaster* master, int32_t phase);     // +0x00
};
// Method object behind master+0x1A8: slot +8 called as (master) by 0x00426B20.
struct JpegMethod424 {
    uint8_t pad00[0x08];
    int32_t (__cdecl* step)(JpegMaster* master);                     // +0x08
};
TS3_X86_OFFSET(JpegMethod424, step, 0x08);
// Method object behind master+0x1BC: slot0 called as (master) by 0x00426B20.
struct JpegMethod444 {
    int32_t (__cdecl* step)(JpegMaster* master);                     // +0x00
};
// Object behind master+0x1B0: only its dword at +0x14 is read (0x00426B20).
struct JpegMethod432 {
    uint8_t pad00[0x14];
    int32_t field14;                                                  // +0x14
};
TS3_X86_OFFSET(JpegMethod432, field14, 0x14);

// ---- the decoder master ------------------------------------------------------
//
// Byte offsets are x86 facts (TS3_X86_OFFSET asserts them there; on x64 the pointer
// fields shift, which is expected - this layout describes the 32-bit original).
struct JpegMaster {
    JpegErrorMgr* err;           // +0x000
    JpegOutputMgr* out;          // +0x004
    void* resultState;           // +0x008  (0x00426B20 writes +0x0C/+0x10 into it)
    uint8_t pad00C[0x14 - 0x00C];
    JpegSourceMgr* src;          // +0x014
    uint8_t pad018[0x20 - 0x018];
    int32_t componentCount;      // +0x020  a1[8]  (0x00428450 loop bound)
    uint8_t pad024[0x40 - 0x024];
    int32_t flag40;              // +0x040  a1[16] (0x00426B20 tail)
    int32_t flag44;              // +0x044  a1[17] (0x00426B20: nonzero skips the tail)
    uint8_t pad048[0x54 - 0x048];
    int32_t flag54;              // +0x054  a1[21]
    uint8_t pad058[0x5C - 0x058];
    int32_t flag5C;              // +0x05C  a1[23]
    uint8_t pad060[0x64 - 0x060];
    int32_t flag64;              // +0x064  a1[25]
    uint8_t pad068[0x6C - 0x068];
    int32_t flag6C;              // +0x06C  a1[27]
    int32_t width;               // +0x070  a1[28] (0x00426EF0 pairs = width >> 1)
    int32_t height;              // +0x074  a1[29] (0x004281F0 clamp)
    uint8_t pad078[0x88 - 0x078];
    int32_t flag88;              // +0x088  a1[34]
    uint8_t pad08C[0xD8 - 0x08C];
    int32_t componentInfo;       // +0x0D8  a1[54] (0x00428450: +84 bytes per component)
    uint8_t pad0DC[0xE4 - 0x0DC];
    uint8_t dhtLow[16];          // +0x0E4  id < 16: value & 0x0F  (0x00425540)
    uint8_t dhtHigh[32];         // +0x0F4  id < 16: value >> 4; id 16..31: raw value
    int32_t restartInterval;     // +0x114  a1[69] = the DRI count (0x00425C20)
    int32_t jfifSeen;            // +0x118  a1[70] = 1 (0x004250E0)
    uint8_t jfifUnits;           // +0x11C
    uint8_t pad11D;              // +0x11D
    uint16_t jfifXDensity;       // +0x11E  (big-endian decoded)
    uint16_t jfifYDensity;       // +0x120
    uint8_t pad122[0x124 - 0x122];
    int32_t adobeSeen;           // +0x124  a1[73] = 1 (0x00425380)
    uint8_t adobeTransform;      // +0x128
    uint8_t pad129[0x134 - 0x129];
    int32_t outputRows;          // +0x134  a1[77]
    uint8_t pad138[0x140 - 0x138];
    int32_t colorOffset;         // +0x140  a1[80] (0x00426EF0 LUT bias)
    uint8_t pad144[0x19C - 0x144];
    int32_t currentMarker;       // +0x19C  a1[103] (0 / 0xFF.. marker byte)
    JpegPassState* passState;    // +0x1A0  a1[104]
    JpegMethod420* method420;    // +0x1A4  a1[105]
    JpegMethod424* method424;    // +0x1A8  a1[106]
    JpegCConvertState* cconvert; // +0x1AC  a1[107]
    JpegMethod432* method432;    // +0x1B0  a1[108]
    JpegEntropyState* entropy;   // +0x1B4  a1[109]
    uint8_t pad1B8[0x1BC - 0x1B8];
    JpegMethod444* method444;    // +0x1BC  a1[111]
    JpegColorCvt* colorCvt;      // +0x1C0  a1[112]
    JpegPut* put;                // +0x1C4  a1[113]
    JpegPutRows* putRows;        // +0x1C8  a1[114]
};
TS3_X86_OFFSET(JpegMaster, src, 0x14);
TS3_X86_OFFSET(JpegMaster, componentCount, 0x20);
TS3_X86_OFFSET(JpegMaster, flag88, 0x88);
TS3_X86_OFFSET(JpegMaster, componentInfo, 0xD8);
TS3_X86_OFFSET(JpegMaster, dhtLow, 0xE4);
TS3_X86_OFFSET(JpegMaster, dhtHigh, 0xF4);
TS3_X86_OFFSET(JpegMaster, restartInterval, 0x114);
TS3_X86_OFFSET(JpegMaster, jfifSeen, 0x118);
TS3_X86_OFFSET(JpegMaster, adobeSeen, 0x124);
TS3_X86_OFFSET(JpegMaster, outputRows, 0x134);
TS3_X86_OFFSET(JpegMaster, colorOffset, 0x140);
TS3_X86_OFFSET(JpegMaster, currentMarker, 0x19C);
TS3_X86_OFFSET(JpegMaster, passState, 0x1A0);
TS3_X86_OFFSET(JpegMaster, cconvert, 0x1AC);
TS3_X86_OFFSET(JpegMaster, entropy, 0x1B4);
TS3_X86_OFFSET(JpegMaster, putRows, 0x1C8);

// ---- marker readers (source-mgr refill pattern) ------------------------------

// 0x004250E0: APP0/JFIF. Reads the big-endian length, verifies "JFIF\0" and the
// version (major must be 1; minor > 2 warns), decodes units/densities/thumbnail into
// the master, warns on thumbnail present / density mismatch, then skips the payload.
int32_t JpegApp0Jfif(JpegMaster* master);

// 0x00425380: APP14/Adobe. Reads the big-endian length, verifies "Adobe", stores the
// four parsed header words into the error mgr, sets adobeSeen + the transform byte,
// then skips the payload.
int32_t JpegApp14Adobe(JpegMaster* master);

// 0x00425540: DHT (define Huffman tables) - THE ARGUMENT ARRIVES IN ESI, nothing is
// pushed (single caller 0x004260BA); the portable body below takes it normally, and
// the x86-only entry reproduces the original ABI for hooks and tests. Per table pair:
// warn 78 (id, value), fatal 26 for id >= 32, then for id < 16 stores value's low
// nibble at master+0xE4+id and high nibble at +0xF4+id (fatal 27 when low > high),
// for id 16..31 the raw byte at +0xF4+id.
int32_t JpegReadDhtBody(JpegMaster* master);
#if defined(_M_IX86)
// The original's real entry: reads its only argument from ESI (nothing is pushed),
// cdecl ret 0. The definition is __declspec(naked) with an asm body (MSVC only allows
// naked on the definition itself).
int32_t __cdecl JpegReadDht(void);
#endif

// 0x00425C20: DRI (define restart interval). The full length word (NOT minus 2) must
// be 4 (fatal 9 otherwise; execution continues when errorExit returns), then the
// big-endian interval is read, warn 81, and stored to master->restartInterval.
int32_t JpegReadDri(JpegMaster* master);

// 0x00425D30: unknown marker - warn 90 with (currentMarker, length) and skip
// length - 2 bytes.
int32_t JpegSkipMarker(JpegMaster* master);

// 0x00425DE0: entropy skip to the next marker: counts non-0xFF bytes into
// entropy->skipCount (stores the cursor BEFORE each inner refill), skips 0xFF fill,
// treats 0xFF 0x00 as two more skipped bytes, and on a real marker warns 112 at
// level -1 with (count, marker) when count != 0, clears the count, sets
// master->currentMarker and stores the cursor. Returns 0 only when a refill fails.
int32_t JpegSkipToMarker(JpegMaster* master);

// 0x00425F10: SOI verify - read two bytes; anything but FF D8 triggers fatal 52 with
// (b0, b1); either way master->currentMarker = b1 and the cursor is stored. Returns
// 0 when a refill fails.
int32_t JpegVerifySoi(JpegMaster* master);

// ---- restart + pass control --------------------------------------------------

// 0x00426370: restart processing (NOT an autotest candidate). When currentMarker is
// 0 it calls 0x00425DE0. On marker == RST0 + entropy->restartNum: warn 97 at level 3
// with the restart number and clear the marker; otherwise call src->markerHandler
// (returning 0 stops). On success restartNum = (restartNum + 1) & 7.
int32_t JpegProcessRestart(JpegMaster* master);

// 0x00426B20: pass control. Drives the method-table branch (begun / flag54 / flag88
// / flag5C / flag6C / flag64 selection, fatal 45 when neither alternative applies),
// runs the consume/prepare steps (skipped when flag44), then writes the result row
// range into *resultState (+0x0C/+0x10) and returns resultState (0 when unset).
void* JpegStartPass(JpegMaster* master);

// ---- color conversion / output pipeline --------------------------------------

// 0x00426EF0: YCbCr -> RGB over width >> 1 pairs (3 bytes per output pixel) using
// the four LUTs at colorCvt+0x10..+0x1C, then one odd tail pixel when width is odd.
// Returns the output pointer BEFORE the odd tail's three bytes (the original does).
uint8_t* JpegYccToRgb(JpegMaster* master, uint8_t** planes, int32_t inputRow,
                      uint8_t** outRow);

// 0x00428140: request a whole row group when needed, run colorCvt->colorConvert,
// copy the newly produced rows through putRows->copyRows and advance the state;
// returns cconvert->groupRows.
int32_t JpegConsumeRows(JpegMaster* master, int32_t a2, int32_t a3, int32_t a4,
                        int32_t dst, int32_t* dstCount);

// 0x004281F0: like 0x00428140 but clamps the copy to (groupRows - consumed),
// (dstCapacity - *dstCount) and (height - groupTop); returns cconvert->groupRows.
int32_t JpegCopyRowsClamped(JpegMaster* master, int32_t a2, int32_t a3, int32_t a4,
                            int32_t dst, int32_t* dstCount, int32_t dstCapacity);

// 0x00428450: when colorCvt->outRow reaches outputRows, runs every component's
// method slot (componentInfo + 84 * i, strides from colorCvt+0x64+4i) and resets
// outRow; then copies min(outputRows - outRow, pendingRows, capacity - *dstCount)
// rows through put->putRows. When outRow reaches outputRows again it increments
// *inputRow and returns inputRow; otherwise the putRows result.
int32_t JpegUpsampleGroup(JpegMaster* master, int32_t planes, int32_t* inputRow,
                          int32_t a4, int32_t dst, int32_t* dstCount, int32_t dstCapacity);

// 0x004287B0: 3:1-weighted vertical upsample loop over outputRows rows for two
// components (writes 3:1 interpolated bytes from plane rows and returns the number
// of output rows written).
int32_t JpegUpsampleRows(JpegMaster* master, int32_t planeInfo, unsigned __int8** srcPlanes,
                         int32_t** dstRows);

}  // namespace ts3::misc
