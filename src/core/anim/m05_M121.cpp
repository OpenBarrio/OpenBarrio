#include "ts3/anim/m05_M121.h"
namespace ts3::anim::m121 {
namespace {
const char* Descriptor() {
#if defined(_M_IX86)
 return reinterpret_cast<const char*>(0x01023104);
#else
 return "Locomotion/StyleProgressionEntry/AgeGenderExclusionVector";
#endif
}
void Initialize(Entry& e) { e.inlineBase=&e.storage; e.begin=e.end=&e.storage; e.capacity=&e.tail; e.descriptor=Descriptor(); }
}
Result* LargeTable::Insert(void* output,const void* input,uintptr_t hint) {
 auto* result=static_cast<Result*>(output);auto* pair=static_cast<const uint32_t*>(input);
 (void)hint;
 uint32_t key=*pair,index=key%bucketCount;
 auto** bucket=buckets+index;
 for(auto* node=*bucket;node;node=node->next) {
  if(node->key==key) { result->node=node;result->inserted=0;result->bucket=bucket;return result; }
 }
 struct Rehash { uint8_t required; uint8_t padding[3]; uint32_t count; } rehash;
 CallGameMethod<void,0x004E09B0>(policy,&rehash,bucketCount,count,1u);
 auto* node=CallGameMethod<LargeNode*,0x00842690>(this,pair);
 if(rehash.required) { index=key%rehash.count;CallGameMethod<void,0x0083AC90>(this,rehash.count); }
 node->next=buckets[index];buckets[index]=node;++count;
 result->node=node;result->inserted=1;result->bucket=buckets+index;return result;
}
uintptr_t Entries::Insert(void* positionInput,void* sourceInput) {
 auto* position=static_cast<Entry*>(positionInput);auto* source=static_cast<Entry*>(sourceInput);
 if(end!=capacity) {
  if(reinterpret_cast<uintptr_t>(source)>=reinterpret_cast<uintptr_t>(position) && reinterpret_cast<uintptr_t>(source)<reinterpret_cast<uintptr_t>(end)) ++source;
  if(end) CallGameMethod<void,0x0083FE90>(end,end-1);
  uintptr_t result=CallGameFunction<uintptr_t,0x0083FF60>(position,end-1,end);
  if(position!=source) {
   position->end=position->begin;
   result=CallGameMethod<uintptr_t,0x0046AD60>(position,source->begin,source->end,position);
  }
  position->tail=source->tail;++end;return result;
 }
 int32_t size=static_cast<int32_t>(reinterpret_cast<intptr_t>(end)-reinterpret_cast<intptr_t>(begin))/sizeof(Entry);
 uint32_t newCapacity=size?2u*size:1u;
 Entry* storage=nullptr;
 if(newCapacity) {
#if defined(_M_IX86)
  const void* file=reinterpret_cast<const void*>(0x00F98980);
#else
  const void* file=nullptr;
#endif
  storage=CallGameFunction<Entry*,0x004E3F90>(uint32_t(sizeof(Entry)*newCapacity),descriptor,0u,0u,file,0xe7u);
 }
 Entry* cursor;
 CallGameFunction<void,0x0083FFB0>(&cursor,begin,position,storage,position);
 Entry* inserted=cursor;
 if(inserted) CallGameMethod<void,0x0083FE90>(inserted,source);
 CallGameFunction<void,0x0083FFB0>(&cursor,position,end,inserted+1,inserted);
 CallGameMethod<void,0x00759410>(this,begin,end);
 if(begin && begin!=inlineBase) CallGameFunction<uintptr_t,0x004E3F70>(begin);
 end=cursor;begin=storage;capacity=storage+newCapacity;return newCapacity;
}
uintptr_t Entries::Append() {
 Entry* slot=end;
 if(reinterpret_cast<uintptr_t>(slot)<reinterpret_cast<uintptr_t>(capacity)) {
  ++end;if(slot) Initialize(*slot);return reinterpret_cast<uintptr_t>(slot);
 }
 Entry temporary;Initialize(temporary);Insert(slot,&temporary);
 uintptr_t result=reinterpret_cast<uintptr_t>(temporary.begin);
 if(temporary.begin && temporary.begin!=temporary.inlineBase) result=CallGameFunction<uintptr_t,0x004E3F70>(temporary.begin);
 return result;
}
SmallValue* SmallTable::Get(const void* input) {
 auto* key=static_cast<const uint32_t*>(input);
 struct Find { void* node;void* bucket;uint8_t inserted; } found;
 CallGameMethod<void,0x0083B900>(this,&found,key);
 if(found.node==buckets[bucketCount]) {
  SmallValue temporary;temporary.begin=temporary.end=temporary.storage;temporary.capacity=temporary.storage+80;
  struct Pair { uint32_t key;SmallValue value; } pair;
  pair.key=*key;pair.value.begin=pair.value.end=pair.value.storage;pair.value.capacity=pair.value.storage+sizeof(pair.value.storage);
  CallGameMethod<void,0x0083ED50>(&pair.value,temporary.begin,temporary.end,found.node);
  uintptr_t hint=reinterpret_cast<uintptr_t>(found.node)&~uintptr_t(0xff);
  CallGameMethod<void,0x008425D0>(this,&found,&pair,hint);
  CallGameMethod<void,0x00912B60>(&pair.value,pair.value.begin,pair.value.end);
  CallGameMethod<void,0x00912B60>(&temporary,temporary.begin,temporary.end);
 }
 return reinterpret_cast<SmallValue*>(static_cast<unsigned char*>(found.node)+offsetof(LargeNode,value));
}
bool Style::Copy(uint32_t key,void* output) {
 auto* destination=static_cast<SmallValue*>(output);
 SmallValue* source=table.Get(&key);
 if(destination!=source) {
  CallGameMethod<void,0x00912B60>(destination,destination->begin,destination->end);
  destination->end=destination->begin;
  CallGameMethod<void,0x0083ED50>(destination,source->begin,source->end,key);
 }
 return true;
}
LargeValue* LargeTable::Get(const void* input) {
 auto* key=static_cast<const uint32_t*>(input);
 Result found;
 CallGameMethod<void,0x0083B8A0>(this,&found,key);
 if(found.node!=buckets[bucketCount]) return &found.node->value;
 LargeValue temporary;temporary.begin=temporary.end=temporary.storage;
 struct Pair { uint32_t key;LargeValue value; } pair;
 pair.key=*key;pair.value.begin=pair.value.end=pair.value.storage;
 pair.value.capacity=pair.value.storage+sizeof(pair.value.storage);
 temporary.capacity=reinterpret_cast<unsigned char*>(&pair.key);
 CallGameMethod<void,0x0083F2A0>(&pair.value,temporary.begin,temporary.end,found.node);
 uintptr_t hint=reinterpret_cast<uintptr_t>(found.node)&~uintptr_t(0xff);
 Insert(&found,&pair.key,hint);
 LargeValue* value=&found.node->value;
 for(auto* p=pair.value.begin;p<pair.value.end;p+=124) CallGameMethod<void,0x00754060>(p);
 for(auto* p=temporary.begin;p<temporary.end;p+=124) CallGameMethod<void,0x00754060>(p);
 return value;
}
}
