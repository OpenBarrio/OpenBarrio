#include "ts3/misc/clu_4a9e90_x.h"
#include <cstring>
#include <utility>
namespace ts3::misc::clu4a9e90 {
namespace {
uint32_t Key(PointerKey* p) { return p->key; }
uint32_t Key(const SortedEntry& p) { return p.item->key; }
uint64_t Key(const Record& p) { return p.key; }
uint64_t Key(uint64_t p) { return p; }
template<class T> T* Median(T* a, T* b, T* c) {
    if (Key(*a) < Key(*b)) {
        if (Key(*b) < Key(*c)) return b;
        return Key(*a) < Key(*c) ? c : a;
    }
    if (Key(*a) < Key(*c)) return a;
    return Key(*b) < Key(*c) ? c : b;
}
template<class T> int Push(T* a, int top, int hole, const T* value) {
    int parent = (hole - 1) >> 1;
    while (hole > top && Key(a[parent]) < Key(*value)) {
        a[hole] = a[parent]; hole = parent; parent = (parent - 1) >> 1;
    }
    a[hole] = *value;
    return hole;
}
template<class T> int Adjust(T* a, int top, int n, int hole, const T* value) {
    int child = 2 * hole + 2;
    while (child < n) {
        // Equal children select the right one, as in the original EASTL instantiation.
        if (Key(a[child]) < Key(a[child-1])) --child;
        a[hole] = a[child]; hole = child; child = 2 * child + 2;
    }
    if (child == n) { a[hole] = a[child-1]; hole = child-1; }
    return Push(a, top, hole, value);
}
template<class T> void Make(T* a, T* end) {
    int n = static_cast<int>(end-a);
    if (n < 2) return;
    int parent = (n-2)/2+1;
    do { --parent; T value = a[parent]; Adjust(a,parent,n,parent,&value); } while(parent);
}
template<class T> int Sort(T* a, T* end) {
    int n = static_cast<int>(end-a);
    while(n > 1) { T value = end[-1]; end[-1] = *a; Adjust(a,0,n-1,0,&value); --end; --n; }
    return n;
}
template<class T> int Partial(T* a, T* middle, T* end) {
    Make(a,middle);
    for (T* p=middle; p<end; ++p) {
        if (Key(*p) < Key(*a)) { T value=*p; *p=*a; Adjust(a,0,static_cast<int>(middle-a),0,&value); }
    }
    return Sort(a,middle);
}
template<class T> T* Partition(T* a, T* end, const T* pivot) {
    // The pivot key is captured, not an alias into the array being swapped.
    const auto key = Key(*pivot);
    for (;;) {
        while(Key(*a)<key) ++a;
        do { --end; } while(key<Key(*end));
        if(a>=end) return a;
        std::swap(*a,*end); ++a;
    }
}
template<int Stride, class T> int Quick(T* a, T* end, int depth) {
    int result=static_cast<int>(end-a)*Stride;
    while (end-a > 28 && depth>0) {
        T* pivot=Median(a,a+(end-a)/2,end-1);
        T* cut=Partition(a,end,pivot);
        --depth;
        result=Quick<Stride>(cut,end,depth);
        end=cut;
    }
    // EA quirk: depth < 0 does not trigger the heap fallback; small spans are untouched.
    if(depth==0) return Partial(a,end,end);
    return result;
}
} // namespace
// 0x004A9E90
const uint64_t* Median64(const uint64_t* a, const uint64_t* b, const uint64_t* c) { return Median(a,b,c); }
// 0x004A9F10
PointerKey* PushPointer(PointerKey** a, int top, int hole, PointerKey* const* value, uint32_t comparator) { hole = Push(a,top,hole,value); return *value; }
// 0x004AA2D0
PointerKey* AdjustPointer(PointerKey** a, int top, int n, int hole, PointerKey* const* value, uint32_t comparator) { hole = Adjust(a,top,n,hole,value); return *value; }
// 0x004AA860
void MakePointer(PointerKey** a, PointerKey** end, uint32_t comparator) { Make(a,end); }
// 0x004AA8B0
int SortPointer(PointerKey** a, PointerKey** end, uint32_t comparator) { return Sort(a,end); }
// 0x004AB230
PointerKey** PartitionPointer(PointerKey** a, PointerKey** end, PointerKey* const* pivot, uint32_t comparator) { return Partition(a,end,pivot); }
// 0x004AB280
int PartialPointer(PointerKey** a, PointerKey** middle, PointerKey** end, uint32_t comparator) { return Partial(a,middle,end); }
// 0x004AB950
int QuickPointer(PointerKey** a, PointerKey** end, int depth, uint32_t comparator) { return Quick<4>(a,end,depth); }
// 0x004A9F70
uint32_t PushSorted(SortedEntry* a, int top, int hole, const SortedEntry* value, uint32_t comparator) { hole = Push(a,top,hole,value); return value->payload; }
// 0x004AA340
uint32_t AdjustSorted(SortedEntry* a, int top, int n, int hole, const SortedEntry* value, uint32_t comparator) { hole = Adjust(a,top,n,hole,value); return value->payload; }
// 0x004AA910
void MakeSorted(SortedEntry* a, SortedEntry* end, uint32_t comparator) { Make(a,end); }
// 0x004AA970
int SortSorted(SortedEntry* a, SortedEntry* end, uint32_t comparator) { return Sort(a,end); }
// 0x004AB2F0
SortedEntry* PartitionSorted(SortedEntry* a, SortedEntry* end, const SortedEntry* pivot, uint32_t comparator) { return Partition(a,end,pivot); }
// 0x004AB360
int PartialSorted(SortedEntry* a, SortedEntry* middle, SortedEntry* end, uint32_t comparator) { return Partial(a,middle,end); }
// 0x004ABA10
int QuickSorted(SortedEntry* a, SortedEntry* end, int depth, uint32_t comparator) { return Quick<8>(a,end,depth); }
// 0x004A9FD0
Record* PushRecord(Record* a, int top, int hole, const Record* value, uint32_t comparator) { hole = Push(a,top,hole,value); return a + hole; }
// 0x004AA3C0
Record* AdjustRecord(Record* a, int top, int n, int hole, const Record* value, uint32_t comparator) { hole = Adjust(a,top,n,hole,value); return a + hole; }
// 0x004AA9F0
void MakeRecord(Record* a, Record* end, uint32_t comparator) { Make(a,end); }
// 0x004AAA70
int SortRecord(Record* a, Record* end, uint32_t comparator) { return Sort(a,end); }
// 0x004AB3F0
Record* PartitionRecord(Record* a, Record* end, const Record* pivot, uint32_t comparator) { return Partition(a,end,pivot); }
// 0x004AB490
int PartialRecord(Record* a, Record* middle, Record* end, uint32_t comparator) { return Partial(a,middle,end); }
// 0x004ABCE0
int QuickRecord(Record* a, Record* end, int depth, uint32_t comparator) { return Quick<16>(a,end,depth); }
// 0x007410D0
uint32_t PushUint64(uint64_t* a, int top, int hole, const uint64_t* value) { hole = Push(a,top,hole,value); return static_cast<uint32_t>(*value >> 32); }
// 0x007411B0
uint32_t AdjustUint64(uint64_t* a, int top, int n, int hole, const uint64_t* value) { hole = Adjust(a,top,n,hole,value); return static_cast<uint32_t>(*value >> 32); }
// 0x007413B0
void MakeUint64(uint64_t* a, uint64_t* end) { Make(a,end); }
// 0x00741400
int SortUint64(uint64_t* a, uint64_t* end) { return Sort(a,end); }
// 0x00741470
uint64_t* PartitionUint64(uint64_t* a, uint64_t* end, const uint64_t* pivot) { return Partition(a,end,pivot); }
// 0x007414E0
int PartialUint64(uint64_t* a, uint64_t* middle, uint64_t* end) { return Partial(a,middle,end); }
// 0x00741560
int QuickUint64(uint64_t* a, uint64_t* end, int depth) { return Quick<8>(a,end,depth); }
// 0x005C8360
int FloatExponent(uint32_t value) { float f = static_cast<float>(value); uint32_t bits; std::memcpy(&bits,&f,4); return static_cast<int>(bits >> 23)-127; }

// 0x004AA730
uintptr_t HashTable::Rehash(uint32_t n) {
    auto fresh=static_cast<HashNode**>(allocator->Alloc(sizeof(HashNode*)*(size_t(n)+1),name,flags));
    std::memset(fresh,0,sizeof(HashNode*)*n);
    fresh[n]=reinterpret_cast<HashNode*>(~uintptr_t(0));
    for(uint32_t i=0;i<bucketCount;++i) while(auto node=buckets[i]) {
        uint32_t index=(node->value[0]^node->value[3])%n;
        buckets[i]=node->next; node->next=fresh[index]; fresh[index]=node;
    }
    uintptr_t result=bucketCount;
    if(bucketCount>1) result=CallGameVirtual<uintptr_t,3>(allocator,buckets,uint32_t(sizeof(HashNode*)*(size_t(bucketCount)+1)));
    buckets=fresh; bucketCount=n; return result;
}
// 0x004AB691: interior of 0x004AB670; reconstruct its complete owning function.
HashInsertResult* HashTable::Insert(HashInsertResult* out,const uint32_t* value,uint32_t) {
    uint32_t hash=value[0]^value[3];
    uint32_t index=hash%bucketCount;
    auto slot=buckets+index;
    auto found=CallGameMethod<HashNode*,0x009A32B0>(this,*slot,value,hash);
    if(found) { out->node=found; out->bucket=slot; out->inserted=0; return out; }
    struct Decision { uint8_t needed; uint8_t padding[3]; uint32_t count; } decision{};
    CallGameMethod<void,0x004E09B0>(policy,&decision,bucketCount,count,uint32_t(1));
    auto node=static_cast<HashNode*>(allocator->Alloc(sizeof(HashNode),name,flags));
    if(node) std::memcpy(node->value,value,32);
    node->next=nullptr; // EA assumes allocation succeeds, despite guarding the copy.
    if(decision.needed) { index=hash%decision.count; Rehash(decision.count); }
    node->next=buckets[index]; buckets[index]=node; ++count;
    out->node=node; out->bucket=buckets+index; out->inserted=1; return out;
}
// 0x004AA7E0
TreeNode** Tree::InsertNode(TreeNode** out,TreeNode* parent,const TreeValue* value,uint8_t forceLeft) {
    auto sentinel=reinterpret_cast<TreeNode*>(&header);
    bool right=!forceLeft && parent!=sentinel && value->key>=parent->value.key;
    const char* source=nullptr;
#if defined(_M_IX86)
    source=reinterpret_cast<const char*>(uintptr_t(0x00F98980));
#endif
    auto node=CallGameFunction<TreeNode*,0x004E3F90>(uint32_t(sizeof(TreeNode)),allocator,uint32_t(0),uint32_t(0),source,uint32_t(231));
    // Preserve EA's peculiar check of allocation+16, not allocation itself.
    if(reinterpret_cast<uintptr_t>(node)+offsetof(TreeNode,value)!=0) node->value=*value;
    CallGameFunction<void,0x004E0C40>(node,parent,sentinel,int(right));
    ++count; *out=node; return out;
}
// 0x004AB1A0
InsertResult* Tree::InsertUnique(InsertResult* out,const TreeValue* value,uint32_t) {
    auto sentinel=reinterpret_cast<TreeNode*>(&header);
    auto node=header.parent; auto parent=sentinel; bool less=true;
    while(node) { parent=node; less=value->key<node->value.key; node=less?node->left:node->right; }
    auto candidate=parent;
    if(less) {
        if(parent==header.left) { InsertNode(&out->node,parent,value,0); out->inserted=1; return out; }
        candidate=CallGameFunction<TreeNode*,0x004E0B60>(parent);
    }
    if(candidate->value.key<value->key) { InsertNode(&out->node,parent,value,0); out->inserted=1; }
    else { out->node=candidate; out->inserted=0; }
    return out;
}
// 0x004AB890
TreeNode** Tree::InsertHint(TreeNode** out,TreeNode* hint,const TreeValue* value,uint32_t comparator) {
    auto sentinel=reinterpret_cast<TreeNode*>(&header);
    if(hint==header.right || hint==sentinel) {
        if(count && header.right->value.key<value->key) return InsertNode(out,header.right,value,0);
    } else {
        auto next=CallGameFunction<TreeNode*,0x004E0B20>(hint);
        if(hint->value.key<value->key && value->key<next->value.key) {
            if(hint->right) return InsertNode(out,next,value,1);
            return InsertNode(out,hint,value,0);
        }
    }
    InsertResult result;
    InsertUnique(&result,value,comparator); *out=result.node; return out;
}
// 0x004ABF00
TreeNode** Tree::Insert(TreeNode** out,TreeNode* hint,const TreeValue* value) { InsertHint(out,hint,value,0); return out; }
namespace {
uint32_t MipDimension(uint32_t size,uint32_t level) { return CallGameFunction<uint32_t,0x00617790>(size,level); }
uint32_t MipBytes(void* format,uint32_t width,uint32_t height) {
    uint32_t pitch=CallGameMethod<uint32_t,0x00617870>(format,width);
    return pitch*CallGameMethod<uint32_t,0x00617770>(format,height);
}
}
// 0x004ADBB0: existing verified dependency, called directly.
uint8_t* Stream::Data() const { return ts3::io::MemoryStream::Data(); }
// 0x0060CF40
uintptr_t TrimMips(void* format,uint32_t* width,uint32_t* height,uint32_t* levels,Stream* stream,const int32_t* limits) {
    uint32_t offsets[17]={}; int32_t trim=0;
    uintptr_t result=(*width<*height?*width:*height);
    if(result<static_cast<uint32_t>(limits[1])) return result;
    for(uint32_t i=0;i<*levels;++i) {
        uint32_t w=MipDimension(*width,i), h=MipDimension(*height,i);
        offsets[0]+=MipBytes(format,w,h); offsets[i+1]=offsets[0];
        if(w>=8 && h>=8 && i!=*levels-1 && trim<limits[0]) ++trim;
        result=i+1; // EAX is overwritten by the loop increment before the exit comparison.
    }
    if(!trim) return result;
    offsets[0]+=128;
    result=CallGameVirtual<uint32_t,7>(stream);
    if(result==offsets[0]+16) {
        uint8_t* tail=stream->Data()+offsets[0]; result=reinterpret_cast<uintptr_t>(tail);
        uint32_t tailWidth,tailHeight;
        std::memcpy(&tailWidth,tail+4,4); std::memcpy(&tailHeight,tail+8,4);
        if(!tailWidth || !tailHeight) return result;
        (void)FloatExponent(tailWidth / *width); // EA computes this but never uses it.
        int exponent=FloatExponent(tailHeight / *height); result=static_cast<uintptr_t>(exponent);
        trim-=exponent; if(trim<=0) return result;
    }
    *width=MipDimension(*width,trim); *height=MipDimension(*height,trim); *levels-=trim;
    return CallGameVirtual<uintptr_t,10>(stream,offsets[trim],int32_t(1));
}
// 0x0060D140
bool LastMipZero(Stream* stream,uint32_t width,uint32_t height,uint32_t levels,uint32_t formatId) {
    auto base=stream->Data();
    uint32_t size=CallGameVirtual<uint32_t,7>(stream);
    uint32_t position=CallGameVirtual<uint32_t,9>(stream,int32_t(0));
    auto format=CallGameFunction<void*,0x006177B0>(formatId);
    if(levels<3 || MipDimension(width,levels-1)>64) return false;
    uint32_t sum=0;
    for(uint32_t i=0;i<levels;++i) {
        uint32_t bytes=MipBytes(format,MipDimension(width,i),MipDimension(height,i));
        if(size-position<bytes) return false; // unsigned subtraction, including beyond-end positions
        if(i==levels-1) for(uint32_t j=0;j<bytes;++j) sum+=base[position++];
        else position+=bytes;
    }
    return sum==0;
}
} // namespace ts3::misc::clu4a9e90
