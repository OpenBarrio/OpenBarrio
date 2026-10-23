#include "ts3/anim/m05_M97.h"
#include <cstring>
#include <emmintrin.h>
namespace ts3::anim::m97 {
namespace {
template<class T> T& field(void* p,size_t o) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+o); }
template<unsigned A> void* global() {
#if defined(_M_IX86)
 return *reinterpret_cast<void**>(A);
#else
 AbortNotRewritten("M97 game global"); return nullptr;
#endif
}
void* component(void* p,void* key) { return CallGameMethod<void*,0x00935770>(p,key); }
void* entity(Context* c) {
 void* r=nullptr;
 if(c->group->primary) r=CallGameVirtual<void*,9>(c->group->primary);
 if(!r && c->group->fallback) r=CallGameMethod<void*,0x005775D0>(c->group->fallback);
 return r;
}
}
uintptr_t Detach(Group* g,uintptr_t incomingEax) {
 uintptr_t result=incomingEax;
 for(Node* n=g->sentinel.next;n!=&g->sentinel;n=n->next) {
  if(n->controller) {
   CallGameMethod<void,0x00567820>(n->controller);
   result=CallGameMethod<uintptr_t,0x005668A0>(n->controller,g);
  }
 }
 return result;
}
uintptr_t Activate(Group* g,uintptr_t incomingEax) {
 if(g->active==0) {
  g->active=1;
  return Detach(g,incomingEax);
 }
 return incomingEax;
}
void* Find(Group* g,uint32_t key) {
 for(Node* n=g->sentinel.next;n!=&g->sentinel;n=n->next) {
  if(n->controller) {
   void* r=CallGameMethod<void*,0x00566240>(n->controller,key);
   if(r) return r;
  }
 }
 return nullptr;
}
uintptr_t SurfaceEvent(Group* self,Context* c,void* event,uintptr_t,uintptr_t) {
 auto* payload=static_cast<unsigned char*>(event)+40;
 uint32_t key=CallGameFunction<uint32_t,0x00800100>(L"object");
 void* object=c->group->manager ? CallGameMethod<void*,0x00581840>(c->group->manager,key):nullptr;
 key=CallGameFunction<uint32_t,0x00800100>(L"surface");
 void* surface=c->group->manager ? CallGameMethod<void*,0x00581840>(c->group->manager,key):nullptr;
 void* result=object;
 if(!object) {
  result=entity(c);
  if(!result && self->fallback) result=CallGameVirtual<void*,9>(self->fallback);
 }
 if(result) {
  result=component(result,global<0x011E8E70>());
  if(result) return CallGameMethod<uintptr_t,0x009442D0>(result,payload,0,surface,0);
 }
 return reinterpret_cast<uintptr_t>(result);
}
void VisibilityEvent(Context* c,void* event,uintptr_t,uintptr_t) {
 if(!event || field<uint32_t>(event,8)!=0xc1e40006) return;
 void* obj=entity(c);
 if(!obj) return;
 bool special=component(obj,global<0x011E90FC>())!=nullptr;
 void* flags=component(obj,global<0x011DC484>());
 bool visible=field<float>(event,40)>0x1p-16f;
 if(flags && field<int32_t>(flags,44)<0) {
  struct Record { uint32_t type,zero; uint64_t id; void* mask; void* value; } r;
  r.type=0x04878bc4; r.zero=0; r.id=field<uint64_t>(obj,8);
  r.mask=global<0x0114F988>(); r.value=visible?r.mask:nullptr;
  CallGameFunction<void,0x00692F70>(&r);
  CallGameMethod<void,0x00582640>(&r);
  return;
 }
 void* target=component(obj,global<0x011E92A8>());
 if(!target) return;
 if(field<uint8_t>(global<0x011CD800>(),49)) {
  uint32_t color=0xff8844;
  if(visible) {
   void* p=CallGameMethod<void*,0x00582760>(c);
   color=p && (field<uint8_t>(p,12)&2)==2 ? 0xff00:0;
  }
  CallGameMethod<void,0x0095F520>(target,color,0.0f,0,0);
 } else {
  // Only AL of this argument is consumed by 0095FCB0.
  CallGameMethod<void,0x0095FCB0>(target,uint32_t(visible));
  if(special && !visible) {
   uint64_t id=field<uint64_t>(obj,8);
   CallGameFunction<void,0x00797B20>(uint32_t(id),uint32_t(id>>32),0);
  }
 }
}

