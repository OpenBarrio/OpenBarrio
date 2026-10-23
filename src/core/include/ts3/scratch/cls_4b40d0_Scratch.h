#pragma once
#include "ts3/layout.h"
#include <cstdio>

namespace ts3::scratch {
struct XmlString {
    uint32_t length, capacity;
    char data[1];
};
TS3_X86_OFFSET(XmlString, data, 8);
// Stamp dependency F62C90 also uses the last stamped input and tab width.
struct XmlCursor { int32_t row, column; const char* stamped; int32_t tabWidth; };
TS3_X86_OFFSET(XmlCursor, column, 4);
TS3_X86_OFFSET(XmlCursor, stamped, 8);
TS3_X86_OFFSET(XmlCursor, tabWidth, 12);
TS3_X86_SIZE(XmlCursor, 16);
struct XmlNode {
    void** vtable;
    int32_t row, column;
    void* user;
    XmlNode* parent;
    uint32_t type;
    XmlNode* first;
    XmlNode* last;
    XmlString* value;
    XmlNode* previous;
    XmlNode* next;
    XmlNode* Init(uint32_t nodeType);                     // F619F0, thiscall ret 4
    uintptr_t Accept7(void* visitor);                    // 4B40D0, thiscall ret 4
    uintptr_t Accept8(void* visitor);                    // 4B40F0, thiscall ret 4
    bool IsWhitespace();                                // F63340, thiscall
    const char* ParseChildren(const char*, XmlCursor*, int); // F63780, thiscall ret 12
};
TS3_X86_OFFSET(XmlNode, row, 4);
TS3_X86_OFFSET(XmlNode, column, 8);
TS3_X86_OFFSET(XmlNode, user, 12);
TS3_X86_OFFSET(XmlNode, parent, 16);
TS3_X86_OFFSET(XmlNode, type, 20);
TS3_X86_OFFSET(XmlNode, first, 24);
TS3_X86_OFFSET(XmlNode, last, 28);
TS3_X86_OFFSET(XmlNode, value, 32);
TS3_X86_OFFSET(XmlNode, previous, 36);
TS3_X86_OFFSET(XmlNode, next, 40);
TS3_X86_SIZE(XmlNode, 44);
struct XmlText : XmlNode {
    uint8_t cdata;
    uint8_t padding[3];
    XmlText* InitText(const char*);                       // F62530, ret 4
    XmlText* Destroy(uint32_t flags);                    // F625A0, ret 4
    XmlText* Clone();                                    // F62920
    uintptr_t Print(std::FILE*, int depth);              // F62100, ret 8
    const char* Parse(const char*, XmlCursor*, int);      // F643B0, ret 12
};
TS3_X86_OFFSET(XmlText, cdata, 44);
TS3_X86_SIZE(XmlText, 48);
struct XmlDeclaration : XmlNode {
    XmlString* version;
    XmlString* encoding;
    XmlString* standalone;
    XmlDeclaration* InitDeclaration();                   // F625F0
    XmlDeclaration* Destroy(uint32_t flags);             // F62630, ret 4
    XmlDeclaration* Clone();                             // F629A0
    uintptr_t CopyTo(XmlDeclaration*);                   // F62320, ret 4
    uintptr_t Print(std::FILE*, int depth);              // F621B0, ret 8, virtual slot 17
    uintptr_t PrintTo(std::FILE*, int depth, XmlString**);// F621D0, ret 12
};
TS3_X86_OFFSET(XmlDeclaration, version, 44);
TS3_X86_OFFSET(XmlDeclaration, encoding, 48);
TS3_X86_OFFSET(XmlDeclaration, standalone, 52);
TS3_X86_SIZE(XmlDeclaration, 56);
struct XmlDocument : XmlNode {
    uint8_t error;
    uint8_t padding[3];
    uint32_t errorCode;
    XmlString* errorText;
    uint32_t unused;
    int32_t errorRow, errorColumn;
};
TS3_X86_OFFSET(XmlDocument, error, 44);
TS3_X86_OFFSET(XmlDocument, errorCode, 48);
TS3_X86_OFFSET(XmlDocument, errorText, 52);
TS3_X86_OFFSET(XmlDocument, errorRow, 60);
TS3_X86_OFFSET(XmlDocument, errorColumn, 64);
XmlString* XmlEmpty();
}
