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

#ifndef AGRemapCore_RemapFileResource_H
#define AGRemapCore_RemapFileResource_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`AGRemapCore::RemapIniResource`

     Class for adding a brand new file, of contents the fix already knows, used by the overall remap
     process -- eg. the source of a ``CustomShader`` the fix draws a part with, or the constant buffer
     that shader reads :raw-html:`<br />` :raw-html:`<br />`

     The non-texture counterpart to :cpp:class:`AGRemapCore::RemapTexAddResource`: an add has only the
     one path, which is both what it writes and what it is counted under in
     :cpp:member:`RemapStats::fileAdd`

     .. note::
        #_fix returns ``true`` on success
     @endrst
     */
    class RemapFileAddResource: public RemapIniResource {
        public:
            /**
             * @param iniFolderPath The folder of the ``.ini`` file that references the file
             * @param srcPath The path of the file to write
             * @param bytes The file's contents
             * @param type The name of the type of resource. **Default**: ``"resourceRemapFileAdd"``
             * @param fixFunc A replacement for #_fix, or an empty ``std::function`` for the default
             */
            RemapFileAddResource(const std::string& iniFolderPath, const std::string& srcPath, std::vector<std::uint8_t> bytes,
                                 std::string type = "resourceRemapFileAdd", std::function<bool(RemapFileAddResource&)> fixFunc = nullptr);

            /**
             * @brief The contents written to the file
             */
            std::vector<std::uint8_t> bytes;

            /**
             * @brief A replacement for #_fix, or an empty ``std::function`` for the default
             */
            std::function<bool(RemapFileAddResource&)> fixFunc;

            bool srcEncounteredError(const RemapStats& stats) const override;
            bool srcIsFixed(const RemapStats& stats) const override;
            bool fixEncounteredError(const RemapStats& stats) const override;
            bool fixIsFixed(const RemapStats& stats) const override;
            bool fixExists(const RemapStats& stats) const override;

            /**
             * @brief Writes the file -- through #fixFunc when one is given
             *
             * @return Whether the file was written
             */
            bool fix();

        protected:
            /**
             * @brief Writes #bytes to the resource's path, creating its folder when it is missing
             *
             * @return Whether the whole file was written
             */
            virtual bool _fix();
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`AGRemapCore::RemapFileAddResource`

     A mod's buffer rewritten in another layout -- see :cpp:func:`BufFile::restride` -- and written
     to the fixed path :raw-html:`<br />` :raw-html:`<br />`

     The source is read when the resource is FIXED, not when it is built, so a buffer the run itself
     writes or downloads first is read as it ends up. Like every add, it is counted under
     :cpp:member:`RemapStats::fileAdd`, by the path it writes
     @endrst
     */
    class RemapBufRestrideResource: public RemapFileAddResource {
        public:
            /**
             * @param iniFolderPath The folder of the ``.ini`` file that references the buffer
             * @param fromPath The mod's buffer to read
             * @param fixedPath The path the rewritten buffer is written to
             * @param fromElements The layout 'fromPath' is in, in byte order
             * @param toElements The layout to write, in byte order
             * @param fillFrom See :cpp:func:`BufFile::restride`. **Default**: none
             * @param type The name of the type of resource. **Default**: ``"resourceRemapFileAdd"``
             */
            RemapBufRestrideResource(const std::string& iniFolderPath, const std::string& fromPath, const std::string& fixedPath,
                                     std::vector<std::unique_ptr<BufElementType>> fromElements,
                                     std::vector<std::unique_ptr<BufElementType>> toElements,
                                     std::unordered_map<std::string, std::string> fillFrom = {},
                                     std::string type = "resourceRemapFileAdd");

            /**
             * @brief The mod's buffer to read, as an absolute path
             */
            std::string fromPath;

            /**
             * @brief The layout #fromPath is in
             */
            std::vector<std::unique_ptr<BufElementType>> fromElements;

            /**
             * @brief The layout to write
             */
            std::vector<std::unique_ptr<BufElementType>> toElements;

            /**
             * @brief See :cpp:func:`BufFile::restride`
             */
            std::unordered_map<std::string, std::string> fillFrom;

        protected:
            /**
             * @brief Reads #fromPath, rewrites it in #toElements and writes the result
             *
             * @return Whether the whole file was written. ``false`` when #fromPath cannot be read in
             *      #fromElements
             */
            bool _fix() override;
    };
}

#endif
