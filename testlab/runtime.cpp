#include "runtime.h"
#include "game.h"
#include "mover.h"
#include <dbghelp.h>
#include <psapi.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace openbarrio::lab {
namespace {
struct Handle {
    HANDLE value = nullptr;
    explicit Handle(HANDLE v = nullptr) : value(v) {}
    ~Handle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
    Handle(const Handle&) = delete; Handle& operator=(const Handle&) = delete;
};
std::string Error(const char* what) { return std::string(what) + " (Windows error " + std::to_string(GetLastError()) + ")"; }
std::string Hex(uintptr_t n) { std::ostringstream o; o << "0x" << std::hex << std::uppercase << n; return o.str(); }
std::string FileName(HANDLE file) {
    wchar_t path[32768]; const DWORD n = GetFinalPathNameByHandleW(file, path, 32768, 0);
    return n && n < 32768 ? Utf8(fs::path(std::wstring(path,n)).filename().wstring()) : "unknown";
}
struct Module { uintptr_t base = 0; DWORD size = 0; std::string name; };
Module ReadModule(HANDLE process, uintptr_t base, HANDLE file) {
    // Loader lists may not yet exist at CREATE_PROCESS/LOAD_DLL events. Read PE headers
    // directly instead of relying on PSAPI's loader-list enumeration during startup.
    IMAGE_DOS_HEADER dos{}; IMAGE_NT_HEADERS32 nt{}; SIZE_T read = 0; DWORD size = 0;
    if (ReadProcessMemory(process,reinterpret_cast<void*>(base),&dos,sizeof dos,&read) &&
        dos.e_magic == IMAGE_DOS_SIGNATURE && dos.e_lfanew > 0 && dos.e_lfanew < 1024*1024 &&
        ReadProcessMemory(process,reinterpret_cast<void*>(base+static_cast<uintptr_t>(dos.e_lfanew)),&nt,sizeof nt,&read) &&
        nt.Signature == IMAGE_NT_SIGNATURE) size=nt.OptionalHeader.SizeOfImage;
    std::string name=file && file != INVALID_HANDLE_VALUE ? FileName(file) : "unknown";
    if (name=="unknown") {
        wchar_t mapped[32768]; const DWORD n=GetMappedFileNameW(process,reinterpret_cast<void*>(base),mapped,32768);
        if(n && n<32768) name=Utf8(fs::path(std::wstring(mapped,n)).filename().wstring());
    }
    return {base,size,name};
}
std::vector<MapSymbol> ReadMap(const fs::path& path) {
    auto symbols=ParseMap(ReadText(path));
    for(auto& symbol:symbols) {
        char clean[4096];
        if(UnDecorateSymbolName(symbol.name.c_str(),clean,sizeof clean,UNDNAME_COMPLETE))symbol.name=clean;
    }
    return symbols;
}
void CrashReport(const Run& run, HANDLE process, DWORD pid, const DEBUG_EVENT& event, const std::vector<Module>& modules) {
    EXCEPTION_RECORD record = event.u.Exception.ExceptionRecord;
    const uintptr_t address = reinterpret_cast<uintptr_t>(record.ExceptionAddress);
    std::ostringstream report;
    report << "Unhandled second-chance exception: " << Hex(record.ExceptionCode) << "\nAddress: " << Hex(address)
           << "\nThread: " << event.dwThreadId << "\n";
    std::string moduleName = "unknown"; uintptr_t rva = 0;
    for (const auto& m : modules) if (address >= m.base && address - m.base < m.size) {
        moduleName = m.name; rva = address - m.base; break;
    }
    if (moduleName=="unknown") {
        MEMORY_BASIC_INFORMATION memory{};
        if(VirtualQueryEx(process,reinterpret_cast<void*>(address),&memory,sizeof memory)) {
            const auto mapped=ReadModule(process,reinterpret_cast<uintptr_t>(memory.AllocationBase),nullptr);
            moduleName=mapped.name; rva=address-mapped.base;
        }
    }
    report << "Module: " << moduleName << "\nModule offset: " << Hex(rva) << "\n";
    if (_stricmp(moduleName.c_str(),"openbarrio_test.dll") == 0) {
        const auto symbols = ReadMap(run.session / "runtime/openbarrio_test.map");
        const MapSymbol* nearest = nullptr;
        for (const auto& s : symbols) { if (s.address > rva) break; nearest = &s; }
        if (nearest) {
            report << "Nearest rewritten symbol: " << nearest->name << " + " << Hex(rva-nearest->address)
                   << "\nSymbol boundaries are approximate; folded functions can share an address.\n";
            bool found = false;
            for (const auto& h : run.hooks) if (run.enabled.contains(h.name) && !h.function.empty() && nearest->name.find(h.function) != std::string::npos) {
                report << "Matching hook: " << h.name << " (" << h.function << ")\n"; found = true;
            }
            if (!found) report << "No exact hook match; this may be shared support code or an adapter.\n";
        } else report << "No matching local MAP symbol.\n";
        // Proximity is evidence only, never silently treated as the culprit.
        std::vector<std::pair<uintptr_t,std::string>> suspects;
        for (const auto& h : run.hooks) if (run.enabled.contains(h.name)) {
            for (const auto& s : symbols) if (!h.function.empty() && s.name.find(h.function) != std::string::npos) {
                suspects.emplace_back(s.address > rva ? s.address-rva : rva-s.address, h.name); break;
            }
        }
        std::sort(suspects.begin(),suspects.end());
        for (size_t i=0;i<std::min<size_t>(5,suspects.size());++i) report << "Nearby rewrite suspect: " << suspects[i].second << " (distance " << Hex(suspects[i].first) << ")\n";
    } else if (_stricmp(moduleName.c_str(),"TS3W.exe") == 0) {
        for (const auto& h : NearestHooks(run.hooks,run.enabled,static_cast<uint32_t>(address)))
            report << "Nearest enabled original hook below address: " << h.name << " / " << h.function << " + " << Hex(address-h.address) << '\n';
    } else report << "No direct rewritten-hook attribution. Use the enabled set for bisection.\n";
    report << "\nSuspects are not proof: memory corruption can surface far from its cause.\n";
    Handle thread(OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,FALSE,event.dwThreadId));
    CONTEXT context{}; context.ContextFlags = CONTEXT_ALL;
    const bool gotContext = thread.value && GetThreadContext(thread.value,&context);
    EXCEPTION_POINTERS pointers{&record,&context};
    MINIDUMP_EXCEPTION_INFORMATION ex{event.dwThreadId,&pointers,FALSE};
    Handle file(CreateFileW((run.session / "crash.dmp").c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
    const BOOL dumped = file.value != INVALID_HANDLE_VALUE && MiniDumpWriteDump(process,pid,file.value,MiniDumpNormal,gotContext ? &ex : nullptr,nullptr,nullptr);
    report << "Dump: " << (dumped ? "crash.dmp" : Error("FAILED to write dump")) << '\n';
    if (!gotContext) report << "Thread context unavailable; dump has no exception context.\n";
    WriteText(run.session / "crash_report.txt",report.str());
}
RunResult DebugRun(const Run& run, const std::wstring& argument, bool inject, const Notify& notify) {
    RunResult result; const auto begin = std::chrono::steady_clock::now();
    std::wstring command = Quote(run.exe.wstring()); if (!argument.empty()) command += L" " + Quote(argument);
    STARTUPINFOW startup{sizeof startup}; PROCESS_INFORMATION pi{};
    if (!CreateProcessW(run.exe.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_SUSPENDED,nullptr,
                        run.exe.parent_path().c_str(),&startup,&pi)) { result.error = Error("Start failed"); return result; }
    Handle process(pi.hProcess), mainThread(pi.hThread);
    auto abort = [&](const std::string& why) { TerminateProcess(process.value,1); WaitForSingleObject(process.value,5000); result.error=why; return result; };
    if (inject) {
        const auto dll = (run.session / "runtime/openbarrio_test.dll").wstring();
        const SIZE_T bytes = (dll.size()+1)*sizeof(wchar_t);
        void* memory = VirtualAllocEx(process.value,nullptr,bytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        if (!memory || !WriteProcessMemory(process.value,memory,dll.c_str(),bytes,nullptr)) return abort(Error("DLL injection allocation failed"));
        const auto load = reinterpret_cast<LPTHREAD_START_ROUTINE>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW")));
        Handle remote(CreateRemoteThread(process.value,nullptr,0,load,memory,0,nullptr));
        DWORD loaded=0;
        if (!remote.value || WaitForSingleObject(remote.value,60000) != WAIT_OBJECT_0 || !GetExitCodeThread(remote.value,&loaded) || !loaded)
            return abort(Error("Test DLL did not load; game stopped before start"));
        VirtualFreeEx(process.value,memory,0,MEM_RELEASE);
    }
    if (!DebugActiveProcess(pi.dwProcessId)) return abort(Error("Debugger attach failed; game stopped before start"));
    DebugSetProcessKillOnExit(FALSE);
    if (ResumeThread(mainThread.value) == static_cast<DWORD>(-1)) { DebugActiveProcessStop(pi.dwProcessId); return abort(Error("Resume failed")); }
    result.started=true; notify(L"Game running under debugger. Only second-chance exceptions count as crashes.");
    bool initialBreakpoint = true, done = false; std::vector<Module> modules;
    while (!done) {
        DEBUG_EVENT e{};
        if (!WaitForDebugEvent(&e,250)) {
            if (GetLastError()==ERROR_SEM_TIMEOUT) continue;
            result.error=Error("Debugger event wait failed"); DebugActiveProcessStop(pi.dwProcessId);
            // Keep the game alive if observation fails; report an unknown outcome, not a fabricated crash.
            WaitForSingleObject(process.value,INFINITE); DWORD code=0; GetExitCodeProcess(process.value,&code); result.exitCode=code; break;
        }
        DWORD status=DBG_CONTINUE;
        if (e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT) {
            modules.push_back(ReadModule(process.value,reinterpret_cast<uintptr_t>(e.u.CreateProcessInfo.lpBaseOfImage),e.u.CreateProcessInfo.hFile));
            if (e.u.CreateProcessInfo.hFile) CloseHandle(e.u.CreateProcessInfo.hFile);
            // Windows closes the debug-event process/thread handles after the exit event is continued.
        } else if (e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {
            auto m=ReadModule(process.value,reinterpret_cast<uintptr_t>(e.u.LoadDll.lpBaseOfDll),e.u.LoadDll.hFile);
            if (e.u.LoadDll.hFile) CloseHandle(e.u.LoadDll.hFile);
            if (_stricmp(m.name.c_str(),"openbarrio_redirect.dll")==0) {
                result.error="Conflicting redirect DLL detected. Game stopped: never load both redirect DLLs.";
                TerminateProcess(process.value,1);
            }
            modules.push_back(m);
        } else if (e.dwDebugEventCode==UNLOAD_DLL_DEBUG_EVENT) {
            const uintptr_t base=reinterpret_cast<uintptr_t>(e.u.UnloadDll.lpBaseOfDll);
            std::erase_if(modules,[&](const Module& m){return m.base==base;});
        } else if (e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
            status=DBG_EXCEPTION_NOT_HANDLED;
            if (initialBreakpoint && e.u.Exception.dwFirstChance && e.u.Exception.ExceptionRecord.ExceptionCode==EXCEPTION_BREAKPOINT) {
                initialBreakpoint=false; status=DBG_CONTINUE;
            } else if (!e.u.Exception.dwFirstChance) {
                result.crashed=true;
                try { CrashReport(run,process.value,pi.dwProcessId,e,modules); }
                catch (const std::exception& ex) { result.error=ex.what(); }
                notify(L"Unhandled crash captured. Writing the report and minidump.");
            }
        } else if (e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT) { result.exitCode=e.u.ExitProcess.dwExitCode; done=true; }
        if (!ContinueDebugEvent(e.dwProcessId,e.dwThreadId,status)) {
            result.error=Error("ContinueDebugEvent failed"); DebugActiveProcessStop(pi.dwProcessId);
            WaitForSingleObject(process.value,INFINITE); DWORD code=0; GetExitCodeProcess(process.value,&code); result.exitCode=code; break;
        }
    }
    const auto seconds=std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-begin).count();
    try {
    const auto live=run.session / "runtime/openbarrio_test.log";
    if (fs::exists(live)) fs::copy_file(live,run.session/"openbarrio_test.log",fs::copy_options::overwrite_existing);
    if (!result.crashed) WriteText(run.session/"crash_report.txt","No second-chance crash observed. Nonzero exit codes alone are not classified as crashes.\n"+result.error+"\n");
    WriteText(run.session/"summary.txt",Summary(run.replace,run.hooks,run.enabled,static_cast<uint64_t>(seconds),result.crashed,result.exitCode,
        ReadText(run.session/"openbarrio_test.log"))+"\nObserver note: "+result.error+"\n");
    } catch (const std::exception& e) {
        result.error += std::string(" Session finalization failed: ")+e.what();
    }
    return result;
}
DWORD Command(const std::wstring& executable, const std::vector<std::wstring>& args, const fs::path& cwd, HANDLE output) {
    std::wstring command=Quote(executable); for (const auto& a:args) command+=L" "+Quote(a);
    STARTUPINFOW si{sizeof si}; si.dwFlags=STARTF_USESTDHANDLES; si.hStdOutput=output; si.hStdError=output;
    Handle input(CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr));
    si.hStdInput=input.value; SetHandleInformation(input.value,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(nullptr,command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,cwd.c_str(),&si,&pi)) throw std::runtime_error(Error("Could not run CMake"));
    Handle process(pi.hProcess),thread(pi.hThread); WaitForSingleObject(process.value,INFINITE);
    DWORD code=1; GetExitCodeProcess(process.value,&code); return code;
}
} // namespace
std::string ReadText(const fs::path& path) { std::ifstream in(path,std::ios::binary); return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()}; }
void WriteText(const fs::path& path,const std::string& text) {
    std::ofstream out(path,std::ios::binary); out.write(text.data(),static_cast<std::streamsize>(text.size()));
    if (!out) throw std::runtime_error("Could not write session file");
}
void WriteIni(const fs::path& path,const std::wstring& text) {
    std::ofstream out(path,std::ios::binary); const wchar_t bom=0xFEFF;
    out.write(reinterpret_cast<const char*>(&bom),sizeof bom); out.write(reinterpret_cast<const char*>(text.data()),static_cast<std::streamsize>(text.size()*sizeof(wchar_t)));
    if (!out) throw std::runtime_error("Could not write INI");
}
std::wstring Wide(const std::string& text) {
    const int n=MultiByteToWideChar(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0);
    std::wstring out(static_cast<size_t>(n),L'\0'); MultiByteToWideChar(CP_UTF8,0,text.data(),static_cast<int>(text.size()),out.data(),n); return out;
}
std::string Utf8(const std::wstring& text) {
    const int n=WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),nullptr,0,nullptr,nullptr);
    std::string out(static_cast<size_t>(n),'\0'); WideCharToMultiByte(CP_UTF8,0,text.data(),static_cast<int>(text.size()),out.data(),n,nullptr,nullptr); return out;
}
fs::path NewSession(const fs::path& root) {
    fs::create_directories(root); SYSTEMTIME t{}; GetLocalTime(&t); wchar_t stamp[64];
    swprintf_s(stamp,L"%04u%02u%02u_%02u%02u%02u",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);
    for (unsigned i=0;i<10000;++i) {
        const auto dir=root/(std::wstring(stamp)+(i ? L"_"+std::to_wstring(i):L""));
        if (fs::create_directory(dir)) return dir;
    }
    throw std::runtime_error("Could not create a unique session folder");
}
RunResult Play(const Run& run,const Notify& notify) {
    if (!launcher::RunningGamePrograms().empty()) throw std::runtime_error("Close the game and Sims launcher first");
    if (!launcher::SupportedBuild(mover::Sha256File(run.exe.wstring()))) throw std::runtime_error("Unsupported executable. Only original Steam TS3W.exe 1.67.2 is allowed (SHA-256 check).");
    if (_wcsicmp(run.exe.filename().c_str(),L"TS3W.exe")!=0) throw std::runtime_error("Select the original TS3W.exe");
    fs::create_directories(run.session/"runtime");
    fs::copy_file(run.dll,run.session/"runtime/openbarrio_test.dll");
    fs::copy_file(run.dll.parent_path()/"HOOKS.csv",run.session/"runtime/HOOKS.csv");
    const auto map=run.dll.parent_path()/"openbarrio_test.map";
    if (fs::exists(map)) fs::copy_file(map,run.session/"runtime/openbarrio_test.map");
    fs::copy_file(run.session/"harness_public.ini",run.session/"runtime/harness_public.ini");
    WriteText(run.session/"build.txt","Test DLL SHA-256: "+Utf8(mover::Sha256File(run.dll.wstring()))+"\nGame build: Steam 1.67.2\n");
    return DebugRun(run,L"",true,notify);
}
RunResult DebugFixture(const fs::path& exe,const std::wstring& arg,const fs::path& session) {
    fs::create_directories(session); Run run; run.exe=exe; run.session=session;
    return DebugRun(run,arg,false,[](const std::wstring&){});
}
BuildResult Build(const fs::path& source,const fs::path& runningExe,unsigned preset,const fs::path& log,const Notify& notify) {
    BuildResult result;
    try {
        const auto p=Preset(preset);
        if (!fs::exists(source/"CMakePresets.json") || !fs::exists(source/"testlab/logic.cpp")) throw std::runtime_error("Choose the OpenBarrio source folder");
        // A separate build tree avoids overwriting the running GUI. Alternate if launched from that tree.
        auto destination=source/"out"/(L"testlab-"+p.configure);
        if (mover::IsInside(runningExe.wstring(),destination.wstring())) destination+=L"-next";
        if (mover::IsInside(runningExe.wstring(),destination.wstring())) throw std::runtime_error("Run the Test Lab from the other build folder before rebuilding");
        result.directory=destination; fs::create_directories(source/"out/tmp");
        SetEnvironmentVariableW(L"TEMP",(source/"out/tmp").c_str()); SetEnvironmentVariableW(L"TMP",(source/"out/tmp").c_str());
        SECURITY_ATTRIBUTES sa{sizeof sa,nullptr,TRUE};
        Handle output(CreateFileW(log.c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr));
        if (output.value==INVALID_HANDLE_VALUE) throw std::runtime_error(Error("Build log could not be opened"));
        notify(L"Configuring "+p.configure+L" in "+destination.wstring());
        if (Command(L"cmake.exe",{L"--preset",p.configure,L"-B",destination.wstring()},source,output.value)) throw std::runtime_error("CMake configure failed; see build.log");
        notify(L"Building "+p.build+L" (one build job). Output: "+destination.wstring());
        if (Command(L"cmake.exe",{L"--build",destination.wstring(),L"--config",L"Release",L"--parallel",L"1"},source,output.value)) throw std::runtime_error("Build failed; see build.log");
        if (preset==0 && Command(L"ctest.exe",{L"--test-dir",destination.wstring(),L"-C",L"Release",L"--output-on-failure"},source,output.value)) throw std::runtime_error("Build succeeded, but tests failed; see build.log");
        result.ok=true;
    } catch (const std::exception& e) { result.error=e.what(); }
    return result;
}
void CopyData(const std::wstring& source,const std::wstring& base,const std::wstring& gameFolder,const Notify& notify) {
    if (!launcher::RunningGamePrograms().empty()) throw std::runtime_error("Close the game and Sims launcher before copying");
    if (mover::CheckDestination(base,source,gameFolder)!=mover::DestinationCheck::Ok) throw std::runtime_error("Invalid destination: use a writable folder outside the source and game install");
    const auto destination=mover::GameDataFolder(base);
    if (mover::FindExistingTree(destination)==mover::ExistingTree::SomeoneElse) throw std::runtime_error("Destination already contains data; it will not be merged or overwritten");
    const auto plan=mover::MakePlan(source,base);
    if (plan.bytes>plan.freeBytes) throw std::runtime_error("Not enough free space for the copy");
    if (plan.skippedLinks) throw std::runtime_error("Source contains links/junctions; choose a source without links for a complete backup");
    if (!mover::BeginCopy(destination)) throw std::runtime_error("Could not create copy marker");
    auto outcome=mover::CopyTree(source,destination,[&](const mover::Progress& p){ notify(L"Copying "+std::to_wstring(p.filesDone)+L" files: "+p.currentFile); return true; });
    if (outcome.result!=mover::Result::Done) throw std::runtime_error(Utf8(outcome.error));
    notify(L"Verifying copied files and save hashes...");
    const auto verified=mover::Verify(source,destination);
    if (!verified.ok) throw std::runtime_error(Utf8(verified.problem));
    if (mover::ClearReadOnly(destination).failed) throw std::runtime_error("Some copied files still have read-only flags");
    if (!mover::FinishCopy(destination,source)) throw std::runtime_error("Could not finalize the copy marker");
}
} // namespace openbarrio::lab
