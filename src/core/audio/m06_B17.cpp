#include "ts3/audio/m06_B17.h"
#include <cstring>
#include <cstddef>
#include <string>
#include <xmmintrin.h>
#include "ts3/usertool/clu_8638c0_UserTool.h"
#include "ts3/scripting/cls_86c000_Scripting.h"
namespace ts3::audio::b17 {
namespace {
const void* Table(uintptr_t address) { return reinterpret_cast<const void*>(address); }
void Free(void* p) {
#if defined(_M_IX86)
    CallGameFunction<void,0x004E3F70>(p);
#else
    ::operator delete(p);
#endif
}
void ReleaseString(String& s) {
    // EA treats the low bit of the capacity address as a tag, and compares the
    // resulting *signed byte span*, not the logical string length.
    auto span = static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s.capacity) -
        reinterpret_cast<uintptr_t>(s.begin)) & ~uintptr_t(1));
    if (span > 2 && s.begin) Free(s.begin);
}
uint32_t ResolveName(void* event, char* output, uint32_t capacity, uint32_t hash) {
#if defined(_M_IX86)
    char name[32]{};
    std::memcpy(name,Table(0x00FE21E0),5);
    uint64_t key=0;
    auto manager = CallGameFunction<void*,0x00938B50>();
    void* actor=nullptr;
    if (CallGameMethod<uint8_t,0x0089F980>(event,hash,&key))
        actor=CallGameMethod<void*,0x00939560>(manager,static_cast<uint32_t>(key),static_cast<uint32_t>(key>>32));
    auto gate=*reinterpret_cast<void**>(0x011ED034);
    if (gate && actor) {
        auto data=CallGameMethod<uint8_t*,0x00C8EC50>(gate,static_cast<uint8_t*>(actor)+0x18);
        if (data) {
            auto first=*reinterpret_cast<char**>(data+0x218);
            if (first!=*reinterpret_cast<char**>(data+0x21c))
                CallGameFunction<void,0x004FF320>(name,first,32u);
        }
    }
    CallGameFunction<uint32_t,0x004FF3F0>(output,Table(0x00FB1E50),capacity);
    return CallGameFunction<uint32_t,0x004FF3F0>(output,name,capacity);
#else
    (void)event; (void)output; (void)capacity; (void)hash;
    NotRewritten("B17 native actor/world resolver interfaces");
#endif
}
}
Modifier* __fastcall ConstructTarget(Modifier* self, void*) {
    self->state=0;
    self->vtable=const_cast<void*>(Table(0x01029640));
    return self;
}
Modifier* __fastcall ConstructName(Modifier* self, void*, void* delegate) {
    self->state=0;
    self->vtable=const_cast<void*>(Table(0x01029668));
    self->delegate=delegate;
    return self;
}
void* __fastcall DestroyBase(Modifier* self, void*, uint32_t flags) {
    self->vtable=const_cast<void*>(Table(0x00FF5F1C));
    if (flags & 1) Free(self);
    return self;
}
void* __fastcall DestroySkill(SkillModifier* self, void*, uint32_t flags) {
    self->base.vtable=const_cast<void*>(Table(0x010296B8));
    ReleaseString(self->name);
    return DestroyBase(&self->base,nullptr,flags);
}
SkillModifier* __fastcall ConstructSkill(SkillModifier* self, void*, void* delegate, const char16_t* name) {
    self->base.delegate=delegate;
    self->base.state=0;
    self->base.vtable=const_cast<void*>(Table(0x010296B8));
#if defined(_M_IX86)
    auto empty=reinterpret_cast<char16_t*>(0x011C7E54);
    self->name={empty,empty,empty+1,Table(0x010296CC)};
    const char16_t* end=name;
    while (*end) ++end;
    CallGameMethod<void,0x00408E20>(&self->name,name,end);
#else
    auto count=std::char_traits<char16_t>::length(name);
    static char16_t empty[1]{};
    auto data=count?static_cast<char16_t*>(::operator new((count+1)*sizeof(char16_t))):empty;
    std::memcpy(data,name,(count+1)*sizeof(char16_t));
    self->name={data,data+count,data+count+1,Table(0x010296CC)};
#endif
    return self;
}
uint32_t __fastcall ActorName(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t) {
    return ResolveName(event,output,capacity,0x13F18A1F);
}
uint32_t __fastcall TargetName(Modifier*, void*, void* event, char* output, uint32_t capacity, uint32_t) {
    return ResolveName(event,output,capacity,0x6BF16A1E);
}

