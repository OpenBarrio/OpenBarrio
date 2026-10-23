#include "ts3/resource/m03_G29.h"
#include <algorithm>
namespace ts3::resource::m03::g29 {
namespace {
struct Key { uint32_t low,high,type,group; };
void* Script() { return CallGameFunction<void*,0x007692B0>(); }
Key MakeKey(void* script,uintptr_t name) {
 auto text=CallGameVirtual<const char16_t*,22>(script,name);
 return {CallGameFunction<uint32_t,0x007CFDA0>(text),0,0x4e97bad8,0};
}
uintptr_t Release(void* p) { return CallGameVirtual<uintptr_t,2>(p); }
uintptr_t Close(void* p) { return CallGameVirtual<uintptr_t,8>(p); }
uint32_t Open(Cache* self,const Key& key,void** record,bool write) {
 return CallGameVirtual<uint32_t,13>(self->database,&key,record,write?2u:1u,write?2u:6u,1u,0u);
}
uintptr_t Commit(Cache* self) {
 auto manager=CallGameFunction<void*,0x0072F1C0>();
 if(manager) return CallGameVirtual<uintptr_t,24>(manager,self->database,0u);
 return CallGameVirtual<uintptr_t,9>(self->database);
}
}
uint8_t __fastcall ReadBytes(Cache* self,void*,uintptr_t name,uintptr_t* out) {
 *out=0;
 if(!self->database) return 0;
 void* script=Script(); auto key=MakeKey(script,name); void* record=nullptr;
 if(!static_cast<uint8_t>(Open(self,key,&record,false))) { if(record) Release(record); return 0; }
 auto stream=CallGameVirtual<void*,6>(record);
 uint32_t size=CallGameVirtual<uint32_t,7>(stream);
 uintptr_t array=0; CallGameVirtual<void,32>(script,&array,size);
 void* data=nullptr; uint32_t length=0;
 CallGameVirtual<void,28>(script,array,&data,&length);
 bool ok=CallGameVirtual<uint32_t,12>(stream,data,size)==size;
 if(ok) *out=array;
 else CallGameVirtual<void,37>(script,array);
 Close(record); if(record) Release(record);
 return ok?1:0;
}
uintptr_t __fastcall ReadHandle(Cache* self,void*,uintptr_t name) {
 if(!self->database) return 0;
 auto key=MakeKey(Script(),name); void* record=nullptr;
 if(!static_cast<uint8_t>(Open(self,key,&record,false))) { if(record) Release(record); return 0; }
 auto stream=CallGameVirtual<void*,6>(record);
 if(stream) CallGameVirtual<uintptr_t,1>(stream);
 bool valid=static_cast<uint8_t>(CallGameFunction<uint32_t,0x00739320>(&stream))!=0;
 uintptr_t result=0;
 if(valid) {
  result=CallGameFunction<uintptr_t,0x00774E00>(stream);
 }
 Close(record);
 if(stream) Release(stream);
 if(record) Release(record);
 return result;
}
uintptr_t __fastcall WriteBytes(Cache* self,void*,uintptr_t name,uintptr_t array,uint32_t count) {
 auto script=Script(); auto key=MakeKey(script,name); void* record=nullptr;
 uintptr_t result=Open(self,key,&record,true);
 if(static_cast<uint8_t>(result)) {
  void* data=nullptr; uint32_t length=0;
  CallGameVirtual<void,28>(script,array,&data,&length);
  auto stream=CallGameVirtual<void*,6>(record);
  CallGameVirtual<uintptr_t,14>(stream,data,std::min(count,length));
  Close(record); result=Commit(self);
 }
 if(record) result=Release(record);
 return result;
}
uintptr_t __fastcall WriteHandle(Cache* self,void*,uintptr_t name,uintptr_t handle) {
 auto source=CallGameFunction<void*,0x00774DC0>(handle);
 if(!source) return 0;
 auto key=MakeKey(Script(),name); void* record=nullptr;
 uintptr_t result=Open(self,key,&record,true);
 if(static_cast<uint8_t>(result)) {
  auto target=CallGameVirtual<void*,6>(record); bool ok=false;
  if(static_cast<uint8_t>(CallGameVirtual<uint32_t,10>(source,0u,0u))) {
   uint32_t remaining=CallGameVirtual<uint32_t,7>(source);
   if(remaining!=0xffffffffu) {
    ok=true; char buffer[4096];
    while(remaining) {
     uint32_t n=std::min(remaining,4096u);
     if(CallGameVirtual<uint32_t,12>(source,buffer,n)!=n ||
        !static_cast<uint8_t>(CallGameVirtual<uint32_t,14>(target,buffer,n))) { ok=false; break; }
     remaining-=n;
    }
   }
  }
  Close(record);
  if(ok) result=Commit(self);
  else result=CallGameVirtual<uintptr_t,16>(self->database,&key);
 }
 if(record) result=Release(record);
 return result;
}
}
