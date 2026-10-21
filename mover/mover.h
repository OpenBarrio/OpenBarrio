// The save mover: copies The Sims 3 user data to a new folder without ever changing the old one, checks the copy,
// and clears the read-only flag that OneDrive leaves on copied files (the cause of EA's "error 16" on saving).
// No UI here: the launcher calls these functions, and tests/mover runs them on fixture folders.
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace openbarrio::mover {

// The game appends this to the Documents folder it is given.
inline constexpr wchar_t kGameSubfolder[] = L"Electronic Arts\\The Sims 3";
// Written into the new folder while a copy is in progress (so a cancelled copy can be resumed, and is never mistaken
// for an existing tree), and the note left there afterwards.
inline constexpr wchar_t kInProgressMarker[] = L"OPENBARRIO-COPY-IN-PROGRESS.txt";
inline constexpr wchar_t kMovedNote[] = L"MOVED-BY-OPENBARRIO.txt";
// The game's folder lookup fails for a base of 259 characters or more (it adds a separator, within 260).
inline constexpr size_t kMaxBaseLength = 258;

// `base` + "\Electronic Arts\The Sims 3".
std::wstring GameDataFolder(const std::wstring& base);
// The base for a folder the player picked: the base itself, its "Electronic Arts" folder, or its
// "Electronic Arts\The Sims 3" folder all give the same base (normalised).
std::wstring BaseFromChosen(const std::wstring& chosen);
// True when the folder itself looks like a Sims 3 data folder (it has Saves, Mods or Options.ini), e.g. a
// "The Sims 3" folder pasted somewhere without its "Electronic Arts" parent.
bool LooksLikeDataFolder(const std::wstring& folder);
// Absolute, normalised, without a trailing separator ("" when the path can't be resolved).
std::wstring Normalize(const std::wstring& path);
// True when `path` is `folder` or lies inside it (case-insensitive, on normalised paths).
bool IsInside(const std::wstring& path, const std::wstring& folder);

enum class DestinationCheck {
    Ok,
    NotAbsolute,
    TooLong,       // the base is longer than kMaxBaseLength
    Missing,       // the base folder doesn't exist
    NotWritable,
    InsideSource,  // inside the current data folder
    SourceInside,  // the current data folder is inside the new one's data folder
    InsideGame,    // inside the game's install folder
};
// The OneDrive check is separate (a warning, not an error): see IsInsideOneDrive.
DestinationCheck CheckDestination(const std::wstring& base, const std::wstring& sourceTree, const std::wstring& gameFolder);
// OneDrive folders from the OneDrive, OneDriveConsumer and OneDriveCommercial environment variables.
std::vector<std::wstring> OneDriveFolders();
bool IsInsideOneDrive(const std::wstring& path);

enum class ExistingTree {
    None,        // nothing there yet
    Ours,        // an unfinished copy of ours (the in-progress marker is there): resume it
    SomeoneElse  // a Sims 3 tree that isn't an unfinished copy: never merge into it
};
ExistingTree FindExistingTree(const std::wstring& destTree);

struct Plan {
    uint64_t files = 0;
    uint64_t bytes = 0;
    uint64_t cloudOnly = 0;     // files OneDrive keeps only online: copying downloads them
    uint64_t skippedLinks = 0;  // folder links (junctions, symlinks) inside the tree: not followed
    uint64_t freeBytes = 0;     // free space where the new folder goes
};
Plan MakePlan(const std::wstring& sourceTree, const std::wstring& destBase);

struct Progress {
    uint64_t filesDone = 0;
    uint64_t bytesDone = 0;
    std::wstring currentFile;  // relative to the tree
};
// Return false to cancel. Called often (at least once per file, and during large files).
using ProgressFn = std::function<bool(const Progress&)>;

enum class Result { Done, Cancelled, Failed };
struct CopyOutcome {
    Result result = Result::Failed;
    uint64_t copied = 0;
    uint64_t skipped = 0;  // already there from a cancelled run (same size and SHA-256)
    std::wstring error;
};
// Copies every file of sourceTree into destTree (created as needed), keeping timestamps. The source is only read.
// Files left by a cancelled run are skipped when they match, and copied again when they don't.
CopyOutcome CopyTree(const std::wstring& sourceTree, const std::wstring& destTree, const ProgressFn& progress);

struct VerifyOutcome {
    bool ok = false;
    uint64_t files = 0;
    uint64_t hashed = 0;  // files compared by SHA-256 (everything under Saves)
    std::wstring problem;
};
// Same files, same sizes, and the same SHA-256 for every file under "Saves".
VerifyOutcome Verify(const std::wstring& sourceTree, const std::wstring& destTree);

struct ReadOnlyOutcome {
    uint64_t cleared = 0;
    uint64_t failed = 0;
};
// Clears the read-only attribute on every file and folder in the tree. Changes nothing else.
ReadOnlyOutcome ClearReadOnly(const std::wstring& tree);

// The copy steps around CopyTree: start writes the in-progress marker, finish replaces it with the note.
bool BeginCopy(const std::wstring& destTree);
bool FinishCopy(const std::wstring& destTree, const std::wstring& sourceTree);

// SHA-256 of a file as lowercase hex ("" on a read error).
std::wstring Sha256File(const std::wstring& path);

// Tests only: decides whether a found file counts as cloud-only (default: its attributes say so).
void SetCloudOnlyProbeForTests(bool (*probe)(const WIN32_FIND_DATAW&));

}  // namespace openbarrio::mover
