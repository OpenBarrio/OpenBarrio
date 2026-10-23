#include "ts3/gameutils/cls_a18870_GameUtils.h"

#include <cstdlib>
#include <cstring>

namespace ts3::gameutils {
namespace {
uint8_t* B(void* p) { return static_cast<uint8_t*>(p); }
const uint8_t* B(const void* p) { return static_cast<const uint8_t*>(p); }
uint32_t U32(const void* p, size_t off) { uint32_t v; std::memcpy(&v, B(p)+off, 4); return v; }
void W32(void* p, size_t off, uint32_t v) { std::memcpy(B(p)+off, &v, 4); }
uint16_t U16(const void* p, size_t off) { uint16_t v; std::memcpy(&v, B(p)+off, 2); return v; }
void W16(void* p, size_t off, uint16_t v) { std::memcpy(B(p)+off, &v, 2); }
uint32_t Ptr32(const void* p) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p)); }
void* GamePtr(uint32_t p) { return reinterpret_cast<void*>(static_cast<uintptr_t>(p)); }

template<class R, class... A> R Callback(void* self, size_t off, A... args)
{
    using Fn = R(__cdecl*)(void*, A...);
    const uint32_t address = U32(self, off);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(address))(self, args...);
}

// 0x00A191F0. The unusual ESI self convention is private to this class.
int32_t SendRequest(void* self, int32_t code, char* url, int32_t payload)
{
    W32(self, 0x16164, 0);
    const char* const savedUrl = reinterpret_cast<const char*>(B(self) + 0x15D20);
    char oldUrl[1024];
    std::strcpy(oldUrl, savedUrl); // original has no bound check on the fixed stack copy
    const int32_t previousSize = static_cast<int32_t>(U32(self, 0x16130));
    W32(self, 0x16160, static_cast<uint32_t>(code));
    ts3::CallGameFunction<void, 0x00A0F650>(self, url);

    const uint32_t customHandler = U32(self, 0x3000);
    if (customHandler) {
        using Fn = int32_t(__cdecl*)(void*, char*, const char*, uint32_t);
        const int32_t result = reinterpret_cast<Fn>(static_cast<uintptr_t>(customHandler))(
            self, url, reinterpret_cast<const char*>(B(self)+0x15D20), U32(self,0x16130));
        W32(self, 0x16164, static_cast<uint32_t>(result));
        if (result) return result;
    } else if (U32(self, 0x3010) && B(self)[0x15D20]) {
        bool reconnect = !U16(self, 0x16134) || U32(self, 0x3080) == 0xFFFFFFFFu ||
            std::strcmp(reinterpret_cast<const char*>(reinterpret_cast<const char*>(B(self)+0x15D20)), oldUrl) != 0 ||
            static_cast<int32_t>(U32(self, 0x16130)) != previousSize;
        const uint32_t reuse = U32(self, 0x3024);
        if (!reconnect && reuse) {
            using Fn = int32_t(__cdecl*)(void*);
            reconnect = reinterpret_cast<Fn>(static_cast<uintptr_t>(reuse))(self) != 0;
        } else if (!reuse) reconnect = true;
        if (reconnect) {
            W32(self, 0x10, U32(self,0x10) & ~4u);
            W16(self, 0x16134, 0);
            ts3::CallGameFunction<void, 0x00A0D920>(self);
            if (url && std::strncmp(url, "soap.udp:", 9) == 0)
                W32(self, 0x10, U32(self,0x10) | 4u);
            using Connect = int32_t(__cdecl*)(void*, char*, const char*, uint32_t);
            const int32_t connected = reinterpret_cast<Connect>(static_cast<uintptr_t>(U32(self,0x3010)))(
                self, url, reinterpret_cast<const char*>(B(self)+0x15D20), U32(self,0x16130));
            W32(self, 0x3080, static_cast<uint32_t>(connected));
            if (static_cast<int32_t>(U32(self,0x16164))) return static_cast<int32_t>(U32(self,0x16164));
            W16(self,0x16134, static_cast<uint16_t>((U32(self,0x10)>>4)&1));
        }
    }

    const int32_t timeout = ts3::CallGameFunction<int32_t, 0x00A0EFE0>(self);
    if (ts3::CallGameFunction<int32_t, 0x00A137A0>(self)) return static_cast<int32_t>(U32(self,0x16164));
    if (code != 2000) W32(self, 8, (U32(self,8)&0xFFFFFFFCu)|1u);
    const int32_t size = ts3::CallGameFunction<int32_t, 0x00A10A20>(self, payload);
    const uint32_t flags = U32(self,8);
    W32(self,0x16120,static_cast<uint32_t>(size));
    if ((flags&3u)==2 || (flags&0x40u) || !url) return code==2000 ? 0 : ProcessResponse(static_cast<GameUtils*>(self));
    W32(self,8, flags & 0xFFFFFBFCu);
    if (flags&3u) W32(self,8,U32(self,8)|1u);
    using Send = int32_t(__cdecl*)(void*,char*,const char*,uint32_t,char*,int32_t,int32_t);
    const int32_t result = reinterpret_cast<Send>(static_cast<uintptr_t>(U32(self,0x2FD0)))(
        self,url,reinterpret_cast<const char*>(B(self)+0x15D20),U32(self,0x16130),reinterpret_cast<char*>(B(self)+0x15920),payload,timeout);
    W32(self,0x16164,static_cast<uint32_t>(result));
    if (!result) {
        if ((flags&3u)==3 && ts3::CallGameFunction<int32_t,0x00A12F80>(self))
            return static_cast<int32_t>(U32(self,0x16164));
        W32(self,8,flags);
        if (code==2000) return 0;
        return ProcessResponse(static_cast<GameUtils*>(self));
    }
    return result;
}

