#ifndef AGRemapCore_FileService_H
#define AGRemapCore_FileService_H

#include <string>


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
    };
}

#endif
