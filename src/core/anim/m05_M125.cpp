#include "ts3/anim/m05_M125.h"
#include <cstring>
#include <intrin.h>
namespace ts3::anim::m125_impl {
namespace {
float Accumulate(float height, double base, float sum) {
#if defined(_M_IX86)
    // The original subtract/add stay on the x87 stack until the final float store.
    float result;
    __asm {
        fld qword ptr [base]
        fsubr dword ptr [height]
        fadd dword ptr [sum]
        fstp dword ptr [result]
    }
    return result;
#else
    return static_cast<float>((static_cast<double>(height)-base)+static_cast<double>(sum));
#endif
}
void Retain(Entity* p) { if (p) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&p->references),1); }
void Release(Entity* p) {
    if (p && _InterlockedDecrement(reinterpret_cast<volatile long*>(&p->references)) == 0) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&p->references),1);
        CallGameVirtual<void,0>(p,1);
    }
}
void Transfer(Entity*& p, Entity* next) {
    if (next != p) { Retain(next); Entity* old=p; p=next; Release(old); }
}
}
int32_t PositionLevel(const Float4Value* position) {
#if defined(_M_IX86)
    void* world=*reinterpret_cast<void**>(0x011ecbc4);
    if (!world) return 0;
    void* region=CallGameMethod<void*,0x00c68990>(world,position->lane[0],position->lane[2]);
    if (!region) return 0;
    alignas(16) uint32_t result[4];
    result[0]=result[1]=0xbf800000u; result[2]=0;
    if (CallGameFunction<uint8_t,0x00b01390>(region,position,0,1))
        CallGameFunction<void,0x00add800>(region,position,result);
    else CallGameFunction<void,0x00add740>(region,position,result);
    return static_cast<int32_t>(result[2]);
#else
    (void)position; NotRewritten("m125 routing region global");
#endif
}
void* FindStyle(StyleList* self) {
    for (Link* p=self->children.next; p != &self->children; p=p->next) {
        StyleNode* node=p ? reinterpret_cast<StyleNode*>(reinterpret_cast<char*>(p)-offsetof(StyleNode,link)) : nullptr;
        void* result=CallGameVirtual<void*,1>(node,0x065e0c92u);
        if (result) return result;
    }
    return nullptr;
}
double HeightDifference(Entity* entity, uint32_t idLow, uint32_t idHigh) {
    float sum=0.0f;
    while (entity) {
        float height=static_cast<float>(CallGameVirtual<double,75>(entity));
        double base=CallGameVirtual<double,41>(entity,idLow,idHigh);
        sum=Accumulate(height,base,sum);
        Retain(entity);
        if (!CallGameFunction<uint8_t,0x0081f550>(entity)) { Release(entity); break; }
        Transfer(entity,CallGameVirtual<Entity*,31>(entity));
    }
    return sum;
}
Float4Value* AncestorPosition(Float4Value* out, Entity* entity) {
    while (entity) {
        Float4Value scratch;
        const Float4Value* p=CallGameVirtual<Float4Value*,77>(entity,&scratch);
        std::memcpy(out,p,sizeof(*out));
        Transfer(entity,CallGameVirtual<Entity*,31>(entity));
    }
    return out;
}
}

