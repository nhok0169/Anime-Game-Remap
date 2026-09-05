#ifndef AGRemapCore_FilePrefixes_H
#define AGRemapCore_FilePrefixes_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Prefixes this software puts on the files it renames :raw-html:`<br />` :raw-html:`<br />`

     A complete port of the pure-Python ``FilePrefixes`` enum (``constants/FilePrefixes.py``)
     @endrst
     */
    class FilePrefixes {
        public:

            /**
             * @brief What a disabled (backed-up) file's name is prefixed with
             */
            static inline const std::string BackupFilePrefix = "RemapBKUP";

            /**
             * @brief
             @rst
             What version 3 of this software prefixed a disabled (backed-up) file's name with
             :raw-html:`<br />` :raw-html:`<br />`

             Only ever *recognized*, never written -- a mod folder fixed by an old version of this
             software can still hold one, and :cpp:func:`RemapService::fix` has to know not to
             treat it as a mod's own ``.ini`` file
             @endrst
             */
            static inline const std::string OldBackupFilePrefixV3 = "DISABLED_BossFixBackup_";

            /**
             * @brief
             @rst
             What version 4.3 of this software prefixed a disabled (backed-up) file's name with --
             recognized but never written, exactly as #OldBackupFilePrefixV3 is
             @endrst
             */
            static inline const std::string OldBackupFilePrefixV4_3 = "DISABLED_RemapBackup_";
    };
}

#endif
