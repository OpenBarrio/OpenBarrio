// TS3W.exe 0x004853F0..0x00489050 + 0x004A08E0..0x004A14C0 (unit cls_4853f0_x).
// The decompiler's stack-variable names in the provenance comments are its own; the
// control flow and the field offsets were re-checked against the disassembly.
#include "ts3/misc/cls_4853f0_x.h"

#include <cstring>

namespace ts3::misc {
namespace {

// Class descriptors stored by the AST parser/allocator (x86 game addresses).
constexpr uintptr_t kClassBase = 0x00FB1F3C;
constexpr uintptr_t kClassNot = 0x00FB1F6C;
constexpr uintptr_t kClassAnd = 0x00FB1F74;
constexpr uintptr_t kClassOr = 0x00FB1F7C;
constexpr uintptr_t kClassWord = 0x00FB1F84;
// The boolean keywords the parser matches (wide strings in .rdata at 0xFB1F8C..0xFB1FA4).
constexpr char16_t kKeywordClose[] = u")";
constexpr char16_t kKeywordOpen[] = u"(";
constexpr char16_t kKeywordNot[] = u"not";
constexpr char16_t kKeywordAnd[] = u"and";
constexpr char16_t kKeywordOr[] = u"or";
// Cursor error codes the original writes to +0xC on failure.
constexpr int32_t kQueryError6 = -798294010;  // 0xD06B0006
constexpr int32_t kQueryError7 = -798294009;  // 0xD06B0007

// Case-insensitive wide comparison with the same result as the CRT wcsicmp the game calls
// through the IAT slot 0x00F95758 (ASCII keywords; non-ASCII falls back to a plain
// code-unit comparison, which cannot matter for the keywords used here).
int CompareWideICase(const char16_t* a, const char16_t* b) {
    for (;;) {
        char16_t ca = *a++;
        char16_t cb = *b++;
        if (ca >= u'A' && ca <= u'Z') ca = static_cast<char16_t>(ca + 32);
        if (cb >= u'A' && cb <= u'Z') cb = static_cast<char16_t>(cb + 32);
        if (ca != cb) return ca < cb ? -1 : 1;
        if (!ca) return 0;
    }
}

bool IsKeyword(const char16_t* token, const char16_t* keyword) {
    return CompareWideICase(token, keyword) == 0;
}

// The four cdecl readers 0x4859C0 selects by the block type byte.
uint32_t ReadBlockValue1(Stream* s) { return static_cast<uint32_t>(CallGameFunction<int32_t, 0x00484BA0>(s)); }
uint32_t ReadBlockValue2(Stream* s) { return static_cast<uint32_t>(CallGameFunction<int32_t, 0x00484C50>(s)); }
uint32_t ReadBlockValue3(Stream* s) { return static_cast<uint32_t>(CallGameFunction<int32_t, 0x00484DC0>(s)); }
uint32_t ReadBlockValue4(Stream* s) { return static_cast<uint32_t>(CallGameFunction<int32_t, 0x00484FE0>(s)); }

#if defined(_M_IX86)
// 0x004852B0 reads the stream from EDI (usercall; the bridge cannot pass a register
// argument). Used by 0x485BC0 and 0x485D90 for tag 30. The callee's own frame saves
// EBX/EBP/ESI but not EDI, so the thunk saves EBX/ESI/EDI itself.
__declspec(naked) int32_t __cdecl ReadRandom4852B0(void* /*stream*/) {
    __asm {
        push ebx
        push esi
        push edi
        mov edi, [esp + 0x10]      // the stream argument ([E+4])
        mov eax, 0x004852B0
        call eax
        pop edi
        pop esi
        pop ebx
        ret
    }
}
// 0x00488D50 reads the loader from EAX (usercall): (self, int* out, int code) -> AL.
// Used by 0x488E30. Its cdecl args are already in the right stack order behind the
// register one, so the thunk only has to load EAX and call.
__declspec(naked) int32_t __cdecl LookupSeed488D50(void* /*loader*/, int32_t* /*out*/, int32_t /*code*/) {
    __asm {
        push ebx
        push esi
        push edi
        mov eax, [esp + 0x10]      // loader
        push dword ptr [esp + 0x18]  // code
        push dword ptr [esp + 0x18]  // out (now one slot lower)
        mov ecx, 0x00488D50
        call ecx
        add esp, 8
        movzx eax, al
        pop edi
        pop esi
        pop ebx
        ret
    }
}
// 0x00481580 / 0x004815B0 read their object from EAX (usercall, no stack args). 0x488E30
// relies on EAX still holding 0x488A60's return value, so the value must arrive in EAX.
__declspec(naked) int32_t __cdecl Call481580WithEax(uintptr_t /*self*/) {
    __asm {
        push ebx
        push esi
        push edi
        mov eax, [esp + 0x10]      // self
        mov ecx, 0x00481580
        call ecx
        pop edi
        pop esi
        pop ebx
        ret
    }
}
__declspec(naked) int32_t __cdecl Call4815B0WithEax(uintptr_t /*self*/) {
    __asm {
        push ebx
        push esi
        push edi
        mov eax, [esp + 0x10]      // self
        mov ecx, 0x004815B0
        call ecx
        pop edi
        pop esi
        pop ebx
        ret
    }
}
#else
int32_t ReadRandom4852B0(void*) { AbortNotRewritten("0x4852B0 (stream in EDI)"); return 0; }
int32_t LookupSeed488D50(void*, int32_t*, int32_t) { AbortNotRewritten("0x488D50 (loader in EAX)"); return 0; }
int32_t Call481580WithEax(uintptr_t) { AbortNotRewritten("0x481580 (object in EAX)"); return 0; }
int32_t Call4815B0WithEax(uintptr_t) { AbortNotRewritten("0x4815B0 (object in EAX)"); return 0; }
#endif

// Reads one byte through the stream's memory window or its read() callback, matching the
// original inline sequence (0x495E80 refills the window, 0x49AE20 reports read errors).
uint8_t ReadStreamByte(Stream* s) {
    if (s->base) {
        if (s->read) {
            if (s->position - s->windowStart + 1 > s->capacity)
                CallGameFunction<void, 0x00495E80>(s);
            const uint8_t value = s->base[s->position - s->windowStart];
            ++s->position;
            return value;
        }
        const uint8_t value = s->base[s->position];
        s->position = s->position + 1;
        return value;
    }
    const uint32_t position = s->position;
    s->position = position + 1;
    if (s->read(s->ctx, s->inlineBuffer, static_cast<int32_t>(position), 1) >= 0)
        return s->inlineBuffer[0];
    CallGameFunction<void, 0x0049AE20>(s->errorSystem, 10024);
    return 0;
}

// The bump arena behind 0x4A0B40: +8 limit, +0xC cursor, +0x10 high-water mark. Allocation
// either fits and bumps, or calls the game's grow method 0x4E8650 (thiscall, one size arg).
struct QueryArena {
    uintptr_t reserved0;
    uintptr_t reserved1;
    uintptr_t limit;   // +0x08
    uintptr_t cursor;  // +0x0C
    uintptr_t high;    // +0x10
};

bool ArenaAllocate(void* arenaRaw, uint32_t size, void** out) {
    auto* arena = static_cast<QueryArena*>(arenaRaw);
    if (static_cast<intptr_t>(arena->limit - arena->cursor - size) >= 0 ||
        CallGameMethod<bool, 0x004E8650>(arena, size)) {
        const uintptr_t cursor = arena->cursor;
        arena->cursor = cursor + size;
        arena->high = cursor + size;
        *out = reinterpret_cast<void*>(cursor);
        return true;
    }
    *out = nullptr;
    return false;
}

// bool thiscall slot 1 of a node's class descriptor (the AST Test()).
bool QueryNodeTest(void* node, int32_t value) {
    void** klass = *reinterpret_cast<void***>(node);
    auto test = reinterpret_cast<bool(__thiscall*)(void*, int32_t)>(klass[1]);
    return test(node, value);
}

}  // namespace

// 0x004853F0: read the next value from `stream` for the record tag `tag`.
int32_t ReadStreamValue(Stream* stream, int32_t tag) {
    if (tag == 28) {  // two big-endian bytes; BYTE1(v) holds the first one
        uint32_t value = static_cast<uint32_t>(ReadStreamByte(stream)) << 8;
        value |= ReadStreamByte(stream);
        return static_cast<int32_t>(value);
    }
    if (tag == 29) {  // four big-endian bytes
        uint32_t value = static_cast<uint32_t>(ReadStreamByte(stream)) << 8;
        value = (value | ReadStreamByte(stream)) << 8;
        value = (value | ReadStreamByte(stream)) << 8;
        value |= ReadStreamByte(stream);
        return static_cast<int32_t>(value);
    }
    if (tag <= 246) return tag - 139;
    if (tag > 250) {
        if (tag <= 254) {
            const uint8_t byte = ReadStreamByte(stream);
            return 64148 - (tag << 8) - byte;
        }
        return 0;  // 255 (and anything above): the original leaves EAX 0
    }
    const uint8_t byte = ReadStreamByte(stream);  // 247..250
    return (tag << 8) + byte - 63124;
}

// 0x004859C0: allocate and fill the 20-byte block index from the stream.
Block* BuildBlockIndex(Stream* stream, void* pool) {
    auto* block = static_cast<Block*>(CallGameFunction<void*, 0x0049AFC0>(pool, 20));
    block->pool = pool;
    block->entries = nullptr;
    const auto count = static_cast<uint16_t>(CallGameFunction<int32_t, 0x00495F60>(stream));
    block->count = count;
    if (count) {
        block->type = ReadStreamByte(stream);
        block->entries = static_cast<uint32_t*>(
            CallGameFunction<void*, 0x0049AFC0>(pool, static_cast<uint32_t>(4 * count + 4)));
        uint32_t (*reader)(Stream*) = ReadBlockValue4;
        switch (block->type) {
            case 1: reader = ReadBlockValue1; break;
            case 2: reader = ReadBlockValue2; break;
            case 3: reader = ReadBlockValue3; break;
            default: reader = ReadBlockValue4; break;
        }
        for (uint32_t i = 0; i <= count; ++i) block->entries[i] = reader(stream);
        int32_t position = CallGameFunction<int32_t, 0x00496390>(stream);
        block->offset = static_cast<uint32_t>(--position);
        CallGameFunction<int32_t, 0x00496360>(
            stream, position + static_cast<int32_t>(block->entries[count]));
    }
    return block;
}

// 0x00485B20: read one entry's bytes (index is bounds-checked against the u16 count).
void* ReadBlockEntry(int32_t index, Block* block, Stream* stream) {
    void* result = nullptr;
    if (index >= 0 && index < static_cast<int32_t>(block->count)) {
        const int32_t saved = CallGameFunction<int32_t, 0x00496390>(stream);
        CallGameFunction<int32_t, 0x00496360>(
            stream, static_cast<int32_t>(block->offset + block->entries[index]));
        const uint32_t size = block->entries[index + 1] - block->entries[index];
        auto* buffer = static_cast<uint8_t*>(
            CallGameFunction<void*, 0x0049AFC0>(block->pool, size + 2));
        CallGameFunction<int32_t, 0x00496000>(stream, buffer, size);
        buffer[size] = 0;
        buffer[size + 1] = 0;
        CallGameFunction<int32_t, 0x00496360>(stream, saved);
        result = buffer;
    }
    return result;
}

// 0x00485B90: the loader's +0x374 block entry 0x238, only for kind 4.
void* LoaderEntryForKind(void* loaderRaw, int32_t /*unused*/, int16_t kind) {
    auto* loader = static_cast<Loader*>(loaderRaw);
    if (kind == 4 && loader->block374)
        return ReadBlockEntry(static_cast<int32_t>(loader->index238), loader->block374, loader->stream);
    return nullptr;
}

// 0x00485BC0: rebuild the loader's event index (fields 0x4FC..0x508), restoring the stream.
int32_t RebuildLoaderIndex(Loader* loader) {
    Stream* stream = loader->stream;
    const int32_t saved = CallGameFunction<int32_t, 0x00496390>(stream);
    loader->field4fc = 0;
    loader->field500 = 0;
    loader->field504 = 0;
    loader->field508 = 0;
    CallGameFunction<int32_t, 0x00496360>(
        stream, static_cast<int32_t>(loader->baseOffset + loader->field3c0));
    const uint32_t endPosition = loader->baseOffset + loader->field3c0 + loader->field3bc;
    uint32_t values[64];
    int32_t count = 0;
    while (static_cast<uint32_t>(CallGameFunction<int32_t, 0x00496390>(stream)) < endPosition) {
        const uint8_t byte = ReadStreamByte(stream);
        if (byte > 0x1Bu && byte != 31) {
            if (byte == 30) values[count] = static_cast<uint32_t>(ReadRandom4852B0(stream));
            else values[count] = static_cast<uint32_t>(ReadStreamValue(stream, byte));
            ++count;
            continue;
        }
        if (byte != 12) {
            const int32_t base = byte - 19;
            if (base) {
                const int32_t second = base - 1;
                if (second) {
                    if (second == 1) loader->field508 = values[0];
                } else {
                    loader->field504 = values[0];
                }
            } else {
                loader->field4fc = values[0];
            }
        }
        count = 0;  // LABEL_52
    }
    if (loader->field4fc) loader->field500 = loader->baseOffset + loader->field4fc + loader->field3c0;
    return CallGameFunction<int32_t, 0x00496360>(stream, saved);
}

// 0x00485D90: parse the record table of one block entry into the fixed `Record` layout.
int32_t ParseLoaderRecord(int32_t index, Stream* stream, Block* block, Record* out) {
    const int32_t saved = CallGameFunction<int32_t, 0x00496390>(stream);
    out->field18 = 0;
    out->field10 = 0;
    out->field1c = 0;
    out->field14 = 0;
    out->field24 = 0;
    out->field160 = 0;
    out->field164 = 0;
    out->field34 = 0;
    out->field38 = 0;
    out->field40 = 0;
    out->field44 = 0;
    out->field48 = 0;
    out->field50 = 0;
    out->field54 = nullptr;
    out->field20 = 0;
    out->byte16c = 0;
    out->field28 = static_cast<uint32_t>(-100);
    out->field2c = 50;
    out->field15c = 0x10000;
    out->field168 = 0x10000;
    out->field4c = 1;
    out->byte30 = 2;
    CallGameFunction<int32_t, 0x00496360>(
        stream, static_cast<int32_t>(block->offset + block->entries[index]));
    const uint32_t endPosition = block->offset + block->entries[index + 1];
    int32_t count = 0;
    uint32_t values[64];
    if (static_cast<uint32_t>(CallGameFunction<int32_t, 0x00496390>(stream)) < endPosition) {
        uint32_t current = 0;  // EA bug: the original's first `current` is an uninitialised stack
                               // slot; a record always starts with a value token, which fills it.
        do {
            const uint8_t byte = ReadStreamByte(stream);
            if (byte > 0x1Bu && byte != 31) {
                if (byte == 30) values[count] = static_cast<uint32_t>(ReadRandom4852B0(stream));
                else values[count] = static_cast<uint32_t>(ReadStreamValue(stream, byte));
                current = values[0];
                ++count;
                continue;
            }
            if (byte != 12) {
                switch (byte) {
                    case 0: out->word00 = static_cast<uint16_t>(current); break;
                    case 1: out->word02 = static_cast<uint16_t>(current); break;
                    case 2: out->word04 = static_cast<uint16_t>(current); break;
                    case 3: out->word06 = static_cast<uint16_t>(current); break;
                    case 4: out->word08 = static_cast<uint16_t>(current); break;
                    case 5:
                        out->field10 = current;
                        out->field14 = values[1];
                        out->field18 = values[2];
                        out->field1c = values[3];
                        break;
                    case 0xD: out->field0c = current; break;
                    case 0xF: out->field34 = current; break;
                    case 0x10: out->field38 = current; break;
                    case 0x11: out->field3c = current; break;
                    case 0x12:
                        out->field40 = current;
                        out->field44 = values[1];
                        break;
                    default: break;
                }
                count = 0;
                continue;
            }
            const uint8_t field = ReadStreamByte(stream);
            switch (field) {
                case 1: out->field20 = current; break;
                case 2: out->field24 = current; break;
                case 3: out->field28 = current; break;
                case 4: out->field2c = current; break;
                case 6: out->byte30 = static_cast<uint8_t>(current); break;
                case 7: goto label39;
                case 24: {
                    out->field48 = static_cast<uint32_t>(count - 4);
                    out->field4c = current;
                    const int32_t copied = count - 4;
                    for (int32_t i = 0; i < copied; ++i) out->data[i] = values[1 + i];
                    const uint32_t arrayCount = values[count - 3];
                    out->field50 = arrayCount;
                    out->field54 = CallGameFunction<void*, 0x0049AFC0>(
                        block->pool, static_cast<uint32_t>(4) * arrayCount);
                    auto* bytes = reinterpret_cast<uint8_t*>(out);
                    *reinterpret_cast<uint16_t*>(bytes + 0x98) = static_cast<uint16_t>(values[count - 2]);
                    *reinterpret_cast<uint16_t*>(bytes + 0x9A) = static_cast<uint16_t>(values[count - 1]);
                    goto label39;
                }
                case 30: out->byte16c = 1; break;
                default: break;
            }
            count = 0;
            continue;
        label39:
            out->field15c = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B2F0>(static_cast<int32_t>(current), 65536000));
            out->field164 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B2F0>(static_cast<int32_t>(values[1]), 65536000));
            out->field160 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B2F0>(static_cast<int32_t>(values[2]), 65536000));
            out->field168 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B2F0>(static_cast<int32_t>(values[3]), 65536000));
            count = 0;
        } while (static_cast<uint32_t>(CallGameFunction<int32_t, 0x00496390>(stream)) < endPosition);
    }
    return CallGameFunction<int32_t, 0x00496360>(stream, saved);
}

