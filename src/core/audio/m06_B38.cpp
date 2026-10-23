#include "ts3/audio/m06_B38.h"
#include "ts3/layout.h"
#include <cstring>
#include <cmath>
namespace ts3::audio::b38 {
namespace {
void* Table(uintptr_t value) { return reinterpret_cast<void*>(value); }
uintptr_t Release(void* p) {
#if defined(_M_IX86)
    return CallGameMethod<uintptr_t,0x00ED7230>(*reinterpret_cast<void**>(0x011FFC28),p,0u);
#else
    return CallGameMethod<uintptr_t,0x00ED7230>(static_cast<void*>(nullptr),p,0u);
#endif
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
float Float(uint32_t u) {float f;std::memcpy(&f,&u,4);return f;}
uintptr_t Partial(void* end) { return (reinterpret_cast<uintptr_t>(end)&~uintptr_t(255))|1; }
}
LongEffect* LongConstruct(LongEffect* self) {
    self->header.vtable=Table(0x010357B8);
    for(auto& e:self->entries) {e.allocation=nullptr;for(auto& w:e.words) w=0;}
    return self;
}
uintptr_t LongDestroy(LongEffect* self) {
    uintptr_t result=0;
    for(int i=5;i>=0;--i) {result=reinterpret_cast<uintptr_t>(self->entries[i].allocation);if(result) result=Release(self->entries[i].allocation);}
    self->header.vtable=Table(0x0111DB94);return result;
}
LongEffect* LongDelete(LongEffect* self,uint32_t flags) { LongDestroy(self);if(flags&1) Free(self);return self; }
uintptr_t LongInitialize(LongEffect* self) {
    if(self) LongConstruct(self);
    self->header.data=self->parameters;
    for(int i=0;i<30;++i) self->parameters[i*2]=0;
    return Partial(self->parameters+60);
}
uint32_t LongSize() {return 0xD8;}
void* LongDescriptor() {return Table(0x01169DDC);}
uintptr_t Destroy(Effect* self) {
    uintptr_t result=0;
    for(int i=5;i>=0;--i) {result=reinterpret_cast<uintptr_t>(self->entries[i].allocation);if(result) result=Release(self->entries[i].allocation);}
    self->header.vtable=Table(0x0111DB94);return result;
}
Effect* Delete(Effect* self,uint32_t flags) {Destroy(self);if(flags&1) Free(self);return self;}
Effect* Construct(Effect* self,uint32_t offset) {
    if(self) {self->header.vtable=Table(0x01035808);for(auto& e:self->entries) {e.allocation=nullptr;for(auto& w:e.words) w=0;}}
    if(offset) self->header.data=reinterpret_cast<uint8_t*>(self)+offset;
    return self;
}
uintptr_t Initialize(Effect* self) {
    Construct(self,static_cast<uint32_t>(offsetof(Effect,parameters)));
    for(int i=0;i<13;++i) self->parameters[i*2]=0;
    return Partial(self->parameters+26);
}
uint32_t Size() {return 0xE8;}
void* Descriptor() {return Table(0x01169FD4);}
uintptr_t InitializeModulator(Effect* self) {
    Construct(self,static_cast<uint32_t>(offsetof(Effect,parameters)));
    for(int i=0;i<14;++i) self->parameters[i*2]=0;
    self->parameters[2]=0x3f800000;self->phase=0;
    return Partial(self->parameters+28);
}
uint32_t ModulatorSize() {return 0x1E0;}
uint32_t Process(Effect* self,Context* context) {
    float frequency=Float(self->parameters[0]);
    if(frequency==0) return 1;
    float wet=Float(self->parameters[2]),dry=1.0f-wet;
    int64_t step64;
#if defined(_M_IX86)
    float scale=context->scale,rate=context->rate->rate;
    float k1=4096.f,k2=65536.f;unsigned short cw,trunc;
    __asm { fnstcw cw }
    trunc=cw|0xC00;
    __asm {
        fld scale
        fdiv rate
        fmul frequency
        fmul k1
        fmul k2
        fldcw trunc
        fistp step64
        fldcw cw
    }
#else
    step64=static_cast<int64_t>(static_cast<long double>(context->scale)/context->rate->rate*frequency*4096.f*65536.f);
#endif
    uint32_t phase=self->phase,step=static_cast<uint32_t>(step64);
    const float* table;
#if defined(_M_IX86)
    table=reinterpret_cast<const float*>(0x01035830);
#else
    // The table belongs to the original image; native callers cannot reach it yet.
    AbortNotRewritten("m06_B38 modulation table");
    table=nullptr;
#endif
    uint32_t channel=0;
    while(channel<self->header.channels) {
        float* in=context->input->samples+context->input->stride*channel;
        float* out=context->output->samples+context->output->stride*channel;
        phase=self->phase;
        for(int i=0;i<context->frames;++i) {
            uint32_t index=phase>>16;
            float gain=table[index],sample=in[i];
            if(wet==0) {out[i]=sample;}
            else {
                float modulated=sample*gain;
                if(wet==1) out[i]=modulated;
                else {float a=sample*dry,b=modulated*wet;out[i]=a+b;}
            }
            phase+=step;if(phase>=0x10000000u) phase-=0x10000000u;
        }
        ++channel;
    }
    // EA zero-channel fallthrough reloads ECX from the self argument.
    self->phase=self->header.channels?phase:static_cast<uint32_t>(reinterpret_cast<uintptr_t>(self));
    auto* temp=context->input;context->input=context->output;context->output=temp;
    return 1;
}
}
