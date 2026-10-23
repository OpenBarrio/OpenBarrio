#include "ts3/resource/m03_G07.h"
namespace ts3::resource::m03::g07 {
namespace {
uintptr_t Address(unsigned address) {
#if defined(_M_IX86)
    return address;
#else
    (void)address; NotRewritten("m03 G07 locale static data/mutex bridge");
#endif
}
}
bool Reload(Owner* self) {
    CallGameMethod<void,0x0059AF00>(self);
    auto manager=CallGameFunction<void*,0x004AFD20>();
    if(!manager) return false;
    List replacement{};
    replacement.next=reinterpret_cast<Node*>(&replacement);
    replacement.previous=reinterpret_cast<Node*>(&replacement);
    replacement.name=reinterpret_cast<const char*>(Address(0xFD9D9C));
    struct Vector { Key* begin; Key* end; Key* capacity; const char* name; } keys{nullptr,nullptr,nullptr,reinterpret_cast<const char*>(Address(0xFD9D74))};
    auto count=CallGameVirtual<uint32_t,8>(manager,&keys,&self->fields[3],0u);
    if(count>0) {
        for(auto key=keys.begin;key!=keys.end;++key) {
            uint32_t group=key->words[1]&0xFF000000u;
            // Only the first value word is initialized by EA; the spare word is
            // indeterminate at insertion, then copied verbatim with the node.
            Payload payload{*key,0,0};
            if(self->primary[0]==0 && group==self->primary[1]) {
                CallGameMethod<void,0x005BE2D0>(&self->tables,&self->tables,&payload);
                CallGameMethod<void,0x0059B190>(self,key);
            } else if(self->fallback[0]==0 && group==self->fallback[1]) {
                CallGameMethod<void,0x005BE2D0>(&replacement,&replacement,&payload);
            }
        }
    }
    auto sentinel=reinterpret_cast<Node*>(&replacement);
    for(auto source=replacement.next;source!=sentinel;source=source->next) {
        auto node=CallGameFunction<Node*,0x004E3F90>(sizeof(Node),self->tables.name,0u,0u,reinterpret_cast<const char*>(Address(0xF98980)),0xE7u);
        // EA does not check allocation failure before linking the new node.
        node->payload=source->payload;
        node->next=reinterpret_cast<Node*>(&self->tables);
        node->previous=self->tables.previous;
        self->tables.previous->next=node;
        self->tables.previous=node;
        CallGameMethod<void,0x0059B190>(self,&source->payload.key);
    }
    if(keys.begin) CallGameFunction<void,0x004E3F70>(keys.begin);
    auto node=replacement.next;
    while(node!=sentinel) {
        auto next=node->next;
        CallGameFunction<void,0x004E3F70>(node);
        node=next;
    }
    return true;
}
uintptr_t ResourceChanged(uintptr_t ignored,const Key* key,Owner* self) {
    (void)ignored;
    auto mutex=reinterpret_cast<void*>(Address(0x11CE2D8));
    CallGameMethod<void,0x004E16F0>(mutex,reinterpret_cast<const char*>(Address(0xFD98B4)));
    if(self) CallGameMethod<void,0x0059B190>(self,key);
    return CallGameMethod<uintptr_t,0x004E17B0>(mutex);
}
#if defined(_M_IX86)
bool __fastcall HookReload(Owner* self,void*) { return Reload(self); }
#endif
}
