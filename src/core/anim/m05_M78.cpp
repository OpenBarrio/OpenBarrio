#include "ts3/anim/m05_M78.h"
#include <cstring>
namespace ts3::anim::m78 {
uint32_t AssignId(Id* self, const Target* target)
{
    *self = target ? target->id : Id{0,0};
    return self->low;
}
void* Rebind(State* self, void* key, Target* target)
{
    void* result = key;
    if (key) {
        if (self->key == key) {
            self->target = target;
            if (target) self->saved = target->id;
        }
        result = self->gateBegin;
        if (self->gateBegin != self->gateEnd && self->begin != self->end) {
            self->flags |= 3;
            for (Row* row=self->begin; row!=self->end; ++row) {
                Record* record=row->begin;
                for (; record!=row->end; ++record) {
                    void* match = record->index >= 0 ? self->lookup[record->index] : nullptr;
                    if (match == key) {
                        record->current = target ? target->id : Id{0,0};
                        if (target) record->saved = target->id;
                    }
                }
                result = record;
            }
        }
    }
    return result;
}
namespace {
Target* Get(Id* id) { return CallGameMethod<Target*,0x00935830>(id); }
void Word(Writer* writer, uint32_t value) {
    uint8_t status=CallGameFunction<uint8_t,0x004DDBD0>(writer->stream,value,writer->mode);
    CallGameMethod<void,0x007D77C0>(writer,status);
}
void Identifier(Writer* writer, Target* target) {
    Id id=target ? target->id : Id{0,0};
    uint8_t status=CallGameFunction<uint8_t,0x004DDD20>(writer->stream,id.low,id.high,writer->mode);
    CallGameMethod<void,0x007D77C0>(writer,status);
}
}
uint8_t Save(State* self, Writer* writer, uint32_t unused)
{
    (void)unused;
    Word(writer,self->flags);
    Word(writer,static_cast<uint32_t>(self->end-self->begin));
    Word(writer,0xffffffffu);
    Identifier(writer,self->target);
    for (Row* row=self->begin; row!=self->end; ++row) {
        Word(writer,static_cast<uint32_t>(row->end-row->begin));
        for (Record* record=row->begin; record!=row->end; ++record) {
            Word(writer,record->channel);
            Word(writer,record->kind);
            Word(writer,record->flags);
            // The original repeats the lookup after a successful first lookup.
            Identifier(writer,Get(&record->current) ? Get(&record->current) : nullptr);
        }
    }
    return writer->status;
}
uint8_t Resolve(State* self)
{
    void* registry=CallGameFunction<void*,0x00938B50>();
    if (!(self->flags & 4)) {
        if (!self->target) {
            if (!self->saved.low && !self->saved.high) return 0;
            self->target=CallGameMethod<Target*,0x00939560>(registry,self->saved.low,self->saved.high);
            if (!self->target) {
                self->flags |= 4;
                return 0;
            }
        }
        size_t completed=0;
        for (Row* row=self->begin; row!=self->end; ++row) {
            size_t resolved=0;
            for (Record* record=row->begin; record!=row->end; ++record) {
                if (Get(&record->current) || (!record->saved.low && !record->saved.high)) {
                    ++resolved;
                } else {
                    Target* target=CallGameMethod<Target*,0x00939560>(registry,record->saved.low,record->saved.high);
                    AssignId(&record->current,target);
                    if (Get(&record->current)) ++resolved;
                }
            }
            if (resolved == static_cast<size_t>(row->end-row->begin)) ++completed;
        }
        if (completed == static_cast<size_t>(self->end-self->begin)) self->flags |= 4;
    }
    return 1;
}
Bytes* AssignBytes(Bytes* self, const char* begin, const char* end)
{
    const size_t size=self->end-self->begin;
    const size_t count=end-begin;
    if (count > size) {
        std::memmove(self->begin,begin,size);
        CallGameMethod<void*,0x0099C9C0>(self,begin+(self->end-self->begin),end);
    } else {
        std::memmove(self->begin,begin,count);
        uint8_t* newEnd=self->begin+count;
        uint8_t* oldEnd=self->end;
        if (newEnd != oldEnd) {
            std::memmove(newEnd,oldEnd,1);
            self->end += newEnd-oldEnd;
        }
    }
    return self;
}
Record* CopyRecords(const Record* begin, const Record* end, Record* destination)
{
    if (begin == end) return destination;
    do {
        CallGameMethod<void*,0x00568D30>(destination,begin);
        ++begin;
        ++destination;
    } while (begin != end);
    return destination;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m78::hooks {
uint32_t __fastcall AssignId(Id* p,void*,const Target* t) { return m78::AssignId(p,t); }
void* __fastcall Rebind(State* p,void*,void* k,Target* t) { return m78::Rebind(p,k,t); }
uint8_t __fastcall Save(State* p,void*,Writer* w,uint32_t u) { return m78::Save(p,w,u); }
uint8_t __fastcall Resolve(State* p,void*) { return m78::Resolve(p); }
Bytes* __fastcall AssignBytes(Bytes* p,void*,const char* b,const char* e) { return m78::AssignBytes(p,b,e); }
}
#endif
