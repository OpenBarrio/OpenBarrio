#include "ts3/render/m07_B35.h"
#include <atomic>
#include <cstring>
#include <xmmintrin.h>
namespace ts3::render::m07::b35 {
Resolver resolve=nullptr;
uint8_t* fading=nullptr;
SculptureIds* sculptureIds=nullptr;
namespace {
template<class R,uint32_t A,class... T> R F(T... a) {
    if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(a...);
    return CallGameFunction<R,A>(a...);
}
template<class R,uint32_t A,class... T> R M(void* s,T... a) {
    if(resolve) if(auto p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(s,nullptr,a...);
    return CallGameMethod<R,A>(s,a...);
}
template<class R,int S,class... T> R V(void* s,T... a) {
    return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>((*static_cast<void***>(s))[S])(s,nullptr,a...);
}
template<class T,uint32_t A> T& G() {
    if(resolve) if(auto p=resolve(A)) return *static_cast<T*>(p);
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(A);
#else
    AbortNotRewritten("effect selection global");static T unavailable{};return unavailable;
#endif
}
void* Alloc(uint32_t n,const char* tag) {return F<void*,0x004E3F90>(n,tag,0u,0u,0u,0u);}
}
void* __cdecl CaptureTexture(int32_t width,int32_t height) {
    if(!uint8_t(F<uintptr_t,0x00619770>(&width,&height,0u,0u))) return nullptr;
    auto p=Alloc(52,"Renderer/ScreenCaptureEffects/TempBufferTexture");
    if(!p) return nullptr;
    auto texture=M<void*,0x00619BC0>(p);if(!texture) return nullptr;
    V<void,1>(texture);
    if(!uint8_t(M<uintptr_t,0x00619C80>(texture,width,height,1u,63u,1u,"ScreenCapture"))) {
        V<void,2>(texture);return nullptr;
    }
    return texture;
}
EffectBin* __fastcall ConstructEffects(EffectBin* s,void*) {
    for(auto& row:s->records) for(auto& word:row) word=0;
    s->filters={nullptr,nullptr,nullptr,"Renderer/ScreenCaptureEffects/EffectBin/Filters"};
    s->ramps={nullptr,nullptr,nullptr,"Renderer/ScreenCaptureEffects/EffectBin/mRamps"};
    M<void,0x004FA800>(&s->random,-1);
    for(auto& word:s->state) word=0;
    s->ready=0;return s;
}
void __fastcall InitializeEffects(EffectBin* s,void*) {
    if(s->ready) return;
    s->effects=M<uint32_t,0x0079B840>(G<void*,0x011D753C>(),"ScreenCaptureEffects",0u,0u);
    s->styles=M<uint32_t,0x0079B840>(G<void*,0x011D753C>(),"PhotographyStyles",0u,0u);
    s->styleTexture=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"StyleTexture",1u,5u);
    s->source=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"SourceTexture",1u,5u);
    s->distortion=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"DistortionTexture",1u,5u);
    s->value=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"ValueRampTexture",1u,5u);
    s->red=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"RedRampTexture",1u,5u);
    s->green=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"GreenRampTexture",1u,5u);
    s->blue=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"BlueRampTexture",1u,5u);
    s->scale=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"SourceUVScale",1u,2u);
    s->controls=M<uint32_t,0x0079A160>(G<void*,0x011D7530>(),"Controls",1u,2u);
    M<void,0x00707680>(s);M<void,0x007088B0>(s);s->ready=1;
}
Link* __fastcall AddDependency(Dependencies* s,void*,uint32_t key) {
    auto* head=&s->head;
    for(auto* link=head->next;link!=head;link=link->next) {
        auto* entry=reinterpret_cast<Dependency*>(reinterpret_cast<uint8_t*>(link)-offsetof(Dependency,link));
        if(entry->key==key) {++entry->count;return link;}
    }
    auto p=Alloc(32,"Renderer/DependencyEntry");if(!p) return nullptr;
    auto* entry=M<Dependency*,0x006DECB0>(p,s,key);if(!entry) return nullptr;
    auto* link=&entry->link;
    link->previous=head->previous;link->next=head;head->previous=link;link->previous->next=link;
    return link;
}
uint8_t __stdcall SimpleRTT(int32_t w,int32_t h,uint32_t ti,uint32_t ri,uint32_t format,uint32_t) {
    auto* manager=F<void*,0x00616FC0>();
    if(!format) format=61;
    auto p=Alloc(52,"Renderer/RenderSystem/AddSimpleRTT");if(!p) return 0;
    auto* texture=M<void*,0x00619BC0>(p);if(!texture) return 0;
    V<void,1>(texture);
    if(uint8_t(M<uintptr_t,0x00619C80>(texture,w,h,1u,format,1u,"RenderSystem"))) {
        p=Alloc(40,"Renderer/RenderSystem/AddSimpleRTT");
        if(p) {
            auto* target=M<void*,0x00617FE0>(p);
            if(target) {
                V<void,1>(target);
                M<void,0x006194B0>(texture,0u,target);
                M<void,0x00617580>(manager,ri,target);
                M<void,0x00617650>(manager,ti,texture);
                V<void,2>(target);V<void,2>(texture);return 1;
            }
        }
    }
    V<void,2>(texture);return 0;
}
uint8_t __stdcall RenderTarget(int32_t w,int32_t h,uint32_t index,uint32_t format) {
    auto* manager=F<void*,0x00616FC0>();
    if(!format) format=141;
    auto p=Alloc(40,"Renderer/RenderSystem/RenderTarget");if(!p) return 0;
    auto* target=M<void*,0x00618060>(p);if(!target) return 0;
    V<void,1>(target);
    if(!uint8_t(M<uintptr_t,0x00617BB0>(target,w,h,format,16u))) {V<void,2>(target);return 0;}
    M<void,0x00617580>(manager,index,target);V<void,2>(target);return 1;
}
uintptr_t __stdcall CubeRTT(void* manager,int32_t size,uint32_t index) {
    auto p=Alloc(52,"Renderer/RenderSystem/AddSimpleCubeRTT");if(!p) return 0;
    auto* texture=M<void*,0x00619BC0>(p);if(!texture) return 0;
    V<void,1>(texture);
    if(uint8_t(M<uintptr_t,0x00619910>(texture,size,1u,61u,1u,"RenderSystem"))) M<void,0x00617650>(manager,index,texture);
    return V<uintptr_t,2>(texture);
}
void __fastcall ResolvePick(PickState* self,void*) {
    if(self->x<0||!self->dirty) return;
    self->dirty=0;
    auto** target=M<void**,0x00617540>(G<void*,0x011CECDC>(),0x6ee45453u);
    self->savedX=self->x;self->savedY=self->y;self->pixel=0;self->objectIndex=self->partIndex=-1;
    for(int r=0;r<4;++r) _mm_store_ps(self->savedTransform[r],_mm_load_ps(self->transform[r]));
    self->x=self->y=-1;
    if(!target) return;
    PickLock locked;
    M<void,0x00618DF0>(*target,&locked,0u,2u);
    if(!locked.data) return;
    PickMap map;const char* tag="Scratch/RenderPickManager/IndexToObjInfoMap";
    M<void,0x00AA3730>(&map,&tag);
    for(uint32_t y=0;y<16;++y) for(uint32_t x=0;x<16;++x) {
        auto pixel=*reinterpret_cast<uint32_t*>(locked.data+y*locked.pitch+x*4);
        uint32_t key=uint32_t(uint16_t(pixel))-1;
        PickIterator found;M<void,0x00C45F60>(&map,&found,&key);
        uint32_t dx=8-x,dy=8-y,distance=dx*dx+dy*dy;
        if(found.node==map.buckets[map.bucketCount]) {
            PickEntry entry{key,distance,pixel};PickInsertResult result;
            M<void,0x006DFDD0>(&map,&result,&entry,0u);
        } else if(found.node->distance>distance||(found.node->distance==distance&&found.node->pixel>pixel)) {
            found.node->distance=distance;found.node->pixel=pixel;
        }
    }
    uint32_t chosen=*reinterpret_cast<uint32_t*>(locked.data+8*locked.pitch+32);
    if(map.size>1) {
        uint32_t best=0xffffffffu;PickIterator it;M<void,0x00626610>(&map,&it);
        auto* end=map.buckets[map.bucketCount];
        while(it.node!=end) {
            auto* object=M<void*,0x006E4020>(self->base.context,it.node->key);
            if(object&&(M<uint32_t,0x006C93D0>(object)&0x800)&&it.node->distance<best) {
                chosen=it.node->pixel;best=it.node->distance;
            }
            it.node=it.node->next;
            while(!it.node) it.node=*++it.bucket;
        }
    }
    self->pixel=chosen;self->objectIndex=int32_t(uint32_t(uint16_t(chosen))-1);
    _mm_store_ps(self->position,_mm_setzero_ps());
    if(self->objectIndex>=0) {
        if(self->partPick) self->partIndex=int32_t(chosen>>24);
        else {
            struct Dimensions {uint8_t prefix[12];float x,y;};
            auto* dims=reinterpret_cast<Dimensions*>(self->base.renderSystem);
            __m128 half=_mm_set_ss(G<float,0x00FF7554>());
            __m128 y=_mm_cvtsi32_ss(_mm_setzero_ps(),self->savedY);
            y=_mm_add_ss(y,half);y=_mm_mul_ss(y,_mm_set_ss(self->inverseHeight));
            y=_mm_sub_ss(half,y);y=_mm_mul_ss(y,_mm_set_ss(dims->y));
            y=_mm_shuffle_ps(y,y,0);
            __m128 p=_mm_mul_ps(y,_mm_load_ps(self->savedTransform[1]));
            __m128 x=_mm_cvtsi32_ss(_mm_setzero_ps(),self->savedX);
            x=_mm_add_ss(x,half);x=_mm_mul_ss(x,_mm_set_ss(self->inverseWidth));x=_mm_sub_ss(x,half);
            x=_mm_mul_ss(x,_mm_set_ss(dims->x));x=_mm_shuffle_ps(x,x,0);
            p=_mm_add_ps(p,_mm_mul_ps(x,_mm_load_ps(self->savedTransform[0])));
            p=_mm_sub_ps(p,_mm_load_ps(self->savedTransform[2]));
            __m128 depth=_mm_cvtsi32_ss(_mm_setzero_ps(),int32_t(chosen>>16));
            depth=_mm_mul_ss(depth,_mm_set_ss(G<float,0x00FF7650>()));
            depth=_mm_sub_ss(depth,_mm_set_ss(G<float,0x00FF7648>()));
            depth=_mm_div_ss(_mm_set_ss(G<float,0x00FF764C>()),depth);depth=_mm_shuffle_ps(depth,depth,0);
            p=_mm_mul_ps(p,depth);p=_mm_add_ps(p,_mm_load_ps(self->savedTransform[3]));
            _mm_store_ps(self->position,p);
        }
    }
    M<void,0x00619160>(*target,&locked);
    // The cleanup helper is stdcall, despite an incidental ECX value in the caller.
    using Cleanup=void(__stdcall*)(PickNode**,uint32_t);
    if(resolve&&resolve(0x00AEBDB0)) reinterpret_cast<Cleanup>(resolve(0x00AEBDB0))(map.buckets,map.bucketCount);
#if defined(_M_IX86)
    else reinterpret_cast<Cleanup>(0x00AEBDB0)(map.buckets,map.bucketCount);
#else
    else AbortNotRewritten("pick map cleanup");
#endif
    map.size=0;
    if(map.bucketCount>1) F<void,0x004E3F70>(map.buckets);
}
uintptr_t __fastcall BuildSculpture(SculptureBuilder* self,void*,void* source) {
    auto* drawable=static_cast<uint8_t*>(source)+0x210;
    self->state=0;
    auto* mesh=M<void*,0x006DB800>(drawable);
    auto* model=M<MaterialList*,0x006D0290>(mesh,0u);
    if(!model) {model=M<MaterialList*,0x006DB810>(drawable);if(!model) return 0;}
    if(!uint8_t(M<uintptr_t,0x006DAEF0>(model))) {
        auto* other=M<void*,0x006DB810>(drawable);
        if(other&&other!=model&&M<void*,0x00F1D820>(drawable)) {
            auto* data=M<void*,0x00F1D820>(drawable);M<void,0x006DAEB0>(model,data);
        }
    }
    auto result=M<uintptr_t,0x006DAEF0>(model);if(!uint8_t(result)) return result;
    auto count=M<uint32_t,0x006C93D0>(model);
    auto* p=Alloc(736,"Renderer/SculptureCapture/ModelBuilder");
    self->builder=p?M<uint8_t*,0x006D3E30>(p,0u):nullptr;
    if(!self->builder) return 0;
    self->builder[704]=1;self->builder[705]=1;
    uint32_t flag=0;
    M<void,0x006D66A0>(self->builder,1u,&flag);
    M<void,0x006D0A40>(self->builder,0u,0.0f,200.0f);
    M<void,0x006D7660>(self->builder);
    M<void,0x006D45F0>(self->builder,0u,7u,0u);
    M<void,0x006D45F0>(self->builder,1u,5u,0u);
    M<void,0x006D45F0>(self->builder,5u,5u,0u);
    M<void,0x006D45F0>(self->builder,2u,6u,0u);
    M<void,0x006D6310>(self->builder);
    alignas(16) uint8_t parameters[32];
    M<void,0x00799300>(parameters);
    float diffuse[4]={1.0f,1.0f,1.0f,1.0f};uint32_t uv[4]={939524352u,0,0,0};
    M<void,0x00799700>(parameters,"diffuse",diffuse,3u);
    M<void,0x00799700>(parameters,"UVScales",uv,3u);
    M<void,0x006D4750>(self->builder,G<uint32_t,0x0115B3C0>(),parameters);
    flag=(flag&0xffffff00u)|1;uint8_t enabled=1;uint32_t mode=3;
    M<void,0x006D76E0>(self->builder,&mode,&enabled,&flag,0u,0u);
    auto* owner=M<void*,0x006F4A50>(source);auto id=M<uint32_t,0x00580420>(owner);
    for(uint32_t i=0;i<count;++i) if(auto* part=model->materials[i]) {
        M<void,0x0068F3F0>(part,id);M<void,0x0070A1A0>(self,part);
    }
    M<void,0x006D1870>(self->builder);M<void,0x006D7650>(self->builder);
    return M<uintptr_t,0x00799330>(parameters);
}
uintptr_t __stdcall FinishSculpture(void* source) {
    CaptureKey output{};
    auto* app=F<uint8_t*,0x00984550>();
    struct App {uint8_t prefix[0x664];void* capture;};
    struct Path {void* first;void* last;uint32_t token;};
    if(app&&reinterpret_cast<App*>(app)->capture) {
        Path path;
        M<void,0x007D76D0>(&path);
        if(uint8_t(M<uintptr_t,0x009CB600>(reinterpret_cast<App*>(app)->capture,&path))) {
            uint64_t time=0;
            using Clock=int(__stdcall*)(uint64_t*);
            G<Clock,0x00F953B8>()(&time);
            CaptureKey key{time,23466547u,46382583u};
            auto* resources=F<void*,0x0072F1C0>();
            if(resources) {
                while(uint8_t(V<uintptr_t,1>(resources,&key,0u,0u,1u,6u))) key.instance=++time;
                if(uint8_t(M<uintptr_t,0x007D8290>(&path,&key,1u))) {
                    auto saved=uint8_t(M<uintptr_t,0x00709900>(source,path.token,&key));
                    M<void,0x007D7840>(&path);
                    if(saved) output=key;
                }
            }
        }
        M<void,0x007D7760>(&path);
    }
    auto* bus=F<void*,0x007692C0>();
    uintptr_t result=reinterpret_cast<uintptr_t>(bus);
    if(bus) {
        auto* message=static_cast<CaptureDoneMessage*>(source);
        result=M<uintptr_t,0x009DC780>(&message,"Renderer/SculptureCaptureDoneMessage");
        if(message) {
            message->key[0]=uint32_t(output.instance);message->key[1]=uint32_t(output.instance>>32);
            message->key[2]=output.type;message->key[3]=output.group;
            V<void,7>(bus,0xebde6e98u,message,0u,0u);
            result=V<uintptr_t,2>(message);
        }
    }
    return result;
}
uint32_t __cdecl ChooseEffects(uint32_t category,uint32_t level,uint32_t* filter,uint32_t* effect) {
    uint32_t mask=(&G<uint32_t,0x0115B318>())[category];
    if(G<uint8_t,0x011D2070>()) mask=G<uint32_t,0x0115B314>();
    if(G<uint8_t,0x011D2068>()) level=G<uint32_t,0x011D206C>();
    auto& flags=G<uint32_t,0x011D2064>();
    auto& fi=G<Vector,0x011D2054>();auto& ei=G<Vector,0x011D2044>();
    if(!(flags&1)) {
        flags|=1;fi={nullptr,nullptr,nullptr,"Renderer/ScreenCaptureEffects/EffectBin/sFilterIndices"};
#if defined(_M_IX86)
        F<void,0x00F7E9A1>(reinterpret_cast<void*>(0x00F92430));
#else
        AbortNotRewritten("effect filter atexit callback");
#endif
    }
    fi.end=fi.begin;
    if(!(flags&2)) {
        flags|=2;ei={nullptr,nullptr,nullptr,"Renderer/ScreenCaptureEffects/EffectBin/sEffectIndices"};
#if defined(_M_IX86)
        F<void,0x00F7E9A1>(reinterpret_cast<void*>(0x00F92410));
#else
        AbortNotRewritten("effect index atexit callback");
#endif
    }
    ei.end=ei.begin;
    auto append=[](Vector& v,uint32_t i) {
        if(reinterpret_cast<uintptr_t>(v.end)<reinterpret_cast<uintptr_t>(v.capacity)) {
            auto* p=static_cast<uint32_t*>(v.end);v.end=p+1;if(p) *p=i;
        } else M<void,0x008621C0>(&v,v.end,&i);
    };
    auto* first=G<Filter*,0x011D1F90>();auto* last=G<Filter*,0x011D1F94>();
    for(uint32_t i=0;i<uint32_t(last-first);++i) if((mask&first[i].mask)&&first[i].maximum>=level&&first[i].minimum<=level) append(fi,i);
    auto* ef=G<Effect*,0x011D1FA0>();auto* el=G<Effect*,0x011D1FA4>();
    for(uint32_t i=0;i<uint32_t(el-ef);++i) if((mask&ef[i].mask)&&ef[i].maximum>=level&&ef[i].minimum<=level) append(ei,i);
    *filter=G<uint32_t,0x011D203C>();
    void* random=nullptr;
    if(resolve) random=resolve(0x011D1FB0);
#if defined(_M_IX86)
    else random=reinterpret_cast<void*>(0x011D1FB0);
#endif
    if(!G<uint8_t,0x011D2040>()) {
        auto n=uint32_t(static_cast<uint32_t*>(fi.end)-static_cast<uint32_t*>(fi.begin));
        auto i=M<uint32_t,0x004FA840>(random,n);*filter=static_cast<uint32_t*>(fi.begin)[i];
    }
    uint32_t result=G<uint32_t,0x011D2034>();*effect=result;
    if(!G<uint8_t,0x011D2038>()) {
        auto n=uint32_t(static_cast<uint32_t*>(ei.end)-static_cast<uint32_t*>(ei.begin));
        result=M<uint32_t,0x004FA840>(random,n);*effect=static_cast<uint32_t*>(ei.begin)[result];
    }
    return result;
}
float* __fastcall BindSculpture(SculptureParameters* self,void*,SculptureBinding* out,int32_t* count,const SculptureStyle* style,uint32_t group) {
    SculptureIds ids{};SculptureIds* state=sculptureIds;
#if defined(_M_IX86)
    if(!state) {
        ids={*reinterpret_cast<uint32_t*>(0x011D209C),*reinterpret_cast<uint32_t*>(0x011D2098),*reinterpret_cast<uint32_t*>(0x011D2094),*reinterpret_cast<uint32_t*>(0x011D2090),*reinterpret_cast<uint32_t*>(0x011D208C),*reinterpret_cast<uint32_t*>(0x011D2088)};
        state=&ids;
    }
#else
    if(!state) AbortNotRewritten("sculpture binding identifiers");
#endif
    uint32_t* keys[]={&state->diffuse,&state->normal,&state->shininess,&state->specular,&state->specStyle};
    const char* names[]={"DiffuseMap","NormalMap","Shininess","Specular","SpecStyle"};
    for(unsigned i=0;i<5;++i) if(!(state->flags&(1u<<i))) {
        state->flags|=1u<<i;
#if defined(_M_IX86)
        if(!sculptureIds) *reinterpret_cast<uint32_t*>(0x011D209C)=state->flags;
#endif
        *keys[i]=F<uint32_t,0x007CFE70>(names[i]);
#if defined(_M_IX86)
        if(!sculptureIds) *reinterpret_cast<uint32_t*>(0x011D2098-4*i)=*keys[i];
#endif
    }
    uint32_t groupId=0;uint32_t diffuse[4]{},normal[4]{};
    F<void,0x007D7630>(group,&groupId);
    // This is a callback address, not an empty string (IDA labels it 0x00B2D882).
    void* empty=resolve?resolve(0x00B2D882):nullptr;
#if defined(_M_IX86)
    if(!empty) empty=reinterpret_cast<void*>(0x00B2D882);
#else
    if(!empty) AbortNotRewritten("sculpture key callback");
#endif
    F<void,0x004AECE0>(diffuse,style->diffuse,empty,groupId,0u,0u);
    out[*count].id=state->diffuse;out[*count].type=4;
    for(int i=0;i<4;++i) out[*count].key[i]=diffuse[i];
    ++*count;
    if(style->normal) {
        F<void,0x004AECE0>(normal,style->normal,empty,groupId,0u,0u);
        out[*count].id=state->normal;out[*count].type=4;
        for(int i=0;i<4;++i) out[*count].key[i]=normal[i];
        ++*count;
    }
    out[*count].id=state->shininess;out[*count].type=1;out[*count].count=1;
    *self->next=style->shininess;out[*count].values=self->next;++self->next;++*count;
    out[*count].id=state->specStyle;out[*count].type=1;out[*count].count=1;
    *self->next=style->specStyle;out[*count].values=self->next;++self->next;++*count;
    out[*count].id=state->specular;out[*count].type=1;out[*count].count=3;
    auto* result=self->next;
    for(int i=0;i<3;++i) {
        // EA uses x87 fld/fstp here, unlike the two scalar movss copies above.
#if defined(_M_IX86)
        const float* from=&style->specular[i];float* to=result+i;
        __asm { mov ecx, from }
        __asm { mov edx, to }
        __asm { fld dword ptr [ecx] }
        __asm { fstp dword ptr [edx] }
#else
        uint32_t bits;std::memcpy(&bits,&style->specular[i],4);
        if((bits&0x7f800000u)==0x7f800000u&&(bits&0x007fffffu)) bits|=0x00400000u;
        std::memcpy(result+i,&bits,4);
#endif
    }
    out[*count].values=result;self->next+=3;++*count;return result;
}
uintptr_t __fastcall SelectDiffuse(DiffuseSelection* self,void*,MaterialList* list) {
    const auto phong=F<uint32_t,0x007CFE70>("Phong");
    const auto solid=F<uint32_t,0x007CFE70>("SolidPhong");
    uintptr_t result=M<uintptr_t,0x006C93D0>(list);
    for(uint32_t i=0;i<result;) {
        auto* material=M<void*,0x00444710>(list->materials[i]);
        if(material) {
            auto* descriptor=M<uint32_t*,0x006C93D0>(material);
            if(descriptor[2]==phong||descriptor[2]==solid) {
                auto* binding=M<Binding*,0x006C9B60>(material,"DiffuseMap");
                result=reinterpret_cast<uintptr_t>(binding);
                if(binding&&binding->kind==6&&binding->type==4) {
                    material=M<void*,0x00444710>(list->materials[i]);
                    result=M<uintptr_t,0x00F1D820>(material)+binding->data->offset;
                    auto* src=reinterpret_cast<uint32_t*>(result);
                    for(int j=0;j<4;++j) self->value[j]=src[j];
                }
                return result;
            }
        }
        ++i;result=M<uintptr_t,0x006C93D0>(list);
    }
    return result;
}
uint8_t __cdecl SetFading(uint8_t enabled) {
    uint8_t* state=fading;
#if defined(_M_IX86)
    if(!state) state=reinterpret_cast<uint8_t*>(0x01158F0C);
#else
    if(!state) AbortNotRewritten("object fading global");
#endif
    if(enabled==*state) return enabled;
    if(!enabled) *state=0;
    else {
        auto disabled=uint8_t(F<uintptr_t,0x0058B100>("DisableObjectFading",0u));
        *state=disabled?0:1;
    }
    auto* manager=F<void*,0x0044C470>();if(!manager) return 0;
    struct Message {void** table;uint32_t refs,a,b,id,tail;};
    auto* message=static_cast<Message*>(Alloc(24,"Renderer/SceneObjectManager/Msg"));
    if(message) {
        message->id=0;
        if(resolve) message->table=static_cast<void**>(resolve(0x0100DF80));
#if defined(_M_IX86)
        else message->table=reinterpret_cast<void**>(0x0100DF80);
#else
        else AbortNotRewritten("fading message vtable");
#endif
        reinterpret_cast<std::atomic<uint32_t>*>(&message->refs)->exchange(0);
        V<void,1>(message);
    }
    message->id=0xc415b92du;
    V<void,7>(manager,0xc415b92du,message,0u,0u);
    return uint8_t(V<uintptr_t,2>(message));
}
uintptr_t __cdecl SendSculpture(uint32_t a,uint32_t b) {
    auto* manager=F<void*,0x0044C470>();if(!manager) return 0;
    struct Message {void** table;uint32_t refs,a,b;};
    auto* message=F<Message*,0x004E3FB0>(16u,8u,0u,"Renderer/SculptureCaptureMessage",0u,0u,0u,0u);
    if(!message) return 0;
    if(resolve) message->table=static_cast<void**>(resolve(0x0100DF80));
#if defined(_M_IX86)
    else message->table=reinterpret_cast<void**>(0x0100DF80);
#else
    else AbortNotRewritten("sculpture message vtable");
#endif
    reinterpret_cast<std::atomic<uint32_t>*>(&message->refs)->exchange(0);
    V<void,1>(message);message->a=a;message->b=b;
    V<void,7>(manager,0x6b08859bu,message,0u,0u);
    return V<uintptr_t,2>(message);
}
}
