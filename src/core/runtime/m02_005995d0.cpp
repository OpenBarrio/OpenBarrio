#include "ts3/runtime/m02_005995d0.h"
namespace ts3::runtime::m02 {
uintptr_t DrainScheduler(Scheduler* scheduler) {
    auto head = &scheduler->queues[10];
    for (;;) {
        auto node = head->next;
        while (node != head && node == scheduler->reservedJob) node = node->next;
        if (node == head) break;
        CallGameVirtual<void,6>(scheduler, reinterpret_cast<Job*>(node));
        // The callback may change the list, the reserved job, or the vtable.
        // Restart at the head, not at the old node's successor.
    }
    for (;;) {
        auto result = CallGameVirtual<uintptr_t,9>(scheduler, scheduler->reservedJob, static_cast<void*>(nullptr), uint32_t(1));
        if (static_cast<uint8_t>(result) == 0) return result;
    }
}
}
