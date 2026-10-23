// Native test interfaces, deliberately NOT claimed ABI-compatible with TS3W.
// Raw original dispatch still requires per-entry assembly recovery before use.
#pragma once
#include "ts3/resource/m03_layouts.h"
#include "ts3/resource/m03_abi.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>
namespace ts3::resource::m03 {
// Native dispatch has the same typed results but is never a game vtable object.
struct NativeDispatch {
    virtual ~NativeDispatch()=default;
    virtual ResourceManager* Manager()=0;
    virtual DirectoryManager* Directory()=0;
};
class Boundary final {
    NativeDispatch* native_=nullptr;
#if defined(_M_IX86)
    OriginalDispatch original_{};
#endif
public:
    explicit Boundary(NativeDispatch* native=nullptr):native_(native) {}
#if defined(_M_IX86)
    explicit Boundary(OriginalDispatch original):original_(original) {}
#endif
    ResourceManager* Manager() const;
    DirectoryManager* Directory() const;
};
struct Record {
    virtual ~Record()=default;
    virtual int AddRef()=0;
    virtual int Release()=0;
    virtual size_t Read(void*,size_t)=0;
};
struct Database {
    virtual ~Database()=default;
    virtual Record* Open(const Key&)=0;
};
struct Factory {
    virtual ~Factory()=default;
    virtual Record* Create(const Key&)=0;
};
struct KeyPredicate {
    virtual ~KeyPredicate()=default;
    virtual bool Accept(const Key&)=0;
};
// Injected native contracts only, NOT raw ScriptCore/Service vtables. IDs are
// managed tokens, never encoded host pointers. Returned values are owned copies;
// BorrowHandle remains valid until matching ReleaseHandle or service reset.
struct ScriptServices {
    virtual ~ScriptServices()=default;
    virtual uint32_t String(std::u16string)=0;
    virtual uint32_t Array(std::vector<uint8_t>)=0;
    virtual uint32_t Handle(Record&)=0;
    virtual void ReleaseHandle(uint32_t)=0;
    virtual std::u16string ReadString(uint32_t) const=0;
    virtual std::vector<uint8_t> ReadArray(uint32_t) const=0;
    virtual Record& BorrowHandle(uint32_t) const=0;
};
enum class TextDomain { Config, Registry, Css, Debug };
struct TextServices {
    virtual ~TextServices()=default;
    // Missing and present-but-empty must remain distinguishable.
    virtual std::optional<std::string> Lookup(TextDomain,const std::string&)=0;
    virtual void Assign(TextDomain,const std::string&,std::string)=0;
};
// Fail explicitly on either architecture unless a native/fake boundary is
// injected. Do not silently invoke an image address or truncate a pointer.
template<class T> T& Require(T* service) {
    if(!service) throw std::logic_error("M03 boundary not injected");
    return *service;
}
}
