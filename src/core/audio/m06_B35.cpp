#include "ts3/audio/m06_B35.h"
#include <cstring>
#include <cmath>
namespace ts3::audio::b35 {
namespace {
void* Address(uintptr_t a) {
#if defined(_M_IX86)
 return reinterpret_cast<void*>(a);
#else
 (void)a; return nullptr;
#endif
}
void* System() { return CallGameFunction<void*,0x008B0A70>(); }
Object* RW() {
#if defined(_M_IX86)
 return *reinterpret_cast<Object**>(0x011FFC28);
#else
 NotRewritten("B35 RW singleton");
#endif
}
uintptr_t Release(void* p) {return CallGameVirtual<uintptr_t,1>(p);}
uintptr_t Free(void* p) {return CallGameFunction<uintptr_t,0x004E3F70>(p);}
uint32_t Bits(float f) {uint32_t u;std::memcpy(&u,&f,4);return u;}
float Float(uint32_t u) {float f;std::memcpy(&f,&u,4);return f;}
float SignSelect(float difference,float negative,float positive) {return Float((Bits(difference)&0x80000000u)?Bits(negative):Bits(positive));}
void ReleaseClear(Cell& c) {auto p=c.p;if(p) {c.p=nullptr;Release(p);}}
}
uintptr_t Destruct(Object* s) {
 At(s,0).p=Address(0x01031550);At(s,4).p=Address(0x01031540);At(s,8).p=Address(0x0103153C);
 if(At(s,0x424).u&1) {CallGameMethod<uintptr_t,0x008A0BD0>(s);At(s,0x424).u&=~1u;}
 ReleaseClear(At(s,0x374));At(s,0x4e8).p=nullptr;
 auto first=At(s,0x398).p;auto last=At(s,0x3a0).p;
 auto span=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(last)-reinterpret_cast<uintptr_t>(first))&~intptr_t(1);
 if(span>2 && first) Free(first);
 if(At(s,0x380).p) CallGameMethod<uintptr_t,0x00EC17A0>(At(s,0x380).p);
 if(At(s,0x378).p) Release(At(s,0x378).p);
 if(At(s,0x374).p) Release(At(s,0x374).p);
 if(At(s,0x370).p) Release(At(s,0x370).p);
 auto p=At(s,0x10).p;uintptr_t result=reinterpret_cast<uintptr_t>(p);
 if(p && p!=At(s,0x20).p) result=Free(p);
 At(s,8).p=Address(0x0107396C);return result;
}
Object* Delete(Object* s,uint32_t flags) {Destruct(s);if(flags&1) Free(s);return s;}
Object* DeleteAdjusted(Object* s,uint32_t flags) {return Delete(reinterpret_cast<Object*>(reinterpret_cast<Cell*>(s)-2),flags);}
Object* Construct(Object* s) {
 At(s,0).p=Address(0x01031550);At(s,4).p=Address(0x01031540);At(s,8).p=Address(0x0103153C);At(s,0xc).u=0;
 CallGameMethod<void*,0x008AAF70>(View(s,0x10));
 for(unsigned off=0x370;off<=0x394;off+=4) if(off!=0x37c) At(s,off).u=0;
 Byte(s,0x37c)=0;
 At(s,0x3a4).p=Address(0x010315E8);At(s,0x3a0).p=Address(0x011C7E56);
 At(s,0x398).p=At(s,0x39c).p=Address(0x011C7E54);
 At(s,0x3b8).u=At(s,0x3bc).u=0;At(s,0x3a8).u=At(s,0x3c0).u=0;
 Byte(s,0x3c8)=Byte(s,0x3c9)=0;
 At(s,0x3cc).f=At(s,0x3d0).f=1;At(s,0x3b0).u=782826392;At(s,0x3c4).u=2;
 for(unsigned off=0x3d4;off<=0x3e0;off+=4) At(s,off).u=0;
 At(s,0x3e4).f=At(s,0x3e8).f=1;At(s,0x3ec).f=10;
 At(s,0x3f0).f=At(s,0x3f4).f=At(s,0x3f8).f=1;At(s,0x3fc).u=0;
 At(s,0x400).f=At(s,0x404).f=At(s,0x408).f=-1;
 for(unsigned off=0x40c;off<=0x428;off+=4) At(s,off).u=0;
 At(s,0x4e8).p=nullptr;return s;
}
double Curve(Object* s,float distance,float,float nearDistance,float farDistance) {
 if(distance>farDistance) return 0;
 float ratio=(farDistance-distance)/(farDistance-nearDistance);
 float lower=SignSelect(-0.0f-ratio,ratio,0.0f);
 float upper=SignSelect(1.0f-lower,1.0f,lower);
 return CallGameMethod<double,0x008AA6B0>(View(s,0x10),1.0f-upper);
}
uintptr_t Insert(Vector* s,Triple* position,const Triple* source) {
 if(s->end!=s->capacity) {
  auto value=source;if(source>=position && source<s->end) value=source+1;
  if(s->end) *s->end=s->end[-1];
  for(auto p=s->end-1;p!=position;) {--p;p[1]=*p;}
  *position=*value;++s->end;return value->c.u;
 }
 auto n=(s->end-s->begin)*2;if(!n)n=1;
 Triple* output=nullptr;
 CallGameFunction<uintptr_t,0x00912640>(&output,s->begin,position,output);
 if(output) *output=*source;
 auto result=CallGameFunction<uintptr_t,0x00912640>(&output,position,s->end,output+1);
 s->begin=nullptr;s->end=output;s->capacity=reinterpret_cast<Triple*>(n*sizeof(Triple));return result;
}
uintptr_t Retain(Object* s,void** source) {
 auto end=static_cast<void**>(At(s,0x95c).p);
 if(reinterpret_cast<uintptr_t>(end)>=reinterpret_cast<uintptr_t>(At(s,0x960).p))
  return CallGameMethod<uintptr_t,0x008B70E0>(View(s,0x958),end,source);
 At(s,0x95c).p=end+1;
 if(end) {auto value=*source;*end=value;if(value)return CallGameVirtual<uintptr_t,0>(value);}
 return reinterpret_cast<uintptr_t>(end);
}
bool Empty(Object* s) {return At(s,0x20).u==0;}
uintptr_t Toggle(Object* s,uint8_t active) {
 auto depth=At(s,0x10).u;bool notify=active?depth==0:depth==1;
 if(active)++depth;else if(depth)--depth;At(s,0x10).u=depth;uintptr_t result=depth;
 if(notify) {
  auto bucket=static_cast<Object**>(At(s,0x18).p);Object* node=*bucket;
  while(!node)node=*++bucket;
  result=At(s,0x1c).u;auto sentinel=static_cast<Object**>(At(s,0x18).p)[result];
  while(node!=sentinel) {
   result=CallGameVirtual<uintptr_t,3>(At(node,4).p,uint32_t(0xb85523e5),float(active),1u);
   node=static_cast<Object*>(At(node,8).p);
   while(!node)node=*++bucket;
  }
 }
 return result;
}
Object* InitPool(Object* s,uint32_t count,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,Object* allocator) {
 At(s,8).u=At(s,0xc).u=0;At(s,0x10).f=1;At(s,0x14).f=2;At(s,0x18).p=nullptr;At(s,0x1c).u=0;
 At(s,0x20).p=Address(0x00FA99E8);
 auto storage=At(allocator,0).p;
 CallGameMethod<void,0x004E07A0>(View(s,0x1c),storage,600u,12u,4u,0u);
 At(s,0x24).p=storage;At(s,0x28).p=static_cast<uint8_t*>(storage)+600;At(s,0x2c).u=12;At(s,0x30).p=At(allocator,0x14).p;
 if(count>=2) {
  auto buckets=CallGameMethod<uint32_t,0x004E0870>(View(s,0x10),count);At(s,8).u=buckets;
  auto p=static_cast<void**>(At(s,0x30).p);std::memset(p,0,buckets*sizeof(void*));p[buckets]=reinterpret_cast<void*>(~uintptr_t(0));At(s,4).p=p;
 } else {At(s,0xc).u=0;At(s,0x18).p=nullptr;At(s,8).u=1;At(s,4).p=Address(0x0114C900);}
 return s;
}
uint8_t Load(Object* s) {
 auto sys=System();ReleaseClear(At(s,0x374));
 if(!CallGameVirtual<uint8_t,114>(sys,&At(s,0x388),&At(s,0x374).p)) {
  // The failure diagnostics own a lazy EASTL string and conversion buffer.
#if defined(_M_IX86)
  auto flags=reinterpret_cast<uint32_t*>(0x011D9F78);
  if(!(*flags&1)) {
   *flags|=1;*reinterpret_cast<void**>(0x011D9F74)=Address(0x0103152C);
   *reinterpret_cast<void**>(0x011D9F68)=Address(0x011C7E54);*reinterpret_cast<void**>(0x011D9F6C)=Address(0x011C7E54);*reinterpret_cast<void**>(0x011D9F70)=Address(0x011C7E56);
   CallGameFunction<int,0x00F7E9A1>(Address(0x00F93270));
  }
  auto service=CallGameFunction<void*,0x004AFD20>();
  CallGameVirtual<void,33>(service,&At(s,0x388),Address(0x011D9F68));
  CallGameFunction<int,0x004F4260>(Address(0x011D9D68),512, *reinterpret_cast<void**>(0x011D9F68),-1);
#else
  NotRewritten("B35 sound-load diagnostics");
#endif
  return 0;
 }
 uint32_t key[4]={At(s,0x388).u,At(s,0x38c).u,32437818u,At(s,0x394).u};
 Byte(s,0x37c)=CallGameVirtual<uintptr_t,111>(System(),key)!=0;
 if(Byte(s,0x37c)) {key[2]=70032571;auto other=System();ReleaseClear(At(s,0x378));CallGameVirtual<void,114>(other,key,&At(s,0x378).p);}
 auto data=static_cast<Object*>(At(s,0x374).p);At(s,0x4e8).p=At(data,0x18).p;At(s,0x4e0).p=At(data,0x18).p;At(s,0x4e4).u=0;
 auto read=[&](uint32_t bits){return CallGameMethod<uint32_t,0x008A0AE0>(View(s,0x4e0),bits);};
 read(4);At(s,0x404).f=float(uint8_t(read(4)));At(s,0x3ac).u=uint8_t(read(6))+1u;
 auto divisor=read(18);At(s,0x408).f=float(read(2));
 if(read(1)) At(s,0x424).u|=8;else At(s,0x424).u&=~8u;
 At(s,0x400).f=float(read(29)/divisor);
 if(At(s,0x408).f==0 || Byte(s,0x37c)) return 1;
 CallGameVirtual<uintptr_t,23>(s);return 0;
}
uint8_t Voices(Object* s,int32_t channels,Object* out) {
 auto rw=RW();if(!rw)return 0;CallGameMethod<void,0x00ED7170>(rw);
 auto registry=CallGameMethod<void*,0x00EDA660>(rw);
 if(!registry) {if(RW())CallGameMethod<void,0x00ED7190>(RW());return 0;}
 bool nonMono=channels!=1;
 auto begin=static_cast<Triple*>(At(s,0x28).p);auto end=static_cast<Triple*>(At(s,0x2c).p);
 unsigned index=0;
 for(auto p=begin;p<end;++p,++index) {
  if(p->a.u==1399156272u) {
   auto tail=static_cast<uint32_t*>(At(out,0x8a4).p);
   if(reinterpret_cast<uintptr_t>(tail)>=reinterpret_cast<uintptr_t>(At(out,0x8a8).p))CallGameMethod<void,0x00B7B1E0>(View(out,0x8a0),tail,&p->c.u);
   else {At(out,0x8a4).p=tail+1;if(tail)*tail=p->c.u;}
  }
  if(p->a.u==1349399088u && nonMono) continue;
  if(p->a.u==1349399088u || p->a.u==1349399089u) channels=(channels&~255)|CallGameVirtual<uint8_t,54>(System());
  auto descriptor=CallGameMethod<void*,0x00ED5A30>(registry,p->a.u);if(!descriptor)continue;
  auto v=reinterpret_cast<Vector*>(&At(out,0x10));Triple zero{};
  if(v->end<v->capacity) {auto dest=v->end++;if(dest)*dest=zero;}else Insert(v,v->end,&zero);
  auto added=v->end-1;auto tail=static_cast<uint32_t*>(At(out,0x8cc).p);
  if(reinterpret_cast<uintptr_t>(tail)>=reinterpret_cast<uintptr_t>(At(out,0x8d0).p))CallGameMethod<void,0x00B7B1E0>(View(out,0x8c8),tail,&p->b.u);
  else {At(out,0x8cc).p=tail+1;if(tail)*tail=p->b.u;}
  added->b.p=descriptor;added->a.p=(!index && Byte(out,0x968))?&At(out,0x96c):nullptr;
  reinterpret_cast<uint8_t*>(&added->c)[0]=p->a.u==1147233072u?0:uint8_t(channels);
 }
 auto first=static_cast<Cell*>(At(s,0x1a4).p);auto last=static_cast<Cell*>(At(s,0x1a8).p);
 for(auto p=first;p<last;p+=6) if(p[3].u!=1349399088u || !nonMono)
  CallGameMethod<void,0x008C1D80>(out,p[0].p,p[3].u,p[4].u,p[1].u,(p[5].u&1)?&p[2]:nullptr);
 if(RW())CallGameMethod<void,0x00ED7190>(RW());return 1;
}
uint8_t CreateVoice(Object* s) {
 auto resource=At(s,0x370).p;
 if(!resource || !CallGameVirtual<uint8_t,6>(resource,1414432011u))return 0;
 auto voice=static_cast<Object*>(At(s,0x380).p);
 if(!voice) {
  uint32_t key[4]{};CallGameVirtual<void,13>(resource,1414432011u,key);key[2]=43922235;key[3]=0x1407ec;
  Object* description=nullptr;
  if(!CallGameVirtual<uint8_t,112>(System(),key,&description)) {if(description)Release(description);return 0;}
  At(s,0x424).u|=0x10;
  auto allocation=CallGameFunction<Object*,0x004E3F90>(0x970u,"Audio/EmitterSndPlayer/CreateVoice/VoiceContainer",0u,0u,0u,0u);
  auto created=allocation?CallGameMethod<Object*,0x008C16D0>(allocation):nullptr;
  CallGameMethod<void,0x008708C0>(&At(s,0x380).p,created);voice=static_cast<Object*>(At(s,0x380).p);
  if(At(s,0x424).u&0x800) {Byte(voice,0x968)=1;At(voice,0x96c).f=5;}
  auto ok=Voices(description,At(s,0x3ac).i,voice);if(description)Release(description);if(!ok)return 0;
 } else At(s,0x424).u|=0x10;
 if(At(voice,0x10).p==At(voice,0x14).p)return 0;
 Retain(voice,&At(s,0x374).p);
 if(!Byte(voice,0x954)) {
  if(CallGameVirtual<uint8_t,6>(resource,803249283u)) {
   auto id=CallGameVirtual<uint32_t,16>(resource,803249283u);
   if(id)CallGameMethod<void,0x008C1E40>(voice,CallGameVirtual<uint32_t,68>(System(),id));
  }
  if(RW())CallGameMethod<void,0x00ED7170>(RW());CallGameMethod<void,0x008C1400>(voice,0u);if(RW())CallGameMethod<void,0x00ED7190>(RW());
 }
 CallGameVirtual<void,33>(s,At(s,0x3c4).u);At(s,0x40c).u=CallGameVirtual<uint32_t,35>(s);
 At(s,0x410).u=CallGameMethod<uint32_t,0x008C1370>(voice,1383297072u);
 if(At(voice,0xc).p) {
  auto v=reinterpret_cast<Vector*>(&At(voice,0x10));At(s,0x414).u=static_cast<uint32_t*>(At(voice,0xc).p)[v->end-v->begin-1];
 }
 At(s,0x418).u=CallGameMethod<uint32_t,0x008C1370>(voice,1414755376u);
 At(s,0x420).u=CallGameMethod<uint32_t,0x008C1370>(voice,1348564272u);return At(s,0x40c).u!=0;
}
uint8_t Spatial(Object* s,const float* point) {
 At(s,0x428).u&=~0x70u;At(s,0x424).u&=~0x40u;auto sys=System();if(!sys)return 0;
 float origin[3],matrix[9];CallGameVirtual<void,35>(sys,At(s,0x3c0).u,origin,matrix);
 float x=point[0]-origin[0],y=point[1]-origin[1],z=point[2]-origin[2];
 float local[3];for(unsigned j=0;j<3;++j)local[j]=(matrix[6+j]*z+matrix[3+j]*y)+matrix[j]*x;
 float distance;
#if defined(_M_IX86)
 float first=local[0],second=local[1],third=local[2];
 __asm {
 fld third
 fmul st(0),st(0)
 fld second
 fmul st(0),st(0)
 faddp st(1),st(0)
 fld first
 fmul st(0),st(0)
 faddp st(1),st(0)
 fsqrt
 fstp distance
 }
#else
 distance=float(std::sqrt(double(local[2])*local[2]+double(local[1])*local[1]+double(local[0])*local[0]));
#endif
 CallGameVirtual<void,32>(s,distance);
 if(At(s,0x424).u&0x100) {
  double value=CallGameVirtual<double,20>(s,distance,At(s,0x3e4).f,At(s,0x3e8).f,At(s,0x3ec).f);
  if(double(At(s,0x3f0).f)==value)At(s,0x428).u&=~0x80u;else At(s,0x428).u|=0x80;
  At(s,0x3f0).f=float(value);
 }
 CallGameVirtual<void,35>(sys,1u,origin,matrix);x=point[0]-origin[0];y=point[1]-origin[1];z=point[2]-origin[2];
 float horiz=(y*matrix[3]+z*matrix[6])+matrix[0]*x;
 float forward=(y*matrix[5]+z*matrix[8])+matrix[2]*x;
 float radius,angle;
#if defined(_M_IX86)
 __asm {
 fld forward
 fld st(0)
 fmul st(0),st(1)
 fld horiz
 fld st(0)
 fmul st(0),st(1)
 faddp st(2),st(0)
 fxch st(1)
 fsqrt
 fstp radius
 fxch st(1)
 fchs
 fpatan
 fstp angle
 }
#else
 radius=float(std::sqrt(double(forward)*forward+double(horiz)*horiz));angle=float(std::atan2(double(horiz),-double(forward)));
#endif
 if(angle<0)angle=6.283185482025146484375f+angle;
 auto voice=At(s,0x380).p;CallGameMethod<uintptr_t,0x008C1040>(voice,1249339706u,angle*57.29578f);
 float nearDistance=At(s,0x3e8).f,farDistance=At(s,0x3ec).f;
 CallGameMethod<uintptr_t,0x008C1040>(voice,1857529623u,nearDistance>radius?radius/nearDistance:1.0f);
 if(At(s,0x3ac).i>1 && farDistance!=nearDistance) {
  float lo=SignSelect(nearDistance-radius,radius,nearDistance);
  float hi=SignSelect(farDistance-lo,farDistance,lo);
  CallGameMethod<uintptr_t,0x008C1040>(voice,0xce78d695u,1.0f-(hi-nearDistance)/(farDistance-nearDistance));
 }
 return 1;
}
}

