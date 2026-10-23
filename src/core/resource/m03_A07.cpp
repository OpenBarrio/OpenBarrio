#include "ts3/resource/m03_A07.h"
#include <intrin.h>
#include <cstring>
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::resource::m03::a07 {
thread_local Clock clockProvider=nullptr;
namespace {
int32_t ClockNow(int64_t* out) {
 if(clockProvider) return clockProvider(out);
#if defined(_M_IX86)
 LARGE_INTEGER now; const int32_t result=QueryPerformanceCounter(&now); *out=now.QuadPart; return result;
#else
 (void)out; NotRewritten("A07 QPC provider");
#endif
}
uint32_t Hash(const uint32_t* k) { return (k[0]+k[1]+k[2]+k[3])%191u; }
bool Equal(const uint32_t* a,const uint32_t* b) { return a[0]==b[0] && a[2]==b[2] && a[3]==b[3] && a[1]==b[1]; }
void Advance(Iterator& i,int32_t n) {
 const int32_t index=static_cast<int32_t>(static_cast<uint32_t>(n)+static_cast<uint32_t>(i.current-i.begin));
 if(static_cast<uint32_t>(index)<32) { i.current+=n; return; }
 // EA's signed division expression implements floor(index / 32).
 const int32_t page=static_cast<int32_t>((static_cast<int64_t>(index)+0x1000000)/32-0x80000);
 i.block+=page; i.begin=*i.block; i.end=i.begin+32; i.current=i.begin+(index-page*32);
}
}
uintptr_t __fastcall Grow(Growth* p,void*) {
 const uint32_t old=p->capacity; uint32_t n=old&0x7fffffffu;
 if(old&0x80000000u) { n*=2; if(n>454) n=454; p->capacity=(old&0x80000000u)|n; }
 void* allocator=nullptr;
#if defined(_M_IX86)
 allocator=*reinterpret_cast<void**>(0x011CB864u);
#endif
 void* memory=CallGameMethod<void*,0x004E6A20>(allocator,n*36u+4u,4u,0u,0u);
 return CallGameMethod<uintptr_t,0x007CCEA0>(p,memory,4u,36u,4u,n);
}
int32_t __fastcall Stamp(LoadInfo* p,void*) {
 int64_t now; const int32_t result=ClockNow(&now); p->timestamp=now; return result;
}
double __fastcall Elapsed(LoadInfo* p,void*) {
 if(!p->timestamp) return 0.0;
 int64_t now; ClockNow(&now);
 const int64_t diff=static_cast<int64_t>(static_cast<uint64_t>(now)-static_cast<uint64_t>(p->timestamp));
 return CallGameFunction<double,0x004F3480>(4)*static_cast<double>(diff);
}
// Adapted from verified ResourceBaseSec::AddRef, changing only the field offset.
int32_t __fastcall AddRef(LoadInfo* p,void*) { return _InterlockedIncrement(reinterpret_cast<volatile long*>(&p->refs)); }
// Adapted from verified DebugObject::Release; LoadInfo uses slot2 and +90.
int32_t __fastcall Release(LoadInfo* p,void*) {
 const int32_t remaining=_InterlockedDecrement(reinterpret_cast<volatile long*>(&p->refs));
 if(!remaining) CallGameVirtual<void,2>(p,1);
 return remaining;
}
uint32_t __fastcall Erase(Table* p,void*,const Node* key) {
 const uint32_t old=p->count; Node** link=&p->buckets[Hash(key->key)];
 while(*link) { Node* node=*link; if(Equal(key->key,node->key)) { *link=node->next; --p->count; } else link=&node->next; }
 return old-p->count;
}
// Same copy/advance recipe as verified C4BA30, with 8-byte entries and 32 per block.
Iterator* __fastcall AdvanceCopy(const Iterator* p,void*,Iterator* out,int32_t n) {
 Iterator copy=*p; Advance(copy,n); *out=copy; return out;
}
Node* __stdcall FindNode(Node* p,const Node* key) { while(p && !Equal(key->key,p->key)) p=p->next; return p; }
Node* __stdcall FindKey(Node* p,const uint32_t* key,uintptr_t) { while(p && !Equal(p->key,key)) p=p->next; return p; }
uintptr_t __fastcall Produce(Loader* p,void*,Descriptor* descriptor,uint32_t control) {
 LoadInfo* load=descriptor->load; load->flags[1]=0;
 if(!load->flags[7]) { load->flags[7]=1; _InterlockedIncrement(reinterpret_cast<volatile long*>(&p->activeCount)); p->bytes+=load->accountedBytes; }
 if(load->record) { void* old=load->record; load->record=nullptr; CallGameVirtual<void,2>(old); }
 const uintptr_t opened=CallGameFunction<uintptr_t,0x00727140>(&load->record,load->size,load->key);
 if(!(opened&0xff)) return opened;
 auto* stream=CallGameVirtual<void*,6>(load->record);
 load->flags[4]=0; load->flags[5]=1; load->flags[3]=1; load->flags[8]=1;
 // Only the documented pointer fields of the borrowed stream/block are read.
 void* block=*reinterpret_cast<void**>(static_cast<uint8_t*>(stream)+sizeof(void*));
 void* buffer=block?*reinterpret_cast<void**>(static_cast<uint8_t*>(block)+2*sizeof(void*)):nullptr;
 void* driver=CallGameMethod<void*,0x0072CAA0>(load->service,load->key);
 void* callback=nullptr;
#if defined(_M_IX86)
 callback=reinterpret_cast<void*>(0x00727230u);
#endif
 return CallGameMethod<uintptr_t,0x00401400>(&load->request,driver,buffer,load->offset,0u,load->size,0u,callback,descriptor,control);
}
uintptr_t __fastcall Pop(Loader* p,void*,uint8_t throttle) {
 const void* tag=nullptr;
#if defined(_M_IX86)
 tag=reinterpret_cast<void*>(0x00FFD2DCu);
#endif
 CallGameMethod<uintptr_t,0x004E16F0>(p->mutex,tag);
 if(p->current!=p->finish && (!throttle || p->bytes<=p->budget || p->bytes<=p->limit)) {
  auto* descriptor=static_cast<Descriptor*>(p->current->descriptor); const uint32_t priority=p->current->priority;
  Produce(p,nullptr,descriptor,priority);
  Entry* next=p->current+1;
  if(next!=p->end) p->current=next;
  else { if(p->begin) CallGameFunction<void,0x004E3F70>(p->begin); ++p->block; p->begin=*p->block; p->end=p->begin+32; p->current=p->begin; }
  CallGameMethod<void,0x0059A990>(descriptor);
 }
 return CallGameMethod<uintptr_t,0x004E17B0>(p->mutex);
}
uintptr_t __fastcall Notify(Loader* p,void*,uint32_t a,uint32_t b,float progress) {
 uintptr_t result=0; // Empty-list EAX has no defined value in EA; callers ignore it.
 for(Client* c=p->clients.next;c!=&p->clients;c=c->next) result=CallGameVirtual<uintptr_t,0>(c->receiver,a,b,progress);
 return result;
}
InsertResult* __fastcall Insert(Table* p,void*,InsertResult* out,Node* node,uintptr_t) {
 Node** bucket=&p->buckets[Hash(node->key)]; Node* found=FindNode(*bucket,node);
 if(found) { out->pos={found,bucket}; out->inserted=0; }
 else { node->next=*bucket; *bucket=node; ++p->count; out->pos={node,bucket}; out->inserted=1; }
 return out;
}
Position* __fastcall Lookup(Table* p,void*,Position* out,const uint32_t* key,uintptr_t,uintptr_t ignored) {
 Node** bucket=&p->buckets[Hash(key)]; Node* found=FindKey(*bucket,key,ignored);
 *out=found?Position{found,bucket}:Position{p->sentinel,&p->sentinel}; return out;
}
Iterator* __cdecl UpperBound(Iterator* out,Iterator first,Iterator last,const Entry* value) {
 int32_t count=static_cast<int32_t>((first.end-first.current)+32*(last.block-first.block)+(last.current-last.begin)-32);
 while(count>0) {
  const int32_t half=count>>1; Iterator middle=first; Advance(middle,half);
  // 0072800A is JG, not JA: priority is compared as signed int32.
  if(static_cast<int32_t>(value->priority)>static_cast<int32_t>(middle.current->priority)) count=half;
  else { first=middle; Advance(first,1); count-=1+half; }
 }
 *out=first; return out;
}
}
