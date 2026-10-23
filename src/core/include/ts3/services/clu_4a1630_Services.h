#pragma once
#include "ts3/layout.h"
namespace ts3::services {
struct AppendString16 {
    char16_t* begin;
    char16_t* end;
    char16_t* limit;
    void* allocator;
    unsigned flags;
    const char* name;
    char16_t* inlineBuffer;
    // TS3W.exe 0x004A1630: append a UTF-16 range, preserving alias-sensitive copy order.
    AppendString16* Append(const char16_t* first, const char16_t* last);
};
struct AppendTreeNode {
    AppendTreeNode* child;
    AppendTreeNode* next;
    void* fields[2];
    char16_t* text;
    char16_t* textEnd;
    char16_t* textLimit;
    void* tail[2];
};
struct AppendTree {
    void* fields[6];
    void* allocator;
    // TS3W.exe 0x004A4A10: destroy child/sibling chains with the original allocation policies.
    void Clear(AppendTreeNode* node);
};
TS3_X86_OFFSET(AppendString16, end, 4);
TS3_X86_OFFSET(AppendString16, limit, 8);
TS3_X86_OFFSET(AppendString16, allocator, 12);
TS3_X86_OFFSET(AppendString16, flags, 16);
TS3_X86_OFFSET(AppendString16, name, 20);
TS3_X86_OFFSET(AppendString16, inlineBuffer, 24);
TS3_X86_SIZE(AppendString16, 28);
TS3_X86_OFFSET(AppendTreeNode, text, 16);
TS3_X86_OFFSET(AppendTreeNode, textLimit, 24);
TS3_X86_SIZE(AppendTreeNode, 36);
TS3_X86_OFFSET(AppendTree, allocator, 24);
TS3_X86_SIZE(AppendTree, 28);
}
