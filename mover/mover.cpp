#include "mover.h"

#include <bcrypt.h>

#include <cwchar>

#pragma comment(lib, "bcrypt.lib")

#ifndef FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000
#endif

namespace openbarrio::mover {
namespace {

bool DefaultCloudOnly(const WIN32_FIND_DATAW& found)
{
    return (found.dwFileAttributes & (FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS | FILE_ATTRIBUTE_OFFLINE)) != 0;
}
bool (*g_cloudOnly)(const WIN32_FIND_DATAW&) = &DefaultCloudOnly;

// Paths longer than MAX_PATH are common in Downloads and Mods: every file call goes through the \\?\ form.
std::wstring Long(const std::wstring& path)
{
    if (path.rfind(L"\\\\?\\", 0) == 0)
        return path;
    if (path.rfind(L"\\\\", 0) == 0)
        return L"\\\\?\\UNC\\" + path.substr(2);
    return L"\\\\?\\" + path;
}

std::wstring Join(const std::wstring& a, const std::wstring& b)
{
    if (b.empty())
        return a;
    if (a.empty())
        return b;
    return a.back() == L'\\' ? a + b : a + L'\\' + b;
}

std::wstring Lower(std::wstring s)
{
    if (!s.empty())
        CharLowerBuffW(s.data(), static_cast<DWORD>(s.size()));
    return s;
}

std::wstring ErrorText(DWORD code)
{
    wchar_t* text = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                   code, 0, reinterpret_cast<wchar_t*>(&text), 0, nullptr);
    std::wstring result = text ? text : L"";
    LocalFree(text);
    while (!result.empty() && (result.back() == L'\n' || result.back() == L'\r' || result.back() == L' '))
        result.pop_back();
    return result.empty() ? L"error " + std::to_wstring(code) : result;
}

bool IsFolderLink(const WIN32_FIND_DATAW& found)
{
    return (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
           (found.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT || found.dwReserved0 == IO_REPARSE_TAG_SYMLINK);
}

uint64_t SizeOf(const WIN32_FIND_DATAW& found)
{
    return (static_cast<uint64_t>(found.nFileSizeHigh) << 32) | found.nFileSizeLow;
}

// Visits everything under root (depth first, folders before their contents). `rel` is relative to root. The
// visitor returns false to stop the walk. Folder links are reported (as links) but not followed.
enum class Entry { File, Folder, FolderLink };
template <class Visitor>
bool Walk(const std::wstring& root, const std::wstring& rel, Visitor& visit)
{
    WIN32_FIND_DATAW found;
    HANDLE find = FindFirstFileExW(Long(Join(Join(root, rel), L"*")).c_str(), FindExInfoBasic, &found,
                                   FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
    if (find == INVALID_HANDLE_VALUE)
        return true;
    bool go = true;
    do {
        if (!std::wcscmp(found.cFileName, L".") || !std::wcscmp(found.cFileName, L".."))
            continue;
        const std::wstring child = Join(rel, found.cFileName);
        if (IsFolderLink(found)) {
            go = visit(Entry::FolderLink, child, found);
        } else if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            go = visit(Entry::Folder, child, found) && Walk(root, child, visit);
        } else {
            go = visit(Entry::File, child, found);
        }
    } while (go && FindNextFileW(find, &found));
    FindClose(find);
    return go;
}

bool CreateFolders(const std::wstring& path)
{
    const DWORD attributes = GetFileAttributesW(Long(path).c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES)
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    const size_t cut = path.find_last_of(L'\\');
    if (cut != std::wstring::npos && cut > 2 && !CreateFolders(path.substr(0, cut)))
        return false;
    return CreateDirectoryW(Long(path).c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool FileExists(const std::wstring& path)
{
    const DWORD attributes = GetFileAttributesW(Long(path).c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool FileSize(const std::wstring& path, uint64_t* size)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(Long(path).c_str(), GetFileExInfoStandard, &data) ||
        (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
    *size = (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    return true;
}

bool WriteTextFile(const std::wstring& path, const std::wstring& text)
{
    HANDLE file = CreateFileW(Long(path).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), utf8.data(), bytes, nullptr, nullptr);
    DWORD written = 0;
    const bool ok = WriteFile(file, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr) && written == utf8.size();
    CloseHandle(file);
    return ok;
}

bool StartsWithFolder(const std::wstring& rel, const wchar_t* folder)
{
    const size_t n = std::wcslen(folder);
    return rel.size() > n && rel[n] == L'\\' && _wcsnicmp(rel.c_str(), folder, n) == 0;
}

struct CopyContext {
    const ProgressFn* progress;
    Progress state;
    uint64_t fileStartBytes;
    bool cancelled;
};

DWORD CALLBACK CopyProgress(LARGE_INTEGER, LARGE_INTEGER transferred, LARGE_INTEGER, LARGE_INTEGER, DWORD, DWORD,
                            HANDLE, HANDLE, LPVOID data)
{
    auto* context = static_cast<CopyContext*>(data);
    context->state.bytesDone = context->fileStartBytes + static_cast<uint64_t>(transferred.QuadPart);
    if (*context->progress && !(*context->progress)(context->state)) {
        context->cancelled = true;
        return PROGRESS_CANCEL;
    }
    return PROGRESS_CONTINUE;
}

// CopyFileExW keeps the last-write time; the creation time is set here too.
void CopyCreationTime(const WIN32_FIND_DATAW& source, const std::wstring& dest)
{
    HANDLE file = CreateFileW(Long(dest).c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;
    SetFileTime(file, &source.ftCreationTime, nullptr, &source.ftLastWriteTime);
    CloseHandle(file);
}

}  // namespace

std::wstring GameDataFolder(const std::wstring& base)
{
    return Join(base, kGameSubfolder);
}

std::wstring BaseFromChosen(const std::wstring& chosen)
{
    std::wstring base = Normalize(chosen);
    for (const wchar_t* tail : {L"\\Electronic Arts\\The Sims 3", L"\\Electronic Arts"}) {
        const size_t n = std::wcslen(tail);
        if (base.size() >= n + 2 && _wcsicmp(base.c_str() + base.size() - n, tail) == 0) {
            base.resize(base.size() - n);
            if (base.size() == 2 && base[1] == L':')
                base += L'\\';  // the root of a drive
            break;
        }
    }
    return base;
}

bool LooksLikeDataFolder(const std::wstring& folder)
{
    const DWORD saves = GetFileAttributesW(Long(Join(folder, L"Saves")).c_str());
    const DWORD mods = GetFileAttributesW(Long(Join(folder, L"Mods")).c_str());
    return (saves != INVALID_FILE_ATTRIBUTES && (saves & FILE_ATTRIBUTE_DIRECTORY)) ||
           (mods != INVALID_FILE_ATTRIBUTES && (mods & FILE_ATTRIBUTE_DIRECTORY)) ||
           FileExists(Join(folder, L"Options.ini"));
}

std::wstring Normalize(const std::wstring& path)
{
    if (path.empty())
        return L"";
    const DWORD needed = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    if (needed == 0)
        return L"";
    std::wstring full(needed, L'\0');
    const DWORD length = GetFullPathNameW(path.c_str(), needed, full.data(), nullptr);
    if (length == 0 || length >= needed)
        return L"";
    full.resize(length);
    while (full.size() > 3 && (full.back() == L'\\' || full.back() == L'/'))
        full.pop_back();
    return full;
}

bool IsInside(const std::wstring& path, const std::wstring& folder)
{
    const std::wstring p = Lower(Normalize(path));
    std::wstring f = Lower(Normalize(folder));
    if (p.empty() || f.empty())
        return false;
    if (p == f)
        return true;
    if (f.back() != L'\\')
        f += L'\\';
    return p.rfind(f, 0) == 0;
}

DestinationCheck CheckDestination(const std::wstring& base, const std::wstring& sourceTree, const std::wstring& gameFolder)
{
    const bool absolute = (base.size() >= 3 && base[1] == L':' && (base[2] == L'\\' || base[2] == L'/')) ||
                          base.rfind(L"\\\\", 0) == 0;
    if (!absolute)
        return DestinationCheck::NotAbsolute;
    const std::wstring folder = Normalize(base);
    if (folder.empty())
        return DestinationCheck::NotAbsolute;
    if (folder.size() > kMaxBaseLength)
        return DestinationCheck::TooLong;
    const DWORD attributes = GetFileAttributesW(Long(folder).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return DestinationCheck::Missing;
    const std::wstring destTree = GameDataFolder(folder);
    if (!sourceTree.empty() && IsInside(destTree, sourceTree))
        return DestinationCheck::InsideSource;
    if (!sourceTree.empty() && IsInside(sourceTree, destTree))
        return DestinationCheck::SourceInside;
    if (!gameFolder.empty() && IsInside(folder, gameFolder))
        return DestinationCheck::InsideGame;
    const std::wstring probe = Join(folder, L"openbarrio-write-test-" + std::to_wstring(GetCurrentProcessId()) + L".tmp");
    HANDLE file = CreateFileW(Long(probe).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                              FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return DestinationCheck::NotWritable;
    CloseHandle(file);
    return DestinationCheck::Ok;
}

std::vector<std::wstring> OneDriveFolders()
{
    std::vector<std::wstring> folders;
    for (const wchar_t* name : {L"OneDrive", L"OneDriveConsumer", L"OneDriveCommercial"}) {
        wchar_t value[MAX_PATH];
        const DWORD length = GetEnvironmentVariableW(name, value, MAX_PATH);
        if (length > 0 && length < MAX_PATH)
            folders.emplace_back(value, length);
    }
    return folders;
}

bool IsInsideOneDrive(const std::wstring& path)
{
    for (const std::wstring& folder : OneDriveFolders())
        if (IsInside(path, folder))
            return true;
    return false;
}

ExistingTree FindExistingTree(const std::wstring& destTree)
{
    const DWORD attributes = GetFileAttributesW(Long(destTree).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return ExistingTree::None;
    if (FileExists(Join(destTree, kInProgressMarker)))
        return ExistingTree::Ours;
    bool empty = true;
    auto anything = [&](Entry, const std::wstring&, const WIN32_FIND_DATAW&) { empty = false; return false; };
    Walk(destTree, L"", anything);
    return empty ? ExistingTree::None : ExistingTree::SomeoneElse;
}

Plan MakePlan(const std::wstring& sourceTree, const std::wstring& destBase)
{
    Plan plan;
    auto count = [&](Entry entry, const std::wstring&, const WIN32_FIND_DATAW& found) {
        if (entry == Entry::FolderLink) {
            ++plan.skippedLinks;
        } else if (entry == Entry::File) {
            ++plan.files;
            plan.bytes += SizeOf(found);
            if (g_cloudOnly(found))
                ++plan.cloudOnly;
        }
        return true;
    };
    Walk(sourceTree, L"", count);
    ULARGE_INTEGER freeBytes;
    if (GetDiskFreeSpaceExW(Long(destBase).c_str(), &freeBytes, nullptr, nullptr))
        plan.freeBytes = freeBytes.QuadPart;
    return plan;
}

CopyOutcome CopyTree(const std::wstring& sourceTree, const std::wstring& destTree, const ProgressFn& progress)
{
    CopyOutcome outcome;
    if (!CreateFolders(destTree)) {
        outcome.error = L"Can't create " + destTree + L": " + ErrorText(GetLastError());
        return outcome;
    }
    CopyContext context{&progress, {}, 0, false};
    auto copy = [&](Entry entry, const std::wstring& rel, const WIN32_FIND_DATAW& found) {
        const std::wstring dest = Join(destTree, rel);
        if (entry == Entry::FolderLink)
            return true;
        if (entry == Entry::Folder) {
            if (CreateFolders(dest))
                return true;
            outcome.error = L"Can't create " + dest + L": " + ErrorText(GetLastError());
            return false;
        }
        const std::wstring source = Join(sourceTree, rel);
        const uint64_t size = SizeOf(found);
        context.state.currentFile = rel;
        context.fileStartBytes = context.state.bytesDone;
        if (progress && !progress(context.state)) {
            context.cancelled = true;
            return false;
        }
        uint64_t existing = 0;
        if (FileSize(dest, &existing) && existing == size && Sha256File(dest) == Sha256File(source)) {
            ++outcome.skipped;
        } else {
            // A file left by a cancelled run may carry the read-only flag; it is copied again.
            const DWORD destAttributes = GetFileAttributesW(Long(dest).c_str());
            if (destAttributes != INVALID_FILE_ATTRIBUTES && (destAttributes & FILE_ATTRIBUTE_READONLY))
                SetFileAttributesW(Long(dest).c_str(), destAttributes & ~FILE_ATTRIBUTE_READONLY);
            BOOL cancel = FALSE;
            if (!CopyFileExW(Long(source).c_str(), Long(dest).c_str(), &CopyProgress, &context, &cancel, 0)) {
                if (!context.cancelled)
                    outcome.error = L"Can't copy " + rel + L": " + ErrorText(GetLastError());
                return false;
            }
            CopyCreationTime(found, dest);
            ++outcome.copied;
        }
        ++context.state.filesDone;
        context.state.bytesDone = context.fileStartBytes + size;
        return true;
    };
    const bool finished = Walk(sourceTree, L"", copy);
    outcome.result = finished ? Result::Done : context.cancelled ? Result::Cancelled : Result::Failed;
    if (progress && finished)
        progress(context.state);
    return outcome;
}

VerifyOutcome Verify(const std::wstring& sourceTree, const std::wstring& destTree)
{
    VerifyOutcome outcome;
    auto check = [&](Entry entry, const std::wstring& rel, const WIN32_FIND_DATAW& found) {
        if (entry != Entry::File)
            return true;
        ++outcome.files;
        const std::wstring dest = Join(destTree, rel);
        uint64_t size = 0;
        if (!FileSize(dest, &size)) {
            outcome.problem = L"Missing in the new folder: " + rel;
            return false;
        }
        if (size != SizeOf(found)) {
            outcome.problem = L"Different size in the new folder: " + rel;
            return false;
        }
        if (StartsWithFolder(rel, L"Saves")) {
            ++outcome.hashed;
            const std::wstring hash = Sha256File(dest);
            if (hash.empty() || hash != Sha256File(Join(sourceTree, rel))) {
                outcome.problem = L"Different content in the new folder: " + rel;
                return false;
            }
        }
        return true;
    };
    outcome.ok = Walk(sourceTree, L"", check);
    return outcome;
}

ReadOnlyOutcome ClearReadOnly(const std::wstring& tree)
{
    ReadOnlyOutcome outcome;
    auto clear = [&](const std::wstring& path, DWORD attributes) {
        if (!(attributes & FILE_ATTRIBUTE_READONLY))
            return;
        DWORD cleared = attributes & ~FILE_ATTRIBUTE_READONLY;
        if (cleared == 0)
            cleared = FILE_ATTRIBUTE_NORMAL;
        if (SetFileAttributesW(Long(path).c_str(), cleared))
            ++outcome.cleared;
        else
            ++outcome.failed;
    };
    const DWORD rootAttributes = GetFileAttributesW(Long(tree).c_str());
    if (rootAttributes == INVALID_FILE_ATTRIBUTES)
        return outcome;
    clear(tree, rootAttributes);
    auto visit = [&](Entry entry, const std::wstring& rel, const WIN32_FIND_DATAW& found) {
        if (entry != Entry::FolderLink)
            clear(Join(tree, rel), found.dwFileAttributes);
        return true;
    };
    Walk(tree, L"", visit);
    return outcome;
}

bool BeginCopy(const std::wstring& destTree)
{
    return CreateFolders(destTree) &&
           WriteTextFile(Join(destTree, kInProgressMarker),
                         L"OpenBarrio is copying The Sims 3 data into this folder. If the copy was interrupted, run "
                         L"OpenBarrio again and choose this folder to finish it.\r\n");
}

bool FinishCopy(const std::wstring& destTree, const std::wstring& sourceTree)
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    wchar_t date[16];
    swprintf_s(date, L"%04u-%02u-%02u", now.wYear, now.wMonth, now.wDay);
    const bool noted = WriteTextFile(Join(destTree, kMovedNote),
                                     std::wstring(L"OpenBarrio copied The Sims 3 data into this folder on ") + date +
                                         L", from:\r\n" + sourceTree +
                                         L"\r\n\r\nThat folder was not changed. It is a backup now: delete it yourself "
                                         L"once you're happy with this copy.\r\n");
    return DeleteFileW(Long(Join(destTree, kInProgressMarker)).c_str()) && noted;
}

std::wstring Sha256File(const std::wstring& path)
{
    HANDLE file = CreateFileW(Long(path).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return L"";
    BCRYPT_HASH_HANDLE hash = nullptr;
    std::wstring hex;
    if (BCRYPT_SUCCESS(BCryptCreateHash(BCRYPT_SHA256_ALG_HANDLE, &hash, nullptr, 0, nullptr, 0, 0))) {
        std::vector<unsigned char> buffer(1 << 20);
        DWORD read = 0;
        bool ok = true;
        while ((ok = ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &read, nullptr) != 0) && read > 0)
            BCryptHashData(hash, buffer.data(), read, 0);
        unsigned char digest[32];
        if (ok && BCRYPT_SUCCESS(BCryptFinishHash(hash, digest, sizeof digest, 0))) {
            static const wchar_t kHex[] = L"0123456789abcdef";
            for (unsigned char byte : digest) {
                hex += kHex[byte >> 4];
                hex += kHex[byte & 15];
            }
        }
        BCryptDestroyHash(hash);
    }
    CloseHandle(file);
    return hex;
}

void SetCloudOnlyProbeForTests(bool (*probe)(const WIN32_FIND_DATAW&))
{
    g_cloudOnly = probe ? probe : &DefaultCloudOnly;
}

}  // namespace openbarrio::mover
