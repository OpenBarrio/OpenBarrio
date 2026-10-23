#include "ts3/render/m07_B33.h"
#include <xmmintrin.h>
namespace ts3::render::m07::b33 {
#if defined(_M_IX86)
Constants constants{reinterpret_cast<void**>(0x0114C900),reinterpret_cast<const char*>(0x00FF5640),reinterpret_cast<const char*>(0x00FF5620),reinterpret_cast<const char*>(0x00FF56AC),reinterpret_cast<const char*>(0x00FF5684),reinterpret_cast<const char*>(0x00FF5884),reinterpret_cast<const char*>(0x00FF5854)};
#else
static void* empty[2]{};
Constants constants{empty,"Lighting/LightUpdateTree/mWorldObjects","Lighting/LightUpdateTree/mLots","Lighting/LightUpdateTree/mCollections","Lighting/LightUpdateTree/mDirtyObjects","Lighting/LightTreeLevel/mDirtyRooms","Lighting/LightTreeLevel/mRoomsPendingRestart"};
#endif
static void InitHash(Hash& h,const char* name) {
    h.maxLoad=1.0f; h.growth=2.0f; h.name=name; h.size=0; h.pool=nullptr;
    h.buckets=constants.emptyBuckets; h.bucketCount=1;
}
WorldTree* __fastcall InitWorld(WorldTree* self,void*,void* owner) {
    InitHash(self->objects,constants.objects); InitHash(self->lots,constants.lots);
    self->active=nullptr; self->owner=owner;
    for(auto& f:self->bounds) f=0.0f;
    self->enabled=1;
    return self;
}
Group* __fastcall InitGroup(Group* self,void*,uint8_t kind) {
    self->kind=kind;
    InitHash(self->collections,constants.collections); InitHash(self->dirty,constants.dirty);
    return self;
}
Level* __fastcall InitLevel(Level* self,void*) {
    InitHash(self->dirtyRooms,constants.dirtyRooms); InitHash(self->pending,constants.pending);
    for(unsigned i=0;i<5;++i) InitGroup(&self->groups[i],nullptr,i==4?1:0);
    self->first=nullptr; self->second=nullptr; self->state=0;
    return self;
}
Dependencies dependencies;
static void* Allocate(uint32_t bytes,const char* name) {
    return Original(0x004E3F90,dependencies.allocate)(bytes,name,0,0,0,0);
}
static uintptr_t InsertNode(Group* group,uint32_t lo,uint32_t hi,Node* node) {
    return dependencies.append?dependencies.append(group,nullptr,lo,hi,node):Append(group,nullptr,lo,hi,node);
}
void* __fastcall Lookup(Group* self,void*,uint32_t lo,uint32_t hi) {
    Iterator it; Key key{lo,hi};
    Original(0x00D726F0,dependencies.find)(&self->collections,nullptr,&it,&key);
    return it.node==self->collections.buckets[self->collections.bucketCount]?nullptr:it.node->value;
}
void* __fastcall GetLot(WorldTree* self,void*,uint32_t lo,uint32_t hi) {
    Key key{lo,hi}; InsertResult result;
    Original(0x00A39C70,dependencies.insert)(&self->lots,nullptr,&result,&key,0);
    if(!result.inserted) return result.iterator.node->value;
    void* storage=Allocate(0x13c0,"Lighting/UpdateTracker/Lot");
    void* lot=storage?Original(0x006C70C0,dependencies.lotCtor)(storage,nullptr,self,lo,hi):nullptr;
    result.iterator.node->value=lot;
    return lot;
}
Node* __fastcall WorldLeaf(Hash* self,void*,SceneObject* object) {
    void* data=Original(0x006FACA0,dependencies.leafData)(object,nullptr);
    void* storage=Allocate(64,"Lighting/LightUpdateTree/WorldLeaf");
    Node* node=storage?Original(0x006C7ED0,dependencies.leafCtor)(storage,nullptr,object,data,nullptr,nullptr):nullptr;
    *Original(0x009B4A70,dependencies.index)(self,nullptr,&object->key)=node;
    return node;
}
uintptr_t __fastcall Append(Group* self,void*,uint32_t lo,uint32_t hi,Node* node) {
    Hash* map=&self->collections; Key key{lo,hi}; Iterator it;
    void* sentinel=map->buckets[map->bucketCount];
    auto* found=Original(0x00D726F0,dependencies.find)(map,nullptr,&it,&key);
    if(found->node==sentinel) {
        auto* collection=static_cast<Collection*>(Allocate(sizeof(Collection),"Lighting/LightTreeLevel/LightTreeNodeCollection"));
        if(collection) {
            collection->local=collection->storage; collection->end=collection->storage;
            collection->begin=collection->storage; collection->capacity=collection->storage+4;
            collection->name="Lighting/LightUpdateTree/mLightTreeNodes";
        }
        *Original(0x009B4A70,dependencies.index)(map,nullptr,&key)=collection;
    }
    auto* collection=static_cast<Collection*>(*Original(0x009B4A70,dependencies.index)(map,nullptr,&key));
    Node** end=collection->end;
    if(reinterpret_cast<uintptr_t>(end)>=reinterpret_cast<uintptr_t>(collection->capacity))
        Original(0x00B7B1E0,dependencies.grow)(collection,nullptr,end,&node);
    else {
        collection->end=reinterpret_cast<Node**>(reinterpret_cast<uintptr_t>(end)+sizeof(Node*));
        if(end) *end=node;
    }
    return CallGameVirtual<uintptr_t,3>(node);
}
Node* __fastcall Emitter(Level* self,void*,void* emitter,void* data,uint32_t lo,uint32_t hi) {
    void* storage=Allocate(48,"Lighting/LightUpdateTree/NewLightEmitterNode");
    Node* node=storage?Original(0x006C8000,dependencies.emitterCtor)(storage,nullptr,lo,hi,emitter,data,self,&self->groups[1]):nullptr;
    InsertNode(&self->groups[1],lo,hi,node); InsertNode(&self->groups[4],lo,hi,node);
    return node;
}
void* __fastcall Rig(Level* self,void*,void* rig,void* owner,uint32_t lo,uint32_t hi,uint8_t paired) {
    void* storage=Allocate(48,"Lighting/LightUpdateTree/NewLightRigNode");
    Node* node=storage?Original(0x006C80E0,dependencies.rigCtor)(storage,nullptr,lo,hi,rig,self,owner,paired,&self->groups[0]):nullptr;
    InsertNode(&self->groups[0],lo,hi,node); InsertNode(&self->groups[4],lo,hi,node);
    CallGameVirtual<void,2>(node);
    return node->value;
}
uintptr_t __fastcall SceneLeaf(Level* self,void*,void* data,SceneObject* object) {
    void* storage=Allocate(64,"Lighting/LightUpdateTree/SceneObjectNode");
    Node* node=storage?Original(0x006C7ED0,dependencies.leafCtor)(storage,nullptr,object,data,self,&self->groups[3]):nullptr;
    InsertNode(&self->groups[3],object->key.lo,object->key.hi,node);
    InsertNode(&self->groups[4],object->key.lo,object->key.hi,node);
    if(dependencies.populate) return dependencies.populate(self,nullptr,node);
    return Populate(self,nullptr,node);
}

#if defined(_M_IX86)
Globals globals{reinterpret_cast<PointerArray*>(0x011D1230),reinterpret_cast<uint32_t*>(0x011D1240),reinterpret_cast<PointerArray*>(0x011D1244),reinterpret_cast<uint32_t*>(0x011D1254)};
#else
static PointerArray nativeWorld{},nativeRooms{};
static uint32_t nativeWorldInit=0,nativeRoomInit=0;
Globals globals{&nativeWorld,&nativeWorldInit,&nativeRooms,&nativeRoomInit};
#endif
void __fastcall SelectRoom(WorldTree* self,void*) {
    if(self->active) return;
    alignas(16) Rank local[1024/sizeof(Rank)];
    RankArray array{local,local,local+1024/sizeof(Rank),"Lighting/LightUpdateTree/RankArray",local};
    void** bucket=self->lots.buckets;
    while(!*bucket) ++bucket;
    Entry* entry=static_cast<Entry*>(*bucket);
    void* sentinel=self->lots.buckets[self->lots.bucketCount];
    if(entry==sentinel) return;
    do {
        auto* lot=static_cast<LotLevels*>(entry->value);
        for(int level=0;level<12;++level)
            if(lot->levels[level].room) Original(0x006A8190,dependencies.rank)(lot->levels[level].room,nullptr,&array);
        entry=entry->next;
        while(!entry) entry=static_cast<Entry*>(*++bucket);
    } while(entry!=sentinel);
    if(array.begin!=array.end) {
        Original(0x006C58B0,dependencies.sort)(array.begin,array.end);
        if(array.begin->weight>0.001f) {
            self->active=array.begin->room;
            Original(0x0069E860,dependencies.retainRoom)(self->active,nullptr);
        }
    }
    if(array.begin && array.begin!=array.local) Original(0x004E3F70,dependencies.free)(array.begin);
}
uintptr_t __fastcall Populate(Level* self,void*,Node* node) {
    uint32_t lo=node->key.lo,hi=node->key.hi;
    auto* first=Original(0x006C7780,dependencies.getRig)(node,nullptr,0);
    auto* second=Original(0x006C7780,dependencies.getRig)(node,nullptr,1);
    uint8_t paired=(first && second)?1:0;
    auto addRig=[&](RigSource* source) {
        if(dependencies.rig) dependencies.rig(self,nullptr,source,node,lo,hi,paired);
        else Rig(self,nullptr,source,node,lo,hi,paired);
    };
    if(first) addRig(first);
    if(second) addRig(second);
    void* manager=Original(0x006B05A0,dependencies.manager)();
    if(!manager) return 0;
    LightingData* data=Original(0x006B0860,dependencies.lighting)(manager,nullptr,lo,hi);
    if(!data) return 0;
    Original(0x006CAC00,dependencies.enable)(node,nullptr,1);
    if(data->begin!=data->end) {
        uint32_t count=uint32_t(data->end-data->begin);
        bool grouped=uint8_t(Original(0x006C7A70,dependencies.grouped)(node,nullptr))!=0;
        bool any=false;
        if(grouped) for(uint32_t i=0;i<count;++i)
            if(uint8_t(CallGameVirtual<uint32_t,6>(data->begin[i]))) { any=true;break; }
        auto add=[&](EmitterSource* source) {
            return dependencies.emitter?dependencies.emitter(self,nullptr,source,data,lo,hi):Emitter(self,nullptr,source,data,lo,hi);
        };
        if(any) {
            // EA's fixed two groups of eight are intentional, not an unbounded vector.
            Node* groups[16]; uint32_t a=0,b=0; Node* firstA=nullptr;Node* firstB=nullptr;
            for(uint32_t i=0;i<count;++i) {
                auto* source=data->begin[i]; Node* n=add(source);
                if(source->flags&8) { groups[8+b++]=n;if(!firstB) firstB=n; }
                else { groups[a++]=n;if(!firstA) firstA=n; }
            }
            if(a && b) {
                for(uint32_t i=0;i<a;++i) Original(0x006CABF0,dependencies.link)(groups[i],nullptr,firstB);
                for(uint32_t i=0;i<b;++i) Original(0x006CABF0,dependencies.link)(groups[8+i],nullptr,firstA);
            }
        } else for(uint32_t i=0;i<count;++i) add(data->begin[i]);
    }
    if(data->occluders==data->occludersEnd) first->flags&=uint8_t(~4u);
    else {
        void* storage=Allocate(40,"Lighting/LightUpdateTree/NewOccluderNode");
        Node* occluder=storage?Original(0x006C8180,dependencies.occluderCtor)(storage,nullptr,lo,hi,data,node,self,&self->groups[2]):nullptr;
        InsertNode(&self->groups[2],lo,hi,occluder);InsertNode(&self->groups[4],lo,hi,occluder);
        first->flags|=4;
    }
    return reinterpret_cast<uintptr_t>(first);
}
static void Initialize(PointerArray* array,uint32_t* flag,const char* name,uint32_t destructor) {
    if((*flag&1)==0) {
        *flag|=1;
        array->begin=nullptr;array->end=nullptr;array->capacity=nullptr;array->name=name;
        void (__cdecl* callback)()=nullptr;
#if defined(_M_IX86)
        callback=reinterpret_cast<void (__cdecl*)()>(static_cast<uintptr_t>(destructor));
#else
        (void)destructor;
#endif
        Original(0x00F7E9A1,dependencies.atExit)(callback);
    }
}
uintptr_t __fastcall UpdateWorld(Level* self,void*,Room* room) {
    PointerArray* array=globals.worldLights;
    Initialize(array,globals.worldInit,"Lighting/LightUpdateTree/worldLights",0x00F92240);
    Original(0x006A18A0,dependencies.clearLights)(room,nullptr);
    uintptr_t result=Original(0x006C6990,dependencies.collect)(self,nullptr,room,1);
    if(room->id) return result;
    auto* owner=static_cast<Owner*>(self->second);
    if(!self->state && owner) Original(0x006C6990,dependencies.collect)(&owner->level,nullptr,room,0);
    result=reinterpret_cast<uintptr_t>(owner);
    if(owner->data->settings->lights) {
        array->end=array->begin;
        float x=float(double(room->width)*0.5),z=float(double(room->height)*0.5);
        auto* m=room->transform;
        __m128 v=_mm_add_ps(_mm_mul_ps(_mm_set1_ps(x),_mm_load_ps(m[0])),_mm_load_ps(m[3]));
        v=_mm_add_ps(v,_mm_mul_ps(_mm_set1_ps(0.0f),_mm_load_ps(m[1])));
        v=_mm_add_ps(v,_mm_mul_ps(_mm_set1_ps(z),_mm_load_ps(m[2])));
        alignas(16) float point[4];_mm_store_ps(point,v);
        result=Original(0x006B66B0,dependencies.query)(owner->data->settings->lights,nullptr,point,array);
        for(void** it=array->begin;it!=array->end;++it) result=Original(0x006A2060,dependencies.addLight)(room,nullptr,*it);
    }
    return result;
}
uintptr_t __fastcall UpdateRoom(Level* self,void*,Room* room) {
    PointerArray* array=globals.roomOccluders;
    Initialize(array,globals.roomInit,"Lighting/LightUpdateTracker/roomOccluders",0x00F92260);
    Original(0x0069E600,dependencies.clearOccluders)(room,nullptr);
    Original(0x006C6A20,dependencies.collectOccluders)(self,nullptr,room->id,array);
    for(void** it=array->begin;it!=array->end;++it) Original(0x0069E5E0,dependencies.addOccluder)(room,nullptr,*it);
    if(dependencies.updateWorld) dependencies.updateWorld(self,nullptr,room); else UpdateWorld(self,nullptr,room);
    void* callback=nullptr;
#if defined(_M_IX86)
    callback=reinterpret_cast<void*>(0x006C5FD0);
#endif
    Original(0x006C6800,dependencies.visit)(self,nullptr,&self->groups[0],callback,room->id);
    return Original(0x006A2E20,dependencies.finish)(room,nullptr);
}

}
