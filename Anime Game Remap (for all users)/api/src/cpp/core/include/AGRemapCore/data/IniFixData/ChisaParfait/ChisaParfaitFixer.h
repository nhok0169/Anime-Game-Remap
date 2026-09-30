#ifndef AGRemapCore_ChisaParfaitFixer_H
#define AGRemapCore_ChisaParfaitFixer_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     ChisaParfait's own ``.ini`` fixers -- the reverse of :cpp:class:`ChisaFixer`, through
     :cpp:func:`makeWWMIFixer`. What is specific to this direction, beyond the plan being the
     forward one read backwards:

     * **the target is past 256 merged bones.** Chisa's merged skeleton is 420 slots and a WWMI
       ``Blend.buf`` names a bone in 8 bits, so the fix writes WWMI's own blend remap and supplies
       skeleton buffers sized for 512 bones (:cpp:member:`WWMIFixerConfig::mergedSkeletonSlots`).
       ChisaParfait is the other way round -- 264 slots, and the bones her model weights stop at 250
       -- so her mods carry no blend remap of their own and her ``Blend.buf`` holds plain 8-bit
       merged ids
     * **her mods are 8-influence** (16 bytes a vertex) and carry no blend remap, which is the one
       shape that reaches ``wwmiBlendElements`` -- see ``wwmiBlendInfluences`` in
       :cpp:func:`makeWWMIFixer`'s source
     * **two merges**: she has EIGHT draw slots against Chisa's seven, so her frilled panel goes
       through Chisa's upper body and her right-hip prop through Chisa's lower body, each landing in
       its own generated ``.ini`` file
     * **seven of the shaders it tags are also the forward direction's**, so their ``filter_index``
       values are ITS (:cpp:member:`WWMIFixerConfig::filterIndices`) -- a ``[ShaderOverride]`` is
       keyed by shader hash across every loaded ``.ini``, and two mods fixed opposite ways would
       otherwise disagree and silently unbind each other's textures
     * **Chisa's body layout is normal / mask / diffuse at ps-t0 / t1 / t2**, where ChisaParfait's
       inserts an ``R8_UNORM`` detail map at ``ps-t2`` and puts the diffuse at ``ps-t3``. So the
       diffuse moves DOWN a register in this direction and the detail map, which Chisa's shader has
       no input for, is not bound at all
     @endrst
     */
    class ChisaParfaitFixer {
        public:
            ChisaParfaitFixer() = delete;

            /**
             * @brief ``ChisaParfait -> Chisa`` at game version ``2.8`` -- see
             *        :cpp:func:`IniFixBuilderFuncs::chisa2_8`
             */
            static IniFixBuilder::Factory chisa2_8();
    };
}

#endif
