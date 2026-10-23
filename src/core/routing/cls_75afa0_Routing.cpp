#include "ts3/routing/cls_75afa0_Routing.h"
#include <xmmintrin.h>
#include <cmath>
#include <cstring>

namespace ts3::routing {
namespace {
constexpr float tau = 6.283185482025146484375f;
constexpr float pi = 3.1415927410125732421875f;
constexpr float inverseTau = 0.15915493667125701904296875f;
__m128 Horizontal(__m128 v) {
    alignas(16) float lanes[4];
    _mm_store_ps(lanes, v);
    lanes[1] = 0.0f;
    return _mm_load_ps(lanes);
}
__m128 Sum3(__m128 v) {
    return _mm_shuffle_ps(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2),
        _mm_shuffle_ps(v,v,1)),v),_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2),
        _mm_shuffle_ps(v,v,1)),v),0);
}
// Preserve EA's approximate reciprocal square root and both Newton steps.
__m128 Normalize(__m128 v) {
    const __m128 squared = Sum3(_mm_mul_ps(v,v));
    __m128 r = _mm_rsqrt_ps(squared);
    const __m128 one = _mm_set1_ps(1.0f), half = _mm_set1_ps(0.5f);
    for (int i = 0; i < 2; ++i)
        r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,
            _mm_mul_ps(_mm_mul_ps(r,r),squared)),_mm_mul_ps(r,half)),r);
    return _mm_mul_ps(r,v);
}
}

// Original 0x0075AFA0. The decompile loses the registry receiver on 81C880,
// the sampler receiver on 759B20, and the accepted-vector receiver on 654350.
bool RoutingSampler::SampleSegment(void* context, void* registry,
    const RoutingPoint* start, const RoutingPoint* end, const float* direction,
    int32_t level, void* arg8, void* arg9, void* arg10, void* accepted, void* rejected)
{
    if (CallGameMethod<uint8_t,0x81C880>(registry,0x1000000,0,0)) return false;
    __m128 forward = Horizontal(_mm_sub_ps(_mm_load_ps(end->position),_mm_load_ps(start->position)));
    __m128 backward = Horizontal(_mm_sub_ps(_mm_load_ps(start->position),_mm_load_ps(end->position)));
    if (start->level != level) return false;
    const float distance = _mm_cvtss_f32(_mm_sqrt_ps(Sum3(_mm_mul_ps(forward,forward))));
    const float lower = minimum - 0.0010000000474974513f;
    // COMISS/JB also rejects unordered operands.
    if (!(distance >= (lower > 0.0f ? lower : 0.0f))) return false;
    const float upper = minimum > maximum ? minimum : maximum;
    if (!(upper + 0.0010000000474974513f >= distance)) return false;
    const __m128 facingVector = Horizontal(_mm_loadu_ps(direction));
    const bool nonzero = distance > 0.0f;
    if (nonzero) {
        forward = Normalize(forward);
        backward = Normalize(backward);
        if (cone < tau && _mm_cvtss_f32(Sum3(_mm_mul_ps(facingVector,facingVector))) > 0.0f) {
            const __m128 a = Normalize(Normalize(facingVector));
            const __m128 b = Normalize(backward);
            const __m128 dot = _mm_min_ps(_mm_max_ps(Sum3(_mm_mul_ps(a,b)),_mm_set1_ps(-1.0f)),_mm_set1_ps(1.0f));
            float angle = static_cast<float>(std::acos(static_cast<double>(_mm_cvtss_f32(dot))));
            const __m128 cross = _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(a,a,18),_mm_shuffle_ps(b,b,9)),
                _mm_mul_ps(_mm_shuffle_ps(a,a,9),_mm_shuffle_ps(b,b,18)));
            const float sign = _mm_cvtss_f32(Sum3(_mm_mul_ps(cross,_mm_set_ps(0,0,1,0))));
            if (sign < 0.0f) angle = tau - angle;
            const double turns = static_cast<double>(angle) * inverseTau;
            const float roundedTurns = static_cast<float>(turns);
            const double fraction = static_cast<double>(roundedTurns) - std::floor(turns);
            float wrapped = static_cast<float>(fraction);
            if (fraction > 0.5) wrapped -= 1.0f;
            wrapped *= tau;
            if (wrapped > pi) wrapped -= tau;
            if (wrapped < -pi) wrapped += tau;
            if (!(cone * 0.5f >= std::fabs(wrapped))) return false;
        }
    }
    RoutingPoint candidate; // Only 52 bytes are initialized/copied by EA.
    std::memcpy(candidate.position,start->position,16);
    _mm_store_ps(candidate.direction,_mm_setzero_ps());
    if (nonzero && facing == 1) _mm_store_ps(candidate.direction,forward);
    else if (nonzero && facing == 2) _mm_store_ps(candidate.direction,backward);
    candidate.weight = weight * 100.0f;
    candidate.level = start->level;
    std::memcpy(candidate.metadata,start->metadata,sizeof candidate.metadata);
    RoutingPoint target;
    std::memcpy(target.position,end->position,16);
    _mm_store_ps(target.direction,_mm_setzero_ps());
    target.weight = 1.0f;
    target.level = level;
    std::memset(target.metadata,0,sizeof target.metadata);
    float scale = 1.0f;
    if (!CallGameMethod<uint8_t,0x759B20>(this,context,&target,&candidate,registry,arg8,arg9,arg10,&scale)) {
        candidate.weight = scale * candidate.weight;
        CallGameMethod<void,0x654350>(accepted,&candidate);
        return true;
    }
    CallGameFunction<void,0x74C650>(rejected,&candidate,1);
    return false;
}

