#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace openbarrio::lab {
struct Hook { std::string name, function, module; uint32_t address = 0; bool compareSafe = false; };
using Selection = std::set<std::string>;
std::vector<Hook> ParseHooks(const std::string& csv);
Selection Effective(const std::vector<Hook>& hooks, const Selection& wanted, bool replace, bool redirect);
std::wstring MakeIni(const std::vector<Hook>& hooks, const Selection& wanted, bool replace, const std::wstring& redirect);
std::vector<Hook> NearestHooks(const std::vector<Hook>& hooks, const Selection& enabled, uint32_t address, size_t count = 5);
struct MapSymbol { uintptr_t address = 0; std::string name; };
std::vector<MapSymbol> ParseMap(const std::string& text);
struct Bisect {
    Selection candidates, trial;
    bool modules = true, finished = false, reproduced = false;
};
void Propose(Bisect& state, const std::vector<Hook>& hooks);
void Observe(Bisect& state, bool crashed, const std::vector<Hook>& hooks);
std::string SaveBisect(const Bisect& state);
Bisect LoadBisect(const std::string& text, const std::vector<Hook>& hooks);
unsigned RemainingRuns(const Bisect& state, const std::vector<Hook>& hooks);
struct Counts { uint64_t calls = 0, mismatches = 0; };
std::map<std::string, Counts> ParseLog(const std::string& log);
std::string Summary(bool replace, const std::vector<Hook>& hooks, const Selection& enabled, uint64_t seconds,
                    bool crash, uint32_t exitCode, const std::string& log);
struct BuildPreset { std::wstring configure, build; };
BuildPreset Preset(unsigned index);
std::wstring Quote(const std::wstring& arg);
} // namespace openbarrio::lab
