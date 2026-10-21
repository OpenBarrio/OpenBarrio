#include "ts3/world/clu_408cc0_World.h"
#include <cstring>
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include "ts3/securom/clu_59ada0_SecuRom.h"
#include <initializer_list>

namespace ts3::world408cc0 {
// 0x00408CC0. Compare exactly the even byte count; embedded NULs are significant.
int EqualWorldString(const WorldString* a, const WorldString* b)
{
    const auto an = (reinterpret_cast<uintptr_t>(a->end) - reinterpret_cast<uintptr_t>(a->begin)) / 2;
    const auto bn = (reinterpret_cast<uintptr_t>(b->end) - reinterpret_cast<uintptr_t>(b->begin)) / 2;
    if (an != bn) return 0;
    return an == 0 || std::memcmp(a->begin, b->begin, an * 2) == 0;
}

// 0x004A3A50. The last two words form an unsigned little-endian 64-bit value.
int LessWorldKey(const WorldKey* a, const WorldKey* b)
{
    for (unsigned i : {0u, 1u, 3u, 2u}) {
        if (a->word[i] != b->word[i]) return a->word[i] < b->word[i];
    }
    return 0;
}

// 0x00A517B0.
bool WorldOrderA::Less(const WorldOrderA* other) const
{
    for (unsigned i : {0u, 3u, 1u, 2u}) {
        if (std::memcmp(&key[i], &other->key[i], sizeof(WorldKey)) != 0)
            return LessWorldKey(&key[i], &other->key[i]) != 0;
    }
    return false;
}

// 0x00A8C8A0.
bool WorldOrderB::Less(const WorldOrderB* other) const
{
    for (unsigned i : {0u, 1u, 2u, 3u}) {
        if (std::memcmp(&key[i], &other->key[i], sizeof(WorldKey)) != 0)
            return LessWorldKey(&key[i], &other->key[i]) != 0;
    }
    return false;
}

// 0x00594CF0.
WorldString* FindWorldString(WorldString* begin, WorldString* end, const WorldString* key)
{
    while (begin != end && !EqualWorldString(begin, key)) ++begin;
    return begin;
}

// 0x004091F0. EA hashes through the first NUL but tests equality using the full span.
WorldIterator* WorldStringTable::Find(WorldIterator* out, const WorldString* key) const
{
    uint32_t hash = 0x811c9dc5u;
    for (auto p = key->begin; *p; ++p) hash = (hash * 0x01000193u) ^ *p;
    auto bucket = buckets + hash % bucketCount;
    auto node = *bucket;
    while (node && !EqualWorldString(key, &node->key)) node = node->next;
    if (!node) {
        bucket = buckets + bucketCount;
        node = *bucket;
    }
    *out = {node, bucket};
    return out;
}

// 0x009A3900.
#if defined(_M_IX86)
WorldChainNode* __stdcall FindWorldChain(WorldChainNode* head, const WorldString* key, void* ignored)
#else
WorldChainNode* FindWorldChain(WorldChainNode* head, const WorldString* key, void* ignored)
#endif
{
    (void)ignored;
    while (head && !EqualWorldString(key, &head->key)) head = head->next;
    return head;
}
// 0x004A4290.
bool LessWorldString(const WorldString* a, const WorldString* b)
{
    return CallGameFunction<int32_t, 0x00446b50>(a->begin, a->end, b->begin, b->end) < 0;
}

// 0x0085A760. Return the final assignment's EAX, or the source second string on alias.
void* WorldNameOwner::SetNames(const WorldString* pair)
{
    if ((flags & 2) && !EqualWorldString(pair, names)) flags &= ~2u;
    if (pair != names)
        CallGameMethod<void*, 0x00408e20>(&names[0], pair[0].begin, pair[0].end);
    if (pair + 1 != names + 1)
        return CallGameMethod<void*, 0x00408e20>(&names[1], pair[1].begin, pair[1].end);
    return const_cast<WorldString*>(pair + 1);
}

// 0x00C93410.
uint8_t WorldProductName::GetName(WorldString* out) const
{
    if (hashLow == 0x84222325u && hashHigh == 0xcbf29ce4u) {
        if (out != &name) CallGameMethod<void*, 0x00408e20>(out, name.begin, name.end);
        return 1;
    }
    auto service = securom::GetLocalizedStringService0059ADA0();
    if (!service) return 0;
    return CallGameMethod<uint8_t, 0x0059cb00>(service, hashLow, hashHigh, out);
}
namespace {
uint32_t Hash(const WorldString& key)
{
    uint32_t hash=0x811c9dc5u;
    for (auto p=key.begin; *p; ++p) hash=(hash*0x01000193u)^*p;
    return hash;
}
struct RehashDecision { uint32_t required, count; };
template<class Table> RehashDecision NeedsRehash(Table& t)
{
    RehashDecision decision;
    CallGameMethod<RehashDecision*,0x004e09b0>(t.policy,&decision,t.bucketCount,t.size,1u);
    return decision;
}
template<class Node> uint32_t RehashNodes(WorldNodeTable<Node>& t, uint32_t count)
{
    auto fresh=static_cast<Node**>(CallGameVirtual<void*,2>(t.allocator,
        sizeof(Node*)*(size_t(count)+1),t.name,t.flags));
    std::memset(fresh,0,sizeof(Node*)*count);
    fresh[count]=reinterpret_cast<Node*>(~uintptr_t(0));
    for (uint32_t i=0;i<t.bucketCount;++i) {
        while (auto node=t.buckets[i]) {
            const auto bucket=Hash(node->key)%count;
            t.buckets[i]=node->next;
            node->next=fresh[bucket];
            fresh[bucket]=node;
        }
    }
    auto result=t.bucketCount;
    if (result>1) {
        CallGameVirtual<void,3>(t.allocator,t.buckets,sizeof(Node*)*(size_t(result)+1));
        result=count;
    }
    t.buckets=fresh;
    t.bucketCount=count;
    return result;
}
template<class Table, class Node, unsigned CopyValue> WorldNodeResult<Node>* InsertNodes(
    Table& t,WorldNodeResult<Node>* out,const Node* pair)
{
    const auto hash=Hash(pair->key);
    auto bucket=hash%t.bucketCount;
    auto node=t.buckets[bucket];
    while (node && !EqualWorldString(&pair->key,&node->key)) node=node->next;
    if (node) {
        out->node=node; out->bucket=t.buckets+bucket; out->inserted=0;
        return out;
    }
    const auto decision=NeedsRehash(t);
    node=static_cast<Node*>(CallGameVirtual<void*,2>(t.allocator,sizeof(Node),t.name,t.flags));
    if (node) {
        CallGameMethod<void*,0x00408eb0>(&node->key,&pair->key);
        CallGameMethod<void*,CopyValue>(&node->value,&pair->value);
    }
    // EA unconditionally dereferences allocation failure, rather than recovering.
    node->next=nullptr;
    if (uint8_t(decision.required)) {
        bucket=hash%decision.count;
        t.Rehash(decision.count);
    }
    node->next=t.buckets[bucket]; t.buckets[bucket]=node; ++t.size;
    out->node=node; out->bucket=t.buckets+bucket; out->inserted=1;
    return out;
}
} // namespace
// 0x00408DD0. Signed ownership test; capacity low bit is a flag.
void* WorldOwnedString::Release()
{
    if (value && --value->refs==0) {
        value->refs=1;
        CallGameVirtual<void,0>(value,1u);
    }
    auto result=const_cast<uint16_t*>(key.begin);
    auto span=(reinterpret_cast<uintptr_t>(key.capacity)-reinterpret_cast<uintptr_t>(key.begin)) & ~uintptr_t(1);
    if (static_cast<intptr_t>(span)>2 && result) return scratch::Free4E3F70(result);
    return result;
}
// 0x00409320.
WorldInsertResult* WorldStringTable::Insert(WorldInsertResult* out,const WorldOwnedString* pair,uint32_t ignored)
{
    (void)ignored;
    const auto hash=Hash(pair->key);
    auto bucket=hash%bucketCount;
    auto node=buckets[bucket];
    while (node && !EqualWorldString(&pair->key,&node->key)) node=node->next;
    if (!node) {
        const auto decision=NeedsRehash(*this);
        node=CallGameMethod<WorldStringNode*,0x00409080>(this,pair);
        if (uint8_t(decision.required)) {
            bucket=hash%decision.count;
            CallGameMethod<void,0x00408f30>(this,decision.count);
        }
        node->next=buckets[bucket]; buckets[bucket]=node; ++size;
        out->inserted=1;
    } else out->inserted=0;
    out->node=node; out->bucket=buckets+bucket;
    return out;
}
// 0x004094D0.
WorldReference** WorldStringTable::Get(const WorldString* key)
{
    WorldIterator it;
    Find(&it,key);
    if (it.node!=buckets[bucketCount]) return reinterpret_cast<WorldReference**>(&it.node->value);
    WorldOwnedString pair;
    CallGameMethod<void*,0x00408eb0>(&pair.key,key);
    pair.value=nullptr;
    WorldInsertResult result;
    Insert(&result,&pair,0);
    auto value=reinterpret_cast<WorldReference**>(&result.node->value);
    pair.Release();
    return value;
}
// 0x0058B8A0.
uint32_t WorldPairTable::Rehash(uint32_t count) { return RehashNodes(*this,count); }
// 0x0058B980.
uint32_t WorldRecordTable::Rehash(uint32_t count) { return RehashNodes(*this,count); }
// 0x0058C240.
WorldNodeResult<WorldPairNode>* WorldPairTable::Insert(WorldNodeResult<WorldPairNode>* out,const WorldPairNode* pair,uint32_t ignored)
{
    (void)ignored;
    return InsertNodes<WorldPairTable,WorldPairNode,0x00408eb0>(*this,out,pair);
}
// 0x0058C710.
WorldNodeResult<WorldRecordNode>* WorldRecordTable::Insert(WorldNodeResult<WorldRecordNode>* out,const WorldRecordNode* pair,uint32_t ignored)
{
    (void)ignored;
    return InsertNodes<WorldRecordTable,WorldRecordNode,0x0058bff0>(*this,out,pair);
}
// 0x0044C340. Constant pointers refer to the game's audited UTF-16 locale records.
uint8_t EnsureWorldLocale()
{
#if defined(_M_IX86)
    auto active=reinterpret_cast<WorldString*>(0x011b0318);
    auto def=reinterpret_cast<WorldString*>(0x01147c8c);
    if (EqualWorldString(active,def)) {
        CallGameFunction<void,0x00449a50>(reinterpret_cast<const uint16_t*>(0x00fa1638),reinterpret_cast<const uint16_t*>(0x00fa1968));
        CallGameFunction<void,0x00449a50>(reinterpret_cast<const uint16_t*>(0x00fa1638),reinterpret_cast<const uint16_t*>(0x00fa1868));
        CallGameFunction<void,0x0044b9b0>(def,0,0);
    }
#else
    NotRewritten("World locale globals");
#endif
    return 1;
}
// 0x0044C390.
uint8_t ResetWorldLocale()
{
#if defined(_M_IX86)
    auto active=reinterpret_cast<WorldString*>(0x011b0318);
    auto def=reinterpret_cast<WorldString*>(0x01147c8c);
    if (!EqualWorldString(active,def)) {
        CallGameFunction<void,0x0044b9b0>(def,0,0);
        if (active->begin!=active->end) {
            *const_cast<uint16_t*>(active->begin)=0;
            active->end=active->begin;
        }
        CallGameMethod<void,0x00455f50>(active,def->begin,def->end);
        // The cache header includes padding beside its byte flag; those bytes stay intact.
        struct Cache { void* parent; void* left; void* right; void* root; uint8_t flag; uint8_t pad[3]; uint32_t size; };
        for (uintptr_t address : {uintptr_t(0x01147d44),uintptr_t(0x01147d28)}) {
            auto cache=reinterpret_cast<Cache*>(address);
            CallGameMethod<void,0x00449650>(cache,cache->root);
            cache->left=cache->right=&cache->left;
            cache->root=nullptr; cache->flag=0; cache->size=0;
        }
    }
#else
    NotRewritten("World locale globals");
#endif
    return 1;
}
// 0x00CE8360. The only architecture-specific code is the small ABI adapter below.
uint8_t LessWorldProduct(const WorldProduct* a,const WorldProductFilter* filter,const WorldProduct* b,int32_t mode)
{
    if (!a || !b) return reinterpret_cast<uintptr_t>(b)<reinterpret_cast<uintptr_t>(a);
    if (a->key.word[2]!=b->key.word[2]) return a->key.word[2]<b->key.word[2];
    auto lowest=[](uint64_t value) {return value & (uint64_t(0)-value);};
    if (a->key.word[2]==0x319e4f1du && filter) {
        if (mode==1) {
            auto x=lowest(filter->masks[0]&a->tags[0]),y=lowest(filter->masks[0]&b->tags[0]);
            auto xx=x?0:lowest(filter->masks[1]&a->tags[1]);
            auto yy=y?0:lowest(filter->masks[1]&b->tags[1]);
            if (!x && !xx) {if(y || yy) return 1;}
            else if (!y && !yy) return 0;
            else if(x!=y || xx!=yy) {
                if(!x) return y ? 0 : xx<yy;
                return y ? x<y : 1;
            }
        } else if (mode==2) {
            auto x=lowest(filter->masks[2]&a->tags[2]),y=lowest(filter->masks[2]&b->tags[2]);
            if(x!=y) return x<y;
        } else if (mode==3) {
            auto x=uint32_t(lowest(filter->mask32&a->tag32)),y=uint32_t(lowest(filter->mask32&b->tag32));
            if(x!=y) return x<y;
        }
    }
    if(a->order!=b->order) return a->order<b->order;
    if(a->price!=b->price) return b->price>a->price; // NaNs return false, without localization.
    WorldString x,y;
#if defined(_M_IX86)
    const void* tag=reinterpret_cast<void*>(0x0108059c);
#else
    const void* tag="World/UserToolHost/SortProductVectorOrdering";
#endif
    CallGameMethod<void*,0x00408890>(&x,&tag);
    CallGameMethod<void*,0x00408890>(&y,&tag);
    auto name=[](const WorldProduct* product,WorldString* out) -> uint8_t {
        WorldProductName view{};
        view.hashLow=product->hashLow; view.hashHigh=product->hashHigh; view.name=product->name;
        return view.GetName(out);
    };
    // Both entries are part of this unit; the product view only changes their object layout.
    uint8_t result;
    if(mode==4 || !name(a,&x) || !name(b,&y) || EqualWorldString(&x,&y))
        result=uint8_t(LessWorldKey(&a->key,&b->key));
    else result=LessWorldString(&x,&y);
    CallGameMethod<void,0x004ef330>(&y);
    CallGameMethod<void,0x004ef330>(&x);
    return result;
}
#if defined(_M_IX86)
// ABI glue only: retain EBX/EBP/ESI/EDI, marshal game registers to the portable cdecl body.
__declspec(naked) void LessWorldProductUsercall()
{
    __asm {
        push dword ptr [esp+4]
        push edi
        push ecx
        push eax
        call LessWorldProduct
        add esp,16
        ret
    }
}
#endif
// 0x00CE8980.
WorldProduct** MergeWorldProducts(WorldProduct** a,WorldProduct** ae,WorldProduct** b,WorldProduct** be,
    WorldProduct** out,int32_t mode,const WorldProductFilter* filter)
{
    while(a!=ae && b!=be) {
        if(LessWorldProduct(*b,filter,*a,mode)) *out++=*b++;
        else *out++=*a++;
    }
    const auto an=ae-a,bn=be-b;
    std::memmove(out,a,size_t(an)*sizeof(*out));out+=an;
    std::memmove(out,b,size_t(bn)*sizeof(*out));return out+bn;
}
// 0x00CE9490. Return count for n<=1, otherwise the end pointer returned by the final merge.
uintptr_t SortWorldProducts(WorldProduct** begin,WorldProduct** end,WorldProduct** scratch,int32_t mode,const WorldProductFilter* filter)
{
    const auto n=end-begin;
    if(n<=1) return uintptr_t(n);
    const auto half=n/2;
    auto mid=begin+half;
    if(half<=1) scratch[0]=begin[0];
    else {
        auto quarter=half/2;
        SortWorldProducts(begin,begin+quarter,scratch,mode,filter);
        SortWorldProducts(begin+quarter,mid,scratch+quarter,mode,filter);
        MergeWorldProducts(begin,begin+quarter,begin+quarter,mid,scratch,mode,filter);
    }
    if(n-half<=1) scratch[half]=*mid;
    else {
        auto quarter=(half+n)/2;
        SortWorldProducts(mid,begin+quarter,scratch+half,mode,filter);
        SortWorldProducts(begin+quarter,end,scratch+quarter,mode,filter);
        MergeWorldProducts(mid,begin+quarter,begin+quarter,end,scratch+half,mode,filter);
    }
    return reinterpret_cast<uintptr_t>(MergeWorldProducts(scratch,scratch+half,scratch+half,scratch+n,begin,mode,filter));
}
// 0x005DE510. The comparator compares a key against itself after both priority fields tie:
// EA bug, so the product key must NOT become a third ordering field.
intptr_t PartitionWorldEntries(WorldSortEntry* begin,WorldSortEntry* end,int32_t depth,uint32_t context)
{
    auto less=[](const WorldSortEntry& a,const WorldSortEntry& b) {
        if(!a.product || !b.product) return b.product!=nullptr;
        if(a.product->priority!=b.product->priority) return a.product->priority<b.product->priority;
        return a.order<b.order;
    };
    auto n=end-begin;
    while(n>28 && depth>0) {
        auto mid=begin+n/2,last=end-1,pivot=mid;
        if(less(*begin,*mid)) {
            if(less(*mid,*last)) pivot=mid;
            else if(less(*begin,*last)) pivot=last;
            else pivot=begin;
        } else {
            if(less(*begin,*last)) pivot=begin;
            else if(less(*mid,*last)) pivot=last;
        }
        auto cut=CallGameFunction<WorldSortEntry*,0x005de240>(begin,end,pivot,context);
        PartitionWorldEntries(cut,end,--depth,context);
        end=cut;n=end-begin;
    }
    if(!depth) return CallGameFunction<intptr_t,0x005dc350>(begin,end,end,context);
    return n;
}
} // namespace ts3::world408cc0
