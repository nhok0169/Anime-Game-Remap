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


#pragma once

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Tools for rendering numbers as text :raw-html:`<br />` :raw-html:`<br />`

     Separate from :cpp:class:`StringTools`, which promises `grapheme`_ semantics that mean nothing
     for a run of ASCII digits, and from :cpp:class:`TextTools`, which is a partial port of the
     pure-Python ``TextTools`` and should keep matching it
     @endrst
     */
    class NumTools {
        public:

            /**
             * @brief
             @rst
             'value' as the shortest text that still says it, to at most 'decimals' places
             :raw-html:`<br />` :raw-html:`<br />`

             Rounded to 'decimals', then trailing zeros and a trailing ``.`` are removed, so ``1.5``
             comes out ``1.5``, ``2.0`` comes out ``2`` and ``0.30000000000000004`` comes out
             ``0.3`` :raw-html:`<br />` :raw-html:`<br />`

             For a number being written INTO a ``.ini`` file, which is why the trimming matters: the
             file is read by a person as often as by the game, and a value this library computed
             should not arrive looking like a floating-point accident. Locale-independent -- the
             separator is always ``.``
             @endrst
             *
             * @param value The number to render
             * @param decimals The most decimal places to keep. **Default**: ``4``
             *
             * @return The number as text
             */
            static std::string formatDouble(double value, int decimals = 4);
    };
}
