// EA::IO::IStream, the stream interface the game's file and memory streams implement.
// MSVC lays virtual functions out in declaration order, so on x86 this matches the
// game's stream vtables slot for slot. Slots 7, 10 and 12 are confirmed from
// DatabasePackedFile's calls; the others follow EA's interface and are unconfirmed.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3::io {

using size_type = size_t;
using off_type = ptrdiff_t;
inline constexpr size_type kSizeTypeError = static_cast<size_type>(-1);

enum PositionType { kPositionTypeBegin, kPositionTypeCurrent, kPositionTypeEnd };
// What GetState reports.
enum State { kStateSuccess = 0, kStateError = -1, kStateNotOpen = -2 };

class IStream {
public:
    virtual ~IStream() = default;                                                             // 0
    virtual int AddRef() = 0;                                                                 // 1
    virtual int Release() = 0;                                                                // 2
    virtual uint32_t GetType() const = 0;                                                     // 3
    virtual int GetAccessFlags() const = 0;                                                   // 4
    virtual int GetState() const = 0;                                                         // 5
    virtual bool Close() = 0;                                                                 // 6
    virtual size_type GetSize() const = 0;                                                    // 7 (+0x1C)
    virtual bool SetSize(size_type size) = 0;                                                 // 8
    virtual off_type GetPosition(PositionType type = kPositionTypeBegin) const = 0;           // 9
    virtual bool SetPosition(off_type position, PositionType type = kPositionTypeBegin) = 0;  // 10 (+0x28)
    virtual size_type GetAvailable() const = 0;                                               // 11
    virtual size_type Read(void* data, size_type size) = 0;                                   // 12 (+0x30)
    virtual bool Flush() = 0;                                                                 // 13
    virtual bool Write(const void* data, size_type size) = 0;                                 // 14
};

}  // namespace ts3::io
