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

#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What has already been fetched during one remap, keyed by URL -- so the same file is
     pulled off the network once and COPIED everywhere else it is needed
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`FileDownload` has a cache of its own (``prevPath_``), and in the pure-Python
     original that was enough: its ``IniParseBuilder`` was a flyweight, so every ``.ini`` file
     of a given mod type shared ONE parser, one ``DownloadData``, and therefore one
     ``FileDownload`` object holding one ``_prevPath``. This port builds a parser per
     :cpp:class:`IniFile` (and the builders were de-flyweighted deliberately), so that
     per-object cache can never be hit -- a fresh :cpp:class:`FileDownload` reaches every
     download with an empty ``prevPath_``. Measured on a 36-``.ini`` XingqiuBamboo mod:
     *downloaded 36 files, copied 0 files from existing downloads*, all 36 the same URL
     :raw-html:`<br />` :raw-html:`<br />`

     So the cache moves to where the question actually belongs. "Have we already fetched this
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
             * @brief Records that 'url' now sits at 'path'
             *
             * @param url The link the file was downloaded from
             * @param path The full path the file was downloaded to
             */
            void remember(const std::string& url, const std::string& path);

        private:
            std::unordered_map<std::string, std::string> paths_;
    };


    /**
     * @brief
     @rst
     Class to handle file downloads from some server :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``FileDownload`` class (``tools/files/FileDownload.py``) --
     :cpp:func:`get`'s caching decision logic (whether to re-download, copy a cached file, or
     re-download after a failed copy) is fully ported and independently testable :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        :cpp:func:`download` is backed by `libcurl`_'s easy API (``curl_easy_*``), matching the
        Python original's use of the `requests`_ package -- entirely confined to
        ``FileDownload.cpp``, so this public header (and every other public ``AGRemapCore`` header
        that transitively includes it) stays free of any ``<curl/curl.h>`` dependency, the same
        "wrap a third-party C library without leaking it into public headers" posture this codebase
        already takes for `Z3`_ (see the Architecture doc's own section on that). Unlike `Z3`_
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

        protected:

            /**
             * @brief The previous full path to the downloaded file, if any
             */
            std::optional<std::string> prevPath_;
    };
}

#endif
