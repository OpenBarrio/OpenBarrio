#include "ts3/io/file_stream.h"

#include <cwchar>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ts3::io {

namespace {

void SetGameVtable(FileStream& s, uintptr_t address)
{
#if defined(_M_IX86)
    s.vtable = reinterpret_cast<void* const*>(address);
#else
    (void)address;
    s.vtable = nullptr;
#endif
}

constexpr uintptr_t kFileStreamVtable = 0x00FB8300;
constexpr uintptr_t kIStreamVtable = 0x00FA99FC;

HANDLE Handle(const FileStream& s) { return static_cast<HANDLE>(s.handle); }
bool IsOpen(const FileStream& s) { return s.handle != INVALID_HANDLE_VALUE; }

// wcsncpy: at most `count` characters, the rest of the `count` filled with zeros, no
// terminator when src is that long.
void CopyWide(wchar_t* dst, const wchar_t* src, size_t count)
{
    size_t i = 0;
    for (; i < count && src[i]; ++i)
        dst[i] = src[i];
    for (; i < count; ++i)
        dst[i] = 0;
}

void ConstructFields(FileStream& s)
{
    SetGameVtable(s, kFileStreamVtable);
    s.handle = INVALID_HANDLE_VALUE;
    s.refCount = 0;
    s.accessFlags = 0;
    s.creationMode = 0;
    s.shareFlags = 0;
    s.usageHints = 0;
    s.lastError = 0;
    s.path[0] = 0;
}

}  // namespace

void FileStream::Construct(const char* utf8Path)
{
    ConstructFields(*this);
    if (utf8Path)
        MultiByteToWideChar(CP_UTF8, 0, utf8Path, -1, path, kMaxPath);
}

void FileStream::Construct(const wchar_t* widePath)
{
    ConstructFields(*this);
    if (widePath)
        CopyWide(path, widePath, kMaxPath);
}

void FileStream::Destruct()
{
    SetGameVtable(*this, kFileStreamVtable);
    Close();
    SetGameVtable(*this, kIStreamVtable);
}

void FileStream::DeletingDestruct(uint32_t flags)
{
    Destruct();
    if (flags & 1)
        CallGameFunction<void, 0x004E3F70>(static_cast<void*>(this));
}

int FileStream::AddRef()
{
    return ++refCount;
}

int FileStream::Release()
{
    if (refCount > 1)
        return --refCount;
    CallGameVirtual<void, 0>(this, 1u);
    return 0;
}

int FileStream::GetState() const
{
    return IsOpen(*this) ? lastError : kStateNotOpen;
}

bool FileStream::Close()
{
    if (IsOpen(*this)) {
        CloseHandle(Handle(*this));
        handle = INVALID_HANDLE_VALUE;
        accessFlags = 0;
        creationMode = 0;
        shareFlags = 0;
        usageHints = 0;
    }
    return true;
}

size_type FileStream::GetSize()
{
    if (!IsOpen(*this))
        return kSizeTypeError;
    LARGE_INTEGER size;
    if (GetFileSizeEx(Handle(*this), &size))
        return static_cast<size_type>(size.LowPart);
    lastError = static_cast<int32_t>(GetLastError());
    return kSizeTypeError;
}

bool FileStream::SetSize(size_type size)
{
    if (!IsOpen(*this))
        return false;
    const off_type position = GetPosition(kPositionTypeBegin);
    LARGE_INTEGER distance, unused;
    distance.QuadPart = static_cast<uint32_t>(size);
    if (SetFilePointerEx(Handle(*this), distance, &unused, FILE_BEGIN) && SetEndOfFile(Handle(*this))) {
        SetPosition(position, kPositionTypeBegin);
        return true;
    }
    SetPosition(position, kPositionTypeBegin);
    lastError = static_cast<int32_t>(GetLastError());
    return false;
}

off_type FileStream::GetPosition(PositionType type)
{
    LARGE_INTEGER zero, position;
    zero.QuadPart = 0;
    if (!SetFilePointerEx(Handle(*this), zero, &position, FILE_CURRENT))
        return -1;
    if (type == kPositionTypeBegin)
        return static_cast<off_type>(position.LowPart);
    if (type != kPositionTypeEnd)
        return 0;
    const size_type size = GetSize();
    if (size == kSizeTypeError)
        return -1;
    return static_cast<off_type>(position.LowPart - static_cast<uint32_t>(size));
}

