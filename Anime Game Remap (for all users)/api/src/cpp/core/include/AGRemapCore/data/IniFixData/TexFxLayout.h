#ifndef AGRemapCore_TexFxLayout_H
#define AGRemapCore_TexFxLayout_H

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

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegNewVals.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Which of `TexFx`_'s per-layout sub-commands a remapped part calls :raw-html:`<br />` :raw-html:`<br />`

     TexFx has one entry point per shader layout: ``.0`` for a part with no normal map, ``.1`` for one whose normal map
     sits at ``ps-t0`` (``T.0`` / ``T.1``, ``Transparency.0`` / ``.1``, ``TN.0`` / ``.1``, ``C.0`` / ``.1`` and the
     rest). A mod's call names its OWN character's layout, so a remap that moves a part between layouts has to move the
     call too: Lumine10's glowing parts, ``T.0`` on LumineHeaven's normal-map slots, glowed faintly until moved to
     ``T.1`` (in game, 2026-09-30). The unsuffixed names are TexFx's own aliases: ``T``, ``Transparency``, ``C`` and
     ``Component`` mean ``.0``, the Natlan families ``TN``, ``TNat``, ``TransparencyNatlan``, ``CN``, ``CNat`` and
     ``ComponentNatlan`` mean ``.1`` :raw-html:`<br />` :raw-html:`<br />`

     A call already naming the target's layout is never touched, so a part whose layout does not change keeps
     whatever its author chose
     @endrst
     */
    class TexFxLayout {
        public:
            /**
             * @brief
             @rst
             The same family's call for the given layout, or ``std::nullopt`` when ``value`` is not one of TexFx's
             per-layout sub-commands or already names that layout. Case-insensitive, as 3DMigoto matches a path
             @endrst
             *
             * @param value A ``run =`` value
             * @param normalMap Whether the part is drawn on the normal-map layout
             */
            static std::optional<std::string> retarget(const std::string& value, bool normalMap);

            /**
             * @brief
             @rst
             The register edits that move every TexFx call in a part onto ``normalMap``'s variant -- one per family,
             each rewriting only the calls :cpp:func:`retarget` would
             @endrst
             */
            static std::vector<std::unique_ptr<RegNewVals<>>> switches(bool normalMap);
    };
}

#endif