// Original 0x0075B610. Retains lazy initialization of the original shared state,
// unsigned low-dword sample counts, and the 52-byte copy into 64-byte vector slots.
uint32_t RoutingSampler::SampleArc(void* context, void* registry, uint32_t weightByDistance,
    const float* center, float radius, int32_t level, float firstAngle,
    float lastAngle, RoutingPointVector* accepted, void* arg11, void* arg12,
    void* arg13, float* firstPoint, float* lastPoint, void* rejected, void* unused)
{
    (void)unused;
#if defined(_M_IX86)
    auto& initialized = *reinterpret_cast<uint32_t*>(0x11D4BA4);
    auto& reciprocalTau = *reinterpret_cast<float*>(0x11D4BA0);
    auto* upHalf = reinterpret_cast<float*>(0x11D4B90);
    void* world = *reinterpret_cast<void**>(0x11ECBC4);
#else
    static uint32_t initialized = 0;
    static float reciprocalTau;
    static float upHalf[4];
    void* world = nullptr;
#endif
    if (!(initialized & 1)) {
        initialized |= 1;
        reciprocalTau = inverseTau;
    }
    if (!(initialized & 2)) {
        initialized |= 2;
        upHalf[0] = 0; upHalf[1] = 0.5f; upHalf[2] = 0; upHalf[3] = 0;
    }
    const double range = static_cast<double>(lastAngle) - firstAngle;
    const double samples = static_cast<double>(reciprocalTau) * range * radius * tau / spacing + 0.5;
    // x87 fistp writes INT64_MIN for invalid/out-of-range conversion.
    const int64_t integral = std::isfinite(samples) && samples >= -9223372036854775808.0 &&
        samples < 9223372036854775808.0 ? static_cast<int64_t>(samples) : INT64_MIN;
    const uint32_t rawCount = static_cast<uint32_t>(integral);
    const uint32_t count = rawCount > 1 ? rawCount : 1;
    const float increment = count > 1 ? static_cast<float>(range / (count - 1)) : 0.0f;
    RoutingPoint target;
    std::memcpy(target.position,center,16);
    _mm_store_ps(target.direction,_mm_setzero_ps());
    target.weight = 1;
    target.level = level;
    std::memset(target.metadata,0,sizeof target.metadata);
    for (uint32_t i = 0; i < count; ++i) {
        RoutingPoint candidate;
        _mm_store_ps(candidate.direction,_mm_setzero_ps());
        candidate.weight = 1;
        candidate.level = level;
        std::memset(candidate.metadata,0,sizeof candidate.metadata);
        const double angle = static_cast<double>(i) * increment + firstAngle;
        // x87 FSIN/FCOS leave out-of-range finite operands unchanged (C2=1).
        const bool outsideTrigRange = std::isfinite(angle) && std::fabs(angle) >= 9223372036854775808.0;
        const float sine = static_cast<float>(outsideTrigRange ? angle : std::sin(angle));
        const float cosine = static_cast<float>(outsideTrigRange ? angle : std::cos(angle));
        _mm_store_ps(candidate.position,_mm_add_ps(_mm_mul_ps(_mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(sine),_mm_set_ps(0,1,0,0)),
            _mm_mul_ps(_mm_set1_ps(cosine),_mm_set_ps(0,0,0,1))),_mm_set1_ps(radius)),_mm_loadu_ps(center)));
        const auto height = [&](int32_t atLevel, int terrain, int surface) {
            void* manager = CallGameFunction<void*,0x8292D0>();
            return manager ? static_cast<float>(CallGameMethod<double,0x829470>(manager,
                candidate.position[0],candidate.position[2],atLevel,terrain,surface)) : 0.0f;
        };
        void* lot = world ? CallGameMethod<void*,0xC68990>(world,candidate.position[0],candidate.position[2]) : nullptr;
        if (lot) {
            struct Location { float x, z; int32_t level; } location{-1,-1,0};
            CallGameFunction<void,0xADD7A0>(lot,candidate.position,&location);
            candidate.level = location.level;
            const bool basement = candidate.level < 0 && CallGameFunction<uint8_t,0xB015B0>(lot,&location);
            const bool allowed = CallGameFunction<uint8_t,0xB01060>(lot,&location,1) != 0;
            if (candidate.level < 0 && !basement && !allowed) candidate.level = 0;
            const auto* lotBytes = static_cast<const uint8_t*>(lot);
            int32_t kind, subtype;
            std::memcpy(&kind,lotBytes+1032,4);
            std::memcpy(&subtype,lotBytes+1036,4);
            const bool surface = (kind != 1 || subtype != 80 || candidate.level != 0) && (!basement || allowed);
            candidate.position[1] = height(candidate.level,1,surface);
        } else {
            candidate.level = 0;
            const bool terrain = !CallGameMethod<uint8_t,0x81C880>(registry,1,0,1);
            candidate.position[1] = height(0,terrain,1);
            if (!CallGameMethod<uint8_t,0x81C880>(registry,1,0,1) && candidate.position[1] > center[1] + 3.0f) {
                const float threshold = static_cast<float>(CallGameMethod<double,0xC61670>(world) - 1.0);
                if (center[1] > threshold) candidate.position[1] = height(candidate.level,0,1);
            }
        }
        if (facing == 1 || facing == 2) {
            const __m128 delta = Horizontal(facing == 1 ?
                _mm_sub_ps(_mm_loadu_ps(center),_mm_load_ps(candidate.position)) :
                _mm_sub_ps(_mm_load_ps(candidate.position),_mm_loadu_ps(center)));
            if (_mm_cvtss_f32(Sum3(_mm_mul_ps(delta,delta))) > 0.0f)
                _mm_store_ps(candidate.direction,Normalize(delta));
        }
        if (i != 0 || count == 1) std::memcpy(lastPoint,candidate.position,16);
        if (i == 0) std::memcpy(firstPoint,candidate.position,16);
        float scale = 1;
        if (CallGameMethod<uint8_t,0x759B20>(this,context,&target,&candidate,registry,arg11,arg12,arg13,&scale)) {
            CallGameFunction<void,0x74C650>(rejected,&candidate,1);
        } else {
            if (weightByDistance > 0) {
                const float savedWeight = weight;
                candidate.weight = static_cast<float>(CallGameMethod<double,0x74F8E0>(this,&candidate) * savedWeight * scale);
            } else candidate.weight = weight * scale;
            RoutingPoint* destination = accepted->end;
            if (reinterpret_cast<uintptr_t>(destination) >= reinterpret_cast<uintptr_t>(accepted->capacity))
                CallGameMethod<void,0x653870>(accepted,destination,&candidate);
            else {
                accepted->end = reinterpret_cast<RoutingPoint*>(reinterpret_cast<uintptr_t>(destination) + sizeof(RoutingPoint));
                if (destination) std::memcpy(destination,&candidate,52);
            }
        }
    }
    return count;
}
}
