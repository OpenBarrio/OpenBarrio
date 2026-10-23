#include "ts3/resource/m03_manager_refs_contracts.h"
namespace ts3::resource::m03::manager::refs {
uint32_t NativeBoundary::ResolveCachedRecord(manager::NativeState& state,uint32_t cacheFlag,const Key* key,void** out,uint32_t flags,NativeDatabase* db,NativeCallback* factory,const Key* alias) const {
    if(!calls_.resolveCachedRecord) throw std::logic_error("M03 refs: missing resolveCachedRecord");
    return calls_.resolveCachedRecord(calls_.context,state,cacheFlag,key,out,flags,db,factory,alias);
}
uint8_t NativeBoundary::ApplyRecord(manager::NativeState& state,NativeRecordResource& resource,uint32_t word,NativeDatabase* db,NativeCallback* factory,const Key* key) const {
    if(!calls_.applyRecord) throw std::logic_error("M03 refs: missing applyRecord");
    return calls_.applyRecord(calls_.context,state,resource,word,db,factory,key);
}
void NativeBoundary::RehashTypeNameBuckets(NativeBucketOwner& owner,uint32_t count) const {
    if(!calls_.rehashTypeNameBuckets) throw std::logic_error("M03 refs: missing rehashTypeNameBuckets");
    calls_.rehashTypeNameBuckets(calls_.context,owner,count);
}
void NativeBoundary::RehashBuckets(NativeBucketOwner& owner,uint32_t count) const {
    if(!calls_.rehashBuckets) throw std::logic_error("M03 refs: missing rehashBuckets");
    calls_.rehashBuckets(calls_.context,owner,count);
}
uint32_t NativeBoundary::DelegateRecord(manager::NativeState& state,const Key* key,void** out,uint32_t flags,NativeDatabase* db,NativeCallback* factory,const Key* alias) const {
    if(!calls_.delegateRecord) throw std::logic_error("M03 refs: missing delegateRecord");
    return calls_.delegateRecord(calls_.context,state,key,out,flags,db,factory,alias);
}
uint32_t NativeBoundary::Callback7(NativeCallback& cb,const Key* key,uint32_t flag) const {
    if(!calls_.callback7) throw std::logic_error("M03 refs: missing callback7");
    return calls_.callback7(calls_.context,cb,key,flag);
}
uint32_t NativeBoundary::Callback10(NativeCallback& cb,void* argument) const {
    if(!calls_.callback10) throw std::logic_error("M03 refs: missing callback10");
    return calls_.callback10(calls_.context,cb,argument);
}
uint32_t NativeBoundary::RetainDatabase(NativeDatabase& db) const {
    if(!calls_.retainDatabase) throw std::logic_error("M03 refs: missing retainDatabase");
    return calls_.retainDatabase(calls_.context,db.reference);
}
uint32_t NativeBoundary::ReleaseDatabase(NativeDatabase& db) const {
    if(!calls_.releaseDatabase) throw std::logic_error("M03 refs: missing releaseDatabase");
    return calls_.releaseDatabase(calls_.context,db.reference);
}
uint32_t NativeBoundary::RetainCallback(NativeCallback& cb) const {
    if(!calls_.retainCallback) throw std::logic_error("M03 refs: missing retainCallback");
    return calls_.retainCallback(calls_.context,cb);
}
uint32_t NativeBoundary::ReleaseCallback(NativeCallback& cb) const {
    if(!calls_.releaseCallback) throw std::logic_error("M03 refs: missing releaseCallback");
    return calls_.releaseCallback(calls_.context,cb);
}
#if defined(_M_IX86)
uint32_t ResolveCachedRecord(ResourceManager* manager,uint32_t cacheFlag,const Key* key,void** out,uint32_t flags,void* db,RawCallback* factory,const Key* alias) {
    return CallGameMethod<uint32_t,0x004B0480>(manager,cacheFlag,key,out,flags,db,factory,alias);
}
uint8_t ApplyRecord(ResourceManager* manager,RecordResource* resource,uint32_t word,void* db,RawCallback* factory,const Key* key) {
    return CallGameMethod<uint8_t,0x004B0840>(manager,resource,word,db,factory,key);
}
void RehashTypeNameBuckets(TypeNameBuckets* owner,uint32_t count) {
    CallGameMethod<void,0x004B0330>(owner,count);
}
void RehashBuckets(CallbackBuckets* owner,uint32_t count) {
    CallGameMethod<void,0x004B0280>(owner,count);
}
uint32_t DelegateRecord(ResourceManager* manager,const Key* key,void** out,uint32_t flags,void* db,RawCallback* factory,const Key* alias) {
    return CallGameMethod<uint32_t,0x004B0810>(manager,key,out,flags,db,factory,alias);
}
uint32_t ConvertDelegateRecord(ResourceManager* manager,const Key* key,void** out,uint32_t flags,void* db,RawCallback* factory,const Key* alias) {
    return CallGameMethod<uint32_t,0x00736610>(manager,key,out,flags,db,factory,alias);
}
uint32_t Delegate7(ResourceManager* manager,const Key* key,uint32_t flag) {
    return CallGameMethod<uint32_t,0x004B00C0>(manager,key,flag);
}
uint32_t Delegate10(ResourceManager* manager,RawCallback* callback,void* argument) {
    return CallGameMethod<uint32_t,0x004B0130>(manager,callback,argument);
}
uint32_t ReleaseDatabases(DatabaseEntry* first,DatabaseEntry* last) {
    return CallGameMethod<uint32_t,0x004B0220>(static_cast<void*>(nullptr),first,last);
}
uint32_t ReleaseCallbacks(CallbackEntry* first,CallbackEntry* last) {
    return CallGameMethod<uint32_t,0x004B0250>(static_cast<void*>(nullptr),first,last);
}
DatabaseEntry** CopyDatabases(DatabaseEntry** out,const DatabaseEntry* first,const DatabaseEntry* last,DatabaseEntry* dest) {
    return CallGameFunction<DatabaseEntry**,0x004B03E0>(out,first,last,dest);
}
CallbackEntry** CopyCallbacks(CallbackEntry** out,const CallbackEntry* first,const CallbackEntry* last,CallbackEntry* dest) {
    return CallGameFunction<CallbackEntry**,0x004B0430>(out,first,last,dest);
}
uint32_t RetainDatabase(void* db) {
    return CallGameVirtual<uint32_t,0>(static_cast<uint8_t*>(db)+4);
}
uint32_t ReleaseDatabase(void* db) {
    return CallGameVirtual<uint32_t,1>(static_cast<uint8_t*>(db)+4);
}
uint32_t RetainCallback(RawCallback* cb) { return CallGameVirtual<uint32_t,1>(cb); }
uint32_t ReleaseCallback(RawCallback* cb) { return CallGameVirtual<uint32_t,2>(cb); }
uint32_t Callback7(RawCallback* cb,const Key* key,uint32_t flag) { return CallGameVirtual<uint32_t,7>(cb,key,flag); }
uint32_t Callback10(RawCallback* cb,void* argument) { return CallGameVirtual<uint32_t,10>(cb,argument); }
#endif
}
