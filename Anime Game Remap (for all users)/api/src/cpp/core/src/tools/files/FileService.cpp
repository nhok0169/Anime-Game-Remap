// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "AGRemapCore/tools/files/FileService.h"

#include <algorithm>

#include <filesystem>
#include <tuple>


namespace {
    // ===== THE ORDER A WALK VISITS THINGS IN IS PART OF THE OUTPUT =====
    //
    // std::filesystem promises no order at all, and neither did the os.walk this ports. On NTFS the
    // OS happens to hand names back sorted, so every Windows run saw one order; on ext4 it is the
    // directory's hash order, which differs from machine to machine. And the order reaches the
    // output: which .ini of a folder is fixed first decides the names its fix generates, and which
    // mod is visited last decides what the summary log opens with. The Integration Tester's goldens
    // (produced on NTFS) failed 8 tests on a GitHub Actions runner (ext4) for exactly this.
    //
    // So both lists are sorted, and into WINDOWS' order specifically, so that making the order fixed
    // changed nothing any Windows user has ever seen: a name is compared with its ASCII letters folded
    // to upper case, byte by byte, which is how NTFS collates -- 'a' before 'B', and 'ab' before 'a_b'
    // because 'B' (0x42) sorts before '_' (0x5F). Non-ASCII letters are not folded (NTFS would), so two
    // names differing only in the case of a non-ASCII letter may order differently from Windows; they
    // still order the same way on every machine, which is the property that matters.
    //
    // A path is compared component by component, not as one string, so that a recursive walk comes
    // out PRE-ORDER as it does on NTFS -- a folder, everything under it, then its next sibling. As a
    // single string 'A-B' would sort before 'A/X' ('-' is 0x2D, '/' is 0x2F).
    std::string collationKey(std::string name) {
        for (char& c : name) {
            if (c >= 'a' && c <= 'z') {
                c = static_cast<char>(c - 'a' + 'A');
            }
        }
        return name;
    }

    void sortForWalk(std::vector<std::string>& paths) {
        struct Entry {
            std::vector<std::string> keys;
            std::vector<std::string> names;
            std::string path;
        };

        std::vector<Entry> entries;
        entries.reserve(paths.size());

        for (std::string& path : paths) {
            Entry entry;
            for (const std::filesystem::path& part : AGRemapCore::FileService::strToPath(path)) {
                const std::string name = AGRemapCore::FileService::pathToStr(part);
                entry.keys.push_back(collationKey(name));
                entry.names.push_back(name);
            }

            entry.path = std::move(path);
            entries.push_back(std::move(entry));
        }

        // the exact names break a tie two keys cannot -- two names differing only in case, which NTFS
        // never holds in one folder and ext4 can
        std::sort(entries.begin(), entries.end(), [](const Entry& left, const Entry& right) {
            return std::tie(left.keys, left.names) < std::tie(right.keys, right.names);
        });

        paths.clear();
        for (Entry& entry : entries) {
            paths.push_back(std::move(entry.path));
        }
    }
}


namespace AGRemapCore {
    std::string FileService::pathToStr(const std::filesystem::path& path) {
        // u8string() rather than string(): the latter goes through the active code page and throws
        // on anything it cannot represent. See this function's own docs for the mod that proved it.
        const std::u8string utf8 = path.u8string();
        return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
    }

    std::string FileService::pathToIniStr(const std::filesystem::path& path) {
        std::string result = pathToStr(path);

        // On Windows this is already the separator std::filesystem produced, so the loop finds
        // nothing. Elsewhere it converts the native '/' back to what the .ini format uses.
        std::replace(result.begin(), result.end(), '/', '\\');
        return result;
    }

    std::filesystem::path FileService::strToPath(const std::string& path) {
        // ===== A BACKSLASH IS A SEPARATOR HERE, EVEN ON LINUX =====
        //
        // GIMI .ini files are Windows artifacts and say so: a mod's own resources are written
        //
        //     filename = .\AyakaspringbloomMod1\AyakaspringbloomBlend.buf
        //
        // On POSIX a backslash is an ordinary filename character, so that whole string names one
        // file that does not exist rather than a file two directories down, and the fix dies with
        // "Unable to open file" on a mod that is perfectly well formed. Found by running the real
        // CLI under WSL (2026-09-11) -- every mod whose .ini points into a subfolder was affected,
        // which is most merged mods.
        //
        // Translating here rather than at the .ini layer because this is the ONE place a path-shaped
        // std::string becomes a std::filesystem::path, so every caller is covered at once: resource
        // reads, texture writes, backups, downloads.
        //
        // Windows is left byte-for-byte alone. It already treats both separators as equivalent, so
        // there is nothing to gain there and a compiled-out branch cannot regress the platform that
        // currently works.
#ifndef _WIN32
        if (path.find('\\') != std::string::npos) {
            std::string posix = path;
            std::replace(posix.begin(), posix.end(), '\\', '/');
            return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(posix.data()), posix.size()));
        }
