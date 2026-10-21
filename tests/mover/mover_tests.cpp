// Unit tests for the save mover on fixture folders under %TEMP%. No game needed. Run: mover_tests.exe (or ctest).
#include "mover.h"

#include <cstdio>
#include <cwchar>
#include <string>

namespace mover = openbarrio::mover;

namespace {

int g_failures = 0;
#define CHECK(condition)                                                                   \
    do {                                                                                   \
        if (!(condition)) {                                                                \
            ++g_failures;                                                                  \
            std::printf("  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition);          \
        }                                                                                  \
    } while (0)

std::wstring Long(const std::wstring& path)
{
    return L"\\\\?\\" + path;
}

void RemoveTree(const std::wstring& path)
{
    WIN32_FIND_DATAW found;
    HANDLE find = FindFirstFileW(Long(path + L"\\*").c_str(), &found);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            if (!std::wcscmp(found.cFileName, L".") || !std::wcscmp(found.cFileName, L".."))
                continue;
            const std::wstring child = path + L"\\" + found.cFileName;
            SetFileAttributesW(Long(child).c_str(), FILE_ATTRIBUTE_NORMAL);
            if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                RemoveTree(child);
            else
                DeleteFileW(Long(child).c_str());
        } while (FindNextFileW(find, &found));
        FindClose(find);
    }
    SetFileAttributesW(Long(path).c_str(), FILE_ATTRIBUTE_NORMAL);
    RemoveDirectoryW(Long(path).c_str());
}

// A fresh folder under %TEMP% for one test, removed afterwards.
struct TempFolder {
    std::wstring path;
    TempFolder()
    {
        static int counter = 0;
        wchar_t temp[MAX_PATH];
        GetTempPathW(MAX_PATH, temp);
        path = mover::Normalize(std::wstring(temp) + L"openbarrio-tests-" + std::to_wstring(GetCurrentProcessId()) +
                                L"-" + std::to_wstring(++counter));
        RemoveTree(path);
        CreateDirectoryW(path.c_str(), nullptr);
    }
    ~TempFolder() { RemoveTree(path); }
};

void MakeFolders(const std::wstring& path)
{
    const size_t cut = path.find_last_of(L'\\');
    if (cut != std::wstring::npos && cut > 2)
        MakeFolders(path.substr(0, cut));
    CreateDirectoryW(Long(path).c_str(), nullptr);
}

void WriteFile(const std::wstring& path, const std::string& content)
{
    MakeFolders(path.substr(0, path.find_last_of(L'\\')));
    HANDLE file = CreateFileW(Long(path).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD written = 0;
    ::WriteFile(file, content.data(), static_cast<DWORD>(content.size()), &written, nullptr);
    CloseHandle(file);
}

FILETIME LastWrite(const std::wstring& path)
{
    WIN32_FILE_ATTRIBUTE_DATA data{};
    GetFileAttributesExW(Long(path).c_str(), GetFileExInfoStandard, &data);
    return data.ftLastWriteTime;
}

bool IsReadOnly(const std::wstring& path)
{
    const DWORD attributes = GetFileAttributesW(Long(path).c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY);
}

// A small Sims 3 data tree: saves, mods, settings.
std::wstring MakeSource(const std::wstring& root)
{
    const std::wstring tree = mover::GameDataFolder(root + L"\\Documents");
    WriteFile(tree + L"\\Saves\\Sunset Valley.sims3\\Meta.data", std::string(5000, 'm'));
    WriteFile(tree + L"\\Saves\\Sunset Valley.sims3\\SunsetValley_0x0859db4c.nhd", std::string(70000, 's'));
    WriteFile(tree + L"\\Mods\\Packages\\some_mod.package", std::string(30000, 'p'));
    WriteFile(tree + L"\\Options.ini", "[options]\r\n");
    return tree;
}

void TestCopyVerifyAndSourceUntouched()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring save = source + L"\\Saves\\Sunset Valley.sims3\\Meta.data";
    const FILETIME before = LastWrite(save);
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");

    const mover::CopyOutcome copied = mover::CopyTree(source, dest, nullptr);
    CHECK(copied.result == mover::Result::Done);
    CHECK(copied.copied == 4 && copied.skipped == 0);
    const mover::VerifyOutcome verified = mover::Verify(source, dest);
    CHECK(verified.ok);
    CHECK(verified.files == 4 && verified.hashed == 2);
    const FILETIME copiedTime = LastWrite(dest + L"\\Saves\\Sunset Valley.sims3\\Meta.data");
    const FILETIME sourceTime = LastWrite(save);
    CHECK(CompareFileTime(&before, &copiedTime) == 0);
    CHECK(CompareFileTime(&before, &sourceTime) == 0);  // the source is only read
}

void TestResumeAfterCancel()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");
    const mover::CopyOutcome first =
        mover::CopyTree(source, dest, [](const mover::Progress& p) { return p.filesDone < 2; });
    CHECK(first.result == mover::Result::Cancelled);
    CHECK(first.copied == 2);
    const mover::CopyOutcome second = mover::CopyTree(source, dest, nullptr);
    CHECK(second.result == mover::Result::Done);
    CHECK(second.skipped == 2 && second.copied == 2);
    CHECK(mover::Verify(source, dest).ok);
}

void TestResumeRecopiesDamagedFile()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");
    CHECK(mover::CopyTree(source, dest, nullptr).result == mover::Result::Done);
    // Same size, other content (like a file cut short by a crash after it was preallocated).
    WriteFile(dest + L"\\Mods\\Packages\\some_mod.package", std::string(30000, 'x'));
    const mover::CopyOutcome again = mover::CopyTree(source, dest, nullptr);
    CHECK(again.copied == 1 && again.skipped == 3);
}

