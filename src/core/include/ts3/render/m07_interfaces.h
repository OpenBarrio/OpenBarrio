#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
#include <cstddef>
namespace ts3::render::m07 {
using HResult=int32_t;
constexpr bool Failed(HResult hr) { return hr<0; }
// Native injection interfaces are NOT EA binary vtables. Borrowed callbacks,
// engine resources and COM objects deliberately have distinct lifetime APIs.
struct ComObject {
    virtual ~ComObject()=default;
    virtual uint32_t AddRef()=0;
    virtual uint32_t Release()=0;
};
struct EngineResource {
    virtual ~EngineResource()=default;
    virtual uint32_t RetainResource()=0;
    virtual uint32_t ReleaseResource()=0;
};
enum class ObjectKind : uint32_t { D3D9, Device, SwapChain, VertexBuffer, IndexBuffer, Texture, PixelShader, VertexShader };
enum class Operation : uint32_t { Create, Reset, Present, Lock, Unlock, State, Draw, Constants, Lost, Destroy };
struct RenderDevice : ComObject {
    virtual HResult Invoke(Operation,uint32_t key,const void* bytes,size_t count)=0;
    virtual HResult Create(ObjectKind,ComObject** output)=0;
};
struct AdjustedInterface { void* const* vtable; }; // receiver already adjusted
// Use &owner.secondary (native offset), not owner+8, when building native fakes.
using SecondaryInterface = AdjustedInterface;
// TS3W.exe 0x00611C60/00611C30: exact wrapping locked-xadd return counts.
uint32_t ComAddRef(int32_t* refs);
uint32_t ComRelease(int32_t* refs);
}
