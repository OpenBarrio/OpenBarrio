# Coverage

Every function listed here is a rewrite of one function of the original game, identified by its address in
the original executable. Each one was checked against the original with a differential test (the same
inputs through the original and the rewrite, results compared), on x86 and x64 builds. The tests run
privately against a legally owned copy of the game and aren't part of this repository yet.

**319 functions** in this release:

| Module | Functions |
|---|---|
| Package reader (DBPF reader, RefPack, FNV hashes, packed records) | 41 |
| m01 Streams and file I/O | 69 |
| m02 Core runtime (threads, TLS, job manager, stopwatch, allocator helpers) | 136 |
| Other verified functions (not on a module list yet) | 72 |
| m06_audio (one function used by the code above) | 1 |

Not in this release yet: 159 more finished functions
of these modules live in source files that also contain code of modules that aren't finished. They follow
once those files are complete. Four files with functions that are still being verified are left out too.

## Package reader (DBPF reader, RefPack, FNV hashes, packed records)

| Original address | Rewrite | File |
|---|---|---|
| `0x004A3A50` | `ts3::world408cc0::LessWorldKey` | `src/core/world/clu_408cc0_World.cpp` |
| `0x004A4230` | `ts3::ui::Utf16Vector::Assign4A4230` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x004A4290` | `ts3::world408cc0::LessWorldString` | `src/core/world/clu_408cc0_World.cpp` |
| `0x004A7AD0` | `ts3::resource::IsValidPackageHeader` | `src/core/resource/package_header.cpp` |
| `0x004A9B20` | `ts3::scratch::ScratchLoaderHost::RunLocked` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004A9C50` | `ts3::misc::SortIndexNodeRange` | `src/core/misc/cls_4a9c50_x.cpp` |
| `0x004A9CA0` | `ts3::misc::SortIndexNodeRangeUnguarded` | `src/core/misc/cls_4a9c50_x.cpp` |
| `0x004A9E90` | `ts3::misc::clu4a9e90::Median64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004A9F10` | `ts3::misc::clu4a9e90::PushPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004A9F70` | `ts3::misc::clu4a9e90::PushSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004A9FD0` | `ts3::misc::clu4a9e90::PushRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA280` | `ts3::misc::FreeIndexMapNodes` | `src/core/misc/cls_4a9c50_x.cpp` |
| `0x004AA2D0` | `ts3::misc::clu4a9e90::AdjustPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA340` | `ts3::misc::clu4a9e90::AdjustSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA3C0` | `ts3::misc::clu4a9e90::AdjustRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA470` | `ts3::resource::IndexCompressed::GetRecord` | `src/core/resource/package_index.cpp` |
| `0x004AA570` | `ts3::misc::EraseIndexMapNode` | `src/core/misc/cls_4a9c50_x.cpp` |
| `0x004AA730` | `ts3::misc::clu4a9e90::HashTable::Rehash` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA7E0` | `ts3::misc::clu4a9e90::Tree::InsertNode` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA860` | `ts3::misc::clu4a9e90::MakePointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA8B0` | `ts3::misc::clu4a9e90::SortPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA910` | `ts3::misc::clu4a9e90::MakeSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA970` | `ts3::misc::clu4a9e90::SortSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AA9F0` | `ts3::misc::clu4a9e90::MakeRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AAA70` | `ts3::misc::clu4a9e90::SortRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AAFD0` | `ts3::resource::IndexCompressed::IsEmpty` | `src/core/include/ts3/resource/package_index.h` |
| `0x004AB1A0` | `ts3::misc::clu4a9e90::Tree::InsertUnique` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB230` | `ts3::misc::clu4a9e90::PartitionPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB280` | `ts3::misc::clu4a9e90::PartialPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB2F0` | `ts3::misc::clu4a9e90::PartitionSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB360` | `ts3::misc::clu4a9e90::PartialSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB3F0` | `ts3::misc::clu4a9e90::PartitionRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB490` | `ts3::misc::clu4a9e90::PartialRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB691` | `ts3::misc::clu4a9e90::HashTable::Insert` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB890` | `ts3::misc::clu4a9e90::Tree::InsertHint` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004AB950` | `ts3::misc::clu4a9e90::QuickPointer` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004ABA10` | `ts3::misc::clu4a9e90::QuickSorted` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004ABCE0` | `ts3::misc::clu4a9e90::QuickRecord` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004ABF00` | `ts3::misc::clu4a9e90::Tree::Insert` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x004ADBB0` | `ts3::io::MemoryStream::Data` | `src/core/include/ts3/io/memory_stream.h` |
| `0x004ADBB0` | `ts3::misc::clu4a9e90::Stream::Data` | `src/core/misc/clu_4a9e90_x.cpp` |

## m01 Streams and file I/O

| Original address | Rewrite | File |
|---|---|---|
| `0x004559C0` | `ts3::io::m01::FileType` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004559F0` | `ts3::io::m01::MemoryAddRef` | `src/core/io/m01_stream_leaves.cpp` |
| `0x00455A00` | `ts3::io::m01::MemoryRelease` | `src/core/io/m01_streams_00455a00.cpp` |
| `0x00455A20` | `ts3::io::m01::MemoryType` | `src/core/io/m01_stream_leaves.cpp` |
| `0x00455A30` | `ts3::io::m01::MemoryClose` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004CDC40` | `ts3::io::m01::DestroyMemory` | `src/core/io/m01_streams_004cdc40.cpp` |
| `0x004DB380` | `ts3::io::FileStream::AddRef` | `src/core/io/file_stream.cpp` |
| `0x004DB390` | `ts3::io::FileStream::Release` | `src/core/io/file_stream.cpp` |
| `0x004DB3B0` | `ts3::io::FileStream::SetPath` | `src/core/io/file_stream.cpp` |
| `0x004DB3E0` | `ts3::io::FileStream::SetPath` | `src/core/io/file_stream.cpp` |
| `0x004DB410` | `ts3::io::FileStream::GetPath` | `src/core/io/file_stream.cpp` |
| `0x004DB470` | `ts3::io::FileStream::GetPath` | `src/core/io/file_stream.cpp` |
| `0x004DB4C0` | `ts3::io::FileStream::Open` | `src/core/io/file_stream.cpp` |
| `0x004DB600` | `ts3::io::FileStream::Close` | `src/core/io/file_stream.cpp` |
| `0x004DB640` | `ts3::io::FileStream::GetAccessFlags` | `src/core/io/file_stream.cpp` |
| `0x004DB650` | `ts3::io::FileStream::GetState` | `src/core/io/file_stream.cpp` |
| `0x004DB670` | `ts3::io::FileStream::GetSize` | `src/core/io/file_stream.cpp` |
| `0x004DB6B0` | `ts3::io::FileStream::SetSize` | `src/core/io/file_stream.cpp` |
| `0x004DB740` | `ts3::io::FileStream::GetPosition` | `src/core/io/file_stream.cpp` |
| `0x004DB7D0` | `ts3::io::FileStream::SetPosition` | `src/core/io/file_stream.cpp` |
| `0x004DB840` | `ts3::io::FileStream::GetAvailable` | `src/core/io/file_stream.cpp` |
| `0x004DB850` | `ts3::io::FileStream::Read` | `src/core/io/file_stream.cpp` |
| `0x004DB8A0` | `ts3::io::FileStream::Write` | `src/core/io/file_stream.cpp` |
| `0x004DB8E0` | `ts3::io::FileStream::Flush` | `src/core/io/file_stream.cpp` |
| `0x004DBA20` | `ts3::io::FileStream::DeletingDestruct` | `src/core/io/file_stream.cpp` |
| `0x004DEA60` | `ts3::io::m01::DestroyStreamBase` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004DEAC0` | `ts3::io::m01::MemorySetOption` | `src/core/io/m01_streams_004deac0.cpp` |
| `0x004DEBA0` | `ts3::io::m01::WrapBuffer` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004DEBE0` | `ts3::io::m01::AllocateBuffer` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004DEDE0` | `ts3::io::m01::AttachShared` | `src/core/io/m01_streams_004dede0.cpp` |
| `0x004DF0C0` | `ts3::io::m01::ConstructShared` | `src/core/io/m01_stream_leaves.cpp` |
| `0x004F3CA0` | `ts3::io::m01::ContinuationCount` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x005BFB90` | `ts3::io::m01::MemoryState` | `src/core/io/m01_stream_leaves.cpp` |
| `0x006D8F20` | `ts3::io::m01::DestroyBuffer` | `src/core/io/m01_stream_leaves.cpp` |
| `0x00774D50` | `ts3::casutils::StoreBytes774D50` | `src/core/casutils/clu_774d50_CASUtils.cpp` |
| `0x00774DB0` | `ts3::io::m01::BindingName` | `src/core/io/m01_stream_leaves.cpp` |
| `0x00774DC0` | `ts3::io::m01::LookupStream` | `src/core/io/m01_streams_00774dc0.cpp` |
| `0x00774E30` | `ts3::casutils::ResolveCasObject774E30` | `src/core/casutils/clu_774d50_CASUtils.cpp` |
| `0x00774E90` | `ts3::casutils::SetCasProperty774E90` | `src/core/casutils/clu_774d50_CASUtils.cpp` |
| `0x00774F30` | `ts3::io::m01::WriteInt64` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00774FC0` | `ts3::io::m01::WriteFloat` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775040` | `ts3::io::m01::WriteDouble` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007750C0` | `ts3::io::m01::WriteDecimal` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775160` | `ts3::io::m01::WriteByteArray` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775230` | `ts3::io::m01::WriteCharUTF8` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007752C0` | `ts3::io::m01::ReadInt` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775350` | `ts3::io::m01::ReadInt64` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007753E0` | `ts3::io::m01::ReadFloat` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775470` | `ts3::io::m01::ReadDouble` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775510` | `ts3::io::m01::ReadDecimal` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007755C0` | `ts3::io::m01::ReadCharUTF8` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007756D0` | `ts3::io::m01::ReadByteArray` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x007757B0` | `ts3::io::m01::Create` | `src/core/io/m01_streams_007757b0.cpp` |
| `0x00775830` | `ts3::io::m01::CreateFromBuffer` | `src/core/io/m01_streams_00775830.cpp` |
| `0x00775910` | `ts3::io::m01::Close` | `src/core/io/m01_streams_00775910.cpp` |
| `0x00775920` | `ts3::io::m01::GetSize` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775990` | `ts3::io::m01::GetPosition` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775A00` | `ts3::io::m01::SetPosition` | `src/core/io/m01_streamhost_scalar.cpp` |
| `0x00775A80` | `ts3::io::m01::CopyStream` | `src/core/io/m01_streams_00775a80.cpp` |
| `0x00775BA0` | `ts3::io::m01::ToArray` | `src/core/io/m01_streams_00775ba0.cpp` |
| `0x00775CA0` | `ts3::io::m01::RegisterStreamBindings` | `src/core/io/m01_streams_00775ca0.cpp` |
| `0x00775CE0` | `ts3::io::m01::WriteCharArrayUTF8` | `src/core/io/m01_streams_00775ce0.cpp` |
| `0x00775E10` | `ts3::io::m01::WriteStringUTF8` | `src/core/io/m01_streams_00775e10.cpp` |
| `0x00776180` | `ts3::io::m01::ReadStringUTF8` | `src/core/io/m01_streams_00776180.cpp` |
| `0x00776420` | `ts3::io::m01::ReadCharArrayUTF8` | `src/core/io/m01_streams_00776420.cpp` |
| `0x00776650` | `ts3::io::m01::BindingTable` | `src/core/io/m01_stream_leaves.cpp` |
| `0x007EAEA0` | `ts3::resource::IndexModifiable::IsModifiable` | `src/core/include/ts3/resource/index_modifiable.h` |
| `0x0083F5F0` | `ts3::io::m01::MemoryAccess` | `src/core/io/m01_stream_leaves.cpp` |
| `0x00D1AA10` | `ts3::io::m01::MemorySize` | `src/core/io/m01_stream_leaves.cpp` |

