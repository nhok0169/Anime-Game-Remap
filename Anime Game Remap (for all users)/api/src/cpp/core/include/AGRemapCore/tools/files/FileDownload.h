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
        :cpp:func:`download` itself -- the actual network request -- is **not yet implemented**.
        Porting a real HTTP client (matching the Python original's use of the `requests`_ package)
        is a separate concern from this ``iniresources`` path/data-model port and hasn't happened
        yet; #download throws ``std::logic_error`` for now. It's declared ``virtual`` specifically
        so a subclass (eg. a test double, or a future real implementation) can override it without
        needing to touch #get's own caching logic at all
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
             Downloads the required file :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                Not yet implemented -- see this class's own doc comment
             @endrst
             *
             * @param folder The folder to store the downloaded file
             * @param proxy The link to the proxy server used for any internet network access, if any
             *
             * @return The full path to the downloaded file
             *
             * @throws std::logic_error Always, until a real implementation is wired in
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
