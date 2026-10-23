#include "ts3/render/m07_M08.h"
#include <xmmintrin.h>
namespace ts3::render::m07::m08 {
namespace {
uint32_t Intern(Bindings* b,const char* n,uint32_t a,uint32_t c) { return CallThis(Original<Thiscall<uint32_t,Bindings,const char*,uint32_t,uint32_t>>(0x0079A160),b,n,a,c); }
void* Device() { return Original<void*(*)()>(0x0060E2C0)(); }
void* Resources() { return Original<void*(*)()>(0x00616FC0)(); }
TextureSize** Lookup(void* r,uint32_t key) { return CallThis(Original<Thiscall<TextureSize**,void,uint32_t>>(0x00617540),r,key); }
void Enable(ScenePass* s,uint32_t a) { CallThis(Original<Thiscall<void,ScenePass,uint32_t>>(0x006EDE00),s,a); }
void Select(void* s,uint32_t id) { CallThis(Original<Thiscall<void,void,uint32_t>>(0x0079A2D0),s,id); }
void Upload(Bindings* b,uint32_t id,const float* value) { CallThis(Original<Thiscall<void,Bindings,uint32_t,const float*>>(0x0068E870),b,id,value); }
uint8_t SignalA(void* s) { return CallThis(Original<Thiscall<uint8_t,void>>(0x00594540),s); }
uint8_t SignalB(void* s) { return CallThis(Original<Thiscall<uint8_t,void>>(0x005943C0),s); }
void* Texture(void* s) { return CallThis(Original<Thiscall<void*,void>>(0x00722F30),s); }
void Bind(Bindings* b,uint32_t id,void* value) { CallThis(Original<Thiscall<void,Bindings,uint32_t,void*>>(0x005F72F0),b,id,value); }
uint32_t Find(void* s,const char* n) { return CallThis(Original<Thiscall<uint32_t,void,const char*>>(0x0079A550),s,n); }
void Draw(void* s,uint32_t a) { CallThis(Original<Thiscall<void,void,uint32_t>>(0x0060F990),s,a); }
void Clear(void* s,uint32_t a,uint32_t b) { CallThis(Original<Thiscall<void,void,uint32_t,uint32_t>>(0x0079A300),s,a,b); }
template<class T> T* Global(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<T*>(uintptr_t(a));
#else
    (void)a; throw std::runtime_error("M08 globals require native injection");
#endif
}
void Set(Bindings* b,uint32_t id,const void* value) {
    if(id < b->count) {
        Binding& e=b->entries[id]; ++e.revision;
        e.count=1;e.type=2;e.generation=b->generation;e.value=value;
    }
}
float Depth(const Camera* c,float z) {
    // Keep the four SSE products and the left-to-right packed additions, including zero products.
    __m128 r=_mm_add_ps(_mm_mul_ps(_mm_set1_ps(0.0f),_mm_load_ps(c->rows[1])),
                        _mm_mul_ps(_mm_set1_ps(0.0f),_mm_load_ps(c->rows[0])));
    r=_mm_add_ps(r,_mm_mul_ps(_mm_set1_ps(z),_mm_load_ps(c->rows[2])));
    r=_mm_add_ps(r,_mm_mul_ps(_mm_set1_ps(1.0f),_mm_load_ps(c->rows[3])));
    return _mm_cvtss_f32(_mm_div_ps(_mm_shuffle_ps(r,r,170),_mm_shuffle_ps(r,r,255)));
}
void Offsets(float* dest,const TextureSize* t) {
    // No intermediate float reciprocal: the original retains x87 extended precision until each store.
#if defined(_M_IX86)
    uint32_t w=t->width,h=t->height;
    static const float half=0.5f,twoHalf=2.5f;
    static const double unsignedFix=4294967296.0;
    __asm {
        fild w
        test w,80000000h
        jz positiveW
        fadd unsignedFix
    positiveW:
        fld1
        fdivrp st(1),st(0)
        fild h
        test h,80000000h
        jz positiveH
        fadd unsignedFix
    positiveH:
        fld1
        fdivrp st(1),st(0)
        mov eax,dest
        fld st(1)
        fmul half
        fstp dword ptr[eax]
        fld st(0)
        fmul half
        fstp dword ptr[eax+4]
        fxch st(1)
        fmul twoHalf
        fstp dword ptr[eax+8]
        fmul twoHalf
        fstp dword ptr[eax+12]
    }
#else
    long double x=1.0L/t->width,y=1.0L/t->height;
    dest[0]=float(x*0.5L);dest[1]=float(y*0.5L);dest[2]=float(x*2.5L);dest[3]=float(y*2.5L);
#endif
}
}
const PassCalls& OriginalPassCalls() {
    static const PassCalls c{Intern,Device,Resources,Lookup,Enable,Select,Upload,SignalA,SignalB,Texture,Bind,Find,Draw,Clear}; return c;
}
PassGlobals OriginalPassGlobals() {
    return {*Global<Bindings*>(0x011D7530),Global<uint32_t>(0x011D1BF4),
        {Global<uint32_t>(0x011D1BF0),Global<uint32_t>(0x011D1BEC),Global<uint32_t>(0x011D1BE8),Global<uint32_t>(0x011D1BE4),Global<uint32_t>(0x011D1BE0)},
        Global<uint32_t>(0x011D1BB4),Global<float>(0x011D1BD0),Global<float>(0x011D1BC0),Global<float>(0x011D08D0),Global<uint8_t>(0x011D188C),
        {Global<void*>(0x011D1874),Global<void*>(0x011D186C),Global<void*>(0x011D1870),Global<void*>(0x011D1878)},
        {Global<uint32_t>(0x011D187C),Global<uint32_t>(0x011D1880),Global<uint32_t>(0x011D1884),Global<uint32_t>(0x011D1888)},Global<void>(0x011CDD48)};
}
uintptr_t SubmitPass(ScenePass* self,int32_t pass,const PassGlobals& g,const PassCalls& c) {
    static const char* names[]={"scratchTextureOffsets","translucentTextureOffsets","BloomData","ShimmerControl","UnderSeaControl"};
    for(unsigned i=0;i<5;++i) if((*g.flags & (1u<<i))==0) {
        *g.flags |= 1u<<i;
        *g.ids[i]=c.intern(g.bindings,names[i],1,2);
    }
    void* device=c.device();
    if(static_cast<uint8_t*>(device)[0x8D]) return reinterpret_cast<uintptr_t>(device);
    void* resources=c.resources();
    TextureSize** scratch=c.lookup(resources,0x76B9AF5E);
    TextureSize** translucent=c.lookup(resources,0x75BD7813);
    alignas(16) float a[4],b[4];
    Offsets(a,*scratch);Offsets(b,*translucent);
    Set(g.bindings,*g.ids[0],a);Set(g.bindings,*g.ids[1],b);
    auto bind=[&](unsigned i) { c.bind(g.bindings,*g.textureIds[i],*g.texturesEnabled?c.texture(*g.textures[i]):nullptr); };
    switch(pass) {
    case 0:
        c.enable(self,1);c.select(self->shaders,self->passes[1]);c.upload(g.bindings,*g.ids[2],g.bloom);break;
    case 1: {
        c.enable(self,1);c.select(self->shaders,self->passes[2]);
        const float depth=Depth(self->camera,-0.0f-self->waterHeight);
        *g.flags |= 0x20;
        const float active=(c.signalA(g.signal)||c.signalB(g.signal))?0.0f:1.0f;
        g.shimmer[0]=depth;g.shimmer[1]=active;
        g.shimmer[2]=0.0f;g.shimmer[3]=0.0f;
        c.upload(g.bindings,*g.ids[3],g.shimmer);bind(0);break;
    }
    case 2:
        bind(1);bind(2);c.select(self->shaders,self->passes[0]);break;
    case 3:
        if((*g.flags & 0x80)==0) { *g.flags |= 0x80;*g.shadowId=c.find(self->shaders,"ShadowDebug"); }
        c.select(self->shaders,*g.shadowId);break;
    case 4:
        c.enable(self,1);c.select(self->shaders,self->passes[3]);
        g.underSea[0]=Depth(self->camera,-1.0f);*g.flags |= 0x40;
        g.underSea[1]=self->sea[0];g.underSea[2]=self->sea[1];g.underSea[3]=self->sea[2];
        c.upload(g.bindings,*g.ids[4],g.underSea);bind(3);break;
    default: break;
    }
    c.draw(device,2);c.clear(self->shaders,0,0);
    Set(g.bindings,*g.ids[0],nullptr);
    uintptr_t result=*g.ids[1];
    if(*g.ids[1] < g.bindings->count) {
        Set(g.bindings,*g.ids[1],nullptr);
        result=reinterpret_cast<uintptr_t>(&g.bindings->entries[*g.ids[1]]);
    }
    return result;
}
#if defined(_M_IX86)
uintptr_t __fastcall HookSubmitPass(ScenePass* self,void*,int32_t pass) { return SubmitPass(self,pass,OriginalPassGlobals()); }
#endif
namespace {
template<uint32_t A,class R=void,class T,class... Args> R Method(T* self,Args... args) {
    return CallThis(Original<Thiscall<R,T,Args...>>(A),self,args...);
}
template<uint32_t A,class R=void,class... Args> R Function(Args... args) {
    return Original<R(*)(Args...)>(A)(args...);
}
uint32_t TargetId(const TargetSystem* s,size_t offset) {
    return *reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(s)+offset);
}
void TargetBinding(Bindings* b,uint32_t id,const void* value) {
    if(id<b->count) {auto& e=b->entries[id];++e.revision;e.generation=b->generation;e.value=value;}
}
uint32_t FloorPower(uint32_t v) {
    uint32_t p=v-1;p|=p>>16;p|=p>>8;p|=p>>4;p|=p>>2;p|=p>>1;++p;
    if(p>v)v=p>>1;
    return v<256?256:v;
}
uintptr_t EngineRef(void* p,unsigned slot) {
    return reinterpret_cast<Thiscall<uintptr_t,void>>((*static_cast<void***>(p))[slot])(p,nullptr);
}
void* AllocateTarget(uint32_t size,const char* name) {
    return Function<0x004E3F90,void*>(size,name,0u,0u,0u,0u);
}
uint32_t TruncateLow(float value) {
#if defined(_M_IX86)
    uint16_t cw,trunc;int64_t result;
    __asm { fnstcw cw }
    trunc=cw|0xC00;
    __asm {
        fldcw trunc
        fld value
        fistp result
        fldcw cw
    }
    return uint32_t(result);
#else
    if(!(value>=-9223372036854775808.0 && value<9223372036854775808.0))return 0;
    return uint32_t(int64_t(value));
#endif
}
}
uintptr_t InitializeTargets(TargetSystem* s,uint8_t refresh) {
    Bindings* bindings=*Global<Bindings*>(0x011D7530);
    Method<0x006E3D20>(s);
    s->totalBytes=(s->pendingBytes+s->totalBytes)|0x80000000u;s->pendingBytes=0;
    void* device=Device();void* resources=Resources();
    auto* mask=Method<0x00533300,uint32_t*>(device);
    Method<0x0060EE40>(device);
    uint32_t w=FloorPower(mask[6]>>1),h=FloorPower(mask[7]>>1),format=mask[8];
    Method<0x006E1D00>(s,resources,128u,801975116u);s->ready=1;
    Method<0x006E1D70>(s,resources);
    auto target=[&](uint32_t x,uint32_t y,uint32_t key,uint32_t name,uint32_t f) {
        Method<0x006E1B80>(s,x,y,key,name,f,0u);
    };
    auto pair=[&](uint32_t name,uint32_t key,unsigned offset) {
        Method<0x006E11E0>(s,name,key,TargetId(s,offset),TargetId(s,offset+4));
    };
    target(w,h,1991880542u,1207485594u,format);
    *Global<uint32_t>(0x011D16B0)|=1;
    float* dimensions=Global<float>(0x011D16A0);
    dimensions[0]=float(w);dimensions[1]=float(h);dimensions[2]=float(1.0/double(w));dimensions[3]=float(1.0/double(h));
    TargetBinding(bindings,TargetId(s,28),dimensions);
    target(w>>1,h>>1,uint32_t(-177601888),uint32_t(-2133443540),format);
    uint32_t tw=w,th=h;
    if(s->enhanced) {tw=2*w;th=2*h;if(tw>2048)tw=2048;if(th>1024)th=1024;}
    target(tw,th,1975351315u,1738382669u,format);
    if(s->feature179)target(w,h,uint32_t(-1140053774),uint32_t(-1269698946),format);
    auto special=[&](uint32_t object,uint32_t flag,uint32_t key,unsigned offset) {
        Method<0x0070BE70>(Global<void>(object));*Global<uint8_t>(flag)=1;
        Method<0x0070BEB0>(Global<void>(object));
        void** slot=Method<0x00617540,void**>(resources,key);
        TargetBinding(bindings,TargetId(s,offset),slot?*slot:nullptr);
    };
    if(s->feature178) {
        target(64,64,1950398374u,uint32_t(-816017598),61);
        special(0x0115B3C4,0x0115B3CE,0x633E72E2,200);
    } else *Global<uint8_t>(0x0115B3CE)=0;
    if(s->feature178 && s->feature17b) special(0x0115B3DC,0x0115B3E6,0x0D536718,56);
    else *Global<uint8_t>(0x0115B3E6)=0;
    pair(1207485594,1991880542,20);pair(1738382669,1975351315,32);
    void** slot=Method<0x00617540,void**>(resources,0xF56A02A0u);
    TargetBinding(bindings,TargetId(s,176),slot?*slot:nullptr);
    if(refresh && s->feature180) Method<0x006E21D0>(s,resources);
    Method<0x006E2500>(s,resources);
    if(s->feature17c) {
        const uint32_t keys[]={1584606201u,435537592u,735463696u,1860377906u};
        const uint32_t names[]={697572883u,1784620324u,uint32_t(-211326148),290389438u};
        for(unsigned i=0;i<4;++i)target(512,512,keys[i],names[i],*Global<uint32_t>(i==3?0x01158FBC:0x0115B0C4));
        for(unsigned i=0;i<4;++i)pair(names[i],keys[i],60+i*8);
    }
    pair(uint32_t(-816017598),1950398374u,180);
    static const char* stock[]={"Renderer/OpaqueWhiteTexture","Renderer/OpaqueGrey2DTexture","Renderer/TransparentWhiteTexture","Renderer/OpaqueBlackTexture","Renderer/TransparentBlackTexture","Renderer/FlatNormalMapTexture"};
    for(unsigned i=0;i<6;++i) {
        if(!s->textures[i]) {
            void* p=AllocateTarget(52,stock[i]);
            s->textures[i]=p?Method<0x00619BC0,void*>(p):nullptr;
            // The original dereferences null here if allocation/construction failed.
            EngineRef(s->textures[i],1);
            Method<0x00619C80,uint8_t>(s->textures[i],4u,4u,1u,i==5?35u:25u,2u,"RenderSystem");
        }
    }
    for(unsigned i=0;i<6;++i) Method<0x006191D0>(s->textures[i],0u,Global<void>(0x00FF7C10-i*32),i==5?4u:8u,0u,0u);
    auto pick=[&](bool texture,uint32_t fmt,uint32_t flags,uint32_t key) {
        void* p=AllocateTarget(texture?52:40,texture?"Renderer/RenderSystem/RenderPickColorTexture":"Renderer/RenderSystem/RenderPickColorTarget");
        if(!p)return static_cast<void*>(nullptr);
        void* object=texture?Method<0x00619BC0,void*>(p):Method<0x00618060,void*>(p);
        if(object) {
            EngineRef(object,1);
            uint8_t ok=texture?Method<0x00619C80,uint8_t>(object,16u,16u,1u,fmt,flags,"RenderSystem"):
                Method<0x00617BB0,uint8_t>(object,16u,16u,fmt,flags);
            if(ok) {
                if(texture)Method<0x00617650>(resources,key,object);
                else Method<0x00617580>(resources,key,object);
            }
        }
        return object;
    };
    void* color=pick(false,61,1,0x6EE45453);
    void* texture=pick(true,61,256,0x6EE45453);
    void* depth=pick(false,141,17,0x90361C05);
    alignas(16) float configured[4];
    if(Function<0x0058AC30,uint8_t>(*Global<void*>(0x0114FAB0),Global<void>(0x00FCC954),configured)) {
        uint32_t x=TruncateLow(configured[0]),y=TruncateLow(configured[1]);
        if(x && y) {
            void* p=AllocateTarget(40,"Renderer/RenderSystem/RenderPickColorTexture");
            if(p) {
                void* object=Method<0x00618060,void*>(p);
                if(object) {
                    EngineRef(object,1);
                    if(Method<0x006E3D60,uint8_t>(s,x,y,object)) Method<0x00617580>(resources,0x5A6AF52Cu,object);
                    EngineRef(object,2);
                }
            }
        }
    }
    uintptr_t result=Method<0x006E1F90,uintptr_t>(s,resources);
    if(depth)result=EngineRef(depth,2);
    if(texture)result=EngineRef(texture,2);
    if(color)result=EngineRef(color,2);
    return result;
}
#if defined(_M_IX86)
uintptr_t __fastcall HookInitializeTargets(TargetSystem* s,void*,uint8_t refresh) {return InitializeTargets(s,refresh);}
#endif

}
