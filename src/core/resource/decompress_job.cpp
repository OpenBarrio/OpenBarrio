#include "ts3/resource/decompress_job.h"

#include <intrin.h>

#include "ts3/io/memory_stream.h"
#include "ts3/layout.h"

namespace ts3::resource {

namespace {

// The container kinds the loader decodes with the built-in RefPack codec (TS3W.exe
// 0x00727AF0, compared against slot 3 of the request's container).
constexpr uint32_t kRefPackKinds[] = {0x2A43BD44u, 0x03C4B9A4u, 0x03DB0260u};

// TS3W.exe 0x008825E0: reference-counted pointer assignment. Keeps `next` (AddRef through
// its slot 1) in `slot`, releasing what was there (slot 2). Equal values change nothing.
void RefAssign(void** slot, void* const* value)
{
    void* next = *value;
    if (next == *slot)
        return;
    if (next)
        CallGameVirtual<void, 1>(next);
    void* previous = *slot;
    *slot = next;
    if (previous)
        CallGameVirtual<void, 2>(previous);
}

// The record attachment's data pointer: attach[4] is a holder whose [8] is the bytes
// (TS3W.exe 0x00727A3D; for a record over a memory stream the attachment is the stream and
// the holder is its shared buffer).
void* DataOf(void* attach)
{
    if (!attach)
        return nullptr;
    void* holder = *reinterpret_cast<void**>(static_cast<uint8_t*>(attach) + 4);
    return holder ? *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(holder) + 8) : nullptr;
}

// TS3W.exe 0x00727140 (still game code, __cdecl: the caller cleans the three stack
// arguments; the register values it is called with are unused): builds the record the
// decoded bytes land in — a memory stream of `memSize` bytes (tag
// "ResourceLoad/AsyncLoader/MemoryStream") wrapped by the loader's record class, keyed by
// the request's key. Fails when either allocation fails or the stream cannot be sized.
// Needs the general heap, so x64 builds stop here.
bool CreateDestinationRecord(void** out, uint32_t memSize, void* key)
{
    *out = nullptr;
#if defined(_M_IX86)
    using Fn = bool(__cdecl*)(void** out, uint32_t memSize, void* key);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(0x00727140))(out, memSize, key);
#else
    (void)memSize;
    (void)key;
    AbortNotRewritten("the async loader's destination record (TS3W.exe 0x00727140)");
    return false;
#endif
}

// TS3W.exe 0x0072A020 (still game code): hands the finished request back to the loader,
// which queues the follow-up work waiting on the resource. Answers whether that worked.
bool Completion(DecompressJobHost* host, DecompressRequest* req)
{
    return CallGameMethod<bool, 0x0072A020>(host, req);
}

}  // namespace

