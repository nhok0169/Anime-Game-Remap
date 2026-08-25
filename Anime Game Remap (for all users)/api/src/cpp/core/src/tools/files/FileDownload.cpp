#include "AGRemapCore/tools/files/FileDownload.h"

#include <filesystem>
#include <stdexcept>
#include <utility>


namespace AGRemapCore {
    FileDownload::FileDownload(std::string url, std::string filename, bool cache):
        url(std::move(url)), filename(std::move(filename)), cache(cache) {}

    std::string FileDownload::download(const std::string& folder, std::optional<std::string> proxy) {
        (void)folder;
        (void)proxy;
        throw std::logic_error("FileDownload::download is not yet implemented -- no HTTP client is wired into AGRemapCore yet");
    }

    std::tuple<std::string, bool, bool> FileDownload::get(const std::string& folder, std::optional<std::string> proxy) {
        // NOTE: matches the Python original's own "wasDownloaded = self._prevPath is None" --
        // despite the name (and despite that class's own docstring calling the 3rd tuple element
        // "whether a previous download to the file existed"), this is actually true when there was
        // NOT yet a previous download (ie. this is the first download). Nothing in this codebase's
        // current iniresources port actually reads this 3rd tuple element, so this mismatch is
        // inert for now -- preserved here (value-for-value) rather than "fixed", since there's no
        // concrete wrong-output example (unlike IniNamingTools::getModSuffixedName's confirmed bug)
        // to justify diverging from the original's real behavior.
        bool isFirstDownload = !prevPath_.has_value();

        if (!cache || isFirstDownload) {
            prevPath_ = download(folder, proxy);
            return std::make_tuple(*prevPath_, true, isFirstDownload);
        }

        std::string resolvedFilename = (std::filesystem::path(folder) / std::filesystem::path(filename).filename()).string();
        bool downloadRequired = false;

        if (*prevPath_ == resolvedFilename) {
            return std::make_tuple(resolvedFilename, downloadRequired, isFirstDownload);
        }

        std::error_code copyError;
        std::filesystem::copy_file(*prevPath_, resolvedFilename, std::filesystem::copy_options::overwrite_existing, copyError);

        if (copyError) {
            prevPath_ = download(folder, proxy);
            downloadRequired = true;
            return std::make_tuple(*prevPath_, downloadRequired, isFirstDownload);
        }

        return std::make_tuple(resolvedFilename, downloadRequired, isFirstDownload);
    }
}