// 0x00488930: free every list the loader owns and then the loader itself.
int32_t DestroyLoader(Loader* loader) {
    if (!loader) return 0;  // the original returns its uninitialised `result` here
    auto freeBlock = [](Block* block) {
        if (!block) return;
        CallGameFunction<void, 0x0049B1A0>(block->pool, block->entries);
        CallGameFunction<void, 0x0049B1A0>(block->pool, block);
    };
    freeBlock(loader->block374);
    CallGameFunction<void, 0x0049B1A0>(loader->pool, loader->free3d0);
    freeBlock(loader->block378);
    freeBlock(loader->block4ec);
    freeBlock(loader->block4f0);
    freeBlock(loader->block4f8);
    freeBlock(loader->block50c);
    CallGameFunction<void, 0x0049B9F0>(loader->freeList214);
    CallGameFunction<void, 0x0049B9F0>(loader->freeList218);
    CallGameFunction<void, 0x0049B1A0>(loader->pool, loader->entryTable);
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(
        CallGameFunction<void*, 0x0049B1A0>(loader->pool, loader)));
}

// 0x00488C90: shell sort of `count` 4-byte {u16 key; u16 value} records by key.
int32_t SortLoaderEntries(void* recordsRaw, int32_t count) {
    auto* records = static_cast<uint8_t*>(recordsRaw);
    auto key = [records](int32_t at) -> uint16_t& {
        return *reinterpret_cast<uint16_t*>(records + 4 * at);
    };
    int32_t result = count - (count >> 31);
    for (int32_t gap = count / 2; gap > 0 && gap < count; gap /= 2) {
        for (int32_t at = gap; at < count; ++at) {
            int32_t left = at - gap;
            while (left >= 0) {
                if (key(left) <= key(left + gap)) break;
                const uint32_t a = *reinterpret_cast<uint32_t*>(records + 4 * left);
                const uint32_t b = *reinterpret_cast<uint32_t*>(records + 4 * (left + gap));
                *reinterpret_cast<uint32_t*>(records + 4 * left) = b;
                *reinterpret_cast<uint32_t*>(records + 4 * (left + gap)) = a;
                left -= gap;
            }
        }
        result = gap / 2;
    }
    return result;
}