bool DecompressJob(DecompressJobHost* host, const void* taskContext, uint32_t state)
{
    DecompressRequest& req =
        **reinterpret_cast<DecompressRequest* const*>(static_cast<const uint8_t*>(taskContext) + 0x10);
    void* const container = req.container;

    if (state != 4) {
        // TS3W.exe 0x0072796E, the destroy state: stop a read still in flight and drop the
        // source record.
        if (req.ioInFlight) {
            CallGameMethod<void, 0x00401260>(req.asyncIo);  // cancel
            uint32_t timeout = thread::kTimeoutNone;
            CallGameMethod<uint32_t, 0x00401290>(req.asyncIo, &timeout);
            req.ioInFlight = 0;
        }
        if (req.cancelled) {
            req.cancelled = 0;
            CallGameVirtual<void, 21>(container);  // the read's own cleanup
        }
        if (req.sourceRecord) {
            CallGameVirtual<void, 8>(req.sourceRecord);
            if (req.sourceRecord) {
                void* old = req.sourceRecord;
                req.sourceRecord = nullptr;
                CallGameVirtual<void, 2>(old);
            }
        }
        return true;
    }

    // State 4, the execute path (TS3W.exe 0x007279E4): one job and its bytes leave the
    // queue before anything else.
    CallGameVirtual<void, 21>(container);
    req.cancelled = 0;
    _InterlockedExchangeAdd(&host->pendingJobs, -1);
    _InterlockedExchangeAdd(&host->pendingBytes, static_cast<long>(0u - req.storedBytes));
    IResourceManager* manager = CallGameFunction<IResourceManager*, 0x004AFD20>();
    req.flag0x95 = 0;
    req.flag0x96 = 1;

    if (req.compression == kCompressionNone) {
        // TS3W.exe 0x00727BDD: nothing to decode, the read's record becomes the result.
        host->mutex.Lock();
        void* source = req.sourceRecord;
        void* previous = req.resultRecord;
        if (source != previous) {
            if (source)
                CallGameVirtual<void, 1>(source);
            req.resultRecord = source;
            if (previous)
                CallGameVirtual<void, 2>(previous);
        }
        if (req.sourceRecord) {
            void* old = req.sourceRecord;
            req.sourceRecord = nullptr;
            CallGameVirtual<void, 2>(old);
        }
        host->mutex.Unlock();
        return Completion(host, &req);
    }

    // TS3W.exe 0x00727A2C: the compressed bytes and their size, through the source
    // record's attachment.
    void* attach = CallGameVirtual<void*, 6>(req.sourceRecord);
    const void* src = DataOf(attach);
    const uint32_t srcLen = CallGameVirtual<uint32_t, 7>(attach);

    if (req.memSize == 0xFFFFFFFFu) {
        req.failed = 1;  // TS3W.exe 0x00727A63: the read had no size
        return false;
    }

    void* created = nullptr;
    if (!CreateDestinationRecord(&created, req.memSize, &req.key)) {
        // TS3W.exe 0x00727A8C: nothing was built (created stays null), report the failure.
        req.failed = 1;
        if (created)
            CallGameVirtual<void, 2>(created);
        return false;
    }

    attach = CallGameVirtual<void*, 6>(created);
    void* const dst = DataOf(attach);
    const uint32_t cap = CallGameVirtual<uint32_t, 7>(attach);

    // TS3W.exe 0x00727AD4: the built-in RefPack codec, on the stack, keyed by the default
    // allocator. Its slot 2 (0x004EC010) is rewritten as ts3::refpack::Decompress.
    RefPackCodec refPack(io::DefaultAllocatorDirect());
    ICompressionCodec* codec = nullptr;
    if (req.compression == kCompressionRefPack) {
        const uint32_t kind = CallGameVirtual<uint32_t, 3>(container);
        for (const uint32_t id : kRefPackKinds) {
            if (kind == id) {
                codec = &refPack;
                break;
            }
        }
    }
    if (!codec)
        codec = manager->GetCodec(static_cast<uint16_t>(req.compression));  // low 16 bits only
    if (!codec) {
        req.failed = 1;
        CallGameVirtual<void, 2>(created);
        return false;
    }

    // TS3W.exe 0x00727B2C: decode into the fresh buffer; the job accepts nothing less than
    // exactly the destination's size.
    if (codec->Decompress(src, srcLen, dst, cap, 1) != cap) {
        req.failed = 1;
        CallGameVirtual<void, 2>(created);
        return false;
    }

    // TS3W.exe 0x00727B36: the source record is spent; the new one becomes the result.
    CallGameVirtual<void, 8>(req.sourceRecord);
    if (req.sourceRecord) {
        void* old = req.sourceRecord;
        req.sourceRecord = nullptr;
        CallGameVirtual<void, 2>(old);
    }
    host->mutex.Lock();
    RefAssign(&req.resultRecord, &created);
    host->mutex.Unlock();
    CallGameVirtual<void, 2>(created);
    return Completion(host, &req);
}

}  // namespace ts3::resource
