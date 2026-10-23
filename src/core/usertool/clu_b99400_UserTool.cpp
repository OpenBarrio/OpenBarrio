#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"
#include "ts3/usertool/clu_b99400_UserTool.h"

namespace ts3::usertool {
namespace {

// The container stores 0x20-byte query records on x86. Padding adapts the record
// to native pointers in the x64 build while retaining that fixed record stride.
#pragma pack(push, 4)
struct QueryRecord {
    uint32_t unknown;
    void** nodes;
    uint32_t stopIndex;
    uint8_t padding[0x20 - sizeof(uint32_t) - sizeof(void*) - sizeof(uint32_t)];
};
TS3_X86_OFFSET(QueryRecord, nodes, 0x04);
TS3_X86_OFFSET(QueryRecord, stopIndex, 0x08);
TS3_X86_SIZE(QueryRecord, 0x20);
#pragma pack(pop)

struct UserTool {
    uint8_t prefix[0x0C];
    QueryRecord* recordsBegin;
    QueryRecord* recordsEnd;
    uint8_t beforeMutex[0xA0 - 0x0C - 2 * sizeof(void*)];
    uint8_t mutexStorage[0x30];
};
TS3_X86_OFFSET(UserTool, recordsBegin, 0x0C);
TS3_X86_OFFSET(UserTool, recordsEnd, 0x10);
TS3_X86_OFFSET(UserTool, mutexStorage, 0xA0);

struct OutputVector {
    void** begin;
    void** end;
    void** capacity;
};
TS3_X86_OFFSET(OutputVector, begin, 0x00);
TS3_X86_OFFSET(OutputVector, end, 0x04);
TS3_X86_OFFSET(OutputVector, capacity, 0x08);

struct TreeNode {
    void* value;
    uint32_t unknown;
    TreeNode* next;
};
TS3_X86_OFFSET(TreeNode, value, 0x00);
TS3_X86_OFFSET(TreeNode, next, 0x08);

void Append(OutputVector* output, void* value)
{
    void** slot = output->end;
    if (reinterpret_cast<uintptr_t>(slot) < reinterpret_cast<uintptr_t>(output->capacity)) {
        output->end = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(slot) + sizeof(void*));
        // EA quirk: the cursor advances even when it is null; no value is stored.
        if (slot)
            *slot = value;
    } else {
        ts3::CallGameMethod<void*, 0x00B7B1E0>(output, slot, &value);
    }
}

}  // namespace

// 0x00B99400: under the embedded mutex, append values from each query's first
// node through (but excluding) its selected stop node. The output vector grow
// path remains delegated to the original EA helper at 0x00B7B1E0.
int32_t __fastcall AppendUserToolValues(void* self, void*, void* output)
{
    auto* tool = static_cast<UserTool*>(self);
    auto* vector = static_cast<OutputVector*>(output);
    auto* mutex = static_cast<void*>(tool->mutexStorage);
    ts3::CallGameMethod<int32_t, 0x004E16F0>(mutex, reinterpret_cast<void*>(0x0106FB00));

    const uint32_t count = static_cast<uint32_t>(static_cast<int32_t>(
        reinterpret_cast<uintptr_t>(tool->recordsEnd) - reinterpret_cast<uintptr_t>(tool->recordsBegin)) >> 5);
    for (uint32_t recordIndex = 0; recordIndex != count; ++recordIndex) {
        QueryRecord& record = tool->recordsBegin[recordIndex];
        void** slot = record.nodes;
        TreeNode* node = static_cast<TreeNode*>(slot[0]);
        if (!node) {
            do {
                ++slot;
            } while (!*slot);
            node = static_cast<TreeNode*>(*slot);
        }

        auto* const stop = static_cast<TreeNode*>(record.nodes[record.stopIndex]);
        while (node != stop) {
            Append(vector, node->value);
            node = node->next;
            while (!node) {
                ++slot;
                node = static_cast<TreeNode*>(*slot);
            }
        }
    }

    return ts3::CallGameMethod<int32_t, 0x004E17B0>(mutex);
}

// 0x00C04050: only the two known UserTool type identifiers pass.
int32_t __cdecl IsAcceptedType(int32_t type)
{
    return type == 17 || type == 28;
}

}  // namespace ts3::usertool
