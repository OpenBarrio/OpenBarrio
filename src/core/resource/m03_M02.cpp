#include "ts3/resource/m03_M02.h"
#include "ts3/io/memory_stream.h"
#include <string>
#include <cwchar>
#include <cstring>
#include <atomic>
namespace ts3::resource::m03::m02 {
namespace {
template<unsigned A,class R,class... Args> R Import(Args... args) {
#if defined(_M_IX86)
    return (*reinterpret_cast<R(__stdcall**)(Args...)>(A))(args...);
#else
    ((void)args,...); AbortNotRewritten("M02 platform import");return R{};
#endif
}
void* Image(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a;AbortNotRewritten("M02 original datum");return nullptr;
#endif
}
using Text=std::u16string;
// These reproduce EASTL left/right unsigned-count clamping, including npos arithmetic.
Text Left(const Text& s,uint32_t n) {return s.substr(0,n);}
Text Right(const Text& s,uint32_t n) {return n>=s.size()?s:s.substr(s.size()-n);}
uint32_t Find(const Text& s,char16_t c) {return uint32_t(s.find(c));}
void Trim(Text& s) {auto i=s.find_first_not_of(u' ');s=Right(s,uint32_t(s.size())-uint32_t(i));}
bool Prefix(const char16_t* a,const char16_t* b,size_t n) {
    return _wcsnicmp(reinterpret_cast<const wchar_t*>(a),reinterpret_cast<const wchar_t*>(b),n)==0;
}
}
uint8_t __cdecl Parse(const char16_t* section,const char16_t* value,ParseContext* ctx) {
    if(!Prefix(section,ctx->prefix,ctx->length)) return 1;
    Text all(value);uint32_t colon=Find(all,u':');
    Text type=Left(all,colon),names=Right(all,uint32_t(all.size())-colon-1);
    Trim(type);type=Right(type,uint32_t(type.size())-Find(type,u'x')-1);Trim(names);
    uint32_t typeId=uint32_t(std::wcstoul(reinterpret_cast<const wchar_t*>(type.c_str()),nullptr,16));
    uint32_t split;
    do {
        split=Find(names,u' ');Text name=Left(names,split);
        if(name.empty()) break;
        uint64_t hash;
        if(typeId==0x6b20c4f3) hash=CallGameFunction<uint64_t,0x0057A470>(name.c_str(),0u);
        else {char bytes[256];CallGameFunction<void,0x004FDDD0>(bytes,name.c_str(),256,-1);hash=CallGameFunction<uint64_t,0x007CFE10>(bytes);}
        Key key{hash,typeId,0};auto* keys=ctx->keys;
        if(keys->end<keys->capacity) {auto* p=keys->end++;if(p)*p=key;}
        else CallGameMethod<void,0x006D3810>(keys,keys->end,&key);
        names=Right(names,uint32_t(names.size())-split-1);Trim(names);
    } while(split!=uint32_t(-1));
    return 1;
}
Shadow* __fastcall Construct(Shadow* s,void*) {
    s->primary=Image(0x00fb9c58);
    CallGameMethod<void,0x0072CC60>(&s->database,0u);
    s->primary=Image(0x00ffe0d0);s->database=Image(0x00ffe078);s->reference=Image(0x00ffe068);s->index=nullptr;
    Import<0x00f95124,void>(s->mutex);
    std::atomic_ref<int32_t>(s->state).exchange(0);
    Import<0x00f953b8,int>(&s->tick);
    long double frequency=CallGameFunction<double,0x004F3480>(4u);
    s->delay=uint64_t(1.0L/frequency*2000.0L);
    s->stamp=0;s->attributes=0;s->mode=2;
    return s;
}
void __fastcall Refresh(Shadow* s,void*) {
    Import<0x00f9512c,void>(s->mutex);
    if(s->state==1) {
        CallGameMethod<void,0x0072C6C0>(&s->database,1u);
        void* raw=CallGameFunction<void*,0x004E3F90>(size_t(36),static_cast<const char*>(Image(0x00ffe194)),0u,0u,0u,0u);
        s->index=raw?CallGameMethod<void*,0x0072DB40>(raw):nullptr;
        if(s->index) {
            CallGameMethod<void,0x0072D890>(s->index);
            Keys keys{nullptr,nullptr,nullptr,static_cast<const char*>(Image(0x00ffe168))};
            CallGameMethod<void,0x0072C930>(&s->database,&keys,0u);
            CallGameMethod<void,0x0072DC00>(s->index,&keys);
            if(keys.begin) CallGameFunction<void,0x004E3F70>(keys.begin);
        }
        auto* path=CallGameMethod<const char16_t*,0x0072C920>(&s->database);
        s->stamp=CallGameFunction<uint64_t,0x004DBCF0>(path,2u);
        s->attributes=CallGameFunction<uint32_t,0x004DBC60>(path);
        CallGameMethod<void,0x0072C8F0>(&s->database);
        CallGameMethod<void,0x0072C6C0>(&s->database,0u);
        Import<0x00f953b8,int>(&s->tick);
        std::atomic_ref<int32_t>(s->state).exchange(0);
    }
    Import<0x00f95130,void>(s->mutex);
}
uintptr_t __fastcall Save(void* self,void*,const char16_t* source,const char16_t* prefix,uint8_t metadata,int endian) {
    io::MemoryStream stream;
    CallGameMethod<void,0x004DF0C0>(&stream,0u,0u,0u);
    CallGameMethod<void,0x004DEAC0>(&stream,1u,1.0f);
    uintptr_t r;
    auto u32=[&](uint32_t v,int e) {r=CallGameFunction<uintptr_t,0x004DDBD0>(&stream,v,e);return uint8_t(r)!=0;};
    auto clean=[&](uint8_t result) {
        stream.vtable=static_cast<void* const*>(Image(0x00fb5bcc));
        if(stream.buffer) r=CallGameMethod<uintptr_t,0x004DEA80>(stream.buffer);
        return (r&~uintptr_t(255))|result;
    };
    if(!u32(0x4b444e49,endian)||!u32(1,endian)) return clean(0);
    const size_t n=std::char_traits<char16_t>::length(prefix);
    auto* bytes=static_cast<uint8_t*>(self);void* mutex=bytes+0x218;
    CallGameMethod<void,0x004E16F0>(mutex,Image(0x00ffdfc4));
    auto** first=*reinterpret_cast<void***>(bytes+0x2b8);auto** last=*reinterpret_cast<void***>(bytes+0x2bc);
    uint32_t count=0;
    for(auto** p=first;p!=last;++p) if(Prefix(CallGameVirtual<const char16_t*,2>(*p),prefix,n)) ++count;
    auto unlock=[&]() {r=CallGameMethod<uintptr_t,0x004E17B0>(mutex);};
    if(!u32(count,endian)) {unlock();return clean(0);}
    // Original takes another begin snapshot after writing count, but retains the old end.
    first=*reinterpret_cast<void***>(bytes+0x2b8);
    for(auto** p=first;p!=last;++p) {
        void* watcher=*p;auto* path=CallGameVirtual<const char16_t*,2>(watcher);
        if(!Prefix(path,prefix,n)) continue;
        path+=n;while(*path==u'\\'||*path==u'/'||*path==u':') ++path;
        void* index=nullptr;uint64_t stamp,attr;
        auto fail=[&](bool destructor) {
            if(index) CallGameMethod<void,0x0072DB80>(index);
            unlock();
            if(destructor) {r=CallGameMethod<uintptr_t,0x004559D0>(&stream);return r&~uintptr_t(255);}
            return clean(0);
        };
        r=CallGameVirtual<uintptr_t,7>(watcher,&index,&stamp,&attr);
        if(!uint8_t(r)) return fail(false);
        r=CallGameFunction<uintptr_t,0x004DDEC0>(&stream,path,std::char_traits<char16_t>::length(path),0u);
        if(!uint8_t(r)) return fail(false);
        if(!metadata) stamp=attr=0;
        r=CallGameFunction<uintptr_t,0x004DDD20>(&stream,uint32_t(stamp),uint32_t(stamp>>32),endian);
        if(!uint8_t(r)) return fail(false);
        r=CallGameFunction<uintptr_t,0x004DDD20>(&stream,uint32_t(attr),uint32_t(attr>>32),endian);
        if(!uint8_t(r)) return fail(false);
        if(!u32(0x58444e49,endian)) return fail(true);
        uint32_t offset=CallGameMethod<uint32_t,0x004DEB70>(&stream,0u);
        if(!u32(0,1)) return fail(true);
        r=CallGameMethod<uintptr_t,0x0072D8F0>(index,&stream,endian);
        if(!uint8_t(r)) return fail(true);
        uint32_t end=CallGameMethod<uint32_t,0x004DEB70>(&stream,0u);
        uint32_t length=end-offset-4;std::memcpy(stream.Data()+offset,&length,4);
        if(index) CallGameMethod<void,0x0072DB80>(index);
    }
    unlock();
    alignas(void*) uint8_t file[552];
    CallGameMethod<void,0x004DB970>(file,source);
    r=CallGameMethod<uintptr_t,0x004DB4C0>(file,2u,2u,0u,0u);
    if(uint8_t(r)) {
        r=CallGameMethod<uintptr_t,0x004DB8A0>(file,stream.Data(),stream.size);
        if(uint8_t(r)) {
            uint8_t ok=uint8_t(CallGameMethod<uintptr_t,0x004DB600>(file));
            r=CallGameMethod<uintptr_t,0x004DB9D0>(file);return clean(ok);
        }
    }
    r=CallGameMethod<uintptr_t,0x004DB9D0>(file);return clean(0);
}
}
