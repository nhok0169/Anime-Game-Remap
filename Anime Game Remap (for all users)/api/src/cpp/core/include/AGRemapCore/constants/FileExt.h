#ifndef AGRemapCore_FileExt_H
#define AGRemapCore_FileExt_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Different file extensions for files :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``FileExt`` enum (``constants/FileExt.py``)
     @endrst
     */
    class FileExt {
        public:

            /**
             * @brief Initialization file extension
             */
            static inline const std::string Ini = ".ini";

            /**
             * @brief Text file extension
             */
            static inline const std::string Txt = ".txt";

            /**
             * @brief Buffer file extension
             */
            static inline const std::string Buf = ".buf";

            /**
             * @brief
             @rst
             `Direct Draw Surface`_ file extension
             @endrst
             */
            static inline const std::string DDS = ".dds";
    };
}

#endif
