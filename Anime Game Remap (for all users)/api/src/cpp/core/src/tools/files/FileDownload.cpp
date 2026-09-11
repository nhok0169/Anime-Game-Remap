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
#include "AGRemapCore/tools/files/FileDownload.h"

#include <curl/curl.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <thread>
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

        // How long to wait for a CONNECTION, not for the transfer. Without it libcurl's own
        // default is 300s, which is finite but makes a retry policy a lie: three attempts at an
        // unreachable host would sit there for fifteen minutes before reporting anything. This
        // bounds only the connect phase, so a genuinely slow download of a large texture is
        // untouched -- CURLOPT_TIMEOUT, which caps the WHOLE transfer, would not be, and is
        // deliberately not set.
        const long ConnectTimeoutSeconds = 30;

        // Worth asking again about: nothing here is an answer from the server about the FILE, it
        // is the conversation failing to happen. Everything not listed (a malformed URL, an
        // unsupported protocol, a local write error) would fail identically forever.
        bool isTransient(CURLcode code) {
            switch (code) {
                case CURLE_COULDNT_RESOLVE_PROXY:
                case CURLE_COULDNT_RESOLVE_HOST:
                case CURLE_COULDNT_CONNECT:
                case CURLE_OPERATION_TIMEDOUT:
                case CURLE_PARTIAL_FILE:
                case CURLE_SEND_ERROR:
                case CURLE_RECV_ERROR:
                case CURLE_GOT_NOTHING:
                case CURLE_SSL_CONNECT_ERROR:
                    return true;
                default:
                    return false;
            }
        }

        // CURLOPT_FAILONERROR collapses every >= 400 into one CURLE_HTTP_RETURNED_ERROR, so the
        // status is the only thing separating "this file does not exist" from "come back in a
        // moment". 408 and 429 say so outright; a 5xx is the server having a bad time, not a
        // verdict on the URL.
        bool isTransientStatus(long status) {
            return status == 408 || status == 429 || status >= 500;
        }

        struct DownloadAttempt {
            bool ok = false;
            bool worthRetrying = false;
            std::string error;
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

    std::optional<std::string> DownloadCache::pathOf(const std::string& url) const {
        auto it = paths_.find(url);
        if (it == paths_.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    void DownloadCache::remember(const std::string& url, const std::string& path) {
        paths_[url] = path;

        // It came back. Whatever was wrong earlier in the run is over, so a later resource that
        // somehow still needs to fetch this gets its full complement of attempts again.
        failed_.erase(url);
    }

    void DownloadCache::markFailed(const std::string& url) {
        failed_.insert(url);
    }

    bool DownloadCache::hasFailed(const std::string& url) const {
        return failed_.count(url) != 0;
    }

    FileDownload::FileDownload(std::string url, std::string filename, bool cache):
        url(std::move(url)), filename(std::move(filename)), cache(cache) {}

    namespace {
        // ONE go at it. Split out of download() so the retry loop below reads as a loop rather
        // than as curl setup with a loop wrapped round it, and so every attempt starts from a
        // freshly truncated file and a fresh handle -- a second attempt appending to whatever a
        // half-finished first one left is exactly the kind of corruption a retry is supposed to
        // avoid.
        DownloadAttempt attemptDownload(const std::string& url, const std::string& path,
                                         const std::optional<std::string>& proxy) {
            DownloadAttempt attempt;

            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            if (!out) {
                // A local problem. Asking the server again would not create the folder.
                attempt.error = "unable to open destination file for writing: " + path;
                return attempt;
            }

            CurlEasyHandle curl;

            // libcurl's own message for THIS request rather than the generic one for the error
            // code: "Could not resolve host: github.com" instead of "Could not resolve
            // hostname". The buffer has to outlive curl_easy_perform, which it does -- it and
            // the handle are locals of the same scope, and it is read before either goes.
            char errorBuffer[CURL_ERROR_SIZE];
            errorBuffer[0] = '\0';

            curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeChunkToFile);
            curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out);
            curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
            curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer);
            curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, ConnectTimeoutSeconds);

            if (proxy.has_value()) {
                // A single CURLOPT_PROXY applies to whichever protocol the request actually uses
                // -- the same effective behavior as the Python original's {"http": proxy,
                // "https": proxy, "ftp": proxy} dict (identical proxy for all three), just
                // expressed as one option instead of a per-scheme map.
                curl_easy_setopt(curl.get(), CURLOPT_PROXY, proxy->c_str());
            }

            CURLcode result = curl_easy_perform(curl.get());
            out.close();

            if (result == CURLE_OK) {
                attempt.ok = true;
                return attempt;
            }

            std::error_code removeError;
            std::filesystem::remove(FileService::strToPath(path), removeError);  // best-effort -- don't leave a partial file behind

            attempt.error = errorBuffer[0] != '\0' ? errorBuffer : curl_easy_strerror(result);
            attempt.worthRetrying = isTransient(result);

            if (result == CURLE_HTTP_RETURNED_ERROR) {
                long status = 0;
                curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);
                attempt.worthRetrying = isTransientStatus(status);
            }

            return attempt;
        }
    }

    std::string FileDownload::download(const std::string& folder, std::optional<std::string> proxy) {
        ensureCurlGlobalInit();

        std::filesystem::create_directories(FileService::strToPath(folder));
        std::string path = FileService::pathToStr((FileService::strToPath(folder) / FileService::strToPath(filename).filename()));

        const int attempts = std::max(1, maxAttempts);
        std::chrono::milliseconds wait = retryDelay;
        DownloadAttempt attempt;

        for (int number = 1; number <= attempts; ++number) {
            attempt = attemptDownload(url, path, proxy);
            if (attempt.ok) {
                return path;
            }

            // Out of tries, or the failure was an ANSWER rather than a hiccup -- see maxAttempts.
            if (!attempt.worthRetrying || number == attempts) {
                break;
            }

            if (onRetry) {
                onRetry(number, attempts, attempt.error, wait);
            }

            std::this_thread::sleep_for(wait);
            wait *= 2;
        }

        throw std::runtime_error("FileDownload::download: request failed: " + attempt.error);
    }

    std::optional<std::string> FileDownload::cachedPath(const DownloadCache* sharedCache) const {
        if (!cache) {
            return std::nullopt;
        }

        // This object's own previous download first, then the run's. The order matters only in
        // that the local one is the original behaviour and is free; the shared one is what makes
        // the cache reachable at all now that every download gets its own FileDownload.
        if (prevPath_.has_value()) {
            return prevPath_;
        }

        if (sharedCache != nullptr) {
            return sharedCache->pathOf(url);
        }

        return std::nullopt;
    }

    std::tuple<std::string, bool, bool> FileDownload::get(const std::string& folder, std::optional<std::string> proxy,
                                                          DownloadCache* sharedCache) {
        // NOTE: matches the Python original's own "wasDownloaded = self._prevPath is None" --
        // despite the name (and despite that class's own docstring calling the 3rd tuple element
        // "whether a previous download to the file existed"), this is actually true when there was
        // NOT yet a previous download (ie. this is the first download). Nothing in this codebase's
        // current iniresources port actually reads this 3rd tuple element, so this mismatch is
        // inert for now -- preserved here (value-for-value) rather than "fixed", since there's no
        // concrete wrong-output example (unlike IniNamingTools::getModSuffixedName's confirmed bug)
        // to justify diverging from the original's real behavior.
        bool isFirstDownload = !prevPath_.has_value();
        std::optional<std::string> source = cachedPath(sharedCache);

        if (!source.has_value()) {
            prevPath_ = download(folder, proxy);
            if (sharedCache != nullptr) {
                sharedCache->remember(url, *prevPath_);
            }

            return std::make_tuple(*prevPath_, true, isFirstDownload);
        }

        std::string resolvedFilename = FileService::pathToStr((FileService::strToPath(folder) / FileService::strToPath(filename).filename()));
        bool downloadRequired = false;

        if (*source == resolvedFilename) {
            return std::make_tuple(resolvedFilename, downloadRequired, isFirstDownload);
        }

        std::error_code copyError;
        std::filesystem::copy_file(FileService::strToPath(*source), resolvedFilename, std::filesystem::copy_options::overwrite_existing, copyError);

        if (copyError) {
            // The remembered file is gone (or unreadable). Go to the network and re-remember --
            // otherwise every later use of this URL repeats the same failed copy first.
            prevPath_ = download(folder, proxy);
            downloadRequired = true;
            if (sharedCache != nullptr) {
                sharedCache->remember(url, *prevPath_);
            }

            return std::make_tuple(*prevPath_, downloadRequired, isFirstDownload);
        }

        return std::make_tuple(resolvedFilename, downloadRequired, isFirstDownload);
    }
}
