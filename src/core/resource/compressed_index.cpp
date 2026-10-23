#include "ts3/resource/compressed_index.h"

#include "ts3/eastl_compat/sort.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <list>
#include <memory>

namespace ts3::resource {

namespace {

uint32_t ReadU32(const uint8_t* p)
{
    uint32_t v;
    std::memcpy(&v, p, 4);
    return v;
}

// eastl::prime_rehash_policy as compiled into TS3W.exe (0x004E09B0): max load factor
// 1.0, growth factor 2.0, bucket counts from EASTL's 256-entry prime table (0x00FB8558).
struct PrimeRehashPolicy {
    float maxLoadFactor = 1.0f;
    float growthFactor = 2.0f;
    uint32_t nextResize = 0;

    static uint32_t NextPrime(uint32_t atLeast);

    // Returns the new bucket count, or 0 when no rehash is needed.
    uint32_t RehashRequired(uint32_t bucketCount, uint32_t elementCount, uint32_t elementAdd)
    {
        const uint32_t wanted = elementCount + elementAdd;
        if (wanted <= nextResize)
            return 0;
        if (bucketCount == 1)
            bucketCount = 0;
        const float minBuckets = static_cast<float>(wanted) / maxLoadFactor;
        const float buckets = static_cast<float>(bucketCount);
        if (buckets < minBuckets) {
            const float grown = growthFactor * buckets;
            const float target = grown <= minBuckets ? minBuckets : grown;
            const uint32_t prime = NextPrime(static_cast<uint32_t>(static_cast<int64_t>(target)));  // truncates
            nextResize = static_cast<uint32_t>(static_cast<int64_t>(std::ceil(static_cast<float>(prime) * maxLoadFactor)));
            return prime;
        }
        nextResize = static_cast<uint32_t>(static_cast<int64_t>(std::ceil(buckets * maxLoadFactor)));
        return 0;
    }
};

// EASTL gPrimeNumberArray (identical to the table at TS3W.exe 0x00FB8558).
const uint32_t kPrimes[256] = {
    2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u, 31u, 37u, 41u, 43u, 47u, 53u, 59u, 61u, 67u, 71u, 73u, 79u, 83u,
    89u, 97u, 103u, 109u, 113u, 127u, 137u, 139u, 149u, 157u, 167u, 179u, 193u, 199u, 211u, 227u, 241u, 257u, 277u,
    293u, 313u, 337u, 359u, 383u, 409u, 439u, 467u, 503u, 541u, 577u, 619u, 661u, 709u, 761u, 823u, 887u, 953u,
    1031u, 1109u, 1193u, 1289u, 1381u, 1493u, 1613u, 1741u, 1879u, 2029u, 2179u, 2357u, 2549u, 2753u, 2971u,
    3209u, 3469u, 3739u, 4027u, 4349u, 4703u, 5087u, 5503u, 5953u, 6427u, 6949u, 7517u, 8123u, 8783u, 9497u,
    10273u, 11113u, 12011u, 12983u, 14033u, 15173u, 16411u, 17749u, 19183u, 20753u, 22447u, 24281u, 26267u,
    28411u, 30727u, 33223u, 35933u, 38873u, 42043u, 45481u, 49201u, 53201u, 57557u, 62233u, 67307u, 72817u,
    78779u, 85229u, 92203u, 99733u, 107897u, 116731u, 126271u, 136607u, 147793u, 159871u, 172933u, 187091u,
    202409u, 218971u, 236897u, 256279u, 277261u, 299951u, 324503u, 351061u, 379787u, 410857u, 444487u,
    480881u, 520241u, 562841u, 608903u, 658753u, 712697u, 771049u, 834181u, 902483u, 976369u, 1056323u,
    1142821u, 1236397u, 1337629u, 1447153u, 1565659u, 1693859u, 1832561u, 1982627u, 2144977u, 2320627u,
    2510653u, 2716249u, 2938679u, 3179303u, 3439651u, 3721303u, 4026031u, 4355707u, 4712381u, 5098259u,
    5515729u, 5967347u, 6456007u, 6984629u, 7556579u, 8175383u, 8844859u, 9569143u, 10352717u, 11200489u,
    12117689u, 13109983u, 14183539u, 15345007u, 16601593u, 17961079u, 19431899u, 21023161u, 22744717u,
    24607243u, 26622317u, 28802401u, 31160981u, 33712729u, 36473443u, 39460231u, 42691603u, 46187573u,
    49969847u, 54061849u, 58488943u, 63278561u, 68460391u, 74066549u, 80131819u, 86693767u, 93793069u,
    101473717u, 109783337u, 118773397u, 128499677u, 139022417u, 150406843u, 162723577u, 176048909u,
    190465427u, 206062531u, 222936881u, 241193053u, 260944219u, 282312799u, 305431229u, 330442829u,
    357502601u, 386778277u, 418451333u, 452718089u, 489790921u, 529899637u, 573292817u, 620239453u,
    671030513u, 725980837u, 785430967u, 849749479u, 919334987u, 994618837u, 1076067617u, 1164186217u,
    1259520799u, 1362662261u, 1474249943u, 1594975441u, 1725587117u, 1866894511u, 2019773507u,
    2185171673u, 2364114217u, 2557710269u, 2767159799u, 2993761039u, 3238918481u, 3504151727u,
    3791104843u, 4101556399u, 4294967291u};

uint32_t PrimeRehashPolicy::NextPrime(uint32_t atLeast)
{
    const uint32_t* p = std::lower_bound(kPrimes, kPrimes + 256, atLeast);
    return p == kPrimes + 256 ? kPrimes[255] : *p;
}

struct Group {
    uint32_t type;
    uint32_t group;
    std::vector<IndexRecord> records;
    Group* next = nullptr;  // hash-map chain
};

// eastl::hash_map<{type, group}, ...> with hash type + group: insertion at the head of a
// bucket, rehash moving nodes head-first (TS3W.exe 0x004AC030 / 0x004AA620).
class GroupMap {
public:
    GroupMap() : buckets_(1, nullptr) {}