void TestVerifyCatchesDifferentSave()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");
    mover::CopyTree(source, dest, nullptr);
    WriteFile(dest + L"\\Saves\\Sunset Valley.sims3\\Meta.data", std::string(5000, 'X'));
    CHECK(!mover::Verify(source, dest).ok);
    DeleteFileW(Long(dest + L"\\Options.ini").c_str());
    CHECK(!mover::Verify(source, dest).ok);
}

void TestReadOnlyCleared()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring flagged = source + L"\\Saves\\Sunset Valley.sims3\\Meta.data";
    SetFileAttributesW(Long(flagged).c_str(), FILE_ATTRIBUTE_READONLY);
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");
    mover::CopyTree(source, dest, nullptr);
    const std::wstring copy = dest + L"\\Saves\\Sunset Valley.sims3\\Meta.data";
    CHECK(IsReadOnly(copy));  // the copy keeps the flag ...
    const mover::ReadOnlyOutcome cleared = mover::ClearReadOnly(dest);
    CHECK(cleared.cleared == 1 && cleared.failed == 0);
    CHECK(!IsReadOnly(copy));  // ... until it is cleared
    CHECK(IsReadOnly(flagged));  // and the source keeps its own
    // A read-only copy left by an earlier run is replaced when the source file differs.
    SetFileAttributesW(Long(copy).c_str(), FILE_ATTRIBUTE_READONLY);
    SetFileAttributesW(Long(flagged).c_str(), FILE_ATTRIBUTE_NORMAL);
    WriteFile(flagged, std::string(5000, 'n'));
    const mover::CopyOutcome again = mover::CopyTree(source, dest, nullptr);
    CHECK(again.result == mover::Result::Done && again.copied == 1);
    CHECK(mover::Sha256File(copy) == mover::Sha256File(flagged));
}

void TestPathLengthLimit()
{
    TempFolder temp;
    for (size_t length : {size_t{258}, size_t{259}, size_t{260}}) {
        std::wstring base = temp.path + L"\\";
        while (base.size() < length)
            base += base.size() % 50 == 49 ? L'\\' : L'a';
        if (base.back() == L'\\')
            base.back() = L'b';
        MakeFolders(base);
        const mover::DestinationCheck check = mover::CheckDestination(base, L"", L"");
        if (length <= mover::kMaxBaseLength)
            CHECK(check == mover::DestinationCheck::Ok);
        else
            CHECK(check == mover::DestinationCheck::TooLong);
    }
}

void TestDestinationRules()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    const std::wstring documents = temp.path + L"\\Documents";
    const std::wstring gameFolder = temp.path + L"\\Steam\\The Sims 3";
    MakeFolders(gameFolder + L"\\Game\\Bin");
    MakeFolders(source + L"\\Saves\\Inside");
    MakeFolders(temp.path + L"\\Fine");
    CHECK(mover::CheckDestination(documents, source, gameFolder) == mover::DestinationCheck::InsideSource);
    CHECK(mover::CheckDestination(source + L"\\Saves\\Inside", source, gameFolder) ==
          mover::DestinationCheck::InsideSource);
    CHECK(mover::CheckDestination(gameFolder + L"\\Game", source, gameFolder) == mover::DestinationCheck::InsideGame);
    CHECK(mover::CheckDestination(temp.path + L"\\Missing", source, gameFolder) == mover::DestinationCheck::Missing);
    CHECK(mover::CheckDestination(L"relative\\folder", source, gameFolder) == mover::DestinationCheck::NotAbsolute);
    CHECK(mover::CheckDestination(temp.path + L"\\Fine", source, gameFolder) == mover::DestinationCheck::Ok);

    const std::wstring oneDrive = temp.path + L"\\OneDrive - Someone";
    MakeFolders(oneDrive + L"\\Documents");
    wchar_t saved[MAX_PATH] = {};
    const bool had = GetEnvironmentVariableW(L"OneDrive", saved, MAX_PATH) > 0;
    SetEnvironmentVariableW(L"OneDrive", oneDrive.c_str());
    CHECK(mover::IsInsideOneDrive(oneDrive + L"\\Documents"));
    CHECK(!mover::IsInsideOneDrive(temp.path + L"\\Fine"));
    CHECK(!mover::IsInsideOneDrive(temp.path + L"\\OneDrive"));  // a prefix of the name isn't inside
    SetEnvironmentVariableW(L"OneDrive", had ? saved : nullptr);
}

