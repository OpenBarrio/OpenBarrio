#include "ts3/render/m07_B31.h"
#include <cstring>
#include <xmmintrin.h>
#include "ts3/renderer/clu_798e30_Renderer.h"
namespace ts3::render::b31_detail {
void Bind(RoofSolver* s, void* owner, void* service) {
    if (!s->initialized) {
        s->owner = owner;
        s->service = service;
        s->initialized = 1;
    }
}
void Tick(RoofSolver* s, uint32_t arg, float dt) {
    if (s->state != 2) {
        const auto ready = CallGameVirtual<uint32_t, 3>(s->service);
        if (static_cast<uint8_t>(ready)) {
            // The fld/fstp argument copy quiets signaling NaNs, including their sign/payload.
            uint32_t bits;std::memcpy(&bits,&dt,4);
            if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu))bits|=0x00400000u;
            float rounded;std::memcpy(&rounded,&bits,4);
            s->state = CallGameVirtual<uint32_t, 7>(s, arg, rounded);
        }
    }
}
float Progress(RoofSolver* s) {
    return s->state == 0 ? -1.0f : s->progress;
}
RoofSolver* Construct(RoofSolver* s) {
    CallGameMethod<void, 0x00688AF0>(s);
#if defined(_M_IX86)
    s->vtable = reinterpret_cast<void**>(0x00FF0714);
    const uintptr_t tags[] = {0x00FF06F0,0x00FF06CC,0x00FF06A4,0x00FF0680,0x00FF065C};
    Vector* vectors[] = {&s->samples,&s->map,&s->blurred,&s->sampleFilter,&s->blurFilter};
    for (unsigned i=0;i<5;++i) {
        vectors[i]->begin = vectors[i]->end = vectors[i]->capacity = nullptr;
        vectors[i]->tag = reinterpret_cast<const char*>(tags[i]);
    }
#else
    NotRewritten("roof solver game vtable and allocation tags");
#endif
    s->texture = nullptr;
    s->minimum = s->maximum = s->inverse = 0.0f;
    return s;
}
RoofSolver* Delete(RoofSolver* s, uint32_t flags) {
    // Same deleting-destructor pattern as ResourceManDestroy (004A6030);
    // use this entry's destructor and allocator targets from raw disassembly.
    CallGameMethod<void, 0x00689680>(s);
    if (flags & 1) CallGameFunction<void, 0x004E3F70>(s);
    return s;
}
void* Reset(RoofSolver* s) {
    s->cursor = s->prepared = s->extra = 0;
    const uint32_t lo = 0x7f7fffff, hi = 0xc47a0000;
    std::memcpy(&s->minimum, &lo, 4);
    std::memcpy(&s->maximum, &hi, 4);
    CallGameMethod<void, 0x0056DBE0>(&s->samples, s->count);
    CallGameMethod<void, 0x00405410>(&s->map, s->bytes);
    CallGameMethod<void, 0x00405410>(&s->blurred, s->bytes);
    auto* samples = static_cast<uint32_t*>(s->samples.begin);
    for (uint32_t i=0;i<s->count;++i) samples[i] = hi;
    std::memset(s->map.begin, 0, s->bytes);
    void* result = std::memset(s->blurred.begin, 255, s->bytes);
    s->state = 0;
    return result;
}
bool RenderLot(LotLightmap* s) {
#if defined(_M_IX86)
    void* shader=*reinterpret_cast<void**>(0x011D7530);
    void* manager=*reinterpret_cast<void**>(0x011D753C);
    void* technique=CallGameMethod<void*,0x0079B840>(manager,reinterpret_cast<const char*>(0x010768EC),0u,0u);
    if(!technique)return false;
    CallGameMethod<void,0x0079A2D0>(technique,0u);
    auto texture=[&](uintptr_t tag) {
        void* allocation=CallGameFunction<void*,0x004E3FB0>(52u,4u,0u,reinterpret_cast<const char*>(tag),0u,0u,0u,0u);
        auto* t=allocation?CallGameMethod<Texture*,0x00619BC0>(allocation):nullptr;
        if(t)CallGameVirtual<void,1>(t);
        return t;
    };
    Texture* normal=texture(0x010768C8);
    CallGameMethod<void,0x00C245D0>(s->normalSource,normal,s->x,s->y,s->width,s->height);
    CallGameMethod<void,0x00799CB0>(shader,s->normalParameter,normal);
    Texture* height=texture(0x010768A4);
    CallGameMethod<void,0x00C28140>(s,height,s->x,s->y,s->width,s->height);
    CallGameMethod<void,0x00799CB0>(shader,s->heightParameter,height);
    struct List {void** vtable;Vector lights;} list;
    list.vtable=reinterpret_cast<void**>(0x010768A0);
    list.lights={nullptr,nullptr,nullptr,reinterpret_cast<const char*>(0x01076890)};
    void* scene=CallGameFunction<void*,0x006E97B0>();
    if(scene)CallGameMethod<void,0x006ACF70>(scene,&list);
    const float x=static_cast<float>(s->x),y=static_cast<float>(s->y);
    const float right=static_cast<float>(s->x+s->width),bottom=static_cast<float>(s->y+s->height);
    for(auto** it=static_cast<LotLight**>(list.lights.begin);it!=list.lights.end;++it) {
        LotLight* light=*it;
        const float* bounds=CallGameVirtual<const float*,11>(light);
        if(!(right>=bounds[0] && bounds[2]>=x && bottom>=bounds[1] && bounds[3]>=y))continue;
        if(CallGameMethod<uint64_t,0x006BC020>(light) && light->excluded)continue;
        alignas(16) float out[4];
        const float* position=CallGameVirtual<const float*,9>(light,out);
        _mm_store_ps(s->position,_mm_load_ps(position));
        CallGameMethod<void,0x00799CF0>(shader,s->positionParameter);
        _mm_store_ps(s->color,_mm_load_ps(light->color));
        // Keep the callback's ST0 result live across both x87 products and the
        // final binary32 store. The original scale constant is FLOAT 0.2, not double.
        float red=light->channels[0],scale=0.2f,value;
        void* method=light->vtable[10];
        __asm {
            mov ecx,light
            mov eax,method
            call eax
            fmul red
            fmul scale
            fstp value
        }
        s->color[3]=value;
        CallGameMethod<void,0x00799CF0>(shader,s->colorParameter);
        CallGameMethod<void,0x0079A300>(technique,0u,0u);
    }
    if(list.lights.begin)CallGameFunction<void,0x004E3F70>(list.lights.begin);
    if(height)CallGameVirtual<void,2>(height);
    if(normal)CallGameVirtual<void,2>(normal);
    return true;
#else
    (void)s;NotRewritten("terrain shader globals and x87 light intensity ABI");
#endif
}
uintptr_t InitProbe(Probe* s,uintptr_t entryEax) {
    if(s->textures[0]) return entryEax;
#if defined(_M_IX86)
    uintptr_t result=entryEax;
    auto create=[&](unsigned index,uintptr_t tag,bool diffuse,uint32_t flags) {
        void* allocation=CallGameFunction<void*,0x004E3F90>(52u,reinterpret_cast<const char*>(tag),0u,0u,0u,0u);
        Texture* t=allocation?CallGameMethod<Texture*,0x00619BC0>(allocation):nullptr;
        Texture* old=s->textures[index];
        if(t!=old) {if(t)CallGameVirtual<void,1>(t);s->textures[index]=t;if(old)CallGameVirtual<void,2>(old);}
        uint32_t size=*reinterpret_cast<uint32_t*>(diffuse?0x01158C1C:0x01158C18);
        // EA calls the create helper even if texture allocation failed.
        result=CallGameMethod<uintptr_t,0x00619910>(s->textures[index],size,diffuse?1u:0u,61u,flags,reinterpret_cast<const char*>(0x00FF3D84));
        if(static_cast<uint8_t>(result)) result=CallGameFunction<uintptr_t,0x006B1CE0>(s->textures[index]);
    };
    create(0,0x00FF3D8C,true,2);
    if(s->dynamic) {create(2,0x00FF3D68,true,256);create(4,0x00FF3D4C,true,256);}
    create(3,0x00FF3D30,false,2);
    if(s->dynamic) {create(1,0x00FF3D14,false,256);create(5,0x00FF3CF8,false,256);}
    const unsigned order[]={0,2,4,3,1,5};
    for(unsigned index:order) if(s->textures[index]) {
        void* secondary=&s->textures[index]->secondary;
        uint32_t bytes=CallGameVirtual<uint32_t,2>(secondary);
        result=CallGameVirtual<uint32_t,3>(secondary);
        *reinterpret_cast<uint32_t*>(0x011D06F8)+=bytes;
        *reinterpret_cast<uint32_t*>(0x011D06FC)+=static_cast<uint32_t>(result);
    }
    return result;
#else
    NotRewritten("light probe global texture sizes and accounting");
#endif
}
uintptr_t InitRoof(RoofSolver* s,void* owner,void* service,uint32_t width,uint32_t height,uintptr_t entryEax) {
    if(s->initialized) return entryEax;
    const auto* settings=static_cast<RoofSettings*>(s->settings);
    s->dimension=settings->dimension;
    s->scale=settings->halfResolution?settings->scale*2.0f:settings->scale;
    const uint32_t extent=width>height?width:height;
    auto projected=[&]() {
        // A uint32 times a binary32 has at most 56 significant bits: double
        // represents this product exactly, like the original x87 intermediate.
        double product=static_cast<double>(s->dimension)*static_cast<double>(s->scale);
        // fistp qword's indefinite result has a zero low word on invalid input.
        int64_t converted=(! (product>=-9223372036854775808.0 && product<9223372036854775808.0)) ? INT64_MIN : static_cast<int64_t>(product);
        return static_cast<uint32_t>(converted)>>1;
    };
    while(projected()>=extent) s->dimension>>=1;
    s->count=s->bytes=s->dimension*s->dimension;
    s->width=width*(settings->halfResolution?settings->texels>>1:settings->texels);
    s->height=height*(settings->halfResolution?settings->texels>>1:settings->texels);
#if defined(_M_IX86)
    void* allocation=CallGameFunction<void*,0x004E3F90>(52u,reinterpret_cast<const char*>(0x00FF0758),0u,0u,0u,0u);
    uintptr_t result=reinterpret_cast<uintptr_t>(allocation);
    auto* t=allocation?CallGameMethod<Texture*,0x00619BC0>(allocation):nullptr;
    if(allocation) result=reinterpret_cast<uintptr_t>(t);
    auto* old=static_cast<Texture*>(s->texture);
    if(t!=old) {if(t)result=CallGameVirtual<uintptr_t,1>(t);s->texture=t;if(old)result=CallGameVirtual<uintptr_t,2>(old);}
    if(s->texture) {
        result=CallGameMethod<uintptr_t,0x00619C80>(s->texture,s->dimension,s->dimension,1u,15u,2u,reinterpret_cast<const char*>(0x00FF0628));
        if(static_cast<uint8_t>(result)) {
            struct Lock {uint8_t* data;int32_t pitch;uint8_t opaque[32];} lock;
            CallGameMethod<void,0x00618DF0>(s->texture,&lock,0u,0u);
            if(lock.data) {
                t=static_cast<Texture*>(s->texture);
                renderer::FillBlocks7D2D40(lock.data,lock.pitch,255,t->width,static_cast<int32_t>(t->height));
                CallGameMethod<void,0x00619160>(s->texture,&lock);
            }
            CallGameMethod<void,0x0068A2C0>(s);
            result=CallGameVirtual<uintptr_t,4>(s);
            Bind(s,owner,service);
            return result;
        }
    }
    if(s->texture) {old=static_cast<Texture*>(s->texture);s->texture=nullptr;return CallGameVirtual<uintptr_t,2>(old);}
    return result;
#else
    (void)owner;(void)service;NotRewritten("roof texture allocation and lock bridge");
#endif
}
uintptr_t GrowWallPool(WallPool* s) {
#if defined(_M_IX86)
    uint32_t height=CallGameMethod<uint32_t,0x00688C50>(s);
    if(s->texture && s->texture->height>=height) return reinterpret_cast<uintptr_t>(s->texture);
    auto account=[&](Texture* t,bool subtract) {
        void* secondary=&t->secondary;
        uint32_t a=CallGameVirtual<uint32_t,2>(secondary),b=CallGameVirtual<uint32_t,3>(secondary);
        *reinterpret_cast<uint32_t*>(0x011D06E8)+=subtract?0u-a:a;
        *reinterpret_cast<uint32_t*>(0x011D06EC)+=subtract?0u-b:b;
    };
    if(s->texture) {
        account(s->texture,true);
        if(s->texture) {auto* old=s->texture;s->texture=nullptr;CallGameVirtual<void,2>(old);}
    }
    void* memory=CallGameFunction<void*,0x004E3F90>(52u,reinterpret_cast<const char*>(0x00FF0634),0u,0u,0u,0u);
    Texture* t=memory?CallGameMethod<Texture*,0x00619BC0>(memory):nullptr;
    auto* old=s->texture;
    if(t!=old) {if(t) CallGameVirtual<void,1>(t);s->texture=t;if(old) CallGameVirtual<void,2>(old);}
    if(!s->texture) return 0;
    uintptr_t result=CallGameMethod<uintptr_t,0x00619C80>(s->texture,1024u,height,1u,16u,2u,reinterpret_cast<const char*>(0x00FF0628));
    if(!static_cast<uint8_t>(result)) {
        if(s->storage) {void* storage=s->storage;CallGameMethod<void,0x006AC300>(storage);CallGameFunction<void,0x004E3F70>(storage);}
        s->storage=nullptr;
        if(s->texture) {old=s->texture;s->texture=nullptr;return CallGameVirtual<uintptr_t,2>(old);}
        return result;
    }
    if(s->texture) account(s->texture,false);
    CallGameMethod<void,0x00688D50>(s);
    if(!s->storage) {
        memory=CallGameFunction<void*,0x004E3FB0>(64u,4u,0u,reinterpret_cast<const char*>(0x00FF0604),0u,0u,0u,0u);
        s->storage=memory?CallGameMethod<void*,0x006AC4F0>(memory,1024u,height,13u):nullptr;
    } else CallGameMethod<void,0x006AC3C0>(s->storage,1024u,height);
    return CallGameMethod<uintptr_t,0x006AC350>(s->storage,50u);
#else
    (void)s;NotRewritten("wall pool texture accounting globals");
#endif
}
uint32_t Solve(RoofSolver* s, uint32_t arg, float dt) {
    struct Owner { uint8_t opaque[0x88]; uint32_t key; };
    const auto key = static_cast<Owner*>(s->owner)->key;
    if (static_cast<uint8_t>(CallGameVirtual<uint32_t,8>(s->service,key))) {
        if (!s->prepared) {
            CallGameVirtual<void,7>(s->service,s);
            s->inverse = 1.0f / (s->maximum - s->minimum);
            s->cursor = 0;
            s->prepared = 1;
        }
        if (!static_cast<uint8_t>(CallGameMethod<uint32_t,0x00689D70>(s,arg,dt))) return 0;
        if (!static_cast<uint8_t>(CallGameMethod<uint32_t,0x00688FA0>(s,arg,dt))) return 0;
        CallGameMethod<void,0x006191D0>(s->texture,0u,s->blurred.begin,s->dimension,0u,0u);
    }
    if ((static_cast<uint8_t*>(s->samples.end)-static_cast<uint8_t*>(s->samples.begin))/4)
        CallGameMethod<void,0x0056DBE0>(&s->samples,0u);
    Vector temp;
    const auto n = static_cast<uint32_t>((static_cast<uint8_t*>(s->samples.end)-static_cast<uint8_t*>(s->samples.begin))/4);
    CallGameMethod<void,0x0069DB60>(&temp,n,&s->samples.tag);
    size_t bytes = static_cast<uint8_t*>(s->samples.end)-static_cast<uint8_t*>(s->samples.begin);
    auto* moved = static_cast<uint8_t*>(std::memmove(temp.begin,s->samples.begin,bytes));
    void* old = s->samples.begin;
    s->samples.begin=temp.begin;s->samples.end=moved+bytes;s->samples.capacity=temp.capacity;
    if (old) CallGameFunction<void,0x004E3F70>(old);
    Vector* vectors[]={&s->map,&s->blurred};
    for (Vector* v:vectors) {
        if (v->end != v->begin) CallGameMethod<void,0x00405410>(v,0u);
        bytes=static_cast<uint8_t*>(v->end)-static_cast<uint8_t*>(v->begin);
        void* allocation=nullptr;
        if (bytes) {
#if defined(_M_IX86)
            allocation=CallGameFunction<void*,0x004E3F90>(static_cast<uint32_t>(bytes),v->tag,0u,0u,reinterpret_cast<const char*>(0x00F98980),0xe7u);
#else
            NotRewritten("roof buffer allocation site");
#endif
        }
        const auto used=static_cast<uint8_t*>(v->end)-static_cast<uint8_t*>(v->begin);
        moved=static_cast<uint8_t*>(std::memmove(allocation,v->begin,used));
        old=v->begin;v->begin=allocation;v->end=moved+used;v->capacity=static_cast<uint8_t*>(allocation)+bytes;
        if(old) CallGameFunction<void,0x004E3F70>(old);
    }
    return 2;
}
bool Notify(RoofSolver* s, uint32_t event, uint32_t) {
    if (event == 0x0486519d) {
        CallGameMethod<void, 0x0068A2C0>(s);
        CallGameVirtual<void, 4>(s);
    }
    return false;
}
uint32_t CastRays(RaySolver* s, uint32_t arg, const float* start, const float* finish, uint32_t mode) {
#if defined(_M_IX86)
    auto* flag = reinterpret_cast<uint32_t*>(0x011CF538);
    auto* results = reinterpret_cast<Vector*>(0x011CF528);
    if (!(*flag & 1)) {
        *flag |= 1;
        results->begin = results->end = results->capacity = nullptr;
        results->tag = reinterpret_cast<const char*>(0x00FF0738);
        CallGameFunction<int, 0x00F7E9A1>(reinterpret_cast<void*>(0x00F91FF0));
    }
    auto cast = [&](uint32_t i, bool negative) {
        const float ratio = static_cast<float>(static_cast<double>(i) / static_cast<double>(s->divisions));
        const __m128 a = _mm_load_ps(start), b = _mm_load_ps(finish);
        const __m128 delta = negative ? _mm_sub_ps(_mm_sub_ps(_mm_setzero_ps(), b), a) : _mm_sub_ps(b, a);
        alignas(16) float point[4];
        _mm_store_ps(point, _mm_add_ps(_mm_mul_ps(_mm_set1_ps(ratio), delta), a));
        CallGameVirtual<void, 1>(s->caster, results, arg, point);
        for (uint32_t j=0;j<s->owner->count;++j) {
            auto* records = *reinterpret_cast<uint8_t**>(0x01158870);
            CallGameMethod<void, 0x00689310>(records + 44*j, results, mode);
        }
    };
    for (uint32_t i=1;i<s->divisions-1;++i) cast(i,false);
    for (uint32_t i=1;i<s->divisions;++i) cast(i,true);
    return s->divisions;
#else
    (void)s;(void)arg;(void)start;(void)finish;(void)mode;
    NotRewritten("AO ray global scratch vector");
#endif
}
uintptr_t ResizeRecords(Vector* s, uint32_t count) {
    auto* begin = static_cast<uint8_t*>(s->begin);
    auto* end = static_cast<uint8_t*>(s->end);
    const uint32_t old = static_cast<uint32_t>((end - begin) / 44);
    if (count <= old)
        return CallGameMethod<uintptr_t, 0x0068AAF0>(s, begin + 44 * count, end);
    struct Record { uint8_t head[12]; Vector vis, blur; };
    Record record;
    record.vis = {nullptr,nullptr,nullptr,nullptr};
    record.blur = {nullptr,nullptr,nullptr,nullptr};
#if defined(_M_IX86)
    record.vis.tag = reinterpret_cast<const char*>(0x00FF05E4);
    record.blur.tag = reinterpret_cast<const char*>(0x00FF05C0);
#endif
    CallGameMethod<void, 0x0068AB40>(s, end, count - old, &record);
    if (record.blur.begin) CallGameFunction<void, 0x004E3F70>(record.blur.begin);
    if (record.vis.begin) return CallGameFunction<uintptr_t, 0x004E3F70>(record.vis.begin);
    return 0;
}
}
