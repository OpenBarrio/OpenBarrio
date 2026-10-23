#include "ts3/render/m07_B23.h"
#include <initializer_list>
namespace ts3::render::b23 {
namespace {
template<unsigned A> void* Data() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    NotRewritten("m07_B23 original global");
#endif
}
void* Allocate(uint32_t size,unsigned tag) {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x004E3F90>(size,reinterpret_cast<const char*>(uintptr_t(tag)),0,0,0,0);
#else
    (void)size;(void)tag; NotRewritten("m07_B23 allocator");
#endif
}
uint32_t ArraySize(uint32_t n) { return n>0x3fffffffu ? 0xffffffffu : n*4; }
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
}
uintptr_t __fastcall SubstituteMaterials(LOD* self,void*,Drawables* source,SubstitutionArgs* args,void* context) {
    MaterialTree tree;
    tree.head.next=tree.head.prev=&tree.head;tree.root=nullptr;tree.flag=0;tree.count=0;
    tree.tag=static_cast<const char*>(Data<0x00FF6ECC>());
    const auto count=self->count;
    for(uint32_t i=0;i<count;++i) {
        void* original=source->items[i];void* drawable=self->items[i];
        if(!original || !drawable) continue;
        if(!CallGameMethod<void*,0x00444710>(original)) CallGameMethod<void,0x00691030>(original,args->fallback,context);
        if(!CallGameMethod<void*,0x00444710>(drawable)) CallGameMethod<void,0x00691030>(drawable,args->fallback,context);
        auto* material=CallGameMethod<Surface*,0x00444710>(drawable);
        void* output=nullptr;
        if(!material) continue;
        if(args->skipId) {
            auto* identity=CallGameMethod<ShaderIdentity*,0x006C93D0>(material);
            if(identity && identity->id==args->skipId) continue;
        }
        void* key=CallGameMethod<void*,0x006C9310>(material);
        CachedMaterialNode* found=nullptr;
        if(key) CallGameMethod<void,0x0061BA60>(&tree,&found,&key);
        if(key && found!=reinterpret_cast<CachedMaterialNode*>(&tree.head)) {
            CallGameMethod<void,0x005AF160>(&output,&found->material);
        } else {
            void* shader=nullptr;
            CallGameMethod<void,0x006CB1E0>(material->shader,args->overrides,args->count,uint32_t(args->modeB),uint32_t(args->modeA),&shader);
            auto* shaderSurface=CallGameMethod<void*,0x00444710>(shader);
            RemapNode* mapping=nullptr;
            CallGameMethod<void,0x00D38B00>(&args->remap,&mapping,&shaderSurface);
            if(mapping!=args->remap.buckets[args->remap.bucketCount]) CallGameMethod<void,0x00F20390>(shader,mapping->second);
            CallGameMethod<void,0x006CAC30>(shader,&output,int32_t(self->materialSelector));
            if(key) {
                auto** slot=CallGameMethod<void**,0x006DB400>(&tree,&key);
                CallGameMethod<void,0x005AF160>(slot,&output);
            }
            if(shader) CallGameVirtual<void,1>(shader);
        }
        CallGameMethod<void,0x00690F20>(drawable,output,0,context);
        CallGameMethod<void,0x0068F920>(drawable,0);
        if(output) CallGameVirtual<void,1>(output);
    }
    return CallGameMethod<uintptr_t,0x006DB220>(&tree,tree.root);
}
uintptr_t __fastcall CollectCoverage(CoverageHost* self,void*,void* object,void* context) {
    CoverageSet set;
    set.buckets=static_cast<CoverageNode**>(Data<0x0114C900>());set.bucketCount=1;
    set.count=0;set.minLoad=1.0f;set.maxLoad=2.0f;set.storage=nullptr;
    set.tag=static_cast<const char*>(Data<0x00FDDE10>());
    CallGameMethod<void,0x0059E630>(self,&set,object,0);
    CallGameMethod<void,0x0059E630>(self,&set,object,13);
    CallGameMethod<void,0x0059E630>(self,&set,object,2);
    CallGameMethod<void,0x004E16F0>(self->mutex,Data<0x00FDDDDC>());
    auto** bucket=set.buckets;
    while(!*bucket) ++bucket;
    auto* node=*bucket;auto* end=set.buckets[set.bucketCount];
    self->output.end=self->output.begin;
    while(node!=end) {
        auto* current=node->object;
        if(CallGameMethod<uint32_t,0x006C93D0>(current)&0x05222000) {
            if(!(CallGameMethod<uint32_t,0x006C93D0>(current)&0x08854020)) {
                uint32_t a;uint8_t b,c,d;
                if(CallGameMethod<uint8_t,0x0059E240>(self,current,node->extra,context,&a,&b,&c,&d)) {
                    uint32_t first=current->key[0],second=current->key[1];
                    if(first|second) {
                        uint64_t key=uint64_t(first)|(uint64_t(second)<<32);
                        uint64_t packed=(uint32_t(uint8_t(a))<<24)|(uint32_t(b)<<16)|(uint32_t(c)<<8)|d;
                        for(const uint64_t value:{key,packed}) {
                            if(reinterpret_cast<uintptr_t>(self->output.end)<reinterpret_cast<uintptr_t>(self->output.capacity)) {
                                auto* dest=self->output.end;self->output.end=reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(dest)+8);
                                if(dest) *dest=value;
                            } else CallGameMethod<void,0x00572260>(&self->output,self->output.end,&value);
                        }
                    }
                }
            }
        }
        node=node->next;
        while(!node) node=*++bucket;
    }
    CallGameMethod<void,0x004E17B0>(self->mutex);
    auto result=CallGameMethod<uintptr_t,0x007D3AF0>(&set,set.buckets,set.bucketCount);
    set.count=0;
    if(set.bucketCount>1) return CallGameFunction<uintptr_t,0x004E3F70>(set.buckets);
    return result;
}
void* __cdecl CoverageVector() {
    auto* guard=static_cast<uint32_t*>(Data<0x011CE370>());
    if(!(*guard&1)) {
        *guard |= 1;
        CallGameMethod<void,0x004E17E0>(Data<0x011CE330>(),0,1);
        *static_cast<void**>(Data<0x011CE360>())=nullptr;
        *static_cast<void**>(Data<0x011CE364>())=nullptr;
        *static_cast<void**>(Data<0x011CE368>())=nullptr;
        *static_cast<const char**>(Data<0x011CE36C>())=static_cast<const char*>(Data<0x00FDDDF0>());
        *static_cast<uint8_t*>(Data<0x011CE328>())=0;
        CallGameFunction<void,0x00F7E9A1>(Data<0x00F919B0>());
    }
    return Data<0x011CE328>();
}
namespace {
uint8_t CloneDrawable(AdapterOwner* owner,Drawable* source,int32_t selector,unsigned mode) {
    void* raw=Allocate(208,0x00FE36F4);
    auto* clone=raw ? CallGameMethod<Drawable*,0x0068FA50>(raw) : nullptr;
    CallGameMethod<void,0x006909E0>(clone,source);
    auto* material=CallGameMethod<Surface*,0x00444710>(source);
    void* output=nullptr;void* shader=nullptr;
    ShaderOverride param;float alpha=1.0f;
    if(mode==0) {
        param.kind=1;
        param.key=CallGameFunction<uint32_t,0x007CFE70>(Data<0x00FE36E4>());
        for(auto& word:param.zeros) word=0;
        param.value=&alpha;param.count=1;
    }
    CallGameMethod<void,0x006CB1E0>(material->shader,mode==0?&param:nullptr,mode==0?1:0,1,1,&shader);
    CallGameMethod<void,0x006CAC30>(shader,&output,selector);
    CallGameMethod<void,0x00690F20>(clone,output,1,0);
    if(mode==0) {
        source->flags1 &= 0x7f; clone->flags1 |= 0x80;
    } else {
        CallGameMethod<void,0x0068EB60>(clone,mode==5?5:mode);
        if(mode==5) {
            CallGameMethod<void,0x0068EE30>(clone,0);clone->flags1 &= 0x7f;
        } else {
            clone->flags1 &= 0x3f;clone->flags2 |= 1;
            auto* desc=CallGameMethod<const uint32_t*,0x0081C8D0>(clone);
            uint32_t copy[6];for(unsigned i=0;i<6;++i) copy[i]=desc[i];
            auto* handle=CallGameMethod<void*,0x00F20C20>(clone);
            auto* buffer=mode==1 ? CallGameMethod<void*,0x005C8860>(owner,handle,copy[4]) : CallGameMethod<void*,0x005C8A00>(owner,handle,copy[4]);
            CallGameMethod<void,0x0068EB30>(clone,buffer);
            copy[4]*=2;copy[5]*=mode==1?9:2;
            CallGameMethod<void,0x0068EB70>(clone,copy);
            CallGameMethod<void,0x0068EE30>(clone,0);
        }
    }
    CallGameMethod<void,0x006DADB0>(owner->target,clone);
    if(shader) CallGameVirtual<void,1>(shader);
    if(output) CallGameVirtual<void,1>(output);
    return 1;
}
}
uint8_t __fastcall CloneHair(AdapterOwner* self,void*,void*,Drawable* source,int32_t selector) { return CloneDrawable(self,source,selector,0); }
uint8_t __fastcall CloneOrdinary(AdapterOwner* self,void*,void*,Drawable* source,int32_t selector) { return CloneDrawable(self,source,selector,5); }
uint8_t __fastcall CloneFin(AdapterOwner* self,void*,void*,Drawable* source,int32_t selector) { return CloneDrawable(self,source,selector,1); }
uint8_t __fastcall CloneFin2(AdapterOwner* self,void*,void*,Drawable* source,int32_t selector) { return CloneDrawable(self,source,selector,2); }
void* __fastcall Query(void* self,void*,uint32_t id) {
    return id==0x0227ed19u || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu ? self : nullptr;
}
uint8_t __fastcall Resize(Drawables* self,void*,uint32_t count) {
    if(self->items) {
        for(uint32_t i=0;i<self->count;++i) if(self->items[i]) {
            CallGameMethod<void,0x0068FC20>(self->items[i]); Free(self->items[i]);
        }
        Free(self->items); self->items=nullptr;self->count=0;
    }
    if(!count) return 1;
    self->items=static_cast<void**>(Allocate(ArraySize(count),0x00FF6E68));
    if(!self->items) return 0;
    self->count=count;
    for(uint32_t i=0;i<count;++i) self->items[i]=nullptr;
    for(uint32_t i=0;i<count;++i) {
        void* raw=Allocate(208,0x00FF6E50);
        self->items[i]=raw ? CallGameMethod<void*,0x0068FA50>(raw) : nullptr;
        if(!self->items[i]) return 0;
        CallGameMethod<void,0x0068E940>(self->items[i],self);
    }
    return 1;
}
uint8_t __fastcall Append(Drawables* self,void*,void* item) {
    auto** items=static_cast<void**>(Allocate(ArraySize(self->count+1),0x00FF6E68));
    if(!items) return 0;
    for(uint32_t i=0;i<self->count;++i) items[i]=self->items[i];
    Free(self->items); items[self->count++]=item;self->items=items;return 1;
}
Substitution* __fastcall InitSubstitution(Substitution* self,void*) {
    for(auto& p:self->first) p=nullptr;
    self->flags[0]=0;self->flags[1]=0;
    self->minLoad=1.0f;self->maxLoad=2.0f;
    self->tag=static_cast<const char*>(Data<0x00FF6EA0>());
    self->bucketCount=1;self->buckets=Data<0x0114C900>();self->count=0;self->storage=nullptr;
    return self;
}
uint8_t __fastcall Clone(void* self,void*,void** output,void* substitutions,void* arg) {
    void* raw=Allocate(128,0x00FF6EF0);
    void* clone=raw ? CallGameMethod<void*,0x006DB0F0>(raw) : nullptr;
    if(clone) CallGameVirtual<void,0>(clone);
    if(CallGameMethod<uint8_t,0x006DB190>(self,clone)) {
        if(substitutions) CallGameMethod<void,0x006DB480>(clone,self,substitutions,arg);
        *output=clone;return 1;
    }
    if(clone) CallGameVirtual<void,1>(clone);
    return 0;
}
uintptr_t __fastcall DelaySwap(Proxy* self,void*,void* resource,void* model,void* callback) {
    if(self->delayed) CallGameMethod<void,0x006DCFC0>(self,1);
    auto* params=static_cast<SwapParams*>(Allocate(16,0x00FF6F2C));
    if(params) { params->first=nullptr;params->second=nullptr;params->callback=nullptr; }
    self->delayed=params;
    void* old=self->model;
    if(model!=old) {
        if(model) CallGameVirtual<void,0>(model);
        self->model=model;
        if(old) CallGameVirtual<void,1>(old);
    }
    auto result=CallGameMethod<uintptr_t,0x005E03C0>(self->delayed,resource);
    old=self->delayed->callback;
    if(callback!=old) {
        if(callback) result=CallGameVirtual<uintptr_t,0>(callback);
        self->delayed->callback=callback;
        if(old) result=CallGameVirtual<uintptr_t,1>(old);
    }
    if(callback) return CallGameVirtual<uintptr_t,6>(callback,self->context);
    return result;
}
Proxy* __fastcall InitProxy(Proxy* self,void*) {
    for(auto& p:self->first) p=nullptr;
    self->one=1;self->enabled=0;self->mask=255;
    self->invalid[0]=self->invalid[1]=0xffffffffu;self->selector=0xffff;
    for(auto& p:self->middle) p=nullptr;
    self->model=nullptr;self->delayed=nullptr;
    self->callbackTag=static_cast<const char*>(Data<0x00FF6F54>());
    self->callbacks.next=self->callbacks.prev=&self->callbacks;
    self->callbackCount=0;self->context=nullptr;
    self->flags[0]=self->flags[1]=self->flags[2]=0;
    for(auto& p:self->selection) p=nullptr;
    self->selectionTag=static_cast<const char*>(Data<0x00FF6F08>());self->tail=nullptr;
    return self;
}
}
