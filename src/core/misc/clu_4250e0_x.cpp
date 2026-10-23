// Cluster unit clu_4250e0_x (misc): an IJG-style JPEG decoder cluster - APP0/JFIF and
// APP14/Adobe marker parsing, DHT / DRI segment parsing, unknown-marker skip, entropy
// skip-to-next-marker, SOI verify, restart processing, pass control, YCbCr -> RGB and
// the output-pipeline row consume functions. Originals: TS3W.exe 0x004250E0,
// 0x00425380, 0x00425540, 0x00425C20, 0x00425D30, 0x00425DE0, 0x00425F10, 0x00426370,
// 0x00426B20, 0x00426EF0, 0x00428140, 0x004281F0, 0x00428450, 0x004287B0.
//
// The parser family shares one source-cursor discipline: the cursor (next, remaining)
// lives in LOCALS, refills call src->fillInputBuffer WITHOUT writing the cursor back
// first (the source mgr would see the pre-function state - that is what the original
// does everywhere except 0x00425DE0's inner loop, which stores before refilling), and
// a failed refill returns 0 with NO write-back. Each function writes the cursor back
// exactly where the original does. Fatal errors call err->errorExit and IGNORE its
// return (EA: execution continues when it returns - the game's own errorExit does
// not return; the differential fakes do, so both sides fall through identically).
#include "ts3/misc/clu_4250e0_x.h"