// 0x00488E30: initialise the loader's random seeds and the min/max span fields.
int32_t InitLoader(Loader* loader) {
    const uint32_t seed = loader->random4d8;
    loader->entryCount = loader->block4f8->count;
    const int32_t first = CallGameFunction<int32_t, 0x0049B430>(65536000, seed);
    const uint32_t base = loader->random4d8;
    loader->field21c = static_cast<uint32_t>(first >> 16);
    loader->random4d8 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B430>(static_cast<int32_t>(base), static_cast<int32_t>(base)));
    loader->random4dc = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B430>(
        static_cast<int32_t>(loader->random4dc), static_cast<int32_t>(base)));
    loader->random4e0 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B430>(
        static_cast<int32_t>(loader->random4e0), static_cast<int32_t>(base)));
    loader->random4e4 = static_cast<uint32_t>(CallGameFunction<int32_t, 0x0049B430>(
        static_cast<int32_t>(loader->random4e4), static_cast<int32_t>(base)));
    loader->low224 = 0x7FFF;
    loader->high228 = 0x7FFF;
    loader->pad220 = 0;
    loader->field234 = loader->field3a0;
    int32_t value8 = 0;
    int32_t value10 = 0;
    int32_t seedOut = 0;
    uint8_t seedAux[4] = {0, 0, 0, 0};
    const bool memoryMode = loader->byte4e8 != 0;
    loader->field51c = 1;
    if (!memoryMode) {
        if (loader->wordDc == 0) {
            loader->field51c = 0;
        } else if (LookupSeed488D50(loader, &seedOut, loader->wordDc)) {
            value8 = seedOut;
        } else {
            loader->field51c = 0;
        }
    } else if (LookupSeed488D50(loader, &seedOut, 102)) {
        value8 = seedOut;
    } else {
        loader->field51c = 0;
    }
    seedOut = 0;
    loader->field51c = 1;
    if (!memoryMode) {
        if (loader->wordDe == 0) {
            loader->field51c = 0;
        } else if (LookupSeed488D50(loader, &seedOut, loader->wordDe)) {
            value10 = seedOut;
        } else {
            loader->field51c = 0;
        }
    } else if (LookupSeed488D50(loader, &seedOut, 103)) {
        value10 = seedOut;
    } else {
        loader->field51c = 0;
    }
    if (loader->low224 == 0x7FFF) {
        if (static_cast<uint16_t>(value8)) {
            const int32_t handle = CallGameFunction<int32_t, 0x00488A60>(
                loader, value8, &seedOut, seedAux);
            if (handle) {
                loader->low224 = static_cast<int16_t>(Call481580WithEax(static_cast<uintptr_t>(handle)));
                CallGameFunction<void, 0x0047AF20>(handle);
            }
        }
    }
    if (loader->high228 == 0x7FFF) {
        if (static_cast<uint16_t>(value10)) {
            const int32_t handle = CallGameFunction<int32_t, 0x00488A60>(
                loader, value10, &seedOut, seedAux);
            if (handle) {
                loader->high228 = static_cast<int16_t>(Call4815B0WithEax(static_cast<uintptr_t>(handle)));
                CallGameFunction<void, 0x0047AF20>(handle);
            }
        }
    }
    const int32_t center = static_cast<int32_t>(loader->field21c);
    const bool lowDefaulted = loader->low224 == 0x7FFF;
    loader->recordSeed = static_cast<uint32_t>(center);
    loader->pad220 = 32;
    if (lowDefaulted) loader->low224 = 750;
    if (loader->high228 == 0x7FFF) loader->high228 = -250;
    const int32_t result = loader->high228 + center - loader->low224;
    loader->span22c = result;
    if (result < 0) loader->span22c = 0;
    return result;
}