namespace {
// The argument array uses 32-byte variants at x86, not UserToolValue's
// 20-byte accessed prefix. Pointer storage grows naturally in native builds.
struct Value {
    void* vtable; uint32_t reserved; uint16_t flags,type; uint32_t reservedC;
    union Data { void* pointer; float number; uint32_t words[4]; } data;
};
TS3_X86_SIZE(Value,32);
void Init(Value& v) { v.vtable=const_cast<void*>(Table(0x00FB96B4)); v.flags=0; v.type=0; }
uintptr_t Dispose(Value& v, uintptr_t eax) {
    v.vtable=const_cast<void*>(Table(0x00FB96B4));
    if (v.flags & 4) return CallGameMethod<uintptr_t,0x004F55A0>(&v,0);
    return eax;
}
void WriteFloat(void* event,uint32_t hash,float value) {
    int bits;
#if defined(_M_IX86)
    // EA forwards via fld/fstp, not a raw SSE move: signaling NaNs become
    // quiet before the setter sees their bits. Keep the caller's x87 state.
    float stored;
    __asm { fld value }
    __asm { fstp stored }
    std::memcpy(&bits,&stored,4);
#else
    std::memcpy(&bits,&value,4);
    if((static_cast<uint32_t>(bits)&0x7F800000u)==0x7F800000u &&
       (static_cast<uint32_t>(bits)&0x007FFFFFu)) bits|=0x00400000;
#endif
    scripting::SetValueByHash89FB90(event,nullptr,static_cast<int>(hash),bits);
}
bool Has(void* event,uint32_t hash) {
    return CallGameMethod<uint8_t,0x0089F8E0>(event,hash,0,0)!=0;
}
float Constant(uintptr_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(address);
#else
    (void)address; NotRewritten("B17 native quality constants");
#endif
}
uintptr_t Evaluate(Modifier* self, String* skill, void* event,char* output,uint32_t capacity,int kind) {
    // kind 0: name (3 arguments); 1: quality (5); 2: skill name (4).
    void* service=CallGameFunction<void*,0x007692B0>();
    uintptr_t eax=reinterpret_cast<uintptr_t>(service);
    if (!service || (kind==0 && !self->delegate)) return eax;
    uint64_t actorKey=0;
    eax=CallGameMethod<uint32_t,0x0089F980>(event,0x13F18A1Fu,&actorKey);
    if (!(eax & 0xFF)) return eax;
    Value args[5];
    const unsigned count=kind==0?3:kind==1?5:4;
    Init(args[0]); CallGameMethod<void,0x004F5E60>(&args[0],&actorKey);
    String converted;
    const void* label=Table(kind==1?0x010296A8:0x0102967C);
    auto str=CallGameFunction<String*,0x004F3750>(&converted,output,-1,label);
    Init(args[1]); CallGameMethod<void,0x004F5800>(&args[1],19,9,str,16,1);
    ReleaseString(converted);
    uint32_t zero=0;
    if(kind==1) {
        Init(args[2]); CallGameMethod<void,0x004F5EE0>(&args[2],&zero);
        zero=0; Init(args[3]); CallGameMethod<void,0x004F5EE0>(&args[3],&zero);
        zero=0; Init(args[4]); CallGameMethod<void,0x004F5D60>(&args[4],&zero);
    } else {
        if(kind==2) { Init(args[2]); CallGameMethod<void,0x004F5800>(&args[2],19,9,skill,16,1); }
        zero=0; Init(args[count-1]); CallGameMethod<void,0x004F5CE0>(&args[count-1],&zero);
    }
    Value result;
    eax=CallGameVirtual<uintptr_t,77>(service,&result,self->delegate,args,count);
    eax=Dispose(result,eax);
#if defined(_M_IX86)
    auto outString=static_cast<String*>(reinterpret_cast<const usertool::UserToolValue*>(&args[1])->GetValue());
#else
    // The verified accessor's offset-only legacy view is x86-specific; use the
    // pointer-sized variant view if native service interfaces are supplied later.
    if(args[1].type!=19 && args[1].type!=16) NotRewritten("B17 native default string variant");
    auto outString=(args[1].flags&0x30)?static_cast<String*>(args[1].data.pointer):reinterpret_cast<String*>(&args[1].data);
#endif
    auto length=static_cast<uint32_t>((reinterpret_cast<uintptr_t>(outString->end)-reinterpret_cast<uintptr_t>(outString->begin))/2);
    auto written=CallGameFunction<uint32_t,0x004F4260>(output,capacity-1,outString->begin,length);
    output[written+1]=0; // EA off-by-one terminator: do not change to [written].
    if(kind==1) {
        if(Has(event,0x489C3762)) {
            auto p=scripting::GetMedatorValue86C6C0(&args[2],nullptr);
            WriteFloat(event,0x489C3762,*static_cast<float*>(p));
        }
        if(Has(event,0x779A5FF4)) {
            auto p=scripting::GetMedatorValue86C6C0(&args[3],nullptr);
            WriteFloat(event,0x779A5FF4,*static_cast<float*>(p));
        }
        uint8_t flags=static_cast<uint8_t>(args[4].data.words[0]);
        if(Has(event,0x4AF82A78)) WriteFloat(event,0x4AF82A78,(flags&1)?Constant(0x0107A538):0.0f);
        if(Has(event,0x703D682B) && (flags&2)) WriteFloat(event,0x703D682B,Constant(0x00FBDF9C));
        if(Has(event,0x703D682B) && (flags&8)) {
            WriteFloat(event,0x15525BAA,Constant(0x01168B28));
            WriteFloat(event,0xFC047EEC,Constant(0x01168B24));
            WriteFloat(event,0x49EB68DB,Constant(0x01168B20));
            WriteFloat(event,0x25DF0108,Constant(0x01168B1C));
        }
        if(flags&16) WriteFloat(event,0x703D682B,Constant(0x01168B18));
        if(flags&4) {
            const uint32_t hashes[]={0x15525BAA,0xFC047EEC,0x49EB68DB,0x25DF0108};
            const uintptr_t constants[]={0x00FAA20C,0x00FAA20C,0x00F9D514,0x010296A4};
            for(unsigned i=0;i<4;++i) if(Has(event,hashes[i])) WriteFloat(event,hashes[i],Constant(constants[i]));
        }
    } else if(kind==2) {
        Value cleanup;
        cleanup.vtable=const_cast<void*>(Table(0x01006FB4)); cleanup.type=10; cleanup.flags=2;
        eax=CallGameMethod<uintptr_t,0x004F62C0>(&cleanup,&args[3]);
        eax=Dispose(cleanup,eax);
    }
#if defined(_M_IX86)
    // Compiler EH vector-destructor helper is stdcall (ret 16), not cdecl.
    using ArrayDtor=uintptr_t(__stdcall*)(Value*,int,unsigned,const void*);
    return reinterpret_cast<ArrayDtor>(0x00446710)(args,32,count,Table(0x004F5660));
#else
    NotRewritten("B17 native variant-array destruction interface");
#endif
}
}
uintptr_t __fastcall EvaluateName(Modifier* self,void*,void* event,char* output,uint32_t capacity,uint32_t) {
    return Evaluate(self,nullptr,event,output,capacity,0);
}
uintptr_t __fastcall EvaluateQuality(Modifier* self,void*,void* event,char* output,uint32_t capacity,uint32_t) {
    return Evaluate(self,nullptr,event,output,capacity,1);
}
uintptr_t __fastcall EvaluateSkill(SkillModifier* self,void*,void* event,char* output,uint32_t capacity,uint32_t) {
    return Evaluate(&self->base,&self->name,event,output,capacity,2);
}

