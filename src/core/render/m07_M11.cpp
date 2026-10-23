#include "ts3/render/m07_M11.h"
#include <cstring>
namespace ts3::render::m07::m11 {
void* (*resolve)(uint32_t)=nullptr;
namespace {
void* A(uint32_t a) { return resolve?resolve(a):Original<void*>(a); }
template<class R,class T,class... X> R M(uint32_t a,T* p,X... x) { return reinterpret_cast<Thiscall<R,T,X...>>(A(a))(p,nullptr,x...); }
template<class R,class... X> R F(uint32_t a,X... x) { return reinterpret_cast<R(__cdecl*)(X...)>(A(a))(x...); }
template<class R,class T,class... X> R V(T* p,unsigned slot,X... x) { return reinterpret_cast<Thiscall<R,T,X...>>((*reinterpret_cast<void***>(p))[slot])(p,nullptr,x...); }
uint32_t& U(uint32_t a) { return *static_cast<uint32_t*>(A(a)); }
void* G(uint32_t a) { return *static_cast<void**>(A(a)); }
void* Offset(void* p,ptrdiff_t n) { return static_cast<uint8_t*>(p)+n; }
void* Alloc(uint32_t n,uint32_t tag,bool aligned=false) {
    if(aligned) return F<void*>(0x004E3FB0,n,4u,0u,A(tag),0u,0u,0u,0u);
    return F<void*>(0x004E3F90,n,A(tag),0u,0u,0u,0u);
}
uintptr_t Append(String* p,const wchar_t* s) { auto* e=s;while(*e) ++e;return M<uintptr_t>(0x00408B90,p,s,e); }
void Key(uint32_t* out,const wchar_t* name,uint32_t group=0) { F<void>(0x004AECE0,out,name,A(0x00B2D882),group,0u,0u); }
uint32_t Intern(uint32_t bit,uint32_t slot,uint32_t name) {
    if(!(U(0x011D20B4)&bit)) { U(0x011D20B4)|=bit;U(slot)=F<uint32_t>(0x007CFE70,static_cast<const char*>(A(name))); }
    return U(slot);
}
}
uintptr_t MakeNames(const wchar_t* source,String* diffuse,String* normal,uint32_t index) {
    Append(diffuse,source);Append(normal,source);
    wchar_t suffix[2]={wchar_t(uint16_t(index+49)),0};
    Append(diffuse,suffix);Append(normal,suffix);
    Append(normal,L"_n");return Append(diffuse,L"_d");
}
#if defined(_M_IX86)
__declspec(naked) uintptr_t __cdecl MakeNamesEntry() {
    __asm { push dword ptr [esp+8] }
    __asm { push edi }
    __asm { push dword ptr [esp+12] }
    __asm { push esi }
    __asm { call MakeNames }
    __asm { add esp,16 }
    __asm { ret }
}
#else
uintptr_t __cdecl MakeNamesEntry() { throw std::runtime_error("M11 ESI/EDI entry requires x86; call MakeNames natively"); }
#endif
int __fastcall SculptureBindings(Sculpture* s,void*,Binding* out,uint32_t* shader) {
    auto ice=Intern(1,0x011D20B0,0x00FFB1E8);
    auto opaque=Intern(2,0x011D20AC,0x00FFB1D8);
    auto multiplier=Intern(4,0x011D20A8,0x00FFB1C0);
    uint32_t kind=s->flags&127,index=s->flags>>8;bool special=(s->flags&128)!=0;
    int count=0;
    if(s->multiplier[0] || kind==3) {
        out[0].key=multiplier;
        if(kind==3) Key(out[0].resource,static_cast<const wchar_t*>(A(0x00FFB1B0)));
        else for(int i=0;i<4;++i) out[0].resource[i]=s->multiplier[i];
        out[0].type=4;count=1;
    }
    auto* empty=static_cast<wchar_t*>(A(0x011C7E54));
    auto* tag=static_cast<const char*>(A(0x00FFB194));
    String d{empty,empty,empty+1,tag},n=d;
    SurfaceParams p{};uint32_t mode=1,source=0;
    switch(kind) {
    case 1: source=0x00FFB17C;p={{},{},{1039516303,1036831949,1035489772},1077936128,1084227584};mode=8;break;
    case 2: source=0x00FFB160;p={{},{},{1039516303,1036831949,1035489772},1091567616,1082130432};mode=8;break;
    case 3: source=0x00FFB0BC;index=0;p={{},{},{1056964608,1056964608,1056964608},1112014848,0x40000000};break;
    case 4: source=0x00FFB148;
        if(special) p={{},{},{1055286886,1055286886,1055286886},1109393408,1065353216};
        else p={{},{},{1045220557,1045220557,1045220557},1101004800,1077936128};break;
    case 64: source=0x00FFB0DC;index=0;p={{},{},{1063843267,1062333317,1041194025},1120403456,1120403456};mode=0x4000;break;
    case 16: {
        auto noise=Intern(8,0x011D20A4,0x00FFB13C),normal=Intern(16,0x011D20A0,0x00FFB0A0);
        uint32_t key[4]={};Key(key,static_cast<const wchar_t*>(A(0x00FFB110)));
        out[count].key=noise;out[count].type=4;for(int i=0;i<4;++i) out[count].resource[i]=key[i];++count;
        uint32_t group=0;F<void>(0x007D7630,8u,&group);
        std::memset(key,0,sizeof key);Key(key,static_cast<const wchar_t*>(A(0x00FFB0F4)),group);
        out[count].key=normal;out[count].type=4;for(int i=0;i<4;++i) out[count].resource[i]=key[i];++count;
        *shader=ice;break;
    }
    default:break;
    }
    if(source) {
        // Keep the original helper as a dependency here; it has its own replacement hook.
#if defined(_M_IX86)
        auto fn=A(0x0070B1E0);auto* src=A(source);auto* dp=&d;auto* np=&n;
        __asm { push esi }
        __asm { push edi }
        __asm { mov esi,src }
        __asm { mov edi,np }
        __asm { push index }
        __asm { push dp }
        __asm { call fn }
        __asm { add esp,8 }
        __asm { pop edi }
        __asm { pop esi }
#else
        MakeNames(static_cast<const wchar_t*>(A(source)),&d,&n,index);
#endif
        p.diffuse=d.begin;p.normal=kind==4 && special?nullptr:n.begin;
        if(kind==1 || kind==2 || kind==4) *shader=opaque;
        M<void>(0x0070AF40,s,out,&count,&p,mode);
        if(kind==3 || kind==64) *shader=opaque;
    }
    auto freeString=[](String& str) {
        auto bytes=(reinterpret_cast<uintptr_t>(str.capacity)-reinterpret_cast<uintptr_t>(str.begin))&~uintptr_t(1);
        if(static_cast<intptr_t>(bytes)>2 && str.begin) F<void>(0x004E3F70,str.begin);
    };
    freeString(n);freeString(d);return count;
}
uint8_t __fastcall ModelFixup(FixupContext* s,void*,uint32_t lo,uint32_t hi,uint32_t kind,uint32_t remove,uint32_t* data,uint32_t count) {
    // 006FDFB0 writes an iterator; the node's resource receiver is adjusted -8.
    uint32_t key[2]={lo,hi};void* found[2];
    M<void>(0x006FDFB0,s->map,found,key);
    if(found[0]==s->sentinel || !found[0] || found[0]==reinterpret_cast<void*>(8)) return 0;
    key[0]=lo;key[1]=hi;M<void>(0x006FDFB0,s->map,found,key);
    if(found[0]==s->sentinel || !found[0] || found[0]==reinterpret_cast<void*>(8)) return 0;
    void* model=V<void*>(Offset(found[0],-8),3,0x017F1D88u);if(!model) return 0;
    void* list=Offset(model,0x210);
    const uint32_t ids[11]={0,0x09549326,0x09FF7876,0x0A8A2EB5,0x0C929E36,0x0D645077,0x0CD37D4A,0x2463AE7D,0x0D646E03,0x0D91794E,0x2463AE7D};
    auto erase=[&](uint32_t id) { M<void>(0x006DD530,list,id); };
    auto find=[&](uint32_t id) { return M<void*>(0x006DC110,list,id); };
    if(uint8_t(remove)) {
        if(kind>10 || kind==5) return 0;
        if(kind) erase(ids[kind]);
    } else {
        void* fixup=nullptr;uint32_t descriptor=0;
        if(kind==0) {
            V<void>(model,33,A(0x00FF6010));
            for(auto id:{ids[2],ids[4],ids[5],ids[3],ids[6]}) if(void* p=find(id)) M<void>(0x007249D0,p,0u);
            erase(ids[7]);erase(ids[8]);erase(ids[9]);
        } else if(kind==1) {
            if(count<6) return 0;
            uint32_t values[7];values[0]=U(0x011CDDA8);for(int i=0;i<6;++i) values[i+1]=data[i];
            void* mem=Alloc(36,0x00FFA0F8);void* p=mem?M<void*>(0x006CFF40,mem,values):nullptr;
            M<void>(0x009DC510,&fixup,p);descriptor=0x00FF6010;
        } else if(kind>=2 && kind<=6) {
            if(void* p=find(ids[kind])) M<void>(0x007249D0,p,1u);
            else {
                const uint32_t tags[]={0x00FFA0DC,0x00FF9FC8,0x00FFA0C4,0x00FFA0A8,0x00FFA090};
                const uint32_t ctors[]={0x00724A60,0x00724AD0,0x00694700,0x00725EE0,0x006873D0};
                const uint32_t descs[]={0x00FF9ED0,0x00FF9ED4,0x00FF2894,0x00FF9ED8,0x00FEFF34};
                auto i=kind-2;void* mem=Alloc(16,tags[i],true);void* created=mem?M<void*>(ctors[i],mem):nullptr;
                M<void>(0x0074B950,&fixup,created);descriptor=descs[i];
            }
        } else if(kind==7 || kind==10) {
            if(count!=9) return 0;erase(ids[7]);void* mem=Alloc(56,0x00FFA078,true);
            void* p=mem?M<void*>(0x007221E0,mem,data,uint8_t(kind==10)):nullptr;
            M<void>(0x0074B950,&fixup,p);descriptor=0x00FF9EDC;
        } else if(kind==8) {
            if(count) return 0;erase(ids[8]);void* mem=Alloc(12,0x00FFA058,true);
            void* p=mem?M<void*>(0x00693BD0,mem,data,0u):nullptr;
            M<void>(0x0074B950,&fixup,p);descriptor=0x00FF27B4;
        } else if(kind==9) {
            if(count!=2) return 0;float rate=-1.0f;
            if(void* p=find(ids[9])) if(void* q=V<void*>(p,3,ids[9])) rate=M<float>(0x006DDC70,q);
            erase(ids[9]);void* mem=Alloc(40,0x00FFA034,true);
            void* p=mem?M<void*>(0x006DDDE0,mem,data,rate):nullptr;
            M<void>(0x0074B950,&fixup,p);descriptor=0x00FF6FD8;
        } else return 0;
        if(descriptor) { V<void>(model,32,fixup,A(descriptor));if(fixup) V<void>(fixup,1); }
    }
    M<void>(0x006DCED0,list);M<void>(0x006FAC70,model);return 1;
}
uint8_t __fastcall ShadowTargets(Shadow* s,void*) {
    uint32_t size=s->requested;
    if(!size) size=F<uint32_t>(0x0058ABA0,G(0x0114FAB0),A(0x00FCC0B0),2048u);
    // The cached minimum is based on the object's requested size, NOT config size.
    if(!(U(0x011D20BC)&1)) { U(0x011D20BC)|=1;U(0x011D20B8)=s->requested?s->requested:256; }
    uint8_t allowNull=F<uint8_t>(0x0058AAD0,G(0x0114FAB0),A(0x00FFB338),1u);
    uint8_t force=F<uint8_t>(0x0058AAD0,G(0x0114FAB0),A(0x00FFB300),1u);
    void* renderer=F<void*>(0x00616FC0);
    uint32_t caps=*static_cast<uint32_t*>(Offset(G(0x011D753C),0x44));
    if(!(caps&0x600)) return 0;
    bool df=(caps&0x400)!=0;s->depthColor=!df;s->depthTexture=df;
    void* mem=Alloc(40,0x00FFB2D8);void* target=mem?M<void*>(0x00618060,mem):nullptr;if(target) V<void>(target,1);
    mem=Alloc(52,0x00FFB2AC);void* depth=mem?M<void*>(0x00619BC0,mem):nullptr;if(depth) V<void>(depth,1);
    auto release=[&]() { if(depth) V<void>(depth,2);if(target) V<void>(target,2); };
    auto supports=[&](uint32_t format,uint32_t usage,uint32_t type) { return M<uint8_t>(0x0060F5F0,G(0x011CECC8),format,usage,type); };
    auto mapped=[&](uint32_t i) { return static_cast<uint32_t*>(A(0x011CECE0))[i]; };
    auto color=[&](uint32_t n,uint32_t fmt) { return M<uint8_t>(0x00617BB0,target,n,n,fmt,1u); };
    auto texture=[&](uint32_t fmt) { return M<uint8_t>(0x00619C80,depth,size,size,1u,fmt,16u,A(0x00FFB298)); };
    uint8_t success=0;bool done=false;
    if(df) {
        uint32_t fmt=151+(supports(U(0x011CEF40),2,3)!=0);
        if(!force) { success=supports(mapped(fmt),2,3);if(!success) { release();return 0; } }
        uint8_t matching=F<uint8_t>(0x0070BE00);uint32_t colorFmt=F<uint32_t>(0x0070BD40,fmt);
        if(!matching) { success=color(1,colorFmt);if(!success) { release();return 0; } }
        if(size>=U(0x011D20B8)) {
            while(true) {
                if(matching) {
                    success=color(size,colorFmt);
                    if(success && texture(fmt)) { done=true;break; }
                    V<void>(target,4);
                } else if(texture(fmt)) { done=true;break; }
                M<void>(0x00619A90,depth);size>>=1;
                // Exhaustion jumps past the stale-BL check (0070C0CB -> C0D1).
                if(size<U(0x011D20B8)) { release();return 0; }
            }
        }
        // EA keeps BL from the format/color check, not the texture result.
        if(!done && !success) { release();return 0; }
    } else {
        uint32_t colorFmt=153;
        if(allowNull && supports(U(0x011CEF44),1,1)) {
            success=color(size,153);if(success) V<void>(target,4);
        }
        if(!success) { colorFmt=F<uint32_t>(0x0070BD40,141u);if(!colorFmt) { release();return 0; } }
        if(size>=U(0x011D20B8)) {
            while(true) {
                success=supports(mapped(colorFmt),1,1);
                if(success) { success=supports(U(0x011CEF14),2,3);if(success) { success=color(size,colorFmt);if(success) { success=texture(141);if(success) break; } } }
                V<void>(target,4);M<void>(0x00619A90,depth);size>>=1;
                if(size<U(0x011D20B8)) { release();return 0; }
            }
        }
        if(!success) { release();return 0; }
    }
    M<void>(0x00617580,renderer,s->colorSlot,target);M<void>(0x00617650,renderer,s->depthSlot,depth);
    release();s->size=size;return 1;
}
}