namespace ts3::misc {
namespace {

// The source cursor, loaded from src->next / src->remaining like the originals do.
struct Cursor {
    const uint8_t* next;
    uint32_t remaining;
};

// Entry: load the cursor; when it is empty call fillInputBuffer once (without writing
// anything back first) and reload. No re-check of the refilled state, like the original.
bool InitCursor(JpegMaster* master, Cursor& c)
{
    JpegSourceMgr* src = master->src;
    c.next = src->next;
    c.remaining = src->remaining;
    if (!c.remaining) {
        if (!src->fillInputBuffer(master))
            return false;
        c.next = src->next;
        c.remaining = src->remaining;
    }
    return true;
}

// One byte: refill when the LOCAL remaining hits 0 (the store-back happens only where
// the original puts it), then consume. false = refill failed (caller returns 0).
bool ReadByte(JpegMaster* master, Cursor& c, uint8_t& out)
{
    if (!c.remaining) {
        if (!master->src->fillInputBuffer(master))
            return false;
        c.next = master->src->next;
        c.remaining = master->src->remaining;
    }
    out = *c.next++;
    --c.remaining;
    return true;
}

// Big-endian length prefix (b0 << 8) + b1, read through the helpers above.
bool ReadBe16(JpegMaster* master, Cursor& c, uint8_t& b0, uint8_t& b1)
{
    return ReadByte(master, c, b0) && ReadByte(master, c, b1);
}

}  // namespace

// 0x004250E0: APP0/JFIF marker. length = BE16 - 2; < 14 warns 76 (arg = payload).
// Reads the 14-byte header: "JFIF\0" (else warn 76 with the full payload), version
// (major != 1 warns 115 at level -1, minor > 2 warns 88), then units/densities into
// the master, warn 86 with (X, Y, units), thumbnail present warns 89, and a payload
// that isn't 3*thumbX*thumbY warns 87. Cursor written back before the payload skip.
int32_t JpegApp0Jfif(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    const int32_t payload = static_cast<int32_t>((static_cast<uint32_t>(b0) << 8) | b1) - 2;
    int32_t left = payload;
    JpegErrorMgr* err = master->err;
    if (payload < 14) {
        err->code = 76;
        err->arg1 = payload;
        err->emitMessage(master, 1);
    } else {
        uint8_t hdr[14];
        for (int i = 0; i < 14; ++i) {
            if (!ReadByte(master, c, hdr[i]))
                return 0;
        }
        left -= 14;
        if (hdr[0] == 'J' && hdr[1] == 'F' && hdr[2] == 'I' && hdr[3] == 'F' && hdr[4] == 0) {
            const uint8_t major = hdr[5];
            const uint8_t minor = hdr[6];
            if (major == 1) {
                if (minor > 2) {
                    err->code = 88;
                    err->arg1 = 1;
                    err->arg2 = minor;
                    err->emitMessage(master, 1);
                }
            } else {
                err->code = 115;
                err->arg1 = major;
                err->arg2 = minor;
                err->emitMessage(master, -1);
            }
            master->jfifUnits = hdr[7];
            master->jfifYDensity = static_cast<uint16_t>((hdr[10] << 8) | hdr[11]);
            master->jfifXDensity = static_cast<uint16_t>((hdr[8] << 8) | hdr[9]);
            master->jfifSeen = 1;
            // The original zero-extends X into err+0x18 (movzx), re-reads Y from the
            // master word and the units from the master byte (movzx both).
            err->arg1 = master->jfifXDensity;
            err->arg2 = master->jfifYDensity;
            err->arg3 = master->jfifUnits;
            err->code = 86;
            err->emitMessage(master, 1);
            // Thumbnail: word at hdr[12..13] little-endian, so low byte = hdr[12].
            const int32_t thumbLo = hdr[12];
            const int32_t thumbHi = hdr[13];
            if ((thumbHi << 8 | thumbLo) != 0) {
                err->code = 89;
                err->arg1 = thumbLo;
                err->arg2 = thumbHi;
                err->emitMessage(master, 1);
            }
            if (left != 3 * thumbLo * thumbHi) {
                err->code = 87;
                err->arg1 = left;
                err->emitMessage(master, 1);
            }
        } else {
            err->code = 76;
            err->arg1 = left + 14;
            err->emitMessage(master, 1);
        }
    }
    src->next = c.next;
    src->remaining = c.remaining;
    if (left > 0)
        src->skipInputData(master, left);
    return 1;
}

// 0x00425380: APP14/Adobe marker. length = BE16 - 2; < 12 warns 77. Reads 12 bytes:
// "Adobe" (else warn 77 with the full payload), then the four header words into the
// error mgr (+0x18 = h5<<8|h6, +0x1C = h7<<8|h8, +0x20 = h9<<8|h10, +0x24 = h11),
// warn 75, and only AFTER the warn sets adobeSeen / adobeTransform. Cursor written
// back before the payload skip.
int32_t JpegApp14Adobe(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    const int32_t payload = static_cast<int32_t>((static_cast<uint32_t>(b0) << 8) | b1) - 2;
    int32_t left = payload;
    JpegErrorMgr* err = master->err;
    if (payload < 12) {
        err->code = 77;
        err->arg1 = payload;
        err->emitMessage(master, 1);
    } else {
        uint8_t hdr[12];
        for (int i = 0; i < 12; ++i) {
            if (!ReadByte(master, c, hdr[i]))
                return 0;
        }
        left -= 12;
        if (hdr[0] == 'A' && hdr[1] == 'd' && hdr[2] == 'o' && hdr[3] == 'b' && hdr[4] == 'e') {
            err->arg1 = static_cast<int32_t>((hdr[5] << 8) | hdr[6]);
            err->arg2 = static_cast<int32_t>((hdr[7] << 8) | hdr[8]);
            err->arg3 = static_cast<int32_t>((hdr[9] << 8) | hdr[10]);
            err->arg4 = hdr[11];
            err->code = 75;
            err->emitMessage(master, 1);
            master->adobeSeen = 1;
            master->adobeTransform = hdr[11];
        } else {
            err->code = 77;
            err->arg1 = left + 12;
            err->emitMessage(master, 1);
        }
    }
    src->next = c.next;
    src->remaining = c.remaining;
    if (left > 0)
        src->skipInputData(master, left);
    return 1;
}

// 0x00425540: DHT (define Huffman tables). Body of the ESI-convention entry below.
// length = BE16 - 2, then pairs (tableId, value): warn 78 with the pair, fatal 26 for
// id >= 32 (EA: execution CONTINUES into the store below when errorExit returns), and
// for id < 16 the value's low nibble at master+0xE4+id and high nibble at +0xF4+id
// (fatal 27 when low > high, EA: continues); id 16..31 stores the raw value at
// +0xF4+id. EA: id >= 32 also falls into the raw store, writing master+0xF4+id up to
// +0x1F3 (past the fields here; callers must give the master that much room).
int32_t JpegReadDhtBody(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    int32_t left = static_cast<int32_t>((static_cast<uint32_t>(b0) << 8) | b1) - 2;
    JpegErrorMgr* err = master->err;
    uint8_t* base = reinterpret_cast<uint8_t*>(master);
    while (left > 0) {
        uint8_t id, value;
        if (!ReadByte(master, c, id))
            return 0;
        if (!ReadByte(master, c, value))
            return 0;
        left -= 2;
        err->code = 78;
        err->arg1 = id;
        err->arg2 = value;
        err->emitMessage(master, 1);
        if (id >= 32) {
            err->code = 26;
            err->arg1 = id;
            err->errorExit(master);
        }
        if (id < 16) {
            base[0xE4 + id] = static_cast<uint8_t>(value & 0x0F);
            base[0xF4 + id] = static_cast<uint8_t>(value >> 4);
            if ((value & 0x0F) > (value >> 4)) {
                err->code = 27;
                err->arg1 = value;
                err->errorExit(master);
            }
        } else {
            base[0xF4 + id] = value;
        }
    }
    src->next = c.next;
    src->remaining = c.remaining;
    return 1;
}

#if defined(_M_IX86)
// The original takes its only argument in ESI - the single caller (0x004260BA) keeps
// the master there and pushes nothing - and returns with a plain ret (cdecl ret 0).
// MSVC asm can only call through a pointer, hence kDhtBody.
static int32_t (__cdecl* const kDhtBody)(JpegMaster*) = &JpegReadDhtBody;

__declspec(naked) int32_t __cdecl JpegReadDht(void)
{
    __asm {
        push esi                 // the master, exactly the original's argument
        call dword ptr [kDhtBody]
        add esp, 4
        ret
    }
}
#endif

// 0x00425C20: DRI (define restart interval). The FULL length word (no -2) must be 4 -
// otherwise fatal 9 (EA: continues when errorExit returns; no other args are set).
// Then the BE16 interval: warn 81 with the value, store to master->restartInterval,
// write the cursor back. No payload skip (the segment is exactly 4 bytes).
int32_t JpegReadDri(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    const int32_t length = static_cast<int32_t>((static_cast<uint32_t>(b0) << 8) | b1);
    JpegErrorMgr* err = master->err;
    if (length != 4) {
        err->code = 9;
        err->errorExit(master);
    }
    uint8_t b2, b3;
    if (!ReadBe16(master, c, b2, b3))
        return 0;
    const int32_t interval = static_cast<int32_t>((static_cast<uint32_t>(b2) << 8) | b3);
    err->code = 81;
    err->arg1 = interval;
    err->emitMessage(master, 1);
    master->restartInterval = interval;
    src->next = c.next;
    src->remaining = c.remaining;
    return 1;
}

// 0x00425D30: unknown-marker skip. BE16 length; warn 90 with (currentMarker, length);
// the cursor is stored BEFORE the skip, then skipInputData(length - 2) is called
// UNCONDITIONALLY (EA: the count can be 0 or negative; there is no > 0 guard here,
// unlike 0x004250E0/0x00425380).
int32_t JpegSkipMarker(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    const int32_t length = static_cast<int32_t>((static_cast<uint32_t>(b0) << 8) | b1);
    JpegErrorMgr* err = master->err;
    err->code = 90;
    err->arg1 = master->currentMarker;
    err->arg2 = length;
    err->emitMessage(master, 1);
    src->next = c.next;
    src->remaining = c.remaining;
    src->skipInputData(master, length - 2);
    return 1;
}

// 0x00425DE0: entropy skip to the next marker. Non-0xFF bytes increment
// entropy->skipCount; the cursor is stored BEFORE each inner refill (unlike the other
// parsers). A run of 0xFF at the marker position is skipped; 0xFF 0x00 is entropy
// stuffing: count += 2, store, and the scan restarts. On a real marker: when the
// count is nonzero warn 112 at level -1 with (count, marker) and clear the count,
// set master->currentMarker, store the cursor, return 1. Returns 0 only when a
// refill fails.
int32_t JpegSkipToMarker(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b;
    if (!ReadByte(master, c, b))
        return 0;
    for (;;) {                       // scan outer loop (reached again after FF 00)
        if (b != 0xFF) {
            do {
                ++master->entropy->skipCount;
                src->next = c.next;          // store BEFORE the refill check - EA
                src->remaining = c.remaining;
                if (!c.remaining) {
                    if (!src->fillInputBuffer(master))
                        return 0;
                    c.next = src->next;
                    c.remaining = src->remaining;
                }
                b = *c.next++;
                --c.remaining;
            } while (b != 0xFF);
        }
        // b == 0xFF: read the marker byte (0xFF fill loops, 0x00 is stuffed data).
        uint8_t marker;
        for (;;) {
            if (!c.remaining) {
                if (!src->fillInputBuffer(master))
                    return 0;
                c.next = src->next;
                c.remaining = src->remaining;
            }
            marker = *c.next++;
            --c.remaining;
            if (marker != 0xFF)
                break;
        }
        if (marker == 0) {
            master->entropy->skipCount += 2;
            src->next = c.next;        // store before re-entering the scan
            src->remaining = c.remaining;
            if (!ReadByte(master, c, b))  // the original jumps back to the scan top
                return 0;
            continue;
        }
        if (master->entropy->skipCount) {
            JpegErrorMgr* err = master->err;
            err->code = 112;
            err->arg1 = master->entropy->skipCount;
            err->arg2 = marker;
            err->emitMessage(master, -1);
            master->entropy->skipCount = 0;
        }
        master->currentMarker = marker;
        src->next = c.next;
        src->remaining = c.remaining;
        return 1;
    }
}

// 0x00425F10: SOI verify. Two bytes; anything but FF D8 sets fatal 52 with (b0, b1)
// (EA: continues when errorExit returns). Either way master->currentMarker = b1, the
// cursor is stored, return 1; a failed refill returns 0 with no store.
int32_t JpegVerifySoi(JpegMaster* master)
{
    JpegSourceMgr* src = master->src;
    Cursor c;
    if (!InitCursor(master, c))
        return 0;
    uint8_t b0, b1;
    if (!ReadBe16(master, c, b0, b1))
        return 0;
    if (b0 != 0xFF || b1 != 0xD8) {
        JpegErrorMgr* err = master->err;
        err->code = 52;
        err->arg1 = b0;
        err->arg2 = b1;
        err->errorExit(master);
    }
    master->currentMarker = b1;
    src->next = c.next;
    src->remaining = c.remaining;
    return 1;
}

// 0x00426370: restart processing. If currentMarker is 0, find the next marker with
// 0x00425DE0 (0 = give up). Marker == RST0 + entropy->restartNum: warn 97 at LEVEL 3
// with the restart number and clear the marker; otherwise call src->markerHandler
// (master, restartNum) and return 0 when it says 0 (EA: no increment then). Success:
// restartNum = (restartNum + 1) & 7.
int32_t JpegProcessRestart(JpegMaster* master)
{
    if (!master->currentMarker && !JpegSkipToMarker(master))
        return 0;
    const int32_t restart = master->entropy->restartNum;
    if (master->currentMarker == restart + 0xD0) {
        JpegErrorMgr* err = master->err;
        err->code = 97;
        err->arg1 = restart;
        err->emitMessage(master, 3);
        master->currentMarker = 0;
    } else if (!master->src->markerHandler(master, restart)) {
        return 0;
    }
    // The original re-reads restartNum AFTER the callback (markerHandler or
    // emitMessage may have changed it) - keep that.
    JpegEntropyState* ent = master->entropy;
    ent->restartNum = (ent->restartNum + 1) & 7;
    return 1;
}

// 0x00426B20: pass control. The already-started branch clears begun and runs
// finish(0) / consume(2) / start(2) on the objects at master+0x1C8 / +0x1AC / +0x1A4.
// Otherwise, when flag54 && !flag88: restart-scan (flag5C && flag6C) installs
// passState->method58 into master+0x1C8 and sets begun, flag64 installs method54,
// neither is fatal 45 (EA: continues when errorExit returns). Then step444, step424
// (slot +8); unless flag44: prepare when field10 is 0, the put step, finish(begun)
// when flag54, consume(begun ? 3 : 0), start(0). Finally the result range is written
// into *resultState (+0x0C = rangeStart, +0x10 = rangeStart + (begun ? 1 : 0) + 1,
// plus (flag6C ? 1 : 0) + 1 when flag40 and method432->field14 == 0) and the state
// pointer returned (0 when unset).
void* JpegStartPass(JpegMaster* master)
{
    JpegPassState* st = master->passState;
    if (st->begun) {
        st->begun = 0;
        master->putRows->finish(master, 0);
        master->cconvert->consume(master, 2);
        master->method420->start(master, 2);
    } else {
        if (master->flag54 && !master->flag88) {
            if (master->flag5C && master->flag6C) {
                master->putRows = static_cast<JpegPutRows*>(st->method58);
                st->begun = 1;
            } else if (master->flag64) {
                master->putRows = static_cast<JpegPutRows*>(st->method54);
            } else {
                master->err->code = 45;
                master->err->errorExit(master);
            }
        }
        master->method444->step(master);
        master->method424->step(master);
        if (!master->flag44) {
            if (!st->field10)
                master->put->prepare(master);
            master->colorCvt->put(master);
            if (master->flag54)
                master->putRows->finish(master, st->begun);
            master->cconvert->consume(master, st->begun ? 3 : 0);
            master->method420->start(master, 0);
        }
    }
    void* result = master->resultState;
    if (result) {
        uint8_t* state = static_cast<uint8_t*>(result);
        *reinterpret_cast<int32_t*>(state + 0x0C) = st->rangeStart;
        int32_t end = st->rangeStart + (st->begun ? 1 : 0) + 1;
        if (master->flag40 && master->method432->field14 == 0)
            end += (master->flag6C ? 1 : 0) + 1;
        *reinterpret_cast<int32_t*>(state + 0x10) = end;
    }
    return result;
}

// 0x00426EF0: YCbCr -> RGB over width >> 1 pixel pairs, plus one odd tail pixel when
// width is odd. The per-sample math is a byte read at (sample + lut + colorOffset) -
// the luts/colorOffset address into a clamp-add table, exactly as the original does.
// Returns the output pointer BEFORE the odd tail's three bytes (the original does not
// advance it past the tail).
uint8_t* JpegYccToRgb(JpegMaster* master, uint8_t** planes, int32_t inputRow, uint8_t** outRow)
{
    JpegColorCvt* cvt = master->colorCvt;
    const int32_t* lutCrR = reinterpret_cast<const int32_t*>(cvt->lutCrR);
    const int32_t* lutCbG = reinterpret_cast<const int32_t*>(cvt->lutCbG);
    const int32_t* lutCrG = reinterpret_cast<const int32_t*>(cvt->lutCrG);
    const int32_t* lutCbB = reinterpret_cast<const int32_t*>(cvt->lutCbB);
    const uint8_t* y = reinterpret_cast<uint8_t* const*>(planes[0])[inputRow];
    const uint8_t* cb = reinterpret_cast<uint8_t* const*>(planes[1])[inputRow];
    const uint8_t* cr = reinterpret_cast<uint8_t* const*>(planes[2])[inputRow];
    uint8_t* result = *outRow;
    const int32_t off = master->colorOffset;
    int32_t pairs = master->width >> 1;
    while (pairs--) {
        const int32_t v11 = *cr;
        const int32_t v12 = *cb;
        const int32_t v30 = lutCrR[v11];
        int32_t mid = lutCrG[v11] + lutCbB[v12];
        const int32_t y0 = *y;
        const int32_t cbg = lutCbG[v12];
        *result = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(y0 + v30 + off));
        mid >>= 16;
        result[1] = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(mid + y0 + off));
        result[2] = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(off + cbg + y0));
        const int32_t y1 = y[1];
        uint8_t* p = result + 3;
        *p = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(y1 + v30 + off));
        const uint8_t g = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(y1 + mid + off));
        ++cb;
        ++cr;
        p[1] = g;
        p[2] = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(off + cbg + y1));
        y += 2;
        result = p + 3;
    }
    if (master->width & 1) {
        const int32_t cbv = *cb;
        const int32_t crv = *cr;
        const int32_t yv = *y;
        const int32_t cbg = lutCbG[cbv];
        const int32_t mid = (lutCrG[crv] + lutCbB[cbv]) >> 16;
        *result = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(yv + lutCrR[crv] + off));
        result[1] = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(yv + mid + off));
        result[2] = *reinterpret_cast<const uint8_t*>(static_cast<intptr_t>(off + cbg + yv));
    }
    return result;
}