// ---------------------------------------------------------------------------
// Group B: query-expression AST.
// ---------------------------------------------------------------------------

// 0x004A0B30: store class descriptor 0x00FB1F3C at +0 and return this.
QueryNode* QueryNode::SetClass(int32_t /*unused*/) {
#if defined(_M_IX86)
    klass = reinterpret_cast<void*>(kClassBase);
#endif
    return this;
}

// 0x004A0980: consume the current token when it equals `keyword`.
bool MatchQueryToken(QueryCursor* cursor, const char16_t* keyword) {
    if (cursor->token == cursor->end) return false;
    if (!IsKeyword(*cursor->token, keyword)) return false;
    ++cursor->token;
    return true;
}

// 0x004A0B40: parse "not" <primary>, "(" <or-chain> ")" or a word node.
QueryNode* ParseQueryPrimary(QueryCursor* cursor) {
    if (cursor->token != cursor->end && IsKeyword(*cursor->token, kKeywordNot)) {
        ++cursor->token;
        QueryNode* child = ParseQueryPrimary(cursor);
        if (child) {
            void* block = nullptr;
            if (ArenaAllocate(cursor->arena, 8, &block)) {
                if (block) {
                    auto* node = static_cast<QueryNotNode*>(block);
                    node->child = child;
#if defined(_M_IX86)
                    node->klass = reinterpret_cast<void*>(kClassNot);
#endif
                    return node;
                }
            }
            return nullptr;
        }
        cursor->error = kQueryError6;
        return nullptr;
    }
    if (cursor->token != cursor->end && IsKeyword(*cursor->token, kKeywordOpen)) {
        ++cursor->token;
        QueryNode* inner = ParseQueryOr(cursor);
        if (!inner) {
            cursor->error = kQueryError6;
            return nullptr;
        }
        if (MatchQueryToken(cursor, kKeywordClose)) return inner;
        cursor->error = kQueryError7;
        return nullptr;
    }
    if (cursor->token == cursor->end) return nullptr;
    const char16_t* word = *cursor->token;
    uint32_t length = 0;
    while (word[length]) ++length;
    // (2*len + 9) & ~7 == 2*(len+1) rounded up to 8; the original copies even for a null
    // arena pointer (EA bug), which we keep.
    const uint32_t size = (2 * length + 9) & 0xFFFFFFF8u;
    void* buffer = nullptr;
    ArenaAllocate(cursor->arena, size, &buffer);
    auto* copy = static_cast<char16_t*>(buffer);
    for (uint32_t i = 0; i <= length; ++i) copy[i] = word[i];
    auto* node = static_cast<QueryWordNode*>(
        CallGameFunction<void*, 0x004A0A80>(8u, cursor->arena));
    if (node) {
        node->word = copy;
#if defined(_M_IX86)
        node->klass = reinterpret_cast<void*>(kClassWord);
#endif
        ++cursor->token;
        return node;
    }
    ++cursor->token;
    return nullptr;
}

