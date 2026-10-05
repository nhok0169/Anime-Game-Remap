#ifndef AGRemapCore_FileDownload_H
#define AGRemapCore_FileDownload_H

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

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What has already been fetched during one remap, keyed by URL -- so the same file is
     pulled off the network once and COPIED everywhere else it is needed
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`FileDownload` has a cache of its own (``prevPath_``), but it only helps when the
     same :cpp:class:`FileDownload` object is asked twice. Every :cpp:class:`IniFile` gets a
     parser of its own, so that per-object cache is never hit across ``.ini`` files -- a fresh
     :cpp:class:`FileDownload` reaches every download with an empty ``prevPath_``, and a mod of 36
     ``.ini`` files all needing the same file would download it 36 times :raw-html:`<br />`
     :raw-html:`<br />`

     So the cache lives where the question actually belongs. "Have we already fetched this
     URL?" is a property of the RUN, not of a parser strategy -- :cpp:class:`RemapService`
     owns one of these and hands it to each download as it goes, exactly as it hands over the
     logger :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Not synchronized, and deliberately so -- one lives per :cpp:class:`RemapService`, which
        walks its folders sequentially. Sharing one across threads needs a lock added here
        first :raw-html:`<br />` :raw-html:`<br />`

        Nothing here checks that a remembered file is still ON DISK. It does not need to:
        :cpp:func:`FileDownload::get` copies from it and falls back to a real download if that
        copy fails, so a remembered path that has since been deleted costs one wasted
        ``copy_file`` and self-heals
     @endrst
     */
    class DownloadCache {
        public:

            /**
             * @brief Where 'url' was last fetched to, if it has been fetched at all
             *
             * @param url The link the file was downloaded from
             */
            std::optional<std::string> pathOf(const std::string& url) const;

            /**
             * @brief Records that 'url' now sits at 'path' -- and that it is no longer failing
             *
             * @param url The link the file was downloaded from
             * @param path The full path the file was downloaded to
             */
            void remember(const std::string& url, const std::string& path);

            /**
             * @brief
             @rst
             Records that 'url' has already been asked for, as many times as it was going to be,
             and did not come back :raw-html:`<br />` :raw-html:`<br />`

             So that the NEXT resource wanting the same file does not repeat the whole retry
             cycle. A 36-``.ini`` mod pointed at a dead url would otherwise spend minutes of pure
             waiting to reach the answer it already had
             @endrst
             *
             * @param url The link that failed
             */
            void markFailed(const std::string& url);

            /**
             * @brief Whether 'url' has already used up its attempts during this run -- see #markFailed
             *
             * @param url The link to ask about
             */
            bool hasFailed(const std::string& url) const;

            /**
             * @brief
             @rst
             Records that the server 'url' names could not be reached at all, after every retry
             :raw-html:`<br />` :raw-html:`<br />`

             #markFailed covers the same file being asked for again; this covers every OTHER file
             on the same host. Without it a machine that is offline pays the full back-off of
             :cpp:member:`FileDownload::maxAttempts` once per distinct file, where once per run says
             everything there is to say. Only a failure to reach the server belongs here -- a
             ``404`` is an answer about one file, not about the host. #remember clears it, so a
             single success restores every later download's retries
             @endrst
             *
             * @param url Any link on the host that could not be reached
             */
            void markHostUnreachable(const std::string& url);

            /**
             * @brief Whether the host 'url' names has already proved unreachable during this run -- see #markHostUnreachable
             *
             * @param url The link to ask about
             */
            bool isHostUnreachable(const std::string& url) const;

        private:
            std::unordered_map<std::string, std::string> paths_;
            std::unordered_set<std::string> failed_;
            std::unordered_set<std::string> unreachableHosts_;
    };


    /**
     * @brief
     @rst
     Class to handle file downloads from some server :raw-html:`<br />` :raw-html:`<br />`

     :cpp:func:`get`'s caching decision logic (whether to re-download, copy a cached file, or
     re-download after a failed copy) is independently testable :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        :cpp:func:`download` is backed by `libcurl`_'s easy API (``curl_easy_*``), entirely
        confined to ``FileDownload.cpp``, so this public header (and every other public
        ``AGRemapCore`` header that transitively includes it) stays free of any ``<curl/curl.h>``
        dependency, the same "wrap a third-party C library without leaking it into public headers"
        posture this codebase takes for `Z3`_. Unlike `Z3`_
        though, no persistent per-instance state needs wrapping here -- a download is a single,
        self-contained ``curl_easy_init``/``curl_easy_perform``/``curl_easy_cleanup`` sequence local
        to one #download call, so no pimpl is needed at all, just keeping the ``#include`` itself
        out of the header. #download is declared ``virtual`` regardless, so a subclass (eg. a test
        double) can still override it without needing to touch #get's own caching logic
     @endrst
     */
    class FileDownload {
        public:

            /**
             * @brief Constructs a new file download
             *
             * @param url The link to the file download
             * @param filename The base name of the file (with extension)
             * @param cache Whether to copy the previously-downloaded file if possible, instead of downloading another copy
             */
            explicit FileDownload(std::string url, std::string filename, bool cache = true);

            virtual ~FileDownload() = default;

            /**
             * @brief The link to the file download
             */
            std::string url;

            /**
             * @brief The base name of the file (with extension)
             */
            std::string filename;

            /**
             * @brief Whether to copy the previously-downloaded file if possible, instead of downloading another copy
             */
            bool cache;

            /**
             * @brief
             @rst
             How many times #download asks for the file in total before giving up -- ``1`` to
             never retry :raw-html:`<br />` :raw-html:`<br />`

             Only failures worth asking again about are retried. ``Could not resolve host`` is a
             DNS hiccup and the request stands a real chance next time; a ``404`` is an answer,
             and asking three times just prints the same thing three times. #download decides
             which is which from the `libcurl`_ result -- and, for an HTTP error, from the status
             code, since ``503`` and ``404`` arrive as the same ``CURLE_HTTP_RETURNED_ERROR``
             :raw-html:`<br />` :raw-html:`<br />`

             The default spreads its attempts over about 23 seconds (see #retryDelay), because a
             failed name lookup on a home connection routinely lasts longer than a few seconds
             @endrst
             */
            int maxAttempts = 6;

            /**
             * @brief
             @rst
             How long to wait before the FIRST retry, DOUBLED for each one after it, up to
             #maxRetryDelay :raw-html:`<br />` :raw-html:`<br />`

             Backing off rather than hammering: whatever was briefly wrong (a resolver that has
             not come back, a rate limit) is more likely to be over after a second than after no
             time at all. With the defaults a file that never comes back costs 1s + 2s + 4s + 8s
             + 8s of waiting on top of its six attempts
             @endrst
             */
            std::chrono::milliseconds retryDelay{1000};

            /**
             * @brief The longest #download waits between two attempts, however many came before
             */
            std::chrono::milliseconds maxRetryDelay{8000};

            /**
             * @brief
             @rst
             Called just before each retry, if set -- with the attempt that just failed, how many
             there are in total, why it failed, and how long #download is about to wait
             :raw-html:`<br />` :raw-html:`<br />`

             A hook rather than a logger, because this class has no view and should not grow one:
             :cpp:class:`RemapIniDownload` owns the :cpp:class:`BaseLogger` and wires this up in
             its own ``fix``. Left unset a retry is silent, which is only right for a caller that
             has nowhere to say it
             @endrst
             */
            std::function<void(int attempt, int attempts, const std::string& reason,
                                std::chrono::milliseconds wait)> onRetry;

            /**
             * @brief
             @rst
             Downloads the required file via `libcurl`_ -- see this class's own doc comment
             @endrst
             *
             * @param folder The folder to store the downloaded file (created if it doesn't already exist)
             * @param proxy The link to the proxy server used for any internet network access, if any
             *
             * @return The full path to the downloaded file
             *
             * @throws std::runtime_error if the download fails for any reason (curl init failure,
             *      transfer error, non-2xx HTTP status, or the destination file couldn't be opened)
             */
            virtual std::string download(const std::string& folder, std::optional<std::string> proxy = std::nullopt);

            /**
             * @brief
             @rst
             Retrieves the required file -- either from #download, or (if #cache is ``true`` and a
             previous download already exists) by copying the previously-downloaded file instead
             @endrst
             *
             * @param folder The folder to store the downloaded file
             * @param proxy The link to the proxy server used for any internet network access, if any
             * @param sharedCache The run's own cache of what has already been fetched, if any -- see \ref DownloadCache
             *
             * @return A tuple containing, in order: the path to the downloaded file; whether a
             *      download actually occurred; whether a previous download to the file already
             *      existed before this call
             */
            std::tuple<std::string, bool, bool> get(const std::string& folder, std::optional<std::string> proxy = std::nullopt,
                                                     DownloadCache* sharedCache = nullptr);

            /**
             * @brief
             @rst
             The file :cpp:func:`get` would COPY from instead of downloading, or nothing if it
             would have to go to the network :raw-html:`<br />` :raw-html:`<br />`

             Public so a caller can say which of the two is about to happen BEFORE it happens --
             the reason :cpp:func:`RemapIniDownload::fix` can name the right one of
             "Downloading" / "Copying download" in a line it prints before the work rather than
             after it
             @endrst
             *
             * @param sharedCache The run's own cache of what has already been fetched, if any
             */
            std::optional<std::string> cachedPath(const DownloadCache* sharedCache = nullptr) const;

            /**
             * @brief
             @rst
             Whether the last #download to throw gave up on a failure worth asking again about
             -- the server could not be reached -- rather than on an answer such as a ``404``
             :raw-html:`<br />` :raw-html:`<br />`

             Reset by every #get and #download, so it describes only the most recent request.
             What :cpp:func:`DownloadCache::markHostUnreachable` is decided by
             @endrst
             */
            bool lastFailureWasTransient() const;

        protected:

            /**
             * @brief The previous full path to the downloaded file, if any
             */
            std::optional<std::string> prevPath_;

            /**
             * @brief See #lastFailureWasTransient
             */
            bool lastFailureTransient_ = false;
    };
}

#endif
