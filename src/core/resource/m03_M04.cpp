#include "ts3/resource/m03_M04.h"
#include <cmath>
#include <limits>
#include <cstring>
#include <cwchar>
#if defined(_M_IX86)
#include <windows.h>
#endif
namespace ts3::resource::m04 {
uint32_t WaitUntil(WaitTimer* timer,const WaitDispatch* dispatch) {
    auto counter=[&]() -> int64_t {
        if(dispatch) return dispatch->counter();
#if defined(_M_IX86)
        LARGE_INTEGER value; QueryPerformanceCounter(&value); return value.QuadPart;
#else
        AbortNotRewritten("M04 performance counter"); return 0;
#endif
    };
    for(;;) {
        const uint64_t difference=uint64_t(timer->deadline)-uint64_t(counter());
        if(int64_t(difference)<0) return uint32_t(difference);
        const int64_t remaining=int64_t(uint64_t(timer->deadline)-uint64_t(counter()));
        int64_t milliseconds;
#if defined(_M_IX86)
        // _ftol2 truncates an extended x87 product, irrespective of rounding mode.
        const float factor=timer->factor;
        __asm {
            fild remaining
            fmul factor
            mov eax,0x00F7EC76
            call eax
            mov dword ptr [milliseconds],eax
            mov dword ptr [milliseconds+4],edx
        }
#else
        const long double product=static_cast<long double>(remaining)*timer->factor;
        milliseconds=std::numeric_limits<int64_t>::min();
        if(product>=-9223372036854775808.0L && product<9223372036854775808.0L)
            milliseconds=static_cast<int64_t>(product);
#endif
        const uint32_t delay=milliseconds<=0?1u:milliseconds>100?100u:uint32_t(milliseconds);
        if(dispatch) dispatch->sleep(delay);
        else CallGameFunction<void,0x004E1320>(&delay);
    }
}
uintptr_t StopCache(CacheRegistration* self,void** singleton) {
    uintptr_t result=0; // No defined EAX in the original's null branch.
    if(self->database) {
        CallGameVirtual<void,7>(self->database);
        CallGameVirtual<void,2>(self->database); // reload: either callback may replace it
        auto* ref=static_cast<unsigned char*>(self->database)+sizeof(void*);
        result=CallGameVirtual<uintptr_t,1>(ref);
        self->database=nullptr;
    }
    if(singleton) *singleton=nullptr;
#if defined(_M_IX86)
    else *reinterpret_cast<void**>(uintptr_t(0x011D3E40))=nullptr;
#else
    else AbortNotRewritten("M04 cache singleton");
#endif
    return result;
}
namespace {
#if defined(_M_IX86)
template<class T> T& At(void* p,size_t offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
template<class T=void> T* Address(uintptr_t address) { return reinterpret_cast<T*>(address); }
void* Allocate(size_t n,uintptr_t name,bool resource=false,uintptr_t file=0,uint32_t line=0) {
    if(resource) return CallGameFunction<void*,0x004E41F0>(n,Address<const char>(name),0u,0u,Address<const char>(file),line);
    return CallGameFunction<void*,0x004E3F90>(n,Address<const char>(name),0u,0u,Address<const char>(file),line);
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
struct String { wchar_t* begin; wchar_t* end; uintptr_t capacity; const char* tag; };
void FreeString(const String& s) {
    if(int32_t((s.capacity-uintptr_t(s.begin))&~uintptr_t(1))>2 && s.begin) Free(s.begin);
}
struct PathInfo { String path; void* database; };
struct Control { uint32_t index; wchar_t* begin; wchar_t* end; uint32_t spare[2]; };
struct ListNode { ListNode *next,*prev; void* database; };
struct List { ListNode* next; ListNode* prev; const char* tag; };
struct Config { uint32_t words[8]; };
#endif
}
uintptr_t StartSystem(void* self) {
#if defined(_M_IX86)
    const auto section=Address<const wchar_t>(0x00FFD330);
    At<uint8_t>(self,0x1e9)=uint8_t(CallGameFunction<uintptr_t,0x0058AAD0>(section,Address<const wchar_t>(0x00FFEAEC),1u));
    void* pool=Allocate(0x80228,0x00FFEAC8);
    At<void*>(self,0x1d4)=pool;
    if(!pool) return 0;
    At<void**>(pool,0)=Address<void*>(0x00FFDFC8);
    InitializeCriticalSection(Address<CRITICAL_SECTION>(uintptr_t(pool)+8));
    std::memset(static_cast<unsigned char*>(pool)+0x80028,0xff,0x200);
    struct DeviceConfig { uint32_t kind,capacity; void* allocator; } deviceConfig{2,1024,pool};
    void* device=CallGameFunction<void*,0x00402100>(&deviceConfig);
    if(!device || !*Address<void*>(0x011AF588)) return uintptr_t(device)&~uintptr_t(0xff);
    if(uint8_t(CallGameFunction<uintptr_t,0x0058AAD0>(section,Address<const wchar_t>(0x00FFEAAC),0u)))
        CallGameFunction<void,0x00C0FD60>();
    const int quantum=CallGameFunction<int,0x0058ABA0>(section,Address<const wchar_t>(0x00FFEA94),0u);
    if(quantum>0) At<int>(device,0x24)=quantum;
    uint32_t info[6]; CallGameMethod<void,0x004E2960>(info); info[2]=2; info[5]=0x00FFEA80;
    std::memcpy(static_cast<unsigned char*>(device)+12,info,sizeof info);
    alignas(4) unsigned char ranges[20];
    CallGameMethod<void,0x0072A9D0>(ranges);
    if(uint8_t(CallGameMethod<uintptr_t,0x0072AD90>(ranges,section,Address<void>(0x00FFEA68))) && At<void*>(ranges,4)!=At<void*>(ranges,8)) {
        void* filter=Allocate(20,0x00FFEA44);
        if(filter) filter=CallGameMethod<void*,0x0072A9D0>(filter);
        At<void*>(self,0x7f0)=filter;
        if(filter) {
            CallGameMethod<void,0x0072AC90>(filter,ranges);
            float latency=CallGameFunction<float,0x0058ABF0>(section,Address<void>(0x00FFEA18),10.0f);
            float bandwidth=CallGameFunction<float,0x0058ABF0>(section,Address<void>(0x00FFE9F8),500000.0f);
            void* driver=Allocate(32,0x00FFE9DC);
            if(driver) driver=CallGameMethod<void*,0x00739620>(driver,Address<const char>(0x00FFD838),latency,bandwidth);
            At<void*>(self,0x7e8)=driver;
            if(driver) At<void*>(self,0x7ec)=CallGameMethod<void*,0x004021D0>(device,driver,0u);
        }
    }
    void* driver=Allocate(16,0x00FFE9B4);
    if(driver) driver=CallGameMethod<void*,0x0072E220>(driver,Address<const char>(0x00FFE9AC));
    At<void*>(self,0x7f4)=driver;
    if(driver) At<void*>(self,0x7f8)=CallGameMethod<void*,0x004021D0>(device,driver,0u);
    driver=Allocate(104,0x00FFE984);
    if(driver) driver=CallGameMethod<void*,0x0072E240>(driver,Address<const char>(0x00FFE978),Address<const char>(0x00FFE9AC));
    At<void*>(self,0x7fc)=driver;
    if(driver) At<void*>(self,0x800)=CallGameMethod<void*,0x004021D0>(device,driver,1u);
    void* map=Allocate(32,0x00FFE958);
    const char* tag=Address<const char>(0x00FFE934);
    if(map) map=CallGameMethod<void*,0x00AA3730>(map,&tag);
    At<void*>(self,0x74c)=map;
    auto finish=[&](bool ok) {
        return (CallGameMethod<uintptr_t,0x0072A9F0>(ranges)&~uintptr_t(0xff))|uintptr_t(ok);
    };
    if(!uint8_t(CallGameMethod<uintptr_t,0x004AFD80>(self))) return finish(false);
    CallGameFunction<void,0x004AFD30>(uintptr_t(self)==8?nullptr:self);
    Config config; tag=Address<const char>(0x00FFE918);
    CallGameMethod<void,0x00AA3730>(&config,&tag);
    uint32_t index=0;
    while((At<uint16_t*>(self,0x740)-At<uint16_t*>(self,0x73c))!=0) {
        auto* begin=At<uint16_t*>(self,0x73c);
        if(begin[index]) {
            auto* value=begin+index; String ignored;
            // Only AL is consumed by this helper's third argument.
            CallGameMethod<void,0x007357F0>(&config,&ignored,&value,0u);
        }
        begin=At<uint16_t*>(self,0x73c); auto* end=At<uint16_t*>(self,0x740);
        if(begin==end) break;
        auto* found=begin; while(found!=end && *found!=uint16_t(index)) ++found;
        if(found==end) break;
        index=uint32_t(found-begin)+1;
        if(index>=uint32_t(end-begin)) break;
    }
    List list; list.next=list.prev=reinterpret_cast<ListNode*>(&list); list.tag=Address<const char>(0x00FFE3A0);
    auto cleanup=[&]() {
        auto* node=list.next;
        while(node!=reinterpret_cast<ListNode*>(&list)) { auto* next=node->next; Free(node); node=next; }
        CallGameMethod<void,0x007F3790>(&config,config.words[1],config.words[2]);
        config.words[3]=0;
        if(config.words[2]>1) Free(Address<void>(config.words[1]));
    };
    auto failedPath=[&]() {
        CallGameMethod<void,0x006263F0>(&list);
        CallGameMethod<void,0x00735BC0>(&config);
        return finish(false);
    };
    uint32_t pathIndex=0;
    for(auto* path=At<PathInfo*>(self,0x71c);path!=At<PathInfo*>(self,0x720);++path,++pathIndex) {
        if(path->path.begin==path->path.end) continue;
        if(!uint8_t(CallGameFunction<uintptr_t,0x004DBF60>(path->path.begin))) return failedPath();
        void* db=Allocate(1124,0x00FFE8F8);
        if(db) db=CallGameMethod<void*,0x004A4740>(db);
        CallGameMethod<void,0x004A43C0>(db,path->path.begin); path->database=db;
        auto* controlEnd=At<Control*>(self,0x730);
        for(auto* control=At<Control*>(self,0x72c);control!=controlEnd;++control) {
            if(control->index!=pathIndex) continue;
            String file; CallGameMethod<void,0x00408EB0>(&file,path);
            CallGameFunction<void,0x004EDD80>(&file);
            CallGameMethod<void,0x00408B90>(&file,control->begin,control->end);
            if(!uint8_t(CallGameFunction<uintptr_t,0x004DBBA0>(file.begin))) { FreeString(file); return failedPath(); }
            alignas(4) unsigned char absolute[0x410]; // original stack span 0x10c..0x51c
            CallGameMethod<void,0x004EDA20>(absolute,file.begin);
            void* chars=CallGameMethod<void*,0x004ED5C0>(absolute);
            String name{nullptr,nullptr,0,Address<const char>(0x00FFE3A0)};
            CallGameMethod<void,0x0040B700>(&name,chars);
            CallGameMethod<void,0x004EDA50>(absolute);
            struct Names { wchar_t** begin; wchar_t** end; wchar_t** capacity; const char* tag; } names;
            names.begin=static_cast<wchar_t**>(Allocate(4,0x00FFE8D8,false,0x00F98980,0xe7));
            names.end=names.capacity=names.begin+1; names.tag=Address<const char>(0x00FFE8D8); *names.begin=name.begin;
            void* normalized=CallGameMethod<void*,0x00F626F0>(absolute);
            CallGameMethod<void,0x00737D70>(static_cast<unsigned char*>(self)-8,normalized,&names,db,&config,&list);
            if(names.begin) Free(names.begin);
            FreeString(name); FreeString(file);
        }
    }
    for(auto* node=list.next;node!=reinterpret_cast<ListNode*>(&list);node=node->next) {
        void* child=node->database; if(!child) continue;
        void* primary=static_cast<unsigned char*>(self)-8;
        void* db=CallGameVirtual<void*,8>(primary,CallGameVirtual<const wchar_t*,10>(child));
        if(!db) {
            String path{Address<wchar_t>(0x011C7E54),Address<wchar_t>(0x011C7E54),0x011C7E56,Address<const char>(0x00FFE0F8)};
            const wchar_t* name=CallGameVirtual<const wchar_t*,10>(child);
            CallGameMethod<void,0x00408E20>(&path,name,name+std::wcslen(name));
            db=Allocate(1124,0x00FFE8F8);
            if(db) db=CallGameMethod<void*,0x004A4740>(db);
            CallGameMethod<void,0x004A43C0>(db,CallGameVirtual<const wchar_t*,10>(child));
            PathInfo entry{path,db}; auto* end=At<PathInfo*>(self,0x720);
            if(uintptr_t(end)<uintptr_t(At<PathInfo*>(self,0x724))) {
                At<PathInfo*>(self,0x720)=end+1;
                if(end) { CallGameMethod<void,0x00408EB0>(end,&entry); end->database=db; }
            } else CallGameMethod<void,0x00737800>(static_cast<unsigned char*>(self)+0x71c,end,&entry);
            FreeString(path);
        }
        CallGameMethod<void,0x004A4930>(db,child,1u);
    }
    for(unsigned n=0;n<2;++n) {
        void* cache=Allocate(160,n?0x00FFE88C:0x00FFE8AC,true);
        if(cache) cache=CallGameMethod<void*,0x00732910>(cache,*Address<void*>(0x011CB864));
        At<void*>(self,n?0x1d8:0x1dc)=cache;
        // EA dereferences allocation failure; deliberately no added null guard.
        InterlockedExchangeAdd(&At<LONG>(cache,4),1);
        CallGameVirtual<void,1>(cache);
        if(!n) {
            CallGameVirtual<void,5>(cache,static_cast<unsigned char*>(self)+0x1e0);
            At<uint32_t>(cache,0x8c)=0x00f00000;
        } else CallGameMethod<void,0x00733140>(cache);
        CallGameVirtual<void,21>(self,1u,cache,n?0u:1000u);
    }
    void* callback=Allocate(88,0x00FFE870);
    if(callback) callback=CallGameMethod<void*,0x0072A5D0>(callback);
    CallGameFunction<void,0x0072A520>(callback);
    void* loader=CallGameFunction<void*,0x00728880>();
    if(!uint8_t(CallGameVirtual<uintptr_t,1>(loader))) {
        CallGameVirtual<void,2>(self); cleanup(); return finish(false);
    }
    CallGameFunction<void,0x00727290>(loader);
    void* settings=CallGameFunction<void*,0x0058B880>();
    if(settings) {
        alignas(4) unsigned char fixedString[0x54];
        CallGameMethod<void,0x00735EB0>(fixedString,section);
        const wchar_t* value;
        if(uint8_t(CallGameMethod<uintptr_t,0x0058C380>(settings,At<void*>(fixedString,0),Address<const wchar_t>(0x00FFE850),&value))) {
            if(!std::wcscmp(value,Address<const wchar_t>(0x00FCC1FC))) At<uint8_t>(self,0x75c)=1;
            else if(!std::wcscmp(value,Address<const wchar_t>(0x00FCC1F8))) At<uint8_t>(self,0x75c)=0;
        }
        if(At<void*>(fixedString,0)!=At<void*>(fixedString,0x10)) FreeString(At<String>(fixedString,0));
    }
    cleanup(); return finish(true);
#else
    (void)self; AbortNotRewritten("M04 ResourceSystem startup external SDK and container bridges"); return 0;
#endif
}
}