#if defined(_M_IX86)
namespace ts3::audio::b35 {
uint8_t __fastcall HookSpatial(Object* self,void*,const float* point) {return Spatial(self,point);}
uint8_t __fastcall HookLoad(Object* self,void*) {return Load(self);}
uintptr_t __fastcall HookDestruct(Object* self,void*) {return Destruct(self);}
Object* __fastcall HookDeleteAdjusted(Object* self,void*,uint32_t flags) {return DeleteAdjusted(self,flags);}
double __fastcall HookCurve(Object* self,void*,float a,float b,float c,float d) {return Curve(self,a,b,c,d);}
uintptr_t __fastcall HookInsert(Vector* self,void*,Triple* position,const Triple* value) {return Insert(self,position,value);}
Object* __fastcall HookConstruct(Object* self,void*) {return Construct(self);}
Object* __fastcall HookDelete(Object* self,void*,uint32_t flags) {return Delete(self,flags);}
uintptr_t __fastcall HookRetain(Object* self,void*,void** source) {return Retain(self,source);}
uint8_t __fastcall HookVoices(Object* self,void*,int32_t channels,Object* out) {return Voices(self,channels,out);}
uint8_t __fastcall HookCreateVoice(Object* self,void*) {return CreateVoice(self);}
bool __fastcall HookEmpty(Object* self,void*) {return Empty(self);}
uintptr_t __fastcall HookToggle(Object* self,void*,uint8_t active) {return Toggle(self,active);}
Object* __fastcall HookInitPool(Object* self,void*,uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,Object* allocator) {return InitPool(self,a,b,c,d,e,f,allocator);}
}
#endif
