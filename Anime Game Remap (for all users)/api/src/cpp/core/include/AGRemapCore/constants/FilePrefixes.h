#ifndef AGRemapCore_FilePrefixes_H
#define AGRemapCore_FilePrefixes_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Prefixes this software puts on the files it renames :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A **partial** port of the pure-Python ``FilePrefixes`` enum (``constants/FilePrefixes.py``)
        -- only the member :cpp:func:`IniFile::disableIni` needs is here. The two
        ``OldBackupFilePrefix`` members belong to recognizing backups an *older* version of this
        software left behind, which nothing in C++ does yet. Same rule as :cpp:class:`IniKeywords`:
        add members as later-ported subsystems need them
     @endrst
     */
    class FilePrefixes {
        public:

            /**
             * @brief What a disabled (backed-up) file's name is prefixed with
             */
            static inline const std::string BackupFilePrefix = "RemapBKUP";
    };
}

#endif
