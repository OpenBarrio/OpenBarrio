#include "ts3/anim/m05_B01.h"
#include <cstring>
namespace ts3::anim::b01 {
namespace {
void* BaseTable() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x00fc1080u);
#else
    static void* table[11]{}; return table;
#endif
}
Node* Owner(ListLink* link) { return link ? reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(Node,link)):nullptr; }
bool Advance(Cursor* s,bool write) {
    s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+4u);
    if(s->position>=0) {
        if(write) CallGameMethod<void,0x00594140>(s,4);
        else CallGameMethod<void,0x00593ff0>(s,4);
    }
    return s->position<0;
}
uint32_t Read(Cursor* s,uint32_t fallback) {
    if(Advance(s,false)) std::memcpy(&fallback,s->input+static_cast<ptrdiff_t>(s->position)-3,4);
    return fallback;
}
void Write(Cursor* s,uint32_t value) {
    if(Advance(s,true)) std::memcpy(s->output+static_cast<ptrdiff_t>(s->position)-3,&value,4);
}
Node* Create(void* host,uint32_t type) { return CallGameMethod<Node*,0x0054c770>(host,type); }
// FLD/FSTP in the original load quiets signaling NaNs (unlike MOVSS in Save).
uint32_t X87Copy(uint32_t bits) {
    if ((bits & 0x7f800000u)==0x7f800000u && (bits & 0x007fffffu)!=0)
        bits |= 0x00400000u;
    return bits;
}
}
float Weight(const Node* n) { return n->weight; }
uintptr_t SetWeight(Node* n,float w) {
    n->weight=w;
    if(n->parent) return CallGameVirtual<uintptr_t,6>(n->parent);
    n->effective=w; return reinterpret_cast<uintptr_t>(n);
}
bool WriteNode(Node* n,Cursor* s) {
    Write(s,CallGameVirtual<uint32_t,0>(n));
    CallGameVirtual<void,9>(n,s); return true;
}
bool ReadNode(void* host,Node** out,Cursor* s) {
    auto type=Read(s,static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s)));
    *out=Create(host,type); CallGameVirtual<void,10>(*out,s); return true;
}
void Load(Node* n,Cursor* s) {
    void* host=n->host;
    uint32_t bits; std::memcpy(&bits,&n->weight,4);
    if (Advance(s,false)) {
        std::memcpy(&bits,s->input+static_cast<ptrdiff_t>(s->position)-3,4);
        bits=X87Copy(bits); std::memcpy(&n->weight,&bits,4);
    }
    bits=X87Copy(bits); std::memcpy(&n->effective,&bits,4);
    CallGameVirtual<void,8>(n,s);
    uint32_t count=Read(s,0), type=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(host));
    for(uint32_t i=0;i<count;++i) {
        type=Read(s,type); Node* child=Create(host,type);
        CallGameVirtual<void,10>(child,s); CallGameVirtual<void,3>(n,child);
    }
}
void Reset(Node* n) { n->vtable=static_cast<void**>(BaseTable()); }
uint32_t Type(Node*) { return 0x73deb8c8u; }
Node* Query(Node* n,uint32_t type) { return type==0x73deb8c8u?n:nullptr; }
uintptr_t Attach(Node* n,Node* child) {
    child->link.previous=n->children.previous;
    child->link.next=&n->children;
    n->children.previous=&child->link;
    child->link.previous->next=&child->link;
    child->parent=n;
    return CallGameVirtual<uintptr_t,6>(n);
}
uintptr_t Detach(Node* n,Node* child) {
    child->link.previous->next=child->link.next;
    child->link.next->previous=child->link.previous;
    child->parent=nullptr;
    return CallGameVirtual<uintptr_t,6>(n);
}
Node* Construct(Node* n,void* host) {
    Reset(n); n->children.next=n->children.previous=&n->children;
    n->parent=nullptr; n->host=host; n->weight=n->effective=1.0f; return n;
}
Node* Destroy(Node* n,uint32_t flags) {
    Reset(n); if(flags&1) CallGameFunction<void,0x004e3f70>(n); return n;
}
void Save(Node* n,Cursor* s) {
    uint32_t bits; std::memcpy(&bits,&n->weight,4); Write(s,bits);
    CallGameVirtual<void,7>(n,s);
    uint32_t count=0;
    for(auto p=n->children.next;p!=&n->children;p=p->next) ++count;
    Write(s,count);
    for(auto p=n->children.next;p!=&n->children;) {
        Node* child=Owner(p); WriteNode(child,s); p=child->link.next;
    }
}
void* Find(uint32_t key) {
#if defined(_M_IX86)
    auto r=reinterpret_cast<Registry*>(0x011cd120u);
    Entry* result[2]{};
    CallGameMethod<void,0x00d38b00>(r,result,&key);
    return result[0]==r->buckets[r->bucketCount]?nullptr:result[0]->value;
#else
    (void)key; AbortNotRewritten("evaluator registry global"); return nullptr;
#endif
}
uint32_t Erase(Registry* r,const uint32_t* key) {
    const uint32_t old=r->count;
    Entry** link=&r->buckets[*key%r->bucketCount];
    while(*link && (*link)->key!=*key) link=&(*link)->next;
    while(*link && (*link)->key==*key) {
        Entry* e=*link; *link=e->next;
        if(e!=r->special) {
            auto address=reinterpret_cast<uintptr_t>(e);
            if(address>=reinterpret_cast<uintptr_t>(r->poolBegin) && address<reinterpret_cast<uintptr_t>(r->poolEnd)) {
                std::memcpy(e,&r->freeList,sizeof(r->freeList)); r->freeList=e;
            } else CallGameFunction<void,0x004e3f70>(e);
        }
        --r->count;
    }
    return old-r->count;
}
}

#if defined(_M_IX86)
namespace ts3::anim::b01::hooks {
float __fastcall Weight(Node* n,void*) {return b01::Weight(n);}
uintptr_t __fastcall SetWeight(Node* n,void*,float w) {return b01::SetWeight(n,w);}
bool __cdecl WriteNode(Node* n,Cursor* s) {return b01::WriteNode(n,s);}
bool __cdecl ReadNode(void* h,Node** n,Cursor* s) {return b01::ReadNode(h,n,s);}
void __fastcall Load(Node* n,void*,Cursor* s) {b01::Load(n,s);}
void __fastcall Reset(Node* n,void*) {b01::Reset(n);}
uint32_t __fastcall Type(Node* n,void*) {return b01::Type(n);}
Node* __fastcall Query(Node* n,void*,uint32_t k) {return b01::Query(n,k);}
uintptr_t __fastcall Attach(Node* n,void*,Node* c) {return b01::Attach(n,c);}
uintptr_t __fastcall Detach(Node* n,void*,Node* c) {return b01::Detach(n,c);}
Node* __fastcall Construct(Node* n,void*,void* h) {return b01::Construct(n,h);}
Node* __fastcall Destroy(Node* n,void*,uint32_t f) {return b01::Destroy(n,f);}
void __fastcall Save(Node* n,void*,Cursor* s) {b01::Save(n,s);}
void* __cdecl Find(uint32_t k) {return b01::Find(k);}
uint32_t __fastcall Erase(Registry* r,void*,const uint32_t* k) {return b01::Erase(r,k);}
}
#endif