    Group* FindOrInsert(uint32_t type, uint32_t group)
    {
        const uint32_t hash = type + group;
        uint32_t index = hash % static_cast<uint32_t>(buckets_.size());
        for (Group* g = buckets_[index]; g; g = g->next)
            if (g->type == type && g->group == group)
                return g;
        const uint32_t newCount =
            policy_.RehashRequired(static_cast<uint32_t>(buckets_.size()), count_, 1);
        nodes_.push_back(std::make_unique<Group>(Group{type, group, {}, nullptr}));
        Group* node = nodes_.back().get();
        if (newCount) {
            index = hash % newCount;
            Rehash(newCount);
        }
        node->next = buckets_[index];
        buckets_[index] = node;
        ++count_;
        return node;
    }

    uint32_t Count() const { return count_; }

    // EASTL iteration order: buckets ascending, then along each chain.
    template <class Fn>
    void ForEach(Fn fn) const
    {
        for (Group* head : buckets_)
            for (Group* g = head; g; g = g->next)
                fn(*g);
    }

private:
    void Rehash(uint32_t newCount)
    {
        std::vector<Group*> fresh(newCount, nullptr);
        for (Group*& head : buckets_) {
            while (Group* g = head) {
                head = g->next;
                const uint32_t i = (g->type + g->group) % newCount;
                g->next = fresh[i];
                fresh[i] = g;
            }
        }
        buckets_.swap(fresh);
    }

    std::vector<Group*> buckets_;
    std::vector<std::unique_ptr<Group>> nodes_;
    uint32_t count_ = 0;
    PrimeRehashPolicy policy_;
};

bool InstanceLess(const IndexRecord& a, const IndexRecord& b)
{
    return a.instanceHigh != b.instanceHigh ? a.instanceHigh < b.instanceHigh : a.instanceLow < b.instanceLow;
}

}  // namespace

bool BuildCompressedIndex(const uint8_t* index, uint32_t bytes, CompressedIndex& out)
{
    out = CompressedIndex{};
    if (bytes < 4)
        return false;
    const uint8_t* p = index;
    const uint8_t* end = index + (bytes & ~3u);
    const uint32_t flags = ReadU32(p);
    p += 4;
    uint32_t constant[3] = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu};
    for (int i = 0; i < 3; ++i) {
        if (flags & (1u << i)) {
            if (p + 4 > end)
                return false;
            constant[i] = ReadU32(p);
            p += 4;
        }
    }

