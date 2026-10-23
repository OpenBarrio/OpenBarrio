#include "ts3/anim/m05_M156.h"
#include <cstring>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim {
namespace {
void SetTables(M156Body* self, uintptr_t primary, uintptr_t secondary) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void*>(primary);
    self->interfaceVtable = reinterpret_cast<void*>(secondary);
#else
    (void)self; (void)primary; (void)secondary;
    NotRewritten("M156 original body vtables");
#endif
}
}
M156Body* Construct953310(M156Body* self, void* owner) {
    CallGameMethod<void, 0x00839A40>(self, owner);
    SetTables(self, 0x01044410, 0x0104440C);
    CallGameMethod<void, 0x00847710>(&self->first, owner);
    CallGameMethod<void, 0x008399E0>(self, &self->first);
    return self;
}
M156Body* Construct953390(M156Body* self, void* owner) {
    CallGameMethod<void, 0x00839A40>(self, owner);
    SetTables(self, 0x0104441C, 0x01044418);
    CallGameMethod<void, 0x00848150>(&self->first, owner);
    CallGameMethod<void, 0x008399E0>(self, &self->first);
    return self;
}
M156Body* Construct953410(M156Body* self, void* owner) {
    CallGameMethod<void, 0x00839A40>(self, owner);
    SetTables(self, 0x01044428, 0x01044424);
    CallGameMethod<void, 0x00846820>(&self->first, owner);
    CallGameMethod<void, 0x00848150>(&self->second, owner);
    CallGameMethod<void, 0x008399E0>(self, &self->first);
    CallGameMethod<void, 0x008399E0>(self, &self->second);
    return self;
}
M156Body* Construct9534A0(M156Body* self, void* owner) {
    CallGameMethod<void, 0x00839A40>(self, owner);
    SetTables(self, 0x01044434, 0x01044430);
    CallGameMethod<void, 0x00846820>(&self->first, owner);
    CallGameMethod<void, 0x00847710>(&self->second, owner);
    CallGameMethod<void, 0x008399E0>(self, &self->first);
    CallGameMethod<void, 0x008399E0>(self, &self->second);
    return self;
}
uintptr_t Update942C00(M156Reference* self, uintptr_t unused) {
    (void)unused;
    M156Resource* pinned = self->resource;
#if defined(_M_IX86)
    if (pinned) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&pinned->refs), 1);
    // This leaf does not set EAX. The nonnull release paths below define the return;
    // a null resource faults in the leaf, rather than returning a friendly zero.
    CallGameMethod<void, 0x00935810>(self->resource);
    if (pinned) {
        auto* refs = reinterpret_cast<volatile long*>(&pinned->refs);
        if (_InterlockedDecrement(refs) == 0) {
            _InterlockedExchange(refs, 1);
            return CallGameVirtual<uintptr_t, 0>(pinned, uint32_t(1));
        }
        return reinterpret_cast<uintptr_t>(refs);
    }
#else
    (void)pinned;
    NotRewritten("M156 resource update original leaf");
