#include "ts3/anim/m05_M69.h"
#include "ts3/anim/m05_B21.h"
#include "ts3/math/m04_B11.h"
#include <cstring>
namespace ts3::anim::m69 {
static const char* Text(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(address);
#else
    (void)address; NotRewritten("M69 game descriptor");
#endif
}
static float GameFloat(uintptr_t address) { return *reinterpret_cast<const float*>(Text(address)); }
static void* Allocate(uint32_t n,uintptr_t tag) { return CallGameFunction<void*,0x004e3f90>(n,Text(tag),0,0,0,0); }
Name* MissingName(Manager* self) {
    if(!self->missingName) {
        auto* name=static_cast<Name*>(Allocate(sizeof(Name),0xfc27b8));
        if(name) { name->descriptor=Text(0xfc27b8); name->begin=const_cast<char*>(Text(0x11c7e54)); name->end=name->begin; name->capacity=name->begin+2; }
        self->missingName=name;
    }
    if(!CallGameFunction<uintptr_t,0x007d8e10>(uint32_t(self->missingKey),uint32_t(self->missingKey>>32)))
        CallGameFunction<void,0x0040e920>(self->missingName,Text(0xfc288c),uint32_t(self->missingKey),uint32_t(self->missingKey>>32));
    return self->missingName;
}
Manager* Construct(Manager* self) {
    self->next=self->previous=self;
    self->listNext=self->listPrevious=&self->listNext;
    self->owner=reinterpret_cast<unsigned char*>(self)-6*sizeof(void*);
    self->count=0; self->removeBegin=self->removeEnd=self->removeCapacity=nullptr;
    self->descriptor=Text(0xfc2c78); self->mode=3;
    std::memset(self->flags,0,6); std::memset(self->flags+6,1,7);
    self->missingKey=CallGameFunction<uint64_t,0x0057a400>(Text(0xfc2c60),0);
    self->missingName=nullptr;
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0xfc2c4cu,0x834u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+3),0xfc2c2cu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+10),0xfc2658u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+11),0xfc263cu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+12),0xfc2bfcu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+8),0xfc2604u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+9),0xfc2620u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+6),0xfc25e8u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+5),0xfc26f0u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+4),0xfc26d4u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags),0xfc26c0u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+2),0xfc26a0u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,16>(service,3,reinterpret_cast<uintptr_t>(self),0xfc267cu,0x572440u,0x572190u,0xfffe7960u,0x186a0u,1);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0,0x7fffffffu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0xfc2be4u,0x802u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,reinterpret_cast<uintptr_t>(self->flags+1),0xfc2bb8u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,10>(service,0x11cd7fcu,0xfc2b8cu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f620u,0xfc2b60u,GameFloat(0xf9a5acu),GameFloat(0x1020a7cu),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x11cd7f0u,0xfc2b3cu,0.0f,GameFloat(0xf9a5acu),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f61cu,0xfc2b18u,0.0f,GameFloat(0xf9a5acu),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x11cd7f4u,0xfc2ae4u,0.0f,1.0f,GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x11cd7f8u,0xfc2ab4u,0.0f,1.0f,GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f624u,0xfc2a78u,1.0f,GameFloat(0xfbb86cu),1.0f);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f630u,0xfc2a44u,GameFloat(0x106d6ccu),GameFloat(0xfc2a74u),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f634u,0xfc2a14u,GameFloat(0xfa1b94u),GameFloat(0xf98a58u),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f628u,0xfc29f4u,GameFloat(0x106d6ccu),1.0f,GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f62cu,0xfc29d4u,1.0f,GameFloat(0xf98a58u),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0,0x7fffffffu);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0xfc29c0u,0x836u);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2d8u,0xfc298cu,0.0f,1.0f,GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2b0u,0xfc2968u,GameFloat(0x106d6ccu),GameFloat(0xf98a58u),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2b4u,0xfc2948u,GameFloat(0x106d6ccu),GameFloat(0xf98a58u),GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2d4u,0xfc291cu,0.0f,1.0f,GameFloat(0x106d6ccu));
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2b8u,0xfc28fcu,0.0f,GameFloat(0xfc0800u),1.0f);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2bcu,0xfc28dcu,0.0f,GameFloat(0xfc0800u),1.0f);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,12>(service,0x114f2c0u,0xfc28bcu,0.0f,GameFloat(0xfc0800u),1.0f);
    if(auto* service=CallGameFunction<void*,0x005a00a0>()) CallGameVirtual<void,6>(service,0,0x7fffffffu);
    return self;
}
uintptr_t Initialize() {
#if defined(_M_IX86)
    struct Row { uint32_t key,object,table,registrar,cleanup; };
    static constexpr Row rows[]={
        {0x13ea6f90u,0x11cd8a8u,0xfc2330u,0x54c530u,0xf91790u},
        {0xf40282a6u,0x11cd8a4u,0xfc2340u,0x54c530u,0xf91770u},
        {0x53deec77u,0x11cd8a0u,0xfc2350u,0x54c530u,0xf91750u},
        {0x73bb514u,0x11cd89cu,0xfc2360u,0x54c530u,0xf91730u},
        {0x9419c8abu,0x11cd898u,0xfc2370u,0x54c530u,0xf91710u},
        {0x33e02b1du,0x11cd894u,0xfc2380u,0x54c530u,0xf916f0u},
        {0xd3dfc020u,0x11cd890u,0xfc2390u,0x54c530u,0xf916d0u},
        {0xd4e4ccf8u,0x11cd88cu,0xfc23a0u,0x54c530u,0xf916b0u},
        {0xf3f23a79u,0x11cd888u,0xfc23b0u,0x54c530u,0xf91690u},
        {0x93dffdc3u,0x11cd884u,0xfc23c0u,0x54c530u,0xf91670u},
        {0x5419e34eu,0x11cd880u,0xfc23d0u,0x54c530u,0xf91650u},
        {0x7419e37cu,0x11cd87cu,0xfc2410u,0x54c530u,0xf91630u},
        {0xb3f291f8u,0x11cd878u,0xfc2450u,0x54c530u,0xf91610u},
        {0x6b264eau,0x11cd874u,0xfc2460u,0x54c530u,0xf915f0u},
        {0x87eb0331u,0x11cd870u,0xfc2470u,0x54c530u,0xf915d0u},
        {0x60d998eu,0x11cd86cu,0xfc2480u,0x54c530u,0xf915b0u},
        {0x65f6bc2u,0x11cd868u,0xfc2490u,0x54c530u,0xf91590u},
        {0x6662b00u,0x11cd864u,0xfc24a0u,0x54c530u,0xf91570u},
        {0xb1dc7dcu,0x11cd860u,0xfc24b0u,0x54c530u,0xf91550u},
        {0xbff764eu,0x11cd85cu,0xfc24c0u,0x54c530u,0xf91530u},
        {0xbb6d0a5u,0x11cd858u,0xfc24d0u,0x54c530u,0xf91510u},
        {0xb489fc2cu,0x11cd854u,0xfc24e0u,0x565d50u,0xf914f0u},
        {0x73b9d8cu,0x11cd850u,0xfc2500u,0x565d50u,0xf914d0u},
        {0x34f49f94u,0x11cd84cu,0xfc2508u,0x565d50u,0xf914b0u},
        {0x6c12d61u,0x11cd848u,0xfc2510u,0x565d50u,0xf91490u},
        {0x6b26363u,0x11cd844u,0xfc2518u,0x565d50u,0xf91470u},
        {0x67c5cabu,0x11cd840u,0xfc2520u,0x565d50u,0xf91450u},
        {0x7a11ba8u,0x11cd83cu,0xfc2528u,0x565d50u,0xf91430u},
        {0x74a5f9du,0x11cd838u,0xfc2530u,0x565d50u,0xf91410u},
        {0x664ed68u,0x11cd834u,0xfc258cu,0x565d50u,0xf913f0u},
        {0x60d99b1u,0x11cd830u,0xfc2594u,0x565d50u,0xf913d0u},
        {0x60caeefu,0x11cd82cu,0xfc259cu,0x565d50u,0xf913b0u},
        {0x75e4c89u,0x11cd824u,0xfc25a4u,0x565d50u,0xf91390u},
        {0x664fb68u,0x11cd820u,0xfc25acu,0x565d50u,0xf91370u},
        {0x75e3ecbu,0x11cd81cu,0xfc25b4u,0x565d50u,0xf91350u},
        {0x681c688u,0x11cd818u,0xfc25bcu,0x565d50u,0xf91330u},    };
    for(unsigned i=0;i<36;++i) {
        auto& bits=*reinterpret_cast<uint32_t*>(i<32?0x11cd8ac:0x11cd828);
        uint32_t bit=1u<<(i%32);
        if(!(bits&bit)) {
            bits|=bit;
            const auto& row=rows[i];
            auto** object=reinterpret_cast<void**>(row.object);
            *object=reinterpret_cast<void*>(row.table);
            if(i<21) CallGameFunction<void,0x0054c530>(row.key,object);
            else CallGameFunction<void,0x00565d50>(row.key,object);
            CallGameFunction<void,0x00f7e9a1>(row.cleanup);
        }
    }
    CallGameFunction<void,0x00574360>();
    CallGameFunction<void,0x005721a0>();
    auto* manager=static_cast<Manager*>(Allocate(sizeof(Manager),0xfc2c98));
    if(manager) manager=Construct(manager);
    *reinterpret_cast<Manager**>(0x11cd800)=manager;
    return (reinterpret_cast<uintptr_t>(manager)&~uintptr_t(255))|1;
#else
    NotRewritten("M69 factory registry globals");
#endif
}
void MarkDirty(Render* self) { self->flags|=1; }
static int32_t Count(RigResource* rig) {
    return rig?b21::Count(reinterpret_cast<const b21::SkeletonView*>(&rig->skeleton)):0;
}
void Update(Render* self,RigPose* pose) {
    if(!pose) return;
    auto& state=self->state;
    if(!state.matrices) {
        uint32_t count=uint32_t(Count(state.rig));
        state.matrices=static_cast<Matrix44Value*>(Allocate(count>0x3ffffff?0xffffffff:count*64,0xfc2ce4));
    }
    // The original redundantly recounts bones here; the verified counter is pure.
    CallGameMethod<void,0x00574460>(pose,state.matrices);
    uint32_t n=CallGameMethod<uint32_t,0x0057b4a0>(state.rig);
    auto* weights=static_cast<float**>(pose->vertexWeights);
    if(n && weights) {
        uint32_t blocks=uint32_t((uint64_t(n)+3)/4);
        if(!state.packedWeights) {
            state.weightCapacity=4*blocks;
            state.packedWeights=static_cast<Float4Value*>(Allocate(blocks>0xfffffff?0xffffffff:16*blocks,0xfc2cb4));
            std::memset(state.packedWeights,0,uint32_t(16*blocks));
        }
        for(uint32_t i=0;i<n;++i) {
            float& weight=(*weights)[i];
            if(weight<0.0f) weight=0.0f;
            reinterpret_cast<float*>(state.packedWeights)[i]=weight;
        }
    }
}
uintptr_t GetMatrix(Render* self,int32_t index,Matrix44Value* out) {
    auto& state=self->state;
    int32_t count=Count(state.rig);
    if(index<0 || index>=count) return uint32_t(count)&~uintptr_t(255);
    uintptr_t result;
    if(state.matrices) {
        *out=state.matrices[index]; result=reinterpret_cast<uintptr_t>(out);
    } else if(state.rig) {
        auto* source=reinterpret_cast<__m128*>(state.rig->worldBegin+index);
        result=reinterpret_cast<uintptr_t>(ts3::math::b11::SrtMatrix(source,reinterpret_cast<__m128*>(out)));
    } else {
        // Unreachable with a stable rig pointer: zero bones already rejects every index.
        std::memset(out,0,sizeof(*out));
        auto* f=reinterpret_cast<float*>(out); f[0]=f[5]=f[10]=f[15]=1;
        result=reinterpret_cast<uintptr_t>(out);
    }
    return (result&~uintptr_t(255))|1;
}
}