#endif

        // Constructing from char8_t is what tells std::filesystem these bytes are UTF-8; from a
        // plain char sequence it would read them as the active code page instead.
        return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(path.data()), path.size()));
    }

    std::string FileService::absPathOfRelPath(const std::string& dstPath, const std::string& relFolder) {
        std::filesystem::path path = strToPath(dstPath);

        if (path.is_absolute()) {
            return pathToStr(path.lexically_normal());
        }

        // An EMPTY folder means the working directory, as os.path.abspath("") does. It has to be
        // spelled out: std::filesystem::absolute("") throws on both MSVC and GCC, and an .ini file
        // with no path (IniFile's 'txt' constructor) hands every resource an empty folder.
        std::filesystem::path absFolder = relFolder.empty() ? std::filesystem::current_path()
                                                            : std::filesystem::absolute(strToPath(relFolder));
        return pathToStr((absFolder / path).lexically_normal());
    }

    const std::string& FileService::defaultPath() {
        // Captured on first use, mirroring FilePathConsts.DefaultPath being evaluated once at
        //   import time rather than re-read per call -- RemapService compares against this to
        //   decide 'pathIsCwd', so it has to stay stable across any later chdir.
        static const std::string startupPath = pathToStr(std::filesystem::current_path());
        return startupPath;
    }

    std::string FileService::parseOSPath(const std::string& path) {
        return pathToStr(strToPath(path).lexically_normal());
    }

    std::string FileService::getPath(const std::optional<std::string>& path) {
        if (!path.has_value()) {
            return defaultPath();
        }

        return *path;
    }

    std::pair<std::vector<std::string>, std::vector<std::string>> FileService::getFilesAndDirs(const std::string& path,
                                                                                              bool recursive) {
        std::vector<std::string> files;
        std::vector<std::string> dirs;

        // A folder the OS refuses to read is not an error here -- os.walk swallows the same
        // failure by default, and RemapService's walk simply has nothing to visit inside it.
        std::error_code err;

        auto collect = [&files, &dirs](const std::filesystem::directory_entry& entry) {
            std::error_code entryErr;

            // A broken symlink, or an entry deleted between listing and stat-ing it, answers
            // neither question -- skip it rather than guessing which list it belongs in.
            if (entry.is_directory(entryErr) && !entryErr) {
                dirs.push_back(pathToStr(entry.path()));
            } else if (entry.is_regular_file(entryErr) && !entryErr) {
                files.push_back(pathToStr(entry.path()));
            }
        };

        if (recursive) {
            // skip_permission_denied so one unreadable subfolder doesn't abort the whole walk,
            // matching os.walk's own silent-by-default error handling.
            std::filesystem::recursive_directory_iterator it(strToPath(path),
                                                             std::filesystem::directory_options::skip_permission_denied,
                                                             err);
            if (err) {
                return {std::move(files), std::move(dirs)};
            }

            for (const std::filesystem::directory_entry& entry : it) {
                collect(entry);
            }

            sortForWalk(files);
            sortForWalk(dirs);
            return {std::move(files), std::move(dirs)};
        }

        std::filesystem::directory_iterator it(strToPath(path), std::filesystem::directory_options::skip_permission_denied, err);
        if (err) {
            return {std::move(files), std::move(dirs)};
        }

        for (const std::filesystem::directory_entry& entry : it) {
            collect(entry);
        }

        sortForWalk(files);
        sortForWalk(dirs);
        return {std::move(files), std::move(dirs)};
    }

    std::string FileService::getRelPath(const std::string& path, const std::string& start) {
        std::filesystem::path relPath = strToPath(path).lexically_relative(strToPath(start));

        // lexically_relative returns an empty path when no relation exists (eg. two different
        // Windows drives) -- that is the case the pure-Python original catches a ValueError for,
        // and it answers it the same way, by handing the original path back.
        if (relPath.empty()) {
            return path;
        }

        return pathToStr(relPath);
    }
}
