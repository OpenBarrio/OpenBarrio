#include "ts3/anim/m05_M36.h"
#include <cstring>
namespace ts3::anim {
namespace {
// Only fields actually constructed by EA are written; +14..1f are opaque
// for quaternion-only records. The vector insertion remains an original bridge.
void* AppendRecord(M36Range& r,uint32_t k,const Float4Value* v,bool timed=false,float a=0,float b=0) {
 alignas(16) M36Record record;
 std::memcpy(&record.value,v,16); record.key=k;
 if(timed) { record.start=a; record.end=b; }
 auto p=r.end;
 if(reinterpret_cast<uintptr_t>(p)<reinterpret_cast<uintptr_t>(r.capacity)) {
  r.end=reinterpret_cast<M36Record*>(reinterpret_cast<uintptr_t>(p)+sizeof(M36Record));
  if(p) {
   std::memcpy(&p->value,&record.value,16); p->key=k;
   if(timed) { p->start=a; p->end=b; }
  }
  return p;
 }
 if(timed) return CallGameMethod<void*,0x0056B010>(&r,p,&record);
 return CallGameMethod<void*,0x0055BD40>(&r,p,&record);
}
const Float4Value identity{{0,0,0,1}};
const Float4Value defaultTimed{{1,0,0,1}};
}
void* M36Append(M36Controller* s,uint32_t k,const Float4Value* v) {
 return AppendRecord(s->rotation,k,v);
}
void* M36Identity(M36Controller* s,uint32_t k) {
 return AppendRecord(s->rotation,k,&identity);
}
void* M36DefaultTimed(M36Controller* s,uint32_t k) {
 AppendRecord(s->timed,k,&defaultTimed,true,0.0f,0.2857142984867096f);
 return M36Identity(s,k);
}
void* M36Timed(M36Controller* s,uint32_t k,float a,float b,const Float4Value* v) {
 AppendRecord(s->timed,k,v,true,a,b);
 return M36Identity(s,k);
}
void* M36AppendOther(M36Controller* s,uint32_t k,const Float4Value* v) {
 return AppendRecord(s->other,k,v);
}
void* M36IdentityOther(M36Controller* s,uint32_t k) {
 return AppendRecord(s->other,k,&identity);
}
}
