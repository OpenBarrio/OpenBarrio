#include "ts3/ui/clu_6e8410_UI.h"
#include <cstring>
namespace ts3::ui {
namespace {
struct ChildLayout { unsigned char prefix[32]; UiLink link; };
TS3_X86_OFFSET(ChildLayout, link, 32);
}

uint32_t UiEntry::GetPair(void* a, void* b) { *static_cast<uint32_t*>(a)=first; const uint32_t result=second; *static_cast<uint32_t*>(b)=result; return result; }
uintptr_t UiEntry::Invoke() { return callback ? callback(this) : 0; }
int UiEntry::IsActive() { return active != 0; }
UiEntry* UiEntry::GetContext() { return context; }
void* UiEntry::GetOwnerValue() { return owner ? owner->value : nullptr; }
int UiEntry::HasBinding() {
    return CallGameMethod<uint8_t,0x6db760>(binding) || CallGameMethod<uint8_t,0x6db830>(binding);
}
uint8_t UiEntry::SetBinding(uint32_t a, uint32_t b, void* arg) {
    if(arg) { CallGameMethod<void,0x6dd160>(binding,a,arg,b); flags668 &= ~1u; }
    else {
        CallGameMethod<void,0x6dcbe0>(binding,a);
        bool same = owner && owner->value == CallGameFunction<void*,0x6e8330>();
        // The original overwrites only the low byte of the first stack argument.
        CallGameMethod<void,0x6db8e0>(binding,b,(a & 0xffffff00u) | uint32_t(same));
        flags668 |= 1;
        if(auto* value=GetOwnerValue()) {
            struct Value { unsigned char pad[12]; void* target; };
            CallGameMethod<void,0x6e75d0>(static_cast<Value*>(value)->target,arg52);
        }
    }
    flags668 = (flags668 & 0xfb) | 2;
    CallGameMethod<void,0x6fac70>(this); return 1;
}
UiIdentity* UiIdentity::Init() {
#if defined(_M_IX86)
    float one=*reinterpret_cast<const float*>(0x107a538);
#else
    float one=1.0f;
#endif
    tag=0; for(int i=0;i<16;++i) matrix[i]=(i%5==0)?one:0.0f; return this;
}
uintptr_t UiEntry::NotifyChildren() {
    // Child-list nodes belong to a different subtype: its link is at +0x20.
    uintptr_t result=0; // Empty-list EAX is undefined in the original.
    if(children.prev != &children)
        for(UiLink* p=children.next;p!=&children;p=p->next)
            result=CallGameMethod<uintptr_t,0x6fb5b0>(p ? reinterpret_cast<unsigned char*>(p)-offsetof(ChildLayout,link) : nullptr,this);
    return result;
}
uint8_t UiManager::Bind(uint32_t lo,uint32_t hi,uint32_t a,uint32_t b,void* arg) {
    uint32_t key[2]={lo,hi}; void* found=nullptr;
    CallGameMethod<void,0x6fdfb0>(reinterpret_cast<unsigned char*>(this)+4,&found,key);
    if(found!=sentinel && found && reinterpret_cast<uintptr_t>(found)!=8) {
        auto* object=static_cast<unsigned char*>(found)-8;
#if defined(_M_IX86)
        auto* entry=CallGameVirtual<UiEntry*,3>(object,reinterpret_cast<void*>(0x17f1d88));
#else
        auto* entry=CallGameVirtual<UiEntry*,3>(object,static_cast<void*>(nullptr));
#endif
        if(entry) return entry->SetBinding(a,b,arg);
    }
    if(arg) CallGameVirtual<void,4>(arg); return 0;
}
namespace {
struct MapNode { unsigned char pad[8]; UiLink list; };
struct Iterator { MapNode* node; uintptr_t extra; };
struct InsertResult { Iterator it; uint8_t inserted; };
}
bool UiManager::Update(UiEntry* e,uint64_t key,int32_t kind,int32_t mode,uint8_t flag) {
    if(!e) return false;
    uint64_t old=CallGameMethod<uint64_t,0x6facb0>(e);
    int32_t oldKind=CallGameMethod<int32_t,0x9158f0>(e);
    CallGameVirtual<void,19>(e,uint32_t(key),uint32_t(key>>32));
    CallGameVirtual<void,20>(e,kind); CallGameVirtual<void,22>(e,mode);
    CallGameMethod<void,0x6facc0>(e,uint8_t(mode==0 || mode==-1));
    CallGameMethod<void,0x6face0>(e,flag);
    auto sentinelNode=[&] { return static_cast<MapNode*>(buckets[bucketIndex]); };
    if(old!=key) {
        if(old) {
            Iterator it{};
            auto* result=CallGameMethod<Iterator*,0xd726f0>(map,&it,&old);
            auto* node=result->node; auto extra=result->extra;
            if(node!=sentinelNode()) {
                e->link.prev->next=e->link.next; e->link.next->prev=e->link.prev;
                if(node->list.prev==&node->list) {
                    Iterator erased{};
                    CallGameMethod<void,0xa7e3b0>(map,&erased,Iterator{node,extra});
                }
            }
        }
        if(key) {
            Iterator it{}; auto* result=CallGameMethod<Iterator*,0xd726f0>(map,&it,&key);
            auto* node=result->node;
            if(node==sentinelNode()) {
                InsertResult inserted{};
                CallGameMethod<void,0x7007b0>(map,&inserted,&key,uint8_t(0));
                if(inserted.inserted) node=inserted.it.node;
            }
            if(node!=sentinelNode()) {
                e->link.prev=node->list.prev; e->link.next=&node->list;
                node->list.prev=&e->link; e->link.prev->next=&e->link;
            }
        }
    }
    if(auto* context=e->GetContext()) {
        auto* value=context->GetOwnerValue();
        CallGameMethod<void,0x6fdd20>(this,e,value,context->arg56);
        return value!=nullptr;
    }
    if(old!=key || oldKind!=kind) {
        uint32_t flags=CallGameMethod<uint32_t,0x6c93d0>(e);
        bool special=(e->flags521&1) && !e->GetContext();
        UiEntry* target=CallGameMethod<UiEntry*,0x700d50>(this,uint32_t(key),uint32_t(key>>32),kind,flags);
        if(((flags&0x200000)||special) && mode==0 && kind==0 &&
           (!target || (!target->IsActive() && !(reinterpret_cast<uintptr_t>(target->owner)&0x10)))) target=nullptr;
        CallGameMethod<void,0x6fdd20>(this,e,e->GetOwnerValue(),target);
    }
    return true;
}
void UiCleanup::Clear(uint8_t release,uint8_t notify) {
    if(entry) {
        // This is a pointer field in cleanup's entry subtype, at +528.
        *reinterpret_cast<void**>(entry->binding)=nullptr;
        if(entry->GetContext()) { alignas(16) unsigned char storage[144]; auto* identity=reinterpret_cast<UiIdentity*>(storage); identity->Init(); CallGameMethod<void,0x6fc860>(entry,0,identity); }
        if(entry->owner) CallGameMethod<void,0x6e4920>(entry->owner,entry);
        if(entry) { auto* p=entry; entry=nullptr; CallGameVirtual<void,1>(p); }
    }
    if(!resource) return;
    if(primary) CallGameMethod<void,0x6faa90>(primary,0);
    if(secondary) CallGameMethod<void,0x6faa90>(secondary,0);
    if(tertiary) CallGameMethod<void,0x6faa90>(tertiary,0);
    auto p=begin; auto limit=blockEnd; auto mapBlock=block; auto finish=end;
    while(p!=finish) { CallGameMethod<void,0x6faa90>(*p,0); ++p; if(p==limit) { p=*++mapBlock; limit=p+64; } }
    if(release && resource) {
        struct Resource { void** vt; uint32_t refs; }; auto* p=static_cast<Resource*>(resource); resource=nullptr;
        if(--p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,1); }
    }
    if(primary && notify) CallGameVirtual<void,4>(primary);
}
void UiAnimation::Tick() {
    float time=0;
    if(auto* value=base.GetOwnerValue()) {
#if defined(_M_IX86)
        time=static_cast<float>(CallGameMethod<double,0x6e8910>(value) * static_cast<double>(*reinterpret_cast<const float*>(0x1029f64)));
#else
        time=static_cast<float>(CallGameMethod<double,0x6e8910>(value)*0.00027777778450399637);
#endif
    }
    if(last!=0.0f) {
        float boundary=threshold+2.0f;
        if(time>=boundary && boundary>last) {
            auto* pending=reinterpret_cast<int32_t*>(base.binding+64);
            if(*pending>=0) { selection=*pending; index=0; *pending=-1; }
            base.binding[69]=base.binding[68];
            index=static_cast<int32_t>(uint32_t(index)+1) % counts[selection];
        }
    }
    last=time;
}
}
