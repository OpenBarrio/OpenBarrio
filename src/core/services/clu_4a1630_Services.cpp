#include "ts3/services/clu_4a1630_Services.h"
#include <algorithm>
#include <cstring>
namespace ts3::services {
AppendString16* AppendString16::Append(const char16_t* first, const char16_t* last)
{
    if (first == last) return this;
    const auto count = last - first;
    const auto length = end - begin;
    const auto capacity = static_cast<size_t>(limit - begin) - 1;
    const auto needed = static_cast<size_t>(length + count);
    if (needed <= capacity) {
        // EA delays reading first[0] until after moving the tail and writing the terminator.
        std::memmove(end + 1, first + 1, (count - 1) * sizeof(char16_t));
        end[count] = 0;
        *end = *first;
        end += count;
        return this;
    }
    const auto newCapacity = std::max(needed, capacity <= 8 ? size_t(8) : capacity * 2);
    const auto bytes = (newCapacity + 1) * sizeof(char16_t);
    auto* fresh = CallGameVirtual<char16_t*, 2>(allocator, bytes, name, flags);
    std::memmove(fresh, begin, length * sizeof(char16_t));
    std::memmove(fresh + length, first, count * sizeof(char16_t));
    fresh[length + count] = 0;
    const auto oldSize = limit - begin;
    if (oldSize > 1 && begin && begin != inlineBuffer)
        CallGameVirtual<void, 3>(allocator, begin, oldSize * sizeof(char16_t));
    begin = fresh;
    end = fresh + length + count;
    limit = fresh + newCapacity + 1;
    return this;
}
void AppendTree::Clear(AppendTreeNode* node)
{
    while (node) {
        Clear(node->child);
        auto* next = node->next;
        const auto span = reinterpret_cast<intptr_t>(node->textLimit) - reinterpret_cast<intptr_t>(node->text);
        if ((span & ~intptr_t(1)) > 2 && node->text)
            CallGameFunction<void, 0x004E3F70>(node->text);
        CallGameVirtual<void, 3>(allocator, node, size_t(sizeof(AppendTreeNode)));
        node = next;
    }
}
}
