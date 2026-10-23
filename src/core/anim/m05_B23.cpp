#include "ts3/anim/m05_B23.h"
#include <initializer_list>
namespace ts3::anim::b23 {
namespace {
bool Less(const Key& a,const Key& b) {
    for (unsigned i : {0u,1u,3u,2u}) if(a.word[i]!=b.word[i]) return a.word[i]<b.word[i];
    return false;
}
Node* AsNode(Link* p) { return reinterpret_cast<Node*>(p); }
template<unsigned A> void* Global() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    NotRewritten("B23 original data");
#endif
}
}
void* __fastcall Query(Resource* self,void*,uint32_t id) {
    return id==0x6b20c4f3u || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu ? self : nullptr;
}
void* __fastcall Name(Resource* self,void*) { return self->view ? self->view->name : nullptr; }
Samples* __fastcall Data(Resource* self,void*) { return self->view ? self->view->samples : nullptr; }
double __fastcall Duration(Resource* self,void*) {
    auto* samples=Data(self,nullptr);
    return samples ? (int32_t(samples->frames)-1)*double(samples->interval) : 0.0;
}
const char* __fastcall ClipName(Resource* self,void*) {
    auto* samples=Data(self,nullptr);
    if(samples && samples->nameOffset!=-1) {
        auto address=reinterpret_cast<uintptr_t>(samples)+intptr_t(samples->nameOffset);
        if(address) return reinterpret_cast<const char*>(address);
    }
    return static_cast<const char*>(Global<0x00FB8AA8>());
}
Link** __fastcall InsertAt(Tree* self,void*,Link** out,Link* position,const Value* value,uint8_t forceLeft) {
    const int direction=forceLeft || position==&self->anchor || Less(value->key,AsNode(position)->value.key) ? 0 : 1;
    auto* node=CallGameFunction<Node*,0x004E3F90>(sizeof(Node),self->allocator,0,0,Global<0x00F98980>(),0xe7);
    if(reinterpret_cast<uintptr_t>(node)!=uintptr_t(0)-offsetof(Node,value)) node->value=*value;
    CallGameFunction<void,0x004E0C40>(&node->links,position,&self->anchor,direction);
    ++self->count;
    *out=&node->links;
    return out;
}
void* __fastcall DeleteBase(void* self,void*,uint32_t flags) {
    CallGameMethod<void,0x00731CF0>(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
uintptr_t __fastcall Clear(Resource* self,void*) {
    CallGameMethod<void,0x004E16F0>(Global<0x011CD8F8>(),Global<0x00FC303C>());
    auto* end=&self->bindings.anchor;
    for(auto* node=end->left;node!=end;node=CallGameFunction<Link*,0x004E0B20>(node)) {
        if(AsNode(node)->value.binding) {
            auto* manager=*static_cast<void**>(Global<0x011CB864>());
            CallGameMethod<void,0x004E5920>(manager,AsNode(node)->value.binding);
            AsNode(node)->value.binding=nullptr;
        }
    }
    CallGameMethod<void,0x006A0850>(&self->bindings,self->bindings.anchor.parent);
    end->right=end; end->left=end; end->parent=nullptr; end->color=0; self->bindings.count=0;
    CallGameMethod<void,0x004E17B0>(Global<0x011CD8F8>());
    self->view=nullptr;
    uintptr_t result=CallGameFunction<uintptr_t,0x004E3F70>(self->raw);
    self->raw=nullptr;
    if(self->ik) result=CallGameVirtual<uintptr_t,0>(self->ik,1);
    self->ik=nullptr;
    return result;
}
uintptr_t __fastcall Unload(Resource* self,void*) { return (Clear(self,nullptr)&~uintptr_t(255))|1; }
Result* __fastcall Insert(Tree* self,void*,Result* out,const Value* value,uintptr_t) {
    auto* node=self->anchor.parent;
    auto* position=&self->anchor;
    bool left=true;
    while(node) {
        left=Less(value->key,AsNode(node)->value.key);
        position=node;
        node=left?node->left:node->right;
    }
    auto* predecessor=position;
    if(left) {
        if(position==self->anchor.left) {
            InsertAt(self,nullptr,&out->node,position,value,0); out->inserted=1; return out;
        }
        predecessor=CallGameFunction<Link*,0x004E0B60>(position);
    }
    if(Less(AsNode(predecessor)->value.key,value->key)) {
        InsertAt(self,nullptr,&out->node,position,value,0); out->inserted=1;
    } else { out->node=predecessor; out->inserted=0; }
    return out;
}
uint32_t __fastcall Type(Resource*,void*) { return 0x6b20c4f3u; }
Link** __fastcall InsertHint(Tree* self,void*,Link** out,Link* hint,const Value* value,uintptr_t unused) {
    auto* right=self->anchor.right;
    if(hint==right || hint==&self->anchor) {
        if(self->count && Less(AsNode(right)->value.key,value->key)) return InsertAt(self,nullptr,out,right,value,0);
    } else {
        auto* next=CallGameFunction<Link*,0x004E0B20>(hint);
        if(Less(AsNode(hint)->value.key,value->key) && Less(value->key,AsNode(next)->value.key)) {
            // 4A3CF0 is the exact unsigned 128-bit key comparator.
            if(hint->right) return InsertAt(self,nullptr,out,next,value,1);
            return InsertAt(self,nullptr,out,hint,value,0);
        }
    }
    Result result;
    Insert(self,nullptr,&result,value,unused&~uintptr_t(255));
    *out=result.node;
    return out;
}
static void** __fastcall BindingImpl(Tree* self,void*,const Key* key,uint32_t stackTail) {
    Link* node=nullptr;
    CallGameMethod<Link**,0x00C5DD00>(self,&node,key);
    if(node==&self->anchor || Less(*key,AsNode(node)->value.key)) {
        Value value{*key,nullptr,stackTail};
        InsertHint(self,nullptr,&node,node,&value,0);
    }
    return &AsNode(node)->value.binding;
}
#if defined(_M_IX86)
// EA leaves the last dword of its 24-byte temporary uninitialized. It copies
// the incoming stack word at entry ESP-4 into the new node. Capture that word
// before any compiler prologue can overwrite it; no undefined C++ loads.
__declspec(naked) void** __fastcall Binding(Tree*,void*,const Key*) {
    __asm {
        mov eax,dword ptr [esp-4]
        push eax
        push dword ptr [esp+8]
        call BindingImpl
        ret 4
    }
}
#else
void** __fastcall Binding(Tree* self,void*,const Key* key) {
    return BindingImpl(self,nullptr,key,0);
}
#endif
}
