// EA::IO::FileStream (TS3W.exe vtable 0x00FB8300, 0x228 bytes on x86): an IStream over a
// Win32 file. Every package the game opens itself reads through one (DatabasePackedFile's
// own stream at +0x34), and write records spill large data into one.
//
// Like MemoryStream, a struct with the game's layout: on x86 the harness hands the game's
// objects to these methods. Calls the original makes through its own vtable (GetPosition
// and SetPosition from SetSize, GetSize from GetPosition) go straight to these methods.
#pragma once

#include <cstdint>

#include "ts3/io/stream.h"
#include "ts3/layout.h"

namespace ts3::io {

inline constexpr uint32_t kFileStreamType = 0x34722300;  // what GetType returns
inline constexpr uint32_t kMaxPath = 260;

struct FileStream {
    void* const* vtable;
    void* handle;                 // 0x004 Win32 handle; INVALID_HANDLE_VALUE when closed
    wchar_t path[kMaxPath];       // 0x008
    int32_t refCount;             // 0x210
    uint32_t accessFlags;         // 0x214 1 read, 2 write; 0 when closed
    uint32_t creationMode;        // 0x218 1 new, 2 always, 3 open existing, 4 open always, 5 truncate
    uint32_t shareFlags;          // 0x21C 1 read, 2 read and write
    uint32_t usageHints;          // 0x220 1 sequential, 2 random access
    int32_t lastError;            // 0x224 GetLastError of the last failed call

    // Constructors on uninitialised memory (x86 builds give the object the game's vtable):
    // TS3W.exe 0x004DB910 (UTF-8 path, converted) and 0x004DB970 (wide path, copied). A path
    // of 260 characters or more isn't terminated (as in the original).
    void Construct(const char* utf8Path);
    void Construct(const wchar_t* widePath);
    // TS3W.exe 0x004DB9D0: closes the file; the object is left with IStream's vtable.
    void Destruct();
    // TS3W.exe 0x004DBA20: Destruct, then frees the object with EA's operator delete when
    // flags & 1.
    void DeletingDestruct(uint32_t flags);

    // Slot 1 (0x004DB380).
    int AddRef();
    // Slot 2 (0x004DB390): the last reference destroys the object through vtable slot 0
    // (with the count still at 1) and returns 0.
    int Release();
    // Slot 3 (0x004559C0, shared with other classes).
    uint32_t GetType() const { return kFileStreamType; }
    // Slot 4 (0x004DB640).
    uint32_t GetAccessFlags() const { return accessFlags; }
    // Slot 5 (0x004DB650): kStateNotOpen when closed, else the last error (0 = fine).
    int GetState() const;
    // Slot 6 (0x004DB600): always true.
    bool Close();
    // Slot 7 (0x004DB670): the low 32 bits of the size; kSizeTypeError when closed or failing.
    size_type GetSize();
    // Slot 8 (0x004DB6B0): truncates or extends, then restores the position.
    bool SetSize(size_type size);
    // Slot 9 (0x004DB740): from the end it is position - size; "current" is always 0. Not
    // checked for a closed file (the Win32 call fails: -1, lastError unchanged).
    off_type GetPosition(PositionType type);
    // Slot 10 (0x004DB7D0). Types other than current and end count from the start.
    bool SetPosition(off_type position, PositionType type);
    // Slot 11 (0x004DB840): -GetPosition(end).
    size_type GetAvailable();
    // Slot 12 (0x004DB850): bytes read, kSizeTypeError on failure.
    size_type Read(void* dst, size_type size);
    // Slot 13 (0x004DB8E0).
    bool Flush();
    // Slot 14 (0x004DB8A0): a short write still counts as success.
    bool Write(const void* src, size_type size);
    // Slots 15 and 16 (0x004DB3E0, 0x004DB3B0; MSVC orders the overloads in reverse): ignored
    // while the file is open or for a null path.
    void SetPath(const wchar_t* newPath);
    void SetPath(const char* utf8Path);
    // Slots 17 and 18 (0x004DB470, 0x004DB410). The wide one copies when dst and capacity
    // are given and returns the path's length; the UTF-8 one returns the converted length
    // (into a scratch buffer when there is no dst).
    uint32_t GetPath(wchar_t* dst, uint32_t capacity);
    uint32_t GetPath(char* dst, uint32_t capacity);
    // Slot 19 (0x004DB4C0): access 1 read, 2 write; creation 1-5 as CreateFileW (6 picks
    // open-always for write access, else open-existing; others open existing); share 1
    // read, 2 read and write; usage 1 sequential scan, 2 random access. An open file stays
    // open (true); access 0 fails.
    bool Open(uint32_t access, uint32_t creation, uint32_t share, uint32_t usage);
};
TS3_X86_OFFSET(FileStream, handle, 0x004);
TS3_X86_OFFSET(FileStream, path, 0x008);
TS3_X86_OFFSET(FileStream, refCount, 0x210);
TS3_X86_OFFSET(FileStream, lastError, 0x224);
TS3_X86_SIZE(FileStream, 0x228);

}  // namespace ts3::io