bool FileStream::SetPosition(off_type position, PositionType type)
{
    if (!IsOpen(*this))
        return false;
    const DWORD method = type == kPositionTypeCurrent ? FILE_CURRENT : type == kPositionTypeEnd ? FILE_END : FILE_BEGIN;
    LARGE_INTEGER distance, unused;
    distance.QuadPart = static_cast<int32_t>(position);
    if (SetFilePointerEx(Handle(*this), distance, &unused, method))
        return true;
    lastError = static_cast<int32_t>(GetLastError());
    return false;
}

size_type FileStream::GetAvailable()
{
    return static_cast<size_type>(-GetPosition(kPositionTypeEnd));
}

size_type FileStream::Read(void* dst, size_type size)
{
    if (!IsOpen(*this))
        return kSizeTypeError;
    DWORD read = 0;
    if (ReadFile(Handle(*this), dst, static_cast<DWORD>(size), &read, nullptr))
        return read;
    lastError = static_cast<int32_t>(GetLastError());
    return kSizeTypeError;
}

bool FileStream::Flush()
{
    if (!IsOpen(*this))
        return false;
    if (FlushFileBuffers(Handle(*this)))
        return true;
    lastError = static_cast<int32_t>(GetLastError());
    return false;
}

bool FileStream::Write(const void* src, size_type size)
{
    if (!IsOpen(*this))
        return false;
    DWORD written = 0;
    if (WriteFile(Handle(*this), src, static_cast<DWORD>(size), &written, nullptr))
        return true;
    lastError = static_cast<int32_t>(GetLastError());
    return false;
}

void FileStream::SetPath(const wchar_t* newPath)
{
    if (!IsOpen(*this) && newPath)
        CopyWide(path, newPath, kMaxPath);
}

void FileStream::SetPath(const char* utf8Path)
{
    if (!IsOpen(*this) && utf8Path)
        MultiByteToWideChar(CP_UTF8, 0, utf8Path, -1, path, kMaxPath);
}

uint32_t FileStream::GetPath(wchar_t* dst, uint32_t capacity)
{
    if (dst && capacity) {
        CopyWide(dst, path, capacity);
        dst[capacity - 1] = 0;
    }
    return static_cast<uint32_t>(wcslen(path));
}

uint32_t FileStream::GetPath(char* dst, uint32_t capacity)
{
    if (dst && capacity)
        return static_cast<uint32_t>(WideCharToMultiByte(CP_UTF8, 0, path, -1, dst, static_cast<int>(capacity), nullptr, nullptr) - 1);
    char scratch[kMaxPath];
    return GetPath(scratch, kMaxPath);
}

bool FileStream::Open(uint32_t access, uint32_t creation, uint32_t share, uint32_t usage)
{
    if (IsOpen(*this))
        return true;
    if (!access)
        return false;
    DWORD desiredAccess = 0;
    if (access & 1)
        desiredAccess = GENERIC_READ;
    if (access & 2)
        desiredAccess |= GENERIC_WRITE;
    DWORD shareMode = 0;
    if (share & 1)
        shareMode = FILE_SHARE_READ;
    if (share & 2)
        shareMode |= FILE_SHARE_READ | FILE_SHARE_WRITE;
    if (creation == 6)
        creation = (access & 2) ? 4 : 3;
    const DWORD disposition = creation >= 1 && creation <= 5 ? creation : OPEN_EXISTING;
    DWORD attributes = 0;
    if (usage & 1)
        attributes = FILE_FLAG_SEQUENTIAL_SCAN;
    else if (usage & 2)
        attributes = FILE_FLAG_RANDOM_ACCESS;
    handle = CreateFileW(path, desiredAccess, shareMode, nullptr, disposition, attributes, nullptr);
    if (!IsOpen(*this)) {
        lastError = static_cast<int32_t>(GetLastError());
        return false;
    }
    accessFlags = access;
    creationMode = creation;
    shareFlags = share;
    usageHints = usage;
    lastError = 0;
    return true;
}

}  // namespace ts3::io
