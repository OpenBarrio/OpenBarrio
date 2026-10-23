#include "ts3/anim/m05_M126.h"
#include <cstddef>
namespace ts3::anim::m126_impl {
namespace {
void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(0x01026E98));
#else
    AbortNotRewritten("M126 original style table"); return nullptr;
#endif
}
}
Style* Construct(Style* self, Link* link) {
    CallGameMethod<void,0x008396F0>(self,link);
    self->table=Table();
    return self;
}
uint32_t Type(Style*) {
    return 0xB4CEC7A5u;
}
void* Query(Style* self,uint32_t id) {
    if(id==0x34CEC21Cu) return self;
    if(id==0xB4CEC7A5u) return self;
    return nullptr;
}
uintptr_t Destroy(Style* self) {
    self->table=Table();
    return CallGameMethod<uintptr_t,0x008396B0>(self);
}
Style* Delete(Style* self,uint32_t flags) {
    Destroy(self);
    if(flags & 1u) CallGameFunction<void,0x004E3F70>(static_cast<void*>(self));
    return self;
}
void* Find(Style* self) {
    auto* end=&self->link;
    for(auto* node=end->next;node!=end;node=node->next) {
        // A null link still leads to a null virtual call in EA, not a skipped entry.
        void* object=node ? reinterpret_cast<unsigned char*>(node)-sizeof(void*) : nullptr;
        void* result=CallGameVirtual<void*,1>(object,0x94D79AD1u);
        if(result) return result;
    }
    return nullptr;
}
}
