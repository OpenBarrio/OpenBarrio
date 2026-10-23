#include "ts3/audio/m06_B10.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <atomic>
#include <cstring>
namespace ts3::audio::b10 {
namespace {
Node* Sentinel(Map* m) { return reinterpret_cast<Node*>(&m->maximum); }
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
void Retain(Retained* p) { if (p) ++reinterpret_cast<std::atomic<int32_t>*>(&p->references)[0]; }
void Release(Retained* p) {
    if (p && --reinterpret_cast<std::atomic<int32_t>*>(&p->references)[0] == 0) {
        reinterpret_cast<std::atomic<int32_t>*>(&p->references)->exchange(1);
        CallGameVirtual<void,0>(p, uint32_t{1});
    }
}
Node* Next(Node* n) { return CallGameFunction<Node*,0x004E0B20>(n); }
Node* Previous(Node* n) { return CallGameFunction<Node*,0x004E0B60>(n); }
Node** Insert(Map* m, Node** out, Node* hint, const uint32_t* key, uint32_t side) {
    return CallGameMethod<Node**,0x00866EE0>(m,out,hint,key,side);
}
Node* Find(Map* m, const uint32_t* key) {
    Node* n = m->root;
    Node* best = Sentinel(m);
    while (n) {
        if (n->key >= *key) { best = n; n = n->left; }
        else n = n->right;
    }
    return best == Sentinel(m) || *key < best->key ? Sentinel(m) : best;
}
void CheckService() {
    void* s = CallGameFunction<void*,0x004D5170>();
    if (s && CallGameVirtual<void*,3>(s,uint32_t{0x04F52D26})) CallGameFunction<void,0x004E1270>();
}
bool Word(void* msg, uint32_t key, uint32_t* out) { return CallGameMethod<uint8_t,0x0089F940>(msg,key,out) != 0; }
Retained* Lookup(Manager* m, uint32_t key) {
    if (Find(&m->arrays,&key) == Sentinel(&m->arrays)) return nullptr;
    Retained* p = *GetOrInsert(&m->arrays,&key);
    Retain(p);
    return p;
}
void ClearVector(VectorView* v, bool symbols) {
    if (symbols) CallGameMethod<void,0x006258F0>(v,v->begin,v->capacity);
    else CallGameMethod<void,0x007D3AF0>(v,v->begin,v->capacity);
    v->size = 0;
    if (v->capacity > 1) Free(v->begin);
}
}
void DestroyTree(Map* m, Node* n) {
    while (n) {
        DestroyTree(m,n->right);
        Node* next = n->left;
        Release(n->value);
        Free(n);
        n = next;
    }
}
InsertResult* InsertUnique(Map* m, InsertResult* out, const uint32_t* key, uint32_t) {
    Node* n = m->root;
    Node* hint = Sentinel(m);
    bool less = true;
    while (n) {
        hint = n;
        less = *key < n->key;
        n = less ? n->left : n->right;
    }
    Node* candidate = hint;
    if (less) {
        if (hint == m->minimum) {
            Insert(m,&out->node,hint,key,0);
            out->inserted = 1;
            return out;
        }
        candidate = Previous(hint);
    }
    if (candidate->key < *key) {
        Insert(m,&out->node,hint,key,0);
        out->inserted = 1;
    } else {
        out->node = candidate;
        out->inserted = 0;
    }
    return out;
}
uint32_t Erase(Map* m, const uint32_t* key) {
    Node* n = Find(m,key);
    if (n == Sentinel(m)) return 0;
    Node* out;
    CallGameMethod<void,0x00866FE0>(m,&out,n);
    return 1;
}
Node** InsertHint(Map* m, Node** out, Node* hint, const uint32_t* key, uint32_t unused) {
    if (hint != m->maximum && hint != Sentinel(m)) {
        Node* next = Next(hint);
        if (hint->key < *key && *key < next->key) {
            if (hint->right) return Insert(m,out,next,key,1);
            return Insert(m,out,hint,key,0);
        }
    } else if (m->count && m->maximum->key < *key) {
        return Insert(m,out,m->maximum,key,0);
    }
    InsertResult result{};
    InsertUnique(m,&result,key,unused & ~255u);
    *out = result.node;
    return out;
}
Retained** GetOrInsert(Map* m, const uint32_t* key) {
    Node* n = m->root;
    Node* hint = Sentinel(m);
    while (n) {
        if (n->key >= *key) { hint = n; n = n->left; }
        else n = n->right;
    }
    if (hint == Sentinel(m) || *key < hint->key) {
        Node* result;
        // Insert helper copies both the key and zero-initialized mapped value.
        struct Pair { uint32_t key; Retained* value; } pair{*key,nullptr};
        InsertHint(m,&result,hint,&pair.key,0);
        hint = result;
    }
    return &hint->value;
}
uint32_t Unsupported(Manager*, uintptr_t, uintptr_t, uintptr_t) { return 0x00F97F7A; }
Manager* DeleteSecondary(void* self, uint32_t flags) {
    auto* parent = reinterpret_cast<Manager*>(static_cast<uint8_t*>(self)-offsetof(Manager,secondary));
    return Delete(parent,flags);
}
Manager* Delete(Manager* self, uint32_t flags) {
    Destruct(self);
    if (flags & 1) Free(self);
    return self;
}
uintptr_t Shutdown(Manager* m) {
    DestroyTree(&m->arrays,m->arrays.root);
    m->arrays.maximum = Sentinel(&m->arrays);
    m->arrays.minimum = Sentinel(&m->arrays);
    m->arrays.root = nullptr;
    m->arrays.color = 0;
    m->arrays.count = 0;
    CheckService();
    return CallGameMethod<uintptr_t,0x00868E60>(m);
}
uintptr_t Destruct(Manager* m) {
#if defined(_M_IX86)
    m->vtable = reinterpret_cast<void**>(0x010289E8);
    m->secondary = reinterpret_cast<void**>(0x010289D8);
#endif
    void* s = CallGameFunction<void*,0x005A00A0>();
    if (s) CallGameVirtual<void,19>(s,m->callback);
    if (m->scratch) {
        static_cast<ts3::scratch::CleanupC16FD0*>(m->scratch)->Release();
        Free(m->scratch); m->scratch = nullptr;
    }
    if (m->service) {
        CallGameMethod<void,0x00863620>(m->service);
        Free(m->service); m->service = nullptr;
    }
    if (m->listener) {
        CallGameMethod<void,0x0086DD80>(m->listener);
        Free(m->listener); m->listener = nullptr;
    }
    DestroyTree(&m->arrays,m->arrays.root);
    ClearVector(&m->pending,false);
    if (m->subscription) CallGameVirtual<void,1>(m->subscription);
#if defined(_M_IX86)
    using DestroyLock = void(__stdcall*)(void*);
    auto destroyLock = *reinterpret_cast<DestroyLock*>(0x00F95128);
    destroyLock(m->mutex2);
    destroyLock(m->mutex1);
#else
    NotRewritten("m06_B10 lock destruction");
#endif
    ClearVector(&m->objects,false);
    ClearVector(&m->symbols,true);
#if defined(_M_IX86)
    *reinterpret_cast<void**>(0x011D8C40) = nullptr;
#endif
    return CallGameMethod<uintptr_t,0x00868DA0>(m);
}
uint8_t QueueMessage(Manager* m, void* msg) {
    uint32_t key = 0;
    if (!Word(msg,0x03475385,&key)) return 0;
    Retained* p = Lookup(m,key);
    if (Find(&m->arrays,&key) == Sentinel(&m->arrays)) return 0;
    uint32_t opcode = 0;
    if (Word(msg,0x03C8A3FE,&opcode) && opcode-1 <= 7) {
        uint32_t count = 0;
        void* data = CallGameMethod<void*,0x0089FA30>(msg,uint32_t{0xA915B62D},&count);
        if (data && count) {
            uint8_t result = CallGameMethod<uint8_t,0x00872D00>(p,opcode,data,count);
            Release(p);
            return result;
        }
    }
    Release(p);
    return 0;
}
uint8_t RemoveMessage(Manager* m, void* msg) {
    uint32_t key = 0;
    if (!Word(msg,0x03475385,&key)) return 0;
    if (Find(&m->arrays,&key) == Sentinel(&m->arrays)) return 0;
    Retained* p = *GetOrInsert(&m->arrays,&key);
    Retain(p);
    CallGameMethod<void,0x00871E20>(p);
    Erase(&m->arrays,&key);
    CallGameVirtual<void,13>(m,key);
    Release(p);
    return 1;
}
uint8_t FloatMessage(Manager* m, void* msg) {
    uint32_t key = 0;
    if (!Word(msg,0x03475385,&key)) return 0;
    if (Find(&m->arrays,&key) == Sentinel(&m->arrays)) return 0;
    Retained* p = *GetOrInsert(&m->arrays,&key);
    Retain(p);
    float value = 0;
    uint8_t result = CallGameMethod<uint8_t,0x0089F900>(msg,uint32_t{0x034753A4},&value);
    if (result) CallGameMethod<void,0x008713D0>(p,value);
    Release(p);
    return result ? 1 : 0;
}
uint8_t Initialize(Manager* m) {
    CheckService();
    uint32_t key = 0xB901F04E;
#if defined(_M_IX86)
    auto* map = reinterpret_cast<uint8_t*>(m) + 0x3368;
    *CallGameMethod<uintptr_t*,0x008B90B0>(map,&key) = 0x00864F20;
    key = 0xB695ED7A;
    *CallGameMethod<uintptr_t*,0x008B90B0>(map,&key) = 0x00864F50;
    void* resourceName = reinterpret_cast<void*>(uintptr_t{0x010289CC});
#else
    (void)key;
    NotRewritten("m06_B10 callback registration and resource name");
    void* resourceName = nullptr;
#endif
    void* resource = nullptr;
    uint64_t resourceKey = CallGameVirtual<uint64_t,70>(m,resourceName,&resource,uint32_t{0x001407EC});
    if (CallGameVirtual<uint8_t,89>(m,resourceKey)) {
        constexpr uint32_t keys[] = {0x2EA8FB98,0x05174939,0x6F394B77,0xEDF036D6,0x687A9528,0x5C770DB7,0x20681CD4};
        for (unsigned i=0;i<7;++i) {
            void* property = nullptr;
            if (!CallGameVirtual<uint8_t,8>(resource,keys[i],&property)) continue;
            auto* b = static_cast<uint8_t*>(property);
            uint16_t type; uint32_t count; void* data;
            std::memcpy(&type,b+10,2);
            std::memcpy(&data,b+16,sizeof(data));
            std::memcpy(&count,b+16+sizeof(void*)+4,4);
            if (type == 10 && (b[8]&0x30) && count == 3) std::memcpy(m->triples[i],data,12);
        }
    }
    uint8_t result = CallGameMethod<uint8_t,0x00868E50>(m);
    if (result) {
        float value;
        if (CallGameVirtual<uint8_t,12>(m->properties,uint32_t{0x3DCE7B0A},&value)) CallGameMethod<void,0x00865770>(m,value);
#if defined(_M_IX86)
        void* platform = *reinterpret_cast<void**>(0x011CDBD0);
        if (m->enabled && platform) {
            reinterpret_cast<uint8_t*>(m)[0x98] = !CallGameVirtual<uint8_t,24>(platform);
        }
#endif
    }
    if (resource) CallGameVirtual<void,1>(resource);
    return result ? 1 : 0;
}
}
