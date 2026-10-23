#include "ts3/anim/m05_B03.h"
#include <cstring>
namespace ts3::anim {
void* __fastcall QueryCEC0(void* s, void*, uint32_t id) {
    return id == 0x33e02b1d || id == 0x73deb8c8 ? s : nullptr;
}
uint32_t __fastcall TypeCF60(void*, void*) { return 0xd3dfc020; }
void* __fastcall QueryCF70(void* s, void*, uint32_t id) {
    return id == 0xd3dfc020 || id == 0x73deb8c8 ? s : nullptr;
}
uint32_t __fastcall TypeCFC0(void*, void*) { return 0xd4e4ccf8; }
void* __fastcall QueryCFD0(void* s, void*, uint32_t id) {
    return id == 0xd4e4ccf8 || id == 0x73deb8c8 ? s : nullptr;
}
// The stream boundary changes the signed cursor; it is not an ordinary std::ostream.
static uintptr_t Write(B03Stream* s, const void* data, uint32_t n, bool bufferResult=false) {
    unsigned char captured[8];
    std::memcpy(captured,data,n);
    s->cursor = static_cast<int32_t>(static_cast<uint32_t>(s->cursor) + n);
    uintptr_t result = 0;
    if (s->cursor >= 0) result = CallGameMethod<uintptr_t,0x00594140>(s,n);
    if (s->cursor < 0) {
        std::memcpy(s->end + s->cursor - (n-1), captured, n);
        result = bufferResult ? reinterpret_cast<uintptr_t>(s->end) : static_cast<uint32_t>(s->cursor);
    }
    return result;
}
uintptr_t __fastcall SaveKeyDC20(B03Stream* s, void*, const B03Key* k) {
    Write(s,&k->word[2],4); Write(s,&k->word[3],4); Write(s,&k->word[0],4);
    return Write(s,&k->word[1],4,true);
}
uint64_t __fastcall ValueDCB0(const void* s, void*) {
    auto p = reinterpret_cast<uintptr_t>(s) + (s ? 0x10 : 8);
    // Volatile preserves the observed null dereference rather than making up a zero key.
    auto w = reinterpret_cast<const volatile uint32_t*>(p);
    return uint64_t(w[0]) | (uint64_t(w[1]) << 32);
}
void __fastcall TimeDCD0(B03Evaluator* s, void*, float v) { std::memcpy(&s->time,&v,4); }
void __fastcall WeightDCF0(B03Evaluator* s, void*, float v) { std::memcpy(&s->weight,&v,4); }
uint8_t __fastcall EnableDD10(B03Evaluator* s, void*, uint8_t v) { s->enabled=v; return v; }
void* __fastcall BindingDD20(B03Evaluator* s, void*, uint32_t row, uint32_t col,
                            uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    auto p = reinterpret_cast<unsigned char*>(s->binding) + uint32_t((row*11+col)*16);
    std::memcpy(p,&a,4); std::memcpy(p+4,&b,4);
    std::memcpy(p+8,&c,4); std::memcpy(p+12,&d,4);
    return p - offsetof(B03Evaluator,binding);
}
uint32_t __fastcall TypeDE50(void*, void*) { return 0x53deec77; }
void* __fastcall QueryDE60(void* s, void*, uint32_t id) {
    return id == 0x53deec77 || id == 0x73deb8c8 ? s : nullptr;
}
uint32_t __fastcall ResourceDE90(B03Evaluator* s, void*, B03Resource* r) {
    auto old = s->resource;
    if (r != old) {
        if (r) CallGameVirtual<void,0>(r);
        s->resource = r;
        if (old) CallGameVirtual<void,1>(old);
    }
    s->key = s->resource->key;
    return (s->key.word[3] & 0xffffff00u) | 1u;
}
static bool Present(const B03Binding& b) {
    return (b.word[0] | b.word[1]) != 0 || b.word[2] != ~0u || b.word[3] != ~0u;
}
uint32_t __fastcall SaveDFE0(B03Evaluator* s, void*, B03Stream* stream) {
#if defined(_M_IX86)
    const auto& empty = *reinterpret_cast<const B03Key*>(0x011cd3f0);
#else
    // Original global is the invalid resource key (all zero); no pointer in this datum.
    const B03Key empty{};
#endif
    uint8_t flags = 0;
    if (std::memcmp(&s->key,&empty,16)) flags |= 1;
    if (std::memcmp(&s->secondKey,&empty,16)) flags |= 2;
    if (s->enabled) flags |= 4;
    if (s->flag) flags |= 8;
    Write(stream,&flags,1);
    if (flags&1) SaveKeyDC20(stream,nullptr,&s->key);
    if (flags&2) SaveKeyDC20(stream,nullptr,&s->secondKey);
    Write(stream,&s->time,4); Write(stream,&s->weight,4);
    uint8_t lastRow=0;
    for (uint8_t row=0;row<5;++row)
        for (uint8_t col=0;col<11;++col)
            if (Present(s->binding[row][col])) { lastRow=row; break; }
    Write(stream,&lastRow,1);
    uint32_t result=0;
    for (uint8_t row=0;row<=lastRow;++row) {
        uint8_t lastCol=0;
        for (uint8_t col=0;col<11;++col) if (Present(s->binding[row][col])) lastCol=col;
        Write(stream,&lastCol,1);
        for (uint8_t col=0;col<=lastCol;++col) {
            auto& b=s->binding[row][col];
            Write(stream,b.word,8); Write(stream,b.word+2,4);
            result=static_cast<uint32_t>(Write(stream,b.word+3,4));
        }
    }
    return result;
}
}
