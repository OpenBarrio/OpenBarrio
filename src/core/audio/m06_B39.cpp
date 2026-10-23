#include "ts3/audio/m06_B39.h"
#include <cmath>
#include <cstring>
#include <cfloat>
#include <emmintrin.h>
namespace ts3::audio::b39impl {
uintptr_t CaptureDescriptor() { return 0x0116A0E4; }
uintptr_t MeterDescriptor() { return 0x0116A144; }
uint32_t MeterSize() { return 352; }
uint8_t CaptureInit(Capture* s) {
    s->vtable = reinterpret_cast<void*>(0x0103987C);
    s->internal = s->storage;
    s->begin = s->end = s->storage;
    s->capacity = s->storage + 100;
    s->label = reinterpret_cast<const char*>(0x00F99AC0);
    s->active = 0;
    s->callback = nullptr;
    s->opaque = nullptr;
    return 1;
}
Capture* CaptureDestroy(Capture* s, uint32_t flags) {
    if (s->begin && s->begin != s->internal) CallGameFunction<void, 0x004E3F70>(s->begin);
    s->vtable = reinterpret_cast<void*>(0x0111DB94);
    if (flags & 1) CallGameFunction<void, 0x004E3F70>(s);
    return s;
}
uintptr_t CaptureCommand(Capture* s, int op, void* arg) {
    if (op == 0) {
        auto* p = static_cast<Start*>(arg);
        s->callback = p->callback; s->opaque = p->opaque;
        s->length = p->length; s->rate = p->rate;
        s->active = 1; s->cursor = 0;
        s->node->duration = (FLT_MAX - s->duration) + s->node->duration;
        s->duration = FLT_MAX;
        return reinterpret_cast<uintptr_t>(s->node);
    }
    if (op == 1) {
        s->active = 0;
        s->node->duration = (-0.0f - s->duration) + s->node->duration;
        s->duration = 0;
        uintptr_t result = CallGameFunction<uintptr_t, 0x008B0A70>();
        if (result) {
            const size_t count = s->end - s->begin;
            for (size_t i = 0; i < count; ++i) {
                result = reinterpret_cast<uintptr_t>(s->callback);
                if (s->callback) result = s->callback(s->begin[i], s->opaque);
            }
        } else result = reinterpret_cast<uintptr_t>(s->begin);
        s->end = s->begin;
        return result;
    }
    if (op == 2) {
        auto* p = static_cast<Packets*>(arg);
        uintptr_t result = reinterpret_cast<uintptr_t>(arg);
        for (uint32_t i = 0; i < p->count; ++i) {
            result = reinterpret_cast<uintptr_t>(s->end);
            if (s->end >= s->capacity) result = CallGameMethod<uintptr_t, 0x00B7B1E0>(&s->begin, s->end, p->data + i);
            else { auto* dest = s->end++; if (dest) *dest = p->data[i]; }
        }
        return result;
    }
    return static_cast<uintptr_t>(op - 2);
}
int CaptureProcess(Capture* s, Context* c) {
    if (s->active) {
        s->node->duration = (FLT_MAX - s->duration) + s->node->duration;
        s->duration = FLT_MAX;
        if (s->begin != s->end) {
            float* input[6]; float* output[6];
            for (unsigned ch = 0; ch < s->channels; ++ch) input[ch] = c->input->data + ch * c->input->stride;
            output[0] = c->output->data; output[1] = output[0] + c->output->stride;
            CallGameFunction<void, 0x00ED5E00>(output, input, 1.0f, 2, int(s->channels), 256);
            const int step = int(double(c->rateInfo[3]) / s->rate);
            uint32_t remaining = int32_t(c->count) / step, pos = 0;
            while (remaining && s->begin != s->end) {
                uint32_t n = (s->length - s->cursor) >> 2;
                if (remaining < n) n = remaining;
                auto* dest = reinterpret_cast<int16_t*>(static_cast<uint8_t*>(*s->begin) + s->cursor);
                for (unsigned ch = 0; ch < 2; ++ch) {
                    const float* src = output[ch] + pos;
                    for (uint32_t i = 0; i < n; ++i) {
                        float v = *src; src += step;
                        if (v > 1.0f) v = 1.0f;
                        else if (v < -1.0f) v = -1.0f;
                        // COMISS leaves NaNs unclamped; CVTTSS2SI then produces INT_MIN.
                        dest[i * 2 + ch] = static_cast<int16_t>(_mm_cvttss_si32(_mm_set_ss(v * 32767.0f)));
                    }
                }
                pos += n; s->cursor += 4 * n; remaining -= n;
                if (s->cursor == s->length) {
                    if (s->callback) s->callback(*s->begin, s->opaque);
                    *s->begin = s->end[-1]; --s->end; s->cursor = 0;
                }
            }
        }
    }
    return 1;
}
void* AppendCommand(System* s, uintptr_t a, uintptr_t b) {
    // Native command words grow with pointers, while used remains a byte offset.
    auto* p = reinterpret_cast<uintptr_t*>(s->commands + s->used);
    s->used += 2 * sizeof(uintptr_t);
    p[0] = a; p[1] = b;
    return p;
}
void* MeterReset(Meter* s) {
    for (unsigned i = 0; i < 38; i += 2) s->display[i] = 0;
    for (unsigned i = 0; i < 6; ++i) s->rms[i] = s->last[i] = s->maximum[i] = 0;
    return s->display + 38;
}
void MeterCancel(Meter* s) {
    if (s->live == 1) CallGameMethod<void, 0x00EDB290>(s->system, s->task);
}
namespace {
uint32_t PublishedBits(const float& value) {
    uint32_t bits; std::memcpy(&bits, &value, 4);
    // The original FLD/FSTP pair quiets signaling NaNs without changing the source.
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
    return bits;
}
}
uint8_t MeterPublish(Meter* s) {
    const unsigned n = s->meterChannels;
    if (n == 1) {
        s->display[26] = PublishedBits(s->maximum[0]);
        s->display[14] = PublishedBits(s->last[0]);
        s->display[2] = PublishedBits(s->rms[0]);
    } else {
        const unsigned count = n == 2 ? 2 : n == 4 ? 4 : 6;
        for (unsigned i = 0; i < count; ++i) {
            const unsigned lane = (n == 2 || n == 4) && i ? i + 1 : i;
            s->display[lane * 2] = PublishedBits(s->rms[i]);
            s->display[12 + lane * 2] = PublishedBits(s->last[i]);
            s->display[24 + lane * 2] = PublishedBits(s->maximum[i]);
        }
    }
    return s->meterChannels;
}
void* MeterCommand(Meter* s, int, int) {
    return AppendCommand(s->system, 0x008A5E90, reinterpret_cast<uintptr_t>(s));
}
float* MeterAccumulate(Meter* s, Block* block, int count) {
    float* result = nullptr;
    int cursor = 0;
    for (unsigned ch = 0; ch < s->meterChannels; ++ch) {
        cursor = s->cursor;
        float* p = block->data + ch * block->stride;
        float* end = p + count;
        result = p;
        while (p < end) {
            int n = s->window - cursor;
            if (n > end - p) n = int(end - p);
            int group = n & ~3;
            if (group) {
                bool simd = false;
#if defined(_M_IX86)
                simd = !(reinterpret_cast<uintptr_t>(p) & 15) && *reinterpret_cast<const uint8_t*>(0x011FFF7C);
#endif
                for (int i = 0; i < group; i += 4) {
                    if (simd) {
                        __m128 square = _mm_mul_ps(_mm_load_ps(p), _mm_load_ps(p));
                        __m128 mask = _mm_castsi128_ps(_mm_set_epi32(0, -1, 0, -1));
                        __m128 pair = _mm_and_ps(_mm_shuffle_ps(square, square, 0xB1), mask);
                        __m128 high = _mm_movehl_ps(square, square);
                        __m128 hp = _mm_and_ps(_mm_shuffle_ps(high, high, 0xB1), mask);
                        float peak = _mm_cvtss_f32(_mm_sqrt_ps(_mm_max_ps(_mm_max_ps(square, pair), _mm_max_ps(high, hp))));
                        s->peak[ch] = _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(s->peak[ch]), _mm_set_ss(peak)));
                        s->maximum[ch] = _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(s->maximum[ch]), _mm_set_ss(peak)));
                        float sum = _mm_cvtss_f32(_mm_add_ps(_mm_add_ps(square, pair), _mm_add_ps(high, hp)));
                        s->sum[ch] = sum + s->sum[ch];
                    } else {
                        for (int j = 0; j < 4; ++j) {
                            float v = p[j]; if (!(v > 0)) v *= -1.0f;
                            if (v > s->peak[ch]) s->peak[ch] = v;
                        }
                        if (s->peak[ch] > s->maximum[ch]) s->maximum[ch] = s->peak[ch];
                        for (int j = 0; j < 4; ++j) s->sum[ch] = p[j] * p[j] + s->sum[ch];
                    }
                    p += 4; cursor += 4;
                }
            } else {
                float v = *p; if (!(v > 0)) v *= -1.0f;
                if (v > s->peak[ch]) s->peak[ch] = v;
                if (v > s->maximum[ch]) s->maximum[ch] = v;
                s->sum[ch] = *p * *p + s->sum[ch]; ++p; ++cursor;
            }
            if (cursor >= s->window) {
                cursor = 0; s->last[ch] = s->peak[ch]; s->peak[ch] = 0;
                s->rms[ch] = float(std::sqrt(double(s->sum[ch]) / s->window)); s->sum[ch] = 0;
            }
            result = p;
        }
    }
    s->cursor = cursor;
    return result;
}
int MeterProcess(Meter* s, Context* c) {
    float enabled;
    std::memcpy(&enabled, &s->display[36], sizeof enabled);
    if (enabled != 0.0f) {
        MeterAccumulate(s, c->input, 256);
        MeterPublish(s); s->processed = 1;
    }
    return 1;
}
uint8_t MeterInit(Meter* s) {
    s->vtable = reinterpret_cast<void*>(0x0103A320);
    CallGameMethod<void, 0x00ED72C0>(s->task);
    s->output = s->display; s->live = 0;
    s->window = int(s->system->rate * 0.01f);
    for (unsigned i = 0; i < 38; i += 2) s->display[i] = 0;
    s->processed = 0; MeterReset(s); s->cursor = 0;
    for (unsigned i = 0; i < 6; ++i) s->sum[i] = s->peak[i] = 0;
    const char* name = "VuMeter";
#if defined(_M_IX86)
    name = reinterpret_cast<const char*>(0x0103A330);
#endif
    if (CallGameMethod<uint8_t, 0x00EDB2A0>(reinterpret_cast<uint8_t*>(s->system) + 0x60,
        s->task, uintptr_t(0x008A5E00), s, name, 1, 1)) return 0;
    s->live = 1; return 1;
}
}
