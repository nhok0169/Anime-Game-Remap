#ifndef AGRemapCore_FileTypes_H
#define AGRemapCore_FileTypes_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Display names for the different types of files the software encounters :raw-html:`<br />`
     :raw-html:`<br />`

     A port of the pure-Python ``FileTypes`` enum (``constants/FileTypes.py``). These are **prose**,
     not matching patterns -- the only thing that reads them is a message being written for a
     person (:cpp:func:`RemapService::reportSummary` and friends), which is why ``"*.ini file"``
     carries a ``*`` that no file name ever does. Anything deciding what a file *is* uses
     :cpp:class:`FileExt`/:cpp:class:`FilePrefixes`/:cpp:class:`FileSuffixes` instead
     @endrst
     */
    class FileTypes {
        public:

            /**
             * @brief Default file type
             */
            static inline const std::string Default = "file";

            /**
             * @brief Initialization files
             */
            static inline const std::string Ini = "*.ini file";

            /**
             * @brief Blend.buf files
             */
            static inline const std::string Blend = "Blend.buf";

            /**
             * @brief Position.buf files
             */
            static inline const std::string Position = "Position.buf";

            /**
             * @brief Texture .dds files
             */
            static inline const std::string Texture = "*.dds";

            /**
             * @brief RemapBlend.buf files created by this fix
             */
            static inline const std::string RemapBlend = "RemapBlend.buf";

            /**
             * @brief RemapPosition.buf files created by this fix
             */
            static inline const std::string RemapPosition = "RemapPosition.buf";

            /**
             * @brief The log file
             */
            static inline const std::string Log = "RemapFixLog.txt";

            /**
             * @brief RemapTex.dds files created by this fix
             */
            static inline const std::string RemapTexture = "RemapTex.dds";

            /**
             * @brief RemapDL download files created by this fix
             */
            static inline const std::string RemapDownload = "RemapDL download";

            /**
             * @brief Texcoord.buf files
             */
            static inline const std::string Texcoord = "Texcoord.buf";

            /**
             * @brief RemapTexcoord.buf files created by this fix
             */
            static inline const std::string RemapTexcoord = "RemapTexcoord.buf";

            /**
             * @brief
             @rst
             Any other ``.buf`` file created by this fix -- the catch-all for the ``.buf`` half of
             :cpp:func:`RemapIniRemover::classifyResource`
             @endrst
             */
            static inline const std::string RemapBuf = "Remap*.buf";

            /**
             * @brief
             @rst
             A file of no recognized kind created by this fix -- the final catch-all of
             :cpp:func:`RemapIniRemover::classifyResource`
             @endrst
             */
            static inline const std::string RemapOther = "Remap file";
    };
}

#endif
