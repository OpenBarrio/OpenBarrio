#include "ts3/render/m07_B08.h"
#include <cmath>
#include <cstring>
namespace ts3::render::m07::b08 {
namespace {
constexpr uint32_t addressModes[]={1,2,3,4,5};
constexpr uint32_t filters[]={1,2,3,1,2,3,1,2,3};
constexpr uint32_t mipFilters[]={0,0,0,1,1,1,2,2,2};
uint32_t SetSampler(Host* h,uint32_t s,uint32_t k,uint32_t v) {
    return CallGameMethod<uint32_t,0x60e900>(h,s,k,v);
}
template<class R,class... A> R Com(ComHeader* p,unsigned slot,A... a) {
    return reinterpret_cast<ComMethod<R,ComHeader,A...>>(p->vtable[slot])(p,a...);
}
template<uint32_t Address,class R,class... A> R Import(A... a) {
#if defined(_M_IX86)
    auto f=*reinterpret_cast<Stdcall<R,A...>*>(Address);
    return f(a...);
#else
    ((void)a,...); throw std::runtime_error("B08 Win32 import requires x86");
#endif
}
int32_t Add(int32_t a,int32_t b) { return int32_t(uint32_t(a)+uint32_t(b)); }
int32_t Sub(int32_t a,int32_t b) { return int32_t(uint32_t(a)-uint32_t(b)); }
int32_t RightClip(int32_t distance,float scale) {
    double rounded;
#if defined(_M_IX86)
    // EA evaluates this side in x87, then rounds once to double before floor.
    const float half=0.5f;
    __asm {
        fild distance
        fmul scale
        fadd half
        fstp rounded
    }
#else
    rounded=double(distance)*double(scale)+0.5;
#endif
    return int32_t(std::floor(rounded));
}
}
uint32_t AddressU(Host* h,uint32_t s,uint32_t v) { return SetSampler(h,s,1,addressModes[v]); }
uint32_t AddressV(Host* h,uint32_t s,uint32_t v) { return SetSampler(h,s,2,addressModes[v]); }
uint32_t AddressW(Host* h,uint32_t s,uint32_t v) { return SetSampler(h,s,3,addressModes[v]); }
uint32_t MinMipFilter(Host* h,uint32_t s,uint32_t v) {
    SetSampler(h,s,6,filters[v]); return SetSampler(h,s,7,mipFilters[v]);
}
uint32_t MagFilter(Host* h,uint32_t s,uint32_t v) { return SetSampler(h,s,5,filters[v]); }
uint32_t Invalidate(Host* h) {
    // FE77D8 is the complete 102-entry render-state identifier table.
    static constexpr uint32_t states[]={0x00000007,0x00000008,0x00000009,0x0000000E,0x0000000F,0x00000010,0x00000013,0x00000014,0x00000016,0x00000017,0x00000018,0x00000019,0x0000001A,0x0000001B,0x0000001C,0x0000001D,0x00000022,0x00000023,0x00000024,0x00000025,0x00000026,0x00000030,0x00000034,0x00000035,0x00000036,0x00000037,0x00000038,0x00000039,0x0000003A,0x0000003B,0x0000003C,0x00000080,0x00000081,0x00000082,0x00000083,0x00000084,0x00000085,0x00000086,0x00000087,0x00000088,0x00000089,0x0000008B,0x0000008C,0x0000008D,0x0000008E,0x0000008F,0x00000091,0x00000092,0x00000093,0x00000094,0x00000097,0x00000098,0x0000009A,0x0000009B,0x0000009C,0x0000009D,0x0000009E,0x0000009F,0x000000A0,0x000000A1,0x000000A2,0x000000A3,0x000000A6,0x000000A7,0x000000A8,0x000000AA,0x000000AB,0x000000AC,0x000000AD,0x000000AE,0x000000AF,0x000000B0,0x000000B2,0x000000B3,0x000000B4,0x000000B5,0x000000B6,0x000000B7,0x000000B8,0x000000B9,0x000000BA,0x000000BB,0x000000BC,0x000000BD,0x000000BE,0x000000BF,0x000000C0,0x000000C1,0x000000C2,0x000000C3,0x000000C6,0x000000C7,0x000000C8,0x000000C9,0x000000CA,0x000000CB,0x000000CC,0x000000CD,0x000000CE,0x000000CF,0x000000D0,0x000000D1};
    for(auto s:states) { uint32_t bit=s-7; h->dirty[bit>>5]|=1u<<(bit&31); }
    for(auto& d:h->savedDirty) d=0;
    for(auto& s:h->samplers) { s.dirty=0x3ffe;s.savedDirty=0; }
    for(unsigned i=0;i<203;++i) h->applied[i]=~h->desired[i];
    for(auto& s:h->samplers) for(unsigned i=0;i<13;++i) s.applied[i]=~s.desired[i];
    h->samplerDirty=0xffff;
    return CallGameMethod<uint32_t,0x60ead0>(h);
}
uintptr_t PopState(Host* h) {
    if(!h->stackIndex) return CallGameMethod<uintptr_t,0x60ed00>(h);
    uint32_t key=h->stack[--h->stackIndex];
    while(key!=0xffffffffu) {
        uint32_t value=h->stack[--h->stackIndex];
        uint32_t stage=key>>16,index=key&0xffff;
        if(stage<16) {
            h->samplers[stage].desired[index]=value;
            h->samplers[stage].dirty|=1u<<(index&31);
        } else {
            h->desired[index]=value;h->dirty[index>>5]|=1u<<(index&31);
        }
        key=h->stack[--h->stackIndex];
    }
    h->stackIndex-=7;
    for(unsigned i=0;i<7;++i) h->invalid[i]=h->stack[h->stackIndex+i];
    for(auto& s:h->samplers) s.invalid=h->stack[--h->stackIndex];
    return reinterpret_cast<uintptr_t>(h);
}
uintptr_t BindTexture(Host* h,TextureBinding* t) {
    return t->texture?CallGameMethod<uintptr_t,0x60eda0>(h,t->texture):0;
}
void* BeginDraw(Host* h,FormatStride* fmt,uint32_t primitive,uint32_t count,uint32_t vertices) {
    if(!h->vertexBuffer) return nullptr;
    uint32_t stride=*fmt->stride,bytes=stride*vertices;
    if(bytes>h->capacity) return nullptr;
    if(primitive==7 && count*2>h->indexBuffer->count/3) return nullptr;
    h->primitive=primitive;h->primitiveCount=count;h->vertexCount=vertices;
    h->stride=stride;h->byteCount=bytes;
    if(h->cursor+bytes>h->capacity) h->cursor=0;
    void* p=CallGameMethod<void*,0x619e80>(h->vertexBuffer,h->cursor,bytes,h->cursor?2u:1u);
    if(!p) return nullptr;
    CallGameMethod<uint32_t,0x60efb0>(h,fmt);
    CallGameMethod<uint32_t,0x60f020>(h,0u,h->vertexBuffer,h->cursor,h->stride);
    if(primitive==7) CallGameMethod<uint32_t,0x60f0f0>(h,h->indexBuffer);
    return p;
}
uint32_t EndDraw(Host* h) {
    CallGameMethod<uint32_t,0x616c10>(h->vertexBuffer);
    ++h->drawCount;
    uint32_t result;
    if(h->primitive==7) result=Com<uint32_t>(h->device,0x148/4,4u,0u,0u,h->vertexCount,0u,h->primitiveCount*2);
    else {
        static constexpr uint32_t primitives[]={0x00000001,0x00000002,0x00000003,0x00000004,0x00000006,0x00000005,0x00000000,0x00000000,0x00000001,0x00000002,0x00000003,0x00000004,0x00000005,0x00000006,0x00000007,0x00000008,0x00000009,0x0000000A,0x0000000B,0x0000000C,0x0000000D};
        result=Com<uint32_t>(h->device,0x144/4,primitives[h->primitive],0u,h->primitiveCount);
    }
    h->cursor+=h->byteCount;return result;
}
uint32_t SetViewport(Host* h,const Rect* rect,float nearZ,float farZ) {
    auto* v=reinterpret_cast<Viewport*>(h->privateData->opaque0000+4);
    v->x=uint32_t(rect->left);v->y=uint32_t(rect->top);
    v->width=uint32_t(rect->right)-uint32_t(rect->left);
    uint32_t height=uint32_t(rect->bottom)-uint32_t(rect->top);
    v->minZ=nearZ;
    v->height=height;
    v->maxZ=farZ;
    return Com<uint32_t>(h->device,0xbc/4,v);
}
bool ClipWindow(void* window,int32_t width,int32_t height,Rect** source,Rect** dest,Rect* sourceOut,Rect* destOut) {
    int32_t left=Import<0xf95884,int32_t>(0x4c),top=Import<0xf95884,int32_t>(0x4d);
    int32_t right=Add(left,Import<0xf95884,int32_t>(0x4e));
    int32_t bottom=Add(top,Import<0xf95884,int32_t>(0x4f));
    Rect client;
    if(!Import<0xf95914,int32_t>(window,&client)) return false;
    if(!Import<0xf95804,int32_t>(window,static_cast<void*>(nullptr),&client,2u)) return false;
    Rect* s=*source;Rect* d=*dest;
    if(s && (s->right<=s->left || s->bottom<=s->top)) return false;
    if(d && (d->right<=d->left || d->bottom<=d->top)) return false;
    if(client.right<=left || client.bottom<=top || client.left>=right || client.top>=bottom) return false;
    // EA only handles windows extending beyond the left or top of the desktop.
    if(client.left>=left && client.top>=top) return false;
    Rect dst=d?*d:Rect{0,0,width,height};
    Rect src=client;
    if(s) src={Add(s->left,client.left),Add(s->top,client.top),Add(s->right,client.left),Add(s->bottom,client.top)};
    float sx=float(Sub(dst.right,dst.left))/float(Sub(src.right,src.left));
    float sy=float(Sub(dst.bottom,dst.top))/float(Sub(src.bottom,src.top));
    if(src.left<left) { volatile float f=float(Sub(left,src.left))*sx; volatile float g=f-0.5f;dst.left=Add(dst.left,int32_t(std::ceil(g)));src.left=left; }
    if(src.top<top) { volatile float f=float(Sub(top,src.top))*sy; volatile float g=f-0.5f;dst.top=Add(dst.top,int32_t(std::ceil(g)));src.top=top; }
    if(src.right>right) { dst.right=Sub(dst.right,RightClip(Sub(src.right,right),sx));src.right=right; }
    if(src.bottom>bottom) { dst.bottom=Sub(dst.bottom,RightClip(Sub(src.bottom,bottom),sy));src.bottom=bottom; }
    if(src.left>=src.right || src.top>=src.bottom || dst.left>=dst.right || dst.top>=dst.bottom) return false;
    sourceOut->left=Sub(src.left,client.left);sourceOut->top=Sub(src.top,client.top);
    sourceOut->right=Sub(src.right,client.left);sourceOut->bottom=Sub(src.bottom,client.top);
    *source=sourceOut;
    destOut->left=dst.left;destOut->top=dst.top;
    destOut->bottom=dst.bottom;destOut->right=dst.right;
    *dest=destOut;
    return true;
}
Link* LinkResource(Host* h,ResourceLink* resource) {
    Link* head=&h->resources;Link* link=&resource->link;
    link->prev=head->prev;link->next=head;head->prev=link;link->prev->next=link;return head;
}

uint32_t __fastcall HookAddressU(Host* h,void*,uint32_t s,uint32_t v) { return AddressU(h,s,v); }

uint32_t __fastcall HookAddressV(Host* h,void*,uint32_t s,uint32_t v) { return AddressV(h,s,v); }

uint32_t __fastcall HookAddressW(Host* h,void*,uint32_t s,uint32_t v) { return AddressW(h,s,v); }

uint32_t __fastcall HookMinMipFilter(Host* h,void*,uint32_t s,uint32_t v) { return MinMipFilter(h,s,v); }

uint32_t __fastcall HookMagFilter(Host* h,void*,uint32_t s,uint32_t v) { return MagFilter(h,s,v); }

uint32_t __fastcall HookInvalidate(Host* h,void*) { return Invalidate(h); }

uintptr_t __fastcall HookPopState(Host* h,void*) { return PopState(h); }

uintptr_t __fastcall HookBindTexture(Host* h,void*,TextureBinding* t) { return BindTexture(h,t); }

void* __fastcall HookBeginDraw(Host* h,void*,FormatStride* f,uint32_t p,uint32_t c,uint32_t v) { return BeginDraw(h,f,p,c,v); }

uint32_t __fastcall HookEndDraw(Host* h,void*) { return EndDraw(h); }

uint32_t __fastcall HookSetViewport(Host* h,void*,const Rect* r,float a,float b) { return SetViewport(h,r,a,b); }

Link* __fastcall HookLinkResource(Host* h,void*,ResourceLink* r) { return LinkResource(h,r); }
}
