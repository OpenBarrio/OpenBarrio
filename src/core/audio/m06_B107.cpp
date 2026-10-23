#include "ts3/audio/m06_B107.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <limits>
namespace ts3::audio::b107 {
TS3_X86_SIZE(Segment, 48);
TS3_X86_SIZE(Slot, 80);
TS3_X86_OFFSET(Player, slots, 0x58);
TS3_X86_OFFSET(Player, limit, 0x1c0);
TS3_X86_OFFSET(Player, fade, 0x1cc);
TS3_X86_OFFSET(Context, input, 0x3000c);
TS3_X86_OFFSET(Context, channels, 0x3002c);
TS3_X86_OFFSET(Mix, buffer, 0x24);
TS3_X86_OFFSET(Mix, gains, 0x34);
TS3_X86_OFFSET(Mix, pending, 0x8c);
TS3_X86_OFFSET(Stretch, owned, 0x30);
TS3_X86_OFFSET(Stretch, size, 0x4c);
namespace {
// Descriptor storage is owned by RW. Native execution still requires that owner.
template<uint32_t Address> void* Datum() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(Address);
#else
    AbortNotRewritten("RW named descriptor storage");
    return nullptr;
#endif
}
Segment* Segments(Player* p) {
    return reinterpret_cast<Segment*>(reinterpret_cast<uint8_t*>(p) + p->segmentOffset);
}
float* FadeValues(Player* p) {
    return reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(p) + p->fadeOffset);
}
struct Bits { const uint8_t* data; uint32_t cursor; };
struct Metadata {
    void* resource;
    uint32_t total, frames, word3c, word40, word44, word18;
    uint8_t format;
};
// Explicit x87 single-store operations retain extended intermediate arithmetic on x86.
float Add(float a, float b) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld a
        fadd b
        fstp result
    }
    return result;
#else
    return float(double(a) + double(b));
#endif
}
float Mul(float a, float b) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld a
        fmul b
        fstp result
    }
    return result;
#else
    return float(double(a) * double(b));
#endif
}
float Div(float a, int32_t b) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld a
        fild b
        fdivp st(1), st(0)
        fstp result
    }
    return result;
#else
    return float(double(a) / double(b));
#endif
}
float CopyFloat(float a) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld a
        fstp result
    }
    return result;
#else
    return a;
#endif
}
float Sub(float a, float b) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld a
        fld b
        fsubp st(1), st(0)
        fstp result
    }
    return result;
#else
    return float(double(a) - double(b));
