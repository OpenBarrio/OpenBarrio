#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
namespace fs=std::filesystem;
struct Search { DWORD pid; HWND window=nullptr; };
BOOL CALLBACK Find(HWND window,LPARAM data) {
    auto& search=*reinterpret_cast<Search*>(data);DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    wchar_t name[80];GetClassNameW(window,name,80);
    if(pid==search.pid && std::wstring(name)==L"OpenBarrioTestLab"){search.window=window;return FALSE;}return TRUE;
}
LRESULT Send(HWND window,UINT msg,WPARAM w=0,LPARAM l=0) {
    DWORD_PTR value=0;if(!SendMessageTimeoutW(window,msg,w,l,SMTO_ABORTIFHUNG,5000,&value))throw std::runtime_error("GUI did not respond");return static_cast<LRESULT>(value);
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3)return 2;
    PROCESS_INFORMATION pi{};
    try {
        const fs::path original=fs::absolute(argv[1]);
        const auto folder=fs::absolute("sessions/gui-tests")/std::to_wstring(GetTickCount64());fs::create_directories(folder);
        const auto exe=folder/original.filename();fs::copy_file(original,exe);fs::copy_file(original.parent_path()/"HOOKS.csv",folder/"HOOKS.csv");
        std::wstring command=L"\""+exe.wstring()+L"\"";STARTUPINFOW si{sizeof si};si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_HIDE;
        if(!CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,folder.c_str(),&si,&pi))throw std::runtime_error("Could not start GUI");
        Search search{pi.dwProcessId};for(unsigned i=0;i<200&&!search.window;++i){EnumWindows(Find,reinterpret_cast<LPARAM>(&search));Sleep(25);}
        if(!search.window)throw std::runtime_error("Window not created");
        if(WaitForInputIdle(pi.hProcess,10000)==WAIT_TIMEOUT)throw std::runtime_error("GUI initialization timed out");
        // Control IDs mirror main.cpp's stable command IDs; no game launch or user input simulation.
        const HWND hooks=GetDlgItem(search.window,108), modules=GetDlgItem(search.window,107), mode=GetDlgItem(search.window,112);
        const auto count=Send(hooks,LVM_GETITEMCOUNT);if(count<100 || Send(modules,LVM_GETITEMCOUNT)<2)throw std::runtime_error("Catalog not loaded");
        Send(search.window,WM_COMMAND,110); // None
        for(int i=0;i<count;++i)if((Send(hooks,LVM_GETITEMSTATE,static_cast<WPARAM>(i),LVIS_STATEIMAGEMASK)&LVIS_STATEIMAGEMASK)!=INDEXTOSTATEIMAGEMASK(1))throw std::runtime_error("None failed");
        Send(search.window,WM_COMMAND,109); // All
        for(int i=0;i<count;++i)if((Send(hooks,LVM_GETITEMSTATE,static_cast<WPARAM>(i),LVIS_STATEIMAGEMASK)&LVIS_STATEIMAGEMASK)!=INDEXTOSTATEIMAGEMASK(2))throw std::runtime_error("All failed");
        Send(mode,CB_SETCURSEL,1);Send(search.window,WM_COMMAND,MAKEWPARAM(112,CBN_SELCHANGE));
        if(Send(mode,CB_GETCURSEL)!=1)throw std::runtime_error("Mode failed");
        Send(mode,CB_SETCURSEL,0);Send(search.window,WM_COMMAND,MAKEWPARAM(112,CBN_SELCHANGE));
        if(argc==3) {
            Send(GetDlgItem(search.window,102),WM_SETTEXT,0,reinterpret_cast<LPARAM>(argv[2]));
            Send(search.window,WM_COMMAND,105); // Real Build button, including configure/build/CTest.
            const auto deadline=GetTickCount64()+900000;
            while(!IsWindowEnabled(GetDlgItem(search.window,105))&&GetTickCount64()<deadline)Sleep(200);
            wchar_t status[4096];Send(GetDlgItem(search.window,134),WM_GETTEXT,4096,reinterpret_cast<LPARAM>(status));
            if(std::wstring(status).find(L"Build and available tests passed")!=0)throw std::runtime_error("Build button failed or timed out");
            std::cout<<"Real Build button configured, built and tested successfully\n";
        }
        Send(search.window,WM_CLOSE);if(WaitForSingleObject(pi.hProcess,5000)!=WAIT_OBJECT_0)throw std::runtime_error("Window did not close");
        if(!fs::exists(folder/"testlab.ini"))throw std::runtime_error("Settings not saved");
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);std::cout<<"GUI catalog, All/None, mode selection, and clean close passed\n";return 0;
    }catch(const std::exception& e){
        if(pi.hProcess){TerminateProcess(pi.hProcess,2);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);}std::cerr<<e.what()<<'\n';return 1;
    }
}
