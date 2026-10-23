#include "ts3/render/m07_M14.h"
#include <cstring>
#include <limits>
namespace ts3::render::m07 {
namespace {
template<class T> size_t Count(const M14Vector<T>& v) { return v.begin ? v.end-v.begin : 0; }
struct Draw { uint32_t priority,index; M14Item* item; };
static __m128 LoadBounds(const float* p) { return _mm_set_ps(p[0],p[2],p[1],p[0]); }
static void Union(__m128* bounds,const __m128* other) {
    ts3::CallGameMethod<void,0x0068BA00>(bounds,bounds,other);
}
static M14List* NewList(M14Renderer* r) {
    M14List temporary; temporary.next=reinterpret_cast<M14Node*>(&temporary);
    temporary.previous=temporary.next; temporary.pool=&r->pool;
    auto& v=r->lists;
    if (v.end<v.capacity) {
        auto p=v.end++;
        if(p) {
            p->next=reinterpret_cast<M14Node*>(p); p->previous=p->next; p->pool=&r->pool;
            ts3::CallGameMethod<void,0x0071F470>(p,p,temporary.next,reinterpret_cast<M14Node*>(&temporary),0);
        }
    } else ts3::CallGameMethod<void,0x00721170>(&v,v.end,&temporary);
    auto n=temporary.next;
    while(n!=reinterpret_cast<M14Node*>(&temporary)) { auto old=n; n=n->next; old->next=r->pool.free; r->pool.free=old; }
    return r->lists.end-1;
}
static void AddIndex(M14List* list,uint32_t index,bool inlineGrow) {
    auto pool=list->pool; auto node=pool->free;
    if(!node) {
        if(inlineGrow) {
            uint32_t size=pool->growth&0x7fffffff;
            if(pool->growth&0x80000000) { size=pool->growth*2; if(size>1363) size=1363; pool->growth=(pool->growth&0x80000000)|size; }
            void* allocator=nullptr;
#if defined(_M_IX86)
            allocator=*reinterpret_cast<void**>(0x011CB864);
#endif
            void* allocation=ts3::CallGameMethod<void*,0x004E6A20>(allocator,uint32_t(sizeof(M14Node)*size+4),4,0,0);
            node=ts3::CallGameMethod<M14Node*,0x007CCEA0>(pool,allocation,4,uint32_t(sizeof(M14Node)),4,size);
        } else node=ts3::CallGameMethod<M14Node*,0x0071AD80>(pool);
    }
    if(node) pool->free=node->next;
    // The original does not tolerate allocation failure here.
    node->index=index; node->next=reinterpret_cast<M14Node*>(list); node->previous=list->previous;
    list->previous->next=node; list->previous=node;
}
static uint32_t MaterialFlags(void* material) {
    auto p=ts3::CallGameMethod<uint8_t*,0x008AB2B0>(material);
    return *reinterpret_cast<uint32_t*>(p+24);
}
static int Flags(uint32_t v) {
    int f=0; if(v&1) f=1024; if(v&2) f|=2048; if(v&4) f|=4; if(v&8) f|=4096; return f?f:1;
}
static uint32_t Priority(const M14Item& item,const float* camera) {
    float x=(item.low[0]+item.high[0])*0.5f-camera[0];
    float y=(item.high[1]+item.low[1])*0.5f-camera[1];
    float z=(item.high[2]+item.low[2])*0.5f-camera[2];
    float d=(x*x+z*z)+y*y;
    int64_t value;
#if defined(_M_IX86)
    // Retain the x87 extended intermediate and truncating FISTP, including integer-indefinite.
    float bias=item.bias, scale=-100.0f; unsigned short saved,truncation;
    __asm { fnstcw saved }
    truncation=saved|0xc00;
    __asm {
        fld bias
        fld d
        fld st(0)
        fadd st(0),st(1)
        fadd st(0),st(2)
        fmulp st(2),st(0)
        faddp st(1),st(0)
        fmul scale
        fldcw truncation
        fistp value
        fldcw saved
    }
#else
    long double distance=d,bias=item.bias;
    long double v=(bias*(distance+distance+bias)+distance)*-100.0L;
    value=(v>=-9223372036854775808.0L && v<9223372036854775808.0L)?int64_t(v):std::numeric_limits<int64_t>::min();
#endif
    return (uint32_t(item.override5c&15)<<28)-uint32_t(value);
}
static bool Compatible(const M14Item& a,const M14Item& b) {
    return (!a.special || (a.key68==b.key68 && a.key6c==b.key6c)) && a.key18==b.key18
        && a.special==b.special && a.mesh==b.mesh && a.key20==b.key20 && a.key24==b.key24
        && a.flag==b.flag && a.variant==b.variant && a.type==b.type && a.key58==b.key58;
}
}
int BuildSwarmDrawLists(M14Renderer* r,M14Output* out,void* transform) {
    out->count=0;
    ts3::CallGameMethod<void,0x0071AB00>(r);
    __m128 initial;
#if defined(_M_IX86)
    auto& flags=*reinterpret_cast<uint32_t*>(0x011D2F90);
    auto& value=*reinterpret_cast<__m128*>(0x011D2F80);
    if(!(flags&1)) { flags|=1; value=_mm_set1_ps(1000000.0f); }
    initial=value;
#else
    initial=_mm_set1_ps(1000000.0f);
#endif
    alignas(16) __m128 bounds[2]={initial,_mm_sub_ps(_mm_setzero_ps(),initial)};
    uint32_t recordCount=0;
    ts3::CallGameMethod<void,0x006F7AB0>(&r->records,uint32_t(Count(r->items)+Count(r->scenery)+1));
    ts3::CallGameMethod<void,0x0071F190>(&r->lists,r->lists.begin,r->lists.end);
    r->lists.end=r->lists.begin;
    for(auto mesh=r->meshes.begin; mesh!=r->meshes.end; ++mesh) {
        if(mesh->object && mesh->enabled) {
            ts3::CallGameVirtual<void,18>(mesh->object);
            auto object=ts3::CallGameMethod<void*,0x006DB800>(static_cast<uint8_t*>(mesh->object)+528);
            if(object) {
                auto model=ts3::CallGameMethod<void*,0x006D0290>(object,0);
                if(model) {
                    auto source=ts3::CallGameMethod<__m128*,0x005042A0>(model);
                    alignas(16) __m128 local[2]={source[0],source[1]};
                    ts3::CallGameMethod<void,0x0068D7C0>(local,local,mesh);
                    Union(bounds,local);
                }
            }
        }
    }
    if(_mm_cvtss_f32(bounds[1])>=_mm_cvtss_f32(bounds[0])) {
        AddIndex(NewList(r),0xffffffff,false);
        r->records.begin[0].flags=1; r->records.begin[0].low=bounds[0]; r->records.begin[0].high=bounds[1];
        recordCount=1;
    }
    if(Count(r->items)) {
        auto cameraObject=ts3::CallGameFunction<void*,0x006E8330>();
        auto cameraData=ts3::CallGameMethod<uint8_t*,0x006E8400>(cameraObject);
        __m128 c=*reinterpret_cast<__m128*>(cameraData+96);
        __m128 position=_mm_add_ps(_mm_add_ps(_mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(c,c,85),r->cameraMatrix[1]),
            _mm_mul_ps(_mm_shuffle_ps(c,c,0),r->cameraMatrix[0])),
            _mm_mul_ps(_mm_shuffle_ps(c,c,170),r->cameraMatrix[2])),
            _mm_mul_ps(_mm_shuffle_ps(c,c,255),r->cameraMatrix[3]));
        alignas(16) float camera[4]; _mm_store_ps(camera,position);
        M14Vector<Draw> groups[5]{};
        for(auto& g:groups) g.tag="Renderer/SwarmRenderer/tDrawList";
        uint32_t index=0;
        for(auto item=r->items.begin;item!=r->items.end;++item,++index) {
            if(!item->enabled || !item->active || (!item->mesh && !item->material) || !(item->low[0]<=item->high[0])) continue;
            int group=0;
            if(item->type==3 || item->type==9) group=2;
            else if(item->type==16) { auto f=MaterialFlags(item->material); group=(f&1)?3:(f&2)?4:(f&4)?0:2-((f&8)!=0); }
            switch(item->override5c) { case 1:group=3;break; case 2:group=4;break; case 4:group=1;break; }
            Draw draw{Priority(*item,camera),index,item}; auto& g=groups[group];
            if(g.end<g.capacity) { auto p=g.end++; if(p) *p=draw; }
            else ts3::CallGameMethod<void,0x006D3940>(&g,g.end,&draw);
        }
        for(uint32_t group=0;group<5;++group) {
            auto& g=groups[group];
            ts3::CallGameFunction<void,0x0071ED70>(g.begin,g.end,0);
            for(auto draw=g.begin;draw!=g.end;++draw) {
                if(draw->index==0xffffffff) continue;
                auto& record=r->records.begin[recordCount++]; auto list=NewList(r);
                AddIndex(list,draw->index,true); draw->index=0xffffffff;
#if defined(_M_IX86)
                record.flags=reinterpret_cast<const uint32_t*>(0x00FFCA58)[group];
#else
                static constexpr uint32_t groupFlags[]={4,4096,1,1024,2048}; record.flags=groupFlags[group];
#endif
                if(draw->item->special) record.flags=128;
                record.low=LoadBounds(draw->item->low); record.high=LoadBounds(draw->item->high);
                for(auto other=draw+1;other!=g.end;++other) {
                    if(other->index!=0xffffffff && Compatible(*draw->item,*other->item)) {
                        alignas(16) __m128 otherBounds[2]={LoadBounds(other->item->low),LoadBounds(other->item->high)};
                        ts3::CallGameMethod<void,0x0068BA30>(&record,otherBounds);
                        AddIndex(list,other->index,false); other->index=0xffffffff;
                    }
                }
                Union(bounds,&record.low);
            }
        }
        for(int i=4;i>=0;--i) if(groups[i].begin) ts3::CallGameFunction<void,0x004E3F70>(groups[i].begin);
    }
    r->swarmListCount=uint32_t(Count(r->lists));
    uint32_t index=0;
    for(auto s=r->scenery.begin;s!=r->scenery.end;++s,++index) {
        if(!s->enabled || !s->active || !s->object || !(s->low[0]<=s->high[0])) continue;
        auto& record=r->records.begin[recordCount++];
        AddIndex(NewList(r),index+r->swarmListCount,true);
        int flags=4;
        if(s->type==16) {
            if(s->material) flags=Flags(MaterialFlags(s->material));
            else {
                auto object=static_cast<uint8_t*>(s->object);
                if(*reinterpret_cast<void**>(object+24)) ts3::CallGameVirtual<void,18>(s->object);
                auto model=ts3::CallGameMethod<uint8_t*,0x006DB810>(object+528);
                if(model && ts3::CallGameMethod<int,0x006C93D0>(model) && **reinterpret_cast<void***>(model+64)) {
                    auto data=**reinterpret_cast<void***>(model+64);
                    auto a=ts3::CallGameMethod<uint8_t,0x0068F990>(data,0);
                    auto b=ts3::CallGameMethod<uint8_t,0x0068F990>(data,7);
                    flags=Flags(a)|Flags(b);
                }
            }
        }
        switch(s->override4c) { case 1:flags=1024;break;case 2:flags=2048;break;case 4:flags=4096;break; }
        record.flags=flags; record.low=LoadBounds(s->low); record.high=LoadBounds(s->high);
        Union(bounds,&record.low);
    }
    out->count=uint32_t(Count(r->lists)); out->records=r->records.begin;
    for(int i=0;i<4;++i) out->matrix[i]=r->matrix[i];
    alignas(16) __m128 matrix[4]; auto transformed=ts3::CallGameMethod<__m128*,0x006FAF40>(transform,matrix);
    ts3::CallGameMethod<void,0x006FCD80>(out,transformed);
    return ts3::CallGameMethod<int,0x006FCE80>(out,bounds);
}
}
