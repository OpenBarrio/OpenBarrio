#include "ts3/resource/m03_G01.h"
namespace ts3::resource::m03::g01 {
namespace {
void Lock(void* p) { CallGameMethod<void,0x004E16F0>(p,reinterpret_cast<void*>(uintptr_t(0x00FB2CD0))); }
uint32_t Unlock(void* p) { return CallGameMethod<uint32_t,0x004E17B0>(p); }
void* Adjust(void* p) { return static_cast<uint8_t*>(p)+sizeof(void*); }
void Retain(void* p) { if(p) CallGameVirtual<void,0>(Adjust(p)); }
void Release(void* p) { if(p) CallGameVirtual<void,1>(Adjust(p)); }
void Erase(Vector& v,Pair* pos,bool primary) {
    if(pos+1<v.end) {
        if(primary) CallGameFunction<Pair*,0x004B0BF0>(pos+1,v.end,pos);
        else CallGameFunction<Pair*,0x004B0B90>(pos+1,v.end,pos);
    }
    --v.end;
    if(v.end->object) {
        if(primary) CallGameVirtual<void,2>(v.end->object);
        else Release(v.end->object);
    }
}
void Append(Vector& v,const Pair* source,bool primary) {
    Pair* p=v.end;
    if(uintptr_t(p)<uintptr_t(v.capacity)) {
        ++v.end;
        if(p) {
            p->object=source->object;
            if(p->object) {
                if(primary) CallGameVirtual<void,1>(p->object);
                else Retain(p->object);
            }
            p->flags=source->flags;
        }
    } else if(primary) CallGameMethod<uintptr_t,0x004B20E0>(&v,p,source);
    else CallGameMethod<uintptr_t,0x004B1F50>(&v,p,source);
}
Pair* Position(Vector& v,int32_t priority) {
    auto* p=v.begin;
    while(p!=v.end && priority<=int32_t(p->flags)) ++p;
    return p;
}
}
uint8_t __fastcall Factory(Manager* m,void*,uint32_t enable,void* object,int32_t priority) {
    if(!object) return 0;
    auto& h=m->factories;
    if(uint8_t(enable)) {
        CallGameVirtual<void,4>(object);
        uint32_t count=CallGameVirtual<uint32_t,11>(object,static_cast<uint32_t*>(nullptr),0u);
        uint32_t local[8]; uint32_t* types=local;
        if(count>8) types=static_cast<uint32_t*>(CallGameVirtual<void*,2>(m->allocator,size_t(4u*count),"ResourceSystem/ResourceMan/TypeIdArray",0u));
        CallGameVirtual<uint32_t,11>(object,types,count);
        Lock(m->factoryMutex);
        for(uint32_t i=0;i<count;++i) {
            void* found[2]; CallGameMethod<void*,0x00576C30>(&h,found,types+i);
            auto* node=static_cast<Node*>(found[0]);
            if(node==h.buckets[h.count]) {
                Vector empty{};
                empty.allocator=GameAllocator("ResourceSystem/ResourceMan/Mgr/FArray",m->allocator,0);
                Node value{};
                CallGameMethod<void*,0x004B1960>(&value,types+i,&empty);
                CallGameMethod<void,0x004B0AB0>(&empty);
                b01::Result out;
                CallGameMethod<void*,0x004B2320>(&h,&out,&value,uintptr_t(0));
                node=out.node;
                CallGameMethod<void,0x004B0AB0>(&value.values);
            }
            auto& v=node->values; auto* p=v.begin;
            while(p!=v.end && p->object!=object && int32_t(p->flags)>=priority) ++p;
            Pair temporary{object,uintptr_t(uint32_t(priority))};
            CallGameVirtual<void,1>(object);
            if(p==v.end) Append(v,&temporary,true);
            else CallGameMethod<uintptr_t,0x004B20E0>(&v,p,&temporary);
            if(temporary.object) CallGameVirtual<void,2>(temporary.object);
        }
        Unlock(m->factoryMutex);
        if(types!=local) CallGameVirtual<uintptr_t,3>(m->allocator,types,size_t(0));
    } else {
        Lock(m->factoryMutex);
        struct Iterator { Node* node; Node** cell; } it;
        CallGameMethod<void*,0x00626610>(&h,&it);
        auto* sentinel=h.buckets[h.count];
        while(it.node!=sentinel) {
            auto& v=it.node->values; auto* p=v.begin;
            while(p!=v.end) {
                if(p->object==object) {
                    if(CallGameVirtual<int32_t,3>(object)==1) CallGameVirtual<void,5>(object);
                    Erase(v,p,true);
                } else ++p;
            }
            it.node=it.node->next;
            while(!it.node) it.node=*++it.cell;
        }
        Unlock(m->factoryMutex);
    }
    return 1;
}
uint32_t __fastcall Database(Manager* m,void*,uint32_t enable,void* object,int32_t priority) {
    Lock(m->databaseMutex); uint8_t result=0; auto& v=m->databases;
    if(uint8_t(enable)) {
        if(!CallGameVirtual<uint8_t,14>(m,object)) {
            Retain(object); Retain(object);
            Pair temporary{object,uintptr_t(uint32_t(priority))}; Release(object);
            auto* p=Position(v,priority);
            if(p==v.end) Append(v,&temporary,false);
            else CallGameMethod<Pair*,0x004B25B0>(&v,p,&temporary);
            Unlock(m->databaseMutex);
            CallGameVirtual<void,1>(object);
            result=CallGameVirtual<uint8_t,18>(object,1u,m,1u);
            Lock(m->databaseMutex);
            Release(temporary.object);
        }
    } else {
        auto* p=v.begin; while(p!=v.end && p->object!=object) ++p;
        if(p!=v.end) {
            void* saved=p->object;
            Unlock(m->databaseMutex);
            result=CallGameVirtual<uint8_t,18>(object,0u,m,1u);
            if(CallGameVirtual<int32_t,4>(saved)==1) CallGameVirtual<void,2>(saved);
            Lock(m->databaseMutex); Erase(v,p,false);
        }
    }
    return (Unlock(m->databaseMutex)&0xffffff00u)|result;
}
uint32_t __fastcall Priority(Manager* m,void*,void* object,int32_t priority) {
    Lock(m->databaseMutex); auto& v=m->databases;
    auto* p=v.begin; while(p!=v.end && p->object!=object) ++p;
    if(p!=v.end && int32_t(p->flags)!=priority) {
        Retain(object); Retain(object); Release(object);
        // EA keeps p as the insertion source across the erase. It neither saves
        // the pair nor writes priority: a middle erase aliases its successor.
        Erase(v,p,false);
        auto* dest=Position(v,priority);
        if(dest==v.end) Append(v,p,false);
        else CallGameMethod<uintptr_t,0x004B1F50>(&v,dest,p);
        Release(object);
    }
    return Unlock(m->databaseMutex);
}
}