// 0x00A19ED0. Split space-separated endpoint alternatives and retry on error 28.
int32_t ProcessAlternatives(void* self, int32_t code, char* text, int32_t payload)
{
    if (!text || !std::strchr(text,' ')) {
        SendRequest(self,code,text,payload);
        return static_cast<int32_t>(U32(self,0x16164));
    }
    char* separator = std::strchr(text,' ');
    char* token = static_cast<char*>(std::malloc(std::strlen(text)+1));
    // EA's original assumes allocation succeeds and uses strncpy + explicit terminators.
    for (;;) {
        size_t len = static_cast<size_t>(separator-text);
        std::strncpy(token,text,len);
        token[len]=0;
        SendRequest(self,code,token,payload);
        if (static_cast<int32_t>(U32(self,0x16164)) != 28) break;
        if (!*separator) break;
        while (*separator==' ') ++separator;
        char* next = std::strchr(separator,' ');
        if (!next) next = separator + std::strlen(separator);
        text = separator;
        len = static_cast<size_t>(next-text);
        std::strncpy(token,text,len);
        token[len]=0;
        separator=next;
    }
    std::free(token);
    return static_cast<int32_t>(U32(self,0x16164));
}
} // namespace

// TS3W.exe 0x00A18870. State-machine response processing.
int32_t ProcessResponse(GameUtils* self)
{
    uint8_t* const o=self->bytes;
    if (U32(o,0x16198)) {
        W32(GamePtr(U32(o,0x161A0)),0,U32(o,0x1619C));
        const uint32_t head=U32(o,0x16198);
        W32(o,0x1619C,U32(GamePtr(head),0)); W32(GamePtr(head),0,0); W32(o,0x161A0,head);
    }
    if (ts3::CallGameFunction<int32_t,0x00A16BE0>(self) ||
        ts3::CallGameFunction<int32_t,0x00A17680>(self)) return static_cast<int32_t>(U32(o,0x16164));
    const bool mode=(o[8]&3)!=0;
    W32(o,0x161AC,0); W32(o,0x161B0,0); W32(o,0x161B4,0);
    W32(o,0x16198,0); W32(o,0x1619C,0); W32(o,0x161A0,0);
    if (mode) {
        if (ts3::CallGameFunction<int32_t,0x00A12F80>(self)) return static_cast<int32_t>(U32(o,0x16164));
        const uint32_t flags=U32(o,8);
        if ((flags&3)==2) {
            if (!(flags&0x40)) {
                const int32_t status=static_cast<int32_t>(U32(o,0x16160));
                W32(o,8,flags-1);
                int32_t r;
                if (status<2000) {
                    if (status==1000) goto drain;
                    r=Callback<int32_t>(self,0x2FEC,status,U32(GamePtr(U32(o,0x64)),8));
                } else {
                    using F=int32_t(__cdecl*)(void*,char*,char*,int32_t,char*,int32_t,int32_t);
                    r=reinterpret_cast<F>(static_cast<uintptr_t>(U32(o,0x2FD0)))(self,reinterpret_cast<char*>(o+0x15920),reinterpret_cast<char*>(o+0x15D20),static_cast<int32_t>(U32(o,0x16120)),reinterpret_cast<char*>(o+0x15520),static_cast<int32_t>(U32(o,0x16130)),static_cast<int32_t>(U32(GamePtr(U32(o,0x64)),8)));
                }
                W32(o,0x16164,static_cast<uint32_t>(r));
                if (r || ts3::CallGameFunction<int32_t,0x00A12F80>(self)) return static_cast<int32_t>(U32(o,0x16164));
                W32(o,8,U32(o,8)+1);
            }
 drain:
            uint32_t owner=U32(o,0x64), node=U32(GamePtr(owner),4), reversed=0;
            while (node) { const uint32_t next=U32(GamePtr(node),0); W32(GamePtr(node),0,reversed); reversed=node; node=next; }
            W32(GamePtr(owner),4,reversed);
            node=reversed;
            while (node) {
                const int32_t pending=ts3::CallGameFunction<int32_t,0x00A0D390>(self,0);
                using Emit=int32_t(__cdecl*)(void*,void*,int32_t);
                const int32_t r=reinterpret_cast<Emit>(static_cast<uintptr_t>(U32(o,0x301C)))(self,GamePtr(node+8),pending);
                W32(o,0x16164,static_cast<uint32_t>(r)); if (r) break;
                owner=U32(o,0x64); const uint32_t next=U32(GamePtr(owner),4);
                if (next) { W32(GamePtr(owner),4,U32(GamePtr(next),0)); std::free(GamePtr(next)); node=U32(GamePtr(owner),4); }
                else node=0;
            }
            ts3::CallGameFunction<void,0x00A0D3B0>(self,nullptr);
            if (static_cast<int32_t>(U32(o,0x16164))) return static_cast<int32_t>(U32(o,0x16164));
        } else if ((flags&3)==3) {
            const int32_t r=Callback<int32_t>(self,0x301C,"\r\n0\r\n\r\n",7);
            W32(o,0x16164,static_cast<uint32_t>(r)); if (r) return r;
        }
    }
    W32(o,0x10,U32(o,0x10)&~0x80000u); W32(o,0x30A8,0); W16(o,0x15510,9); return 0;
}

