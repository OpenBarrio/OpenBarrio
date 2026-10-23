#include "ts3/anim/m05_M86.h"
#include <cstring>
#include <intrin.h>
namespace ts3::anim::m86 {
namespace {
void Read(Reader* r,void* out) { r->good &= CallGameFunction<uint8_t,0x004dd700>(r->stream,out,r->endian); }
void ReadArray(Reader* r,void* out,uint32_t n) { r->good &= CallGameFunction<uint8_t,0x004dd650>(r->stream,out,n,r->endian); }
struct Chain { uint32_t* begin; uint32_t* end; uint32_t* capacity; void* descriptor; uint32_t values[15]; };
TS3_X86_SIZE(Chain,0x4c);
}
uint8_t Load(RigResource* self,Reader* r) {
    uint32_t version; Read(r,&version);
    if (version>=2 && version<=4) {
        if (CallGameMethod<uint8_t,0x005769d0>(&self->skeleton,r)) {
            const uint32_t n=CallGameMethod<uint32_t,0x00575650>(&self->skeleton);
            if (!n) return 0;
            CallGameMethod<void,0x0057bce0>(&self->localBegin,n);
            CallGameMethod<void,0x0057bce0>(&self->worldBegin,n);
            for(uint32_t i=0;i<n;++i) {
                auto& j=self->skeleton.begin[i];
                auto& local=self->localBegin[i]; auto& world=self->worldBegin[i];
                local.translation=j.translation; local.rotation=j.rotation; local.scale=j.scale;
                // The loader uses +58, not the joint's +50 parent field.
                if(j.index58<0) world=local;
                else CallGameFunction<void,0x0054c8d0>(&local,&self->worldBegin[j.index58],&world);
            }
            uint32_t chains=0; Read(r,&chains);
            CallGameMethod<void,0x0057c0f0>(&self->ikBegin,chains);
            for(uint32_t i=0;i<chains;++i) {
                auto* c=static_cast<Chain*>(self->ikBegin)+i;
                uint32_t count=0; Read(r,&count);
                CallGameMethod<void,0x00aef830>(c,count);
                ReadArray(r,c->begin,static_cast<uint32_t>(c->end-c->begin));
                ReadArray(r,c->values,11);
                for(unsigned k=11;k<15;++k) Read(r,c->values+k);
            }
            if(version==2) for(unsigned i=0;i<7;++i) Read(r,self->opaquebc+4*i);
            if(version<=3) {
                auto* name=reinterpret_cast<char**>(reinterpret_cast<unsigned char*>(&self->skeleton.name)+4);
                if(name[0]==name[1]) {
                    uint32_t key[2]; std::memcpy(key,self->base.opaque08,8);
#if defined(_M_IX86)
                    if(key[0]==0x65c68fc1 && key[1]==0x26e972c5)
                        CallGameMethod<void,0x00405a50>(name,reinterpret_cast<const char*>(0xfc33a0),reinterpret_cast<const char*>(0xfc33a5));
                    else {
                        const uint32_t keys[][2]={{0x7e4db284,0x52385bc5},{0x42b56538,0xe96d57c5},{0x576b847c,0x0d5a13c5},{0x94790fe3,0xe0fc2eb3},{0x7167e55a,0xa38013b3},{0xad0032a6,0x0c4b17b3},{0x34c1bcbb,0xd057fcc5},{0x27c38fc9,0x2183c4b3},{0xf5d28a72,0x192f5c47}};
                        for(unsigned i=0;i<9;++i) if(key[0]==keys[i][0] && key[1]==keys[i][1]) {
                            CallGameMethod<void,0x00405b30>(name,reinterpret_cast<const char*>(0xfc3398-8*i)); break;
                        }
                    }
#endif
                }
            }
        }
        CallGameMethod<void,0x0057b5c0>(self);
        CallGameMethod<void,0x0057b3b0>(self);
        CallGameMethod<void,0x00576030>(&self->skeleton,uint8_t(self->ikBegin!=self->ikEnd));
        return r->good;
    }
    uint32_t key[4]; std::memcpy(key,self->base.opaque08,16);
    if(key[2]!=0x8eaf13de || (version!=0xcab067b8 && version!=0x8eaf13de)) return 0;
    key[2]=0x0c244b35;
    void* manager=CallGameFunction<void*,0x004afd20>();
    if(!manager) return 0;
    void* resource=CallGameVirtual<void*,17>(manager,key);
    if(!resource) return 0;
    void* stream=nullptr;
    uint8_t opened=CallGameVirtual<uint8_t,13>(resource,key,&stream,1,3,1,0);
    uint8_t result=0;
    if(opened) {
        result=LoadStream(stream,self);
        CallGameVirtual<void,8>(stream);
    }
    if(stream) CallGameVirtual<void,2>(stream);
    return result;
}
uint8_t LoadStream(void* stream,RigResource* self) {
    Reader reader;
    void* view=CallGameVirtual<void*,6>(stream);
    CallGameMethod<Reader*,0x007d91b0>(&reader,view,1);
    const uint8_t result=Load(self,&reader);
    CallGameMethod<void,0x007d7760>(&reader);
    return result;
}
int32_t Count(const TrackMask* self) {
    return static_cast<int32_t>((reinterpret_cast<intptr_t>(self->end)-reinterpret_cast<intptr_t>(self->begin))>>2);
}
float Weight(const TrackMask* self,uint32_t index) {
    return index<static_cast<uint32_t>(Count(self)) ? self->begin[index] : -1.0f;
}
void* Query(TrackMask* self,uint32_t id) {
    return id==0x033260e3 || id==0x02269ed1 || id==0xeb54b802 || id==0xee3f516e ? self : nullptr;
}
TrackMask* Construct(TrackMask* self) {
    _InterlockedExchange(reinterpret_cast<volatile long*>(&self->base.atomicRefs),0);
    std::memset(self->base.opaque08,0,sizeof(self->base.opaque08));
    self->begin=self->end=self->capacity=nullptr;
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void*>(0x00fc33a8);
    self->descriptor=reinterpret_cast<void*>(0x00fc33c0);
#else
    static const uintptr_t table=0,descriptor=0;
    self->base.vtable=const_cast<uintptr_t*>(&table);
    self->descriptor=const_cast<uintptr_t*>(&descriptor);
#endif
    return self;
}
uint8_t __stdcall HookLoadStream(void* s,RigResource* r) {return LoadStream(s,r);}
int32_t __fastcall HookCount(const TrackMask* s,void*) {return Count(s);}
float __fastcall HookWeight(const TrackMask* s,void*,uint32_t i) {return Weight(s,i);}
void* __fastcall HookQuery(TrackMask* s,void*,uint32_t i) {return Query(s,i);}
TrackMask* __fastcall HookConstruct(TrackMask* s,void*) {return Construct(s);}
}