// 0x00428140: consume rows into the output group. When consumed is 0 request a whole
// group (wholeGroup = 1), run colorCvt->colorConvert (7 args; the 5th declared arg of
// this function, dst, is pushed by callers but NEVER read), and when the converter
// advanced consumed copy the new rows through putRows->copyRows with dst = 0 (EA: the
// original passes 0, not the dst argument), bumping *dstCount. When consumed reaches
// groupRows the group is done: groupTop += groupRows, consumed = 0. Returns groupRows.
int32_t JpegConsumeRows(JpegMaster* master, int32_t a2, int32_t a3, int32_t a4, int32_t dst,
                        int32_t* dstCount)
{
    (void)dst;  // a5 is never read by the original
    JpegCConvertState* v7 = master->cconvert;
    if (!v7->consumed)
        v7->rowBuf = master->out->requestRows(master, v7->groupSize, v7->groupTop, v7->groupRows, 1);
    const uint32_t before = static_cast<uint32_t>(v7->consumed);
    master->colorCvt->colorConvert(master, a2, a3, a4, v7->rowBuf, &v7->consumed, v7->groupRows);
    const uint32_t after = static_cast<uint32_t>(v7->consumed);
    if (after > before) {
        const uint32_t n = after - before;
        master->putRows->copyRows(master, v7->rowBuf + 4 * static_cast<int32_t>(before), 0,
                                  static_cast<int32_t>(n));
        *dstCount += static_cast<int32_t>(n);
    }
    const int32_t result = v7->groupRows;
    if (static_cast<uint32_t>(v7->consumed) >= static_cast<uint32_t>(result)) {
        v7->groupTop += result;
        v7->consumed = 0;
    }
    return result;
}

