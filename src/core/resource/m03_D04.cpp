#include "ts3/resource/m03_D04.h"
namespace ts3::resource::m03::d04 {
Leaves leaves{};
namespace {
template<class R,int S,class... A> R V(void* p,A... a) { return CallGameVirtual<R,S>(p,a...); }
void* Manager() {return leaves.manager?leaves.manager():CallGameFunction<void*,0x004AFD20>();}
void* Scripting() {return leaves.scripting?leaves.scripting():CallGameFunction<void*,0x007692B0>();}
void* Cache() {
 if(leaves.cache) return leaves.cache();
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(0x011D3E40);
#else
 NotRewritten("M03 native CacheManager singleton");
#endif
}
bool Enabled() {
 if(leaves.enabled) return leaves.enabled();
#if defined(_M_IX86)
 return *reinterpret_cast<uint8_t*>(0x0115DBF0)!=0;
#else
 NotRewritten("M03 native CacheManager enabled byte");
#endif
}
Key NamedKey(uintptr_t name) {
 auto* text=V<const char16_t*,22>(Scripting(),name);
 uint64_t hash=leaves.hash?leaves.hash(text):CallGameFunction<uint64_t,0x007CFE40>(text);
 return {uint32_t(hash),uint32_t(hash>>32),0x82b43584,0};
}
void Free(void* p) {if(leaves.free) leaves.free(p);else CallGameFunction<void,0x004E3F70>(p);}
void* Factory() {return V<void*,95>(Scripting());}
}
// Adapted from m01::FileType and m02::c1::RegistrationTable donors.
uintptr_t __fastcall ClassId(void*,void*) {
#if defined(_M_IX86)
 return 0x00FFF648;
#else
 if(!leaves.classId) NotRewritten("M03 native CacheManager class identifier");
 return reinterpret_cast<uintptr_t>(leaves.classId);
#endif
}
const void** __fastcall BindingTable(void*,void*,const void** out) {
#if defined(_M_IX86)
 *out=reinterpret_cast<const void*>(0x0115DBF8);
#else
 if(!leaves.bindings) NotRewritten("M03 native CacheManager bindings");
 *out=leaves.bindings;
#endif
 return out;
}
uint8_t ClearCache(uintptr_t name) {
 if(!Enabled()) return 0;
 void* manager=Manager(); if(!manager) return 0;
 Key key=NamedKey(name);
 void* db=V<void*,17>(manager,&key); if(!db) return 0;
 KeyList keys{nullptr,nullptr,nullptr,
#if defined(_M_IX86)
 reinterpret_cast<const char*>(0x00FFF66C)
#else
 "Scripting/GameplayDataKeyList"
#endif
 };
 V<void,12>(db,&keys,0u);
 bool ok=true;
 while(keys.begin!=keys.end) {
  // EA only commits the pop after successful identity comparison.
  if(V<void*,17>(manager,keys.end-1)!=db) {ok=false;break;}
  --keys.end;
 }
 if(keys.begin) Free(keys.begin);
 return uint8_t(ok);
}
uint8_t ReadStream(uintptr_t name,uint64_t* first,uint64_t* second,uint32_t* third) {
 *first=0; *second=0; *third=0;
 void* manager=Manager(); if(!manager) return 0;
 Key key=NamedKey(name); void* db=V<void*,17>(manager,&key); if(!db) return 0;
 void* stream=nullptr;
 if(!uint8_t(V<unsigned,13>(db,&key,&stream,1u,6u,1u,0u))) {
  if(stream) V<void,2>(stream);
  return 0;
 }
 // push 1 precedes GetSource but belongs to the following archive ctor.
 void* source=V<void*,6>(stream);
 Archive archive;
 if(leaves.construct) leaves.construct(&archive,source,1u);
 else CallGameMethod<void,0x007D91B0>(&archive,source,1u);
 uint8_t r=leaves.read64?leaves.read64(archive.stream,first,archive.mode):CallGameFunction<uint8_t,0x004DD660>(archive.stream,first,archive.mode);
 archive.good &= r;
 r=leaves.read64?leaves.read64(archive.stream,second,archive.mode):CallGameFunction<uint8_t,0x004DD660>(archive.stream,second,archive.mode);
 archive.good &= r;
 r=leaves.read32?leaves.read32(archive.stream,third,archive.mode):CallGameFunction<uint8_t,0x004DD700>(archive.stream,third,archive.mode);
 archive.good &= r;
 V<void,8>(stream);
 uint8_t good=archive.good;
 if(leaves.destroy) leaves.destroy(&archive);else CallGameMethod<void,0x007D7760>(&archive);
 if(stream) V<void,2>(stream);
 return good;
}
uint8_t IsCachingAllowed(uintptr_t name,void* out) {
 void* p=Cache(); if(!p) return 0;
 return uint8_t(leaves.allowed?leaves.allowed(p,name,out):CallGameMethod<uintptr_t,0x0073C910>(p,name,out));
}
uintptr_t ReadCachedData(uintptr_t name) {
 void* p=Cache(); if(!p) return 0;
 return leaves.read?leaves.read(p,name):CallGameMethod<uintptr_t,0x0073CA50>(p,name);
}
uintptr_t ReadChecksum(uintptr_t name,uintptr_t out,unsigned size) {
 void* p=Cache(); if(!p) return 0; // null EAX is unspecified, not a defined zero contract
 return leaves.checksum?leaves.checksum(p,name,out,size):CallGameMethod<uintptr_t,0x0073CB60>(p,name,out,size);
}
uintptr_t WriteCachedData(uintptr_t name,uintptr_t data) {
 void* p=Cache(); if(!p) return 0; // null EAX unspecified
 return leaves.write?leaves.write(p,name,data):CallGameMethod<uintptr_t,0x0073CC70>(p,name,data);
}
uintptr_t Shutdown() {
 void* p=Cache(); if(!p) return 0; // null EAX unspecified
 return leaves.shutdown?leaves.shutdown(p):CallGameMethod<uintptr_t,0x0073CDF0>(p);
}
uintptr_t FactoryCreate(uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d) {return V<uintptr_t,13>(Factory(),a,b,c,d);}
uintptr_t GetPrototype(uintptr_t a) {return V<uintptr_t,14>(Factory(),a);}
uintptr_t GetDataClass(uintptr_t a) {return V<uintptr_t,15>(Factory(),a);}
uintptr_t ReleasePrototype(uintptr_t a) {return V<uintptr_t,16>(Factory(),a);}
uintptr_t ReleaseDataClass(uintptr_t a) {return V<uintptr_t,17>(Factory(),a);}
}