// 0x004A0CA0: parse primary ("and" primary)*.
QueryNode* ParseQueryAnd(QueryCursor* cursor) {
    QueryNode* result = ParseQueryPrimary(cursor);
    for (;;) {
        while (cursor->token != cursor->end && IsKeyword(*cursor->token, kKeywordAnd)) {
            ++cursor->token;
            QueryNode* right = ParseQueryPrimary(cursor);
            if (!right) {
                cursor->error = kQueryError6;
                return nullptr;
            }
            void* block = nullptr;
            if (ArenaAllocate(cursor->arena, 16, &block) && block) {
                auto* node = static_cast<QueryAndNode*>(block);
#if defined(_M_IX86)
                node->klass = reinterpret_cast<void*>(kClassAnd);
#endif
                node->left = result;
                node->right = right;
                result = node;
                continue;  // the original jumps back to the token test
            }
            result = nullptr;  // EA keeps looping (0x4A0D15), it does not return here
            continue;
        }
        return result;
    }
}

// 0x004A0D30: parse and-chain ("or" and-chain)*.
QueryNode* ParseQueryOr(QueryCursor* cursor) {
    QueryNode* result = ParseQueryAnd(cursor);
    for (;;) {
        while (cursor->token != cursor->end && IsKeyword(*cursor->token, kKeywordOr)) {
            ++cursor->token;
            QueryNode* right = ParseQueryAnd(cursor);
            if (!right) {
                cursor->error = kQueryError6;
                return nullptr;
            }
            void* block = nullptr;
            if (ArenaAllocate(cursor->arena, 16, &block) && block) {
                auto* node = static_cast<QueryOrNode*>(block);
#if defined(_M_IX86)
                node->klass = reinterpret_cast<void*>(kClassOr);
#endif
                node->left = result;
                node->right = right;
                result = node;
                continue;  // the original jumps back to the token test
            }
            result = nullptr;  // EA keeps looping (0x4A0DA5), it does not return here
            continue;
        }
        return result;
    }
}

