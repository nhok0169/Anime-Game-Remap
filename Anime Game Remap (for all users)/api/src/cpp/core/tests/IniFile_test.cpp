// -----------------------------------------------------------------------------
// Standalone regression test for AGRemapCore::IniFile (model/files/IniFile.h) --
// covers just the constructor + readFileLines() slice implemented so far (this
// is an incremental, from-scratch C++ port, NOT a 1-1 translation of the
// pure-Python FixRaidenBoss2.model.files.IniFile original -- see IniFile.h's
// own doc comment).
//
// Covers, against the documented contract in IniFile.h (itself matched against
// the pure-Python original's IniFile.__init__/_setupFileLines/readFileLines,
// model/files/IniFile.py):
//   * Constructing with only 'txt' (no file): fileTxt/fileLines/fileLinesRead
//     are populated immediately from 'txt', with no disk access
//   * getFileLines() keepends semantics, matching Python's
//     str.splitlines(keepends = True): trailing "\n" terminates the last line
//     rather than starting a new empty one; empty txt -> zero lines; a single
//     line with no trailing newline is still returned
//   * Constructing with a 'file' path: fileLinesRead() is false and
//     getFileLines()/getFileTxt() are empty until readFileLines() is actually
//     called (no eager disk read in the constructor)
//   * readFileLines() when 'file' is set: reads real bytes off disk, normalizes
//     "\r\n" and lone "\r" line endings down to "\n" (matching Python
//     text-mode's universal newline translation), and returns/stores the
//     resulting keepends lines
//   * readFileLines() when 'file' is std::nullopt: no disk access at all,
//     just returns the existing getFileLines() (from the constructor's 'txt')
//   * readFileLines() throws std::runtime_error for a file path that doesn't
//     exist
//
// This file has NO dependency on the project's build system (CMake/pybind11),
// Z3, utf8proc, ordered-map, or xxHash -- IniFile is pure standard-library
// code so far. Compile directly, e.g.:
//
//   cl /std:c++latest /EHsc /nologo /I <core>/include ^
//      IniFile_test.cpp <core>/src/model/files/IniFile.cpp ^
//      /Fe:test.exe
//
// (g++/clang++ equivalent: swap /std:c++latest /EHsc /I /Fe: for
//  -std=c++23 -I ... -o test.exe)
// -----------------------------------------------------------------------------

#include "AGRemapCore/model/files/IniFile.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using AGRemapCore::IniFile;

