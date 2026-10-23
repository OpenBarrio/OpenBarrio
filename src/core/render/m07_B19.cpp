#include "ts3/render/m07_B19.h"
#include <cstring>
#include <limits>
namespace ts3::render::m07::b19 {
namespace {
void* Alloc(uint32_t n,const char* tag) { return CallGameFunction<void*,0x004E3F90>(n,tag,0,0,0,0); }
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
uint32_t Product(uint32_t n,uint32_t stride) { uint64_t p=uint64_t(n)*stride; return p>0xffffffffu?0xffffffffu:uint32_t(p); }
void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(address));
#else
    (void)address; NotRewritten("m07_B19 image vtable");
#endif
}
}
bool __fastcall SetPrimitiveCount(Drawable* s,void*,uint32_t n) {
    if(s->primitives) { Free(s->primitives); s->primitives=nullptr; s->primitiveCount=0; }
    if(n) {
        s->primitives=Alloc(Product(n,20),"Renderer/Drawable/PrimitiveInfosForStates");
        if(!s->primitives) return false;
        s->primitiveCount=n;
    }
    return true;
}
bool __fastcall SetSkin(Drawable* s,void*,const void* source,uint32_t n) {
    Free(s->skin); s->skin=nullptr;
    Free(s->rig); s->rig=nullptr;
    Free(s->matrices); s->jointCount=0; s->matrices=nullptr;
    if(!source) return true;
    s->skin=static_cast<uint32_t*>(Alloc(Product(n,4),"Renderer/Drawable/JointMapSkin"));
    if(!s->skin) return false;
    std::memcpy(s->skin,source,uint32_t(n*4));
    s->jointCount=n;
    // EA multiplies by twelve with wrapping before the checked multiply by four.
    s->matrices=Alloc(Product(n*12,4),"Renderer/Drawable/SkinningMatrices");
    return s->matrices!=nullptr;
}
PooledMesh* __fastcall DeletePooled(PooledMesh* s,void*,uint32_t flags) {
    s->table=Table(0x00FF5FAC); Free(s->backing);
    s->table=Table(0x00FF5F1C);
    if(flags&1) Free(s);
    return s;
}
PooledMesh* __fastcall MakePooled(PoolView* s,void*,const void* vertices,uint32_t nv,const void* indices,uint32_t ni,uint32_t vb,uint32_t ib) {
    if(nv>s->vertexCount || ni>s->indexCount) return nullptr;
    auto* p=static_cast<PooledMesh*>(Alloc(sizeof(PooledMesh),"Renderer/MeshPool/PooledMesh"));
    if(!p) return nullptr;
    p->refs=0; p->table=Table(0x00FF5FAC);
#if defined(_M_IX86)
    void* allocator=*reinterpret_cast<void**>(0x011CB864);
#else
    void* allocator=nullptr;
#endif
    auto* bytes=CallGameMethod<uint8_t*,0x004E69D0>(allocator,uint32_t(vb+ib),0);
    p->backing=bytes;
    auto* split=bytes+vb;
    std::memcpy(bytes,vertices,vb); std::memcpy(split,indices,ib);
    p->vertices=bytes; p->indices=split; p->vertexCount=nv; p->indexCount=ni;
    p->slot=s->state[2]-1;
    return p;
}
// Adapted from verified m05_B23 Query; only the first interface ID differs.
void* __fastcall Query(void* s,void*,uint32_t id) {
    return id==0x0085f126u || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu ? s : nullptr;
}
uint32_t __fastcall Clear(void* s,void*) {
    CallGameMethod<void,0x006D0230>(s);
    CallGameMethod<void,0x0079FA50>(s);
    return 0;
}
bool __fastcall AllocateLods(ModelView* s,void*,uint32_t n) {
    auto* p=static_cast<LodRecord*>(Alloc(Product(n,sizeof(LodRecord)),"Renderer/Model/LODs"));
    if(p) for(uint32_t i=0;i<n;++i) std::memset(p[i].words,0,sizeof(p[i].words));
    s->lods=p;
    if(!p) return false;
    for(uint32_t i=0;i<n;++i) p[i].drawable=nullptr;
    s->lodCount=n;
    return true;
}
void* __fastcall CopyIndices(IndexBuilder* s,void*,const void* source,int32_t start,uint32_t n,PointerVector* v) {
    if(!s->perElement) return std::memcpy(s->current+start,source,uint32_t(n*4));
    void* copy=Alloc(Product(n,4),"Renderer/ModelBuilder/PerElementIndices");
    std::memcpy(copy,source,uint32_t(n*4));
    auto* old=v->end;
    if(reinterpret_cast<uintptr_t>(old)>=reinterpret_cast<uintptr_t>(v->capacity))
        return CallGameMethod<void*,0x008621C0>(v,old,&copy);
    v->end=old+1;
    if(old) *old=copy;
    return old;
}
uintptr_t __fastcall ResizeColors(ColorVector* s,void*,uint32_t n) {
    uint32_t count=uint32_t(s->end-s->begin);
    if(n<=count) return CallGameMethod<uintptr_t,0x006D5ED0>(s,s->begin+n,s->end);
    ColorRecord record{};
    record.tag="Renderer/ModelBuilder/ColorVertexArray";
    record.maximum=0x7f7fffff;
    return CallGameMethod<uintptr_t,0x006D60A0>(s,s->end,n-count,&record);
}
uint32_t __fastcall BindRig(Drawable* s,void*,void* owner) {
    auto* resolver=static_cast<uint8_t*>(owner)+40;
    CallGameMethod<void,0x00575650>(resolver);
    if(s->skeleton) {
        auto* joints=CallGameMethod<uint32_t*,0x00444710>(s->skeleton);
        Free(s->rig);
        s->rig=static_cast<uint32_t*>(Alloc(Product(s->jointCount,4),"Renderer/Drawable/JointMapRig"));
        for(uint32_t i=0;i<s->jointCount;++i) {
            uint32_t value=CallGameMethod<uint32_t,0x00575B00>(resolver,joints[s->skin[i]]);
            s->rig[i]=value==0xffffffffu?0:value;
        }
    }
    uint32_t result=s->joint;
    if(result) {
        result=CallGameMethod<uint32_t,0x00575B00>(resolver,result);
        s->mappedJoint=result==0xffffffffu?0:result;
    }
    return result;
}
Drawable* __fastcall ConstructDrawable(Drawable* s,void*) {
    s->root=nullptr;
    for(auto& p:s->cleared14) p=nullptr;
    s->primitives=nullptr; s->primitiveCount=0;
    s->skin=nullptr; s->rig=nullptr; s->jointCount=0; s->skeleton=nullptr;
    s->joint=0; s->mappedJoint=0xffffffff; s->matrices=nullptr; s->p74=nullptr; s->p78=nullptr; s->sort=100;
    s->sphere[0]=0; s->sphere[1]=0; s->sphere[2]=1; s->sphere[3]=0;
    s->flags[2]&=0xf0; s->flags[0]=0; s->flags[1]=4;
    s->tailC4=nullptr; s->tailC8=nullptr; s->p1=nullptr; s->p2=nullptr;
#if defined(_M_IX86)
    auto* ids=reinterpret_cast<uint32_t*>(0x011588B8);
    if(ids[6]==0xffffffffu) {
        auto intern=[&](const char* name,uint32_t count,uint32_t type) {
            auto* manager=*reinterpret_cast<void**>(0x011D7530);
            return CallGameMethod<uint32_t,0x0079A160>(manager,name,count,type);
        };
        ids[6]=intern("SkinMatrices",240,2); ids[5]=intern("Diffuse",1,2);
        ids[0]=intern("Opacity",1,2); ids[1]=intern("OverlayAlphaThreshold",1,1);
        ids[2]=intern("ObjectToWorldSpaceMatrix",4,2); ids[3]=intern("ObjectToClipSpaceMatrix",4,2);
        ids[4]=intern("RugSort",1,1); ids[8]=intern("VertexAnimBlendWeights",16,2);
        ids[7]=intern("morphWeights",1,2); ids[9]=intern("SecondaryNormalMapWeights",1,2);
        ids[10]=intern("HasExteriorAndInteriorLighting",1,1); ids[11]=intern("DisableSnowXRay",1,1);
    }
#else
    NotRewritten("m07_B19 shader binding globals");
#endif
    return s;
}
bool __fastcall InitializePool(PoolView* s,void*,FormatView* format,uint32_t vertices,uint32_t indices) {
    if(vertices>0xffff) vertices=0xffff;
    auto* e=format->elements;
    if(e[4]!=7 || e[6] || e[7] || e[2] || e[3]) return false;
    s->indexCount=indices; s->vertexCount=vertices; s->stride=*format->stride; s->format=format;
    CallGameVirtual<void,1>(format);
    void* vb=Alloc(sizeof(VertexBuffer),"Renderer/MeshPool/VertexBuffer");
    s->vb=vb?CallGameMethod<void*,0x0061A380>(vb):nullptr;
    if(!s->vb) return false;
    CallGameVirtual<void,1>(s->vb);
    if(!CallGameMethod<bool,0x0061A1C0>(s->vb,vertices*s->stride,1,"MeshPool")) {
        CallGameMethod<void,0x006CF3E0>(s); return false;
    }
    void* ib=Alloc(sizeof(IndexBuffer),"Renderer/MeshPool/IndexBuffer");
    s->ib=ib?CallGameMethod<void*,0x00616EE0>(ib):nullptr;
    if(!s->ib) return false;
    CallGameVirtual<void,1>(s->ib);
    if(!CallGameMethod<bool,0x00616DF0>(s->ib,indices,0,1)) {
        CallGameMethod<void,0x006CF3E0>(s); return false;
    }
#if defined(_M_IX86)
    auto* manager=*reinterpret_cast<void**>(0x011D7530);
    s->binding=CallGameMethod<uint32_t,0x0079A160>(manager,"jointMatrix",180,2);
#else
    NotRewritten("m07_B19 binding manager global");
#endif
    for(auto& v:s->state) v=0;
    return true;
}
bool __fastcall Ready(ModelView* s,void*) {
    if(s->token) {
        auto* manager=CallGameFunction<void*,0x0072A510>();
        if(CallGameMethod<bool,0x0072A580>(manager,s->token)) return true;
    }
    if(!s->lods) return true;
    auto* manager=CallGameFunction<void*,0x0072A510>();
    if(manager) {
        CallGameMethod<uintptr_t,0x0072A680>(manager,Table(0x006D0220),s,0);
    } else CallGameVirtual<uintptr_t,6>(s);
    return false;
}
bool __fastcall Clone(ModelView* s,void*,ModelView** out,uint32_t flags) {
    auto* memory=Alloc(sizeof(ModelView),"Renderer/ModelClone");
    if(!memory) return false;
    auto* clone=CallGameMethod<ModelView*,0x006D0570>(memory);
    if(!clone) return false;
    CallGameVirtual<void,0>(clone);
    bool ok=AllocateLods(clone,nullptr,s->lodCount);
    if(ok) {
        clone->flag=s->flag;
        uint32_t i=0;
        for(;i<s->lodCount;++i) {
            auto& src=s->lods[i]; auto& dst=clone->lods[i];
            if(src.drawable && !CallGameMethod<bool,0x006DB6E0>(src.drawable,&dst,0,flags)) {
                ok=false; CallGameMethod<void,0x006D0230>(clone); break;
            }
            for(unsigned j=0;j<8;++j) dst.words[j]=src.words[j];
        }
        if(i==s->lodCount) { *out=clone; CallGameVirtual<void,0>(clone); }
    }
    CallGameVirtual<void,1>(clone);
    return ok;
}
Builder* __fastcall ConstructBuilder(Builder* s,void*,uint8_t mode) {
    s->table=Table(0x00FF640C); s->flags10[0]=s->flags10[1]=s->flags10[2]=0;
    auto init=[](Sequence& v,const char* name) { v.begin=v.end=v.capacity=nullptr; v.tag=name; };
    init(s->vb,"Renderer/ModelBuilder/VertexBuffer"); init(s->ib,"Renderer/ModelBuilder/IndexBuffer");
    init(s->meshes,"Renderer/ModelBuilder/MeshesUsingCurrVertexFormat");
    init(s->positionIndices,"Renderer/ModelBuilder/PositionIndexArray");
    init(s->normalIndices,"Renderer/ModelBuilder/NormalIndexArray");
    init(s->tangentIndices,"Renderer/ModelBuilder/TangentIndexArray");
    for(auto& v:s->colorIndices) init(v,"Renderer/ModelBuilder/ColorIndexArray");
    for(auto& v:s->uvIndices) init(v,"Renderer/ModelBuilder/UVIndexArray");
    init(s->vertices,"Renderer/ModelBuilder/VertexArray"); init(s->current,"Renderer/ModelBuilder/CurrVertex");
    init(s->elements,"Renderer/ModelBuilder/mVertexElements");
    init(s->positions,"Renderer/ModelBuilder/PositionVertexArray"); init(s->normals,"Renderer/ModelBuilder/NormalVertexArray");
    init(s->tangents,"Renderer/ModelBuilder/TangentVertexArray");
    for(auto& v:s->colors) init(v,"Renderer/ModelBuilder/ColorVertexArray");
    // The UV vectors deliberately use the ColorVertexArray allocation tag too.
    for(auto& v:s->uvs) init(v,"Renderer/ModelBuilder/ColorVertexArray");
    init(s->lods,"Renderer/ModelBuilder/LODInfos"); s->opaque264=0;
    init(s->ranges,"Renderer/ModelBuilder/MeshDrawableRanges"); init(s->materials,"Renderer/ModelBuilder/MaterialInfos");
    init(s->formats,"Renderer/ModelBuilder/VertexFormatInfos");
    s->flags2C0[0]=0; s->flags2C0[1]=1; s->flags2C0[2]=mode;
    init(s->normalArray,"Renderer/ModelBuilder/NormalArray");
    return s;
}

}