#endif
    return 0;
}
#if defined(_M_IX86)
namespace {
template<class T> T& Field(void* p, size_t offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T> T& Global(uintptr_t address) { return *reinterpret_cast<T*>(address); }
void* Registry(unsigned i) {
    const uintptr_t addresses[] = {0x011E4B28,0x011E07E8,0x011DC4A8};
    return reinterpret_cast<void*>(addresses[i]);
}
bool Readable(M156Stream* s) {
    if (s->cursor>=0) { s->cursor=0; s->failed=1; }
    return !s->failed;
}
uint32_t ReadWord(M156Stream* s, bool enabled) {
    if (!Readable(s) || !enabled) return 0;
    s->cursor=static_cast<int32_t>(static_cast<uint32_t>(s->cursor)+4);
    if (s->cursor>=0) CallGameMethod<void,0x00593FF0>(s,4);
    if (s->cursor>=0) return 0;
    uint32_t word;
    std::memcpy(&word,s->readEnd+s->cursor-3,4);
    return word;
}
unsigned char* ReadPosition(M156Stream* s, bool enabled) {
    return Readable(s) && enabled ? s->positionEnd+s->cursor+1 : nullptr;
}
void ClearRegistries() {
    for (unsigned i=0;i<3;++i) CallGameMethod<void,0x0054C5E0>(Registry(i));
}
void InitRegistries() {
    const uintptr_t exits[]={0x00F93430,0x00F93420,0x00F93410};
    for(unsigned i=0;i<3;++i) {
        auto& flags=Global<uint32_t>(0x011E8E68);
        if (!(flags&(1u<<i))) {
            flags|=1u<<i;
            CallGameMethod<void,0x0054C720>(Registry(i));
            CallGameFunction<int,0x00F7E9A1>(reinterpret_cast<void(*)()>(exits[i]));
        }
    }
}
void* Root(unsigned i) { return CallGameMethod<void*,0x0054C5D0>(Registry(i)); }
void* CachedRoot(unsigned i) { return CallGameMethod<void*,0x0093F720>(Registry(i)); }
void Evaluate(unsigned i, void* context, void* pose) {
    CallGameMethod<void,0x0054C570>(Registry(i),context,pose);
}
void Release(void*& p) { CallGameMethod<void,0x0055CFA0>(&p); }
void AddRef(void* p) { if(p) ++Field<uint32_t>(p,4); }
void* NewPose(void* rig) {
    void* memory=CallGameFunction<void*,0x005748A0>(105264,reinterpret_cast<const char*>(0x00FC10D8),0,0,0,0);
    if (!memory) return nullptr;
    void* rigArgument;
    CallGameMethod<void,0x0074B950>(&rigArgument,rig);
    return CallGameMethod<void*,0x00574F50>(memory,rigArgument);
}
void* NewBuffer() {
    void* p=CallGameFunction<void*,0x004E3F90>(14432,reinterpret_cast<const char*>(0x0104349C),0,0,0,0);
    if(p) {
        Field<uint32_t>(p,4)=0;
        Field<void*>(p,0)=reinterpret_cast<void*>(0x0104348C);
        Field<void*>(p,0x3850)=nullptr;
    }
    return p;
}
void Assign(void*& dst,void* src) { CallGameMethod<void,0x00CF3C70>(&dst,src); }
void* ObtainBuffer(void* body,void* rig,void* context,unsigned registry,uint32_t tick) {
    void* buffer=CallGameMethod<void*,0x00573DA0>(body,tick);
    if(buffer) AddRef(buffer);
    else {
        Assign(buffer,NewBuffer());
        void* pose=NewPose(rig);
        AddRef(pose);
        CallGameMethod<void,0x005743A0>(pose);
        Evaluate(registry,context,pose);
        CallGameMethod<void,0x0055D0C0>(buffer,pose);
        CallGameMethod<void,0x00573E20>(body,buffer,tick);
        Release(pose);
    }
    return buffer;
}
void EvaluateDriver(uint32_t tick,float fraction,void* host,void* body,uint32_t flagsA,uint32_t flagsB) {
    // Private x86 stack context owned by the original context constructor; never a native pointer blob.
    alignas(16) unsigned char context[0x170];
    unsigned char event[8];
    CallGameMethod<void,0x0093E990>(event);
    CallGameMethod<void,0x0083B080>(context);
    if(CallGameMethod<void*,0x00573D40>(body,0)) {
        void* record=CallGameMethod<void*,0x00573D40>(body,0);
        CallGameMethod<void,0x0093F740>(context+0x60,record);
    }
    uint32_t previousTick=Field<uint32_t>(context,0xD4);
    float previousTime=Field<float>(context,0x98);
    int64_t delta=static_cast<int64_t>(tick)-static_cast<int64_t>(previousTick);
    const float step=Global<float>(0x010434C4);
    float elapsed;
    if ((previousTick || tick<=1) && delta) {
        elapsed=static_cast<float>(static_cast<double>(delta)*static_cast<double>(step));
        if(previousTime>=0.0f) elapsed=elapsed-(1.0f-previousTime)*step;
        if(fraction>=0.0f) elapsed=fraction*step+elapsed;
    } else if (!(fraction>=0.0f) || !(previousTime>=0.0f)) elapsed=0.0f;
    else {
        float difference=(fraction-previousTime)*step;
        float negative=-0.0f-difference;
        uint32_t bits,sign;
        std::memcpy(&bits,&difference,4); std::memcpy(&sign,&negative,4);
        bits&=static_cast<uint32_t>(static_cast<int32_t>(sign)>>31);
        std::memcpy(&elapsed,&bits,4);
    }
    std::memcpy(context,static_cast<unsigned char*>(host)+0x80,64);
    void* rig=Field<void*>(body,0x20);
    Field<void*>(context,0x40)=rig;
    Field<void*>(context,0x44)=host;
    Field<void*>(context,0x48)=event;
    void* aux=Field<void*>(body,0x24);
    Field<void*>(context,0x4C)=aux?Field<void*>(aux,0xC):nullptr;
    Field<float>(context,0x50)=elapsed;
    Field<uint32_t>(context,0x54)=2;
    void* settings=Global<void*>(0x011CD800);
    bool blend=settings && Field<uint8_t>(settings,54);
    bool cache=settings && Field<uint8_t>(settings,55);
    void* pose=nullptr;
    if (blend && !(flagsB&4) && fraction>=0.0f && Root(0)) {
        Assign(pose,NewPose(rig));
        CallGameMethod<void,0x005743A0>(pose);
        void* blender=CallGameMethod<void*,0x0093F700>(Registry(2));
        CallGameMethod<void,0x0054C5C0>(Registry(2),blender);
        void* left=Root(0);
        void* right=Root(1);
        if(cache && ((flagsA|flagsB)&2)) {
            left=CachedRoot(0);
            void* a=ObtainBuffer(body,rig,context,0,tick);
            CallGameMethod<void,0x0055D030>(left,a);
            right=CachedRoot(1);
            void* b=ObtainBuffer(body,rig,context,1,tick+1);
            CallGameMethod<void,0x0055D030>(right,b);
            Release(b); Release(a);
        }
        CallGameMethod<void,0x0054BF40>(left,1.0f-fraction);
        CallGameMethod<void,0x0054BF40>(right,fraction);
        CallGameVirtual<void,3>(blender,left);
        CallGameVirtual<void,3>(blender,right);
        Evaluate(2,context,pose);
        CallGameVirtual<void,4>(blender,right);
        CallGameVirtual<void,4>(blender,left);
    } else if (cache && (flagsB&2)) {
        Assign(pose,NewPose(rig));
        void* buffer=CallGameMethod<void*,0x00573DA0>(body,tick+1);
        if(buffer) {
            AddRef(buffer);
            CallGameMethod<void,0x0055CF10>(buffer,pose);
            Release(buffer);
        } else {
            CallGameMethod<void,0x005743A0>(pose);
            Evaluate(1,context,pose);
            CallGameMethod<void,0x00574200>(pose);
            CallGameMethod<void,0x00574220>(pose);
            CallGameMethod<void,0x005741E0>(pose);
            Assign(buffer,NewBuffer());
            CallGameMethod<void,0x0055D0C0>(buffer,pose);
            CallGameMethod<void,0x00573E20>(body,buffer,tick+1);
            Release(buffer);
        }
    } else {
        Assign(pose,NewPose(rig));
        CallGameMethod<void,0x005743A0>(pose);
        Evaluate(1,context,pose);
    }
    CallGameMethod<void,0x00573330>(body,pose);
    void* service=CallGameFunction<void*,0x005EB7C0>();
    rig=Field<void*>(body,0x20);
    if(service && rig) {
        auto category=CallGameMethod<uint32_t,0x005EC9E0>(service,static_cast<unsigned char*>(rig)+8)&0xCF00;
        if(category==512 || category==1024 || category==1280) CallGameMethod<void,0x005737B0>(body,pose);
    }
    CallGameMethod<void,0x006FC9E0>(host);
    CallGameMethod<void,0x006F4ED0>(host);
    CallGameMethod<void,0x006FAC70>(host);
    if(Field<uint8_t>(context,0x160)) {
        Field<uint32_t>(context,0x154)=tick;
        Field<uint8_t>(context,0x160)=1;
        Field<float>(context,0x118)=fraction;
        void* record=CallGameMethod<void*,0x00573D40>(body,1);
        if(record) {
            Field<uint8_t>(context,0x160)=1;
            CallGameMethod<void,0x0093F740>(record,context+0xE0);
        }
    }
    Release(pose);
    settings=Global<void*>(0x011CD800);
    if(settings) {
        if(Field<uint8_t>(settings,47)) {
            void* renderer=CallGameFunction<void*,0x006E8330>();
            alignas(16) unsigned char matrix[64];
            CallGameMethod<void,0x006FAF40>(host,matrix);
            CallGameFunction<void,0x00941230>(host,static_cast<unsigned char*>(renderer)+808,matrix,0,0,0,0,0,0,0,0,0);
        }
        if(Field<uint8_t>(settings,46)) CallGameFunction<void,0x0093ED00>(host);
    }
}
}
#endif
uint8_t Update9420E0(uint32_t tick,M156Stream* first,M156Stream* second,float fraction,void* source) {
#if defined(_M_IX86)
    void* host=source?CallGameVirtual<void*,3>(source,reinterpret_cast<void*>(0x017F1D88)):nullptr;
    bool enabled=fraction>=0.0f;
    uint32_t sizeA=ReadWord(first,enabled);
    uint32_t sizeB=ReadWord(second,true);
    unsigned char* beginA=ReadPosition(first,enabled);
    unsigned char* beginB=ReadPosition(second,true);
    uint32_t flagsA=ReadWord(first,enabled);
    uint32_t flagsB=ReadWord(second,true);
    bool dirty=false;
    if(host) {
        void* body=CallGameMethod<void*,0x006F4A50>(host);
        if(body) {
            dirty=CallGameMethod<uint8_t,0x00573720>(body)!=0;
            if(dirty) CallGameMethod<void,0x00573E80>(body);
        }
        if(dirty || CallGameMethod<uint8_t,0x006F4F50>(host)) {
            InitRegistries(); ClearRegistries();
            if(Readable(first) && enabled && (flagsA&1)) CallGameMethod<void,0x0054C5A0>(Registry(0),first);
            if(Readable(second) && (flagsB&1)) CallGameMethod<void,0x0054C5A0>(Registry(1),second);
            if(!(Field<uint8_t>(host,670)&4) &&
               (dirty || CallGameMethod<uint8_t,0x006F4F50>(host) || !Global<uint32_t>(0x011DC488))) {
                body=CallGameMethod<void*,0x006F4A50>(host);
                if(body && Root(1)) EvaluateDriver(tick,fraction,host,body,flagsA,flagsB);
            }
            ClearRegistries();
            return 1;
        }
    }
    if(Readable(first) && enabled) CallGameMethod<void,0x00593F40>(first,beginA+sizeA);
    if(Readable(second)) CallGameMethod<void,0x00593F40>(second,beginB+sizeB);
    return 1;
#else
    (void)tick; (void)first; (void)second; (void)fraction; (void)source;
    NotRewritten("M156 original pose-update dependencies");
#endif
}

}
