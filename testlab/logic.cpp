#include "logic.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace openbarrio::lab {
namespace {
std::vector<std::string> CsvRow(const std::string& line) {
    std::vector<std::string> cells; std::string cell; bool quoted = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '"') {
            if (quoted && i + 1 < line.size() && line[i+1] == '"') { cell += '"'; ++i; }
            else quoted = !quoted;
        } else if (c == ',' && !quoted) { cells.push_back(cell); cell.clear(); }
        else if (c != '\r') cell += c;
    }
    if (quoted) throw std::runtime_error("Unclosed CSV quote");
    cells.push_back(cell); return cells;
}
bool SafeKey(const std::string& key) { return !key.empty() && key.find_first_of("\r\n=[]") == std::string::npos; }
}
std::vector<Hook> ParseHooks(const std::string& csv) {
    std::istringstream in(csv); std::string line; std::getline(in, line);
    auto header = CsvRow(line);
    if (header.size() < 6 || header[0] != "patch" || header[1] != "address" || header[5] != "compare_safe")
        throw std::runtime_error("Invalid HOOKS.csv header");
    std::vector<Hook> hooks; Selection names;
    while (std::getline(in, line)) {
        if (line.empty() || line == "\r") continue;
        auto c = CsvRow(line);
        if (c.size() != header.size() || !SafeKey(c[0]) || !names.insert(c[0]).second)
            throw std::runtime_error("Invalid or duplicate hook row");
        size_t used = 0; const auto address = std::stoul(c[1], &used, 16);
        if (used != c[1].size() || address < 0x400000) throw std::runtime_error("Invalid hook address");
        hooks.push_back({c[0], c[2], c[3].empty() ? "unassigned" : c[3], static_cast<uint32_t>(address), c[5] == "yes"});
    }
    if (hooks.empty()) throw std::runtime_error("The hook catalog is empty");
    return hooks;
}
Selection Effective(const std::vector<Hook>& hooks, const Selection& wanted, bool replace, bool redirect) {
    Selection out;
    for (const auto& h : hooks)
        if ((wanted.contains(h.name) || (redirect && h.name == "io.SpecialDirectory")) && (replace || h.compareSafe)) out.insert(h.name);
    return out;
}
std::wstring MakeIni(const std::vector<Hook>& hooks, const Selection& wanted, bool replace, const std::wstring& redirect) {
    if (!redirect.empty() && !replace) throw std::runtime_error("Folder redirect requires Replace mode");
    if (redirect.find_first_of(L"\r\n") != std::wstring::npos) throw std::runtime_error("Invalid folder name");
    const auto enabled = Effective(hooks, wanted, replace, !redirect.empty());
    if (!redirect.empty() && !enabled.contains("io.SpecialDirectory")) throw std::runtime_error("Folder redirect hook is missing");
    std::wstring out = L"[harness]\r\nmode=" + std::wstring(replace ? L"replace" : L"compare") +
        L"\r\npatches_default=0\r\nuser_documents_dir=" + redirect + L"\r\n\r\n[patches]\r\n";
    for (const auto& h : hooks) out += std::wstring(h.name.begin(), h.name.end()) + (enabled.contains(h.name) ? L"=1\r\n" : L"=0\r\n");
    return out;
}
std::vector<Hook> NearestHooks(const std::vector<Hook>& hooks, const Selection& enabled, uint32_t address, size_t count) {
    std::vector<Hook> result;
    for (const auto& h : hooks) if (enabled.contains(h.name) && h.address <= address) result.push_back(h);
    std::sort(result.begin(), result.end(), [](const Hook& a, const Hook& b) { return a.address > b.address; });
    if (result.size() > count) result.resize(count);
    return result;
}
std::vector<MapSymbol> ParseMap(const std::string& text) {
    std::istringstream in(text); std::vector<MapSymbol> symbols; std::string line; uintptr_t base = 0;
    while (std::getline(in, line)) {
        constexpr char marker[] = "Preferred load address is";
        const auto at = line.find(marker);
        if (at != std::string::npos) { std::istringstream value(line.substr(at + sizeof(marker)-1)); value >> std::hex >> base; continue; }
        std::istringstream values(line); std::string section, name, address, flag;
        if (!(values >> section >> name >> address >> flag) || section.find(':') == std::string::npos || flag != "f") continue;
        try {
            size_t used=0; const auto absolute=std::stoull(address,&used,16);
            if (base && used==address.size() && absolute>=base && absolute-base<=UINT32_MAX)
                symbols.push_back({static_cast<uintptr_t>(absolute-base),name});
        } catch (...) { /* Not a symbol row. */ }
    }
    std::sort(symbols.begin(),symbols.end(),[](const auto& a,const auto& b){return a.address<b.address;});
    return symbols;
}
void Propose(Bisect& state, const std::vector<Hook>& hooks) {
    state.trial.clear();
    if (state.finished || state.candidates.empty()) { state.finished = true; return; }
    std::map<std::string, Selection> groups;
    for (const auto& h : hooks) if (state.candidates.contains(h.name)) groups[h.module].insert(h.name);
    if (groups.size() <= 1) state.modules = false;
    if (state.modules) {
        size_t n = (groups.size() + 1) / 2;
        for (const auto& [module, names] : groups) { (void)module; if (!n--) break; state.trial.insert(names.begin(), names.end()); }
    } else {
        size_t n = (state.candidates.size() + 1) / 2;
        for (const auto& name : state.candidates) { if (!n--) break; state.trial.insert(name); }
    }
}
void Observe(Bisect& state, bool crashed, const std::vector<Hook>& hooks) {
    if (state.finished || state.trial.empty()) throw std::runtime_error("No pending bisect trial");
    if (crashed) state.candidates = state.trial;
    else for (const auto& name : state.trial) state.candidates.erase(name);
    state.reproduced = crashed;
    state.finished = state.candidates.empty() || (crashed && state.candidates.size() == 1);
    Propose(state, hooks);
}
std::string SaveBisect(const Bisect& s) {
    std::ostringstream out; out << "bisect-v1 " << s.modules << ' ' << s.finished << ' ' << s.reproduced << '\n';
    for (const auto& n : s.candidates) out << "candidate " << n << '\n';
    for (const auto& n : s.trial) out << "trial " << n << '\n';
    return out.str();
}
Bisect LoadBisect(const std::string& text, const std::vector<Hook>& hooks) {
    std::istringstream in(text); Bisect s; std::string version, kind, name; Selection known;
    for (const auto& h : hooks) known.insert(h.name);
    if (!(in >> version >> s.modules >> s.finished >> s.reproduced) || version != "bisect-v1") throw std::runtime_error("Invalid bisect state");
    while (in >> kind >> name) {
        if (!known.contains(name)) throw std::runtime_error("Bisect catalog changed; start a new search");
        if (kind == "candidate") s.candidates.insert(name);
        else if (kind == "trial") s.trial.insert(name);
        else throw std::runtime_error("Invalid bisect entry");
    }
    for (const auto& n : s.trial) if (!s.candidates.contains(n)) throw std::runtime_error("Trial outside candidate set");
    if (!s.finished && (s.candidates.empty() || s.trial.empty())) throw std::runtime_error("Empty active bisect");
    return s;
}
unsigned RemainingRuns(const Bisect& state, const std::vector<Hook>& hooks) {
    if (state.finished) return 0;
    std::set<std::string> modules;
    for (const auto& h : hooks) if (state.candidates.contains(h.name)) modules.insert(h.module);
    auto bits = [](size_t n) { unsigned v = 0; for (size_t k = 1; k < n; k *= 2) ++v; return v; };
    return 1 + bits(state.candidates.size()) + (state.modules ? bits(modules.size()) : 0);
}
std::map<std::string, Counts> ParseLog(const std::string& log) {
    std::map<std::string, Counts> result; std::istringstream in(log); std::string line;
    while (std::getline(in, line)) {
        const auto colon = line.find(": "); if (colon == std::string::npos) continue;
        Counts c; std::istringstream values(line.substr(colon + 2));
        if (values >> c.calls >> c.mismatches) {
            auto& prev = result[line.substr(0, colon)];
            prev.calls = std::max(prev.calls, c.calls); prev.mismatches = std::max(prev.mismatches, c.mismatches);
        }
    }
    return result;
}
std::string Summary(bool replace, const std::vector<Hook>& hooks, const Selection& enabled, uint64_t seconds,
                    bool crash, uint32_t exitCode, const std::string& log) {
    std::ostringstream out; out << "Mode: " << (replace ? "Replace" : "Compare") << "\nPlay time (seconds): " << seconds
        << "\nSecond-chance crash: " << (crash ? "yes" : "no") << "\nExit code: 0x" << std::hex << exitCode << std::dec
        << "\nEnabled hooks: " << enabled.size() << "\n";
    std::map<std::string, size_t> modules;
    for (const auto& h : hooks) if (enabled.contains(h.name)) ++modules[h.module];
    for (const auto& [name, count] : modules) out << "Module " << name << ": " << count << '\n';
    out << "\nTop mismatching hooks (latest cumulative counts):\n";
    std::vector<std::pair<std::string, Counts>> counts;
    for (const auto& entry : ParseLog(log)) if (entry.second.mismatches) counts.push_back(entry);
    std::sort(counts.begin(), counts.end(), [](const auto& a, const auto& b) { return a.second.mismatches > b.second.mismatches; });
    for (size_t i = 0; i < std::min<size_t>(counts.size(), 10); ++i)
        out << counts[i].first << ": " << counts[i].second.calls << " calls, " << counts[i].second.mismatches << " mismatches\n";
    if (counts.empty()) out << "None reported; missing counters do not prove a hook ran.\n";
    out << "\nEnabled hook names:\n";
    for (const auto& name : enabled) out << name << '\n';
    return out.str();
}
BuildPreset Preset(unsigned index) {
    if (index == 0) return {L"x86", L"x86-release"};
    if (index == 1) return {L"x64", L"x64-release"};
    throw std::runtime_error("Unknown build preset");
}
std::wstring Quote(const std::wstring& arg) {
    std::wstring out = L"\""; size_t slashes = 0;
    for (wchar_t c : arg) {
        if (c == L'\\') { ++slashes; continue; }
        out.append(c == L'"' ? 2 * slashes + 1 : slashes, L'\\'); slashes = 0; out += c;
    }
    out.append(2 * slashes, L'\\'); out += L'"'; return out;
}
} // namespace openbarrio::lab
