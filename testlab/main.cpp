#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <algorithm>
#include <fstream>
#include <memory>
#include <thread>
#include "runtime.h"
#include "game.h"
#include "mover.h"

namespace lab = openbarrio::lab;
namespace game = openbarrio::launcher;
namespace mover = openbarrio::mover;
namespace fs = std::filesystem;
namespace {
enum : int { Game=100, BrowseGame, Source, BrowseSource, BuildPreset, Build, OpenBuild,
    Modules, Hooks, All, None, Advanced, Mode, SavePreset, LoadPreset, SourceData, BrowseData,
    Destination, BrowseDest, Copy, UseExisting, Reset, Current, Play, Find, Crashed, NoCrash, EndBisect,
    BisectText, Sessions, OpenSession, OpenSummary, OpenCrash, Log, Status };
constexpr UINT ProgressMessage=WM_APP+1, DoneMessage=WM_APP+2;
struct Done {
    std::wstring message; bool ok=false, play=false, build=false, copy=false, trial=false;
    lab::RunResult run; lab::BuildResult built; fs::path session; std::wstring destination;
};
struct App {
    HWND window=nullptr; HINSTANCE instance=nullptr; HFONT font=nullptr;
    fs::path self, folder, source, dll, settings, live, latestBuild;
    std::vector<lab::Hook> hooks; std::vector<std::string> modules;
    lab::Selection wanted; std::vector<fs::path> sessions;
    bool syncing=false, busy=false, replace=false, advanced=true, bisectActive=false, awaiting=false;
    std::wstring redirect; lab::Bisect bisect; fs::path bisectFile, bisectDll, lastSession;
} g;
HWND Item(int id) { return GetDlgItem(g.window,id); }
std::wstring Text(int id) { const int n=GetWindowTextLengthW(Item(id)); std::wstring s(static_cast<size_t>(n)+1,L'\0'); GetWindowTextW(Item(id),s.data(),n+1); s.resize(static_cast<size_t>(n)); return s; }
void Set(int id,const std::wstring& s) { SetWindowTextW(Item(id),s.c_str()); }
void Message(const std::wstring& s) { MessageBoxW(g.window,s.c_str(),L"OpenBarrio Test Lab",MB_OK|MB_ICONINFORMATION); }
std::wstring ReadSetting(const fs::path& file,const wchar_t* section,const wchar_t* key,const wchar_t* fallback=L"") {
    wchar_t buffer[32768]; GetPrivateProfileStringW(section,key,fallback,buffer,32768,file.c_str()); return buffer;
}
void SaveSettings() {
    lab::WriteIni(g.settings,L"[lab]\r\ngame="+Text(Game)+L"\r\nsource="+Text(Source)+L"\r\nredirect="+g.redirect+
        L"\r\nbisect="+(g.bisectActive?g.bisectFile.wstring():L"")+L"\r\n");
}
fs::path Browse(bool folder,const wchar_t* title,bool save=false) {
    IFileDialog* dialog=nullptr;
    if (FAILED(CoCreateInstance(save?CLSID_FileSaveDialog:CLSID_FileOpenDialog,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&dialog)))) return {};
    DWORD options=0; dialog->GetOptions(&options); dialog->SetOptions(options|FOS_FORCEFILESYSTEM|(folder?FOS_PICKFOLDERS:0)); dialog->SetTitle(title);
    if (save) { dialog->SetDefaultExtension(L"ini"); dialog->SetFileName(L"selection.ini"); }
    fs::path result;
    if (SUCCEEDED(dialog->Show(g.window))) {
        IShellItem* item=nullptr; if (SUCCEEDED(dialog->GetResult(&item))) {
            PWSTR path=nullptr; if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH,&path))) { result=path; CoTaskMemFree(path); } item->Release();
        }
    }
    dialog->Release(); return result;
}
void Open(const fs::path& path) {
    if (!fs::exists(path)) { Message(L"This file is not available for the selected session."); return; }
    if (reinterpret_cast<INT_PTR>(ShellExecuteW(g.window,L"open",path.c_str(),nullptr,nullptr,SW_SHOWNORMAL))<=32) Message(L"Windows could not open this item.");
}
fs::path SelectedSession() {
    const auto i=SendMessageW(Item(Sessions),LB_GETCURSEL,0,0);
    return i>=0 && static_cast<size_t>(i)<g.sessions.size()?g.sessions[static_cast<size_t>(i)]:fs::path{};
}
void RefreshSessions() {
    SendMessageW(Item(Sessions),LB_RESETCONTENT,0,0); g.sessions.clear();
    if (fs::exists(g.folder/"sessions")) for (const auto& e:fs::directory_iterator(g.folder/"sessions"))
        if (e.is_directory() && fs::exists(e.path()/"harness_public.ini")) g.sessions.push_back(e.path());
    std::sort(g.sessions.rbegin(),g.sessions.rend());
    for (const auto& p:g.sessions) SendMessageW(Item(Sessions),LB_ADDSTRING,0,reinterpret_cast<LPARAM>(p.filename().c_str()));
    if (!g.sessions.empty()) SendMessageW(Item(Sessions),LB_SETCURSEL,0,0);
}
void Refresh() {
    g.syncing=true;
    const auto effective=lab::Effective(g.hooks,g.wanted,g.replace,!g.redirect.empty());
    for (size_t i=0;i<g.hooks.size();++i) ListView_SetCheckState(Item(Hooks),static_cast<int>(i),g.wanted.contains(g.hooks[i].name));
    for (size_t i=0;i<g.modules.size();++i) {
        size_t total=0,chosen=0,active=0;
        for (const auto& h:g.hooks) if (h.module==g.modules[i]) { ++total; if(g.wanted.contains(h.name))++chosen; if(effective.contains(h.name))++active; }
        const auto name=lab::Wide(g.modules[i])+L"   "+std::to_wstring(active)+L"/"+std::to_wstring(total)+L" active";
        ListView_SetItemText(Item(Modules),static_cast<int>(i),0,const_cast<wchar_t*>(name.c_str()));
        ListView_SetCheckState(Item(Modules),static_cast<int>(i),chosen==total);
    }
    Set(Current,L"Game data: "+mover::GameDataFolder(g.replace&&!g.redirect.empty()?g.redirect:game::DocumentsFolder())+
        (g.redirect.empty()?L"":g.replace?L" (redirect hook pinned on)":L" (Compare uses normal Documents; redirect is inactive)"));
    if (g.bisectActive) {
        std::wstring s=g.bisect.finished?(g.bisect.candidates.empty()?L"No single culprit reproduced. Check interactions or intermittent crashes.":L"Reproduced candidate: "+lab::Wide(*g.bisect.candidates.begin())):
            std::to_wstring(g.bisect.candidates.size())+L" suspects; next run "+std::to_wstring(g.bisect.trial.size())+L" hooks; up to "+std::to_wstring(lab::RemainingRuns(g.bisect,g.hooks))+L" runs.";
        if (g.awaiting) s+=L" Record the last run: It crashed / No crash.";
        Set(BisectText,s);
    } else Set(BisectText,L"Find the culprit: replay the same actions each run. Assumes one reproducible faulty hook; interactions may not isolate.");
    for (int id:{Game,BrowseGame,Source,BrowseSource,BuildPreset,Build,All,None,Advanced,Mode,SavePreset,LoadPreset,
                 SourceData,BrowseData,Destination,BrowseDest,Copy,UseExisting,Reset,Play,Find,EndBisect,Modules,Hooks}) EnableWindow(Item(id),!g.busy);
    if (g.bisectActive) for (int id:{All,None,Mode,LoadPreset,Copy,UseExisting,Reset,Modules,Hooks,Find,Build}) EnableWindow(Item(id),FALSE);
    if (g.awaiting || (g.bisectActive&&g.bisect.finished)) EnableWindow(Item(Play),FALSE);
    EnableWindow(Item(Crashed),!g.busy&&g.bisectActive&&g.awaiting);
    EnableWindow(Item(NoCrash),!g.busy&&g.bisectActive&&g.awaiting);
    EnableWindow(Item(EndBisect),!g.busy&&g.bisectActive);
    ShowWindow(Item(Hooks),g.advanced?SW_SHOW:SW_HIDE);
    g.syncing=false;
}
void PostProgress(HWND window,const std::wstring& text) {
    auto* message=new std::wstring(text);
    if (!PostMessageW(window,ProgressMessage,0,reinterpret_cast<LPARAM>(message))) delete message;
}
template<class F> void Work(F function) {
    g.busy=true; Refresh(); const HWND window=g.window;
    std::thread([window,function=std::move(function)]() mutable {
        auto done=std::make_unique<Done>();
        try { function(*done,[window](const std::wstring& s){PostProgress(window,s);}); }
        catch(const std::exception& e) { done->message=lab::Wide(e.what()); }
        if (PostMessageW(window,DoneMessage,0,reinterpret_cast<LPARAM>(done.get()))) done.release();
    }).detach();
}
lab::Selection ReadSelection(const fs::path& ini) {
    lab::Selection names; for(const auto& h:g.hooks) {
        const auto key=lab::Wide(h.name);
        if (GetPrivateProfileIntW(L"patches",key.c_str(),0,ini.c_str())) names.insert(h.name);
    } return names;
}
void PersistBisect() {
    lab::WriteText(g.bisectFile,lab::SaveBisect(g.bisect));
    if (!g.lastSession.empty()) lab::WriteText(g.lastSession/"bisect.txt",lab::SaveBisect(g.bisect));
    SaveSettings();
}
void ApplyBisectOutcome(bool crash) {
    lab::Observe(g.bisect,crash,g.hooks); g.awaiting=false; g.wanted=g.bisect.trial; PersistBisect(); Refresh();
}
void PopulateHooks();
void BeginBisect() {
    const auto session=SelectedSession();
    if (session.empty()) { Message(L"Select a completed crash session first."); return; }
    if (MessageBoxW(g.window,L"Start from this session's enabled hooks? Repeat the same save and actions every run. A non-crashing half only clears those hooks for this reproduction.",L"Find the culprit",MB_OKCANCEL|MB_ICONINFORMATION)!=IDOK) return;
    const auto originalDll=session/"runtime/openbarrio_test.dll";
    if (!fs::exists(originalDll)) { Message(L"This session has no DLL snapshot; choose a successfully launched run."); return; }
    auto originalHooks=lab::ParseHooks(lab::ReadText(session/"runtime/HOOKS.csv"));
    g.bisectDll=originalDll;g.hooks=std::move(originalHooks);PopulateHooks();
    const auto ini=session/"harness_public.ini";
    g.replace=ReadSetting(ini,L"harness",L"mode")==L"replace";
    g.redirect=ReadSetting(ini,L"harness",L"user_documents_dir");
    SendMessageW(Item(Mode),CB_SETCURSEL,g.replace?1:0,0);
    g.bisectFile=session/"bisect.state";
    g.bisect=lab::Bisect{}; g.bisect.candidates=lab::Effective(g.hooks,ReadSelection(ini),g.replace,!g.redirect.empty());
    if (!g.redirect.empty()) g.bisect.candidates.erase("io.SpecialDirectory");
    if (g.bisect.candidates.empty()) { Message(L"No testable hooks in this session. The redirect stays pinned while redirected saves are used."); return; }
    lab::Propose(g.bisect,g.hooks); g.bisectActive=true; g.awaiting=false; g.wanted=g.bisect.trial;
    PersistBisect(); Refresh();
}
void StartPlay() {
    if (!game::RunningGamePrograms().empty()) { Message(L"Close The Sims 3 and its launcher first."); return; }
    if (!fs::exists(g.bisectActive?g.bisectDll:g.dll)) { Message(L"Build the x86 preset first; openbarrio_test.dll is missing."); return; }
    const auto redirect=g.replace?g.redirect:L"";
    if (!redirect.empty() && (!fs::is_directory(mover::GameDataFolder(redirect)) ||
        mover::FindExistingTree(mover::GameDataFolder(redirect))==mover::ExistingTree::Ours)) { Message(L"Finish the save copy before launching with this folder."); return; }
    bool backup=false;
    if (g.replace) {
        const int answer=MessageBoxW(g.window,L"Replace mode runs experimental rewrites. A wrong rewrite can crash the game or damage a save.\n\nBack up the active user-data folder before this run?\nYes: copy and verify a backup. No: continue without a new backup. Cancel: do not launch.",L"Replace mode - save backup",MB_YESNOCANCEL|MB_ICONWARNING);
        if (answer==IDCANCEL) return; backup=answer==IDYES;
    }
    lab::Run run; run.exe=Text(Game); run.dll=g.bisectActive?g.bisectDll:g.dll; run.replace=g.replace; run.hooks=g.hooks;
    run.enabled=lab::Effective(g.hooks,g.wanted,g.replace,!redirect.empty());
    run.session=lab::NewSession(g.folder/"sessions");
    lab::WriteIni(run.session/"harness_public.ini",lab::MakeIni(g.hooks,g.wanted,g.replace,redirect));
    if (g.bisectActive) {
        lab::WriteText(run.session/"bisect_before.txt",lab::SaveBisect(g.bisect));
        lab::WriteText(g.bisectFile.parent_path()/"pending.txt",lab::Utf8(run.session.wstring()));
    }
    g.lastSession=run.session; g.live=run.session/"runtime/openbarrio_test.log";
    SaveSettings(); RefreshSessions(); const bool trial=g.bisectActive;
    const auto data=mover::GameDataFolder(redirect.empty()?game::DocumentsFolder():redirect);
    Work([run,backup,data,trial](Done& done,const lab::Notify& notify) {
        done.play=true; done.trial=trial; done.session=run.session;
        try {
            // Check before a potentially large backup as well as immediately before launch.
            if (!game::SupportedBuild(mover::Sha256File(run.exe.wstring()))) throw std::runtime_error("Only original Steam TS3W.exe 1.67.2 is supported (SHA-256 check).");
            if (backup) { fs::create_directories(run.session/"backup"); lab::CopyData(data,(run.session/"backup").wstring(),game::InstallFolder(mover::Normalize(run.exe.wstring())),notify); }
            done.run=lab::Play(run,notify); done.ok=done.run.error.empty();
            done.message=done.ok?(done.run.crashed?L"Crash recorded. Open the crash report or start Find the culprit.":L"Game exited. Session saved."):lab::Wide(done.run.error);
        } catch(const std::exception& e) { done.message=lab::Wide(e.what()); }
        if (!done.run.started) {
            lab::WriteText(run.session/"summary.txt","Launch failed: "+lab::Utf8(done.message)+"\nNo game result.\n");
            lab::WriteText(run.session/"crash_report.txt","No crash recorded: launch failed.\n");
        }
    });
}
void CopySaves() {
    const auto source=Text(SourceData), base=mover::BaseFromChosen(Text(Destination)), exe=Text(Game);
    if (!mover::LooksLikeDataFolder(source)) { Message(L"Choose a user-data folder containing Saves, Mods, or Options.ini."); return; }
    if (MessageBoxW(g.window,L"Copy all user data and verify it? The source stays unchanged. Existing destination data will never be merged. Cloud-only files may be downloaded.",L"Copy user data",MB_OKCANCEL|MB_ICONINFORMATION)!=IDOK) return;
    Work([source,base,exe](Done& done,const lab::Notify& notify){
        if (!game::SupportedBuild(mover::Sha256File(exe))) throw std::runtime_error("Select the supported game executable before copying, so its install folder can be protected");
        lab::CopyData(source,base,game::InstallFolder(mover::Normalize(exe)),notify); done.ok=true; done.copy=true; done.destination=base;
        done.message=L"Copy verified. Old folder kept. The new folder is used only in Replace mode.";
    });
}
void StartBuild() {
    const fs::path source=Text(Source); const auto index=SendMessageW(Item(BuildPreset),CB_GETCURSEL,0,0);
    g.live=g.folder/"build.log"; SaveSettings(); const auto self=g.self,log=g.live;
    Work([source,index,self,log](Done& done,const lab::Notify& notify){
        done.build=true; done.built=lab::Build(source,self,static_cast<unsigned>(index),log,notify); done.ok=done.built.ok;
        done.message=done.ok?L"Build and available tests passed. Open build folder for the new binaries.":lab::Wide(done.built.error);
    });
}
void SelectionPreset(bool save) {
    const auto path=Browse(false,save?L"Save hook selection preset":L"Load hook selection preset",save); if(path.empty()) return;
    if(save) lab::WriteIni(path,lab::MakeIni(g.hooks,g.wanted,g.replace,g.replace?g.redirect:L""));
    else { g.wanted=ReadSelection(path); g.replace=ReadSetting(path,L"harness",L"mode")==L"replace";
        g.redirect=ReadSetting(path,L"harness",L"user_documents_dir"); SendMessageW(Item(Mode),CB_SETCURSEL,g.replace?1:0,0); Refresh(); }
}
HWND Control(const wchar_t* kind,const wchar_t* label,DWORD style,int id,int x,int y,int w,int h) {
    HWND c=CreateWindowExW(kind==std::wstring(L"EDIT")?WS_EX_CLIENTEDGE:0,kind,label,WS_CHILD|WS_VISIBLE|style,x,y,w,h,g.window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),g.instance,nullptr);
    SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(g.font),TRUE); return c;
}
void Label(const wchar_t* text,int x,int y,int w) { Control(L"STATIC",text,0,0,x,y,w,20); }
void Button(const wchar_t* text,int id,int x,int y,int w=100) { Control(L"BUTTON",text,BS_PUSHBUTTON|WS_TABSTOP,id,x,y,w,26); }
void Column(HWND list,int index,const wchar_t* name,int width) {
    LVCOLUMNW c{}; c.mask=LVCF_TEXT|LVCF_WIDTH; c.pszText=const_cast<wchar_t*>(name); c.cx=width; ListView_InsertColumn(list,index,&c);
}
void PopulateHooks() {
    ListView_DeleteAllItems(Item(Hooks)); ListView_DeleteAllItems(Item(Modules)); g.modules.clear();
    for(const auto& h:g.hooks) if(std::find(g.modules.begin(),g.modules.end(),h.module)==g.modules.end())g.modules.push_back(h.module);
    std::sort(g.modules.begin(),g.modules.end()); g.syncing=true;
    for(size_t i=0;i<g.modules.size();++i) { auto text=lab::Wide(g.modules[i]); LVITEMW item{}; item.mask=LVIF_TEXT; item.iItem=static_cast<int>(i); item.pszText=text.data(); ListView_InsertItem(Item(Modules),&item); }
    for(size_t i=0;i<g.hooks.size();++i) {
        auto text=lab::Wide(g.hooks[i].name); LVITEMW item{}; item.mask=LVIF_TEXT; item.iItem=static_cast<int>(i); item.pszText=text.data(); ListView_InsertItem(Item(Hooks),&item);
        auto module=lab::Wide(g.hooks[i].module); ListView_SetItemText(Item(Hooks),static_cast<int>(i),1,module.data());
        ListView_SetItemText(Item(Hooks),static_cast<int>(i),2,const_cast<wchar_t*>(g.hooks[i].compareSafe?L"Compare + Replace":L"Replace only"));
    } g.syncing=false;
}
void Initialize() {
    wchar_t path[32768]; GetModuleFileNameW(nullptr,path,32768); g.self=path; g.folder=g.self.parent_path(); g.settings=g.folder/"testlab.ini";
    g.font=static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    Label(L"OpenBarrio Test Lab - experimental engine hooks",12,8,650);
    Label(L"Game",12,39,65); Control(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,Game,90,34,890,25); Button(L"Browse...",BrowseGame,990,34);
    Label(L"Source",12,72,65); Control(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,Source,90,67,600,25); Button(L"Browse...",BrowseSource,700,67,85);
    auto preset=Control(WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|WS_TABSTOP,BuildPreset,795,67,135,100);
    for(const auto* name:{L"x86-release",L"x64-release"})SendMessageW(preset,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(name)); SendMessageW(preset,CB_SETCURSEL,0,0);
    Button(L"Build",Build,940,67,70); Button(L"Output",OpenBuild,1020,67,70);
    Label(L"Build uses a separate out/ tree so the running window can stay open. x86 builds the tools; x64 builds the core.",90,98,1000);
    Button(L"All",All,12,125,60); Button(L"None",None,80,125,60); Button(L"Advanced",Advanced,150,125,100);
    Label(L"Mode",285,131,40); auto mode=Control(WC_COMBOBOXW,L"",CBS_DROPDOWNLIST|WS_TABSTOP,Mode,330,125,160,120);
    SendMessageW(mode,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Compare (default)")); SendMessageW(mode,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Replace")); SendMessageW(mode,CB_SETCURSEL,0,0);
    Button(L"Save selection",SavePreset,510,125,120); Button(L"Load selection",LoadPreset,640,125,120);
    Label(L"Counts show hooks active in this mode.",780,131,310);
    auto modules=Control(WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|WS_BORDER|WS_TABSTOP,Modules,12,159,265,210);
    ListView_SetExtendedListViewStyle(modules,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER); Column(modules,0,L"Module / active hooks",245);
    auto hooks=Control(WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|WS_BORDER|WS_TABSTOP,Hooks,285,159,805,210);
    ListView_SetExtendedListViewStyle(hooks,LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER); Column(hooks,0,L"Advanced: individual hooks",510); Column(hooks,1,L"Module",75); Column(hooks,2,L"Availability",190);
    Label(L"Copy from",12,383,90); Control(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,SourceData,110,378,870,25); Button(L"Choose...",BrowseData,990,378);
    Label(L"Copy to base",12,416,95); Control(L"EDIT",L"",WS_TABSTOP|ES_AUTOHSCROLL,Destination,110,411,870,25); Button(L"Choose...",BrowseDest,990,411);
    Button(L"Copy + verify",Copy,110,444,130); Button(L"Use existing folder",UseExisting,250,444,150); Button(L"Normal Documents",Reset,410,444,150);
    Label(L"Redirect requires Replace; the separate redirect DLL is never injected.",580,450,510);
    Control(L"STATIC",L"",0,Current,12,479,1080,36);
    Button(L"Play",Play,12,521,100); Button(L"Find the culprit",Find,125,521,135); Button(L"It crashed",Crashed,275,521,100); Button(L"No crash",NoCrash,385,521,100); Button(L"End search",EndBisect,495,521,110);
    Control(L"STATIC",L"",0,BisectText,12,554,1080,36);
    Label(L"Sessions",12,596,150); Label(L"Live harness / build output",285,596,800);
    Control(L"LISTBOX",L"",LBS_NOTIFY|WS_BORDER|WS_VSCROLL|WS_TABSTOP,Sessions,12,620,265,150);
    Control(L"EDIT",L"",ES_READONLY|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL,Log,285,620,805,185);
    Button(L"Folder",OpenSession,12,779,78); Button(L"Summary",OpenSummary,98,779,85); Button(L"Crash",OpenCrash,191,779,86);
    Control(L"STATIC",L"Ready.",0,Status,12,818,1080,36);
    g.source=ReadSetting(g.settings,L"lab",L"source");
    if(g.source.empty()) for(auto p=g.folder;!p.empty()&&p!=p.root_path();p=p.parent_path()) if(fs::exists(p/"CMakePresets.json")){g.source=p;break;}
    Set(Source,g.source.wstring()); g.dll=g.folder/"openbarrio_test.dll";
    g.hooks=lab::ParseHooks(lab::ReadText(g.folder/"HOOKS.csv")); for(const auto& h:g.hooks)g.wanted.insert(h.name);
    auto exe=ReadSetting(g.settings,L"lab",L"game"); if(exe.empty()){const auto candidates=game::FindGameCandidates();if(!candidates.empty())exe=candidates.front();}Set(Game,exe);
    Set(SourceData,mover::GameDataFolder(game::DocumentsFolder())); g.redirect=ReadSetting(g.settings,L"lab",L"redirect");Set(Destination,g.redirect);
    PopulateHooks(); RefreshSessions();
    const fs::path saved=ReadSetting(g.settings,L"lab",L"bisect");
    if(!saved.empty()&&fs::exists(saved)) {
        try {
            g.bisectDll=saved.parent_path()/"runtime/openbarrio_test.dll";
            if(!fs::exists(g.bisectDll))throw std::runtime_error("The search DLL snapshot is missing");
            g.hooks=lab::ParseHooks(lab::ReadText(saved.parent_path()/"runtime/HOOKS.csv"));PopulateHooks();
            g.bisect=lab::LoadBisect(lab::ReadText(saved),g.hooks);g.bisectFile=saved;g.bisectActive=true;g.wanted=g.bisect.trial;
            const auto ini=saved.parent_path()/"harness_public.ini";g.replace=ReadSetting(ini,L"harness",L"mode")==L"replace";g.redirect=ReadSetting(ini,L"harness",L"user_documents_dir");
            SendMessageW(Item(Mode),CB_SETCURSEL,g.replace?1:0,0);
            const auto pending=lab::ReadText(saved.parent_path()/"pending.txt");
            if(!pending.empty()) {g.lastSession=lab::Wide(pending);g.awaiting=true;Set(Status,L"Restored unfinished trial. Inspect its session and record the outcome before continuing.");}
        }catch(const std::exception& e){
            g.bisectActive=false;g.awaiting=false;
            g.hooks=lab::ParseHooks(lab::ReadText(g.dll.parent_path()/"HOOKS.csv"));PopulateHooks();
            Message(lab::Wide(e.what()));
        }
    }
    Refresh(); SetTimer(g.window,1,1500,nullptr);
}
void OnCommand(int id,int code) {
    if(id==OpenSession||id==OpenSummary||id==OpenCrash){auto p=SelectedSession();if(!p.empty())Open(id==OpenSession?p:p/(id==OpenSummary?"summary.txt":"crash_report.txt"));return;}
    if(id==OpenBuild){if(!g.latestBuild.empty())Open(g.latestBuild);return;}
    if(id==Sessions&&code==LBN_SELCHANGE&&!g.busy){auto p=SelectedSession();if(!p.empty())g.live=p/"openbarrio_test.log";return;}
    if(id==Sessions&&code==LBN_DBLCLK){auto p=SelectedSession();if(!p.empty())Open(p);return;}
    if(g.busy)return;
    switch(id){
    case BrowseGame:{auto p=Browse(false,L"Select original Steam TS3W.exe");if(!p.empty())Set(Game,p.wstring());break;}
    case BrowseSource:{auto p=Browse(true,L"Select OpenBarrio source folder");if(!p.empty())Set(Source,p.wstring());break;}
    case BrowseData:{auto p=Browse(true,L"Choose The Sims 3 user-data folder (contains Saves)");if(!p.empty())Set(SourceData,p.wstring());break;}
    case BrowseDest:{auto p=Browse(true,L"Choose the new base or its Electronic Arts / The Sims 3 folder");if(!p.empty())Set(Destination,mover::BaseFromChosen(p.wstring()));break;}
    case All:if(!g.bisectActive){for(const auto& h:g.hooks)g.wanted.insert(h.name);Refresh();}break;
    case None:if(!g.bisectActive){g.wanted.clear();Refresh();}break;
    case Advanced:g.advanced=!g.advanced;Refresh();break;
    case Mode:if(code==CBN_SELCHANGE&&!g.bisectActive){g.replace=SendMessageW(Item(Mode),CB_GETCURSEL,0,0)==1;Refresh();}break;
    case Copy:CopySaves();break;
    case UseExisting:{
        if (!game::SupportedBuild(mover::Sha256File(Text(Game)))) { Message(L"Select the supported game executable first so its install folder can be protected."); break; }
        const auto base=mover::BaseFromChosen(Text(Destination));const auto data=mover::GameDataFolder(base);
        if(mover::IsInside(base,game::InstallFolder(mover::Normalize(Text(Game))))||!fs::is_directory(data)||mover::FindExistingTree(data)==mover::ExistingTree::Ours){Message(L"Choose a complete existing user-data folder outside the game install, or finish copying first.");break;}
        if(!game::RunningGamePrograms().empty()){Message(L"Close the game before changing its data folder.");break;}
        if(mover::ClearReadOnly(data).failed){Message(L"Could not remove all read-only flags.");break;}
        g.redirect=base;SaveSettings();Refresh();break;}
    case Reset:g.redirect.clear();SaveSettings();Refresh();break;
    case Play:StartPlay();break;
    case Build:StartBuild();break;
    case SavePreset:SelectionPreset(true);break;
    case LoadPreset:SelectionPreset(false);break;
    case Find:BeginBisect();break;
    case Crashed:case NoCrash:if(g.bisectActive&&g.awaiting){ApplyBisectOutcome(id==Crashed);lab::WriteText(g.bisectFile.parent_path()/"pending.txt","");}break;
    case EndBisect:g.bisectActive=false;g.awaiting=false;
        g.hooks=lab::ParseHooks(lab::ReadText(g.dll.parent_path()/"HOOKS.csv"));PopulateHooks();SaveSettings();Refresh();break;
    default:break;
    }
}
LRESULT CALLBACK WindowProc(HWND window,UINT message,WPARAM wParam,LPARAM lParam){
    try {
        switch(message){
        case WM_CREATE:g.window=window;Initialize();return 0;
        case WM_COMMAND:OnCommand(LOWORD(wParam),HIWORD(wParam));return 0;
        case WM_NOTIFY:{
            const auto* header=reinterpret_cast<NMHDR*>(lParam);
            if(header->code==LVN_ITEMCHANGED&&!g.syncing&&!g.busy&&!g.bisectActive){
                const auto* item=reinterpret_cast<NMLISTVIEW*>(lParam);
                if(item->iItem<0||!((item->uOldState^item->uNewState)&LVIS_STATEIMAGEMASK))break;
                const bool checked=(item->uNewState&LVIS_STATEIMAGEMASK)==INDEXTOSTATEIMAGEMASK(2);
                if(header->idFrom==Hooks&&static_cast<size_t>(item->iItem)<g.hooks.size()){
                    const auto& name=g.hooks[static_cast<size_t>(item->iItem)].name;if(checked)g.wanted.insert(name);else g.wanted.erase(name);
                }else if(header->idFrom==Modules&&static_cast<size_t>(item->iItem)<g.modules.size()){
                    for(const auto& h:g.hooks)if(h.module==g.modules[static_cast<size_t>(item->iItem)]){if(checked)g.wanted.insert(h.name);else g.wanted.erase(h.name);}
                }Refresh();
            }return 0;}
        case ProgressMessage:{std::unique_ptr<std::wstring> text(reinterpret_cast<std::wstring*>(lParam));Set(Status,*text);return 0;}
        case DoneMessage:{
            std::unique_ptr<Done> done(reinterpret_cast<Done*>(lParam));g.busy=false;Set(Status,done->message);
            if(done->copy&&done->ok){g.redirect=done->destination;SaveSettings();}
            if(done->build){g.latestBuild=done->built.directory;
                if(done->ok&&fs::exists(g.latestBuild/"Release/openbarrio_test.dll")) {
                    g.dll=g.latestBuild/"Release/openbarrio_test.dll";
                    auto catalog=lab::ParseHooks(lab::ReadText(g.latestBuild/"Release/HOOKS.csv"));
                    if(g.bisectActive) g.bisect=lab::LoadBisect(lab::SaveBisect(g.bisect),catalog);
                    g.hooks=std::move(catalog);PopulateHooks();
                }
            }
            if(done->play){
                RefreshSessions();g.live=done->session/"openbarrio_test.log";
                if(done->trial&&g.bisectActive){
                    if(done->run.started){g.awaiting=true;if(done->run.crashed){ApplyBisectOutcome(true);lab::WriteText(g.bisectFile.parent_path()/"pending.txt","");}}
                    else lab::WriteText(g.bisectFile.parent_path()/"pending.txt","");
                }
            }
            Refresh();if(!done->ok)Message(done->message);return 0;}
        case WM_TIMER:if(!g.live.empty()){
            auto text=lab::ReadText(g.live);if(text.size()>60000)text.erase(0,text.size()-60000);
            Set(Log,lab::Wide(text));SendMessageW(Item(Log),EM_SETSEL,static_cast<WPARAM>(-1),static_cast<LPARAM>(-1));SendMessageW(Item(Log),EM_SCROLLCARET,0,0);
        }return 0;
        case WM_CLOSE:if(g.busy){Message(L"Wait for the operation to finish. While the game runs, close it normally first so the debugger can save the session.");return 0;}SaveSettings();DestroyWindow(window);return 0;
        case WM_DESTROY:KillTimer(window,1);PostQuitMessage(0);return 0;
        default:break;
        }
    }catch(const std::exception& e){Message(lab::Wide(e.what()));if(message==WM_CREATE)return -1;}
    return DefWindowProcW(window,message,wParam,lParam);
}
} // namespace
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int show){
    const HRESULT com=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);g.instance=instance;
    INITCOMMONCONTROLSEX controls{sizeof controls,ICC_LISTVIEW_CLASSES|ICC_STANDARD_CLASSES};InitCommonControlsEx(&controls);
    WNDCLASSW cls{};cls.hInstance=instance;cls.lpfnWndProc=WindowProc;cls.lpszClassName=L"OpenBarrioTestLab";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);RegisterClassW(&cls);
    RECT rect{0,0,1105,862};AdjustWindowRect(&rect,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
    HWND window=CreateWindowW(cls.lpszClassName,L"OpenBarrio Test Lab",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,rect.right-rect.left,rect.bottom-rect.top,nullptr,nullptr,instance,nullptr);
    if(!window){if(SUCCEEDED(com))CoUninitialize();return 1;}
    ShowWindow(window,show);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){if(!IsDialogMessageW(window,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
    if(SUCCEEDED(com))CoUninitialize();return static_cast<int>(msg.wParam);
}