// 0x004A1340: FNV-1 lookup of *key; writes {node, bucket slot} to `out` and returns `out`.
// 0x4A11E0 (the bucket-chain search) is the original instruction sequence and takes the map in ECX without reading it,
// so it goes through the thiscall bridge with the map exactly like the original call site.
void** QueryMapFind(QueryMap* map, void** out, const char16_t** key) {
    const char16_t* text = *key;
    uint32_t hash = 0x811C9DC5u;
    for (uint32_t i = 0; text[i]; ++i) {
        hash *= 0x01000193u;
        hash ^= static_cast<uint32_t>(text[i]);
    }
    const uint32_t slot = hash % map->count;
    void* node = CallGameMethod<void*, 0x004A11E0>(map, map->buckets[slot], key, hash);
    if (node) {
        out[0] = node;
        out[1] = &map->buckets[slot];
    } else {
        out[0] = map->buckets[map->count];
        out[1] = &map->buckets[map->count];
    }
    return out;
}

// 0x004A14C0: the word node's Test() - is `word` present in `map`?
bool QueryWordNodeTest(QueryWordNode* node, QueryMap* map) {
    void* sentinel = map->buckets[map->count];
    void* out[2] = {nullptr, nullptr};
    QueryMapFind(map, out, &node->word);
    return out[0] != sentinel;
}