namespace {
void MirrorMatrix(const __m128* matrix,__m128* output) {
 __m128 v24,v25,v26,v27,v28,v29,v30,v31,v32,v33,v34,v35,v36,v37,v38,v39,v40,v41,v42,v43,v44,v45,v46,v47,v48,v54,v55,v56,v57,v58;
 float v62;
            v24 = *matrix;
            v25 = matrix[1];
            v26 = matrix[2];
            v27 = _mm_shuffle_ps(v24, v24, 0);
            v28 = _mm_shuffle_ps(v25, v25, 85);
            v29 = _mm_shuffle_ps(v26, v26, 170);
            v54 = _mm_add_ps(v29, _mm_add_ps(v27, v28));
            
            v62 = 0.0f;
            v30 = _mm_shuffle_ps(_mm_set_ss(0.5f), _mm_set_ss(0.5f), 0);
            if ( _mm_cvtss_f32(v54) <= v62 )
            {
              v62 = _mm_cvtss_f32(v28);
              if ( !(_mm_cvtss_f32(v27) > _mm_cvtss_f32(v28)) || (v62 = _mm_cvtss_f32(v29), !(_mm_cvtss_f32(v27) > _mm_cvtss_f32(v29))) )
              {
                v62 = _mm_cvtss_f32(v29);
                if ( !(_mm_cvtss_f32(v28) > _mm_cvtss_f32(v29)) )
                {
                  v38 = _mm_sub_ps(v29, _mm_add_ps(v27, v28));
                  v31 = _mm_shuffle_ps(_mm_set_ss(1.0f), _mm_set_ss(1.0f), 0);
                  v39 = _mm_sqrt_ps(_mm_add_ps(v38, v31));
                  v58 = _mm_div_ps(v30, v39);
                  v33 = _mm_shuffle_ps(
                          _mm_shuffle_ps(
                            _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v24, v24, 170), _mm_shuffle_ps(v26, v26, 0)), v58),
                            _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v25, v25, 170), _mm_shuffle_ps(v26, v26, 85)), v58),
                            16),
                          _mm_shuffle_ps(
                            _mm_mul_ps(v39, v30),
                            _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v24, v24, 85), _mm_shuffle_ps(v25, v25, 0)), v58),
                            50),
                          136);
                }
                else
                {
                  v36 = _mm_sub_ps(v28, _mm_add_ps(v29, v27));
                  v31 = _mm_shuffle_ps(_mm_set_ss(1.0f), _mm_set_ss(1.0f), 0);
                  v57 = _mm_sqrt_ps(_mm_add_ps(v36, v31));
                  v37 = _mm_div_ps(v30, v57);
                  v33 = _mm_shuffle_ps(
                          _mm_shuffle_ps(
                            _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v25, v25, 0), _mm_shuffle_ps(v24, v24, 85)), v37),
                            _mm_mul_ps(v57, v30),
                            16),
                          _mm_shuffle_ps(
                            _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v25, v25, 170), _mm_shuffle_ps(v26, v26, 85)), v37),
                            _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v26, v26, 0), _mm_shuffle_ps(v24, v24, 170)), v37),
                            50),
                          136);
                }
              }
              else
              {
                v34 = _mm_sub_ps(v27, _mm_add_ps(v29, v28));
                v31 = _mm_shuffle_ps(_mm_set_ss(1.0f), _mm_set_ss(1.0f), 0);
                v35 = _mm_sqrt_ps(_mm_add_ps(v34, v31));
                v56 = _mm_div_ps(v30, v35);
                v33 = _mm_shuffle_ps(
                        _mm_shuffle_ps(
                          _mm_mul_ps(v35, v30),
                          _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v25, v25, 0), _mm_shuffle_ps(v24, v24, 85)), v56),
                          16),
                        _mm_shuffle_ps(
                          _mm_mul_ps(_mm_add_ps(_mm_shuffle_ps(v24, v24, 170), _mm_shuffle_ps(v26, v26, 0)), v56),
                          _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v25, v25, 170), _mm_shuffle_ps(v26, v26, 85)), v56),
                          50),
                        136);
              }
            }
            else
            {
              v31 = _mm_shuffle_ps(_mm_set_ss(1.0f), _mm_set_ss(1.0f), 0);
              v55 = _mm_sqrt_ps(_mm_add_ps(v54, v31));
              v32 = _mm_div_ps(v30, v55);
              v33 = _mm_shuffle_ps(
                      _mm_shuffle_ps(
                        _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v25, v25, 170), _mm_shuffle_ps(v26, v26, 85)), v32),
                        _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v26, v26, 0), _mm_shuffle_ps(v24, v24, 170)), v32),
                        16),
                      _mm_shuffle_ps(
                        _mm_mul_ps(_mm_sub_ps(_mm_shuffle_ps(v24, v24, 85), _mm_shuffle_ps(v25, v25, 0)), v32),
                        _mm_mul_ps(v55, v30),
                        50),
                      136);
            }
            v40 = _mm_shuffle_ps(
                    _mm_shuffle_ps(matrix[3], _mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(matrix[3], matrix[3], 0)), 5),
                    matrix[3],
                    226);
            v41 = _mm_shuffle_ps(
                    _mm_shuffle_ps(v33, _mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v33, v33, 85)), 0),
                    v33,
                    232);
            v42 = _mm_mul_ps(
                    _mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtsi32_si128(0x3fb504f3)), _mm_castsi128_ps(_mm_cvtsi32_si128(0x3fb504f3)), 0),
                    _mm_shuffle_ps(
                      v41,
                      _mm_shuffle_ps(v41, _mm_sub_ps(_mm_setzero_ps(), _mm_shuffle_ps(v41, v41, 170)), 15),
                      36));
            v43 = _mm_mul_ps(_mm_shuffle_ps(v42, v42, 9), v42);
            v44 = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), v42), v42), v30);
            v45 = _mm_mul_ps(_mm_shuffle_ps(v42, v42, 255), _mm_shuffle_ps(v42, v42, 18));
            v46 = _mm_add_ps(v45, v43);
            v47 = _mm_sub_ps(v43, v45);
            v48 = _mm_add_ps(_mm_shuffle_ps(v44, v44, 9), v44);
            output[0] = _mm_and_ps(_mm_castsi128_ps(_mm_set_epi32(0,-1,-1,-1)), _mm_shuffle_ps(_mm_shuffle_ps(v48, v46, 1), v47, 40));
            output[1] = _mm_and_ps(_mm_castsi128_ps(_mm_set_epi32(0,-1,-1,-1)), _mm_shuffle_ps(_mm_shuffle_ps(v47, v48, 32), v46, 24));
            output[2] = _mm_and_ps(_mm_castsi128_ps(_mm_set_epi32(0,-1,-1,-1)), _mm_shuffle_ps(_mm_shuffle_ps(v46, v47, 18), v48, 8));
            output[3] = _mm_shuffle_ps(v40, _mm_shuffle_ps(v40, v31, 250), 132);
}
}
uintptr_t ParentEvent(Context* c,void* event,uintptr_t,uintptr_t) {
 uint32_t type=field<uint32_t>(event,8);
 if(type==0xc1e40002) {
  uintptr_t r=field<uint32_t>(event,40);
  auto* manager=c->group->manager;
  if(!manager) return r;
  void* controller=CallGameMethod<void*,0x00581810>(manager,uint32_t(r));
  if(!controller) return 0;
  void* obj=CallGameMethod<void*,0x00581560>(controller,0);
  if(!obj) return 0;
  void* target=component(obj,global<0x011E9158>());
  if(!target) return 0;
  if(!(field<uint32_t>(event,28)&0x20)) return CallGameMethod<uintptr_t,0x0095AB80>(target,1);
  void* record=CallGameMethod<void*,0x00581910>(controller,1);
  if(!record) return 0;
  void* parent=CallGameMethod<void*,0x005815D0>(record);
  void* matrix=static_cast<unsigned char*>(record)+16;
  if(parent) {
   uint32_t owned=field<uint8_t>(record,80);
   void* joint=CallGameMethod<void*,0x00581700>(record);
   if(field<uint8_t>(record,81)) CallGameMethod<void,0x00582610>(target,parent,joint,matrix,1,owned);
   else CallGameMethod<void,0x005825E0>(target,parent,joint,matrix,1,owned);
  } else if(CallGameMethod<uintptr_t,0x0095AB80>(target,1)&0xff) {
   CallGameMethod<void,0x00959D30>(target,matrix);
  }
  return CallGameFunction<uintptr_t,0x004E3F70>(record);
 }
 if(type!=0xc1e40001) return type;
 auto* manager=c->group->manager;
 void* controller=manager?CallGameMethod<void*,0x00581810>(manager,field<uint32_t>(event,40)):nullptr;
 void* child=manager?CallGameMethod<void*,0x00581840>(manager,field<uint32_t>(event,40)):nullptr;
 void* parent=manager?CallGameMethod<void*,0x00581840>(manager,field<uint32_t>(event,44)):nullptr;
 uint32_t joint=field<uint32_t>(event,48);
 int direct=0;
 uint32_t mapped=0;
 if(c->group->secondary) mapped=CallGameFunction<uint32_t,0x005826D0>(parent,joint,&direct);
 if(!child || !parent || !joint || (c->group->secondary && !mapped)) return reinterpret_cast<uintptr_t>(child);
 void* target=CallGameFunction<void*,0x0074B9B0>(child);
 if(!target) return 0;
 void* oldParent=field<void*>(target,48);
 if(field<uint32_t>(event,28)&0x20) {
  void* record=CallGameFunction<void*,0x004E3F90>(96,"Animation/ParentEvent/SceneEntityParentRecord",0,0,0,0);
  if(record) {
   field<uint64_t>(record,0)=0; field<uint32_t>(record,8)=0;
   field<uint8_t>(record,80)=0; field<uint8_t>(record,81)=0;
  }
  alignas(16) __m128 temp[4];
  void* parentInterface=nullptr;
  void* matrix;
  uint64_t id=0;
  if(oldParent) {
   parentInterface=CallGameMethod<void*,0x009578D0>(target,static_cast<unsigned char*>(record)+81);
   if(parentInterface && CallGameVirtual<void*,2>(parentInterface,3)) {
    void* p=CallGameVirtual<void*,2>(parentInterface,3);
    field<uint8_t>(record,80)=field<void*>(p,28)==child;
   }
   matrix=CallGameMethod<void*,0x009576B0>(target,temp);
   id=field<uint64_t>(oldParent,8);
  } else matrix=CallGameMethod<void*,0x00959060>(target,temp);
  std::memcpy(static_cast<unsigned char*>(record)+16,matrix,64);
  field<uint64_t>(record,0)=id;
  field<uint32_t>(record,8)=parentInterface?field<uint32_t>(parentInterface,sizeof(void*)):0;
  CallGameMethod<void,0x005815A0>(controller,record);
 }
 // EA changes only the low byte of the child pointer used as this argument.
 uintptr_t flags=(reinterpret_cast<uintptr_t>(child)&~uintptr_t(255)) | ((field<uint32_t>(event,28)>>6)&1);
 if((flags&255) && (field<uint8_t>(controller,12)&2)==2) flags&=~uintptr_t(255);
 auto* matrix=reinterpret_cast<__m128*>(static_cast<unsigned char*>(event)+64);
 if(!c->group->secondary || direct) return CallGameMethod<uintptr_t,0x0095B4E0>(target, parent,joint,matrix,flags);
 alignas(16) __m128 converted[4];
 MirrorMatrix(matrix,converted);
 return CallGameMethod<uintptr_t,0x0095B4E0>(target,parent,mapped,converted,flags);
}
}

#if defined(_M_IX86)
namespace ts3::anim::m97::hooks {
static uintptr_t __cdecl DetachSeeded(Group* g,uintptr_t eax) {return m97::Detach(g,eax);}
static uintptr_t __cdecl ActivateSeeded(Group* g,uintptr_t eax) {return m97::Activate(g,eax);}
__declspec(naked) uintptr_t __fastcall Detach(Group*,void*) {
 __asm { push eax
         push ecx
         call DetachSeeded
         add esp,8
         ret }
}
__declspec(naked) uintptr_t __fastcall Activate(Group*,void*) {
 __asm { push eax
         push ecx
         call ActivateSeeded
         add esp,8
         ret }
}
}
#endif
