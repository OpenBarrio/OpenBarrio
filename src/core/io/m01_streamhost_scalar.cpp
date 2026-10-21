#include "ts3/io/m01_streamhost_scalar.h"
#include <cstring>

namespace ts3::io::m01 {
namespace {
IStream* Resolve(uint32_t handle) {
#if defined(_M_IX86)
    auto& cachedHandle=*reinterpret_cast<uint32_t*>(kCacheHandle);
    auto& cachedStream=*reinterpret_cast<IStream**>(kCacheStream);
    if(handle==cachedHandle) return cachedStream;
    if((handle & 0xff000000u)!=0x60000000u) return nullptr;
    void* service=CallGameFunction<void*,0x7692B0>();
    auto* stream=CallGameVirtual<IStream*,51>(service,handle);
    if(stream) { cachedHandle=handle; cachedStream=stream; }
    return stream;
#else
    (void)handle;
    NotRewritten("M01 native Scripting handle boundary");
#endif
}
void Buffer(uint32_t array,void** data,int32_t* length) {
    auto* service=CallGameFunction<void*,0x7692B0>();
    CallGameVirtual<void,28>(service,array,data,length);
}
int ReadStatus(size_t actual,size_t wanted) {
    if(actual==wanted) return 0;
    return actual==kSizeTypeError ? -3 : -1;
}
int WriteValue(uint32_t handle,const void* data,size_t size) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    return stream->Write(data,size) ? 0 : -3;
}
int ReadValue(uint32_t handle,void* data,size_t size,bool clear) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    if(clear) std::memset(data,0,size);
    return ReadStatus(stream->Read(data,size),size);
}
}
int ContinuationCount(const uint8_t* text) {
    // Exact ranges of the byte table at FB9548 (C0/C1 and F5..FF invalid).
    unsigned byte=*text;
    if(byte>=0xc2 && byte<=0xdf) return 1;
    if(byte>=0xe0 && byte<=0xef) return 2;
    if(byte>=0xf0 && byte<=0xf4) return 3;
    return 0;
}
int WriteInt64(uint32_t handle,uint64_t value) { return WriteValue(handle,&value,8); }
int WriteFloat(uint32_t handle,float value) { return WriteValue(handle,&value,4); }
int WriteDouble(uint32_t handle,double value) {
    // The x86 original passes through x87 (including signalling NaN quieting).
#if defined(_M_IX86)
    __asm { fld value }
    __asm { fstp value }
#endif
    return WriteValue(handle,&value,8);
}
int WriteDecimal(uint32_t handle,Decimal value) {
    uint32_t wire[4]={value.words[2],value.words[3],value.words[1],value.words[0]};
    return WriteValue(handle,wire,16);
}
int WriteByteArray(uint32_t handle,uint32_t array,int32_t offset,int32_t count) {
    if(!array) return -6;
    if(offset<0 || count<=0) return -7;
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    void* data=nullptr; int32_t length=0; Buffer(array,&data,&length);
    if(!data) return -6;
    if(offset>=length || length-offset<count) return -7;
    return stream->Write(static_cast<uint8_t*>(data)+offset,count) ? 0 : -3;
}
int WriteCharUTF8(uint32_t handle,uint16_t value) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    uint8_t bytes[8];
    int n=CallGameFunction<int,0x4F4260>(bytes,8,&value,1);
    return stream->Write(bytes,n) ? 0 : -3;
}
int ReadInt(uint32_t handle,int32_t count,uint32_t* value) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    if(count>4) return -3;
    *value=0;
    return ReadStatus(stream->Read(value,static_cast<size_t>(count)),static_cast<size_t>(count));
}
int ReadInt64(uint32_t handle,uint64_t* value) { return ReadValue(handle,value,8,true); }
int ReadFloat(uint32_t handle,float* value) {
    float temp=0;
    int status=ReadValue(handle,&temp,4,false);
    if(!status) std::memcpy(value,&temp,4);
    return status;
}
int ReadDouble(uint32_t handle,double* value) {
    double temp=0;
    int status=ReadValue(handle,&temp,8,false);
    if(!status) {
#if defined(_M_IX86)
        __asm { fld temp }
        __asm { fstp temp }
#endif
        std::memcpy(value,&temp,8);
    }
    return status;
}
int ReadDecimal(uint32_t handle,Decimal* value) {
    uint32_t wire[4];
    int status=ReadValue(handle,wire,16,false);
    if(!status) { value->words[0]=wire[3]; value->words[1]=wire[2]; value->words[2]=wire[0]; value->words[3]=wire[1]; }
    return status;
}
int ReadCharUTF8(uint32_t handle,uint16_t* value) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    uint8_t bytes[8];
    size_t n=stream->Read(bytes,1);
    if(n!=1) return ReadStatus(n,1);
    if(bytes[0]<0x80) { *value=bytes[0]; return 0; }
    int extra=ContinuationCount(bytes);
    if(!extra) return -4;
    n=stream->Read(bytes+1,extra);
    // EA bug: compare to 1, not extra; conversion also receives extra, not extra+1.
    if(n!=1) return ReadStatus(n,1);
    return CallGameFunction<int,0x4F42B0>(value,1,bytes,extra)==1 ? 0 : -4;
}
int ReadByteArray(uint32_t handle,uint32_t array,int32_t offset,int32_t count) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    if(!array) return -6;
    void* data=nullptr; int32_t length=0; Buffer(array,&data,&length);
    if(offset<0 || count<0 || offset>length || length-offset<count) return -7;
    if(!count) return 0;
    size_t n=stream->Read(static_cast<uint8_t*>(data)+offset,count);
    return n==kSizeTypeError ? -3 : static_cast<int>(n);
}
int GetSize(uint32_t handle) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    size_t n=stream->GetSize();
    return n==kSizeTypeError ? -3 : static_cast<int>(n);
}
int GetPosition(uint32_t handle) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    auto n=stream->GetPosition(kPositionTypeBegin);
    return n==-1 ? -3 : static_cast<int>(n);
}
int SetPosition(uint32_t handle,int32_t position,uint32_t origin) {
    auto* stream=Resolve(handle);
    if(!stream) return -2;
    if(origin>=3) return -7;
    return stream->SetPosition(position,static_cast<PositionType>(origin)) ? 0 : -3;
}
}
