#ifndef AGRemapCore_RemapPositionResource_H
#define AGRemapCore_RemapPositionResource_H

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

#include <functional>
#include <string>

#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniFixResource`

     Class for fixing a ``Position.buf`` file used by the overall remap process -- the position
     counterpart of :cpp:class:`RemapBlendResource`, and built the same way :raw-html:`<br />`
     :raw-html:`<br />`

     **Most remaps need nothing of the sort.** A position buffer holds where the mesh's vertices
     ARE, and remapping a mod onto another character does not move them -- of the 47 pairs in the
     pure-Python ``PositionEditorData`` table, 46 are ``None``. The exception is a pair whose two
     models were authored around different origins: Xiangling and XianglingCheer sit about 0.78
     units apart in Y, so a mod remapped between them lands in mid-air unless every vertex is
     translated to match :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The stats predicates read :cpp:member:`RemapStats::position`, not
        :cpp:member:`RemapStats::blend`. Copying the blend resource and leaving them pointing at the
        blend counts a position file as a blend, which is how a summary comes to report more
        ``Blend.buf`` files than the mod has
     @endrst
     */
    class RemapPositionResource: public RemapIniFixResource {
        public:

            /**
             * @brief Constructs a new position resource
             *
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource
             * @param fixedPath The file path to the fixed resource
             * @param edit
             @rst
             What to do to each vertex -- the same per-line filter :cpp:func:`BufFile::fix` takes,
             and the direct equivalent of the pure-Python ``BufEditor(filters = [...])``
             @endrst
             * @param type The name for the type of resource
             * @param fixFunc Custom function for fixing the resource, overriding #_fix if given
             */
            RemapPositionResource(const std::string& iniFolderPath, const std::string& srcPath,
                                   const std::string& fixedPath, BufFile::Filter edit,
                                   std::string type = "position",
                                   std::function<bool(RemapPositionResource&)> fixFunc = nullptr);

            /**
             * @brief What to do to each vertex
             */
            BufFile::Filter edit;

            /**
             * @brief Custom function for fixing the resource, overriding #_fix if set
             */
            std::function<bool(RemapPositionResource&)> fixFunc;

            bool srcEncounteredError(const RemapStats& stats) const override;
            bool srcIsFixed(const RemapStats& stats) const override;
            bool fixEncounteredError(const RemapStats& stats) const override;
            bool fixIsFixed(const RemapStats& stats) const override;

            /**
             * @brief Edits the position file -- calls #fixFunc if set, otherwise #_fix
             */
            bool fix();

        protected:

            /**
             * @brief Reads #srcPath, runs \ref edit over every vertex and writes #fixedPath
             */
            bool _fix();
    };
}

#endif
