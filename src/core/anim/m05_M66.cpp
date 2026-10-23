#include "ts3/anim/m05_M66.h"
#include "ts3/anim/m05_M23.h"
namespace ts3::anim::m66 {
namespace {
void* Table(unsigned index) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(0x00fc25a4u+8u*index));
#else
    static void* tables[4][2]{};
    return tables[index];
#endif
}
void* DestroyFactory(void* self,uint32_t flags,unsigned index,uint32_t key) {
    *static_cast<void**>(self)=Table(index);
    // 00565D30 only consumes the first stack argument (the key), not self.
    m23::Unregister(key);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
}
void* DestroyFirst(void* self,uint32_t flags) { return DestroyFactory(self,flags,0,0x075e4c89u); }
void* DestroySecond(void* self,uint32_t flags) { return DestroyFactory(self,flags,1,0x0664fb68u); }
void* DestroyThird(void* self,uint32_t flags) { return DestroyFactory(self,flags,2,0x075e3ecbu); }
void* DestroyFourth(void* self,uint32_t flags) { return DestroyFactory(self,flags,3,0x0681c688u); }
// ResourceBaseDestroySecondary twin, with the adjustment and target adapted.
void* DestroySecondary(void* self,uint32_t flags) {
    return CallGameMethod<void*,0x00570fa0>(static_cast<unsigned char*>(self)-3*sizeof(void*),flags);
}
// ResourceManDestroy twin: verified evaluator reset and this allocator differ.
m07::Evaluator* DestroyEvaluator(m07::Evaluator* self,uint32_t flags) {
    m07::Reset(self,nullptr);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m66::hooks {
void* __fastcall DestroyFirst(void* p,void*,uint32_t f) {return m66::DestroyFirst(p,f);}
void* __fastcall DestroySecond(void* p,void*,uint32_t f) {return m66::DestroySecond(p,f);}
void* __fastcall DestroyThird(void* p,void*,uint32_t f) {return m66::DestroyThird(p,f);}
void* __fastcall DestroyFourth(void* p,void*,uint32_t f) {return m66::DestroyFourth(p,f);}
void* __fastcall DestroySecondary(void* p,void*,uint32_t f) {return m66::DestroySecondary(p,f);}
m07::Evaluator* __fastcall DestroyEvaluator(m07::Evaluator* p,void*,uint32_t f) {return m66::DestroyEvaluator(p,f);}
}
#endif
