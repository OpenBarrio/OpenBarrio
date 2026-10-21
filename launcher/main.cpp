// OpenBarrio.exe: one window that finds The Sims 3, copies its user data to a folder of the player's choice (out of
// OneDrive, for example), clears the read-only flags behind EA's "error 16", and starts the original game so that
// it uses the new folder. Settings live in openbarrio.ini next to this program; nothing else is written outside the
// chosen folder.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl.h>

#include <atomic>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>

#include "game.h"
#include "mover.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

namespace mover = openbarrio::mover;
namespace game = openbarrio::launcher;

namespace {

enum : int {
    kGameText = 100, kFindGame, kDataText, kFixReadOnly, kNewText, kNewFolder, kChoose, kDetect, kCopy, kUseAsIs,
    kProgress, kStatus, kLaunch, kReset, kInfo
};
constexpr UINT kProgressMessage = WM_APP + 1;  // lParam: Report*, owned by the receiver
constexpr UINT kDoneMessage = WM_APP + 2;      // lParam: Report*, owned by the receiver

struct Report {
    int permille = 0;
    mover::Result result = mover::Result::Failed;
    std::wstring text;
};

struct App {
    HINSTANCE instance = nullptr;
    HWND window = nullptr;
    HFONT font = nullptr;
    HFONT bold = nullptr;
    std::wstring ini;   // openbarrio.ini next to this program
    std::wstring dll;   // openbarrio_redirect.dll next to this program
    std::wstring exe;   // the game's TS3W.exe ("" when not found)
    const wchar_t* build = nullptr;  // the supported build it is, or nullptr
    std::wstring base;    // where the game's Documents are redirected to ("" = the normal Documents folder)
    std::wstring chosen;  // the new base the player picked
    bool busy = false;
    std::atomic<bool> cancel{false};
} g;

std::wstring ProgramFolder()
{
    wchar_t path[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring folder(path, length);
    return folder.substr(0, folder.find_last_of(L'\\') + 1);
}

// ---- settings (UTF-16 INI, so any folder name round-trips) ----------------------------------------------------

std::wstring ReadSetting(const wchar_t* section, const wchar_t* key)
{
    wchar_t value[1024] = {};
    GetPrivateProfileStringW(section, key, L"", value, 1024, g.ini.c_str());
    return value;
}

void WriteSetting(const wchar_t* section, const wchar_t* key, const std::wstring& value)
{
    if (GetFileAttributesW(g.ini.c_str()) == INVALID_FILE_ATTRIBUTES) {
        HANDLE file = CreateFileW(g.ini.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            const wchar_t bom = 0xFEFF;
            DWORD written = 0;
            WriteFile(file, &bom, sizeof bom, &written, nullptr);
            CloseHandle(file);
        }
    }
    WritePrivateProfileStringW(section, key, value.c_str(), g.ini.c_str());
}

// ---- small helpers -------------------------------------------------------------------------------------------

std::wstring SizeText(uint64_t bytes)
{
    wchar_t text[32];
    if (bytes >= (1ull << 30))
        swprintf_s(text, L"%.1f GB", static_cast<double>(bytes) / (1ull << 30));
    else
        swprintf_s(text, L"%.0f MB", static_cast<double>(bytes) / (1ull << 20) + 0.5);
    return text;
}

void Message(const std::wstring& title, const std::wstring& text, PCWSTR icon = TD_INFORMATION_ICON)
{
    TaskDialog(g.window, g.instance, L"OpenBarrio", title.c_str(), text.c_str(), TDCBF_OK_BUTTON, icon, nullptr);
}

bool Confirm(const std::wstring& title, const std::wstring& text, const wchar_t* yes, PCWSTR icon = TD_WARNING_ICON)
{
    TASKDIALOG_BUTTON buttons[] = {{IDOK, yes}};
    TASKDIALOGCONFIG config{sizeof config};
    config.hwndParent = g.window;
    config.hInstance = g.instance;
    config.pszWindowTitle = L"OpenBarrio";
    config.pszMainIcon = icon;
    config.pszMainInstruction = title.c_str();
    config.pszContent = text.c_str();
    config.pButtons = buttons;
    config.cButtons = 1;
    config.dwCommonButtons = TDCBF_CANCEL_BUTTON;
    config.nDefaultButton = IDCANCEL;
    int pressed = IDCANCEL;
    TaskDialogIndirect(&config, &pressed, nullptr, nullptr);
    return pressed == IDOK;
}

std::wstring PickFolder(bool file)
{
    std::wstring result;
    IFileOpenDialog* dialog = nullptr;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
        return result;
    DWORD options = 0;
    dialog->GetOptions(&options);
    if (file) {
        const COMDLG_FILTERSPEC filter[] = {{L"The Sims 3 (TS3W.exe)", L"TS3W.exe"}};
        dialog->SetFileTypes(1, filter);
        dialog->SetTitle(L"Find TS3W.exe (in The Sims 3\\Game\\Bin)");
        dialog->SetOptions(options | FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM);
    } else {
        dialog->SetTitle(L"Choose the folder for your Sims 3 data");
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    }
    IShellItem* item = nullptr;
    if (SUCCEEDED(dialog->Show(g.window)) && SUCCEEDED(dialog->GetResult(&item))) {
        wchar_t* path = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
            result = path;
        CoTaskMemFree(path);
        item->Release();
    }
    dialog->Release();
    return result;
}

// Where the game keeps its data now: the folder OpenBarrio points it at, or the normal one in Documents.
std::wstring CurrentDataFolder()
{
    return mover::GameDataFolder(g.base.empty() ? game::DocumentsFolder() : g.base);
}

// The normal folder in Documents. Copies always come from here, whatever the game was pointed at before.
std::wstring DocumentsDataFolder()
{
    return mover::GameDataFolder(game::DocumentsFolder());
}

bool FolderExists(const std::wstring& path)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool CreateDataFolder(const std::wstring& tree)
{
    const int result = SHCreateDirectoryExW(nullptr, tree.c_str(), nullptr);
    return result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS || FolderExists(tree);
}

std::wstring RunningText(const std::vector<std::wstring>& running)
{
    std::wstring names;
    for (const std::wstring& name : running)
        names += (names.empty() ? L"" : L", ") + name;
    return L"Please close " + names + L" first, then try again.";
}

// ---- the window's contents -----------------------------------------------------------------------------------

void SetText(int id, const std::wstring& text)
{
    SetDlgItemTextW(g.window, id, text.c_str());
}

void Refresh()
{
    if (g.exe.empty())
        SetText(kGameText, L"1. The game: not found. Use \"Find the game\" to point to TS3W.exe.");
    else if (!g.build)
        SetText(kGameText, L"1. The game: " + g.exe +
                               L"\r\nThis isn't a supported original copy (only Steam 1.67.2 for now), so OpenBarrio "
                               L"won't start it.");
    else
        SetText(kGameText, L"1. The game: " + g.exe + L"\r\nOriginal copy, " + g.build + L".");

    const std::wstring documents = DocumentsDataFolder();
    const std::wstring data = CurrentDataFolder();
    std::wstring dataText = L"2. Your Sims 3 data in Documents:\r\n" + documents;
    if (!FolderExists(documents))
        dataText += L"  (not there yet)";
    else if (mover::IsInsideOneDrive(documents))
        dataText += L"\r\nIt's inside OneDrive: OneDrive can make saving fail (error 16).";
    dataText += g.base.empty() ? L"\r\nThe game uses this folder." : L"\r\nThe game uses: " + data;
    SetText(kDataText, dataText);
    SetText(kNewFolder, g.chosen);

    // What is in the chosen folder decides what can be done with it: copy into it (empty, or our unfinished copy),
    // and/or use it without copying (its own data, or a fresh start in an empty folder).
    bool canCopy = false;
    bool canUse = false;
    std::wstring detect;
    if (!g.chosen.empty()) {
        const std::wstring tree = mover::GameDataFolder(g.chosen);
        if (mover::IsInside(tree, data) && mover::IsInside(data, tree)) {
            detect = L"That's the folder the game uses now.";
        } else if (mover::IsInside(tree, documents) && mover::IsInside(documents, tree)) {
            detect = L"That's your Documents folder: \"Use the original Documents folder\" makes the game use it again.";
        } else {
            switch (mover::FindExistingTree(tree)) {
            case mover::ExistingTree::SomeoneElse:
                detect = L"Sims 3 data found in " + tree + L". \"Use this folder\" makes the game use it as it is.";
                canUse = true;
                break;
            case mover::ExistingTree::Ours:
                detect = L"An unfinished copy is in " + tree + L": \"Copy my data there\" finishes it.";
                canCopy = true;
                break;
            case mover::ExistingTree::None:
                if (mover::LooksLikeDataFolder(g.chosen)) {
                    detect = L"This looks like a Sims 3 folder, but the game needs it at ...\\Electronic Arts\\The "
                             L"Sims 3. Put it into a folder named Electronic Arts, then choose that folder.";
                } else {
                    detect = L"Empty. \"Copy my data there\" copies your data from Documents in; \"Use this folder\" "
                             L"starts fresh (the game creates new folders, without your saves).";
                    canCopy = canUse = true;
                }
                break;
            }
        }
    }
    SetText(kDetect, detect);

    EnableWindow(GetDlgItem(g.window, kFindGame), !g.busy);
    EnableWindow(GetDlgItem(g.window, kFixReadOnly), !g.busy && FolderExists(data));
    EnableWindow(GetDlgItem(g.window, kChoose), !g.busy);
    EnableWindow(GetDlgItem(g.window, kUseAsIs), !g.busy && canUse);
    EnableWindow(GetDlgItem(g.window, kCopy), g.busy || canCopy);
    SetText(kCopy, g.busy ? L"Stop copying" : L"Copy my data there");
    EnableWindow(GetDlgItem(g.window, kLaunch), !g.busy && g.build);
    EnableWindow(GetDlgItem(g.window, kReset), !g.busy && !g.base.empty());
}

void UseGame(const std::wstring& exe)
{
    g.exe = exe;
    g.build = exe.empty() ? nullptr : game::SupportedBuild(mover::Sha256File(exe));
}

// ---- actions -----------------------------------------------------------------------------------------------

void FindGame()
{
    const std::wstring exe = PickFolder(true);
    if (exe.empty())
        return;
    UseGame(exe);
    WriteSetting(L"game", L"exe", exe);
    Refresh();
}

void FixReadOnly()
{
    const std::wstring data = CurrentDataFolder();
    if (!Confirm(L"Remove the read-only flags?",
                 L"OpenBarrio removes the read-only flag from every file and folder in:\r\n" + data +
                     L"\r\n\r\nOneDrive often leaves this flag on files, and then the game can't save (error 16). "
                     L"Nothing else is changed.",
                 L"Remove the flags"))
        return;
    const mover::ReadOnlyOutcome outcome = mover::ClearReadOnly(data);
    std::wstring text = std::to_wstring(outcome.cleared) + L" read-only flags removed.";
    if (outcome.failed)
        text += L" " + std::to_wstring(outcome.failed) + L" couldn't be changed (is a file open in another program?).";
    Message(L"Done", text);
}

void Choose()
{
    const std::wstring folder = PickFolder(false);
    if (folder.empty())
        return;
    g.chosen = mover::BaseFromChosen(folder);
    Refresh();
}

void SwitchTo(const std::wstring& base)
{
    g.base = base;
    WriteSetting(L"redirect", L"documents_dir", base);
    Refresh();
}

// The checks every new folder goes through, whether data is copied there or used as it is. Shows why not.
bool CheckChosen(const std::wstring& source)
{
    const std::vector<std::wstring> running = game::RunningGamePrograms();
    if (!running.empty()) {
        Message(L"The game is running", RunningText(running), TD_WARNING_ICON);
        return false;
    }
    switch (mover::CheckDestination(g.chosen, source, g.exe.empty() ? L"" : game::InstallFolder(g.exe))) {
    case mover::DestinationCheck::Ok:
        break;
    case mover::DestinationCheck::TooLong:
        Message(L"That folder's path is too long", L"The game can't use folders whose path is longer than 258 "
                L"characters. Please choose a folder closer to the top of the drive.", TD_ERROR_ICON);
        return false;
    case mover::DestinationCheck::Missing:
    case mover::DestinationCheck::NotAbsolute:
        Message(L"That folder isn't available", g.chosen, TD_ERROR_ICON);
        return false;
    case mover::DestinationCheck::NotWritable:
        Message(L"OpenBarrio can't write to that folder", g.chosen, TD_ERROR_ICON);
        return false;
    case mover::DestinationCheck::InsideSource:
    case mover::DestinationCheck::SourceInside:
        Message(L"Please choose another folder", L"The new folder can't be inside your current Sims 3 data folder, "
                L"or the other way round.", TD_ERROR_ICON);
        return false;
    case mover::DestinationCheck::InsideGame:
        Message(L"Please choose another folder", L"Your data can't go into the game's install folder: OpenBarrio "
                L"never changes anything there.", TD_ERROR_ICON);
        return false;
    }
    return !mover::IsInsideOneDrive(g.chosen) ||
           Confirm(L"That folder is inside OneDrive too", L"Saving can still fail with error 16 there. Use it anyway?",
                   L"Use it anyway");
}

// Points the game at the chosen folder without copying anything: it uses the data already there (copied in by
// hand, for example), or starts fresh in an empty folder. Only the read-only flags in that folder are removed.
void UseAsIs(bool checked = false)
{
    const std::wstring source = DocumentsDataFolder();
    if (!checked && !CheckChosen(source))
        return;
    const std::wstring tree = mover::GameDataFolder(g.chosen);
    const mover::ExistingTree existing = mover::FindExistingTree(tree);
    if (existing == mover::ExistingTree::Ours)
        return Message(L"An unfinished copy is in that folder", L"Finish it with \"Copy my data there\", or choose "
                       L"another folder.", TD_WARNING_ICON);
    std::wstring text;
    if (existing == mover::ExistingTree::None) {
        text = L"The game will start fresh in:\r\n" + tree +
               L"\r\n\r\nIt creates new folders there when it starts. Your saves, mods and settings in Documents "
               L"aren't copied: they stay in " + source + L".";
    } else {
        text = L"The game will use the data in:\r\n" + tree +
               L"\r\n\r\nNothing is copied, and your Documents folder stays as it is. OpenBarrio removes the read-only "
               L"flags in the chosen folder (they make saving fail with error 16).";
        if (!FolderExists(tree + L"\\Saves"))
            text += L"\r\n\r\nThere's no Saves folder in it: the game will start without your saved games.";
    }
    if (!Confirm(L"Use this folder without copying?", text, L"Use this folder", TD_INFORMATION_ICON))
        return;
    std::wstring done;
    // A fresh start gets its (empty) folder now, so that a missing folder at launch still means a missing drive.
    if (existing == mover::ExistingTree::None && !CreateDataFolder(tree))
        return Message(L"OpenBarrio can't create that folder", tree, TD_ERROR_ICON);
    if (existing == mover::ExistingTree::SomeoneElse) {
        const mover::ReadOnlyOutcome flags = mover::ClearReadOnly(tree);
        done = std::to_wstring(flags.cleared) + L" read-only flags removed.";
        if (flags.failed)
            done += L" " + std::to_wstring(flags.failed) + L" couldn't be changed (is a file open in another program?).";
        done += L"\r\n\r\n";
    }
    SwitchTo(g.chosen);
    Message(L"The game now uses this folder", done + L"Start the game with \"Launch The Sims 3\" here. EA's own "
            L"launcher (Sims3Launcher) still uses the old folder.");
}

void Copy()
{
    if (g.busy) {
        g.cancel = true;
        return;
    }
    const std::wstring source = DocumentsDataFolder();
    const std::wstring dest = mover::GameDataFolder(g.chosen);
    if (!CheckChosen(source))
        return;
    if (!FolderExists(source)) {
        if (Confirm(L"There's nothing to copy yet", L"There's no Sims 3 data in " + source +
                    L" yet. The game will create its folders in " + dest + L" when it starts.",
                    L"Use the new folder", TD_INFORMATION_ICON)) {
            if (!CreateDataFolder(dest))
                return Message(L"OpenBarrio can't create that folder", dest, TD_ERROR_ICON);
            SwitchTo(g.chosen);
        }
        return;
    }
    const mover::ExistingTree existing = mover::FindExistingTree(dest);
    if (existing == mover::ExistingTree::SomeoneElse) {
        if (Confirm(L"That folder already has Sims 3 data",
                    dest + L"\r\n\r\nOpenBarrio never mixes two sets of data. You can use the data that is already "
                           L"there as it is (nothing is copied), or cancel and choose another folder.",
                    L"Use the data that is there"))
            UseAsIs(true);
        return;
    }

    SetCursor(LoadCursor(nullptr, IDC_WAIT));
    const mover::Plan plan = mover::MakePlan(source, g.chosen);
    if (plan.bytes > plan.freeBytes)
        return Message(L"Not enough free space", L"The data needs " + SizeText(plan.bytes) + L", but only " +
                       SizeText(plan.freeBytes) + L" are free there.", TD_ERROR_ICON);
    std::wstring text = L"From:\r\n" + source + L"\r\nTo:\r\n" + dest + L"\r\n\r\n" + std::to_wstring(plan.files) +
                        L" files, " + SizeText(plan.bytes) + L" (" + SizeText(plan.freeBytes) + L" free there).";
    if (existing == mover::ExistingTree::Ours)
        text += L"\r\nAn unfinished copy is already there: OpenBarrio finishes it.";
    if (plan.cloudOnly)
        text += L"\r\n" + std::to_wstring(plan.cloudOnly) +
                L" files are only stored online by OneDrive: they are downloaded during the copy.";
    if (plan.skippedLinks)
        text += L"\r\n" + std::to_wstring(plan.skippedLinks) + L" folder links are left out.";
    text += L"\r\n\r\nThis makes a copy. Your Documents folder isn't changed: it stays as a backup that you can delete "
            L"yourself later. The copy is checked, and its read-only flags are removed (they cause error 16).";
    if (!Confirm(L"Copy your Sims 3 data?", text, L"Copy"))
        return;

    g.busy = true;
    g.cancel = false;
    SendDlgItemMessageW(g.window, kProgress, PBM_SETPOS, 0, 0);
    Refresh();
    const uint64_t total = plan.bytes ? plan.bytes : 1;
    std::thread([source, dest, total, window = g.window] {
        auto report = std::make_unique<Report>();
        if (!mover::BeginCopy(dest)) {
            report->text = L"Can't write into " + dest;
            PostMessageW(window, kDoneMessage, 0, reinterpret_cast<LPARAM>(report.release()));
            return;
        }
        DWORD lastPost = 0;
        const mover::CopyOutcome copied = mover::CopyTree(source, dest, [&](const mover::Progress& p) {
            const DWORD now = GetTickCount();
            if (now - lastPost >= 100) {
                lastPost = now;
                auto update = std::make_unique<Report>();
                update->permille = static_cast<int>(p.bytesDone * 1000 / total);
                update->text = L"Copying " + p.currentFile;
                PostMessageW(window, kProgressMessage, 0, reinterpret_cast<LPARAM>(update.release()));
            }
            return !g.cancel.load();
        });
        report->result = copied.result;
        if (copied.result == mover::Result::Done) {
            const mover::VerifyOutcome verified = mover::Verify(source, dest);
            if (!verified.ok) {
                report->result = mover::Result::Failed;
                report->text = L"The copy didn't check out: " + verified.problem;
            } else {
                const mover::ReadOnlyOutcome flags = mover::ClearReadOnly(dest);
                mover::FinishCopy(dest, source);
                report->text = std::to_wstring(verified.files) + L" files copied and checked (" +
                               std::to_wstring(verified.hashed) + L" saves compared byte for byte), " +
                               std::to_wstring(flags.cleared) + L" read-only flags removed.";
            }
        } else {
            report->text = copied.error;
        }
        PostMessageW(window, kDoneMessage, 0, reinterpret_cast<LPARAM>(report.release()));
    }).detach();
}

void CopyDone(std::unique_ptr<Report> report)
{
    g.busy = false;
    const std::wstring dest = mover::GameDataFolder(g.chosen);
    if (report->result == mover::Result::Done) {
        SendDlgItemMessageW(g.window, kProgress, PBM_SETPOS, 1000, 0);
        SetText(kStatus, L"Done.");
        SwitchTo(g.chosen);
        Message(L"Your data is in the new folder", report->text + L"\r\n\r\nFrom now on, start the game with "
                L"\"Launch The Sims 3\" here. EA's own launcher (Sims3Launcher) still uses the old folder. Your old "
                L"folder is untouched: delete it yourself once you're happy.");
    } else if (report->result == mover::Result::Cancelled) {
        SetText(kStatus, L"Stopped.");
        Message(L"Copy stopped", L"Nothing was changed in your Documents folder, and the game still uses the same folder as before. The part "
                L"that was copied is in " + dest + L": choose that folder again to finish the copy.");
    } else {
        SetText(kStatus, L"The copy failed.");
        Message(L"The copy failed", report->text + L"\r\n\r\nYour Documents folder wasn't changed, and the game still "
                L"uses it.", TD_ERROR_ICON);
    }
    Refresh();
}

void Launch()
{
    if (!g.build)
        return;
    const std::vector<std::wstring> running = game::RunningGamePrograms();
    if (!running.empty())
        return Message(L"The game is already running", RunningText(running), TD_WARNING_ICON);
    if (!game::SupportedBuild(mover::Sha256File(g.exe))) {
        UseGame(g.exe);
        Refresh();
        return Message(L"The game has changed", L"TS3W.exe is no longer the supported version (was it updated?). "
                       L"OpenBarrio won't start it.", TD_ERROR_ICON);
    }
    std::wstring dll;
    if (!g.base.empty()) {
        if (!FolderExists(mover::GameDataFolder(g.base)))
            return Message(L"Your Sims 3 folder isn't there", mover::GameDataFolder(g.base) +
                           L"\r\n\r\nIf it's on a drive that's not plugged in, plug it in and try again. OpenBarrio "
                           L"won't quietly start the game with the old folder instead.", TD_ERROR_ICON);
        dll = ProgramFolder() + L"openbarrio_redirect.dll";
        if (GetFileAttributesW(dll.c_str()) == INVALID_FILE_ATTRIBUTES)
            return Message(L"openbarrio_redirect.dll is missing", L"Keep it in the same folder as OpenBarrio.exe.",
                           TD_ERROR_ICON);
        DeleteFileW((ProgramFolder() + L"openbarrio_hook.log").c_str());
    }
    const wchar_t* arguments = PathGetArgsW(GetCommandLineW());
    const std::wstring error = game::StartGame(g.exe, dll, arguments ? arguments : L"");
    if (!error.empty()) {
        std::wstring detail;
        char log[512] = {};
        HANDLE file = CreateFileW((ProgramFolder() + L"openbarrio_hook.log").c_str(), GENERIC_READ, FILE_SHARE_READ,
                                  nullptr, OPEN_EXISTING, 0, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            DWORD read = 0;
            ReadFile(file, log, sizeof log - 1, &read, nullptr);
            CloseHandle(file);
            const int length = MultiByteToWideChar(CP_UTF8, 0, log, -1, nullptr, 0);
            detail.resize(static_cast<size_t>(length));
            MultiByteToWideChar(CP_UTF8, 0, log, -1, detail.data(), length);
        }
        return Message(L"The game wasn't started", error + (detail.empty() ? L"" : L"\r\n\r\n" + detail), TD_ERROR_ICON);
    }
    DestroyWindow(g.window);
}

void Reset()
{
    if (Confirm(L"Use the original Documents folder again?",
                L"The game will use " + mover::GameDataFolder(game::DocumentsFolder()) + L" again.\r\n\r\nYour data "
                L"in " + mover::GameDataFolder(g.base) + L" stays where it is; nothing is copied back.",
                L"Use the original folder"))
        SwitchTo(L"");
}

// ---- window --------------------------------------------------------------------------------------------------

constexpr wchar_t kInfoText[] =
    L"What this does: \"Launch The Sims 3\" starts the game's own TS3W.exe and loads openbarrio_redirect.dll into it. "
    L"That DLL changes one thing in the running game: the folder where it keeps your saves, mods and settings. "
    L"Nothing in the game folder is changed, and OpenBarrio doesn't use the internet.\r\n"
    L"Limits for now: start the game from here (EA's Sims3Launcher still uses the old folder); a few script-side "
    L"folder lookups aren't redirected yet; only the Steam version 1.67.2 is supported.";

HWND Add(const wchar_t* type, const wchar_t* text, DWORD style, int id, int x, int y, int w, int h, int scale)
{
    HWND control = CreateWindowExW(0, type, text, WS_CHILD | WS_VISIBLE | style, x * scale / 96, y * scale / 96,
                                   w * scale / 96, h * scale / 96, g.window,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g.instance, nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(g.font), TRUE);
    return control;
}

void CreateControls()
{
    HDC screen = GetDC(nullptr);
    const int dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(nullptr, screen);
    NONCLIENTMETRICSW metrics{sizeof metrics};
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof metrics, &metrics, 0);
    g.font = CreateFontIndirectW(&metrics.lfMessageFont);
    metrics.lfMessageFont.lfWeight = FW_SEMIBOLD;
    g.bold = CreateFontIndirectW(&metrics.lfMessageFont);

    Add(L"STATIC", L"", SS_LEFT, kGameText, 16, 14, 440, 48, dpi);
    Add(L"BUTTON", L"Find the game...", BS_PUSHBUTTON, kFindGame, 468, 14, 140, 28, dpi);
    Add(L"STATIC", L"", SS_LEFT, kDataText, 16, 74, 440, 70, dpi);
    Add(L"BUTTON", L"Remove read-only flags", BS_PUSHBUTTON, kFixReadOnly, 468, 74, 140, 28, dpi);
    Add(L"STATIC", L"3. Choose a new folder (outside OneDrive), then copy your data there or use what is already "
        L"there:", SS_LEFT, kNewText, 16, 150, 592, 20, dpi);
    Add(L"EDIT", L"", WS_BORDER | ES_READONLY | ES_AUTOHSCROLL, kNewFolder, 16, 172, 440, 24, dpi);
    Add(L"BUTTON", L"Choose...", BS_PUSHBUTTON, kChoose, 468, 170, 140, 28, dpi);
    Add(L"STATIC", L"", SS_LEFT, kDetect, 16, 202, 592, 34, dpi);
    Add(L"BUTTON", L"Copy my data there", BS_PUSHBUTTON, kCopy, 16, 240, 180, 30, dpi);
    Add(L"BUTTON", L"Use this folder (no copy)", BS_PUSHBUTTON, kUseAsIs, 208, 240, 180, 30, dpi);
    Add(PROGRESS_CLASSW, L"", 0, kProgress, 400, 246, 208, 18, dpi);
    Add(L"STATIC", L"", SS_LEFT | SS_ENDELLIPSIS, kStatus, 16, 276, 592, 20, dpi);
    HWND launch = Add(L"BUTTON", L"Launch The Sims 3", BS_DEFPUSHBUTTON, kLaunch, 16, 308, 220, 40, dpi);
    SendMessageW(launch, WM_SETFONT, reinterpret_cast<WPARAM>(g.bold), TRUE);
    Add(L"BUTTON", L"Use the original Documents folder", BS_PUSHBUTTON, kReset, 388, 314, 220, 28, dpi);
    Add(L"STATIC", kInfoText, SS_LEFT, kInfo, 16, 364, 592, 110, dpi);
    SendDlgItemMessageW(g.window, kProgress, PBM_SETRANGE32, 0, 1000);
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        g.window = window;
        CreateControls();
        Refresh();
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case kFindGame: FindGame(); break;
        case kFixReadOnly: FixReadOnly(); break;
        case kChoose: Choose(); break;
        case kCopy: Copy(); break;
        case kUseAsIs: UseAsIs(); break;
        case kLaunch: Launch(); break;
        case kReset: Reset(); break;
        }
        return 0;
    case kProgressMessage: {
        std::unique_ptr<Report> report(reinterpret_cast<Report*>(lParam));
        SendDlgItemMessageW(window, kProgress, PBM_SETPOS, static_cast<WPARAM>(report->permille), 0);
        SetText(kStatus, report->text);
        return 0;
    }
    case kDoneMessage:
        CopyDone(std::unique_ptr<Report>(reinterpret_cast<Report*>(lParam)));
        return 0;
    case WM_CTLCOLORSTATIC:
        SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
        return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
    case WM_CLOSE:
        if (g.busy && !Confirm(L"A copy is running", L"Stop it and close OpenBarrio? The part that was copied is kept, "
                               L"and you can finish it later.", L"Stop and close"))
            return 0;
        g.cancel = true;
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    INITCOMMONCONTROLSEX controls{sizeof controls, ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);

