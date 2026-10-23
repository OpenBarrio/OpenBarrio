#include "ts3/ui/m08_B09.h"
#include "ts3/misc/cls_50f200_x.h"
#include <cmath>
#include <xmmintrin.h>
#include <stdexcept>
#include <cstring>
namespace ts3::ui::b09 {
namespace {
ValueType* LookupType(uint16_t kind) {
#if defined(_M_IX86)
    return reinterpret_cast<ValueType**>(uintptr_t(0x0114e4f8))[kind&0xfff];
#else
    (void)kind;throw std::runtime_error("m08_B09: original property type table unavailable on native host");
#endif
}
}
uint8_t __fastcall ReadProperty(Property* p,void*,PropertyValue* out,PropertySource* source,void* allocator) {
    if(p->methods && p->methods->direct) return p->methods->direct(out,source,p,allocator);
    uint8_t result=1;
    if(p->methods && p->methods->prepare) {
        result=p->methods->prepare(out,source,p,allocator);
        if(!result) return result;
    } else {
        out->count=p->count;out->type=LookupType(uint16_t(p->kind));out->data=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(source->data)+intptr_t(p->offset));
    }
    if((uint16_t(p->kind)&0xfff)==18) {
        auto in=static_cast<const uint8_t*>(out->data);
        auto buffer=ts3::CallGameVirtual<uint32_t*,0>(allocator,out->count*4u,4u);
        if(!buffer) return 0;
        out->data=buffer;
        for(uint32_t i=0;i<out->count;++i) { std::memcpy(buffer+i,in,4);in+=16; }
    }
    return result;
}
uint8_t __fastcall PrepareProperty(Property* p,void*,PropertyValue* out,PropertySource* source,uint32_t count,void* allocator,uint8_t force) {
    if(count!=p->count && p->kind>=0) return 0;
    if(p->methods && p->methods->prepare) {
        if(!p->methods->prepare(out,source,p,allocator)) return 0;
    } else out->type=LookupType(uint16_t(p->kind));
    out->count=count;
    if(!count) {out->data=nullptr;return 1;}
    uint16_t kind=uint16_t(p->kind)&0xfff;
    if(force || kind==19 || kind==18 || (p->methods && p->methods->allocate)) {
        uint32_t bytes=count*out->type->size;
        out->data=ts3::CallGameVirtual<void*,0>(allocator,bytes,4u);
        if(!out->data) return 0;
        if(kind==19 || kind==18 || kind==20) std::memset(out->data,0,bytes);
        return out->data!=nullptr;
    }
    out->data=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(source->data)+intptr_t(p->offset));
    return out->data!=nullptr;
}
uintptr_t __fastcall Traverse(TraversalRenderer* self,void*,DrawTreeNode* list,float* bounds) {
    self->list=list;
    float sx=2.0f/(bounds[2]-bounds[0]),sy=2.0f/(bounds[3]-bounds[1]);
    for(unsigned i=0;i<4;++i)self->bounds[i]=bounds[i];
    float tx=-1.0f-((bounds[0]+0.5f)*sx),ty=((bounds[1]+0.5f)*sy)+1.0f;
    self->translation[0]=tx;self->translation[1]=ty;self->translation[2]=0.5f;self->translation[3]=tx;
    self->scale[0]=sx;self->scale[1]=-0.0f-sy;self->scale[2]=0.0f;self->scale[3]=sx;
    alignas(16) uint8_t transform[80];
    ts3::CallGameMethod<void,0x005125E0>(transform,self->translation,self->scale);
    self->transform=transform;
    uintptr_t result=reinterpret_cast<uintptr_t>(transform);
    if(list->previous==list)return result;
    auto last=self->list;auto first=last->next;
    DrawTreeNode* iterator=nullptr;
    while(last!=first) {
        auto previous=last->previous;
        if(previous->effect && ts3::CallGameVirtual<uint8_t,4>(previous->effect)) {
            ts3::CallGameMethod<void,0x0050E880>(self,&iterator,previous,1);
        }
        last=last->previous;
    }
    auto node=list->next;auto end=self->list;
    self->saved=nullptr;uint16_t level=node->level;bool initial=true;
    result=reinterpret_cast<uintptr_t>(node);
    alignas(16) uint8_t geometry[80];
    while(node!=end && (initial || node->level>level)) {
        initial=false;
        // The selected record stride is exactly 96 bytes; no guessed widget tail.
        auto source=reinterpret_cast<DrawGeometry*>(reinterpret_cast<uint8_t*>(node->geometry)+96u*node->selected);
        self->geometry=source;
        if(self->transform) {
            std::memcpy(geometry,source,68);
            ts3::CallGameMethod<void,0x00511C10>(geometry,self->transform);
            self->geometry=geometry;
        }
        result=self->tint;self->color=source->color;
        if(self->tint!=0xffffffffu) {
            result=ts3::CallGameFunction<uint32_t,0x00513560>(source->color,self->tint);
            self->color=uint32_t(result);
        }
        if(node->effect) {
            if(ts3::CallGameVirtual<uint8_t,5>(node->effect) && self->saved) {
                ts3::CallGameVirtual<uintptr_t,6>(self->saved);self->saved=nullptr;
            }
            result=ts3::CallGameMethod<uintptr_t,0x0050E880>(self,&iterator,node,0);
            node=*reinterpret_cast<DrawTreeNode**>(result);
        } else {
            if(self->color>=0x01000000u) {
                auto child=node->children.next;
                auto head=reinterpret_cast<RenderNode*>(&node->children);
                result=reinterpret_cast<uintptr_t>(head);
                while(child!=head) {
                    if(self->saved!=child->controller) {
                        if(self->saved)ts3::CallGameVirtual<uintptr_t,6>(self->saved);
                        self->saved=child->controller;
                        if(self->saved)ts3::CallGameVirtual<uintptr_t,4>(self->saved);
                    }
                    if(self->saved)ts3::CallGameVirtual<uintptr_t,5>(self->saved,self,child->renderable);
                    else ts3::CallGameVirtual<uintptr_t,3>(child->renderable,self);
                    child=child->next;
                    result=reinterpret_cast<uintptr_t>(&node->children);
                }
            }
            node=node->next;
        }
    }
    if(self->saved) {
        result=ts3::CallGameVirtual<uintptr_t,6>(self->saved);self->saved=nullptr;
    }
    return result;
}
uintptr_t __fastcall Flush(BufferedRenderer* self,void*) {
    void* pending=ts3::CallGameMethod<void*,0x00505B80>(&self->buffer);
    uintptr_t result=reinterpret_cast<uintptr_t>(pending);
    if(pending) {
        if(self->direct) {
            ts3::CallGameVirtual<uintptr_t,0>(pending);
            if(self->active!=self->saved) {
                if(self->saved) ts3::CallGameVirtual<uintptr_t,6>(self->saved);
                ts3::CallGameVirtual<uintptr_t,4>(self->active);
            }
            ts3::CallGameVirtual<uintptr_t,5>(self->active,self,pending);
            if(self->active!=self->saved) {
                ts3::CallGameVirtual<uintptr_t,6>(self->active);
                if(self->saved) ts3::CallGameVirtual<uintptr_t,4>(self->saved);
            }
            result=ts3::CallGameVirtual<uintptr_t,1>(pending);
        } else {
            auto node=ts3::CallGameFunction<RenderNode*,0x00509D80>();
            ts3::CallGameVirtual<uintptr_t,0>(pending);
            node->renderable=pending;node->controller=self->active;
            node->previous=self->head.previous;
            node->next=reinterpret_cast<RenderNode*>(&self->head);
            self->head.previous=node;
            node->previous->next=node;
            result=reinterpret_cast<uintptr_t>(&self->head);
        }
    }
    self->direct=0;
    return result;
}
BufferedRenderer* __fastcall InitRenderer(BufferedRenderer* self,void*,void* owner) {
    self->active=nullptr;self->fallback=nullptr;self->zero4c=0;self->saved=nullptr;self->index=-1;
    self->head.next=self->head.previous=reinterpret_cast<RenderNode*>(&self->head);
    ts3::CallGameMethod<void,0x005064F0>(&self->buffer,owner);
    return self;
}
RenderBuffer* __fastcall SelectRenderer(BufferedRenderer* self,void*,void* controller,uint8_t direct) {
    if(!controller) controller=self->fallback;
    if(controller!=self->active) { Flush(self,nullptr);self->active=controller; }
    self->direct=direct;
    return &self->buffer;
}
uint32_t __cdecl BlendColor(uint32_t a,uint32_t b,float t) {
    uint32_t result=0;
    for(unsigned shift=0;shift<32;shift+=8) {
        int x=int((a>>shift)&255),y=int((b>>shift)&255);
        // CVTTSS2SI's indefinite integer for NaN/overflow is observable modulo 256.
        int delta=_mm_cvtt_ss2si(_mm_mul_ss(_mm_set_ss(float(y-x)),_mm_set_ss(t)));
        result|=((uint32_t(x)+uint32_t(delta))&255u)<<shift;
    }
    return result;
}
namespace {
float Wrap(float phase,float size) {
    // The original divides on x87, passes a double to ceil/floor, and rounds
    // the multiply/add to float only at the final store (not at the quotient).
    if(phase>0.0f) return float(double(phase)-std::ceil(double(phase)/double(size))*double(size));
    if(size<phase) return phase;
    return float(std::floor(-(double(phase)/double(size)))*double(size)+double(phase));
}
void Draw(void* renderer,float* tile,float* source,TileDescriptor* desc,float* bounds,int mode) {
#if defined(_M_IX86)
    ts3::misc::ClipRect50F200(renderer,tile,source,
        static_cast<int>(reinterpret_cast<intptr_t>(desc)),bounds,mode);
#else
    (void)renderer;(void)tile;(void)source;(void)desc;(void)bounds;(void)mode;
    throw std::runtime_error("m08_B09: x86 ClipRect50F200 bridge unavailable on native host");
#endif
}
}
uintptr_t __cdecl ClippedQuad(void* renderer,float* bounds,uint32_t color,float* clip,int mode) {
    uintptr_t result=reinterpret_cast<uintptr_t>(clip);
    float uv[4]={0.0f,0.0f,1.0f,1.0f};
    // COMISS/JB considers unordered outside, unlike ordinary C++ less-than.
    bool outside=!(bounds[0]>=clip[0]) || !(clip[2]>=bounds[2]) || !(bounds[1]>=clip[1]) || !(clip[3]>=bounds[3]);
    float clipped[4];float* draw=bounds;
    if(outside) {
        float w=bounds[2]-bounds[0],h=bounds[3]-bounds[1];
        if(w<1.0f || h<1.0f) return result;
        result=ts3::CallGameMethod<uintptr_t,0x00505BD0>(clipped,bounds,clip);
        auto lahf=[&](float a,float b) {
            unsigned flags=std::isnan(a)||std::isnan(b) ? 0x47 : a==b ? 0x42 : a<b ? 0x03 : 0x02;
            result=(result&~uintptr_t(0xff00))|(uintptr_t(flags)<<8);
        };
        lahf(clipped[0],clipped[2]);if(clipped[0]==clipped[2])return result;
        lahf(clipped[1],clipped[3]);if(clipped[1]==clipped[3])return result;
        float iw=1.0f/w,ih=1.0f/h;
        uv[0]=(clipped[0]-bounds[0])*iw;uv[1]=(clipped[1]-bounds[1])*ih;
        uv[2]=(clipped[2]-bounds[0])*iw;uv[3]=(clipped[3]-bounds[1])*ih;
        draw=clipped;
    }
    if(!mode)return ts3::CallGameVirtual<uintptr_t,20>(renderer,draw,color,uv);
    return ts3::CallGameVirtual<uintptr_t,23>(renderer,draw,color,mode,uv);
}
uint32_t __cdecl BorderTrianglesBody(void* renderer,float* o,float* in,uint32_t* e,uint32_t* c) {
    auto triangle=[&](float ax,float ay,uint32_t ac,float bx,float by,uint32_t bc,float cx,float cy,uint32_t cc) {
        float a[2]={ax,ay},b[2]={bx,by},d[2]={cx,cy};
        return ts3::CallGameVirtual<uint32_t,18>(renderer,a,ac,b,bc,d,cc);
    };
    triangle(o[0],o[1],e[0],in[0],in[1],c[0],o[0],o[3],e[0]);
    triangle(in[0],in[1],c[0],in[0],in[3],c[0],o[0],o[3],e[0]);
    triangle(o[0],o[1],e[1],o[2],o[1],e[1],in[0],in[1],c[1]);
    triangle(in[0],in[1],c[1],o[2],o[1],e[1],in[2],in[1],c[1]);
    triangle(in[2],in[1],c[2],o[2],o[1],e[2],in[2],in[3],c[2]);
    triangle(o[2],o[1],e[2],o[2],o[3],e[2],in[2],in[3],c[2]);
    triangle(o[0],o[3],e[3],in[0],in[3],c[3],in[2],in[3],c[3]);
    return triangle(o[0],o[3],e[3],in[2],in[3],c[3],o[2],o[3],e[3]);
}
#if defined(_M_IX86)
__declspec(naked) uint32_t __cdecl BorderTriangles(void*,float*,float*,uint32_t*,uint32_t*) {
    // LAHF modifies only AH; a normal C++ prologue/early return would lose the
    // caller's other 24 EAX bits. No flags are consumed before the UCOMISS.
    __asm {
        mov ecx,dword ptr [esp+8]
        movss xmm0,dword ptr [ecx]
        ucomiss xmm0,dword ptr [ecx+8]
        lahf
        test ah,44h
        jnp degenerate
        movss xmm0,dword ptr [ecx+4]
        ucomiss xmm0,dword ptr [ecx+12]
        lahf
        test ah,44h
        jnp degenerate
        jmp BorderTrianglesBody
    degenerate:
        ret
    }
}
#else
uint32_t __cdecl BorderTriangles(void* r,float* o,float* in,uint32_t* e,uint32_t* c) {
    if(o[0]==o[2] || o[1]==o[3])throw std::runtime_error("m08_B09: incoming x86 EAX required for degenerate border return");
    return BorderTrianglesBody(r,o,in,e,c);
}
#endif
uint32_t __cdecl DrawBevel(void* renderer,float* bounds,uint32_t color,uint8_t flags) {
    float inner[4]={bounds[0]- -3.0f,bounds[1]- -3.0f,bounds[2]-3.0f,bounds[3]-3.0f};
    uint32_t base=color,edge[4]{};
    unsigned mode=flags&7;
    if(mode<=3) {
        uint32_t target;
        float nearAmount,farAmount;
        if(mode==1) {
            base=BlendColor(color,0xff7f7f7fu,0.2f);
            target=base|0xffffffu;nearAmount=0.2f;farAmount=0.3f;
        } else if(mode==3 || ((flags&8) && mode==0)) {
            base=BlendColor(color,color&0xff000000u,0.1f);
            target=base&0xff000000u;nearAmount=0.4f;farAmount=0.7f;
        } else if(mode==2 && (flags&8)) {
            target=base&0xff000000u;nearAmount=0.4f;farAmount=0.7f;
        } else {
            if(mode==2) base=BlendColor(color,color|0xffffffu,0.4f);
            target=base|0xffffffu;nearAmount=0.6f;farAmount=0.9f;
        }
        edge[0]=BlendColor(base,target,nearAmount);
        edge[1]=BlendColor(base,target,farAmount);
        target= mode==1 ? base&0xff000000u : target^(0xffffffu);
        nearAmount=mode==1 ? 0.2f : 0.4f;
        farAmount=mode==1 ? 0.3f : 0.7f;
        edge[2]=BlendColor(base,target,nearAmount);
        edge[3]=BlendColor(base,target,farAmount);
    }
    uint32_t center[4]={base,base,base,base};
    // 510F00 has an incoming-EAX/LAHF degenerate return contract. It remains
    // an original bridge, including the undefined edge words for flags 4..7.
    ts3::CallGameFunction<uint32_t,0x00510F00>(renderer,bounds,inner,edge,center);
    ts3::CallGameVirtual<void,1>(renderer,base);
    return ts3::CallGameVirtual<uint32_t,14>(renderer,inner[0],inner[1],inner[2],inner[3]);
}
uint32_t __cdecl DrawFeather(void* renderer,float* bounds,uint32_t color,float thickness) {
    uint32_t clear[4]={0,0,0,0},solid[4]={color,color,color,color};
    float middle[4]={bounds[0]-(thickness*0.5f),bounds[1]-(thickness*0.5f),bounds[2]+(thickness*0.5f),bounds[3]+(thickness*0.5f)};
    float outer[4]={bounds[0]-thickness,bounds[1]-thickness,bounds[2]+thickness,bounds[3]+thickness};
    ts3::CallGameFunction<uint32_t,0x00510F00>(renderer,middle,bounds,solid,clear);
    return ts3::CallGameFunction<uint32_t,0x00510F00>(renderer,outer,middle,clear,solid);
}
void __cdecl TileBoth(void* renderer,float* bounds,float* source,TileDescriptor* desc,float sx,float sy,float ox,float oy) {
    float w=((source[2]-source[0])*float(desc->width))*sx;
    float h=((source[3]-source[1])*float(desc->height))*sy;
    if(w<1.0f || h<1.0f) return;
    ox=Wrap(ox,w);oy=Wrap(oy,h);
    float left=bounds[0]+ox;
    for(float y=bounds[1]+oy;bounds[3]>y;y=y+h) {
        for(float x=left;bounds[2]>x;x=x+w) {
            float tile[4]={x,y,x+w,y+h};
            Draw(renderer,tile,source,desc,bounds,0);
        }
    }
}
void __cdecl TileHorizontal(void* renderer,float* bounds,float* source,TileDescriptor* desc,float scale,float phase,int mode) {
    float w=((source[2]-source[0])*float(desc->width))*scale;
    if(w<1.0f) return;
    phase=Wrap(phase,w);
    for(float x=bounds[0]+phase;bounds[2]>x;x=x+w) {
        float tile[4]={x,bounds[1],x+w,bounds[3]};
        Draw(renderer,tile,source,desc,bounds,mode);
    }
}
void __cdecl TileVertical(void* renderer,float* bounds,float* source,TileDescriptor* desc,float scale,float phase,int mode) {
    float h=((source[3]-source[1])*float(desc->height))*scale;
    if(h<1.0f) return;
    phase=Wrap(phase,h);
    for(float y=bounds[1]+phase;bounds[3]>y;y=y+h) {
        float tile[4]={bounds[0],y,bounds[2],y+h};
        Draw(renderer,tile,source,desc,bounds,mode);
    }
}
}