// 0x004A08E0: !child->Test(value) (only AL is used by the original).
bool QueryNotNodeTest(QueryNotNode* node, int32_t value) {
    return !QueryNodeTest(node->child, value);
}

// 0x004A0900: left->Test(value) && right->Test(value).
bool QueryAndNodeTest(QueryAndNode* node, int32_t value) {
    return QueryNodeTest(node->left, value) && QueryNodeTest(node->right, value);
}

// 0x004A0940: left->Test(value) || right->Test(value).
bool QueryOrNodeTest(QueryOrNode* node, int32_t value) {
    return QueryNodeTest(node->left, value) || QueryNodeTest(node->right, value);
}

// ---------------------------------------------------------------------------
// Game-ABI entries for the harness replace hooks: the four entry points whose caller
// passes the object in a register (ESI/EAX/ECX) and cleans the stack itself (plain ret).
// MSVC inline asm can only call through a pointer, hence the static body pointers.
// ---------------------------------------------------------------------------
#if defined(_M_IX86)
namespace {
Block* (__cdecl* const kBuildBlockIndexBody)(Stream*, void*) = &BuildBlockIndex;
int32_t (__cdecl* const kParseLoaderRecordBody)(int32_t, Stream*, Block*, Record*) = &ParseLoaderRecord;
int32_t (__cdecl* const kSortLoaderEntriesBody)(void*, int32_t) = &SortLoaderEntries;
int32_t (__cdecl* const kInitLoaderBody)(Loader*) = &InitLoader;
}  // namespace