// 0x004281F0: like 0x00428140 but the copy is clamped to (groupRows - consumed),
// (dstCapacity - *dstCount) and (height - groupTop) with unsigned compares, copied to
// dst + 4 * *dstCount, and consumed/groupTop advance accordingly. The first three
// declared args (a2, a3, a4) are pushed but never read.
int32_t JpegCopyRowsClamped(JpegMaster* master, int32_t a2, int32_t a3, int32_t a4, int32_t dst,
                            int32_t* dstCount, int32_t dstCapacity)
{
    (void)a2;
    (void)a3;
    (void)a4;
    JpegCConvertState* v7 = master->cconvert;
    if (!v7->consumed)
        v7->rowBuf = master->out->requestRows(master, v7->groupSize, v7->groupTop, v7->groupRows, 0);
    uint32_t n = static_cast<uint32_t>(v7->groupRows - v7->consumed);
    if (n > static_cast<uint32_t>(dstCapacity - *dstCount))
        n = static_cast<uint32_t>(dstCapacity - *dstCount);
    if (n > static_cast<uint32_t>(master->height - v7->groupTop))
        n = static_cast<uint32_t>(master->height - v7->groupTop);
    master->putRows->copyRows(master, v7->rowBuf + 4 * v7->consumed,
                              dst + 4 * (*dstCount), static_cast<int32_t>(n));
    *dstCount += static_cast<int32_t>(n);
    v7->consumed += static_cast<int32_t>(n);
    const int32_t result = v7->groupRows;
    if (static_cast<uint32_t>(v7->consumed) >= static_cast<uint32_t>(result)) {
        v7->groupTop += result;
        v7->consumed = 0;
    }
    return result;
}

