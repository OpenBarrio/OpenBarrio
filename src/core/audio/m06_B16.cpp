#include "ts3/audio/m06_B16.h"
#include "ts3/scratch/clu_4a49d0_Scratch.h"
#include <cstring>
namespace ts3::audio::b16 {
namespace {
void* Table(unsigned index) {
#if defined(_M_IX86)
 constexpr uintptr_t tables[]={0x01029500,0x01029590,0x010295A4,0x010295E8,0x01029618,0x0102962C};
 return reinterpret_cast<void*>(tables[index]);
#else
 // Native identity tokens; invoking game-side modifier interfaces still requires a bridge.
 static const void* tables[6][5]{};
 return static_cast<void*>(tables[index]);
#endif
}
// These offsets describe borrowed game-side records, never native owned objects.
template<class T> T Read(const void* p, size_t o) { T v; std::memcpy(&v, static_cast<const char*>(p)+o,sizeof v); return v; }
void* Global(uintptr_t a) {
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(a);
#else
 (void)a; AbortNotRewritten("m06_B16 game singleton"); return nullptr;
#endif
}
void* Resolve(void* context, uint32_t hash, void* manager=nullptr) {
 uint32_t key[2]{};
 if (!CallGameMethod<uint8_t,0x0089F980>(context,hash,key)) return nullptr;
 if (!manager) manager=CallGameFunction<void*,0x00938B50>();
 return CallGameMethod<void*,0x00939560>(manager,key[0],key[1]);
}
void* Owner(void* object) {
 if (!object) return nullptr;
 void* wrapper=CallGameMethod<void*,0x00935770>(object,Global(0x011E90FC));
 if (!wrapper) return nullptr;
 void* p=Read<void*>(wrapper,0x30);
 return (reinterpret_cast<uintptr_t>(p)&3)?nullptr:p;
}
int Count(void* owner) {
 auto* p=static_cast<scratch::PresetOwner*>(owner);
 return static_cast<int>((reinterpret_cast<intptr_t>(p->end)-reinterpret_cast<intptr_t>(p->begin))/104);
}
scratch::Preset* Preset(void* owner,int i) { return static_cast<scratch::PresetOwner*>(owner)->Get(i); }
void* Value(void* owner,void* preset) { return CallGameMethod<void*,0x005E57F0>(owner,preset); }
void Copy(char* out,const char* in,uint32_t n) { CallGameFunction<uint32_t,0x004FF320>(out,in,n); }
uintptr_t Format(char* out,uint32_t n,const char* text) {
 auto r=CallGameFunction<int,0x004FBF60>(out,n-1,reinterpret_cast<const char*>(uintptr_t(0x01029580)),out,text);
 out[n-1]=0; return static_cast<uint32_t>(r);
}
const char* Str(uintptr_t a) { return reinterpret_cast<const char*>(a); }
bool Property(Modifier* self,void* object,void* region,char* out,uint32_t n,uint32_t& result) {
 if (!(Read<uint32_t>(region,0x98)&0x400000) && self->property!=-1) {
  void* tool=CallGameFunction<void*,0x005A70E0>(object);
  if (tool) if (void* properties=Read<void*>(tool,0x48)) {
   auto* text=CallGameMethod<const char*,0x005BE6F0>(properties,self->property);
   if (text) { result=F86BFD0(out,text,n); return true; }
  }
 }
 return false;
}
}
Modifier* __fastcall F86BAB0(Modifier* self, void*) {
 self->state=0;
 self->vtable=Table(0);
 return self;
}
Modifier* __fastcall F86BE50(Modifier* self, void*) {
 self->state=0;
 self->vtable=Table(1);
 return self;
}
Modifier* __fastcall F86BE60(Modifier* self, void*) {
 self->state=0;
 self->vtable=Table(2);
 return self;
}
Modifier* __fastcall F86BFC0(Modifier* self, void*) {
 self->state=0;
 self->vtable=Table(3);
 return self;
}
Modifier* __fastcall F86C1A0(Modifier* self, void*, int32_t property) {
 self->state=0;
 self->vtable=Table(4);
 self->property=property;
 return self;
}
Modifier* __fastcall F86C300(Modifier* self, void*) {
 self->state=0;
 self->vtable=Table(5);
 return self;
}
uint32_t F86BFD0(char* output,const char* text,uint32_t capacity) {
 CallGameFunction<uint32_t,0x004FF3F0>(output,"_",capacity);
 return CallGameFunction<uint32_t,0x004FF3F0>(output,text,capacity);
}
uintptr_t __fastcall F86B9D0(Modifier*,void*,void* context,char* output,uint32_t,int32_t) {
 void* owner=Owner(Resolve(context,0x13F18A1F));
 if (owner) {
  uint32_t type=Read<uint32_t>(owner,0x38)&0xCF00;
  if (type==0x300 || type==0x400 || type==0x500) {
   int count=Count(owner);
   for (int i=0;i<count;++i) if (auto* p=Preset(owner,i))
    if (Read<uint32_t>(p,0x60)==0x33) if (void* v=Value(owner,p)) return reinterpret_cast<uintptr_t>(v);
  }
 }
 *output=0; return reinterpret_cast<uintptr_t>(output);
}
uintptr_t __fastcall F86BAC0(Modifier*,void*,void* context,char* output,uint32_t capacity,int32_t extra) {
 char text[128]="cpet";
 uint32_t key[2]{};
 if (CallGameMethod<uint8_t,0x0089F980>(context,0x13F18A1Fu,key)) {
  void* manager=CallGameFunction<void*,0x00938B50>();
  void* object=CallGameMethod<void*,0x00939560>(manager,key[0],key[1]);
  CallGameFunction<int,0x008693E0>(object,text,128,extra);
 }
 return Format(output,capacity,text);
}
uintptr_t __fastcall F86BB80(Modifier*,void*,void* context,char* output,uint32_t capacity,int32_t extra) {
 char text[128]="bare";
 void* object=Resolve(context,0x13F18A1F);
 void* owner=Owner(object);
 if (owner) {
  uint32_t type=Read<uint32_t>(owner,0x38)&0xCF00;
  uint32_t normalized=type?type:0x100;
  const char* fixed=nullptr;
  if (CallGameMethod<uint8_t,0x0086B900>(reinterpret_cast<void*>(static_cast<uintptr_t>(extra)),object))
   fixed=Str((normalized==0x100 || type==0x200)?0x01029518:0x01029514);
  else if (normalized==0x100) {
   bool fallback=false;
   int count=Count(owner);
   for (int i=0;i<count;++i) if (auto* p=Preset(owner,i)) {
    uint32_t kind=Read<uint32_t>(p,0x60);
    if (kind==7 || kind==6) {
     if (void* v=Value(owner,p)) {
      bool match=Read<uint32_t>(v,0x34)==((Read<uint32_t>(v,0x40)&0x80000)?6u:7u);
      if (match && Read<void*>(v,0x240)!=Read<void*>(v,0x244)) {
       Copy(text,Read<const char*>(v,0x240),128); break;
      }
     }
    } else if (!fallback && kind==4) {
     if (void* v=Value(owner,p)) if (Read<void*>(v,0x240)!=Read<void*>(v,0x244)) {
      Copy(text,Read<const char*>(v,0x240),128); fallback=true;
     }
    }
   }
  } else {
   switch(type) {
    case 0x200: fixed=Str(0x01029530); break;
    case 0x300: fixed=Str(0x0102952C); break;
    case 0x400: fixed=Str(0x01029538); break;
    case 0x500: fixed=Str(0x01029540); break;
    case 0x600: fixed=Str(0x01029524); break;
    case 0x700: fixed=Str(0x0102951C); break;
   }
  }
  if (fixed) Copy(text,fixed,128);
 }
 return Format(output,capacity,text);
}
uintptr_t __fastcall F86BE70(Modifier*,void*,void* context,char* output,uint32_t capacity,int32_t) {
 char text[128]="bareback";
 void* owner=Owner(Resolve(context,0x13F18A1F));
 if (owner && (Read<uint32_t>(owner,0x38)&0x200)) {
  int count=Count(owner);
  for (int i=0;i<count;++i) if (auto* p=Preset(owner,i))
   if (Read<uint32_t>(p,0x60)==0x34) if (Value(owner,p)) { Copy(text,Str(0x010295B8),128); break; }
 }
 return Format(output,capacity,text);
}
uintptr_t __fastcall F86C000(Modifier* self,void*,void* context,char* output,uint32_t capacity,int32_t extra) {
 char text[32]="lino";
 void* manager=CallGameFunction<void*,0x00938B50>();
 void* object=Resolve(context,0x13F18A1F,manager);
 void* target=Resolve(context,0x6BF16A1E,manager);
 if (void* world=Global(0x011ED034)) {
  if (target) {
   if (void* region=CallGameMethod<void*,0x00C8EC50>(world,static_cast<char*>(target)+0x18)) {
    uint32_t result;
    if (Property(self,target,region,output,capacity,result)) return result;
    if (Read<void*>(region,0x208)!=Read<void*>(region,0x20C)) Copy(text,Read<const char*>(region,0x208),32);
   }
  } else CallGameFunction<int,0x008693E0>(object,text,32,extra);
 }
 return F86BFD0(output,text,capacity);
}
uintptr_t __fastcall F86C1C0(Modifier* self,void*,void* context,char* output,uint32_t capacity,int32_t) {
 char text[32]="lino";
 void* object=Resolve(context,0x13F18A1F);
 if (void* world=Global(0x011ED034)) if (object) {
  if (void* region=CallGameMethod<void*,0x00C8EC50>(world,static_cast<char*>(object)+0x18)) {
   uint32_t result;
   if (Property(self,object,region,output,capacity,result)) return result;
   if (Read<void*>(region,0x208)!=Read<void*>(region,0x20C)) Copy(text,Read<const char*>(region,0x208),32);
  }
 }
 return F86BFD0(output,text,capacity);
}
}
