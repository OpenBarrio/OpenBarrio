#include "ts3/ui/m08_B25.h"
#include "ts3/ui/m08_abi.h"
#include <cstddef>
namespace ts3::ui::b25 {
namespace {
using namespace m08;
Dependencies bridges;
template<class R,size_t Slot,class T,class... A> R V(T* self,A... a) {
    return CallThis(reinterpret_cast<Thiscall<R,T,A...>>((*reinterpret_cast<void***>(self))[Slot]),self,a...);
}
template<class T> T* Owner(void* p,size_t offset) { return reinterpret_cast<T*>(static_cast<uint8_t*>(p)-offset); }
Combo* Choices(void* p) { return Owner<Combo>(p,offsetof(Combo,choices)); }
Combo* Popup(void* p) { return Owner<Combo>(p,offsetof(Combo,popupInterface)); }
void* Renderer(void* context) { return CallThis(Original(0x0050E850,bridges.renderer),context,0u,0u); }
uint32_t Source(Combo* c) { return c->source3C?c->source3C:c->source38; }
void Send(Combo* c,uint32_t kind,uint32_t value) {
    Message m; m.kind=kind;m.source=Source(c);m.value=value;
    V<void,70>(&c->secondary,&m); // Only the three defined message words are consumed.
}
Drawable* Secondary(void* p) { return Owner<Drawable>(p,offsetof(Drawable,secondary)); }
Drawable* Tertiary(void* p) { return Owner<Drawable>(p,offsetof(Drawable,tertiary)); }
}
Dependencies& Bridges() { return bridges; }
char16_t* __fastcall Item(void* self,void*,uint32_t index) {
    auto* c=Choices(self);
    return index<static_cast<uint32_t>(Count(self,nullptr))?c->begin[index].begin:nullptr;
}
int32_t __fastcall Count(void* self,void*) {
    auto* c=Choices(self);
    // Native elements contain full pointers; on x86 this is precisely SAR 4.
    constexpr unsigned shift=sizeof(Text)==16?4:5;
    const intptr_t span=reinterpret_cast<intptr_t>(c->end)-reinterpret_cast<intptr_t>(c->begin);
    return static_cast<int32_t>(span>>shift); // SAR, not signed division: negative partial elements round down.
}
int32_t __fastcall Hit(Combo* c,void*,float x,float y) {
    auto inside=[&](unsigned r) { auto* p=c->rect[r];return x>=p[0] && y>=p[1] && p[2]>x && p[3]>y; };
    if(inside(2) && !(c->flags&2)) return 2;
    if(inside(1)) return 1;
    return inside(0)?0:-1;
}
void __fastcall Select(void* self,void*,uint32_t index,uint8_t notify) {
    auto* c=Choices(self);
    if(index==c->selected) return;
    const uint32_t count=static_cast<uint32_t>(Count(self,nullptr));
    if(index>=count) index=count-1;
    c->selected=index;
    if(notify) {
        Send(c,0x4F5527E8,index);
        V<void,45>(c,c->selected); // Read again after reentrant dispatch.
    }
    V<void,36>(&c->secondary);
}
void __fastcall Open(void* self,void*,void* window) {
    auto* c=Popup(self);
    if(c->opened) return;
    c->popup=window;c->opened=1;
    Send(c,0x4F5527E9,1);
}
void __fastcall Close(void* self,void*,void* window,uint32_t cancelFocus) {
    auto* c=Popup(self);
    if(!c->opened) return;
    c->opened=0;
    if(window) {
        void* selected=V<void*,3>(window,0xAF1EE902u);
        if(selected) {
            if(c->observer) {
                V<void,67>(c->popup,c->observer);
                if(c->observer) { auto* old=c->observer;c->observer=nullptr;V<void,1>(old); }
            }
            if(c->flags&4) V<void,26>(c->manager,1u,c->popup,1u);
            uint32_t index; uint32_t pair[2];
            if(V<uint8_t,54>(selected,pair,&index)) {
                if(index==c->selected) V<void,45>(c,index);
                else V<void,17>(&c->choices,index,uint32_t(1));
            }
            if(!cancelFocus) {
                void* secondary=&c->secondary;
                if(V<void*,24>(c->manager,1u)!=secondary)
                    V<void,25>(c->manager,1u,secondary,0u);
            }
        }
        void* child=c->unknown2A0[2]?V<void*,4>(window):window;
        if(child && V<void*,4>(child)) {
            void* parent=V<void*,4>(child); // EA queries the parent twice.
            V<void,57>(parent,child);
        }
    }
    c->hover=c->pressed=-1;c->popup=nullptr;
    V<void,36>(&c->secondary);
    Send(c,0x4F5527E9,0);
}
uint32_t __fastcall Describe(void* self,void*,Descriptor* record) {
#if defined(_M_IX86)
    record->table=reinterpret_cast<void*>(0x00FBB80C);
#else
    static const uint32_t descriptor=0x00FBB80C;record->table=const_cast<uint32_t*>(&descriptor);
#endif
    record->owner=Secondary(self);record->count=1;
    const uint32_t value=V<uint32_t,5>(self);record->value=value;return value;
}
void* __fastcall Query12(void* self,void*,uint32_t id) {
    return CallThis(Original(0x0051B780,bridges.query),Tertiary(self),id);
}
void* __fastcall Delete4(void* self,void*,uint32_t flags) {
    return CallThis(Original(0x0051CB20,bridges.destruct),Secondary(self),flags);
}
void* __fastcall Delete12(void* self,void*,uint32_t flags) {
    return CallThis(Original(0x0051CB20,bridges.destruct),Tertiary(self),flags);
}
void* __fastcall Query4(void* self,void*,uint32_t id) {
    return CallThis(Original(0x0051B780,bridges.query),Secondary(self),id);
}
void __fastcall Layout(Combo* c,void*) {
    if(!c->dirty) return;
    c->dirty=0;V<void,29>(&c->secondary,8u,0u);
    if(!c->skin) return;
    const float border=(c->flags&1)?1.0f:0.0f;
    float sizes[3][2];
    for(uint32_t i=0;i<3;++i) {
        if(!V<uint8_t,6>(c->skin,sizes[i],0u,i)) {
            const float* r=V<const float*,14>(&c->secondary);
            sizes[i][0]=r[2]-r[0];sizes[i][1]=r[3]-r[1];
            if(i==2) {
                if(c->flags&2) sizes[i][0]=0;
                else { const float third=sizes[i][0]*0.33333334f;
                    sizes[i][0]=sizes[i][1]>third?third:sizes[i][1]; }
            }
        }
    }
    auto* text=c->rect[1];auto* arrow=c->rect[2];
    if(c->orientation==1) {
        arrow[0]=((c->bounds[2]-c->bounds[0])-sizes[2][0])-border;
        arrow[2]=(c->bounds[2]-c->bounds[0])-border;
        // FLD/FSTP of the stored left edge is intentional.
        volatile double edge=arrow[0];text[2]=static_cast<float>(edge);
        text[0]=0;
    } else {
        arrow[0]=border;arrow[2]=sizes[2][0]+border;
        text[0]=arrow[2];text[2]=c->bounds[2]-c->bounds[0];
    }
    arrow[1]=((((c->bounds[3]-c->bounds[1])-sizes[2][1])-(border*2.0f))*0.5f)+border;
    arrow[3]=arrow[1]+sizes[2][1];
    text[1]=0;text[3]=c->bounds[3]-c->bounds[1];
    c->rect[0][0]=c->rect[0][1]=0;c->rect[0][2]=sizes[0][0];c->rect[0][3]=sizes[0][1];
    V<void,36>(&c->secondary);
}
void __fastcall DrawLayer(Combo* c,void*,void* context,uint32_t part) {
    if(c->skin) {
        if(part<3) {
            uint32_t state[4]={0,part,c->color200,c->color1FC};
            if(!(V<uint32_t,10>(&c->secondary)&2)) state[0]=1;
            else if(c->pressed==static_cast<int32_t>(part)) state[0]=3;
            else if(c->hover==static_cast<int32_t>(part)) state[0]=2;
            V<void,4>(c->skin,context,c->rect[part],state);
        }
    } else if(part==0 && c->color200) {
        void* r=Renderer(context);V<void,1>(r,c->color200);
        V<void,14>(r,0.0f,0.0f,c->bounds[2]-c->bounds[0],c->bounds[3]-c->bounds[1]);
    }
}
uint8_t __fastcall Draw(Combo* c,void*,void* context) {
    V<void,43>(c,context,0u);V<void,43>(c,context,1u);
    if((c->flags&1) && (c->pressed!=-1 || c->hover!=-1)) {
        void* r=Renderer(context);V<void,1>(r,0xFF000000u);
        V<void,7>(r,0.0f,0.0f,c->bounds[2]-c->bounds[0],c->bounds[3]-c->bounds[1],1.0f);
    }
    if(c->selected<static_cast<uint32_t>(Count(&c->choices,nullptr))) {
        auto* t=&c->begin[c->selected];
        if(t->begin!=t->end) {
            void* fs=Original(0x0044DA00,bridges.fontService)(1u);
            if(fs) {
                void* font=CallThis(Original(0x0045F200,bridges.lookupFont),fs,c->fontKey,0u);
                if(font && Original(0x0044D900,bridges.otherService)(1u)) {
                    const float h=(c->flags&1)?3.0f:2.0f,v=(c->flags&1)?2.0f:1.0f;
                    float rect[4]={(c->inset[0]+c->rect[1][0])+h,(c->inset[1]+c->rect[1][1])+v,
                        c->rect[1][2]-(c->inset[2]+h),c->rect[1][3]-(c->inset[3]+v)};
                    void* r=Renderer(context);
                    V<void,1>(r,(V<uint32_t,10>(&c->secondary)&2)?c->color268:c->disabledColor);
                    // The font's only accessed word is +254; pointer-free sparse field.
                    auto* mode=reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(font)+0x254);
                    uint32_t saved=*mode;*mode=2;
                    V<void,8>(r,rect,t->begin,static_cast<uint32_t>(t->end-t->begin),font,0u,4u,1u,0u);
                    *mode=saved;
                }
            }
        }
    }
    if(!(c->flags&2)) V<void,43>(c,context,2u);
    return 1;
}
uintptr_t __fastcall DrawPart(Drawable* self,void*,void* context,const float* rect,const uint32_t* state) {
    void* r=Renderer(context);V<void,1>(r,0xFFFFFFFFu);
    const uint32_t part=state[1];
    // The original reads this field before rejecting out-of-range part numbers.
    if(part==1) {
        if(self->texture[0]) return V<uintptr_t,22>(r,rect[0],rect[1],self->texture[0]);
        return Original(0x00511250,bridges.frame)(r,rect,state[2],state[0]&15,2.0f);
    }
    if(part!=2) return static_cast<uint32_t>(part-2);
    if(self->texture[1]) {
        float start=0;
        switch(state[0]&7) {
            case 0:start=(state[0]&8)?0.75f:0.25f;break;
            case 1:break;case 2:start=0.5f;break;case 3:start=0.75f;break;
        }
        float uv[4]={start,0,start+0.25f,1};
        return V<uintptr_t,20>(r,rect,self->texture[1],uv);
    }
    const uint32_t kind=state[0]&7;
    Original(0x00511250,bridges.frame)(r,rect,state[2],state[0]&15,4.0f);
    V<void,1>(r,kind==1?0xFFC0C0C0u:0xFF000000u);
    const float dx=(rect[2]-rect[0])*0.2857143f,dy=(rect[3]-rect[1])*0.2857143f;
    const float shift=kind==3?1.0f:0.0f;
    const float top=(rect[1]+shift)+dy;
    float tip[2]={(((rect[2]-rect[0])*0.5f)+rect[0])+shift,(rect[3]-dy)+shift};
    float right[2]={(rect[2]-dx)+shift,top},left[2]={(rect[0]+shift)+dx,top};
    return V<uintptr_t,15>(r,left,right,tip,left);
}
}