// TS3W.exe 0x00A19470. Constructor body delegates initialization to the base helpers.
int32_t __fastcall Construct(GameUtils* self, void*)
{
    W32(self,0,0x0105830C);
    ts3::CallGameFunction<void,0x00A0E000>(self,0);
    ts3::CallGameFunction<void,0x00A11DF0>(self);
    return ts3::CallGameFunction<int32_t,0x00A186A0>(self);
}

// TS3W.exe 0x00A1A270. Public wrapper selects request mode 2000.
int32_t ProcessRequest(GameUtils* self, char* request, int32_t flags)
{
    return ProcessAlternatives(self,2000,request,flags);
}
} // namespace ts3::gameutils



extern "C" int32_t __cdecl TS3GameUtilsProcess(void* self)
{
    return ts3::gameutils::ProcessResponse(static_cast<ts3::gameutils::GameUtils*>(self));
}
extern "C" int32_t __fastcall TS3GameUtilsConstruct(void* self, void* edx)
{
    return ts3::gameutils::Construct(static_cast<ts3::gameutils::GameUtils*>(self), edx);
}
extern "C" int32_t __cdecl TS3GameUtilsRequest(void* self, char* url, int32_t flags)
{
    return ts3::gameutils::ProcessRequest(static_cast<ts3::gameutils::GameUtils*>(self), url, flags);
}