namespace {

int failures = 0;

void check(bool condition, const char* description) {
    if (condition) {
        std::printf("[PASS] %s\n", description);
    } else {
        std::printf("[FAIL] %s\n", description);
        failures++;
    }
}

bool linesEqual(const std::vector<std::string>& actual, const std::vector<std::string>& expected) {
    return actual == expected;
}

void testTxtOnlyConstruction() {
    // No trailing newline: the last line is still returned, just without a "\n".
    {
        IniFile ini(std::nullopt, "abc");
        check(ini.getFile() == std::nullopt, "txt-only: getFile() is nullopt");
        check(ini.getFileTxt() == "abc", "txt-only, no trailing newline: fileTxt matches");
        check(ini.fileLinesRead(), "txt-only: fileLinesRead() is true immediately after construction");
        check(linesEqual(ini.getFileLines(), {"abc"}), "txt-only, no trailing newline: fileLines == [\"abc\"]");
    }

    // Trailing newline terminates the last line rather than starting a new empty one.
    {
        IniFile ini(std::nullopt, "abc\n");
        check(linesEqual(ini.getFileLines(), {"abc\n"}), "txt-only, trailing newline: fileLines == [\"abc\\n\"]");
    }

    // Multiple lines, keepends semantics.
    {
        IniFile ini(std::nullopt, "[Constants]\nglobal $x = 0\n");
        check(linesEqual(ini.getFileLines(), {"[Constants]\n", "global $x = 0\n"}),
              "txt-only, multi-line: fileLines keeps each line's own newline");
    }

    // Empty content.
    {
        IniFile ini(std::nullopt, "");
        check(ini.getFileTxt().empty(), "txt-only, empty txt: fileTxt is empty");
        check(ini.getFileLines().empty(), "txt-only, empty txt: fileLines is empty");
    }

    // Default 'txt' argument (constructor called with no arguments at all).
    {
        IniFile ini;
        check(ini.getFile() == std::nullopt, "default-constructed: getFile() is nullopt");
        check(ini.getFileTxt().empty(), "default-constructed: fileTxt is empty");
        check(ini.fileLinesRead(), "default-constructed: fileLinesRead() is still true (txt defaults to \"\")");
    }
}

void testFileConstructionIsLazy() {
    IniFile ini("some/nonexistent/path.ini");
    check(ini.getFile().has_value() && *ini.getFile() == "some/nonexistent/path.ini", "file-backed: getFile() returns the given path");
    check(!ini.fileLinesRead(), "file-backed, before readFileLines(): fileLinesRead() is false");
    check(ini.getFileTxt().empty(), "file-backed, before readFileLines(): fileTxt is empty (no eager read)");
    check(ini.getFileLines().empty(), "file-backed, before readFileLines(): fileLines is empty (no eager read)");
}

void testReadFileLinesFromDisk(const std::string& scratchDir) {
    // Mixed line endings + no trailing newline on the last line -- exercises
    // "\n", "\r\n", and lone "\r" normalization all in one file.
    const std::string path = scratchDir + "/IniFile_test_mixed.ini";
    {
        std::ofstream out(path, std::ios::binary);
        out << "[TextureOverrideA]\r\n" << "hash = abc\n" << "match_priority = 0\r" << "no_trailing_newline";
    }

    IniFile ini(path);
    const std::vector<std::string>& returned = ini.readFileLines();

    std::vector<std::string> expected = {
        "[TextureOverrideA]\n",
        "hash = abc\n",
        "match_priority = 0\n",
        "no_trailing_newline",
    };

    check(linesEqual(returned, expected), "readFileLines(): CRLF/CR/LF all normalize to LF, keepends preserved");
    check(linesEqual(ini.getFileLines(), expected), "readFileLines(): getFileLines() reflects the same result afterward");
    check(ini.getFileTxt() == "[TextureOverrideA]\nhash = abc\nmatch_priority = 0\nno_trailing_newline",
          "readFileLines(): getFileTxt() is the normalized, joined text");
    check(ini.fileLinesRead(), "readFileLines(): fileLinesRead() becomes true");

    // Re-reading re-reads from disk rather than just returning a cached value --
    // confirmed by changing the file on disk between calls.
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out << "changed\n";
    }
    const std::vector<std::string>& reread = ini.readFileLines();
    check(linesEqual(reread, {"changed\n"}), "readFileLines(): a second call re-reads from disk (not cached)");

    std::remove(path.c_str());
}

void testReadFileLinesNoFileDoesNotTouchDisk() {
    IniFile ini(std::nullopt, "hello\n");
    const std::vector<std::string>& result = ini.readFileLines();
    check(linesEqual(result, {"hello\n"}), "readFileLines(), file-less: returns the existing txt-derived lines");
}

void testReadFileLinesMissingFileThrows() {
    IniFile ini("some/definitely/missing/path.ini");
    bool threw = false;
    try {
        ini.readFileLines();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    check(threw, "readFileLines(): throws std::runtime_error for a file path that can't be opened");
}

}  // namespace

int main(int argc, char** argv) {
    std::string scratchDir = ".";
    if (argc > 1) {
        scratchDir = argv[1];
    }

    testTxtOnlyConstruction();
    testFileConstructionIsLazy();
    testReadFileLinesFromDisk(scratchDir);
    testReadFileLinesNoFileDoesNotTouchDisk();
    testReadFileLinesMissingFileThrows();

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