#endif
}
float Blend(float a, float b, float t) {
#if defined(_M_IX86)
    float result;
    __asm {
        fld1
        fld t
        fsubp st(1), st(0)
        fmul a
        fld t
        fmul b
        faddp st(1), st(0)
        fstp result
    }
    return result;
#else
    return float((1.0 - double(t))*double(a) + double(t)*double(b));
#endif
}
}
void* __cdecl PlayerDescriptor() { return Datum<0x011A7534>(); }
void* __cdecl MixDescriptor() { return Datum<0x011A7650>(); }
void* __cdecl StretchDescriptor() { return Datum<0x011A76BC>(); }
uint32_t __fastcall Decode(Player* p, void*, uint32_t index, const uint8_t* data) {
    Slot& slot = p->slots[index];
    Segment& seg = Segments(p)[index];
    Bits bits{data,0};
    uint32_t result = 0;
    const auto read = [&](uint32_t n) {
        result = CallGameMethod<uint32_t,0x008A0AE0>(&bits,n);
        return result;
    };
    read(4);
    slot.codec = uint8_t(read(4));
    seg.channels = uint8_t(read(6) + 1);
    seg.rate = float(read(18));
    slot.mode = uint8_t(read(2));
    const bool loop = uint8_t(read(1)) != 0;
    seg.length = read(29);
    seg.end = loop ? int32_t(read(32)) : -1;
    if (slot.mode == 2) slot.end = read(32);
    if (loop) {
        result = (result & 0xffffff00u) | slot.mode;
        bool keep = slot.mode == 1;
        if (slot.mode == 2) {
            result = uint32_t(seg.end);
            keep = seg.end >= int32_t(slot.end);
        }
        slot.loop = keep ? read(32) : 0;
    }
    slot.data = data + (bits.cursor >> 3);
    return result;
}
uint32_t __fastcall Attach(Player* p, void*, uint32_t index, const uint8_t* data, int32_t bytes) {
    Segment& seg = Segments(p)[index];
    Slot& slot = p->slots[index];
    if (bytes <= 0 || !data) {
        seg.frames = 0;
        slot.word3c = 0;
        slot.word40 = 0;
        slot.resource = nullptr;
        slot.format = 1;
        seg.cursor = 0;
        seg.total = 0;
        return uint32_t(bytes); // word44 and slot.total deliberately untouched
    }
    Metadata meta; // RW fills this observed metadata view.
    CallGameMethod<void,0x00EDC9E0>(&meta,data,bytes);
    seg.frames = meta.frames;
    seg.total = meta.total;
    slot.word3c = meta.word3c;
    slot.word40 = meta.word40;
    slot.resource = meta.resource;
    slot.word44 = meta.word44;
    slot.format = meta.format;
    seg.cursor = 0;
    slot.total = seg.total;
    return (meta.word40 & 0xffffff00u) | meta.format;
}
uint8_t __stdcall Offset(Context* c, double time, uint32_t* frames) {
    // The initial x87 zero comparison treats unordered like a past timestamp.
    if (!(time > c->time)) { *frames = 0; return 1; }
    float delta;
#if defined(_M_IX86)
    const double origin = c->time;
    const float rate = c->config->rate;
    __asm {
        fld time
        fsub origin
        fmul rate
        fstp delta
    }
    const float threshold = *reinterpret_cast<const float*>(0x00FCC24C);
#else
    delta = float((time - c->time) * double(c->config->rate));
    const float threshold = 256.0f;
#endif
    if (delta >= threshold) return 0; // no output write on this path
    const float scaled = Mul(delta,c->scale);
    // x87 FISTP under round-toward-zero, including its integer-indefinite result.
    int64_t value;
#if defined(_M_IX86)
    uint16_t oldCW, truncCW;
    __asm { fnstcw oldCW }
    truncCW = oldCW | 0x0c00;
    __asm {
        fldcw truncCW
        fld scaled
        fistp value
        fldcw oldCW
    }
#else
    value = (!std::isfinite(scaled) || double(scaled) >= 9223372036854775808.0 ||
             double(scaled) < -9223372036854775808.0) ? INT64_MIN : int64_t(scaled);
#endif
    *frames = uint32_t(value);
    return 1;
}
uint32_t __fastcall Fade(Player* p, void*, Context* c) {
    const uint32_t count = std::min<uint32_t>(p->fade, p->limit);
    float* values = FadeValues(p);
    Block* block = c->output;
    for (uint32_t channel=0; channel<p->opaque[0x21]; ++channel) {
        const float step = Div(values[channel],p->fade);
        float* dest = block->samples + block->stride * channel;
        for (uint32_t n=0; n<count; ++n) {
            values[channel] = Sub(values[channel],step);
            dest[n] = values[channel];
        }
    }
    p->fade = uint8_t(p->fade - count);
    std::swap(c->input,c->output);
    c->channels = p->opaque[0x21];
    c->rate = CopyFloat(p->outputRate);
    c->count = count;
    if (p->fade == 0) p->active = 0;
    return 1;
}
uint32_t __cdecl SetLimit(Player* p, void*, void*, uint32_t limit) {
    p->limit = uint16_t(limit);
    return 0;
}
uintptr_t __fastcall Advance(Player* p, void*) {
    p->index = uint8_t(p->index + 1);
    if (p->index == p->count) p->index = 0;
    Segment* seg = &Segments(p)[p->index];
    p->cursor = 0;
    p->length = 0;
    if (seg->type == 4 || seg->type == 0) return reinterpret_cast<uintptr_t>(seg);
    p->cursor = 0;
    p->start = CopyFloat(seg->start);
    p->rate = CopyFloat(seg->rate);
    p->length = seg->length;
    return seg->length;
}
void* __cdecl InitPlayerDescriptor() {
    auto* desc = static_cast<uint8_t*>(PlayerDescriptor());
    void* defaults = Datum<0x011A7628>();
    std::memcpy(desc+0x20,&defaults,sizeof(defaults));
    CallGameFunction<void,0x00ED5AB0>(desc,Datum<0x011A7F40>(),Datum<0x011A75C8>());
    return defaults;
}
uintptr_t __fastcall Unlink(Link* link, void*, const float* gains) {
    Mix* owner = link->owner;
    if (!owner) return 0;
    if (owner->head == link) owner->head = link->next;
    if (link->previous) link->previous->next = link->next;
    if (link->next) link->next->previous = link->previous;
    uintptr_t result = reinterpret_cast<uintptr_t>(link->next);
    if (gains) {
        owner->dirty = 1;
        result = 0x34;
        for (uint32_t i=0; i<owner->channels; ++i) {
            owner->gains[i] = Add(gains[i],owner->gains[i]);
            result += 4;
        }
    }
    link->owner = nullptr;
    link->token = nullptr;
    link->active = 0;
    return result;
}
uint32_t __cdecl ProcessMix(Mix* mix, Context* c) {
    if (mix->pending == 0 && mix->dirty == 0) return 0;
    Block* dest = c->output;
    std::swap(c->input,c->output);
    for (uint32_t channel=0; channel<mix->channels; ++channel) {
        float* target = dest->samples + dest->stride * channel;
        if (mix->pending) std::memcpy(target,mix->buffer + 256*channel,1024);
        else std::memset(target,0,1024);
    }
    mix->pending = 0;
    if (mix->dirty) {
        CallGameFunction<void,0x00ED1210>(dest,mix->gains,uint32_t(mix->channels),uint32_t(256));
        mix->dirty = 0;
    }
    std::memset(mix->buffer,0,size_t(mix->channels)*1024);
    return 1;
}
void* __cdecl InitMixDescriptor() {
    auto* desc = static_cast<uint8_t*>(MixDescriptor());
    void* defaults = Datum<0x011A7684>();
    std::memcpy(desc+0x20,&defaults,sizeof(defaults));
    CallGameFunction<void,0x00ED5AB0>(desc,Datum<0x011A7688>(),static_cast<void*>(nullptr));
    return defaults;
}
uintptr_t __fastcall FreeStretch(Stretch* p, void*) {
    if (!p->owned) return 0;
    const uintptr_t result = CallGameMethod<uintptr_t,0x00ED7230>(p->allocator,p->owned,uint32_t(0));
    p->owned = nullptr;
    return result;
}
uint32_t __fastcall Crossfade(Stretch* p, void*, const float* a, const float* b, float* out, int32_t shift) {
    const uint32_t amount = shift > 0 ? uint32_t(shift) : 0u - uint32_t(shift);
    float t = shift > 0 ? 1.0f : 0.0f;
    // Positive shifts use -1/size; negative shifts use +1/size.
    const float step = Div(shift > 0 ? -1.0f : 1.0f,p->size);
    for (int32_t i=int32_t(amount); i<p->size; ++i) {
        out[i-int32_t(amount)] = Blend(a[i-int32_t(amount)],a[i],t);
        t = Add(t,step);
    }
    for (uint32_t i=0; i<amount; ++i) {
        const int32_t dest = p->size - int32_t(amount) + int32_t(i);
        out[dest] = Blend(a[dest],b[i],t);
        t = Add(t,step);
    }
    return amount;
}
}