namespace ts3::anim::m125_impl {
namespace {
template<class T> T& Field(void* p,size_t offset) { return *reinterpret_cast<T*>(static_cast<char*>(p)+offset); }
template<class T,uintptr_t Address> T Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(Address);
#else
    NotRewritten("m125 external game global");
#endif
}
uint64_t Id(void* p,size_t slot) {
    if(slot==1) return CallGameVirtual<uint64_t,1>(p);
    if(slot==4) return CallGameVirtual<uint64_t,4>(p);
    return CallGameVirtual<uint64_t,5>(p);
}
bool Ignored(uint64_t id,const uint64_t* ids,int count) {
    return CallGameFunction<uint8_t,0x0085aaa0>(uint32_t(id),uint32_t(id>>32),ids,count)!=0;
}
bool Related(void* owner,uint64_t id,bool second=true) {
    if(CallGameMethod<uint8_t,0x0081c970>(owner,uint32_t(id),uint32_t(id>>32))) return true;
    return second && CallGameMethod<uint8_t,0x0081c930>(owner,uint32_t(id),uint32_t(id>>32));
}
void PlainRelease(Entity* p) {
    if(--p->references==0) { p->references=1; CallGameVirtual<void,0>(p,1); }
}
float Distance(const Float4Value& a,const Float4Value& b) {
    // Match the SSE reduction: (z*z + y*y) + x*x.
    float x=a.lane[0]-b.lane[0],y=a.lane[1]-b.lane[1],z=a.lane[2]-b.lane[2];
    return (z*z+y*y)+x*x;
}
}
uint64_t FindObstruction(Entity* entity,void* body,const Float4Value* position,const uint64_t* ignore,int count) {
    uint64_t found=0;
    void* owner=entity ? CallGameVirtual<void*,3>(entity) : nullptr;
    void* route=CallGameFunction<void*,0x008292d0>();
    if(route) {
        void* world=Global<void*,0x011ecbc4>();
        void* grid=CallGameMethod<void*,0x00829bc0>(route);
        int level=PositionLevel(position);
        Float4Value point=*position;
        void* current=CallGameFunction<void*,0x008292d0>();
        point.lane[1]=current ? static_cast<float>(CallGameMethod<double,0x00829470>(current,point.lane[0],point.lane[2],level,1,1)) : 0.f;
        void* region=CallGameMethod<void*,0x00c68990>(world,position->lane[0],position->lane[2]);
        struct alignas(16) Probe { Float4Value position,zero; float radius; int level; uint32_t tail[3]; } probe{};
        probe.position=point; probe.radius=1.f; probe.level=level;
        struct Results { Entity** begin; Entity** end; Entity** capacity; const char* label; } results{nullptr,nullptr,nullptr,"Locomotion/FootprintSet"};
        void* flagsObject=body ? static_cast<char*>(body)-8 : nullptr;
        uint32_t flags=flagsObject ? CallGameMethod<uint32_t,0x00953690>(flagsObject) : 256;
        if(CallGameMethod<uint8_t,0x0082edb0>(route,&probe,0.f,&results,1,flags)) {
            for(Entity** it=results.begin;it!=results.end;++it) {
                Entity* p=*it; if(!p) continue;
                ++p->references;
                bool skip=CallGameVirtual<int,2>(p)==20;
                if(!skip) skip=Ignored(Id(p,1),ignore,count);
                if(!skip && owner) {
                    uint64_t id=Id(p,1);
                    skip=Related(owner,id,false);
                    if(!skip) { id=Id(p,1); skip=CallGameMethod<uint8_t,0x0081c930>(owner,uint32_t(id),uint32_t(id>>32))!=0; }
                }
                bool hit=!region;
                if(!skip && region) {
                    uint32_t x,z;
                    CallGameMethod<void,0x0062ef30>(grid,Field<uint32_t>(region,72),Field<uint32_t>(region,76),position->lane[0],position->lane[2],&x,&z);
                    void* footprint=reinterpret_cast<char*>(p)-sizeof(void*);
                    uint32_t n=CallGameVirtual<uint32_t,0>(footprint);
                    for(uint32_t i=0;i<n;++i) {
                        Float4Value cell; uint32_t words[4]={x,0,z,x}; memcpy(&cell,words,16);
                        if(CallGameVirtual<uint8_t,46>(footprint,i,&cell)) { hit=true; break; }
                    }
                }
                if(!skip && hit) found=Id(p,1);
                PlainRelease(p);
                if(!skip && hit) break;
            }
        }
        CallGameMethod<void,0x00aac090>(&results,results.begin,results.end);
        if(results.begin) CallGameFunction<void,0x004e3f70>(results.begin);
        if(found) return found;
    }
    Float4Value source{}; uint64_t sourceId=0;
    if(body) { sourceId=Id(body,4); CallGameVirtual<void,15>(body,&source); }
    void* sentinel=Global<void*,0x011d9140>();
    auto next=[](void* node)->void* { void* link=Field<void*>(node,64); return link ? static_cast<char*>(link)-64 : nullptr; };
    for(void* node=next(sentinel);node!=sentinel;node=next(node)) {
        void* candidate=static_cast<char*>(node)+8;
        if(sourceId) {
            uint64_t id=Id(candidate,5);
            if(id==sourceId || (owner && Related(owner,id))) continue;
        }
        if(CallGameMethod<void*,0x00c93970>(candidate)) continue;
        if(owner && Related(owner,Id(candidate,4),false)) continue;
        if(Ignored(Id(candidate,4),ignore,count)) continue;
        if(CallGameVirtual<int,6>(candidate)!=1 && CallGameVirtual<int,6>(candidate)!=4) continue;
        Float4Value center;
        CallGameVirtual<void,7>(candidate,&center);
        void* object=Field<void*>(node,4);
        void* sim=nullptr;
        if(object) {
            void* component=CallGameMethod<void*,0x00935770>(object,Global<void*,0x011e90fc>());
            if(component && !(Field<uintptr_t>(component,48)&3)) sim=Field<void*>(component,48);
        }
        float extent[4]={.25f,.25f,.25f,.25f}; float radius=.090000004f;
        if(sim) {
            uint32_t flags=Field<uint32_t>(sim,56)&0xcf00;
            if(!flags) flags=256;
            if(flags!=256) {
                void* manager=CallGameFunction<void*,0x005eb7c0>();
                float scratch[4]; float* p=CallGameMethod<float*,0x005ecb30>(manager,scratch,Field<uint32_t>(sim,56));
                memcpy(extent,p,16);
                float x=extent[2]+extent[0],z=extent[3]+extent[1]; radius=(x*x+z*z)*.5f;
            }
        }
        if((body && !(Distance(center,source)>radius)) || !(radius>=Distance(center,*position))) continue;
        void* transform=object ? CallGameMethod<void*,0x00935770>(object,Global<void*,0x011e9158>()) : nullptr;
        alignas(16) Float4Value axes[4]; CallGameMethod<void,0x00959060>(transform,axes);
        float polygon[8];
        for(int lane=0;lane<4;++lane) {
            float up=(axes[2].lane[lane]*1.2f)*extent[1],down=(axes[2].lane[lane]*-1.2f)*extent[3];
            float left=(axes[0].lane[lane]*-1.2f)*extent[2]+center.lane[lane];
            float right=(axes[0].lane[lane]*1.2f)*extent[0]+center.lane[lane];
            if(lane==0 || lane==2) {
                int k=lane==0 ? 0 : 1;
                polygon[k]=left+down; polygon[2+k]=right+down; polygon[4+k]=right+up; polygon[6+k]=left+up;
            }
        }
        float point[2]={position->lane[0],position->lane[2]};
        if(CallGameFunction<uint8_t,0x007cd7e0>(point,polygon,4)) return Id(candidate,4);
    }
    return found;
}
uintptr_t UpdateObstruction(void* self,int unused) {
    (void)unused;
    void* body=Field<void*>(self,12);
    Entity* entity=CallGameMethod<Entity*,0x00c93970>(body);
    if(!entity) return 0;
    Retain(entity); Retain(entity);
    void* host=CallGameMethod<void*,0x0083a280>(body,entity);
    void* style=CallGameVirtual<void*,1>(host,0x14cfd2f8);
    if(!style) {
        if(_InterlockedDecrement(reinterpret_cast<volatile long*>(&entity->references))==0) {
            entity->references=1; return CallGameVirtual<uintptr_t,0>(entity,1);
        }
        return 0;
    }
    Entity* group=Field<Entity*>(style,268);
    if(group) {
        Retain(group);
        if(Field<void*>(group,20)==static_cast<char*>(static_cast<void*>(group))+20) {
            void* ownStyle=CallGameVirtual<void*,1>(body,0x14cfd2f8);
            if(Field<int>(body,12)>0 && Field<uint8_t>(body,32) &&
               (((CallGameMethod<uint32_t,0x00953660>(ownStyle,0)&0xcf00)==0x200) ||
                ((CallGameMethod<uint32_t,0x00953690>(ownStyle)&0xcf00)==0xe00))) {
                void* slave=CallGameMethod<void*,0x00839ca0>(body,0);
                if(slave && slave!=host) CallGameMethod<void,0x0085d760>(group,slave);
            }
        }
        CallGameMethod<void,0x0085d760>(group,host);
        float weight=1.f;
        if(CallGameMethod<StyleList*,0x00580420>(body)) {
            void* s=FindStyle(CallGameMethod<StyleList*,0x00580420>(body));
            if(s) weight=Field<float>(s,12);
        }
        CallGameFunction<void,0x0085d980>(host,entity,group,weight);
    }
    if(!CallGameVirtual<uint8_t,39>(entity)) {
        uint64_t id=Id(body,4); Retain(entity);
        float height=static_cast<float>(HeightDifference(entity,uint32_t(id),uint32_t(id>>32)));
        float limit=15.f;
        if(CallGameMethod<StyleList*,0x00580420>(body)) {
            void* s=FindStyle(CallGameMethod<StyleList*,0x00580420>(body));
            if(s) { float w=Field<float>(s,12)*2.f; limit=1.5f>w ? 1.5f : w; }
        }
        if(limit>=height) {
            Retain(entity); Float4Value position; AncestorPosition(&position,entity);
            int n=Field<int>(body,12); uint32_t total=uint32_t(n)+1;
            uint64_t bytes=uint64_t(total)*8;
            auto* ids=CallGameFunction<uint64_t*,0x004e3f90>(bytes>UINT32_MAX ? UINT32_MAX : uint32_t(bytes),"Locomotion/IgnoreList",0,0,0,0);
            ids[0]=Id(body,4);
            if(n>0) {
                auto* slaves=CallGameFunction<uint64_t*,0x004e3f90>(uint32_t(n)>0x1fffffff ? UINT32_MAX : uint32_t(n)*8,"Locomotion/SlaveIDs",0,0,0,0);
                CallGameMethod<void,0x00839cd0>(body,slaves);
                memcpy(ids+1,slaves,size_t(n)*8); CallGameFunction<void,0x004e3f70>(slaves);
            }
            if(FindObstruction(entity,body,&position,ids,static_cast<int>(total))) CallGameVirtual<void,25>(body);
            CallGameFunction<void,0x004e3f70>(ids);
        }
    }
    Release(group);
    uintptr_t result=reinterpret_cast<uintptr_t>(&entity->references);
    if(_InterlockedDecrement(reinterpret_cast<volatile long*>(&entity->references))==0) {
        entity->references=1; return CallGameVirtual<uintptr_t,0>(entity,1);
    }
    return result;
}
}
