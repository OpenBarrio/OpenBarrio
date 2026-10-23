#include "ts3/resource/m03_G09.h"
#include <cstring>
namespace ts3::resource::m03::g09 {
#if defined(_M_IX86)
namespace {
// The original reads the global allocator cell at every alloc/free site.
void* Allocator() { return *reinterpret_cast<void**>(static_cast<uintptr_t>(0x011CB864)); }
void* Allocate(uint32_t size) { return CallGameMethod<void*,0x004E69D0>(Allocator(),size,1u); }
void Release(void* pointer) { CallGameMethod<void,0x004E5920>(Allocator(),pointer); }
// 0x0059F750 is a __fastcall whose edx formal is unused; the thiscall bridge passes the
// same three stack arguments the game pushes.
void InsertNode(Hash* hash,Node* node) {
    c04::InsertResult result;
    CallGameMethod<c04::InsertResult*,0x0059F750>(hash,&result,node,0u);
}
}
#endif
uintptr_t __fastcall ParseEntries(Table* self,void*,void* stream,uint32_t countLow,uint32_t countHigh,uint32_t encoding) {
#if defined(_M_IX86)
    CallGameMethod<void,0x0059F6C0>(self);
    const uint32_t token=CallGameVirtual<uint32_t,9>(stream,0u);
    // First pass: measure the UTF16 payload only; read failures are ignored and the
    // scratch buffer is a bare 0x1000 bytes reused for every entry.
    uint32_t units=0;
    for(uint32_t index=0;countHigh!=0u||index<countLow;++index) {
        uint64_t key=0;
        if(CallGameFunction<uint8_t,0x004DD660>(stream,&key,encoding)) {
            uint32_t length=0;
            CallGameFunction<uint8_t,0x004DD700>(stream,&length,encoding);
            units=length+units+1u;
            char16_t scratch[0x800];
            CallGameFunction<uint8_t,0x004DD580>(stream,scratch,length,1u);
        }
    }
    CallGameVirtual<void,10>(stream,token,0u);
    self->strings=Allocate(2u*units);
    self->nodes=Allocate(24u*countLow);
    // Second pass: fill the buffers and insert; the high count word makes the 64-bit
    // compare loop forever, and the running size starts from the 0x7BF8 table extent.
    uint32_t size=0x7bf8u;
    uint32_t counter=0,textUnits=0,nodeBytes=0;
    char16_t* strings=static_cast<char16_t*>(self->strings);
    auto setSize=[&]() -> uintptr_t {
        const uint32_t dataSize=size+2u*(units+12u*self->hash.count);
        return CallGameMethod<uintptr_t,0x00731CC0>(self,dataSize);
    };
    for(;countHigh!=0u||counter<countLow;++counter) {
        uint64_t key=0;
        if(CallGameFunction<uint8_t,0x004DD660>(stream,&key,encoding)) {
            // The original zero-extends the byte and compares against 0xFFFFFFFF, so its
            // 0xFF abort branches are dead: a failed length leaves the pre-zeroed 0 and a
            // failed string read still writes the terminator and inserts the node.
            uint32_t length=0;
            CallGameFunction<uint8_t,0x004DD700>(stream,&length,encoding);
            CallGameFunction<uint8_t,0x004DD580>(stream,strings+textUnits,length,1u);
            strings[textUnits+length]=0;
            auto* node=reinterpret_cast<Node*>(static_cast<uint8_t*>(self->nodes)+nodeBytes);
            node->low=static_cast<uint32_t>(key);
            node->high=static_cast<uint32_t>(key>>32);
            node->text=strings+textUnits;
            InsertNode(&self->hash,node);
            size+=2u*length+2u;
            textUnits+=length+1u;
        }
        nodeBytes+=24u;
    }
    return setSize();
#else
    (void)self; (void)stream; (void)countLow; (void)countHigh; (void)encoding;
    NotRewritten("m03_G09 ParseEntries");
#endif
}
uint8_t __fastcall FilterWrite(Table* self,void*,void* src,void* dst,const void* filter) {
#if defined(_M_IX86)
    if(!src) return 0;
    if(!dst) return 0;
    uint32_t empty[4]={0u,0u,0u,0u};
    CallGameMethod<void,0x00500A40>(empty,0);
    if(CallGameFunction<uint8_t,0x00500AC0>(filter,empty)) return 0;
    if(!CallGameMethod<uint8_t,0x0059FD00>(self,src)) return 0;
    // Local replacement hash. The original copies a trailing dword into the table's
    // `last` field from uninitialized stack; the deterministic zero here is masked in
    // the differential test and documented.
    struct Local { Hash hash; uint32_t last; } local{};
    std::memset(local.hash.buckets,0,0x7bbcu);
    local.hash.sentinel=reinterpret_cast<Node*>(static_cast<uintptr_t>(0xFFFFFFFFu));
    local.hash.count=0;
    void* nodeBuffer=Allocate(24u*self->hash.count);
    struct Begin { Node* node; Node** bucket; } begin;
    CallGameMethod<void,0x006FE1B0>(&self->hash,&begin);
    Node* node=begin.node;
    Node** bucket=begin.bucket;
    Node* sentinel=self->hash.sentinel;
    if(node!=sentinel) {
        uint8_t* write=static_cast<uint8_t*>(nodeBuffer)+8;
        for(;;) {
            uint32_t key[2]={node->low,node->high};
            if((self->flags&0x02000000u)!=0u &&
               !CallGameFunction<uint8_t,0x0099E570>(key,filter)) return 0;
            std::memcpy(write,key,8);
            *reinterpret_cast<const char16_t**>(write+8)=node->text;
            InsertNode(&local.hash,reinterpret_cast<Node*>(write-8));
            node=node->next;
            while(!node) { ++bucket; node=*bucket; }
            write+=24;
            if(node==sentinel) break;
        }
    }
    Release(self->nodes);
    std::memset(self->hash.buckets,0,0x7bbcu);
    self->hash.count=0;
    std::memcpy(&self->hash,&local,0x7bc8u);
    self->nodes=nodeBuffer;
    return CallGameMethod<uint8_t,0x0059F860>(self,dst)!=0;
#else
    (void)self; (void)src; (void)dst; (void)filter;
    NotRewritten("m03_G09 FilterWrite");
#endif
}
}