#include "ts3/scratch/cls_4b40d0_Scratch.h"
#include <cctype>
#include <cstdlib>
#include <cstring>

namespace ts3::scratch {
namespace {
void** Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(address);
#else
    (void)address; return nullptr; // Game virtual dispatch remains bridged on x64.
#endif
}
uintptr_t Set(XmlString** target, const char* text, uint32_t count) { return CallGameMethod<uintptr_t,0xF61360>(target,text,count); }
uintptr_t Append(XmlString** target, const char* text, uint32_t count) { return CallGameMethod<uintptr_t,0xF61410>(target,text,count); }
uintptr_t Release(XmlString* p) { return CallGameFunction<uintptr_t,0x4E3F70>(p); }
struct Runtime { uint8_t padding[0x4B4]; void* (*allocate)(size_t); void (*release)(void*); };
TS3_X86_OFFSET(Runtime, allocate, 0x4B4);
TS3_X86_OFFSET(Runtime, release, 0x4B8);
Runtime* GetRuntime() {
#if defined(_M_IX86)
    return *reinterpret_cast<Runtime**>(0x121CAD0);
#else
    AbortNotRewritten("XML allocator singleton"); return nullptr;
#endif
}
void* Allocate(size_t size) {
    auto* r=GetRuntime();
    if(r->allocate) return r->allocate(size);
#if defined(_M_IX86)
    return (*reinterpret_cast<void*(__cdecl**)(size_t)>(0xF956E0))(size);
#else
    return std::malloc(size);
#endif
}
void FreeNode(void* p) {
    auto* r=GetRuntime();
    if(r->release) { r->release(p); return; }
#if defined(_M_IX86)
    (*reinterpret_cast<void(__cdecl**)(void*)>(0xF956E4))(p);
#else
    std::free(p);
#endif
}
// A game's FILE* is owned by msvcr80; do not pass it to the rewrite DLL's CRT.
template<class... Args> int PrintFile(std::FILE* file,const char* format,Args... args) {
#if defined(_M_IX86)
    using Print=int(__cdecl*)(std::FILE*,const char*,...);
    return (*reinterpret_cast<Print*>(0xF9571C))(file,format,args...);
#else
    return std::fprintf(file,format,args...);
#endif
}
bool Prefix(const char* p,const char* token) { return p && *p && std::strncmp(p,token,std::strlen(token))==0; }
void Error(XmlDocument* document, uint32_t code, uintptr_t address) {
    if(!document || document->error) return;
    document->error=1;
    document->errorCode=code;
#if defined(_M_IX86)
    const char* message=*reinterpret_cast<const char**>(address);
    Set(&document->errorText,message,static_cast<uint32_t>(std::strlen(message)));
#else
    (void)address; AbortNotRewritten("XML error message table");
#endif
    document->errorRow=document->errorColumn=-1;
}
}
XmlString* XmlEmpty() {
#if defined(_M_IX86)
    return reinterpret_cast<XmlString*>(0x121D758);
#else
    static XmlString empty{0,0,{0}}; return &empty;
#endif
}
// 0x004B40D0 / 0x004B40F0: tail-dispatch visitor slots, ECX changes to visitor.
uintptr_t XmlNode::Accept7(void* visitor) { return CallGameVirtual<uintptr_t,7>(visitor,this); }
uintptr_t XmlNode::Accept8(void* visitor) { return CallGameVirtual<uintptr_t,8>(visitor,this); }
// 0x00F619F0: base initializer (not a C++ constructor; preserves surrounding bytes).
XmlNode* XmlNode::Init(uint32_t nodeType) {
    row=column=-1; user=nullptr; parent=nullptr; type=nodeType;
    first=last=previous=next=nullptr; value=XmlEmpty(); vtable=Table(0x113E940);
    return this;
}
// 0x00F62530
XmlText* XmlText::InitText(const char* text) {
    Init(4); vtable=Table(0x113EAC0);
    Set(&value,text,static_cast<uint32_t>(std::strlen(text))); cdata=0;
    return this;
}
// 0x00F625F0
XmlDeclaration* XmlDeclaration::InitDeclaration() {
    Init(5); vtable=Table(0x113EA00);
    version=encoding=standalone=XmlEmpty(); return this;
}
// 0x00F625A0: deleting destructor, ret 4; reconstructed through both ret paths.
XmlText* XmlText::Destroy(uint32_t flags) {
    vtable=Table(0x113EAC0); CallGameMethod<void,0xF61A30>(static_cast<XmlNode*>(this));
    if(flags&1) FreeNode(this);
    return this;
}
// 0x00F62630: free derived strings in reverse order before the base destructor.
XmlDeclaration* XmlDeclaration::Destroy(uint32_t flags) {
    vtable=Table(0x113EA00);
    if(standalone!=XmlEmpty()) Release(standalone);
    if(encoding!=XmlEmpty()) Release(encoding);
    if(version!=XmlEmpty()) Release(version);
    CallGameMethod<void,0xF61A30>(static_cast<XmlNode*>(this));
    if(flags&1) FreeNode(this);
    return this;
}
// 0x00F62320: ECX for every string assignment is a destination member's address.
uintptr_t XmlDeclaration::CopyTo(XmlDeclaration* target) {
    Set(&target->value,value->data,static_cast<uint32_t>(std::strlen(value->data)));
    target->user=user;
    Set(&target->version,version->data,version->length);
    Set(&target->encoding,encoding->data,encoding->length);
    return Set(&target->standalone,standalone->data,standalone->length);
}
// 0x00F62920: text copies strlen(value), not the stored string length.
XmlText* XmlText::Clone() {
    auto* target=static_cast<XmlText*>(Allocate(sizeof(XmlText)));
    if(!target) return nullptr;
    target->InitText("");
    Set(&target->value,value->data,static_cast<uint32_t>(std::strlen(value->data)));
    target->user=user; target->cdata=cdata; return target;
}
// 0x00F629A0
XmlDeclaration* XmlDeclaration::Clone() {
    auto* target=static_cast<XmlDeclaration*>(Allocate(sizeof(XmlDeclaration)));
    if(!target) return nullptr;
    target->InitDeclaration(); CopyTo(target); return target;
}
// 0x00F63340: unsigned bytes passed to isspace; examines stored length, including NUL.
bool XmlNode::IsWhitespace() {
    for(uint32_t i=0;i<value->length;++i) {
        unsigned char c=static_cast<unsigned char>(value->data[i]);
        if(!std::isspace(c) && c!=10 && c!=13) return false;
    }
    return true;
}
// 0x00F62100: FILE* belongs to caller's CRT; escaped-string dependency stays bridged.
uintptr_t XmlText::Print(std::FILE* stream, int depth) {
    if(cdata) {
        PrintFile(stream,"\n");
        for(int i=0;i<depth;++i) PrintFile(stream,"    ");
        return static_cast<uintptr_t>(PrintFile(stream,"<![CDATA[%s]]>\n",value->data));
    }
    XmlString* escaped=XmlEmpty();
    CallGameFunction<void,0xF618B0>(&value,&escaped);
    PrintFile(stream,"%s",escaped->data);
    if(escaped!=XmlEmpty()) return Release(escaped);
    return reinterpret_cast<uintptr_t>(escaped);
}
// 0x00F621B0: virtual slot 17 receives a null output-string argument.
uintptr_t XmlDeclaration::Print(std::FILE* stream,int depth) { return CallGameVirtual<uintptr_t,17>(this,stream,depth,static_cast<XmlString**>(nullptr)); }
// 0x00F621D0: both FILE and string sinks are independently optional.
uintptr_t XmlDeclaration::PrintTo(std::FILE* stream,int depth,XmlString** output) {
    (void)depth;
    uintptr_t result=0;
    if(stream) result=static_cast<uintptr_t>(PrintFile(stream,"<?xml "));
    if(output) result=Append(output,"<?xml ",6);
    const char* labels[]={"version=\"","encoding=\"","standalone=\""};
    XmlString* fields[]={version,encoding,standalone};
    for(unsigned i=0;i<3;++i) {
        result=reinterpret_cast<uintptr_t>(fields[i]);
        if(!fields[i]->length) continue;
        if(stream) result=static_cast<uintptr_t>(PrintFile(stream,"%s%s\" ",labels[i],fields[i]->data));
        if(output) {
            Append(output,labels[i],static_cast<uint32_t>(std::strlen(labels[i])));
            Append(output,fields[i]->data,fields[i]->length);
            result=Append(output,"\" ",2);
        }
    }
    if(stream) result=static_cast<uintptr_t>(PrintFile(stream,"?>"));
    if(output) result=Append(output,"?>",2);
    return result;
}
// 0x00F63780: typed thiscall, despite Hex-Rays' erroneous stdcall prototype.
const char* XmlNode::ParseChildren(const char* input,XmlCursor* cursor,int mode) {
    auto* document=CallGameMethod<XmlDocument*,0xF61530>(this);
    auto* p=CallGameFunction<const char*,0xF62EB0>(input,mode);
    const char* raw=input;
    if(!p) { Error(document,6,0x11ADD78); return nullptr; }
    while(*p) {
        XmlNode* child;
        const char* end;
        if(*p!='<') {
            auto* text=static_cast<XmlText*>(CallGameFunction<void*,0xF614F0>(sizeof(XmlText)));
            if(!text) { Error(document,3,0x11ADD6C); return nullptr; }
            text->Init(4); text->vtable=Table(0x113EAC0); Set(&text->value,"",0); text->cdata=0;
#if defined(_M_IX86)
            const bool condense=*reinterpret_cast<const uint8_t*>(0x11ADB0C)!=0;
#else
            const bool condense=true;
#endif
            end=CallGameVirtual<const char*,2>(text,condense?p:raw,cursor,mode);
            child=text;
            if(text->IsWhitespace()) CallGameVirtual<void,0>(text,1u);
            else CallGameMethod<void,0xF61A70>(this,child);
        } else {
            if(Prefix(p,"</")) return p;
            child=CallGameMethod<XmlNode*,0xF63580>(this,p,mode);
            if(!child) return nullptr;
            end=CallGameVirtual<const char*,2>(child,p,cursor,mode);
            CallGameMethod<void,0xF61A70>(this,child);
        }
        raw=end; p=CallGameFunction<const char*,0xF62EB0>(end,mode);
        if(!p) { Error(document,6,0x11ADD78); return nullptr; }
    }
    return p;
}
// 0x00F643B0: preserve CDATA's sticky flag on a malformed opening marker.
const char* XmlText::Parse(const char* input,XmlCursor* cursor,int mode) {
    Set(&value,"",0);
    auto* document=CallGameMethod<XmlDocument*,0xF61530>(static_cast<XmlNode*>(this));
    if(cursor) { CallGameMethod<void,0xF62C90>(cursor,input,mode); row=cursor->row; column=cursor->column; }
    if(!cdata && !Prefix(input,"<![CDATA[")) {
        auto* end=CallGameFunction<const char*,0xF639D0>(input,&value,1,"<",0,mode);
        return end ? end-1:nullptr;
    }
    cdata=1;
    if(!Prefix(input,"<![CDATA[")) {
        CallGameMethod<void,0xF630D0>(document,15,input,cursor,mode); return nullptr;
    }
    const char* p=input+9;
    while(*p && !Prefix(p,"]]>")) { Append(&value,p,1); ++p; }
    XmlString* temporary=XmlEmpty();
    auto* end=CallGameFunction<const char*,0xF639D0>(p,&temporary,0,"]]>",0,mode);
    if(temporary!=XmlEmpty()) Release(temporary);
    return end;
}
}
