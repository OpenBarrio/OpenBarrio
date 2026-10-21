#include "ts3/runtime/m02_0059a650.h"
namespace ts3::runtime::m02 {
Link* Recycle(Scheduler* scheduler, RecycleJob* job) {
    // Transition helper owns graph cleanup; it is not implemented in this unit.
    CallGameMethod<void,0x0059a540>(scheduler,job,uint32_t(0));
    void* retained=job->retained;
    if(retained) {
        job->retained=nullptr;
        CallGameMethod<void,0x004e17b0>(&scheduler->mutex);
        CallGameVirtual<void,1>(retained);
        CallGameMethod<void,0x004e16f0>(&scheduler->mutex,
            reinterpret_cast<void*>(uintptr_t(0xfd9744)));
    }
    job->links[1].next=nullptr;
    job->links[1].prev=nullptr;
    Link& node=job->links[0];
    node.prev->next=node.next;
    node.next->prev=node.prev;
    Link* head=&scheduler->queues[2];
    node.prev=head->prev;
    node.next=head;
    head->prev=&node;
    node.prev->next=&node;
    job->state=0;
    return head;
}
}
