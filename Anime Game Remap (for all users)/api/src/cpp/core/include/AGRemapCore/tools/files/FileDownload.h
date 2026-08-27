#ifndef AGRemapCore_FileDownload_H
#define AGRemapCore_FileDownload_H

#include <optional>
#include <string>
#include <tuple>


namespace AGRemapCore {

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
             *
             * @return A tuple containing, in order: the path to the downloaded file; whether a
             *      download actually occurred; whether a previous download to the file already
             *      existed before this call
             */
            std::tuple<std::string, bool, bool> get(const std::string& folder, std::optional<std::string> proxy = std::nullopt);

        protected:

            /**
             * @brief The previous full path to the downloaded file, if any
             */
            std::optional<std::string> prevPath_;
    };
}

#endif
