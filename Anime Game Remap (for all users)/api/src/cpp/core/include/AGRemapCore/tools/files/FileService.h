#ifndef AGRemapCore_FileService_H
#define AGRemapCore_FileService_H

#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tools for handling with files and folders :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a **partial** port of the pure-Python ``FileService`` class
        (``tools/files/FileService.py``) -- only #absPathOfRelPath is included so far, since it's
        the one method the ``iniresources`` model classes need. Add more methods as later-ported
        subsystems need them
     @endrst
     */
    class FileService {
        public:

            /**
             * @brief
             @rst
             Retrieves the absolute path of a (possibly relative) file path with respect to a
             certain folder :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike the pure-Python original (which always normalizes through Windows-style
                ``ntpath`` rules first, then swaps in the host OS's separator), this uses
                ``std::filesystem`` directly and returns the host platform's own native separator
                style throughout -- matching this codebase's existing precedent (see
                :cpp:class:`IniNamingTools`'s own path-joining methods) rather than replicating
                Python's Windows-first normalization quirk
             @endrst
             *
             * @param dstPath The target file path to resolve
             * @param relFolder The folder 'dstPath' is relative to, if it isn't already absolute
             *
             * @return The absolute path for 'dstPath'
             */
            static std::string absPathOfRelPath(const std::string& dstPath, const std::string& relFolder);

            /**
             * @brief
             @rst
             The folder the software was started from :raw-html:`<br />` :raw-html:`<br />`

             Ports the pure-Python ``FilePathConsts.DefaultPath``
             (``constants/FilePathConsts.py``), including its *when* -- that constant is the
             process's working directory as it stood when the package was first imported, **not**
             whatever the working directory happens to be at the moment it's read. So this is
             captured once, on the first call, and every later call gets that same answer even if
             something has since called ``chdir``
             @endrst
             *
             * @return The folder the software was started from
             */
            static const std::string& defaultPath();

            /**
             * @brief
             @rst
             Normalizes a string containing some sort of file path :raw-html:`<br />`
             :raw-html:`<br />`

             Carries the same caveat as :cpp:func:`absPathOfRelPath`: the pure-Python original
             normalizes through Windows-style ``ntpath`` rules and then swaps in the host OS's
             separator, whereas this uses ``std::filesystem`` directly and stays in the host
             platform's own native separator style throughout
             @endrst
             *
             * @param path The string containing some sort of file path
             *
             * @return The normalized file path
             */
            static std::string parseOSPath(const std::string& path);

            /**
             * @brief
             @rst
             Retrieves a file path, falling back to :cpp:func:`defaultPath` when none was given
             :raw-html:`<br />` :raw-html:`<br />`

             Ports the pure-Python ``FileService.getPath``/``FilePathConsts.getPath``
             @endrst
             *
             * @param path The file path to retrieve, if any
             *
             * @return 'path' when it has a value, and :cpp:func:`defaultPath` otherwise
             */
            static std::string getPath(const std::optional<std::string>& path);

            /**
             * @brief
             @rst
             Retrieves the files and folders contained in a certain folder :raw-html:`<br />`
             :raw-html:`<br />`

             Ports the pure-Python ``FileService.getFilesAndDirs``, including its two rather
             different shapes: non-recursively this lists only the folder's *direct* children,
             while recursively it is an ``os.walk`` -- every descendant file and every descendant
             folder, at any depth, flattened into the same two lists :raw-html:`<br />`
             :raw-html:`<br />`

             A folder that cannot be read (it doesn't exist, or the OS refuses) yields two empty
             lists rather than throwing, matching how :cpp:func:`RemapService::fix` treats an
             unreadable folder as simply having nothing to visit
             @endrst
             *
             * @param path The folder to look inside
             * @param recursive Whether to recursively check every folder underneath 'path'
             *
             * @return The files within the folder, then the folders within it
             */
            static std::pair<std::vector<std::string>, std::vector<std::string>> getFilesAndDirs(const std::string& path,
                                                                                                bool recursive = false);

            /**
             * @brief
             @rst
             Tries to get the path of a file/folder relative to another folder :raw-html:`<br />`
             :raw-html:`<br />`

             Ports the pure-Python ``FileService.getRelPath``, including its fallback: when no
             relative path exists -- eg. the two paths sit on different mounts (a ``C:/`` drive
             file against a ``D:/`` drive folder) -- the original path is handed back untouched
             rather than an error being raised
             @endrst
             *
             * @param path The file/folder to get the relative path of
             * @param start The folder 'path' should be expressed relative to
             *
             * @return 'path' relative to 'start', or 'path' itself when that isn't possible
             */
            static std::string getRelPath(const std::string& path, const std::string& start);
    };
}

#endif
