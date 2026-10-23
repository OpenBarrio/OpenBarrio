#include "logic.h"
#include <iostream>
#include <stdexcept>
using namespace openbarrio::lab;
static unsigned checks = 0;
void Check(bool condition) { ++checks; if (!condition) throw std::runtime_error("check " + std::to_string(checks)); }
int main() {
    try {
        const auto hooks = ParseHooks("patch,address,function,module,in_game,compare_safe\n"
            "a,0x401000,\"A,x\",m01,passed,yes\n"
            "b,0x402000,B,m01,passed,replace-only\n"
            "c,0x403000,C,m02,passed,yes\n"
            "io.SpecialDirectory,0x4DC100,D,pilot,passed,replace-only\n");
        Check(hooks.size() == 4 && hooks[0].function == "A,x");
        Selection all{"a", "b", "c"};
        Check(Effective(hooks, all, false, false) == Selection({"a", "c"}));
        Check(Effective(hooks, all, true, true).contains("io.SpecialDirectory"));
        const auto ini = MakeIni(hooks, {"a"}, false, L"");
        Check(ini.find(L"patches_default=0") != std::wstring::npos);
        Check(ini.find(L"b=0") != std::wstring::npos);
        Check(ini.find(L"a=1") != std::wstring::npos);
        bool threw = false; try { MakeIni(hooks, all, false, L"folder"); } catch (...) { threw = true; } Check(threw);
        Check(MakeIni(hooks, {}, true, L"folder").find(L"io.SpecialDirectory=1") != std::wstring::npos);
        Check(NearestHooks(hooks, all, 0x402010, 1)[0].name == "b");
        Check(NearestHooks(hooks, {"a"}, 0x402010, 1)[0].name == "a");
        Check(NearestHooks(hooks, all, 0x400000).empty());
        // Reproduce each possible single culprit, including negative halves and the final confirmation run.
        for (const auto& culprit : all) {
            Bisect b; b.candidates = all; Propose(b, hooks); unsigned runs = 0;
            while (!b.finished && runs++ < 8) {
                Check(!b.trial.empty()); b = LoadBisect(SaveBisect(b), hooks);
                Observe(b, b.trial.contains(culprit), hooks);
            }
            Check(b.finished && b.reproduced && b.candidates == Selection({culprit}));
        }
        Bisect b; b.candidates = {"a"}; Propose(b, hooks); Observe(b, false, hooks); Check(b.finished && b.candidates.empty());
        const auto counts = ParseLog("a: 8 2\na: 12 3\nb: 4 0\nnoise\n");
        Check(counts.at("a").mismatches == 3 && counts.at("a").calls == 12);
        Check(Summary(false, hooks, {"a"}, 4, false, 0, "a: 12 3\n").find("3 mismatches") != std::string::npos);
        const auto symbols=ParseMap(" Preferred load address is 10000000\n"
            " 0001:00001000 ?function@@ 10002000 f object.obj\n"
            " 0002:00000000 data 10003000 object.obj\n"
            " 0001:00000000 ?other@@ 10001000 f i object.obj\n");
        Check(symbols.size()==2 && symbols[0].address==0x1000 && symbols[1].address==0x2000);
        Check(ParseMap("0001:00000000 bad 10001000 f\n").empty());
        bool invalid=false;try{ParseHooks("wrong,header\n");}catch(...){invalid=true;}Check(invalid);
        invalid=false;try{LoadBisect("bisect-v1 0 0 0\ncandidate removed\ntrial removed\n",hooks);}catch(...){invalid=true;}Check(invalid);
        Check(Preset(0).configure == L"x86" && Preset(1).build == L"x64-release");
        Check(Quote(L"folder name\\") == L"\"folder name\\\\\"");
        Check(Quote(L"a\"b") == L"\"a\\\"b\"");
        std::cout << checks << " Test Lab checks passed\n"; return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