// 0x00428450: output-group upsample. When colorCvt->outRow >= outputRows (signed),
// run every component's method slot (colorCvt+0x34+4i with the componentInfo object
// at +84-byte stride, planes[i] + 4 * inputRow * stride_i as the source, and the
// colorCvt+0x0C+4i slot pointer as the 4th arg) and reset outRow; then copy
// min(outputRows - outRow, pendingRows, dstCapacity - *dstCount) rows through
// put->putRows (srcRow = outRow, dst = a5 + 4 * *dstCount) and advance the counters.
// When outRow reaches outputRows again: ++*inputRow and return &inputRow (the
// pointer!); otherwise the putRows return value. a4 is never read.
int32_t JpegUpsampleGroup(JpegMaster* master, int32_t planes, int32_t* inputRow, int32_t a4,
                          int32_t dst, int32_t* dstCount, int32_t dstCapacity)
{
    (void)a4;
    JpegColorCvt* cvt = master->colorCvt;
    if (cvt->outRow >= master->outputRows) {
        const int32_t cc = master->componentCount;
        int32_t compInfo = master->componentInfo;
        for (int32_t i = 0; i < cc; ++i) {
            int32_t (*const* slots)(JpegMaster*, int32_t, int32_t, int32_t) = &cvt->compMethod0;
            const int32_t stride = (&cvt->stride0)[i];
            const int32_t* planes32 = reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(planes));
            const uint32_t src = static_cast<uint32_t>(planes32[i]) +
                                 4u * static_cast<uint32_t>(*inputRow) * static_cast<uint32_t>(stride);
            slots[i](master, compInfo, static_cast<int32_t>(src),
                     static_cast<int32_t>(reinterpret_cast<uintptr_t>(
                         reinterpret_cast<const uint8_t*>(cvt) + 0x0C + 4 * i)));
            compInfo += 84;
        }
        cvt->outRow = 0;
    }
    uint32_t n = static_cast<uint32_t>(master->outputRows - cvt->outRow);
    if (n > static_cast<uint32_t>(cvt->pendingRows))
        n = static_cast<uint32_t>(cvt->pendingRows);
    if (n > static_cast<uint32_t>(dstCapacity - *dstCount))
        n = static_cast<uint32_t>(dstCapacity - *dstCount);
    const int32_t result = master->put->putRows(
        master, static_cast<int32_t>(reinterpret_cast<uintptr_t>(cvt) + 0x0C),
        cvt->outRow, dst + 4 * (*dstCount), static_cast<int32_t>(n));
    *dstCount += static_cast<int32_t>(n);
    cvt->pendingRows -= static_cast<int32_t>(n);
    cvt->outRow += static_cast<int32_t>(n);
    if (cvt->outRow >= master->outputRows) {
        ++*inputRow;
        return static_cast<int32_t>(reinterpret_cast<uintptr_t>(inputRow));
    }
    return result;
}

