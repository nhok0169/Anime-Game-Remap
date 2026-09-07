#include "AGRemapCore/tools/files/FileService.h"

#include <filesystem>


namespace AGRemapCore {
    std::string FileService::pathToStr(const std::filesystem::path& path) {
        // u8string() rather than string(): the latter goes through the active code page and throws
        // on anything it cannot represent. See this function's own docs for the mod that proved it.
        const std::u8string utf8 = path.u8string();
        return std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size());
    }

    std::filesystem::path FileService::strToPath(const std::string& path) {
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