void TestExistingTree()
{
    TempFolder temp;
    const std::wstring dest = mover::GameDataFolder(temp.path + L"\\New");
    CHECK(mover::FindExistingTree(dest) == mover::ExistingTree::None);
    MakeFolders(dest);
    CHECK(mover::FindExistingTree(dest) == mover::ExistingTree::None);
    WriteFile(dest + L"\\Saves\\Old.sims3\\Meta.data", "old");
    CHECK(mover::FindExistingTree(dest) == mover::ExistingTree::SomeoneElse);
    CHECK(mover::BeginCopy(dest));
    CHECK(mover::FindExistingTree(dest) == mover::ExistingTree::Ours);
    CHECK(mover::FinishCopy(dest, L"C:\\Old"));
    CHECK(mover::FindExistingTree(dest) == mover::ExistingTree::SomeoneElse);
}

void TestChosenFolderDetection()
{
    TempFolder temp;
    const std::wstring base = temp.path + L"\\Games";
    const std::wstring tree = mover::GameDataFolder(base);
    // The base, its Electronic Arts folder and its The Sims 3 folder all give the same base.
    CHECK(mover::BaseFromChosen(base) == base);
    CHECK(mover::BaseFromChosen(base + L"\\Electronic Arts") == base);
    CHECK(mover::BaseFromChosen(tree) == base);
    CHECK(mover::BaseFromChosen(base + L"\\ELECTRONIC ARTS\\the sims 3\\") == base);
    CHECK(mover::BaseFromChosen(L"D:\\Electronic Arts\\The Sims 3") == L"D:\\");
    CHECK(mover::GameDataFolder(L"D:\\") == L"D:\\Electronic Arts\\The Sims 3");
    // A "The Sims 3" folder pasted without its Electronic Arts parent stays where it is, and is recognised.
    const std::wstring pasted = temp.path + L"\\Backup\\The Sims 3";
    CHECK(mover::BaseFromChosen(pasted) == pasted);
    CHECK(!mover::LooksLikeDataFolder(pasted));
    MakeFolders(pasted + L"\\Saves");
    CHECK(mover::LooksLikeDataFolder(pasted));
    CHECK(!mover::LooksLikeDataFolder(temp.path + L"\\Backup"));
}

bool NameSaysCloud(const WIN32_FIND_DATAW& found)
{
    return std::wcsstr(found.cFileName, L"cloud") != nullptr;
}

void TestCloudOnlyCounted()
{
    TempFolder temp;
    const std::wstring source = MakeSource(temp.path);
    WriteFile(source + L"\\Downloads\\cloud_item.sims3pack", std::string(1000, 'c'));
    mover::SetCloudOnlyProbeForTests(&NameSaysCloud);
    const mover::Plan plan = mover::MakePlan(source, temp.path);
    mover::SetCloudOnlyProbeForTests(nullptr);
    CHECK(plan.files == 5);
    CHECK(plan.cloudOnly == 1);
    CHECK(plan.bytes == 5000 + 70000 + 30000 + 11 + 1000);
    CHECK(plan.freeBytes > 0);
}

}  // namespace

int main()
{
    struct Test {
        const char* name;
        void (*run)();
    } tests[] = {
        {"copy, verify, source untouched", TestCopyVerifyAndSourceUntouched},
        {"resume after cancel", TestResumeAfterCancel},
        {"resume copies a damaged file again", TestResumeRecopiesDamagedFile},
        {"verify catches a different save", TestVerifyCatchesDifferentSave},
        {"read-only flags cleared", TestReadOnlyCleared},
        {"path length 258/259/260", TestPathLengthLimit},
        {"destination rules and OneDrive", TestDestinationRules},
        {"existing tree is never merged", TestExistingTree},
        {"chosen folder detection", TestChosenFolderDetection},
        {"cloud-only files counted", TestCloudOnlyCounted},
    };
    for (const Test& test : tests) {
        const int before = g_failures;
        test.run();
        std::printf("%s %s\n", g_failures == before ? "ok    " : "FAILED", test.name);
    }
    std::printf("%d failure(s)\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}