    // 1. Parse entries, grouping records by (type, group) in index order.
    GroupMap map;
    uint32_t total = 0;
    while (p < end) {
        uint32_t key[3];
        for (int i = 0; i < 3; ++i) {
            if (flags & (1u << i)) {
                key[i] = constant[i];
            } else {
                if (p + 4 > end)
                    return false;
                key[i] = ReadU32(p);
                p += 4;
            }
        }
        if (p + 16 > end)
            return false;
        IndexRecord r{};
        r.instanceLow = ReadU32(p);
        r.instanceHigh = key[2];
        r.offset = ReadU32(p + 4);
        uint32_t size = ReadU32(p + 8);
        const uint32_t memSize = ReadU32(p + 12);
        p += 16;
        if (size & 0x80000000u) {
            p += 4;  // compression + committed
            size &= 0x7FFFFFFFu;
        }
        if (size < 0xFFFF && memSize < 0xFFFF) {
            r.size16 = static_cast<uint16_t>(size);
            r.memSize16 = static_cast<uint16_t>(memSize);
        } else {
            r.size16 = 0xFFFF;
            r.memSize16 = static_cast<uint16_t>(out.overflow.size());
            out.overflow.push_back({size, memSize});
        }
        map.FindOrInsert(key[0], key[1])->records.push_back(r);
        ++total;
    }

    // 2. Distribute groups into `count` chains by (type + group) % count, appending in
    //    hash-map iteration order.
    const uint32_t count = map.Count();
    out.groupCount = count;
    out.buckets.assign(count + 1, IndexBucket{0, 0, 0, 0xFFFFFFFFu});
    out.records.resize(total);
    if (count == 0) {
        out.buckets[0].firstRecord = 0;
        return true;
    }
    std::vector<std::list<Group*>> chains(count);
    map.ForEach([&](Group& g) { chains[(g.type + g.group) % count].push_back(&g); });

    // 3. Coalesced placement: bucket i takes the head of chain i; empty buckets take the
    //    tail of the next chain (scanning forward) that still has more than one group,
    //    and are linked into that chain's bucket list.
    uint32_t written = 0;
    uint32_t donor = 0;
    for (uint32_t i = 0; i < count; ++i) {
        IndexBucket& bucket = out.buckets[i];
        bucket.firstRecord = written;
        Group* g;
        if (chains[i].empty()) {
            while (chains[donor].size() <= 1)
                ++donor;
            g = chains[donor].back();
            chains[donor].pop_back();
            bucket.next = out.buckets[donor].next;
            out.buckets[donor].next = i;
        } else {
            g = chains[i].front();
        }
        bucket.type = g->type;
        bucket.group = g->group;
        eastl_compat::Sort(g->records.data(), g->records.data() + g->records.size(), InstanceLess);
        std::copy(g->records.begin(), g->records.end(), out.records.begin() + written);
        written += static_cast<uint32_t>(g->records.size());
    }
    out.buckets[count].firstRecord = written;
    return true;
}

uint32_t MatchingKeys(const CompressedIndex& index, const std::function<bool(const ResourceKey&)>* filter,
                      std::vector<ResourceKey>* out)
{
    uint32_t count = 0;
    if (out && !filter)
        out->reserve(out->size() + index.records.size());
    for (uint32_t b = 0; b < index.groupCount; ++b) {
        const IndexBucket& bucket = index.buckets[b];
        for (uint32_t r = bucket.firstRecord; r < index.buckets[b + 1].firstRecord; ++r) {
            const ResourceKey key{index.records[r].instanceLow, index.records[r].instanceHigh, bucket.type, bucket.group};
            if (filter && !(*filter)(key))
                continue;
            if (out)
                out->push_back(key);
            ++count;
        }
    }
    return count;
}

uint32_t SortedKeys(const CompressedIndex& index, std::vector<ResourceKey>* out)
{
    const auto total = static_cast<uint32_t>(index.records.size());
    if (!out || total == 0)
        return total;
    struct Entry {
        uint32_t record;
        uint32_t bucket;
    };
    std::vector<Entry> entries;
    entries.reserve(total);
    for (uint32_t b = 0; b < index.groupCount; ++b)
        for (uint32_t r = index.buckets[b].firstRecord; r < index.buckets[b + 1].firstRecord; ++r)
            entries.push_back({r, b});
    eastl_compat::Sort(entries.data(), entries.data() + entries.size(), [&](const Entry& a, const Entry& b) {
        return index.records[a.record].offset < index.records[b.record].offset;
    });
    for (const Entry& e : entries) {
        const IndexRecord& r = index.records[e.record];
        out->push_back({r.instanceLow, r.instanceHigh, index.buckets[e.bucket].type, index.buckets[e.bucket].group});
    }
    return total;
}

}  // namespace ts3::resource
