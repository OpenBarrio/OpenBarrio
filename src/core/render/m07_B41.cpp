#include "ts3/render/m07_B41.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::render::m07::b41 {
Resolver resolve=nullptr;
namespace {
template<class R,uint32_t A,class... T> R M(void* self,T... args) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(self,nullptr,args...);
    return CallGameMethod<R,A>(self,args...);
}
template<class R,uint32_t A,class... T> R F(T... args) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(args...);
    return CallGameFunction<R,A>(args...);
}
template<class R,int S,class... T> R V(void* self,T... args) {
    return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>((*static_cast<void***>(self))[S])(self,nullptr,args...);
}
template<class T> T& At(void* p,size_t n) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+n); }
template<class T,uint32_t A> T& G() {
    if(resolve) if(void* p=resolve(A)) return *static_cast<T*>(p);
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(A);
#else
    AbortNotRewritten("m07_B41 original global: inject a native datum");static T unused{};return unused;
#endif
}
// Translate only fields whose original offsets are proved by this batch. The
// opaque native gaps are not a wire image; pointer members remain pointer-sized.
size_t ManagerOffset(size_t wire) {
#define FIELD(off,field) case off:return offsetof(Manager,field)
    switch(wire) {
        FIELD(0,table);FIELD(8,scene);FIELD(0x10,display);FIELD(0x24,camera);
        FIELD(0x1c0,cullFlags);FIELD(0x262,frustumEnabled);FIELD(0x267,filterEnabled);FIELD(0x270,render);
        FIELD(0x28e,temporary1);FIELD(0x29c,temporary2);FIELD(0x29d,state);FIELD(0x2a2,mode);
        FIELD(0x2a4,enabled);FIELD(0x2c0,sortFlags);FIELD(0x2c4,mask);
        FIELD(0x2d4,first);FIELD(0x2d8,second);FIELD(0x2dc,names1);FIELD(0x2e0,names2);
        FIELD(0x328,mutex1);FIELD(0x344,mutex2);FIELD(0x3c4,scratch);FIELD(0x3c8,capacity);
        FIELD(0x4d4,mutex3);FIELD(0x4f0,callback);FIELD(0x4f4,hash);FIELD(0x4f8,pending);FIELD(0x504,extraStorage);
        FIELD(0x514,isolate);FIELD(0x515,debug);FIELD(0x530,extra);
        default:AbortNotRewritten("m07_B41 unproved manager offset");return 0;
    }
#undef FIELD
}
template<class T> T& SM(void* p,size_t off) {return At<T>(p,ManagerOffset(off));}
void* Sub(void* p,size_t off) {return static_cast<uint8_t*>(p)+ManagerOffset(off);}
void* Datum(uint32_t a) {
    if(resolve) if(void* p=resolve(a)) return p;
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    AbortNotRewritten("m07_B41 original datum: inject a native datum");return nullptr;
#endif
}
uintptr_t Free(void* p) {return F<uintptr_t,0x004E3F70>(p);}
void* Alloc(uint32_t n,const char* tag) {return F<void*,0x004E3F90>(n,tag,0u,0u,0u,0u);}
void* AllocAligned(uint32_t n,const char* tag) {return F<void*,0x004E3FB0>(n,4u,0u,tag,0u,0u,0u,0u);}
}
bool __fastcall ReplaceTree(Scene* s,void*,void* bounds,uint32_t size) {
    if(s->tree) {
        M<void,0x00705400>(s->tree);
        if(s->tree) {void* old=s->tree;M<void,0x007057A0>(old);Free(old);}
        s->tree=nullptr;
    }
    if(!size) return false;
    void* tree=Alloc(0xa0,"Renderer/Scene/SceneQuadtree");
    if(tree) tree=M<void*,0x00705380>(tree);
    s->tree=tree;M<void,0x007057B0>(tree,bounds,size);return true;
}
uintptr_t __cdecl ForceSimShaders(Model* s) {
    if(s->shader!=260068253u) V<void,27>(s,260068253u,0u);
    uintptr_t result=M<uintptr_t,0x0079FBE0>(s->variant);
    if(result) return result;
    result=M<uintptr_t,0x006DC110>(s->variant,0x066a6197u);
    if(result) return result;
    void* fix=AllocAligned(8,"Renderer/ForceIngameSimShadersFixupLOD");
    if(fix) {fix=M<void*,0x00692E30>(fix);if(fix) V<void,0>(fix);}
    V<void,32>(s,fix,Datum(0x00FF2668));M<void,0x006DCED0>(s->variant);
    result=M<uintptr_t,0x006FAC70>(s);
    if(fix) result=V<uintptr_t,1>(fix);
    return result;
}
void __cdecl IsolateHead(Model* s) {
    if(s->shader==0x8ed0d624u) return;
    s->state|=1;
    alignas(16) float scale[4]={1,1,1,0};M<void,0x006FD4D0>(s,scale);
    M<void,0x006F4EE0>(s,Datum(0x006E6630));
    void* variant=M<void*,0x0079FBE0>(s->variant);
    if(!variant || reinterpret_cast<uintptr_t>(variant)==40) return;
    void* base=static_cast<uint8_t*>(variant)-40;
    V<void,27>(s,0x8ed0d624u,0u);
    void* fix=AllocAligned(12,"Renderer/IsolateSimHeadFixupLOD");
    if(fix) {fix=M<void*,0x0069A130>(fix,base);if(fix) V<void,0>(fix);}
    V<void,32>(s,fix,Datum(0x00FDE654));M<void,0x006DCED0>(s->variant);M<void,0x006FAC70>(s);
    if(fix) V<void,1>(fix);
}
uintptr_t __fastcall Schedule(float* s,void*,const float* time) {
    std::memcpy(&G<uint32_t,0x0115B148>(),time,4);
    // The original stores through x87: a signaling NaN is quieted here, not in the global SSE copy.
#if defined(_M_IX86)
    __asm { mov eax,time
 fld dword ptr [eax]
 mov ecx,s
 fstp dword ptr [ecx] }
#else
    *s=*time;
#endif
    void* scheduler=F<void*,0x006E8330>();if(!scheduler) return 0;
    Vector list{nullptr,nullptr,nullptr,"Renderer/SceneLODScheduler/objlist"};
    M<void,0x006E84D0>(scheduler,&list);
    for(void** p=list.begin;p!=list.end;++p) if(*p) M<void,0x006FCC50>(*p);
    return list.begin?Free(list.begin):0;
}
uintptr_t __fastcall ForceHighLOD(Owner* s,void*,uint32_t lo,uint32_t hi,const float* center,int32_t mask,uintptr_t arg) {
    Vector list{nullptr,nullptr,nullptr,"Renderer/ForceToHiLODBufferVector"};M<void,0x006E4EA0>(s->scene,&list);
    alignas(16) float extent[4]={1000,1000,1000,1000};
    for(void** p=list.begin;p!=list.end;++p) {
        void* obj=*p;if(!obj) continue;
        if(mask!=-1 && !(M<uint32_t,0x006C93D0>(obj)&uint32_t(mask))) continue;
        uint64_t id=M<uint64_t,0x006FACB0>(obj);
        bool match=id==(uint64_t(hi)<<32|lo);
        if(!match) {
            alignas(16) uint8_t transform[64];void* matrix=M<void*,0x006FAF40>(obj,transform);
            __m128 d=_mm_sub_ps(_mm_load_ps(&At<float>(matrix,48)),_mm_load_ps(center));
            __m128 q=_mm_mul_ps(d,d);
            float distance=_mm_cvtss_f32(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(q,q,2),_mm_shuffle_ps(q,q,1)),q));
            match=4096.0f>distance;
        }
        if(match) V<void,16>(obj,0.0f,1u,extent,arg);
    }
    return list.begin?Free(list.begin):0;
}
uintptr_t __fastcall LightingDeleted(Owner* s,void*,void* solver) {
    Vector list{nullptr,nullptr,nullptr,"Renderer/NotifyLightingSolverDeleted"};M<void,0x006E4EA0>(s->scene,&list);
    for(void** p=list.begin;p!=list.end;++p) M<void,0x006FB360>(*p,solver);
    return list.begin?Free(list.begin):0;
}
bool __fastcall HasActive(void* s,void*,void* query,int32_t mask) {
    if(!SM<uint8_t>(s,0x29d) && SM<void*>(s,8)) M<void,0x006E4130>(SM<void*>(s,8));
    auto& flags=G<uint32_t,0x011D1BAC>();auto& list=G<Vector,0x011D1B9C>();
    if(!(flags&1)) {flags|=1;list={nullptr,nullptr,nullptr,"Renderer/SceneManager"};F<int,0x00F7E9A1>(Datum(0x00F92350));}
    list.end=list.begin;M<void,0x006E4EF0>(SM<void*>(s,8),&list,query);
    for(void** p=list.begin;p!=list.end;++p) if((mask==-1 || (M<uint32_t,0x006C93D0>(*p)&uint32_t(mask))) && (At<uint8_t>(*p,0x40)&0x42)) return true;
    return false;
}
bool __fastcall FindActive(Owner* s,void*,int32_t mask,uint8_t includeMoving,Object** out,uintptr_t context) {
    M<void,0x006F3C90>(s,context);
    Vector list{nullptr,nullptr,nullptr,"Renderer/SceneManager"};M<void,0x006E4EA0>(s->scene,&list);
    Object* found=nullptr;uint32_t flags=includeMoving?0x42:2;
    for(void** p=list.begin;p!=list.end;++p) {auto* obj=static_cast<Object*>(*p);if((mask==-1 || (uint32_t(mask)&obj->mask)) && (flags&obj->flags)) {found=obj;break;}}
    if(out) *out=found;
    if(list.begin) Free(list.begin);
    return found!=nullptr;
}
uintptr_t __fastcall SortEntries(void* s,void*,void* vector,uint8_t byId) {
    Entry* begin=static_cast<Entries*>(vector)->begin;Entry* end=static_cast<Entries*>(vector)->end;uint32_t count=uint32_t(end-begin);
    __m128 direction=_mm_sub_ps(_mm_setzero_ps(),_mm_load_ps(&At<float>(SM<void*>(s,0x24),0x50)));
    for(auto* p=begin;p!=end;++p) {
        if(byId) p->key=static_cast<SortObject*>(p->object)->id;
        else {
            __m128 pos=_mm_load_ps(static_cast<SortObject*>(p->object)->position);float w=_mm_cvtss_f32(_mm_shuffle_ps(pos,pos,255));
            if(w<10.0f) pos=_mm_sub_ps(pos,_mm_mul_ps(_mm_set1_ps(w),direction));
            __m128 q=_mm_mul_ps(pos,direction);
            float key=_mm_cvtss_f32(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(q,q,2),_mm_shuffle_ps(q,q,1)),q));std::memcpy(&p->key,&key,4);
        }
    }
    if(!byId) return F<uintptr_t,0x006EF480>(begin,end,uint8_t(0));
    if(count>SM<uint32_t>(s,0x3c8)) {
        if(SM<void*>(s,0x3c4)) Free(SM<void*>(s,0x3c4));
        SM<uint32_t>(s,0x3c8)=count;SM<void*>(s,0x3c4)=Alloc(count>0xffffffffu/sizeof(Entry)?0xffffffffu:uint32_t(sizeof(Entry)*count),"Renderer/SceneManager/RenderSetEntryBuffer");
    }
    return F<uintptr_t,0x006EF500>(begin,end,SM<void*>(s,0x3c4),uint8_t(0));
}
uintptr_t __fastcall DestroyManager(void* s,void*) {
    SM<void*>(s,0)=Datum(0x00FF91B8);
    if(G<void*,0x011D1868>()==s) {
        M<void,0x00723730>(s);
        const uint32_t names[]={0x00FF91F4,0x00FF9160,0x00FF9140,0x00FF9128,0x00FF910C,0x00FF90BC,0x00FF90A4,0x00FF9064,0x00FF8FFC,0x00FF8FBC,0x00FF8F5C,0x00FF8F38,0x00FF8F04,0x00FF8ECC,0x00FF8E9C,0x00FF8E70};
        // Addresses for literal strings are recovered below by their actual callsite, not inferred table extent.
        auto unregister=[](void* name) {void* manager=F<void*,0x005A00A0>();if(manager) V<void,18>(manager,name);};
        for(uint32_t name:names) unregister(Datum(name));
        for(size_t off:{size_t(0x2dc),size_t(0x2e0)}) if(void* array=SM<void*>(s,off)) {
            uint32_t n=At<uint32_t>(static_cast<uint8_t*>(array)-sizeof(void*),0);
            struct Name {void* name;uint8_t opaque[12];};
            while(int32_t(--n)>=0) unregister(static_cast<Name*>(array)[n].name);Free(static_cast<uint8_t*>(array)-sizeof(void*));
        }
        G<void*,0x011D1868>()=nullptr;
        if(void* callback=SM<void*>(s,0x4f0)) {M<void,0x00694120>(callback);Free(callback);SM<void*>(s,0x4f0)=nullptr;}
    }
    Free(SM<void*>(s,0x3c4));
    if(void* table=SM<void*>(s,0x4f4)) {
        M<void,0x0056C350>(table,static_cast<Hash*>(table)->data,static_cast<Hash*>(table)->count);static_cast<Hash*>(table)->used=0;
        if(static_cast<Hash*>(table)->count>1) Free(static_cast<Hash*>(table)->data);Free(table);
    }
    if(SM<void*>(s,0x504)) Free(SM<void*>(s,0x504));
    M<void,0x006EEAF0>(Sub(s,0x4f8));
    M<void,0x00C16FD0>(Sub(s,0x4d4));M<void,0x00C16FD0>(Sub(s,0x344));
    uintptr_t result=M<uintptr_t,0x00C16FD0>(Sub(s,0x328));
    if(SM<void*>(s,0x2d8)) result=V<uintptr_t,1>(SM<void*>(s,0x2d8));
    if(SM<void*>(s,0x2d4)) result=V<uintptr_t,1>(SM<void*>(s,0x2d4));
    SM<void*>(s,0)=Datum(0x00FB2CBC);return result;
}
uintptr_t __fastcall Cull(void* s,void*) {
    alignas(16) uint8_t filter[112],frustum[1056];
    M<void,0x0068D9E0>(frustum,SM<void*>(s,0x24));M<void,0x0068DF50>(filter);
    uint32_t mask=0xffffffffu;bool isolation=SM<uint8_t>(s,0x514)!=0,debug=SM<uint8_t>(s,0x515)!=0,render=SM<uint8_t>(s,0x270)!=0;
    if(isolation) mask=0xe3ca8205u;else if(debug&&!render) mask=0xefee8205u;
    uint32_t mode=0;if(SM<uint32_t>(s,0x2a4)&&(!debug||render)) mode=1;if(SM<uint8_t>(s,0x2a2)) mode=1;
    alignas(16) float position[4],direction[4];void* camera=SM<void*>(s,0x24);
    _mm_store_ps(position,_mm_load_ps(&At<float>(camera,0x60)));_mm_store_ps(direction,_mm_sub_ps(_mm_setzero_ps(),_mm_load_ps(&At<float>(camera,0x50))));
    M<void,0x0068DFF0>(filter,SM<uint8_t>(s,0x262)?frustum:nullptr,direction,position,SM<uint32_t>(s,0x2c0),mask&SM<uint32_t>(s,0x2c4),0u,mode);
    if(render) filter[0x64]=1;
    filter[0x66]=SM<uint8_t>(s,0x2a2);if(!SM<uint8_t>(s,0x267)) At<uint32_t>(filter,12)&=~0x10000u;
    if((debug&&!render)||isolation) {
        struct CullList {Vector objects;void* extra;uint32_t flags;uint8_t enabled,pad[3];int32_t mask;uint8_t a,b;};
        CullList list{{nullptr,nullptr,nullptr,"Renderer/SceneCullFilter/ObjectList"},SM<void*>(s,0x530),0x2080000,1,{},-129,0,0};
        if(isolation) {list.flags=0x2280000;filter[0x67]=1;}
        M<void,0x006E6570>(SM<void*>(s,8),&list,SM<void*>(s,0x10),filter,SM<uint32_t>(s,0x1c0));if(list.objects.begin) Free(list.objects.begin);
    } else M<void,0x006E6530>(SM<void*>(s,8),SM<void*>(s,0x10),filter,SM<uint32_t>(s,0x1c0));
    M<void,0x006F16B0>(s,static_cast<uint8_t*>(SM<void*>(s,0x10))+offsetof(DisplayList,first),uint8_t(1));M<void,0x006F16B0>(s,static_cast<uint8_t*>(SM<void*>(s,0x10))+offsetof(DisplayList,second),uint8_t(1));
    void* app=F<void*,0x0059EAD0>();uintptr_t result=M<uintptr_t,0x00500B80>(app);
    if(uint8_t(result)) {app=F<void*,0x0059EAD0>();result=M<uintptr_t,0x0059E7C0>(app,SM<void*>(s,0x10),SM<void*>(s,0x24));}
    return result;
}
uintptr_t __fastcall RenderOverride(void* s,void*,void* camera,uintptr_t unused,uintptr_t a4,uintptr_t a5,uintptr_t a6,uintptr_t a7,uint8_t a8,uintptr_t a9,uintptr_t a10,uint8_t a11) {
    (void)unused;(void)a7;
    M<void,0x006EB0C0>(s,1u,camera);void* device=F<void*,0x0060E2C0>();
    DisplayList displayStorage;void* display=&displayStorage;alignas(16) uint8_t filter[112],frustum[1056];
    M<void,0x0068E540>(display);M<void,0x00962940>(camera,0u);M<void,0x006E18C0>(G<void*,0x011D1530>(),camera);M<void,0x006EAC70>(s);
    M<void,0x0060EF20>(device,0u,a4);M<void,0x0060EF70>(device,a5);M<void,0x0060FFE0>(device,a6,0.0f,1.0f);M<void,0x0060F270>(device,uint32_t(a8?7:6),a9,1.0f,0u);
    struct Params {void* frustum;uint32_t flags;void* camera;void* camera2;uint32_t one;uint8_t zero,mode,option,pad;uint32_t words[7];};
    Params params{frustum,65,camera,camera,1,0,SM<uint8_t>(s,0x2a2),a11,0,{0,0,0,0,0xffffffffu,0,0}};
    M<void,0x0068D9E0>(frustum,camera);M<void,0x0068DF50>(filter);M<void,0x0068DEE0>(filter,frustum,1u,a10,2u,1u);
    M<void,0x006E4130>(SM<void*>(s,8));M<void,0x006E6530>(SM<void*>(s,8),display,filter,0u);
    if(uint8_t(M<uintptr_t,0x00611620>(device))) {
        uint8_t saved1=SM<uint8_t>(s,654),saved2=SM<uint8_t>(s,668);SM<uint8_t>(s,654)=SM<uint8_t>(s,668)=0;M<void,0x0060EC50>(device);
        auto& flags=G<uint32_t,0x011D1C24>();auto& key=G<uint32_t,0x011D1C20>();void* constants=G<void*,0x011D7530>();
        if(!(flags&1)) {flags|=1;key=F<uint32_t,0x0079A160>("WallFloorSpecScaleOverride",4u,1u);}
        if(!(flags&2)) {flags|=2;std::memset(Datum(0x011D1C10),0,16);}
        if(!(flags&4)) {flags|=4;float ones[4]={1,1,1,1};std::memcpy(Datum(0x011D1C00),ones,16);}
        auto constant=[&](void* data) {auto* c=static_cast<Constants*>(constants);if(key<c->count) {auto& r=c->records[key];++r.revision;r.serial=c->serial;r.data=data;}};
        constant(Datum(0x011D1C10));M<void,0x006E8F90>(s);M<void,0x006E9090>(s,&params);M<void,0x0060F7E0>(device,0u);M<void,0x006ED230>(s);M<void,0x006EA950>(s,display,camera);
        uint8_t changed=0;void* begin=displayStorage.first.begin;uint32_t count=uint32_t(reinterpret_cast<uint8_t*>(displayStorage.first.end)-static_cast<uint8_t*>(begin))/sizeof(Entry);
        M<void,0x006EE5E0>(s,display,begin,count,&params,&changed);constant(Datum(0x011D1C00));
        M<void,0x0060FCE0>(device);SM<uint8_t>(s,654)=saved1;SM<uint8_t>(s,668)=saved2;M<void,0x00C0FD60>(device);
    }
    return M<uintptr_t,0x0068E4A0>(display);
}
}