// 0x004859C0: stream in ESI, pool on the stack (caller cleans).
__declspec(naked) Block* __cdecl BuildBlockIndexEntry(void) {
    __asm {
        push dword ptr [esp + 4]
        push esi
        call dword ptr [kBuildBlockIndexBody]
        add esp, 8
        ret
    }
}
// 0x00485D90: record index in EAX, stream in ECX, block/out on the stack (caller cleans).
__declspec(naked) int32_t __cdecl ParseLoaderRecordEntry(void) {
    __asm {
        mov edx, [esp + 8]
        push edx
        push dword ptr [esp + 8]
        push ecx
        push eax
        call dword ptr [kParseLoaderRecordBody]
        add esp, 16
        ret
    }
}
// 0x00488C90: records in ESI, count on the stack (caller cleans).
__declspec(naked) int32_t __cdecl SortLoaderEntriesEntry(void) {
    __asm {
        push dword ptr [esp + 4]
        push esi
        call dword ptr [kSortLoaderEntriesBody]
        add esp, 8
        ret
    }
}
// 0x00488E30: loader in ESI (nothing is pushed).
__declspec(naked) int32_t __cdecl InitLoaderEntry(void) {
    __asm {
        push esi
        call dword ptr [kInitLoaderBody]
        add esp, 4
        ret
    }
}
#endif

}  // namespace ts3::misc
