#include "ts3/ui/m08_B04.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::ui::b04 {
namespace {
template<class T> T& Global(uintptr_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(address);
#else
    (void)address; NotRewritten("M08 B04 global: inject original x86 services");
#endif
}
Texture* Query(void* p) { return p ? CallGameVirtual<Texture*,3>(p,uint32_t(0x01be6ab3)) : nullptr; }
void Draw(DrawContext* self,const Rect* bounds,const Rect* uv,Texture* a,Texture* b) {
    CallGameMethod<void,0x00508570>(self,bounds,uv,a,b);
}
Rect Native(float x,float y,Texture* t) { return {x,y,x+float(t->width),y+float(t->height)}; }
const Rect unit{0,0,1,1};
struct Link { Link* next; Link* prev; };
void Lock() { CallGameMethod<void,0x004E16F0>(&Global<unsigned char>(0x011cd000),&Global<char>(0x00fba474)); }
void Unlock() { CallGameMethod<void,0x004E17B0>(&Global<unsigned char>(0x011cd000)); }
void* Allocate(uint32_t size,uintptr_t name) {
    return CallGameFunction<void*,0x004E3F90>(size,&Global<char>(name),uint32_t(0),uint32_t(0),uint32_t(0),uint32_t(0));
}
void* Pop(Link& head) { Link* p=head.next; p->next->prev=&head; head.next=p->next; return p; }
}
void DrawNative(DrawContext* self,float x,float y,void* resource) {
    if(self->flags<0x1000000u) return;
    if(auto* t=Query(resource)) { const Rect bounds=Native(x,y,t); Draw(self,&bounds,&unit,t,nullptr); }
}
void DrawNativeUv(DrawContext* self,float x,float y,void* resource,const Rect* uv) {
    if(self->flags<0x1000000u) return;
    if(auto* t=Query(resource)) { const Rect bounds=Native(x,y,t); Draw(self,&bounds,uv,t,nullptr); }
}
void DrawRect(DrawContext* self,const Rect* bounds,void* resource,const Rect* uv) {
    if(self->flags<0x1000000u) return;
    if(auto* t=Query(resource)) Draw(self,bounds,uv,t,nullptr);
}
void DrawPairNative(DrawContext* self,float x,float y,void* a,void* b) {
    if(self->flags<0x1000000u) return;
    auto* first=Query(a); auto* second=Query(b); // Query second even when first failed.
    if(first && second) { const Rect bounds=Native(x,y,first); Draw(self,&bounds,&unit,first,second); }
}
void DrawPairNativeUv(DrawContext* self,float x,float y,void* a,void* b,const Rect* uv) {
    if(self->flags<0x1000000u) return;
    auto* first=Query(a); auto* second=Query(b);
    if(first && second) { const Rect bounds=Native(x,y,first); Draw(self,&bounds,uv,first,second); }
}
void DrawPairRect(DrawContext* self,const Rect* bounds,void* a,void* b,const Rect* uv) {
    if(self->flags<0x1000000u) return;
    auto* first=Query(a); auto* second=Query(b);
    if(first && second) Draw(self,bounds,uv,first,second);
}
uint8_t Initialize(void* service) {
    auto& manager=Global<void*>(0x011cc9a8);
    if(!manager) manager=service;
    CallGameVirtual<void,2>(manager,uint32_t(1),uint32_t(0));
#if defined(_M_IX86)
    const auto previous=_InterlockedExchangeAdd(&Global<volatile long>(0x011cc9b8),1);
#else
    NotRewritten("M08 B04 interlocked original global"); const long previous=0;
#endif
    if(previous!=0) return 1;
    auto& pool=Global<void*>(0x011cc9b0);
    if(!pool) { auto* memory=Allocate(0x68,0x00fba42c); pool=memory ? CallGameMethod<void*,0x00509900>(memory) : nullptr; }
    auto* descriptor=CallGameVirtual<unsigned char*,13>(manager,uint32_t(0),uint32_t(0));
    Global<void*>(0x0114e480)=*reinterpret_cast<void**>(descriptor+0x38);
    CallGameMethod<void,0x00513630>(&Global<unsigned char>(0x011cc9c4));
    Global<void*>(0x011cc9c8)=&Global<unsigned char>(0x0114e47c);
    Global<uint32_t>(0x011cc9cc)=0;
    Global<Rect>(0x011cc9d0)=unit;
    Global<void*>(0x011cc9e0)=*reinterpret_cast<void**>(descriptor+0x4c);
    Global<void*>(0x011cc9e4)=*reinterpret_cast<void**>(descriptor+0x4c);
    Global<uint32_t>(0x011cc9e8)=5;
    CallGameFunction<void,0x00505DA0>();
    if(auto* callback=Global<void*>(0x011cc9bc)) {
        if(auto* api=CallGameVirtual<void*,3>(callback,uint32_t(0x00838293))) {
            if(!CallGameVirtual<uint8_t,7>(api)) { CallGameFunction<void,0x00507410>(); return 0; }
        }
    }
    return 1;
}
void* MakeWindow(uint32_t ignored,void* allocator) {
    (void)ignored;
    void* memory=CallGameFunction<void*,0x00504E70>(uint32_t(0x250),uint32_t(16),&Global<char>(0x00fba458),allocator);
    if(!memory) return nullptr;
    auto* window=CallGameMethod<unsigned char*,0x00515C80>(memory);
    return window ? window+sizeof(void*) : nullptr;
}
void* AllocateRenderable() {
    Lock(); auto& head=Global<Link>(0x0114e490);
    void* result=head.prev==&head ? Allocate(16,0x00fba478) : Pop(head);
    Unlock(); return result;
}
void* AllocateDisplay() {
    Lock(); auto& head=Global<Link>(0x0114e488); void* result;
    if(head.prev!=&head) result=Pop(head);
    else {
        result=Allocate(0xf0,0x00fba494);
        if(result) { auto* list=reinterpret_cast<Link*>(static_cast<unsigned char*>(result)+0x20); list->next=list; list->prev=list; }
    }
    Unlock(); return result;
}
void* QueryMap(void* self,uint32_t id) { return id==0xee3f516eu || id==0xf074e1c8u ? self : nullptr; }
uint8_t CellParity(CellMap* self,const int32_t* cell) {
    const int32_t x=cell[0],y=cell[1];
    if(x<0 || x>=self->width || y<0 || y>=self->height) return 0;
    const uint32_t index=uint32_t(y)*uint32_t(self->width)+uint32_t(x);
    const auto* found=CallGameFunction<uint32_t*,0x004E0810>(self->begin,self->end,&index);
    return uint8_t(((reinterpret_cast<uintptr_t>(found)-reinterpret_cast<uintptr_t>(self->begin))/4)&1);
}
void* QueryDraw(void* self,uint32_t id) { return id==0x01be8ca6u || id==0xee3f516eu ? self : nullptr; }
void* QueryTexture(Texture* self,uint32_t id) {
    if(id==0xee3f516eu || id==0x01be6ab3u || id==0xeec58382u) return self;
    if(id==0x02f84737u) return reinterpret_cast<unsigned char*>(self)+offsetof(Texture,width);
    return nullptr;
}
void* BeginIterator(void* self,Iterator* output) {
    output->table=&Global<void*>(0x00fba4b8); output->owner=self; output->kind=1;
    output->cursor=CallGameVirtual<void*,5>(self); return output->cursor;
}
}
