#include "ts3/anim/m05_M57.h"
#include "ts3/anim/m05_M22.h"
#include "ts3/anim/m05_M29.h"
#include "ts3/anim/m05_M32.h"
namespace ts3::anim::m57 {
// Constant-return twin of m01::FileType/MemoryType; only the immediate differs.
uint32_t Size() { return 0x48; }
void* __stdcall ConstructPair(void* host,void* storage) {
    if(!storage) return nullptr;
    return m22::ConstructPair(static_cast<m22::Evaluator*>(storage),host);
}
void* __stdcall ConstructScale(void* host,void* storage) {
    if(!storage) return nullptr;
    return m22::ConstructScale(static_cast<m22::Evaluator*>(storage),host);
}
static void* Allocate(size_t size) {
#if defined(_M_IX86)
    auto descriptor=reinterpret_cast<const void*>(uintptr_t(0x00fc24e8));
#else
    const void* descriptor=nullptr;
#endif
    return CallGameFunction<void*,0x004e3f90>(size,descriptor,0,0,0,0);
}
void* __stdcall CreateClip(void* host) {
    auto* storage=static_cast<m29::Clip*>(Allocate(sizeof(m29::Clip)));
    if(!storage) return nullptr;
    return m29::Construct(storage,host,0.0f);
}
void* __stdcall CreateController(void* host) {
    void* storage=Allocate(0x48);
    if(!storage) return nullptr;
    return CallGameMethod<void*,0x0056e420>(storage,host);
}
void* __stdcall CreatePair(void* host) {
    auto* storage=static_cast<m32::PairController*>(Allocate(sizeof(m32::PairController)));
    if(!storage) return nullptr;
    return m32::Construct(storage,host);
}
}
