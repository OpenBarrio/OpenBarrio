#include "ts3/render/m07_B15.h"
#include <cstring>
#include <algorithm>
namespace ts3::render::m07::b15 {
Dependencies dependencies;
namespace {
template<class T> T* At(T* begin,T* end,int32_t i) {
    return i>=0 && i < end-begin ? begin+i : nullptr;
}
void CopyFloat(float& to,const float& from) {
#if defined(_M_IX86)
    const float* s=&from; float* d=&to;
    __asm { mov eax,s }
    __asm { fld dword ptr [eax] }
    __asm { mov eax,d }
    __asm { fstp dword ptr [eax] }
#else
    uint32_t b; std::memcpy(&b,&from,4);
    if((b&0x7f800000)==0x7f800000 && (b&0x007fffff)) b|=0x00400000;
    std::memcpy(&to,&b,4);
#endif
}
void Copy3(float* d,const float* s) { for(int i=0;i<3;++i) CopyFloat(d[i],s[i]); }
}
void __fastcall Bounds(Owner* s,void*,int32_t i,const float* v,float r) {
    auto* p=At(s->params,s->paramsEnd,i); if(!p) return;
    Copy3(&p->f38,v+3); Copy3(&p->f2C,v);
    p->f2C-=r; p->f30-=r; p->f34-=r;
    p->f38+=r; p->f3C+=r; p->f40+=r;
}
void __fastcall ParamEnabled(Owner* s,void*,int32_t i,uint8_t v) {
    if(auto* p=At(s->params,s->paramsEnd,i)) p->b44=v;
}
void __fastcall ParamWord(Owner* s,void*,int32_t i,uint32_t v) {
    if(auto* p=At(s->params,s->paramsEnd,i)) p->d18=v;
}
void __fastcall ParamPosition(Owner* s,void*,int32_t i,const float* v,float r) {
    if(auto* p=At(s->params,s->paramsEnd,i)) { Copy3(&p->f48,v); p->f54=r; }
}
void __fastcall TextureEnabled(Owner* s,void*,int32_t i,uint8_t v) {
    if(auto* p=At(s->textures,s->texturesEnd,i)) p->enabled=v;
}
void __fastcall TexturePosition(Owner* s,void*,int32_t i,const float* v,float r) {
    if(auto* p=At(s->textures,s->texturesEnd,i)) { Copy3(p->position,v); p->radius=r; }
}
void __fastcall Animate(Owner* s,void*,int32_t i,uint8_t a,uint8_t b,float f) {
    auto* p=At(s->textures,s->texturesEnd,i); if(!p) return;
    auto* manager=CallThis(Original(0x006879A0,dependencies.textureManager),static_cast<void*>(s));
    if(!manager || !p->resource) return;
    struct Args { float f; uint32_t a,b; uint8_t enabled; uint8_t padding[3]; } args;
    args.f=f; args.a=a; args.b=b; args.enabled=1;
    CallThis(Original(0x00687BA0,dependencies.animate),manager,p->resource,static_cast<const void*>(&args));
}
void __fastcall MatrixEnabled(Owner* s,void*,int32_t i,uint8_t v) {
    if(auto* p=At(s->matrices,s->matricesEnd,i)) p->enabled=v;
}
void __fastcall MatrixTransform(Owner* s,void*,int32_t i,const Transform* input) {
    auto* p=At(s->matrices,s->matricesEnd,i); if(!p) return;
    Transform t;
    CallThis(Original(0x0071AFB0,dependencies.copy),&t,input);
    t.scale=p->scale*t.scale; t.flags|=1; ++t.revision;
    CallThis(Original(0x0071ADE0,dependencies.normalize),&t,dependencies.normalization);
    for(int row=0;row<3;++row) for(int col=0;col<3;++col)
        p->matrix[row*4+col]=t.basis[row*3+col]*t.scale;
    for(int j=0;j<3;++j) p->matrix[12+j]=t.position[j];
    p->matrix[3]=0.0f; p->matrix[7]=0.0f; p->matrix[11]=0.0f; p->matrix[15]=1.0f;
}
void __fastcall MatrixColor(Owner* s,void*,int32_t i,const float* v,float a) {
    if(auto* p=At(s->matrices,s->matricesEnd,i)) {
        float tmp[4]; std::memcpy(tmp+2,v+2,4); std::memcpy(tmp+1,v+1,4);
        std::memcpy(tmp,v,4); std::memcpy(tmp+3,&a,4); std::memcpy(p->color,tmp,16);
    }
}
void __fastcall MatrixValue(Owner* s,void*,int32_t i,const void*,float f) {
    if(auto* p=At(s->matrices,s->matricesEnd,i)) std::memcpy(&p->value,&f,4);
}
void __fastcall Bind(Owner* s,void*,int32_t i,uint64_t key) {
    auto* p=At(s->matrices,s->matricesEnd,i); if(!p) return;
    auto** table=*static_cast<void***>(s->manager);
    if(p->handle) {
        auto fn=reinterpret_cast<Thiscall<void,void,uint64_t>>(table[4]);
        CallThis(fn,s->manager,p->key);
        s->matrices[i].handle=nullptr; s->matrices[i].key=0;
    }
    s->matrices[i].key=key;
    table=*static_cast<void***>(s->manager);
    auto fn=reinterpret_cast<Thiscall<void,void,uint64_t,void**>>(table[3]);
    CallThis(fn,s->manager,key,&s->matrices[i].handle);
}
int32_t __fastcall Next(Cursor* s,void*,int32_t request,void** output,uint32_t* stride) {
    if(s->index>=s->limit) {
        CallThis(Original(0x0071A9E0,dependencies.refresh),static_cast<void*>(reinterpret_cast<uint8_t*>(s)-0x98));
        s->index=0;
    }
    if(s->kind!=0 && s->kind!=1 && s->kind!=17) return 0;
    const int32_t size=s->kind==1?20:(s->kind==0?24:52);
    *output=s->data+static_cast<intptr_t>(s->index)*size;
    int32_t remaining=s->kind==1 ? s->limit-s->index : s->limit/4-s->index/4;
    int32_t count=std::min(remaining,request);
    s->index=static_cast<int32_t>(uint32_t(s->index)+uint32_t(count)*(s->kind==1?1u:4u));
    *stride=size;
    return count;
}
}