## m02 Core runtime (threads, TLS, job manager, stopwatch, allocator helpers)

| Original address | Rewrite | File |
|---|---|---|
| `0x004E10E0` | `ts3::runtime::m02::t1::Reap` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1160` | `ts3::runtime::m02::t1::RegisterHandle` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1200` | `ts3::runtime::m02::t1::SetTls` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1270` | `ts3::runtime::m02::tls::CurrentHandle` | `src/core/runtime/m02_004e1270.cpp` |
| `0x004E1320` | `ts3::runtime::m02::t1::Sleep` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1340` | `ts3::runtime::m02::t1::InitParameters` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1380` | `ts3::runtime::m02::t1::InitCondition` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1420` | `ts3::runtime::m02::t1::ConstructCondition` | `src/core/runtime/m02_T1.cpp` |
| `0x004E14A0` | `ts3::runtime::m02::t1::DestroyCondition` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1590` | `ts3::runtime::m02::t1::Signal` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1670` | `ts3::runtime::m02::t1::DestroyMutex` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1690` | `ts3::runtime::m02::t1::InitMutex` | `src/core/runtime/m02_T1.cpp` |
| `0x004E16F0` | `ts3::thread::Mutex::Lock` | `src/core/thread/mutex.cpp` |
| `0x004E17B0` | `ts3::thread::Mutex::Unlock` | `src/core/thread/mutex.cpp` |
| `0x004E17E0` | `ts3::runtime::m02::t1::ConstructMutex` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1850` | `ts3::runtime::m02::t1::InitPoolParameters` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1880` | `ts3::runtime::m02::t1::ConstructThreads` | `src/core/runtime/m02_T1.cpp` |
| `0x004E18E0` | `ts3::runtime::m02::t1::ClearWork` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1920` | `ts3::runtime::m02::t1::ClearThreads` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1960` | `ts3::runtime::m02::PoolPredicateWait` | `src/core/runtime/m02_004e1960.cpp` |
| `0x004E1B00` | `ts3::runtime::m02::t1::ConstructWork` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1B80` | `ts3::runtime::m02::t1::AppendWork` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1C00` | `ts3::runtime::m02::PoolShutdown` | `src/core/runtime/m02_004e1c00.cpp` |
| `0x004E1CE0` | `ts3::runtime::m02::PoolReapRecord` | `src/core/runtime/m02_004e1ce0.cpp` |
| `0x004E1D80` | `ts3::runtime::m02::PoolDestroy` | `src/core/runtime/m02_004e1d80.cpp` |
| `0x004E1DF0` | `ts3::runtime::m02::PoolWorkerLoop` | `src/core/runtime/m02_004e1df0.cpp` |
| `0x004E1F30` | `ts3::runtime::m02::t1::Restart` | `src/core/runtime/m02_T1.cpp` |
| `0x004E1F90` | `ts3::runtime::m02::PoolSubmitWork` | `src/core/runtime/m02_004e1f90.cpp` |
| `0x004E2020` | `ts3::runtime::m02::t1::Submit` | `src/core/runtime/m02_T1.cpp` |
| `0x004E2080` | `ts3::runtime::m02::PoolResize` | `src/core/runtime/m02_004e2080.cpp` |
| `0x004E2210` | `ts3::runtime::m02::PoolInitialize` | `src/core/runtime/m02_004e2210.cpp` |
| `0x004E2320` | `ts3::runtime::m02::t2::PoolConstruct` | `src/core/runtime/m02_T2.cpp` |
| `0x004E23E0` | `ts3::runtime::m02::t2::RWInitialize` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2450` | `ts3::runtime::m02::RWAcquireTimed` | `src/core/runtime/m02_004e2450.cpp` |
| `0x004E2530` | `ts3::runtime::m02::RWRelease` | `src/core/runtime/m02_004e2530.cpp` |
| `0x004E25B0` | `ts3::runtime::m02::t2::RWConstruct` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2620` | `ts3::runtime::m02::t2::RWDestroy` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2640` | `ts3::runtime::m02::SemaphoreCounterUpdate` | `src/core/runtime/m02_004e2640.cpp` |
| `0x004E26A0` | `ts3::runtime::m02::t2::SemaphoreDefaults` | `src/core/runtime/m02_T2.cpp` |
| `0x004E26E0` | `ts3::runtime::m02::t2::SemaphoreDestroy` | `src/core/runtime/m02_T2.cpp` |
| `0x004E26F0` | `ts3::runtime::m02::t2::SemaphoreInitialize` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2760` | `ts3::runtime::m02::SemaphoreWaitTimed` | `src/core/runtime/m02_004e2760.cpp` |
| `0x004E28C0` | `ts3::runtime::m02::t2::SemaphoreConstruct` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2920` | `ts3::runtime::m02::t2::SemaphoreCount` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2960` | `ts3::runtime::m02::t2::ParameterDefaults` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2980` | `ts3::runtime::m02::c1::DefaultWrapper` | `src/core/runtime/m02_C1.cpp` |
| `0x004E2990` | `ts3::runtime::m02::c1::AlternateWrapper` | `src/core/runtime/m02_C1.cpp` |
| `0x004E29A0` | `ts3::runtime::m02::TimedJoin` | `src/core/runtime/m02_004e29a0.cpp` |
| `0x004E2A70` | `ts3::runtime::m02::t2::Poll` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2AE0` | `ts3::runtime::m02::c1::NativeHandle` | `src/core/runtime/m02_C1.cpp` |
| `0x004E2AF0` | `ts3::runtime::m02::t2::Priority` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2B70` | `ts3::runtime::m02::t2::DebugName` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2C10` | `ts3::runtime::m02::t2::Allocate` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2C30` | `ts3::runtime::m02::t2::Initialize` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2C50` | `ts3::runtime::m02::t2::Ideal` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2D10` | `ts3::runtime::m02::t2::QueueApc` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2D30` | `ts3::runtime::m02::ReserveThreadRecord` | `src/core/runtime/m02_004e2d30.cpp` |
| `0x004E2DA0` | `ts3::runtime::m02::RecycleThreadRecord` | `src/core/runtime/m02_004e2da0.cpp` |
| `0x004E2E10` | `ts3::runtime::m02::t2::Release` | `src/core/runtime/m02_T2.cpp` |
| `0x004E2E30` | `ts3::runtime::m02::ThreadCallbackEntry` | `src/core/runtime/m02_004e2e30.cpp` |
| `0x004E2F30` | `ts3::runtime::m02::LaunchCallback` | `src/core/runtime/m02_004e2f30.cpp` |
| `0x004E30A0` | `ts3::runtime::m02::VirtualThreadEntry` | `src/core/runtime/m02_004e30a0.cpp` |
| `0x004E31A0` | `ts3::runtime::m02::LaunchVirtualThread` | `src/core/runtime/m02_004e31a0.cpp` |
| `0x004E3310` | `ts3::runtime::m02::t2::ReleaseInPlace` | `src/core/runtime/m02_T2.cpp` |
| `0x004E3330` | `ts3::runtime::m02::t2::Delete` | `src/core/runtime/m02_T2.cpp` |
| `0x004E3360` | `ts3::runtime::m02::c1::DeleteThreadBase` | `src/core/runtime/m02_C1.cpp` |
| `0x004E3380` | `ts3::runtime::m02::s1::Restart` | `src/core/runtime/m02_S1.cpp` |
| `0x004F30E0` | `ts3::runtime::m02::s1::SetMode` | `src/core/runtime/m02_S1.cpp` |
| `0x004F3220` | `ts3::objects::StopwatchStop4F3220` | `src/core/objects/clu_4f3220_Objects.cpp` |
| `0x004F32C0` | `ts3::runtime::m02::s1::ElapsedUInt` | `src/core/runtime/m02_S1.cpp` |
| `0x004F33C0` | `ts3::runtime::m02::s1::ElapsedFloat` | `src/core/runtime/m02_S1.cpp` |
| `0x004F3480` | `ts3::runtime::m02::s1::UnitFactor` | `src/core/runtime/m02_S1.cpp` |
| `0x004F34F0` | `ts3::runtime::m02::s1::SetDeadline` | `src/core/runtime/m02_S1.cpp` |
| `0x004F35B0` | `ts3::runtime::m02::s1::Construct` | `src/core/runtime/m02_S1.cpp` |
| `0x00599160` | `ts3::runtime::m02::c1::SchedulerSingleton` | `src/core/runtime/m02_C1.cpp` |
| `0x00599170` | `ts3::runtime::m02::j1::Initialize` | `src/core/runtime/m02_J1.cpp` |
| `0x005991D0` | `ts3::runtime::m02::j1::Stamp` | `src/core/runtime/m02_J1.cpp` |
| `0x00599220` | `ts3::runtime::m02::j1::State` | `src/core/runtime/m02_J1.cpp` |
| `0x00599260` | `ts3::runtime::m02::c1::ReplaceCallback` | `src/core/runtime/m02_C1.cpp` |
| `0x00599290` | `ts3::runtime::m02::j1::SetCancel` | `src/core/runtime/m02_J1.cpp` |
| `0x005992D0` | `ts3::runtime::m02::StartJobService` | `src/core/runtime/m02_005992d0.cpp` |
| `0x00599380` | `ts3::runtime::m02::StopService` | `src/core/runtime/m02_00599380.cpp` |
| `0x005993B0` | `ts3::runtime::m02::ShutdownJobService` | `src/core/runtime/m02_005993b0.cpp` |
| `0x005993E0` | `ts3::runtime::m02::c1::ServiceSingleton` | `src/core/runtime/m02_C1.cpp` |
| `0x00599420` | `ts3::runtime::m02::j1::AllocateLink` | `src/core/runtime/m02_J1.cpp` |
| `0x00599460` | `ts3::jobmanager::RefTarget::AddRef` | `src/core/jobmanager/clu_599460_JobManager.cpp` |
| `0x00599480` | `ts3::runtime::m02::j1::RetainExternal` | `src/core/runtime/m02_J1.cpp` |
| `0x005994E0` | `ts3::runtime::m02::c1::SetSecondaryConfig` | `src/core/runtime/m02_C1.cpp` |
| `0x005994F0` | `ts3::runtime::m02::j1::Construct` | `src/core/runtime/m02_J1.cpp` |
| `0x005995A0` | `ts3::runtime::m02::j1::Destroy` | `src/core/runtime/m02_J1.cpp` |
| `0x005995D0` | `ts3::runtime::m02::DrainScheduler` | `src/core/runtime/m02_005995d0.cpp` |
| `0x00599620` | `ts3::runtime::m02::AllocateJob` | `src/core/runtime/m02_00599620.cpp` |
| `0x00599720` | `ts3::runtime::m02::CompleteJob` | `src/core/runtime/m02_00599720.cpp` |
| `0x005998B0` | `ts3::jobmanager::JobManager::CancelDepList` | `src/core/jobmanager/clu_599460_JobManager.cpp` |
| `0x00599920` | `ts3::runtime::m02::j1::Insert` | `src/core/runtime/m02_J1.cpp` |
| `0x00599970` | `ts3::runtime::m02::j1::ConstructService` | `src/core/runtime/m02_J1.cpp` |
| `0x005999C0` | `ts3::runtime::m02::DestroyJobService` | `src/core/runtime/m02_005999c0.cpp` |
| `0x00599A10` | `ts3::runtime::m02::j1::Update` | `src/core/runtime/m02_J1.cpp` |
| `0x00599A60` | `ts3::runtime::m02::RegisterJobService` | `src/core/runtime/m02_00599a60.cpp` |
| `0x00599AC0` | `ts3::runtime::m02::j1::LowerRaw` | `src/core/runtime/m02_J1.cpp` |
| `0x00599B60` | `ts3::runtime::m02::j1::LowerView` | `src/core/runtime/m02_J1.cpp` |
| `0x00599BF0` | `ts3::runtime::m02::FreeSchedulerResources` | `src/core/runtime/m02_00599bf0.cpp` |
| `0x00599CF0` | `ts3::runtime::m02::CreateWorker` | `src/core/runtime/m02_00599cf0.cpp` |
| `0x00599E00` | `ts3::runtime::m02::j1::Find` | `src/core/runtime/m02_J1.cpp` |
| `0x00599E60` | `ts3::runtime::m02::DispatchEligible` | `src/core/runtime/m02_00599e60.cpp` |
| `0x00599FC0` | `ts3::runtime::m02::RouteJob` | `src/core/runtime/m02_00599fc0.cpp` |
| `0x0059A190` | `ts3::runtime::m02::ScheduleEligibleJob` | `src/core/runtime/m02_0059a190.cpp` |
| `0x0059A220` | `ts3::runtime::m02::WaitForJob` | `src/core/runtime/m02_0059a220.cpp` |
| `0x0059A350` | `ts3::runtime::m02::j1::Invoke` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A380` | `ts3::runtime::m02::j1::Reorder` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A430` | `ts3::runtime::m02::j1::Configure` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A470` | `ts3::runtime::m02::j1::Propagate` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A500` | `ts3::runtime::m02::j1::Operate` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A540` | `ts3::runtime::m02::CancelTransition` | `src/core/runtime/m02_0059a540.cpp` |
| `0x0059A650` | `ts3::runtime::m02::Recycle` | `src/core/runtime/m02_0059a650.cpp` |
| `0x0059A6E0` | `ts3::runtime::m02::Depend` | `src/core/runtime/m02_0059a6e0.cpp` |
| `0x0059A800` | `ts3::runtime::m02::j1::ReleaseRaw` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A820` | `ts3::runtime::m02::j1::SetPriority` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A870` | `ts3::runtime::m02::j1::Relate` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A8B0` | `ts3::runtime::m02::ShutdownWorker` | `src/core/runtime/m02_0059a8b0.cpp` |
| `0x0059A990` | `ts3::runtime::m02::j1::Release` | `src/core/runtime/m02_J1.cpp` |
| `0x0059A9F0` | `ts3::jobmanager::JobManager::ProcessDeps` | `src/core/jobmanager/clu_599460_JobManager.cpp` |
| `0x006E9540` | `ts3::jobmanager::RefHolder::Assign` | `src/core/jobmanager/clu_599460_JobManager.cpp` |
| `0x007749C0` | `ts3::runtime::m02::c1::RegistrationName` | `src/core/runtime/m02_C1.cpp` |
| `0x007749D0` | `ts3::runtime::m02::CreateStopwatch` | `src/core/runtime/m02_007749d0.cpp` |
| `0x00774A40` | `ts3::runtime::m02::c1::Stop` | `src/core/runtime/m02_C1.cpp` |
| `0x00774A80` | `ts3::runtime::m02::c1::Elapsed` | `src/core/runtime/m02_C1.cpp` |
| `0x00774AC0` | `ts3::runtime::m02::s1::ManagedFloat` | `src/core/runtime/m02_S1.cpp` |
| `0x00774B80` | `ts3::runtime::m02::s1::DeleteRegistration` | `src/core/runtime/m02_S1.cpp` |
| `0x00774BB0` | `ts3::runtime::m02::c1::Start` | `src/core/runtime/m02_C1.cpp` |
| `0x00774BF0` | `ts3::runtime::m02::s1::Reset` | `src/core/runtime/m02_S1.cpp` |
| `0x00774C30` | `ts3::runtime::m02::c1::Restart` | `src/core/runtime/m02_C1.cpp` |
| `0x00774C70` | `ts3::runtime::m02::s1::IsRunning` | `src/core/runtime/m02_S1.cpp` |
| `0x00774CB0` | `ts3::runtime::m02::RegisterStopWatch` | `src/core/runtime/m02_00774cb0.cpp` |
| `0x00774D00` | `ts3::runtime::m02::s1::Destroy` | `src/core/runtime/m02_S1.cpp` |
| `0x00774D40` | `ts3::runtime::m02::c1::RegistrationTable` | `src/core/runtime/m02_C1.cpp` |

## Other verified functions (not on a module list yet)

| Original address | Rewrite | File |
|---|---|---|
| `0x00408CC0` | `ts3::world408cc0::EqualWorldString` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00408DD0` | `ts3::world408cc0::WorldOwnedString::Release` | `src/core/world/clu_408cc0_World.cpp` |
| `0x004091F0` | `ts3::world408cc0::WorldStringTable::Find` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00409320` | `ts3::world408cc0::WorldStringTable::Insert` | `src/core/world/clu_408cc0_World.cpp` |
| `0x004094D0` | `ts3::world408cc0::WorldStringTable::Get` | `src/core/world/clu_408cc0_World.cpp` |
| `0x0044C340` | `ts3::world408cc0::EnsureWorldLocale` | `src/core/world/clu_408cc0_World.cpp` |
| `0x0044C390` | `ts3::world408cc0::ResetWorldLocale` | `src/core/world/clu_408cc0_World.cpp` |
| `0x004AE820` | `ts3::scratch::ScratchByteRange::FindFrom` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4560` | `ts3::scratch::ScratchContainer::Destroy4B4560` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4880` | `ts3::scratch::ScratchListHead::DestroyNodes` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4AF0` | `ts3::scratch::ScratchContainer::Destroy4B4AF0` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4B60` | `ts3::scratch::ScratchContainer::Delete4B4B60` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4C00` | `ts3::scratch::ScratchContainer::Delete4B4C00` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4CD0` | `ts3::scratch::ScratchContainer::Reset4B4CD0` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004B4F80` | `ts3::scratch::ScratchContainer::Delete4B4F80` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x004DB910` | `ts3::io::FileStream::Construct` | `src/core/io/file_stream.cpp` |
| `0x004DB970` | `ts3::io::FileStream::Construct` | `src/core/io/file_stream.cpp` |
| `0x004DB9D0` | `ts3::io::FileStream::Destruct` | `src/core/io/file_stream.cpp` |
| `0x00523710` | `ts3::ui::MatrixRowList::Init523710` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x00523760` | `ts3::ui::MatrixRowList::InitCopy523760` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x00523800` | `ts3::ui::MatrixRowList::ScrollRows523800` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x005241D0` | `ts3::ui::MatrixRowList::EnsureRowList5241D0` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x00528F20` | `ts3::ui::MatrixRowList::HitTest528F20` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x00529050` | `ts3::ui::MatrixRowList::SetMode529050` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x005297F0` | `ts3::ui::MatrixRowList::OnKey5297F0` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x00529A20` | `ts3::ui::MatrixRowList::Update529A20` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x005319F0` | `ts3::ui::MatrixRowList::Refresh5319F0` | `src/core/ui/cls_4a4230_UI.cpp` |
| `0x0058B8A0` | `ts3::world408cc0::WorldPairTable::Rehash` | `src/core/world/clu_408cc0_World.cpp` |
| `0x0058B980` | `ts3::world408cc0::WorldRecordTable::Rehash` | `src/core/world/clu_408cc0_World.cpp` |
| `0x0058C240` | `ts3::world408cc0::WorldPairTable::Insert` | `src/core/world/clu_408cc0_World.cpp` |
| `0x0058C710` | `ts3::world408cc0::WorldRecordTable::Insert` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00594CF0` | `ts3::world408cc0::FindWorldString` | `src/core/world/clu_408cc0_World.cpp` |
| `0x005C8360` | `ts3::misc::clu4a9e90::FloatExponent` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x005DE510` | `ts3::world408cc0::PartitionWorldEntries` | `src/core/world/clu_408cc0_World.cpp` |
| `0x005E5550` | `ts3::scratch::ScratchExportDesc::Construct` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x005E5890` | `ts3::scratch::ScratchExportDesc::Destroy` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x005E5E20` | `ts3::resource::IndexKeyHash/IndexKeyEqual + eastl::hash_map::find` | `src/core/include/ts3/resource/index_modifiable.h` |
| `0x005E6A30` | `ts3::scratch::ScratchXmlRecord::InitFromKey` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x005E7260` | `ts3::scratch::ScratchXmlRecord::Destroy` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x005E8FA0` | `ts3::scratch::ScratchExportSource::Export` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x0060CF40` | `ts3::misc::clu4a9e90::TrimMips` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x0060D140` | `ts3::misc::clu4a9e90::LastMipZero` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x007410D0` | `ts3::misc::clu4a9e90::PushUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x007411B0` | `ts3::misc::clu4a9e90::AdjustUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x007413B0` | `ts3::misc::clu4a9e90::MakeUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x00741400` | `ts3::misc::clu4a9e90::SortUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x00741470` | `ts3::misc::clu4a9e90::PartitionUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x007414E0` | `ts3::misc::clu4a9e90::PartialUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x00741560` | `ts3::misc::clu4a9e90::QuickUint64` | `src/core/misc/clu_4a9e90_x.cpp` |
| `0x0085A760` | `ts3::world408cc0::WorldNameOwner::SetNames` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00876260` | `ts3::scratch::ScratchByteRange::OverwriteFrom` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x0087D770` | `ts3::scratch::ScratchContainer87D::Destroy87D770` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x0087D810` | `ts3::scratch::ScratchContainer87D::Delete87D810` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x0088EC50` | `ts3::scratch::ConcatCStringSpan` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x008BBD30` | `ts3::jobmanager::DepVector::PushBack` | `src/core/jobmanager/clu_599460_JobManager.cpp` |
| `0x0094EEA0` | `ts3::objects::ObjectsSelect94EEA0` | `src/core/objects/clu_4f3220_Objects.cpp` |
| `0x00985590` | `ts3::scratch::ScratchStringTriple::Destroy` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x0099E4B0` | `ts3::scratch::ScratchPairListHost::AppendKey` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009A0180` | `ts3::scratch::ScratchXmlRecord::InitCopy` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009A02CE` | `ts3::scratch::ExportStringTables` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009A3900` | `ts3::world408cc0::FindWorldChain` | `src/core/world/clu_408cc0_World.cpp` |
| `0x009C1100` | `ts3::scratch::ExportFirstItem` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009C65C0` | `ts3::scratch::ConcatSpanCString` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009CFA30` | `ts3::scratch::ScratchXmlTagHost::ReplaceXmlAttribute` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009CFBD0` | `ts3::scratch::XmlTagMatches` | `src/core/scratch/clu_4a9b20_Scratch.cpp` |
| `0x009F5590` | `ts3::objects::ObjectsRead9F5590` | `src/core/objects/clu_4f3220_Objects.cpp` |
| `0x00A517B0` | `ts3::world408cc0::WorldOrderA::Less` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00A8C8A0` | `ts3::world408cc0::WorldOrderB::Less` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00C93410` | `ts3::world408cc0::WorldProductName::GetName` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00CE8360` | `ts3::world408cc0::LessWorldProductUsercall / ts3::world408cc0::LessWorldProduct` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00CE8980` | `ts3::world408cc0::MergeWorldProducts` | `src/core/world/clu_408cc0_World.cpp` |
| `0x00CE9490` | `ts3::world408cc0::SortWorldProducts` | `src/core/world/clu_408cc0_World.cpp` |

## m06_audio (one function used by the code above)

| Original address | Rewrite | File |
|---|---|---|
| `0x004AAFE0` | `ts3::resource::IndexCompressed::Count` | `src/core/include/ts3/resource/package_index.h` |