// 0x004287B0: vertical 3:1-weighted upsample. For each output row, two components
// (plane window srcPlanes[-1] / srcPlanes[0] / srcPlanes[1], advancing one pointer per
// row) write interpolated bytes into the row pointers held in *dstRows (a table of
// dwords read as pointers - EA: 32-bit slots, x86 original). Inner trip count is
// planeWidth - 2 (the loop is skipped iff width == 2; width < 2 would never
// terminate - EA: the original hangs there too, so keep the identical structure).
// Returns the row counter: 0 when outputRows <= 0, else the last row index (2 * rows
// processed).
int32_t JpegUpsampleRows(JpegMaster* master, int32_t planeInfo, unsigned __int8** srcPlanes,
                         int32_t** dstRows)
{
    int32_t result = 0;
    uint8_t* table = reinterpret_cast<uint8_t*>(*dstRows);
    const int32_t rows = master->outputRows;
    if (rows > 0) {
        uint8_t** v5 = srcPlanes;
        uint8_t** v25 = srcPlanes;
        int32_t v23 = 0;
        do {
            for (int32_t i = 0; i < 2; ++i) {
                uint8_t* v7 = *v5;
                uint8_t* v8 = (i == 0) ? *(v5 - 1) : v5[1];
                const int32_t v9 = *v7;
                uint8_t* v10 = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(
                    reinterpret_cast<const uint32_t*>(table)[result]));
                uint8_t* v11 = v7 + 1;
                v23 = result + 1;
                int32_t v12 = *v8 + 3 * v9;
                int32_t v13 = v8[1] + 3 * *v11;
                *v10++ = static_cast<uint8_t>((4 * v12 + 8) >> 4);
                *v10 = static_cast<uint8_t>((v12 + v13 + 2 * v12 + 7) >> 4);
                uint8_t* v14 = v8 + 2;
                uint8_t* v15 = v11 + 1;
                uint8_t* v16 = v10 + 1;
                int32_t v17 = v12;
                int32_t v18 = v13;
                const int32_t width = *reinterpret_cast<const int32_t*>(planeInfo + 0x28);
                int32_t v21 = width - 2;
                if (width != 2) {
                    bool done;
                    do {
                        v13 = *v14 + 3 * *v15;
                        *v16 = static_cast<uint8_t>((3 * v18 + v17 + 8) >> 4);
                        uint8_t* v19 = v16 + 1;
                        *v19 = static_cast<uint8_t>((3 * v18 + v13 + 7) >> 4);
                        ++v14;
                        ++v15;
                        v16 = v19 + 1;
                        done = (v21-- == 1);
                        v17 = v18;
                        v18 = v13;
                    } while (!done);
                }
                v5 = v25;
                *v16 = static_cast<uint8_t>((v13 + v17 + 2 * v13 + 8) >> 4);
                v16[1] = static_cast<uint8_t>((4 * v13 + 7) >> 4);
                result = v23;
            }
            v5 = ++v25;
        } while (v23 < rows);
    }
    return result;
}

}  // namespace ts3::misc
