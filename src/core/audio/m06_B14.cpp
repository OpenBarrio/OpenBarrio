#include "ts3/audio/m06_B14.h"
#include <cstring>
#include "ts3/scratch/clu_4dbbd0_Scratch.h"
#include "ts3/world/clu_4dbc30_World.h"
namespace ts3::audio::b14impl {
namespace {
template<class T> T Data(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<T>(a);
#else
 (void)a; return nullptr;
#endif
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
void Assign(String* s,const String* t) { if(s!=t) CallGameMethod<void,0x00408E20>(s,t->begin,t->end); }
void Copy(Track* d,const Track* s) { if(d) { CallGameMethod<void,0x00408EB0>(&d->name,&s->name); std::memcpy(d->data,s->data,128); } }
void Clear(Tracks* v) { CallGameMethod<void,0x0077F690>(v,v->begin,v->end); }
void Dispose(String* s) {
 // EA masks the low capacity tag before its signed byte-distance comparison.
 auto bytes=static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s->capacity)-reinterpret_cast<uintptr_t>(s->begin)) & ~uintptr_t(1));
 if(bytes>2 && s->begin) Free(s->begin);
}
String Empty(const char* label) { auto* p=Data<char16_t*>(0x011C7E54); return {p,p,Data<char16_t*>(0x011C7E56),label}; }
uint32_t Count(Tracks* v) { return static_cast<uint32_t>((reinterpret_cast<intptr_t>(v->end)-reinterpret_cast<intptr_t>(v->begin))/sizeof(Track)); }
}
bool SetName(Playlist* p,uint32_t id,const char16_t* text,uint32_t) {
 if(id!=0x9fa9372e) return false;
 auto* end=text; while(*end) ++end;
 CallGameMethod<void,0x00408E20>(&p->name,text,end); return true;
}
Playlist* Construct(Playlist* p) {
 p->vtable=Data<void*>(0x010292A8); p->secondary=Data<void*>(0x01029298); p->notification=Data<void*>(0x01029294);
 p->refs=0; p->flags=0; p->owner=nullptr; p->p18=nullptr;
 p->p20=p->p24=p->p28=p->p2c=nullptr;
 p->f30=p->f34=p->f38=p->f44=p->f4c=0;
#if defined(_M_IX86)
 p->f3c=p->f40=p->f50=p->f54=*reinterpret_cast<const float*>(0x0107A538);
 p->f48=*reinterpret_cast<const float*>(0x01029300); p->f58=*reinterpret_cast<const float*>(0x01045CCC);
#else
 p->f3c=p->f40=p->f50=p->f54=1; p->f48=44000; p->f58=10;
#endif
 p->player=nullptr; p->tracks={nullptr,nullptr,nullptr,Data<const char*>(0x01029284)}; p->index=0;
 p->name=Empty(Data<const char*>(0x01029270)); return p;
}
void Destruct(Playlist* p) {
 p->vtable=Data<void*>(0x010292A8); p->secondary=Data<void*>(0x01029298); p->notification=Data<void*>(0x01029294);
 if(p->flags&1) { CallGameMethod<void,0x00869930>(p); p->flags&=~1u; CallGameMethod<void,0x00F10410>(p->player); p->player=nullptr; }
 Dispose(&p->name); Clear(&p->tracks); if(p->tracks.begin) Free(p->tracks.begin);
 if(p->owner) CallGameVirtual<void,1>(p->owner);
 p->notification=Data<void*>(0x0107396C);
}
Playlist* Delete(Playlist* p,uint32_t flags) { Destruct(p); if(flags&1) Free(p); return p; }
Playlist* DeleteNotification(void* p,uint32_t flags) { return Delete(reinterpret_cast<Playlist*>(static_cast<uint8_t*>(p)-offsetof(Playlist,notification)),flags); }
void Shuffle(Track* first,Track* last,uint32_t (**random)(uint32_t)) {
 for(auto* i=first+1;i<last;++i) { uint32_t n=static_cast<uint32_t>(i-first+1); auto* j=first+(*random)(n); CallGameFunction<void,0x00869F10>(i,j); }
}
void Insert(Tracks* v,Track* pos,const Track* source) {
 auto* end=v->end;
 if(end!=v->capacity) {
  if(source>=pos && source<end) ++source;
  Copy(end,end-1);
  CallGameFunction<void,0x00869FC0>(pos,end-1,end);
  Assign(&pos->name,&source->name); std::memcpy(pos->data,source->data,128); ++v->end; return;
 }
 uint32_t count=Count(v), capacity=count?count*2:1;
 auto* storage=static_cast<Track*>(CallGameFunction<void*,0x004E3F90>(capacity*sizeof(Track),v->label,0,0,Data<const char*>(0x00F98980),0xe7));
 Track* middle=storage;
 CallGameFunction<void,0x0077F6D0>(&middle,v->begin,pos,storage,pos);
 Copy(middle,source); ++middle;
 Track* finish=middle;
 CallGameFunction<void,0x0077F6D0>(&finish,pos,v->end,middle,pos);
 Clear(v); if(v->begin) Free(v->begin);
 v->begin=storage; v->end=finish; v->capacity=storage+capacity;
}
void Append(Tracks* v,const Track* source) {
 auto* d=v->end; if(d<v->capacity) { ++v->end; Copy(d,source); } else Insert(v,d,source);
}
bool Refresh(Playlist* p) {
 CallGameFunction<void,0x0086A810>(&p->tracks);
 using Random=uint32_t(*)(uint32_t); Random random=Data<Random>(0x008698F0);
 Shuffle(p->tracks.begin,p->tracks.end,&random); p->index=0; return p->tracks.begin!=p->tracks.end;
}
bool NextName(Playlist* p,String* out) {
 const String* name=&p->name;
 if(name->begin==name->end) {
  ++p->index; if(p->index>=Count(&p->tracks)) Refresh(p);
  if(p->tracks.begin==p->tracks.end) return false;
  name=&p->tracks.begin[p->index].name;
 }
 Assign(out,name); return true;
}
bool OpenNext(Playlist* p) {
 String name=Empty(Data<const char*>(0x01029258)); bool next=NextName(p,&name); uint32_t remaining=Count(&p->tracks);
 while(next) {
  char filename[260]; CallGameFunction<void,0x004FDDD0>(filename,name.begin,260,static_cast<int32_t>(name.end-name.begin));
  CallGameFunction<void,0x00F02F10>(); int result=CallGameMethod<int,0x00F103F0>(p->player,filename); CallGameFunction<void,0x00F02F20>();
  if(!result) { Dispose(&name); return true; }
  next=NextName(p,&name); if(!--remaining) break;
 }
 Dispose(&name); return false;
}
bool Initialize(Playlist* p) {
 if(p->flags&1) return false;
 bool ok=false; CallGameFunction<void,0x00F02F10>();
 if(!CallGameFunction<int,0x00F0E2C0>(&p->player)) {
#if defined(_M_IX86)
  auto* system=*reinterpret_cast<void**>(0x011FFC28);
#else
  void* system=nullptr;
#endif
  if(system) CallGameMethod<void,0x00ED7170>(system);
  CallGameMethod<void,0x00F01C90>(p->player,p->buffer,0x4e200,0); ok=true;
  if(system) CallGameMethod<void,0x00ED7190>(system);
 }
 CallGameFunction<void,0x00F02F20>(); if(!ok) return false;
 p->flags|=0x11; return p->player && Refresh(p);
}
bool Start(Playlist* p,uint32_t,uint32_t) {
 auto* system=CallGameFunction<void*,0x008B0A70>();
 if(system) { CallGameVirtual<void,32>(system,p?&p->secondary:nullptr); p->flags|=4; }
 p->flags|=0x40; bool result=OpenNext(p); CallGameMethod<void,0x00869D20>(p,0.f,1); return result;
}
bool SetFloat(Playlist* p,uint32_t id,float value,uint32_t) {
 uint32_t bits; std::memcpy(&bits,&value,4);
 if((bits&0x7fffffff) && (((bits+0xff800000)&0x7f800000)>=0x7f000000) && ((bits&0x7fffffff)-1)>=0x7fffff) return false;
 float* field=nullptr; uint32_t flag=0;
 switch(id) {
 case 0x03593710: field=&p->f58; break;
 case 0x050c5d65: field=&p->f38; flag=0x100; break;
 case 0x050c5d66: field=&p->f34; flag=0x100; break;
 case 0x050c5d67: field=&p->f30; flag=0x100; break;
 case 0x0f616b72: field=&p->f54; break;
 case 0x2406a047: field=&p->f40; break;
 case 0x25df0108: field=&p->f3c; break;
 case 0x12d2a4d4: CallGameMethod<uint32_t,0x008698C0>(p,8,value!=0.f); return true;
 case 0x310330cc: {
  if(value==0.f) { int32_t i=static_cast<int32_t>(p->index-2); if(i<0) i+=Count(&p->tracks); p->index=static_cast<uint32_t>(i); }
  CallGameFunction<void,0x00F02F10>(); CallGameMethod<void,0x00F01E00>(p->player); CallGameFunction<void,0x00F02F20>(); OpenNext(p); return true;
 }
 case 0x49eb68db: field=&p->f4c; flag=0x10000; break;
 case 0x647a7836: field=&p->f48; flag=0x8000; break;
 case 0xa26a765f: field=&p->f44; flag=0x4000; break;
 case 0x71bc3009: field=&p->f50; flag=0x20000; break;
 default: return true;
 }
 // EA's lahf/parity sequence skips equal values, but stores unordered values.
 if(flag>=0x4000 && value==*field) return true;
 p->flags|=flag; *field=value; return true;
}
namespace {
struct FileNode { FileNode *next,*prev; uint32_t type; const char16_t* name; };
struct FileList { FileNode *next,*prev; void* allocator; uint32_t count; const char* label; };
void InitFiles(FileList& l) { l.next=l.prev=reinterpret_cast<FileNode*>(&l); l.allocator=CallGameFunction<void*,0x004D9AD0>(); l.count=0; l.label=Data<const char*>(0x00FB23A0); }
void Enumerate(String* out,const char16_t* path,FileList* files) { CallGameMethod<void,0x004DA3C0>(out,path,files,Data<const char16_t*>(0x01029348),2,0x100000); }
void AppendText(String* s,const char16_t* text) { auto* e=text; while(*e) ++e; CallGameMethod<void,0x00408B90>(s,text,e); }
void CopyFiles(FileList& list,const char16_t* existing,const char16_t* destination) {
 for(auto* n=list.next;n!=reinterpret_cast<FileNode*>(&list);n=n->next) {
  char16_t to[520],from[520];
  scratch::ConstructWideFromPath004EDA20(to,nullptr,n->name); CallGameMethod<void,0x004EDD40>(to,destination,0);
  scratch::ConstructWideFromPath004EDA20(from,nullptr,n->name); CallGameMethod<void,0x004EDD40>(from,existing,0);
  auto* a=CallGameMethod<const wchar_t*,0x00F626F0>(to,1); auto* b=CallGameMethod<const wchar_t*,0x00F626F0>(from,1);
  world::CopyFileDbc30(b,a,1);
 }
}
}
void PrepareDirectory() {
 String path=Empty(Data<const char*>(0x01029354));
 auto* service=CallGameFunction<void*,0x0072F1C0>(); auto* base=CallGameVirtual<const char16_t*,9>(service);
 String temp{const_cast<char16_t*>(base),const_cast<char16_t*>(base),nullptr,nullptr}; while(*temp.end) ++temp.end; Assign(&path,&temp);
#if defined(_M_IX86)
 AppendText(&path,*reinterpret_cast<const char16_t**>(0x01168A58));
#else
 NotRewritten("playlist custom music suffix global");
#endif
 if(scratch::EnsureDirectory004DCFF0(reinterpret_cast<const wchar_t*>(path.begin))) {
  String found{}; FileList initial; InitFiles(initial); Enumerate(&found,path.begin,&initial);
  if(initial.next==reinterpret_cast<FileNode*>(&initial)) {
   auto* defaultPath=CallGameMethod<const String*,0x00594D20>(Data<void*>(0x011CDDE0));
   String source; CallGameMethod<void,0x00408EB0>(&source,defaultPath); AppendText(&source,Data<const char16_t*>(0x01029308));
   FileList defaults; InitFiles(defaults); Enumerate(&found,source.begin,&defaults); CopyFiles(defaults,source.begin,path.begin);
   auto* config=CallGameFunction<void*,0x0059ADA0>();
   if(config) {
    AppendText(&source,CallGameMethod<const char16_t*,0x007399F0>(config));
    if(CallGameFunction<bool,0x004DBF60>(source.begin)) {
     FileList alternate; InitFiles(alternate); Enumerate(&found,source.begin,&alternate); CopyFiles(alternate,source.begin,path.begin); CallGameMethod<void,0x00456CC0>(&alternate);
    }
   }
   CallGameMethod<void,0x00456CC0>(&defaults); Dispose(&source);
  }
  CallGameMethod<void,0x00456CC0>(&initial);
 }
 Dispose(&path);
}
}
