#include "ts3/render/m07_B09.h"
#include <cstddef>
#include <cstring>
namespace ts3::render::m07::b09 {
void GameFree(void* p) { Original<Free>(0x004E3F70)(p); }
namespace {
void* Monitor(void* d,uint32_t adapter) {
    auto table=*static_cast<void***>(d);
    return reinterpret_cast<ComMethod<void*,void,uint32_t>>(table[15])(d,adapter);
}
template<class F> F Import(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<F*>(address);
#else
    (void)address; throw std::runtime_error("M07 display imports require native injection");
#endif
}
bool MonitorInfoCall(void* m,MonitorInfo* info) { return Import<Stdcall<int32_t,void*,MonitorInfo*>>(0x00F95800)(m,info)!=0; }
void* CreateDC(const char* driver,const char* device) { return Import<Stdcall<void*,const char*,const char*,void*,void*>>(0x00F950C0)(driver,device,nullptr,nullptr); }
int32_t DeviceCaps(void* dc,int32_t key) { return Import<Stdcall<int32_t,void*,int32_t>>(0x00F950D0)(dc,key); }
void DeleteDC(void* dc) { Import<Stdcall<int32_t,void*>>(0x00F950BC)(dc); }
}
const DisplayCalls& OriginalDisplayCalls() {
    static const DisplayCalls calls{Monitor,MonitorInfoCall,CreateDC,DeviceCaps,DeleteDC}; return calls;
}
void UpdateAspect(Host* host,const DisplayCalls& calls) {
    float result=1.0f;
    void* monitor=calls.monitor(host->d3d9,host->adapter40);
    if(monitor) {
        MonitorInfo info; info.size=0x48;
        if(calls.info(monitor,&info)) {
            void* dc=calls.create(info.device,info.device);
            if(dc) {
                int32_t pixelsX=calls.caps(dc,8),pixelsY=calls.caps(dc,10);
                int32_t sizeX=calls.caps(dc,4),sizeY=calls.caps(dc,6);
                if(sizeX && sizeY) {
                    float physical=static_cast<float>(sizeX)/static_cast<float>(sizeY);
                    float pixels=static_cast<float>(pixelsX)/static_cast<float>(pixelsY);
                    result=physical/pixels;
                }
                calls.destroy(dc);
            }
        }
    }
    host->aspect=result;
}
Host* InitializeHost(Host* h,const ts3::resource::m03::a12::Boundaries& b) {
    h->d3d9=nullptr;h->device=nullptr;h->adapter=0xFFFFFFFF;
    h->word18=0;h->word1c=0;h->word20=0;
    h->flags24[0]=1;h->flags24[1]=0;h->flags24[2]=0;h->flags24[3]=1;
    h->window=nullptr;h->width=0;h->height=0;h->word34=0;h->format=0;h->word3c=0;h->adapter40=0;h->word44=0;
    h->aspect=1.0f;h->word4c=0;h->resource50=nullptr;
    h->words54[0]=0;h->words54[1]=0x40000;h->words54[2]=0x3000;
    for(unsigned i=4;i<11;++i)h->words54[i]=0;
    h->resource80=nullptr;h->resource84=nullptr;h->resource88=nullptr;
    h->sceneActive=0;h->lost=0;h->flag8e=0;
    h->resource90=nullptr;h->resource94=nullptr;h->surface98=nullptr;h->word9c=0;h->worda0=0;
    h->fences={};
#if defined(_M_IX86)
    h->fences.tag=reinterpret_cast<const char*>(0x00FE7AF8);
#else
    h->fences.tag="Services/Device/PendingFences";
#endif
    ts3::resource::m03::a12::Initialize(&h->fences,0,b);
    h->query=nullptr;h->otherListeners={&h->otherListeners,&h->otherListeners};
    h->privateData=nullptr;h->listeners={&h->listeners,&h->listeners};
    h->callback=nullptr;h->userdata=nullptr;h->stateStack=nullptr;h->stackIndex=0;h->stackCapacity=0x400;
    h->words100[0]=0;h->words100[7]=0;h->word80c=0xFFFF;h->capsResult=0;
    std::memset(h->bindings,0,sizeof(h->bindings));
    for(auto& word:h->words7b8)word=0;
    std::memset(h->caps,0,sizeof(h->caps));std::memset(h->samplerBytes,0,sizeof(h->samplerBytes));
    for(unsigned i=1;i<7;++i)h->words100[i]=0;
    for(unsigned i=0;i<256;++i)for(unsigned ramp=0;ramp<3;++ramp)h->gamma[ramp][i]=uint16_t(i*0x101);
    return h;
}
void AddListener(Host* host,Listener* item) {
    Link* head=&host->listeners;
    Link* node=item?&item->links:nullptr;
    for(Link* p=head->next;p!=head;p=p->next) if(p==node) return;
    node->previous=head->previous;
    node->next=head;
    head->previous=node;
    node->previous->next=node;
}
void RemoveListener(Host* host,Listener* item) {
    Link* head=&host->listeners;
    Link* node=item?&item->links:nullptr;
    for(Link* p=head->next;p!=head;p=p->next) if(p==node) {
        p->previous->next=p->next;
        p->next->previous=p->previous;
        return;
    }
}
void NotifyLost(Host* host) {
    Link* head=&host->listeners;
    for(Link* p=head->next;p!=head;p=p->next) {
        auto* item=reinterpret_cast<Listener*>(reinterpret_cast<uint8_t*>(p)-offsetof(Listener,links));
        using Fn=Thiscall<void,Listener>;
        reinterpret_cast<Fn>(item->vtable[0])(item,nullptr);
    }
}
void DestroyFences(PointerDeque* q,Free free) {
    // The original walks trivial pointer elements before freeing their pages.
    auto cur=q->first.cur; auto end=q->first.end; auto page=q->first.node;
    while(cur!=q->last.cur) {
        ++cur;
        if(cur==end) { cur=*++page; end=cur+64; }
    }
    if(q->map) {
        for(auto p=q->first.node;p<q->last.node+1;++p) if(*p) free(*p);
        free(q->map);
    }
}
void DestroyHostFences(Host* host) { DestroyFences(&host->fences); }
namespace {
void EngineRelease(void*& p) {
    if(!p) return;
    auto table=*static_cast<void***>(p);
    reinterpret_cast<Thiscall<void,void>>(table[2])(p,nullptr);
    p=nullptr;
}
void ComRelease(void* p) {
    auto table=*static_cast<void***>(p);
    reinterpret_cast<ComMethod<uint32_t,void>>(table[2])(p);
}
}
namespace {
void* Manager(uint32_t flag) { return Original<void* (*)(uint32_t)>(0x0061A520)(flag); }
void Flush(void* m) { Original<Thiscall<void,void>>(0x0061A940)(m,nullptr); }
void Finish(void* m) { Original<Thiscall<void,void>>(0x0061AAB0)(m,nullptr); }
}
const ShutdownCalls& OriginalShutdownCalls() {
#if defined(_M_IX86)
    static const ShutdownCalls calls{Manager,Flush,Finish,GameFree,reinterpret_cast<void**>(0x011CECCC),reinterpret_cast<void**>(0x011CECD0)};return calls;
#else
    throw std::runtime_error("M07 shutdown globals require native injection");
#endif
}
namespace {
void TempConstruct(SurfaceTemp* t) { Original<Thiscall<void,SurfaceTemp>>(0x00617FE0)(t,nullptr); }
void Temp2D(CaptureTexture* t,uint32_t level,SurfaceTemp* out) { Original<Thiscall<void,CaptureTexture,uint32_t,SurfaceTemp*>>(0x006194B0)(t,nullptr,level,out); }
void TempCube(CaptureTexture* t,uint32_t face,uint32_t level,SurfaceTemp* out) { Original<Thiscall<void,CaptureTexture,uint32_t,uint32_t,SurfaceTemp*>>(0x006194E0)(t,nullptr,face,level,out); }
void TempDestroy(SurfaceTemp* t) { Original<Thiscall<void,SurfaceTemp>>(0x00617CB0)(t,nullptr); }
template<class R,class... A> R DeviceCall(void* object,unsigned slot,A... args) {
    auto table=*static_cast<void***>(object);
    return reinterpret_cast<ComMethod<R,void,A...>>(table[slot])(object,args...);
}
struct SurfaceDesc {uint32_t format,type,usage,pool,multisample,quality,width,height;};
int32_t WrapAdd(int32_t a,int32_t b) {return static_cast<int32_t>(uint32_t(a)+uint32_t(b));}
int32_t WrapSub(int32_t a,int32_t b) {return static_cast<int32_t>(uint32_t(a)-uint32_t(b));}
}
const CaptureCalls& OriginalCaptureCalls() {
    static const CaptureCalls calls{TempConstruct,Temp2D,TempCube,TempDestroy};return calls;
}
uint8_t CaptureTarget(Host* h,CaptureTexture* texture,int32_t x,int32_t y,uint32_t level,uint32_t face,const Rect* rect,const CaptureCalls& b) {
    SurfaceTemp temp;b.construct(&temp);
    if(texture->kind==1)b.surfaceCube(texture,face,level,&temp);else b.surface2d(texture,level,&temp);
    void* target;
    int32_t hr=DeviceCall<int32_t>(h->device,38,uint32_t(0),&target);
    if(hr<0) {b.destroy(&temp);return 0;}
    if(texture->flags&0x100) {
        SurfaceDesc desc;
        hr=DeviceCall<int32_t>(target,12,&desc);
        if(hr>=0) {
            if(x || y || (rect && (rect->left || rect->top || uint32_t(rect->right)!=desc.width || uint32_t(rect->bottom)!=desc.height)) || desc.width!=temp.width || desc.height!=temp.height) {
                // Original returns without releasing target on these validation failures.
                b.destroy(&temp);return 0;
            }
            hr=DeviceCall<int32_t>(h->device,32,target,temp.surface);
            if(uint32_t(hr)==0x88760868 && !h->lost) {h->lost=1;NotifyLost(h);}
        }
    } else {
        Rect source;
        if(rect)source=*rect;
        else {SurfaceDesc desc;DeviceCall<int32_t>(target,12,&desc);source={0,0,int32_t(desc.width),int32_t(desc.height)};}
        Rect destination{x,y,WrapAdd(WrapSub(source.right,source.left),x),WrapAdd(WrapSub(source.bottom,source.top),y)};
        hr=DeviceCall<int32_t>(h->device,34,target,&source,temp.surface,&destination,uint32_t(0));
    }
    ComRelease(target);b.destroy(&temp);return hr>=0;
}
namespace {
uint32_t Clock() { return Import<Stdcall<uint32_t>>(0x00F95120)(); }
void* Foreground() { return Import<Stdcall<void*>>(0x00F958F8)(); }
uint32_t WindowProcess(void* window) {uint32_t pid;Import<Stdcall<uint32_t,void*,uint32_t*>>(0x00F95840)(window,&pid);return pid;}
uint32_t Process() {return Import<Stdcall<uint32_t>>(0x00F95170)();}
uint8_t Recreate(Host* h) {return Original<Thiscall<uint8_t,Host>>(0x0060F430)(h,nullptr);}
void Aspect(Host* h) {UpdateAspect(h);}
void Invalidate(Host* h) {Original<Thiscall<void,Host>>(0x0060FAA0)(h,nullptr);}
uint8_t Resources(Host* h) {return Original<Thiscall<uint8_t,Host>>(0x0060F350)(h,nullptr);}
void* Service() {return Original<void* (*)()>(0x0044C470)();}
uint32_t Word(const uint8_t* bytes,size_t offset) {uint32_t value;std::memcpy(&value,bytes+offset,4);return value;}
void StoreWord(uint8_t* bytes,size_t offset,uint32_t value) {std::memcpy(bytes+offset,&value,4);}
Listener* Owner(Link* p) {return reinterpret_cast<Listener*>(reinterpret_cast<uint8_t*>(p)-offsetof(Listener,links));}
}
const ResetCalls& OriginalResetCalls() {
    static const ResetCalls calls{Clock,Foreground,WindowProcess,Process,Recreate,Aspect,Invalidate,Resources,Service};return calls;
}
uint8_t Reset(Host* h,uint8_t force,const ResetCalls& b) {
    uint32_t tick=b.clock();
    auto* state=static_cast<DevicePrivate*>(h->privateData);
    state->word1C0=tick;
    if(!force && !Word(state->resetPresent,0x20)) {
        void* window=b.foreground();if(!window)return 0;
        const uint32_t ownerPid=b.windowProcess(window);
        const uint32_t currentPid=b.process();
        if(ownerPid!=currentPid)return 0;
    }
    int32_t hr=DeviceCall<int32_t>(h->device,3);
    if(hr<0 && uint32_t(hr)!=0x88760869)return 0;
    state=static_cast<DevicePrivate*>(h->privateData);
    state->word1C4=tick;
    if(!h->lost) {h->lost=1;NotifyLost(h);}
    auto* head=&h->otherListeners;
    for(Link* p=head->next;p!=head;p=p->next) {
        auto* item=Owner(p);reinterpret_cast<Thiscall<void,Listener>>(item->vtable[5])(item,nullptr);
    }
    ReleaseVolatile(h);
    state=static_cast<DevicePrivate*>(h->privateData);
    if(!Word(state->resetPresent,0x20)) {
        uint16_t identity[3][256];
        for(unsigned i=0;i<256;++i) for(unsigned ramp=0;ramp<3;++ramp)identity[ramp][i]=uint16_t(i*0x101);
        DeviceCall<void>(h->device,21,uint32_t(0),uint32_t(0),static_cast<const void*>(identity));
    }
    ++h->word44;
    state=static_cast<DevicePrivate*>(h->privateData);
    if(Word(state->present,0x20))StoreWord(state->present,0x30,0);
    hr=DeviceCall<int32_t>(h->device,16,static_cast<void*>(state->present));
    state=static_cast<DevicePrivate*>(h->privateData);
    state->word1C8=uint32_t(hr);
    if(hr<0)return 0;
    for(unsigned i=0;i<0x38;i+=4)StoreWord(state->resetPresent,i,Word(state->present,i));
    if(!b.recreate(h))return 0;
    b.aspect(h);h->sceneActive=0;
    state=static_cast<DevicePrivate*>(h->privateData);
    DeviceCall<int32_t>(h->device,48,static_cast<void*>(state->opaque0000+4));
    b.invalidate(h);
    for(uint32_t i=0;i<16;++i) {
        DeviceCall<int32_t>(h->device,65,i,static_cast<void*>(nullptr));
        h->bindings[i]=nullptr;
    }
    for(unsigned i=0;i<7;++i)h->words100[i]=0;
    for(uint32_t i=0;i<2;++i)DeviceCall<int32_t>(h->device,100,i,static_cast<void*>(nullptr),uint32_t(0),uint32_t(0));
    h->words100[7]=0;
    DeviceCall<int32_t>(h->device,104,static_cast<void*>(nullptr));h->words54[0]=0;
    uint8_t success=b.resources(h)!=0;
    for(Link* p=head->next;p!=head;p=p->next) {
        auto* item=Owner(p);if(!reinterpret_cast<Thiscall<uint8_t,Listener>>(item->vtable[4])(item,nullptr))success=0;
    }
    if(success) {
        h->lost=0;head=&h->listeners;
        for(Link* p=head->next;p!=head;p=p->next) {
            auto* item=Owner(p);reinterpret_cast<Thiscall<void,Listener>>(item->vtable[1])(item,nullptr);
        }
    }
    state=static_cast<DevicePrivate*>(h->privateData);
    if(!h->flags24[0] && h->flags24[3] && !Word(state->resetPresent,0x20))DeviceCall<void>(h->device,21,uint32_t(0),uint32_t(0),static_cast<const void*>(h->gamma));
    if(h->flag8e) {
        h->flag8e=0;
        if(void* service=b.service()) {
            auto table=*static_cast<void***>(service);
            reinterpret_cast<Thiscall<void,void,uint32_t,uint32_t,uint32_t,uint32_t>>(table[7])(service,nullptr,0x76CBB23D,0,0,0);
        }
    }
    return success;
}
void Present(Host* h,const Rect* destination,const Rect* source,void* window) {
    if(h->callback && !h->lost)reinterpret_cast<void (*)(void*)>(h->callback)(h->userdata);
    if(h->sceneActive) {h->sceneActive=0;DeviceCall<int32_t>(h->device,42);}
    if(!h->lost) {
        auto* state=static_cast<DevicePrivate*>(h->privateData);
        Rect localSource,localDestination;
        if(Word(state->resetPresent,0x20)) {
            Original<void (*)(void*,uint32_t,uint32_t,const Rect**,const Rect**,Rect*,Rect*)>(0x00610040)(h->window,Word(state->resetPresent,0),Word(state->resetPresent,4),&destination,&source,&localDestination,&localSource);
        }
        int32_t hr=DeviceCall<int32_t>(h->device,17,source,destination,window,static_cast<void*>(nullptr));
        if(uint32_t(hr)==0x88760868 && !h->lost) {h->lost=1;NotifyLost(h);}
        if(hr>=0)return;
    }
    Reset(h,0);
}
void PresentDefault(Host* h) {Present(h,nullptr,nullptr,nullptr);}
void Shutdown(Host* h,const ShutdownCalls& b) {
    if(void* manager=b.manager(0)) { b.flush(manager);b.finish(manager);b.free(manager); }
    ReleaseVolatile(h,b.free);
    EngineRelease(h->resource88);EngineRelease(h->resource94);EngineRelease(h->resource90);
    if(h->resource50) {
        auto table=*static_cast<void***>(h->resource50);
        reinterpret_cast<Thiscall<void,void>>(table[1])(h->resource50,nullptr);
        h->resource50=nullptr;
    }
    if(h->device) { ComRelease(h->device);h->device=nullptr; }
    if(h->d3d9) { ComRelease(h->d3d9);h->d3d9=nullptr; }
    *b.globalOwner=nullptr;
    if(h->stateStack) {b.free(h->stateStack);h->stateStack=nullptr;}
    if(h->privateData) {b.free(h->privateData);h->privateData=nullptr;}
    std::memset(h->bindings,0,sizeof(h->bindings));
    for(auto& word:h->words100)word=0;
    if(*b.globalD3D) {ComRelease(*b.globalD3D);*b.globalD3D=nullptr;}
}
void ReleaseVolatile(Host* host,Free free) {
    EngineRelease(host->resource84);
    EngineRelease(host->resource80);
    if(host->surface98) { ComRelease(host->surface98); host->surface98=nullptr; }
    auto& q=host->fences;
    while(q.first.cur!=q.last.cur) {
        void** cursor=q.last.cur;
        if(cursor==q.last.begin) cursor=q.last.node[-1]+64;
        void* item=cursor[-1];
        if(q.last.cur!=q.last.begin) --q.last.cur;
        else {
            if(q.last.begin) free(q.last.begin);
            --q.last.node;
            q.last.begin=*q.last.node;
            q.last.end=q.last.begin+64;
            q.last.cur=q.last.end-1;
        }
        ComRelease(item);
    }
    if(host->query) { ComRelease(host->query); host->query=nullptr; }
    host->word9c=0;
}
}
