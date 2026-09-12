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

        std::filesystem::path absFolder = std::filesystem::absolute(strToPath(relFolder));
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

            return {std::move(files), std::move(dirs)};
        }

        std::filesystem::directory_iterator it(strToPath(path), std::filesystem::directory_options::skip_permission_denied, err);
        if (err) {
            return {std::move(files), std::move(dirs)};
        }

        for (const std::filesystem::directory_entry& entry : it) {
            collect(entry);
        }

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
