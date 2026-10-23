#include "ts3/audio/m06_B01.h"
namespace ts3::audio::b01 {
thread_local Boundaries boundaries;
namespace {
void Lock(Manager* m) {
    if (boundaries.lock) boundaries.lock(m);
    else {
#if defined(_M_IX86)
        CallGameMethod<int32_t,0x004E16F0>(m->lock,reinterpret_cast<void*>(0x00F98090));
#else
        AbortNotRewritten("m06 B01 lock");
#endif
    }
}
int32_t Unlock(Manager* m) {
    if (boundaries.unlock) return boundaries.unlock(m);
    return CallGameMethod<int32_t,0x004E17B0>(m->lock);
}
void* Allocate(size_t n) {
    if (boundaries.allocate) return boundaries.allocate(n);
    return CallGameFunction<void*,0x00401EF0>(n);
}
uint32_t Signal(Manager* m) {
    if (boundaries.signal) return boundaries.signal(m,m->signal);
    return CallGameMethod<uint32_t,0x00401230>(m->event,m->signal);
}
void Retire(Manager* m,Slot* s) {
    if (boundaries.retire) boundaries.retire(m,s);
    else CallGameMethod<void,0x004028D0>(m,s);
}
void Discard(Manager* m,uint32_t n) {
    if (boundaries.discard) boundaries.discard(m,n);
    else CallGameMethod<void,0x00402780>(m,n);
}
Slot* Lookup(Manager* m,uint32_t id) {
    uint32_t i=id & 255;
    if (static_cast<int32_t>(i)>=m->slotCount) return nullptr;
    Slot* s=m->slots+i;
    return s->id==id && s->state!=0 ? s : nullptr;
}
Reader* Numbered(Manager* m,uint32_t n) {
    Reader* r=m->readers;
    while (--n) r=r->next;
    return r;
}
}
uint32_t* Consume(Reader* r) {
    Manager* m=r->manager;
    Lock(m);
    if (!r->remaining) { Unlock(m); return nullptr; }
    Block* b=r->block;
    b->state=1;
    r->remaining-=b->size;
    m->slots[b->slot & 255].remaining-=b->size;
    if (static_cast<int32_t>(r->remaining)>0) {
        Block* next=b;
        do { next=next->next; } while (next->reader!=r->id || next->state!=0);
        r->block=next;
    } else r->block=nullptr;
    Unlock(m);
    return &b->slot;
}
uint32_t Remaining(Handle* h,uint32_t id) {
    Slot* s=Lookup(h->manager,id);
    return s ? s->remaining : 0;
}
uint32_t Mode(Handle* h) { return h->manager->mode; }
uint32_t Finished(Reader* r) { return r->manager->mode==0 && r->remaining==0 ? 1 : 0; }
uint32_t PositionSlot(Reader* r) {
    if (!r->remaining) return 0;
    Manager* m=r->manager;
    Lock(m);
    uint32_t p=r->block->position, start=m->consumed;
    Slot* s=m->head;
    for (Slot* next=s->next; next && next->state!=1; next=s->next) {
        uint32_t end=next->position;
        if (start<=end ? (p>=start && p<end) : (p>=start || p<end)) break;
        s=next; start=end;
    }
    Unlock(m);
    return s->id;
}
uint32_t State(Handle* h,uint32_t id) {
    Slot* s=Lookup(h->manager,id);
    return s ? s->state : 0;
}
uint64_t Stamp(Handle* h) { return h->manager->stamp; }
Reader* Create(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f) {
    void* storage=Allocate(sizeof(Manager));
    Manager* m=nullptr;
    if (storage) {
        if (boundaries.construct) m=boundaries.construct(storage,a,b,c,d,e,f);
        else m=CallGameMethod<Manager*,0x00403030>(static_cast<Manager*>(storage),a,b,c,d,e,f);
    }
    return m->readers;
}
Request* Enqueue(Handle* h,uint32_t reader,void* a,void* b) {
    Manager* m=h->manager;
    Request* r=m->requests;
    // EA dereferences the head before testing it for null.
    if (!r->a) {
        if (r==m->requestTail) m->requests=m->requestTail=nullptr;
        else m->requests=r->next;
        --m->requestCount;
    } else r=static_cast<Request*>(Allocate(sizeof(Request)));
    r->next=nullptr; r->a=a; r->b=b; r->reader=reader;
    Request* tail=m->requestTail;
    ++m->requestCount; m->requestTail=r;
    if (tail) tail->next=r; else m->requests=r;
    return r;
}
uint32_t Threshold(Handle* h,int32_t threshold) {
    Manager* m=h->manager;
    int32_t old=m->threshold;
    m->threshold=threshold;
    bool now=static_cast<int32_t>(m->produced)<threshold;
    bool before=static_cast<int32_t>(m->produced)<old;
    uint32_t result=(m->produced & 0xFFFFFF00u) | uint32_t(before);
    if (before!=now) {
        m->below=uint8_t(now);
        if (now && m->mode==1) return Signal(m);
    }
    return result;
}
Reader* AddReader(Handle* h) {
    Manager* m=h->manager;
    Reader* r=static_cast<Reader*>(Allocate(sizeof(Reader)));
    r->next=nullptr; r->manager=m; r->id=m->readerCount+1;
    r->remaining=0; r->block=nullptr;
    Reader* tail=m->readerTail;
    ++m->readerCount; m->readerTail=r;
    if (tail) tail->next=r; else m->readers=r;
    return r;
}
int32_t Fill(Manager* m) {
    if (m->cursor>=m->end) return 1;
    Slot* s=m->current;
    while (s->state!=4) {
        uint32_t count=0,reader=1;
        int32_t status=0;
        for (Request* r=m->requests; r; r=r->next) {
            status=s->produce(m->cursor,m->end-m->cursor,s->id,s->context,r->a,r->b,&count);
            if (status) { reader=r->reader; break; }
        }
        if (!status) {
            if (!m->current->eof) return 1;
            status=2; reader=1; count=m->end-m->cursor;
        }
        Lock(m);
        if (s->state==4) { Unlock(m); return 1; }
        Block* b=static_cast<Block*>(Allocate(sizeof(Block)));
        b->next=nullptr; b->state=0; b->slot=s->id;
        b->size=count; b->position=m->cursor; b->reader=reader;
        Block* tail=m->blockTail;
        ++m->blockCount; m->blockTail=b;
        if (tail) tail->next=b; else m->blocks=b;
        Reader* r=Numbered(m,reader);
        r->remaining+=count; s->remaining+=count;
        if (r->remaining==count) r->block=b;
        uint32_t old=m->produced;
        m->cursor+=count; m->produced+=count;
        if (static_cast<int32_t>(old)<m->threshold && static_cast<int32_t>(m->produced)>=m->threshold) m->below=0;
        b->state=0;
        Unlock(m);
        if (status==2) return status;
    }
    return 0;
}
int32_t Cancel(Handle* h,uint32_t id) {
    Manager* m=h->manager;
    Lock(m);
    Slot* s=Lookup(m,id);
    if (s && s->state!=4) {
        if (s->state==1) { Retire(m,s); return Unlock(m); }
        s->state=4;
        bool changed=false;
        for (Block* b=m->blocks; b; b=b->next) {
            if (b->slot==s->id && b->state==0) {
                Numbered(m,b->reader)->remaining-=b->size;
                Discard(m,b->size); b->state=2; changed=true;
            }
        }
        if (changed) for (Reader* r=m->readers; r; r=r->next) {
            if (static_cast<int32_t>(r->remaining)<=0) r->block=nullptr;
            else if (r->block->state!=0) {
                Block* b=m->blocks;
                while (b->reader!=r->id || b->state!=0) b=b->next;
                r->block=b;
            }
        }
    }
    return Unlock(m);
}
int32_t Stop(Handle* h) {
    Manager* m=h->manager;
    if (!m->active) return 0;
    while (m->active->state==1 || m->active->state==2) Cancel(h,m->active->id);
    while (m->head!=m->current) Retire(m,m->head);
    m->current->state=4;
    for (Reader* r=m->readers; r; r=r->next) r->remaining=0;
    uint32_t snapshot=m->produced;
    Lock(m);
    uint32_t old=m->produced;
    m->produced-=snapshot;
    Unlock(m);
    if (static_cast<int32_t>(old)>=m->threshold && static_cast<int32_t>(m->produced)<m->threshold) {
        m->below=1;
        if (m->mode==1) Signal(m);
    }
    Lock(m);
    for (Block* b=m->blocks; b; b=b->next) b->state=2;
    if (m->mode==2) m->mode=0;
    return Unlock(m);
}
}
