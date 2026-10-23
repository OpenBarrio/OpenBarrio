#include "ts3/audio/m06_B48.h"
#include <cmath>
#include <cstring>
#include <vector>
#include <cctype>
namespace ts3::audio {
bool B48Build(B48Curve* self,const B48Point* points,int32_t n) {
    self->end=self->begin;
    CallGameMethod<void,0x008AA930>(self,n);
    for(int32_t i=0;i<n;++i) {
        const float x=points[i].x,y=points[i].y;
        const float px=i?points[i-1].x:x-1.0e-10f;
        const float py=i?points[i-1].y:y;
        const double dy=static_cast<double>(y)-py;
        const float dx=x-px;
        float slope;
        if(1.0e-10>std::fabs(dy)) slope=0.0f;
        else if(static_cast<double>(dx)>dy*1.0e-10) slope=static_cast<float>(dy)/dx;
        else continue;
        self->begin[i]={x,y,px,py,slope};
    }
    self->count=static_cast<uint32_t>(self->end-self->begin);
    return self->begin!=self->end;
}
bool B48Load(B48Curve* self,void* resource,uint32_t id) {
    const B48Point identity[]={{0,0},{1,1}};
    B48Build(self,identity,2);
    if(!resource) return false;
    int32_t words; B48Point* points;
    if(!CallGameVirtual<bool,18>(resource,id,&words,&points)) return false;
    B48Build(self,points,words>>1);
    return true;
}
B48Curve* B48Construct(B48Curve* self) {
    self->storage=self->inlineData;
    self->begin=self->end=self->storage;
    self->capacity=self->storage+42;
#if defined(_M_IX86)
    self->label=reinterpret_cast<const char*>(0x00F99AC0);
#else
    self->label="Audio/ResponseCurve";
#endif
    self->count=0;
    const B48Point identity[]={{0,0},{1,1}};
    B48Build(self,identity,2);
    return self;
}
bool B48Parse(B48Curve* self,const char* text,char delimiter) {
    if(!delimiter) {
        for(const char* p=text;*p;++p) if(!std::isalnum(static_cast<unsigned char>(*p))) {delimiter=*p;break;}
    }
    size_t words=1;
    for(const char* p=text;*p;++p) if(*p==delimiter) ++words;
    if(words&1) return false;
    std::vector<float> values;
    const char* p=text;
    do {
        char token[128];size_t used=0;
        while(*p && *p!=delimiter) {if(used<127) token[used++]=*p;++p;}
        token[used]=0;
        values.push_back(static_cast<float>(CallGameFunction<double,0x004FFA20>(token,0)));
        if(!*p) break;
        ++p;
    } while(true);
    return B48Build(self,reinterpret_cast<const B48Point*>(values.data()),static_cast<int32_t>(words/2));
}
double B48Ratio(void*,int32_t a,int32_t b) {
#if defined(_M_IX86)
    const double k=0.0577623,base=69.0,hz=440.0;
    double result;
    __asm {
        fild a
        fld base
        fsub st(1),st(0)
        fld k
        fmul st(2),st(0)
        fxch st(2)
        fldl2e
        fmulp st(1),st(0)
        fld st(0)
        frndint
        fxch st(1)
        fsub st(0),st(1)
        f2xm1
        fld1
        faddp st(1),st(0)
        fscale
        fstp st(1)
        fld hz
        fmul st(1),st(0)
        fild b
        fsubrp st(3),st(0)
        fxch st(2)
        fmulp st(3),st(0)
        fxch st(2)
        fldl2e
        fmulp st(1),st(0)
        fld st(0)
        frndint
        fxch st(1)
        fsub st(0),st(1)
        f2xm1
        fld1
        faddp st(1),st(0)
        fscale
        fstp st(1)
        fmulp st(1),st(0)
        fdivp st(1),st(0)
        fstp result
    }
    return result;
#else
    return (440.0*std::exp((static_cast<double>(a)-69.0)*0.0577623))/(440.0*std::exp((static_cast<double>(b)-69.0)*0.0577623));
#endif
}
void* B48Resource(void* self) { return static_cast<uint8_t*>(self)+0xF4; }
void B48Assign(void* self,void* object) {
    struct View {uint32_t words[3];void* object;};
    auto* view=static_cast<View*>(B48Resource(self));
    void* old=view->object;
    if(old==object) return;
    if(object) CallGameVirtual<void,0>(object);
    view->object=object;
    if(old) CallGameVirtual<void,1>(old);
}
// Adapted from the verified callback/key/record twins; offsets and scalar copy order follow these entries.
uint32_t B48SetKey(B48Sound* self,uint32_t low,uint32_t high) {self->key[0]=low;self->key[1]=high;return low;}
uint64_t B48GetKey(B48Sound* self) {return self->key[0]|(static_cast<uint64_t>(self->key[1])<<32);}
uint32_t B48SetValue(B48Sound* self,uint32_t value) {self->value=value;self->flags|=2;return value;}
uint32_t B48GetValue(B48Sound* self) {return self->value;}
uint32_t B48Dirty(B48Sound* self) {return (self->flags&0xFFFFFF00u)|((self->flags&2)?1u:0u);}
uint32_t* B48Copy(B48Sound* self,uint32_t* out) {
    auto a=self->record[0];out[0]=a;
    auto b=self->record[1];out[1]=b;
    auto c=self->record[2],d=self->record[3];out[2]=c;out[3]=d;
    return out;
}
uint32_t B48Word(void* self) {uint32_t result;std::memcpy(&result,static_cast<uint8_t*>(self)+0x33C,4);return result;}
}
