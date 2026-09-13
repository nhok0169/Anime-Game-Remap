#ifndef AGRemapCore_RegFillMissingMode_H
#define AGRemapCore_RegFillMissingMode_H

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


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Different modes for handling :cpp:class:`IfContentPart`\s with missing registers
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The `Python`_-facing ``RegFillMissingMode`` is a separate, still-pure-Python ``Enum``
        (``constants/RegFillMissingMode.py``) whose members carry the same three string values. The
        binding layer maps one onto the other by that value, rather than either side being replaced
        -- this enum exists so `AGRemapCore` stays usable with no `Python`_ at all, matching what
        ``IniGraphReplaceMode`` already does for the same reason
     @endrst
     */
    enum class RegFillMissingMode {
        /**
         * @brief
         @rst
         Finds all :cpp:class:`IfContentPart`\s missing the desired register and fills those parts
         with the register
         @endrst
         */
        FillMissing,

        /**
         * @brief
         @rst
         Determines whether the caller/callee graph (:cpp:class:`IniSectionGraph`) contains an
         :cpp:class:`IfContentPart` missing the desired register, then adds the register to the
         roots of the graph to cover for the missing registers
         @endrst
         */
        TopdownCover,

        /**
         * @brief
         @rst
         Like :cpp:enumerator:`TopdownCover`, but the register is added at the **bottom** of each root
         -- a fresh last :cpp:class:`IfContentPart` -- so it runs after everything the root sets up
         :raw-html:`<br />` :raw-html:`<br />`

         The mode for a draw call. :cpp:enumerator:`FillMissing` fills the FIRST content part of a
         section that lacks the register, which is the whole section while it is one part -- and the
         wrong end once something has split it (a :cpp:class:`ResGroupCollect` splicing a collected
         register into an ``if`` block does), where the draw would then run before the ``ib`` and
         the textures it depends on (2026-09-12)
         @endrst
         */
        BottomCover
    };
}

#endif
