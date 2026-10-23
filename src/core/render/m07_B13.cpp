#include "ts3/render/m07_B13.h"
#include <intrin.h>
#include <cstring>
#include <algorithm>
namespace ts3::render::m07::b13 {
void* (*resolve)(uint32_t)=nullptr;
namespace {
void* Address(uint32_t a) { return resolve ? resolve(a) : Original<void*>(a); }
template<class R,class... A> R C(uint32_t a,A... args) { return reinterpret_cast<R(__cdecl*)(A...)>(Address(a))(args...); }
template<class R,class T,class... A> R M(uint32_t a,T* s,A... args) { return CallThis(reinterpret_cast<Thiscall<R,T,A...>>(Address(a)),s,args...); }
template<class R,class T,class... A> R V(unsigned slot,T* s,A... args) { return CallThis(reinterpret_cast<Thiscall<R,T,A...>>((*reinterpret_cast<void***>(s))[slot]),s,args...); }
void* Alloc(uint32_t size,uint32_t tag) { return C<void*>(0x004E3F90,size,Address(tag),0,0,0,0); }
uintptr_t Free(void* p) { return C<uintptr_t>(0x004E3F70,p); }
struct World { void** table; volatile long refs; void* a; void* b; };
struct Bytes { void* begin; void* end; void* capacity; const char* tag; };
struct Lock { void* data; uint32_t pitch; uint32_t rest[8]; };
}
uint8_t __fastcall Initialize(Service* s,void*) {
    auto* w=static_cast<World*>(Alloc(sizeof(World),0x00FEFED0));
    if(w) { w->table=static_cast<void**>(Address(0x00FEFEC0)); _InterlockedExchange(&w->refs,0); w->table=static_cast<void**>(Address(0x00FEFE5C));w->a=nullptr;w->b=nullptr; }
    auto* old=s->world;
    if(w!=old) { if(w) V<void>(2,w);s->world=w;if(old) V<void>(3,old); }
    auto* token=C<void*>(0x007692C0);
    auto result=M<uint8_t>(0x00686AA0,s->world,token);
    if(result) { *static_cast<void**>(Address(0x011CF490))=s->world;return 1; }
    return result;
}
Vector* __fastcall Construct(Vector* s,void*) {
    s->table=static_cast<void**>(Address(0x00FF39F8)); _InterlockedExchange(&s->refs,0);
    s->table=static_cast<void**>(Address(0x00FEFFFC));s->begin=nullptr;s->end=nullptr;s->capacity=nullptr;
    s->tag=static_cast<const char*>(Address(0x00FEFFD4));s->period=1.0f;return s;
}
uintptr_t __fastcall Destruct(Vector* s,void*) {
    s->table=static_cast<void**>(Address(0x00FEFFFC));
    while(s->begin!=s->end) { V<void>(2,s->end[-1]);--s->end; }
    uintptr_t result=reinterpret_cast<uintptr_t>(s->begin);if(s->begin) result=Free(s->begin);
    s->table=static_cast<void**>(Address(0x00FF39F8));return result;
}
Vector* __fastcall Delete(Vector* s,void*,uint32_t flags) { Destruct(s,nullptr);if(flags&1) Free(s);return s; }
void* __fastcall Create(void*,void*) { void* p=Alloc(sizeof(Animated),0x00FF000C);return p?M<void*>(0x00688090,p):nullptr; }
uintptr_t __stdcall Fixup(void* s,const void* args) {
    void* held=nullptr;
    void* p=C<void*>(0x004E3FB0,uint32_t(28),4,0,Address(0x00FF00B0),0,0,0,0);
    if(p) {p=M<void*>(0x006877B0,p,s,args);if(p) {V<void>(0,p);held=p;} }
    V<void>(32,s,held,Address(0x00FF0030));
    auto result=M<uintptr_t>(0x006DCED0,static_cast<uint8_t*>(s)+0x210);
    return held?V<uintptr_t>(1,held):result;
}
uintptr_t __fastcall Material(Manager* s,void*,void* key,uint32_t a,uint32_t b) {
    auto valid=M<uintptr_t>(0x004105D0,key);if(!valid) return valid;
    auto* mutex=reinterpret_cast<uint8_t*>(&s->head)+2*sizeof(void*);
    M<void>(0x004E16F0,mutex,Address(0x00FF006C));
    Node* n=s->head.next;
    while(n!=&s->head && n->key!=key) n=n->next;
    if(n==&s->head) {
        n=static_cast<Node*>(Alloc(sizeof(Node),0x00FF00D4));
        if(n) { n->rate=1.0f;n->key=nullptr;n->value=nullptr;n->a=nullptr;n->b=nullptr;n->time=0.0f;n->c=0.0f;n->flag=0; }
        M<void>(0x006879F0,s,n,key,a,b);
        n->prev=s->head.prev;n->next=&s->head;s->head.prev=n;n->prev->next=n;
    } else M<void>(0x006879F0,s,n,key,a,b);
    return M<uintptr_t>(0x004E17B0,mutex);
}
uint8_t __cdecl RegisterManager() {
    void* service=*static_cast<void**>(Address(0x011CE3F4));if(!service) return 0;
    void* p=Alloc(80,0x00FF00EC);p=p?M<void*>(0x00687C10,p):nullptr;
    if(p) V<void>(0,p);
    auto type=V<uint32_t>(10,p);
    if(!M<uint8_t>(0x0059EFE0,service,type)) M<void>(0x0059F500,service,p,1);
    V<void>(1,p);return 1;
}
uint8_t __fastcall Ensure(void* s,void*) {
    auto* texture=reinterpret_cast<TextureView*>(static_cast<uint8_t*>(s)-offsetof(TextureView,secondary));
    if(texture->handle30) return 1;
    return M<uint8_t>(0x00619540,texture,0);
}
void __fastcall Unload(void* s,void*) {
    auto* texture=reinterpret_cast<TextureView*>(static_cast<uint8_t*>(s)-offsetof(TextureView,secondary));
    if(!(texture->flags&0x102)) M<void>(0x006196B0,texture);
}
uint8_t __fastcall Load(Animated* s,void*,void* stream,uint32_t,uint32_t endian) {
    M<void>(0x00687600,&s->textures);
    struct Reader { void* a; void* b; void* input; uint32_t position,mode; uint8_t good,error; } reader;
    M<void>(0x007D91B0,&reader,stream,uint32_t(uint8_t(endian)==0));
    auto read=[&](void* out) { auto ok=C<uint32_t>(0x004DD700,reader.input,out,reader.mode);M<void>(0x007D77C0,&reader,ok); };
    auto done=[&](uint8_t value) {M<void>(0x007D7760,&reader);return value;};
    uint32_t magic,version,count;read(&magic);read(&version);
    if(magic!=0x4d494e41 || version!=1) return done(0);
    float rate;read(&rate);s->textures.period=double(rate)>0.00001?1.0f/rate:1.0f;
    read(&count);s->textures.end=s->textures.begin;
    M<void>(0x00AEF830,&s->textures.begin,count);
    for(uint32_t i=0;i<count;++i) {
        void* p=Alloc(52,0x00FF0144);p=p?M<void*>(0x00619BC0,p):nullptr;
        V<void>(1,p);
        uint32_t offset;read(&offset);
        auto position=V<uint32_t>(9,stream,0);V<void>(10,stream,offset,1);
        // 0060CE70 initializes two pointers and the borrowed string at +8.
        struct NameKey { void* begin; void* end; const char* text; } name;
        auto* namePtr=M<void*>(0x0060CE70,&name,Address(0x00FF0134));
        auto ok=C<uint8_t>(0x0060D9B0,p,stream,namePtr,0);
        V<void>(10,stream,position,0);s->textures.begin[i]=p;
        if(!ok) {M<void>(0x00687600,&s->textures);return done(0);}
    }
    _InterlockedExchange(&s->loaded,1);uint32_t bytes=72;
    for(uint32_t i=0;i<count;++i) bytes+=V<uint32_t>(2,static_cast<uint8_t*>(s->textures.begin[i])+offsetof(TextureView,secondary));
    M<void>(0x00731CC0,s,bytes);return done(1);
}
uint8_t __cdecl Convert(ImageTexture* s,const ImageData* input,uint8_t copy) {
    ImageData previous=*input,current=previous;
    Bytes temp{nullptr,nullptr,nullptr,static_cast<const char*>(Address(0x00FE94B0))};
    if(s->levels>1 && copy) {M<void>(0x00AEF830,&temp,input->height*input->pitch);std::memcpy(temp.begin,input->pixels,input->height*input->pitch);previous.pixels=current.pixels=temp.begin;}
    auto done=[&](uint8_t value) {if(temp.begin) Free(temp.begin);return value;};
    auto lock=[&](Lock* out,uint32_t i) {M<void>(0x00618DF0,s,out,i,0);};
    auto unlock=[&](Lock* out) {M<void>(0x00619160,s,out);};
    auto convert=[&](ImageData* out,const ImageData* in) {if(s->format==1 || s->format==2) C<void>(0x006152F0,out,in);else if(s->format==9 || s->format==10) C<void>(0x006154B0,out,in);else return false;return true;};
    for(uint32_t i=0;i<s->levels;++i) {
        if(i) {
            previous=current;current.width=std::max(current.width>>1,1u);current.height=std::max(current.height>>1,1u);
            if(current.width<4 || current.height<4) {
                uint8_t pixels[64],compressed[16];
                ImageData source{pixels,4,4,16,current.format};ImageData target{compressed,4,4,4,s->format};
                C<uint32_t>(0x006177B0,current.format);
                // EA ignores the helper's EAX and broadcasts the previous image's first pixel.
                C<void>(0x004FA790,pixels,*static_cast<uint32_t*>(previous.pixels),16);
                if(s->format==1 || s->format==2) target.pitch>>=1;
                if(!convert(&target,&source)) return done(0);
                for(;i<s->levels;++i) {Lock out;lock(&out,i);if(!out.data) return done(0);std::memcpy(out.data,compressed,target.pitch*target.height);unlock(&out);}
                return done(1);
            }
            if(!C<uint8_t>(0x006163B0,&current,&previous)) return done(0);
        }
        Lock out;lock(&out,i);if(!out.data) return done(0);
        ImageData target{out.data,out.rest[4],out.rest[5],out.pitch,s->format};
        bool ok=convert(&target,&current);unlock(&out);if(!ok) return done(0);
    }
    return done(1);
}
}