namespace {
struct LotView {
    uint8_t opaque[0x298]; void* roomMap;
    uint8_t tail[0x39C-0x29C]; int32_t topFloor;
};
TS3_X86_OFFSET(LotView,roomMap,0x298);
TS3_X86_OFFSET(LotView,topFloor,0x39C);
struct HitView {
    uint8_t opaque[16]; alignas(16) float position[4];
    uint8_t tail[24]; void* object;
};
TS3_X86_OFFSET(HitView,object,0x38);
void* World() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011ECBC4);
#else
    NotRewritten("B17 native world singleton interface");
#endif
}
__m128 Splat(float x) { return _mm_set1_ps(x); }
__m128 Normalize(__m128 delta) {
    __m128 sq=_mm_mul_ps(delta,delta);
    __m128 sum=_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1));
    sum=_mm_add_ps(sum,sq); sum=_mm_shuffle_ps(sum,sum,0);
    __m128 half=Splat(Constant(0x00F9A5AC)),one=Splat(Constant(0x0107A538));
    __m128 inv=_mm_rsqrt_ps(sum);
    __m128 error=_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inv,inv),sum));
    inv=_mm_add_ps(_mm_mul_ps(error,_mm_mul_ps(half,inv)),inv);
    error=_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inv,inv),sum));
    inv=_mm_add_ps(_mm_mul_ps(error,_mm_mul_ps(half,inv)),inv);
    return _mm_mul_ps(inv,delta);
}
}
uintptr_t __cdecl ResolveRoom(const float* position,void** lotOut,uint16_t* room,int32_t* floor) {
    *lotOut=nullptr; *room=0; *floor=0;
    void* world=World();
    uintptr_t eax=reinterpret_cast<uintptr_t>(lotOut);
    if(!world) return eax;
    auto lot=CallGameMethod<LotView*,0x00C68A40>(world,position[0],position[2]);
    eax=reinterpret_cast<uintptr_t>(lot);
    if(lot) {
        void* map=lot->roomMap;
        alignas(16) float local[4];
        CallGameMethod<void,0x00AB8AB0>(lot,position,local);
        int32_t coords[3];
        float minusOne=Constant(0x00FAA210);
        std::memcpy(&coords[0],&minusOne,4); std::memcpy(&coords[1],&minusOne,4); coords[2]=0;
        CallGameFunction<void,0x00ADD800>(lot,local,coords);
        int32_t surfaceFloor;
        if(CallGameFunction<uint8_t,0x00B01390>(lot,local,&surfaceFloor,1))
            coords[2]=static_cast<int32_t>(static_cast<uint32_t>(surfaceFloor)+1);
        if(coords[2]>lot->topFloor) coords[2]=lot->topFloor;
        *lotOut=lot; *floor=coords[2];
        eax=CallGameMethod<uint32_t,0x00B499D0>(map,coords,0,0);
        *room=static_cast<uint16_t>(eax);
    }
    if(*room==0) {
        uint64_t key=CallGameMethod<uint64_t,0x00C5FB00>(world,0);
        auto fallback=CallGameMethod<LotView*,0x00C62D40>(world,static_cast<uint32_t>(key),static_cast<uint32_t>(key>>32));
        eax=reinterpret_cast<uintptr_t>(fallback);
        if(fallback && fallback->topFloor<0) { *room=0xFFFF; *floor=fallback->topFloor; }
    }
    return eax;
}
uint32_t __cdecl QueryInterior(const float* position,const float* endpoint,void* options,void* rawHit,uint32_t flags) {
    alignas(16) float sample[4]; _mm_store_ps(sample,_mm_load_ps(position));
    void* physics=CallGameFunction<void*,0x00686A90>();
    uint32_t eax=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(physics));
    if(!physics) return eax & 0xFFFFFF00u;
    void* world=World();
    eax=CallGameMethod<uint32_t,0x00686FD0>(physics,sample,endpoint,options,rawHit,flags);
    if(!(eax&0xFF)) return eax & 0xFFFFFF00u;
    auto hit=static_cast<HitView*>(rawHit);
    for(;;) {
        eax=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(hit->object));
        if(!hit->object || !(static_cast<uint8_t*>(hit->object)[0x34]&2)) return (eax&0xFFFFFF00u)|1;
        auto lot=CallGameMethod<LotView*,0x00C68990>(world,hit->position[0],hit->position[2]);
        eax=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(lot));
        if(!lot) return (eax&0xFFFFFF00u)|1;
        int32_t coords[3]; float minusOne=Constant(0x00FAA210);
        std::memcpy(&coords[0],&minusOne,4); std::memcpy(&coords[1],&minusOne,4); coords[2]=0;
        CallGameFunction<void,0x00ADD740>(lot,hit->position,coords);
        eax=CallGameFunction<uint32_t,0x00AFF6D0>(lot,coords,0,0,1);
        if(eax&0xFF) return (eax&0xFFFFFF00u)|1;
        // mov eax,[ebp+0xC] reloads the endpoint before the SIMD block.
        // Subsequent false paths clear only AL, leaving these pointer bits.
        eax=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(endpoint));
        __m128 hitPosition=_mm_load_ps(hit->position),end=_mm_load_ps(endpoint);
        __m128 delta=_mm_sub_ps(end,hitPosition);
        if((_mm_movemask_ps(_mm_cmpeq_ps(delta,_mm_setzero_ps()))&7)==7) return eax&0xFFFFFF00u;
        __m128 direction=Normalize(delta);
        __m128 next=_mm_add_ps(_mm_mul_ps(Splat(Constant(0x0106D6CC)),direction),hitPosition);
        _mm_store_ps(sample,next);
        __m128 difference=_mm_sub_ps(Normalize(_mm_sub_ps(end,next)),direction);
        __m128 absolute=_mm_max_ps(_mm_mul_ps(Splat(minusOne),difference),difference);
        if((_mm_movemask_ps(_mm_cmple_ps(absolute,Splat(Constant(0x0102979C))))&7)!=7) return eax&0xFFFFFF00u;
        eax=CallGameMethod<uint32_t,0x00686FD0>(physics,sample,endpoint,options,rawHit,flags);
        if(!(eax&0xFF)) return eax&0xFFFFFF00u;
    }
}
}
