#include "ts3/resourcesystem/clu_49e9e0_ResourceSystem.h"
#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
#include <algorithm>
#include <cstring>
namespace ts3::resourcesystem {
// 0x004A0A40. Do not replace with memcpy: overlapping ranges use word-at-a-time stores.
Words20* Copy20Backward(const Words20* first, const Words20* last, Words20* end)
{
    while (last != first) {
        --last; --end;
        end->descriptor = last->descriptor;
        end->parameters.directory = last->parameters.directory;
        end->parameters.filename = last->parameters.filename;
        end->parameters.counts = last->parameters.counts;
        end->parameters.flags = last->parameters.flags;
    }
    return end;
}
// 0x00C99A90
Words20* Copy20Forward(const Words20* first, const Words20* last, Words20* out)
{
    while (first != last) {
        out->descriptor = first->descriptor;
        out->parameters.directory = first->parameters.directory;
        out->parameters.filename = first->parameters.filename;
        out->parameters.counts = first->parameters.counts;
        out->parameters.flags = first->parameters.flags;
        ++first; ++out;
    }
    return out;
}
// 0x00C99BF0. EA skips construction at address zero but still advances the address.
Words20** Construct20Range(Words20** end, const Words20* first, const Words20* last, Words20* out)
{
    *end = out;
    if (first != last) {
        do {
            if (out) {
                out->descriptor = first->descriptor;
                out->parameters.directory = first->parameters.directory;
                out->parameters.filename = first->parameters.filename;
                out->parameters.counts = first->parameters.counts;
                out->parameters.flags = first->parameters.flags;
            }
            ++first;
            out = reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(out) + sizeof(Words20));
        } while (first != last);
        *end = out;
    }
    return end;
}
// 0x004A0AC0. Capacity limits stores, not input scanning or the reported count.
uint32_t CopyPathComponent(const char16_t* input, char16_t* output, uint32_t capacity)
{
    uint32_t count = 1;
    char16_t ch = *input++;
    if (ch) {
        for (;;) {
            if (count < capacity) *output++ = ch;
            if (ch == u'\\' || ch == u'/' || ch == u':') { *output = 0; return count; }
            ch = *input++;
            ++count;
            if (!ch) break;
        }
    }
    if (count < capacity) { *output++ = u'\\'; ++count; }
    *output = 0;
    return count;
}
}
namespace ts3::resourcesystem {
namespace {
void CopyPattern(Words20* out, const Words20* in)
{
    out->descriptor = in->descriptor;
    out->parameters.directory = in->parameters.directory;
    out->parameters.filename = in->parameters.filename;
    out->parameters.counts = in->parameters.counts;
    out->parameters.flags = in->parameters.flags;
}
size_t PatternCount(const Words20* first, const Words20* last)
{
    return (reinterpret_cast<uintptr_t>(last) - reinterpret_cast<uintptr_t>(first)) / sizeof(Words20);
}
}
// 0x0049E9E0
uintptr_t PatternVector::Insert(Words20* position, const Words20* value)
{
    if (end != capacity) {
        const Words20* source = value;
        if (reinterpret_cast<uintptr_t>(source) >= reinterpret_cast<uintptr_t>(position) &&
            reinterpret_cast<uintptr_t>(source) < reinterpret_cast<uintptr_t>(end)) ++source;
        if (end) CopyPattern(end, end - 1);
        Copy20Backward(position, end - 1, end);
        CopyPattern(position, source);
        ++end;
        return source->parameters.flags;
    }
    const uint32_t count = static_cast<uint32_t>(PatternCount(begin, end));
    const uint32_t newCount = count ? 2 * count : 1;
    auto* storage = newCount ? ts3::CallGameVirtual<Words20*, 2>(allocator,
        static_cast<uint32_t>(sizeof(Words20) * newCount), name, flags) : nullptr;
    Words20* next;
    Construct20Range(&next, begin, position, storage);
    if (next) CopyPattern(next, value);
    Construct20Range(&next, position, end, reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(next) + sizeof(Words20)));
    if (begin) ts3::CallGameVirtual<void, 3>(allocator, begin, static_cast<uint32_t>(sizeof(Words20) * PatternCount(begin, capacity)));
    begin = storage; end = next;
    capacity = reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(storage) + sizeof(Words20) * newCount);
    return reinterpret_cast<uintptr_t>(capacity);
}
// 0x0049EDE0
uintptr_t PatternLists::AppendPrimary(const Words20* value)
{
    Words20* out = primary.end;
    uintptr_t result;
    if (reinterpret_cast<uintptr_t>(out) < reinterpret_cast<uintptr_t>(primary.capacity)) {
        primary.end = reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(out) + sizeof(Words20));
        if (out) CopyPattern(out, value);
        result = reinterpret_cast<uintptr_t>(out);
    } else result = primary.Insert(out, value);
    dirty = true;
    return result;
}
// 0x0049EE40
uintptr_t PatternLists::AppendSecondary(const void* descriptor, const PatternParameters* values, uint32_t count)
{
    Words20 value{descriptor, {nullptr, nullptr, 0, 0}};
    if (count >= 1) value.parameters.directory = values->directory;
    if (count >= 2) value.parameters.filename = values->filename;
    if (count >= 3) value.parameters.counts = values->counts;
    if (count >= 4) value.parameters.flags = values->flags;
    auto* out = secondary.end;
    if (reinterpret_cast<uintptr_t>(out) >= reinterpret_cast<uintptr_t>(secondary.capacity)) return secondary.Insert(out, &value);
    secondary.end = reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(out) + sizeof(Words20));
    if (out) CopyPattern(out, &value);
    return reinterpret_cast<uintptr_t>(out);
}
// 0x004A1760
Words20* PatternVector::InsertRange(Words20* position, const Words20* first, const Words20* last, int unused)
{
    (void)unused;
    if (first == last) return const_cast<Words20*>(first);
    const uint32_t incoming = static_cast<uint32_t>(PatternCount(first, last));
    if (incoming > PatternCount(end, capacity)) {
        const uint32_t count = static_cast<uint32_t>(PatternCount(begin, end));
        const uint32_t newCount = (std::max)(count ? count * 2 : 1u, count + incoming);
        auto* storage = newCount ? ts3::CallGameVirtual<Words20*, 2>(allocator,
            static_cast<uint32_t>(sizeof(Words20) * newCount), name, flags) : nullptr;
        Words20* next;
        Construct20Range(&next, begin, position, storage);
        Construct20Range(&next, first, last, next);
        Construct20Range(&next, position, end, next);
        if (begin) ts3::CallGameVirtual<void, 3>(allocator, begin,
            static_cast<uint32_t>(sizeof(Words20) * PatternCount(begin, capacity)));
        begin = storage; end = next;
        capacity = reinterpret_cast<Words20*>(reinterpret_cast<uintptr_t>(storage) + sizeof(Words20) * newCount);
        return next;
    }
    Words20* oldEnd = end;
    const uint32_t tail = static_cast<uint32_t>(PatternCount(position, oldEnd));
    Words20* ignored;
    if (incoming < tail) {
        Construct20Range(&ignored, oldEnd - incoming, oldEnd, oldEnd);
        end += incoming;
        Copy20Backward(position, oldEnd - incoming, oldEnd);
        return Copy20Forward(first, last, position);
    }
    const Words20* split = first + tail;
    Construct20Range(&ignored, split, last, oldEnd);
    end += incoming - tail;
    Construct20Range(&ignored, position, oldEnd, end);
    end += tail;
    return Copy20Backward(first, split, position + tail);
}
// 0x004A0E80
char16_t* CopyPatternString(PatternStringPool* self, const char16_t* text)
{
    uint32_t length = 0;
    while (text[length]) ++length;
    auto* result = static_cast<char16_t*>(AllocateResource(2 * length + 2, &self->arena));
    auto* out = result;
    do { *out++ = *text; } while (*text++);
    return result;
}
}
namespace ts3::resourcesystem {
// 0x004ED300. EA leaves the local buffer uninitialized for an overlong absolute
// path (or null input); do not introduce a different, invented error result.
int32_t CompareResourcePaths(const char16_t* first, const char16_t* second, int32_t mode)
{
    char16_t a[260], b[260];
    if (ts3::CallGameFunction<uint8_t, 0x004ED110>(first, 5)) {
        if (ts3::CallGameFunction<uint32_t, 0x004FE0F0>(first) < 259)
            ts3::CallGameFunction<void, 0x004FDD00>(a, first);
    } else if (first) ts3::CallGameFunction<void, 0x004ECCB0>(a, first, 260, 5);
    if (ts3::CallGameFunction<uint8_t, 0x004ED110>(second, 5)) {
        if (ts3::CallGameFunction<uint32_t, 0x004FE0F0>(second) < 259)
            ts3::CallGameFunction<void, 0x004FDD00>(b, second);
    } else if (second) ts3::CallGameFunction<void, 0x004ECCB0>(b, second, 260, 5);
    if (mode == 2 || mode == 3 || mode == 5 || mode == 6) {
        ts3::CallGameFunction<void, 0x004FF530>(a);
        ts3::CallGameFunction<void, 0x004FF530>(b);
    }
    return ts3::CallGameFunction<int32_t, 0x004FE550>(a, b);
}
}
namespace ts3::resourcesystem {
// 0x004A4090. Selected removal returns the key-buffer free result (or zero),
// whereas complete traversal returns the address of the tree header.
uintptr_t ResourceRemovalManager::Remove(uintptr_t unused0, const char16_t* path, uintptr_t selection, uintptr_t unused1)
{
    (void)unused0; (void)unused1;
    auto* node = tree.right;
    while (node != &tree) {
        auto* resource = node->resource;
        RemovalKeyList keys{nullptr, nullptr, nullptr, "ResourceSystem/ResourceMan/KeyList"};
        if (selection) {
            const auto* name = ts3::CallGameVirtual<const char16_t*, 10>(resource);
            if (CompareResourcePaths(name, path, 5)) {
                if (keys.begin) ts3::CallGameFunction<void, 0x004E3F70>(keys.begin);
                node = ts3::CallGameFunction<RemovalTreeNode*, 0x004E0B20>(node);
                continue;
            }
            if (ts3::CallGameVirtual<int32_t, 22>(resource, selection, observer ? &keys : nullptr) && observer)
                ts3::CallGameVirtual<void, 19>(observer, resource, &keys);
            return keys.begin ? ts3::CallGameFunction<uintptr_t, 0x004E3F70>(keys.begin) : 0;
        }
        if (ts3::CallGameVirtual<int32_t, 23>(resource, observer ? &keys : nullptr) && observer)
            ts3::CallGameVirtual<void, 19>(observer, resource, &keys);
        if (keys.begin) ts3::CallGameFunction<void, 0x004E3F70>(keys.begin);
        node = ts3::CallGameFunction<RemovalTreeNode*, 0x004E0B20>(node);
    }
    return reinterpret_cast<uintptr_t>(&tree);
}
}
namespace ts3::resourcesystem {
namespace {
uint32_t U16Length(const char16_t* s) { uint32_t n=0; while(s[n])++n; return n; }
bool U16Equal(const char16_t* a,const char16_t* b) { while(*a==*b) {if(!*a)return true;++a;++b;}return false; }
bool U16Wild(const char16_t* s) { for(;*s;++s)if(*s==u'?'||*s==u'*')return true;return false; }
void ReleasePatterns(PatternVector& v) {
    if(v.begin) ts3::CallGameVirtual<void,3>(v.allocator,v.begin,static_cast<uint32_t>(reinterpret_cast<uintptr_t>(v.capacity)-reinterpret_cast<uintptr_t>(v.begin)));
}
Words20* AppendSelection(PatternVector& v,const Words20& s) {
    if(reinterpret_cast<uintptr_t>(v.end)>=reinterpret_cast<uintptr_t>(v.capacity))v.Insert(v.end,&s);
    else {*v.end=s;++v.end;}
    return v.end-1;
}
void IncrementSelection(Words20& s,bool second) {
    const auto low=static_cast<uint16_t>(s.parameters.counts+1);
    auto high=static_cast<uint16_t>(s.parameters.counts>>16);
    if(second)++high;
    s.parameters.counts=uint32_t(low)|(uint32_t(high)<<16);
}
struct ConfigurationDirectoryEntry { char16_t name[260]; uint8_t directory; };
TS3_X86_OFFSET(ConfigurationDirectoryEntry,directory,520);
}
void ResourceConfiguration::PathOverflow() {
    error=static_cast<int32_t>(0xD06B0001u);
    if(onError)onError(error,path,0,0,errorContext);
}
void ResourceConfiguration::AddOutput(ConfigurationPattern* pattern,const char16_t* filename) {
    auto* node=static_cast<ConfigurationOutput*>(AllocateResource(sizeof(ConfigurationOutput),&arena));
    if(node)node->attached=nullptr;
    node->value=pattern->value;node->extra=pattern->extra;node->inherited=inherited;node->argument=pattern->argument;
    node->flag=pattern->flag;node->allowMissing=pattern->allowMissing;
    if(node->attached) {
        auto* old=node->attached;node->attached=nullptr;
        ts3::CallGameVirtual<void,1>(reinterpret_cast<uint8_t*>(old)+sizeof(void*));
    }
    node->path=CopyPatternString(reinterpret_cast<PatternStringPool*>(this),filename);
    node->context=context;
    node->previous=last;node->next=reinterpret_cast<ConfigurationOutput*>(&first);
    last=node;node->previous->next=node;
}
// 0x004A2A70. Deliberately restore inherited/depth/context only on success; failure
// retains the expanded path, consumed bits and error state, as EA's callers observe.
bool ResourceConfiguration::Expand(PatternVector* input)
{
    const uint32_t savedInherited=inherited,savedDepth=depth;
    void* const savedContext=context;
    PatternVector patterns{nullptr,nullptr,nullptr,allocator,0,"ResourceSystem/ResourceConfig/ResourceConfiguration/PatternList"};
    char16_t component[260],full[260];
    uint8_t stop=0;bool loaded=false;
    for(auto* f=configFiles->begin;f!=configFiles->end;++f) {
        if(ts3::CallGameFunction<int32_t,0x004ED2A0>(full,*f,path,5) && ts3::CallGameFunction<uint8_t,0x004DBBA0>(full)) {
            if(!ts3::CallGameMethod<uint8_t,0x004A1BD0>(this,full,&patterns,&stop)) {ReleasePatterns(patterns);return false;}
            loaded=true;
        }
    }
    if(loaded)for(auto* s=input->begin;s!=input->end;++s)
        if(!s->parameters.filename && (!*s->parameters.directory || !CompareResourcePaths(s->parameters.directory,u".../",5)))
            static_cast<ConfigurationPattern*>(const_cast<void*>(s->descriptor))->consumed=1;
    if(!stop)patterns.InsertRange(patterns.end,input->begin,input->end,static_cast<int32_t>(reinterpret_cast<uintptr_t>(context)));
    auto* setting=ts3::CallGameFunction<ConfigurationPattern*,0x004A0DC0>(&patterns,0,nullptr);
    if(setting){inherited=setting->value;setting->consumed=1;}
    auto* base=ts3::CallGameFunction<ConfigurationPattern*,0x004A0DC0>(&patterns,1,nullptr);
    if(base){base->consumed=1;AddOutput(base,path);}
    bool wild=false;
    for(auto* s=patterns.begin;s!=patterns.end;++s) {
        s->parameters.flags&=~uint32_t(0xFF);
        bool matches;
        if(*s->parameters.directory) {
            CopyPathComponent(s->parameters.directory,component,260);
            matches=U16Wild(component)||!CompareResourcePaths(component,u".../",5);
        } else matches=s->parameters.filename&&U16Wild(s->parameters.filename);
        if(matches){wild=true;s->parameters.flags|=1;}
    }
    PatternVector next{nullptr,nullptr,nullptr,allocator,0,"ResourceSystem/ResourceConfig/ResourceConfiguration/PatternList"};
    if(wild) {
        const uint8_t mode=ts3::CallGameFunction<uint8_t,0x004EC3C0>(5);
        auto* entry=ts3::CallGameFunction<ConfigurationDirectoryEntry*,0x004DA1A0>(path,nullptr,nullptr);
        if(entry)do {
            next.end=next.begin;
            if(U16Equal(entry->name,u".\\")||U16Equal(entry->name,u"..\\"))continue;
            if(entry->directory) {
                const uint32_t length=U16Length(entry->name);
                if(pathLength+length+1>260) {PathOverflow();ReleasePatterns(next);ReleasePatterns(patterns);return false;}
                std::memcpy(path+pathLength,entry->name,2*length);pathLength+=length;path[pathLength]=0;
                for(auto* s=patterns.begin;s!=patterns.end;++s) {
                    if(!*s->parameters.directory)continue;
                    const uint32_t prefix=CopyPathComponent(s->parameters.directory,component,260);
                    if(!CompareResourcePaths(component,u".../",5)) {
                        IncrementSelection(*AppendSelection(next,*s),true);
                        auto* appended=AppendSelection(next,*s);appended->parameters.directory+=prefix;IncrementSelection(*appended,true);
                    } else if(ts3::CallGameFunction<uint8_t,0x005000F0>(entry->name,component,mode)) {
                        auto* appended=AppendSelection(next,*s);IncrementSelection(*appended,static_cast<uint8_t>(s->parameters.flags)!=0);
                        appended->parameters.directory+=prefix;
                    }
                }
                if(next.begin!=next.end&&!Expand(&next)) {ReleasePatterns(next);ReleasePatterns(patterns);return false;}
                pathLength-=length;path[pathLength]=0;
            } else {
                auto* definition=ts3::CallGameFunction<ConfigurationPattern*,0x004A0DC0>(&patterns,1,entry);
                if(definition) {
                    ts3::CallGameFunction<void,0x004ECE50>(full,entry,path,5);
                    definition->consumed=1;AddOutput(definition,full);
                }
            }
        } while(ts3::CallGameFunction<int32_t,0x004D9AF0>(entry));
        ts3::CallGameFunction<void,0x004D9BA0>(entry);
    } else {
        for(auto* s=patterns.begin;s!=patterns.end;++s) {
            if(!*s->parameters.directory) {
                if(s->parameters.filename) {
                    ts3::CallGameFunction<void,0x004ECE50>(full,s->parameters.filename,path,5);
                    auto* definition=static_cast<ConfigurationPattern*>(const_cast<void*>(s->descriptor));
                    if((definition->allowMissing||ts3::CallGameFunction<uint8_t,0x004DBBA0>(full))&&definition->type==1) {
                        definition->consumed=1;AddOutput(definition,full);
                    }
                }
                continue;
            }
            const uint32_t prefix=CopyPathComponent(s->parameters.directory,component,260);
            if(pathLength+prefix+1>260) {PathOverflow();ReleasePatterns(next);ReleasePatterns(patterns);return false;}
            char16_t previous;
            std::memcpy(&previous,reinterpret_cast<const uint8_t*>(path)+int64_t(pathLength)*2-2,2);
            if(!U16Equal(component,u"\\")||(previous!=u'\\'&&previous!=u'/'&&previous!=u':')) {
                std::memcpy(path+pathLength,component,2*prefix);pathLength+=prefix;path[pathLength]=0;
            }
            next.end=next.begin;
            for(auto* candidate=s;candidate!=patterns.end;++candidate) {
                CopyPathComponent(candidate->parameters.directory,full,260);
                if(CompareResourcePaths(component,full,5))continue;
                auto* definition=static_cast<ConfigurationPattern*>(const_cast<void*>(candidate->descriptor));
                if(!ts3::CallGameFunction<uint8_t,0x004DBF60>(path)&&definition->allowMissing!=1)continue;
                auto* appended=AppendSelection(next,*candidate);appended->parameters.directory+=prefix;
                IncrementSelection(*appended,false);
            }
            if(!Expand(&next)){ReleasePatterns(next);ReleasePatterns(patterns);return false;}
            pathLength-=prefix;path[pathLength]=0; // EA subtracts even when a redundant separator was not appended.
        }
    }
    ReleasePatterns(next);
    inherited=savedInherited;depth=savedDepth;context=savedContext;
    ReleasePatterns(patterns);
    return true;
}
}
