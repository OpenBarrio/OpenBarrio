#include "runtime.h"
#include <iostream>
using namespace openbarrio::lab;
int wmain(int argc,wchar_t** argv) {
    if (argc!=2) return 2;
    try {
        const auto root=fs::absolute("sessions/debugger-tests");
        for (const auto* mode:{L"handled",L"crash",L"exit"}) {
            const auto session=NewSession(root); const auto result=DebugFixture(fs::absolute(argv[1]),mode,session);
            const bool crash=std::wstring(mode)==L"crash";
            if (!result.started || !result.error.empty() || result.crashed!=crash || fs::exists(session/"crash.dmp")!=crash)
                throw std::runtime_error("Debugger classification failed: "+result.error);
            if (!crash && result.exitCode!=(std::wstring(mode)==L"handled"?0u:7u)) throw std::runtime_error("Incorrect exit code");
            if (crash && ReadText(session/"crash_report.txt").find("Module: unknown")!=std::string::npos) throw std::runtime_error("Crash module not resolved");
            if (crash && fs::file_size(session/"crash.dmp")<1024) throw std::runtime_error("Dump is empty");
            if (crash && ReadText(session/"crash_report.txt").find("0xE0421001")==std::string::npos) throw std::runtime_error("Exception missing from report");
        }
        std::cout << "Debugger: handled exception ignored, crash dumped, ordinary nonzero exit not a crash\n";
        return 0;
    } catch (const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
