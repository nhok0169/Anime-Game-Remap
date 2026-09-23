#ifndef AGRemapCore_RegDelimitedAddMode_H
#define AGRemapCore_RegDelimitedAddMode_H

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
     How often :cpp:class:`RegDelimitedAdd` places its addition along one execution path
     :raw-html:`<br />` :raw-html:`<br />`

     Every mode places the addition **as late as possible**; they differ only in how many times a
     single path gets it
     @endrst
     */
    enum class RegDelimitedAddMode {
        /**
         * @brief
         @rst
         Once per **delimiter-free stretch** of every path -- immediately before every accepted
         delimiter, plus once at the end of a path that has none :raw-html:`<br />`
         :raw-html:`<br />`

         The right mode for an addition whose effect is per-delimiter and does not accumulate: a
         counter, a log line, a register the next delimiter consumes and clears
         @endrst
         */
        PerSegment,

        /**
         * @brief
         @rst
         Once per **path** -- at the last position that precedes *every* accepted delimiter on that
         path, which is the end of the path when it has none :raw-html:`<br />` :raw-html:`<br />`

         **The mode for re-issuing GIMI's external fix libraries** (``NNFix`` / ``ORFix``), and the
         reason this enum exists. Those command lists read the currently bound ``ps-t`` registers
         and write them back **re-slotted**, so calling one twice over the same bindings undoes it:
         ``CommandListNNFix`` -> ``CommandListReferenceNoNormal`` reads the diffuse out of ``ps-t0``
         and the light map out of ``ps-t1``, and ``CommandListLDX`` writes the light map back to
         ``ps-t0`` and the diffuse to ``ps-t1``. Run once, fixed; run twice, back where it started,
         with the light map sampled as the albedo -- **a model rendered flat green**
         :raw-html:`<br />` :raw-html:`<br />`

         :cpp:enumerator:`PerSegment` is correct only while no path draws more than once. A section
         whose draws sit in INDEPENDENT ``if`` blocks issues several in a single pass -- measured
         over one real mod library, **66 sections across 14 mod folders** do -- and every second draw
         of such a section came out unfixed (2026-09-14) :raw-html:`<br />` :raw-html:`<br />`

         What mod authors write by hand is exactly this mode: one ``run =`` at the top of the
         `section`_, after the texture registers and before anything conditional, however many
         ``drawindexed`` lines follow. The pure-Python original reaches the same placement a
         different way -- it never inserts a call at all, it renames the modder's own out of the way
         and back again, which preserves wherever they put it

         .. note::
            This mode assumes the addition is invalidated only by the **delimiter**, never by
            anything between two of them. For the fix libraries that means a `section`_ must not
            rebind its ``ps-t`` registers after drawing: measured over **33030** real
            ``TextureOverride`` `section`_\\s, **none** do -- but a Citlali mod found since does (bind,
            ``ORFix``, draw, bind again, ``ORFix``, draw), and one call per path drew its second
            binding un-reslotted. Where the mod's own calls are already the right library, keeping
            them is correct and re-issuing is not: see ``keepOwnFixCalls`` in
            ``GIMIComponentFixer.cpp`` and ``Tools/Misc/Diagnostics/unfixedDraws.py``
         @endrst
         */
        PerPath,

        /**
         * @brief
         @rst
         Once per **binding generation** of every path -- the exact rule
         :cpp:enumerator:`PerPath` approximates, and what
         :cpp:member:`RegDelimitedAdd::invalidatorRegs` exists for :raw-html:`<br />`
         :raw-html:`<br />`

         :cpp:enumerator:`PerPath` assumes the addition is invalidated only by a delimiter. The fix
         libraries are invalidated by something else as well: they re-slot the registers bound when
         they run, so **re-binding one starts a new generation** that needs its own call, and a
         second call over registers that have NOT moved undoes the first. One call per path is
         therefore too few for a `section`_ that binds twice and too many for nothing
         :raw-html:`<br />` :raw-html:`<br />`

         Real mods write both shapes. A CitlaliWhisperofStars mod binds its own slot's textures and
         draws, then binds Citlali's and draws again (2026-09-22): under :cpp:enumerator:`PerPath`
         its second draw went un-reslotted. The same mod supplies the other half -- its own
         ``run =`` over its own second binding -- which is why
         :cpp:member:`RegDelimitedAdd::coveredRegs` marks a generation the mod already serves, so
         the author's call is kept and not doubled :raw-html:`<br />` :raw-html:`<br />`

         With no :cpp:member:`RegDelimitedAdd::invalidatorRegs` this mode is exactly
         :cpp:enumerator:`PerPath`: one generation, one call
         @endrst
         */
        PerBindingGeneration
    };
}

#endif