    g.instance = instance;
    g.ini = ProgramFolder() + L"openbarrio.ini";
    g.base = ReadSetting(L"redirect", L"documents_dir");
    g.chosen = g.base;
    std::wstring exe = ReadSetting(L"game", L"exe");
    if (exe.empty() || GetFileAttributesW(exe.c_str()) == INVALID_FILE_ATTRIBUTES) {
        const std::vector<std::wstring> found = game::FindGameCandidates();
        exe = found.empty() ? L"" : found.front();
    }
    UseGame(exe);

    WNDCLASSEXW windowClass{sizeof windowClass};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    windowClass.lpszClassName = L"OpenBarrioWindow";
    RegisterClassExW(&windowClass);

    HDC screen = GetDC(nullptr);
    const int dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(nullptr, screen);
    RECT frame{0, 0, 624 * dpi / 96, 486 * dpi / 96};
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&frame, style, FALSE);
    HWND window = CreateWindowExW(0, windowClass.lpszClassName, L"OpenBarrio: move your Sims 3 data", style,
                                  CW_USEDEFAULT, CW_USEDEFAULT, frame.right - frame.left, frame.bottom - frame.top,
                                  nullptr, nullptr, instance, nullptr);
    ShowWindow(window, show);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    CoUninitialize();
    return 0;
}
