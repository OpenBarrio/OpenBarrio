#include "ts3/resource/m03_B03.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"
#include <cstring>
namespace ts3::resource {
B03Boundary& B03Dependencies() { static B03Boundary b; return b; }
namespace {
void* Destroy(void* p,uint32_t flags) {
    if(auto f=B03Dependencies().destroy) return f(p,flags);
    return CallGameMethod<void*,0x00737040>(p,flags);
}
}
void* B03DestroyObserver(void* p,uint32_t flags) {
    return Destroy(static_cast<uint8_t*>(p)-0x1D8,flags);
}
void* B03DestroyManager(void* p,uint32_t flags) {
    return Destroy(static_cast<uint8_t*>(p)-8,flags);
}
void* B03Open(void* p,const char16_t* path) {
    if(auto f=B03Dependencies().open) return f(p,path,0);
    return static_cast<ts3::resourcesystem::ShadowDirectoryManager*>(p)->OpenShadowDatabase(path,0);
}
void B03Swap(void* data,uint32_t count) {
    auto* p=static_cast<uint8_t*>(data);
    for(uint32_t i=0;i<count;++i,p+=4) {
        uint32_t v; std::memcpy(&v,p,4);
        v=(v>>24)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|(v<<24);
        std::memcpy(p,&v,4);
    }
}
uintptr_t B03Load(void** owner) {
#if defined(_M_IX86)
    const char* label=reinterpret_cast<const char*>(uintptr_t{0x00FFEB64});
#else
    const char* label="ResourceSystem/MemoryStream";
#endif
    void* memory=CallGameFunction<void*,0x004E3F90>(uint32_t{0x2C},label,0,0,nullptr,0);
    if(!memory) return 0;
    void* stream=CallGameMethod<void*,0x004DF0C0>(memory,0,0,0);
    if(!stream) return 0;
    CallGameVirtual<uintptr_t,1>(stream);
    CallGameMethod<void,0x004DEAC0>(stream,1,1.0f);
    const auto size=CallGameVirtual<uint32_t,7>(*owner);
    const auto loaded=CallGameVirtual<uintptr_t,8>(stream,size);
    if(!static_cast<uint8_t>(loaded)) {
        return CallGameVirtual<uintptr_t,2>(stream)&~uintptr_t{0xFF};
    }
    CallGameMethod<void,0x004DEAC0>(stream,1,0.0f);
    void* state=*reinterpret_cast<void**>(static_cast<uint8_t*>(stream)+sizeof(void*));
    void* data=state ? reinterpret_cast<void**>(state)[2] : nullptr;
    const auto read=CallGameVirtual<uintptr_t,12>(*owner,data,size);
    if(!read) {
        return CallGameVirtual<uintptr_t,2>(stream)&~uintptr_t{0xFF};
    }
    CallGameMethod<void,0x007C8D00>(owner,stream);
    return (CallGameVirtual<uintptr_t,2>(stream)&~uintptr_t{0xFF})|1;
}
void* B03_ABI HookB03Observer(void* p,void*,uint32_t f) { return B03DestroyObserver(p,f); }
void* B03_ABI HookB03Manager(void* p,void*,uint32_t f) { return B03DestroyManager(p,f); }
void* B03_ABI HookB03Open(void* p,void*,const char16_t* s) { return B03Open(p,s); }
}
