#pragma once
#include <cstdint>

namespace ts3::io {
// Set once before installing hooks; empty restores EA's folder lookup.
void SetUserDocumentsDir(const wchar_t* path);
// TS3W.exe 0x004DC100: look up a special directory and return its length or -1. EA's colon separator and fixed 260-character shell limit are kept.
int SpecialDirectory(int folderId, wchar_t* path, char requireDirectory, uint32_t capacity);
}
