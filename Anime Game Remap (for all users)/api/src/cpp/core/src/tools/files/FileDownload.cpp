#include "AGRemapCore/tools/files/FileDownload.h"

#include <curl/curl.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>


namespace AGRemapCore {

    namespace {
        // curl_easy_init() implicitly runs curl_global_init(CURL_GLOBAL_DEFAULT) on its own if
        // nothing has called it yet -- but libcurl's own docs are explicit that curl_global_init
        // itself is NOT thread-safe to have race across threads on that first, implicit call. A
        // function-local static's initialization is guaranteed thread-safe and exactly-once (since
        // C++11), so routing the real call through here removes that race regardless of how many
        // threads might call FileDownload::download concurrently. curl_global_cleanup() is
        // deliberately never called to match it -- AGRemapCore is a library, not the process
        // owning main(), and calling it while some other unrelated part of the host process might
        // still be using libcurl would be actively harmful; leaving the one-time init in place for
        // the rest of the process lifetime is the standard, safe posture for a library in this
        // position.
        void ensureCurlGlobalInit() {
            static const bool initialized = [] {
                curl_global_init(CURL_GLOBAL_DEFAULT);
                return true;
            }();
            (void)initialized;
        }

        // RAII wrapper around a CURL* easy handle, so a thrown exception (or any other early exit)
        // still runs curl_easy_cleanup instead of leaking the handle.
        class CurlEasyHandle {
            public:
                CurlEasyHandle(): handle_(curl_easy_init()) {
                    if (handle_ == nullptr) {
                        throw std::runtime_error("FileDownload::download: curl_easy_init() failed");
                    }
                }

                ~CurlEasyHandle() {
                    curl_easy_cleanup(handle_);
                }

                CurlEasyHandle(const CurlEasyHandle&) = delete;
                CurlEasyHandle& operator=(const CurlEasyHandle&) = delete;

                CURL* get() const {
                    return handle_;
                }

            private:
                CURL* handle_;
        };

        // The libcurl write callback -- called by curl_easy_perform with each received chunk of
        // the response body; 'userdata' is the std::ofstream* passed via CURLOPT_WRITEDATA below.
        // Returning anything other than 'size * nmemb' tells curl the write failed and aborts the
        // transfer (see CURLOPT_WRITEFUNCTION's own contract).
        size_t writeChunkToFile(char* data, size_t size, size_t nmemb, void* userdata) {
            std::ofstream* out = static_cast<std::ofstream*>(userdata);
            size_t byteCount = size * nmemb;

            out->write(data, static_cast<std::streamsize>(byteCount));
            return out->good() ? byteCount : 0;
        }
    }

    FileDownload::FileDownload(std::string url, std::string filename, bool cache):
        url(std::move(url)), filename(std::move(filename)), cache(cache) {}

    std::string FileDownload::download(const std::string& folder, std::optional<std::string> proxy) {
        ensureCurlGlobalInit();

        std::filesystem::create_directories(folder);
        std::string path = (std::filesystem::path(folder) / std::filesystem::path(filename).filename()).string();

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) {
            throw std::runtime_error("FileDownload::download: unable to open destination file for writing: " + path);
        }

        CurlEasyHandle curl;
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeChunkToFile);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out);
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);

        if (proxy.has_value()) {
            // A single CURLOPT_PROXY applies to whichever protocol the request actually uses --
            // the same effective behavior as the Python original's {"http": proxy, "https": proxy,
            // "ftp": proxy} dict (identical proxy for all three), just expressed as one option
            // instead of a per-scheme map.
            curl_easy_setopt(curl.get(), CURLOPT_PROXY, proxy->c_str());
        }

        CURLcode result = curl_easy_perform(curl.get());
        out.close();

        if (result != CURLE_OK) {
            std::error_code removeError;
            std::filesystem::remove(path, removeError);  // best-effort -- don't leave a partial file behind
            throw std::runtime_error(std::string("FileDownload::download: request failed: ") + curl_easy_strerror(result));
        }

        return path;
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
