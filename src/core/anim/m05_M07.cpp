#include "ts3/anim/m05_M07.h"
#include <cstring>
namespace ts3::anim::m07 {
static void** Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(0x00fc12d0);
#else
    static void* table[11]{};
    return table;
#endif
}
Evaluator* __fastcall Construct(Evaluator* s,void*,void* host) {
    b01::Construct(&s->base,host);
    s->base.vtable=Table();
    s->value=1.0f;
    s->second=0;
    return s;
}
// Adapted from ResourceManClassId and PackedFileRecordQueryInterfaceById twins.
uint32_t __fastcall Type(void*,void*) { return 0xf3f23a79u; }
void* __fastcall Query(void* s,void*,uint32_t id) {
    if(id==0x73deb8c8u) return s;
    if(id==0xf3f23a79u) return s;
    return nullptr;
}
// WorldSub::Dtor twin: only the table and base destructor differ.
void __fastcall Reset(Evaluator* s,void*) {
    s->base.vtable=Table();
    b01::Reset(&s->base);
}
static bool Step(Stream* s,uint32_t n,bool write,uintptr_t& eax) {
    s->cursor=static_cast<int32_t>(uint32_t(s->cursor)+n);
    if(s->cursor>=0) {
        if(write) eax=CallGameMethod<uintptr_t,0x00594140>(s,n);
        else eax=CallGameMethod<uintptr_t,0x00593ff0>(s,n);
    }
    return s->cursor<0;
}
uintptr_t __fastcall Save(Evaluator* s,void*,Stream* stream) {
    uintptr_t eax=0;
    uint8_t byte=s->first;
    if(Step(stream,1,true,eax)) {
        stream->output[stream->cursor]=byte;
        eax=reinterpret_cast<uintptr_t>(stream->output);
    }
    byte=s->second;
    if(Step(stream,1,true,eax)) {
        stream->output[stream->cursor]=byte;
        eax=uint32_t(stream->cursor);
    }
    uint32_t bits; std::memcpy(&bits,&s->value,4);
    if(Step(stream,4,true,eax)) {
        std::memcpy(stream->output+stream->cursor-3,&bits,4);
        // Final MOVSS uses ECX/EDX: EAX remains the second byte cursor or refill result.
    }
    return eax;
}
uintptr_t __fastcall Load(Evaluator* s,void*,Stream* stream) {
    uintptr_t eax=0;
    uint8_t byte=static_cast<uint8_t>(reinterpret_cast<uintptr_t>(stream));
    if(Step(stream,1,false,eax)) {
        byte=stream->input[stream->cursor];
        eax=(reinterpret_cast<uintptr_t>(stream->input)&~uintptr_t(255))|byte;
    } else eax=(eax&~uintptr_t(255))|byte;
    s->first=byte!=0;
    byte=static_cast<uint8_t>(reinterpret_cast<uintptr_t>(stream));
    if(Step(stream,1,false,eax)) {
        byte=stream->input[stream->cursor];
        eax=(reinterpret_cast<uintptr_t>(stream->input)&~uintptr_t(255))|byte;
    } else eax=(eax&~uintptr_t(255))|byte;
    s->second=byte!=0;
    if(Step(stream,4,false,eax)) {
        auto input=stream->input;
        auto source=input+stream->cursor-3;
#if defined(_M_IX86)
        float* dest=&s->value;
        __asm { mov eax,source }
        __asm { fld dword ptr [eax] }
        __asm { mov eax,dest }
        __asm { fstp dword ptr [eax] }
#else
        uint32_t bits; std::memcpy(&bits,source,4);
        if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
        std::memcpy(&s->value,&bits,4);
#endif
        eax=reinterpret_cast<uintptr_t>(input);
    }
    return eax;
}
}
