#include "ts3/runtime/m02_004e1960.h"
namespace ts3::runtime::m02 {
namespace {
uint32_t Tick() { return CallGameFunction<uint32_t,0x004e1330>(); }
void Pause() { uint32_t delay=10; CallGameFunction<void,0x004e1320>(&delay); }
bool Empty(const ThreadPool* p) {
    using namespace wait1960;
    return reinterpret_cast<WorkNode*>(p->work.sentinel.next)->prev == reinterpret_cast<WorkNode*>(p->work.sentinel.prev);
}
}
int32_t PoolPredicateWait(ThreadPool* p, uint32_t identity, uint32_t predicate, const uint32_t* deadline) {
    if(identity == UINT32_MAX) {
        if(predicate == 0) return 0;
        if(predicate == 1) {
            while(p->busy != 0) {
                if(Tick() >= *deadline) break;
                Pause();
            }
            return p->busy == 0 ? 0 : -2;
        }
        while(p->busy != 0 || !Empty(p)) {
            if(Tick() >= *deadline) break;
            Pause();
        }
        return p->busy == 0 && Empty(p) ? 0 : -2;
    }
    for(;;) {
        bool found=false;
        // EA locks with the shared infinite deadline, ignoring the lock result.
#if defined(_M_IX86)
        const auto infinite=reinterpret_cast<const uint32_t*>(0x00fb8960);
#else
        const uint32_t infinity=UINT32_MAX; const auto infinite=&infinity;
#endif
        CallGameMethod<int32_t,0x004e16f0>(&p->mutex,infinite);
        using namespace wait1960;
        auto workEnd=reinterpret_cast<WorkNode*>(p->work.sentinel.prev);
        for(auto n=reinterpret_cast<WorkNode*>(p->work.sentinel.next)->prev; n!=workEnd; n=n->prev) {
            if(reinterpret_cast<uintptr_t>(n->work.function) == identity) found=true;
        }
        auto threadEnd=reinterpret_cast<ThreadNode*>(p->threads.sentinel.prev);
        for(auto n=reinterpret_cast<ThreadNode*>(p->threads.sentinel.next)->prev; n!=threadEnd; n=n->prev) {
            if(n->info->running != 0 && reinterpret_cast<uintptr_t>(n->info->work.function) == identity) found=true;
        }
        CallGameMethod<int32_t,0x004e17b0>(&p->mutex);
        if(!found) return 0;
        if(Tick() >= *deadline) return -2;
        Pause();
    }
}
}
