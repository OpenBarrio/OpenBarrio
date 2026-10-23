#include "ts3/render/m07_M01.h"
#include <intrin.h>
#include <cstring>
namespace ts3::render::m07 {
uint32_t __stdcall ShaderReleaseM01(FakeShader* self) {
    const auto count=ComRelease(&self->refs);
    if(count==0) CallGameVirtual<void*,5>(self,1u);
    return count;
}
uint32_t __stdcall ShaderAddRefM01(FakeShader* self) {
    return ComAddRef(&self->refs);
}
namespace {
FakeShader* InitShader(FakeShader* self,void* device,const void* bytes,uint32_t size,bool vertex) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void*>(uintptr_t(vertex?0x00FE7E24:0x00FE7E0C));
#else
    throw std::runtime_error("M01 shader image table unavailable natively");
#endif
    self->bytecodeBegin=nullptr;
    self->bytecodeEnd=nullptr;
    self->bytecodeCapacity=nullptr;
    self->tag=vertex?0x00FE7E70:0x00FE7E50;
    self->device=device;
    _InterlockedExchange(reinterpret_cast<volatile long*>(&self->refs),0);
    // The range insertion helper has an additional device argument, even though
    // its ordinary byte-vector instantiation does not use it.
    CallGameMethod<void,0x00726A60>(&self->bytecodeBegin,bytes,static_cast<const uint8_t*>(bytes)+size,device);
    auto table=*static_cast<void* const**>(device);
    reinterpret_cast<Stdcall<uint32_t,void*>>(table[1])(device);
    return self;
}
}
FakeShader* InitPixelM01(FakeShader* self,void* device,const void* bytes,uint32_t size) {
    return InitShader(self,device,bytes,size,false);
}
FakeShader* InitVertexM01(FakeShader* self,void* device,const void* bytes,uint32_t size) {
    return InitShader(self,device,bytes,size,true);
}
}

namespace ts3::render::m07 {
namespace {
struct TextureFormatM01 {uint32_t bits;uint8_t opaque[20];uint8_t compressed;};
struct TextureLockM01 {uint8_t* data;uint32_t pitch;uint32_t opaque[8];};
}
bool UploadTextureM01(void* texture,void* stream,const void* options,uint8_t swizzle) {
    const auto width=CallGameMethod<uint32_t,0x00F20C20>(texture);
    const auto height=CallGameMethod<uint32_t,0x0093ECC0>(texture);
    const auto depth=CallGameMethod<uint32_t,0x006E8400>(texture);
    const uint32_t faces=CallGameMethod<uint32_t,0x00444710>(texture)==1?6:1;
    auto levels=CallGameMethod<uint32_t,0x0072C920>(texture);
    const auto format=CallGameMethod<uint32_t,0x00580420>(texture);
    auto desc=CallGameFunction<TextureFormatM01*,0x006177B0>(format);
    void* target=texture;
    void* copyBack=nullptr;
    if(!(CallGameMethod<uint32_t,0x00F20360>(texture)&2)) {
        copyBack=texture;
        auto p=CallGameFunction<void*,0x004E3F90>(52u,"Renderer/Texture",0u,0u,nullptr,0u);
        target=p?CallGameMethod<void*,0x00619BC0>(p):nullptr;
        if(target) CallGameVirtual<void,1>(target);
        uint32_t flags;std::memcpy(&flags,static_cast<const uint8_t*>(options)+8,4);
        if(!CallGameFunction<bool,0x0060CEA0>(target,faces,depth,width,height,levels,format,256u,flags)) {
            if(target) CallGameVirtual<void,2>(target);
            return false;
        }
    } else if(target) CallGameVirtual<void,1>(target);
    auto src=CallGameMethod<uint8_t*,0x004ADBB0>(stream);
    auto end=src+CallGameVirtual<uint32_t,7>(stream);
    src+=CallGameVirtual<uint32_t,9>(stream,0u);
    if(CallGameMethod<uint32_t,0x0072C920>(target)<levels)
        levels=CallGameMethod<uint32_t,0x0072C920>(target);
    auto finish=[&](bool result) {if(target) CallGameVirtual<void,2>(target);return result;};
    for(uint32_t face=0;face<faces;++face) for(uint32_t level=0;level<levels;++level) {
        auto w=CallGameFunction<uint32_t,0x00617790>(width,level);
        auto h=CallGameFunction<uint32_t,0x00617790>(height,level);
        auto d=CallGameFunction<uint32_t,0x00617790>(depth,level);
        auto pitch=CallGameMethod<uint32_t,0x00617870>(desc,w);
        auto plane=pitch*CallGameMethod<uint32_t,0x00617770>(desc,h);
        auto count=plane*CallGameMethod<uint32_t,0x00617780>(desc,d);
        if(uint32_t(end-src)<count) return finish(false);
        if(swizzle && (desc->bits>8 || desc->compressed)) {
            const auto rows=CallGameMethod<uint32_t,0x00617770>(desc,h);
            (void)CallGameMethod<uint32_t,0x00617780>(desc,d);
            // EA uses the original depth here, not the mip-reduced depth.
            for(uint32_t slice=0;slice<depth;++slice) {
                TextureLockM01 temp,lock;
                TextureLockM01* result;
                if(depth>1) result=CallGameMethod<TextureLockM01*,0x00619070>(target,&temp,slice,level,0u);
                else if(faces>1) result=CallGameMethod<TextureLockM01*,0x00618FB0>(target,&temp,face,level,0u);
                else result=CallGameMethod<TextureLockM01*,0x00618DF0>(target,&temp,level,0u);
                lock=*result;
                if(!lock.data) return finish(false);
                auto input=src;auto output=lock.data;
                if(desc->compressed || desc->bits==16 || desc->bits==64) {
                    for(uint32_t row=0;row<rows;++row) {
                        CallGameFunction<void,0x0060CE10>(output,input,pitch);
                        input+=pitch;output+=lock.pitch;
                    }
                } else if(desc->bits==32 || desc->bits==128) {
                    for(uint32_t row=0;row<rows;++row) {
                        CallGameFunction<void,0x0060CDE0>(output,input,pitch);
                        input+=pitch;output+=lock.pitch;
                    }
                }
                CallGameMethod<void,0x00619160>(target,&lock);
                src+=plane;
            }
        } else {
            if(depth>1) CallGameMethod<void,0x006193B0>(target,level,src,pitch,plane);
            else if(faces>1) CallGameMethod<void,0x006192E0>(target,face,level,src,pitch);
            else CallGameMethod<void,0x006191D0>(target,level,src,pitch,0u,0u);
            src+=count;
        }
    }
    if(copyBack) {
        auto host=CallGameFunction<void*,0x0060E2C0>();
        CallGameMethod<void,0x0060EC10>(host,target,copyBack);
    }
    return finish(true);
}
}

namespace ts3::render::m07 {
namespace {
template<class R,class... A> R ComM01(void* self,size_t slot,A... args) {
    return reinterpret_cast<Stdcall<R,void*,A...>>((*static_cast<void* const**>(self))[slot])(self,args...);
}
void* AllocM01(uint32_t n,const char* tag) {
    return CallGameFunction<void*,0x004E3F90>(n,tag,0u,0u,nullptr,0u);
}
template<class T> T& ImageM01(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(uintptr_t(address));
#else
    (void)address;throw std::runtime_error("M01 image datum unavailable natively");
#endif
}
}
bool InitDeviceM01(DeviceInitM01* self,uint8_t fake) {
    if(!self->privateData) {
        self->privateData=CallGameFunction<DevicePrivate*,0x004E3FB0>(uint32_t(sizeof(DevicePrivate)),4u,0u,"Services/Device/PrivateData",0u,0u,nullptr,0u);
        if(!self->privateData) return false;
        self->privateData->word1C0=0;
        self->privateData->word1C4=0;
        self->privateData->word1C8=0;
    }
    CallGameFunction<void,0x006180F0>();
    auto stackBytes=self->stackCapacity>0x3FFFFFFF?0xFFFFFFFFu:self->stackCapacity*4;
    self->stateStack=AllocM01(stackBytes,"Services/Device/StateStack");
    auto fail=[&]() {CallGameMethod<void,0x00610960>(self);return false;};
    if(!self->stateStack) return fail();
    CallGameMethod<void,0x0060ED00>(self);
    auto p=reinterpret_cast<uint32_t*>(self->privateData->present);
    p[0]=self->width;p[1]=self->height;p[2]=21;p[3]=0;
    p[6]=self->multisample?3:1;
    p[4]=ImageM01<uint32_t>(0x00FE7A60+self->format*4);p[5]=0;
    // Original present parameters contain a 32-bit HWND; never truncate natively.
#if defined(_M_IX86)
    p[7]=uint32_t(uintptr_t(self->window));
#else
    if(self->window) throw std::runtime_error("M01 wire present-parameter HWND unavailable natively");
    p[7]=0;
#endif
    p[8]=self->windowed;p[9]=1;p[10]=75;p[11]=0;
    p[12]=self->windowed?0:self->refresh;p[13]=self->vsync?1:0x80000000u;
    self->fakeDevice=fake;
    auto& shared=ImageM01<void*>(0x011CECD0);
    if(!self->d3d9) {
        if(shared) {self->d3d9=shared;ComM01<uint32_t>(shared,1);}
        else if(fake) {
            self->d3d9=CallGameFunction<void*,0x00611D30>();
            if(self->d3d9) ComM01<uint32_t>(self->d3d9,1);
        } else self->d3d9=Original<Stdcall<void*,uint32_t>>(0x00ECC1E0)(32u);
        if(!self->d3d9) return fail();
    }
    if(!shared) {shared=self->d3d9;ComM01<uint32_t>(shared,1);}
    auto adapter=self->adapter;
    ComM01<uint32_t>(self->d3d9,4);
    self->selectedAdapter=adapter;
    *reinterpret_cast<uint32_t*>(self->privateData)=1;
    if(!CallGameMethod<bool,0x0060E590>(self,self->format)) {
        self->format=CallGameMethod<uint32_t,0x0060E5F0>(self,self->format);
        p[4]=ImageM01<uint32_t>(0x00FE7A60+self->format*4);
    }
    self->capsResult=0;
    if(!fake) {
        uint32_t caps[0x130/4]{};
        uint32_t flags=64;
        if(ComM01<int32_t>(self->d3d9,14,adapter,1u,caps)>=0 && !(caps[7]&0x10000)) flags=32;
        if(ComM01<int32_t>(self->d3d9,16,adapter,1u,self->window,flags,p,&self->device)<0) return fail();
        ++self->creations;
        uint8_t identifier[0x430];
        ComM01<int32_t>(self->d3d9,5,adapter,0u,identifier);
        std::memcpy(&self->capsResult,identifier+0x428,4);
    } else {
        self->device=CallGameFunction<void*,0x00611DA0>();
        if(!self->device) return fail();
        ComM01<uint32_t>(self->device,1);
    }
    std::memcpy(self->privateData->resetPresent,self->privateData->present,0x38);
    ImageM01<void*>(0x011CECCC)=self->device;
    self->thread=ImageM01<Stdcall<uint32_t>>(0x00F95158)();
    if(ComM01<int32_t>(self->device,7,reinterpret_cast<uint8_t*>(self->privateData)+0x1C)<0) return fail();
    uint32_t mode[4];ComM01<int32_t>(self->device,8,0u,mode);
    std::memcpy(self->privateData->opaque01BC,&mode[3],4);
    CallGameMethod<void,0x0060E740>(self);
    self->querySupport=ComM01<int32_t>(self->device,118,8u,nullptr)>=0;
    if(fake) return true;
    self->backBuffer=AllocM01(40,"Services/Device/BackBuffer");
    if(self->backBuffer) self->backBuffer=CallGameMethod<void*,0x00618060>(self->backBuffer);
    if(!self->backBuffer) return fail();
    CallGameVirtual<void,1>(self->backBuffer);
    self->depthBuffer=AllocM01(40,"Services/Device/DepthBuffer");
    if(self->depthBuffer) self->depthBuffer=CallGameMethod<void*,0x00618060>(self->depthBuffer);
    if(!self->depthBuffer) return fail();
    CallGameVirtual<void,1>(self->depthBuffer);
    if(!CallGameMethod<bool,0x0060F430>(self)) return fail();
    CallGameMethod<void,0x006103B0>(self);
    if(!CallGameMethod<bool,0x0060F350>(self)) return fail();
    auto indices=6*(self->quadVertices>>2);
    self->quadIndex=AllocM01(36,"Services/Device/ImmQuadIB");
    if(self->quadIndex) self->quadIndex=CallGameMethod<void*,0x00616EE0>(self->quadIndex);
    if(!self->quadIndex) return fail();
    CallGameVirtual<void,1>(self->quadIndex);
    if(!CallGameMethod<bool,0x00616DF0>(self->quadIndex,indices,0u,0u)) return fail();
    auto out=CallGameMethod<uint16_t*,0x00616BB0>(self->quadIndex,0u,0u,0u);
    if(!out) return fail();
    for(uint32_t i=0;i<(self->quadVertices&~3u);i+=4,out+=6) {
        auto v=uint16_t(i);
        out[1]=uint16_t(v+1);out[0]=v;out[3]=v;
        out[2]=uint16_t(v+2);out[4]=uint16_t(v+2);out[5]=uint16_t(v+3);
    }
    CallGameMethod<void,0x00616C10>(self->quadIndex);
    CallGameMethod<void,0x0060EF00>(self);
    CallGameMethod<void,0x0060F4C0>(self);
    self->word54=0;
    CallGameMethod<void,0x0060E860>(self,22u,1u);
    auto cache=AllocM01(0x1450,"Services/Device/VertexFormatCache");
    if(!cache) return fail();
    cache=CallGameMethod<void*,0x0061AC10>(cache);
    if(!cache) return fail();
    CallGameFunction<void,0x0061A520>(cache);
    if(!CallGameMethod<bool,0x007EAEA0>(cache)) return fail();
    CallGameMethod<void,0x0060F630>(self);
    return true;
}
}
namespace ts3::render::m07 {
bool __fastcall InitDeviceEntryM01(DeviceInitM01* self,void*,uint8_t fake) {return InitDeviceM01(self,fake);}
FakeShader* __fastcall InitPixelEntryM01(FakeShader* self,void*,void* device,const void* bytes,uint32_t size) {return InitPixelM01(self,device,bytes,size);}
FakeShader* __fastcall InitVertexEntryM01(FakeShader* self,void*,void* device,const void* bytes,uint32_t size) {return InitVertexM01(self,device,bytes,size);}
}
