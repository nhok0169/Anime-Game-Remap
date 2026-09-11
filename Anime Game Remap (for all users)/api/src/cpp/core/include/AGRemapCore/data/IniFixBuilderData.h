#ifndef AGRemapCore_IniFixBuilderData_H
#define AGRemapCore_IniFixBuilderData_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Defines how the :cpp:class:`IniFixBuilder` arguments for some mod are built for a
     particular game version -- the C++ counterpart to the pure-Python ``IniFixBuilderFuncs``
     class (``data/IniFixBuilderData.py``) :raw-html:`<br />` :raw-html:`<br />`

     One static method per (mod, version-it-changed-at) pair, named exactly as in the original,
     each returning the :cpp:type:`IniFixBuilder::Factory` for that pair
     :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        **Every method here is a stub except** \ref raiden6_1: they all return
        :cpp:func:`IniFixBuilder::defaultFactory`, which builds a plain
        :cpp:class:`BaseIniFixer`. The real pure-Python generators pick between concrete
        subclasses (``GIMIFixer``, ``GIMIObjRegEditFixer``, ``GIMIObjSplitFixer``, ``MultiModFixer``) and pass per-mod
        arguments, none of which have been ported to C++ yet. The methods exist now so
        that the *table* is real and version selection genuinely works -- fill them in one at a
        time as concrete strategies land, without touching :cpp:class:`IniFixBuilderData` or
        anything downstream

     .. note::
        The pure-Python original also carries shared constants and predicates such as ``TexFxRemove``,
        ``ORFixRemove``, ``IbRemapData`` and ``_isNormalMap``.
        Those exist only to build the arguments the real generators pass, so while every method
        below is a stub they would be dead code -- port them alongside the first generator that
        actually needs them
     @endrst
     */
    class IniFixBuilderFuncs {
        public:

            IniFixBuilderFuncs() = delete;

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.amber4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory amber4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.amberCN4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory amberCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayaka4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayakaSpringbloom4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.barbara4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.barbaraSummertime4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.diluc4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.dilucFlamme4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.fischl4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.fischlHighness4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ganyu4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.hutao4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory hutao4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.jean4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory jean4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.jeanCN4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory jeanCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.jeanSea4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory jeanSea4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kaeya4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kaeyaSailwind4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.keqing4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.keqingOpulent4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kirara4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.klee4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kleeBlossomingStarlight4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.lisa4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.lisaStudent4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory lisaStudent4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.mona4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory mona4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.monaCN4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory monaCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.nilou4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory nilou4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ningguang4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ningguang4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ningguangOrchid4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ningguangOrchid4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.giDefault`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory giDefault();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.rosaria4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory rosaria4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.rosariaCN4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory rosariaCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.shenhe4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory shenhe4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Xiangling onto XianglingCheer** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             A **merge** of all three of her objects onto the skin's head, across THREE ``.ini`` files, and
             the only live fix still registered at 4.0 rather than 6.1. Its head re-issues ``ORFix``, whose
             maintainers baked the GI 6.1 register swap into the library itself -- so a part already calling
             it needed no 6.1 row. See ``data/IniFixData/Xiangling/XianglingFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory xiangling4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.xingqiu4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ganyuTwilight4_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ganyuTwilight4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.shenheFrostFlower4_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.xingqiuBamboo4_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory xingqiuBamboo4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kiraraBoots4_8`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kiraraBoots4_8();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.nilouBreeze4_8`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze4_8();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kaeya5_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kaeya5_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kaeyaSailwind5_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kaeyaSailwind5_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.cherryHuTao5_3`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory cherryHuTao5_3();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.xianglingCheer5_3`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory xianglingCheer5_3();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayaka5_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayaka5_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.arlecchino5_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory arlecchino5_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.nilouBreeze5_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze5_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.lisa5_4`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory lisa5_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.jean5_5`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory jean5_5();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.jeanCN5_5`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory jeanCN5_5();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.hutao5_6`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory hutao5_6();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayaka5_6`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayaka5_6();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayakaSpringbloom5_6`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom5_6();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.amber5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory amber5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.amberCN5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory amberCN5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayaka5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayaka5_7();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Ayaka onto AyakaSpringbloom** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Three objects one-to-one, and the busiest of its batch: every object shifts up a slot to
             meet the skin, three textures are edited, and the normal map the skin reads is INVENTED
             -- a muted purple, not the flat blue. See ``data/IniFixData/Ayaka/AyakaFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory ayaka6_1();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ayakaSpringbloom5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom5_7();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **AyakaSpringbloom onto Ayaka** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             A **merge**, and the only one of its batch: two of the skin's objects land on one of
             Ayaka's, so the fix writes TWO ``.ini`` files. It also SWAPS head and body -- Ayaka's
             head is drawn from the skin's body -- and carries six texture edits, every one gated on
             what the register is bound to. See ``data/IniFixData/AyakaSpringbloom/AyakaSpringbloomFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom6_1();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.arlecchino5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory arlecchino5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.barbara5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory barbara5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.barbaraSummertime5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory barbaraSummertime5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.diluc5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory diluc5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.dilucFlamme5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory dilucFlamme5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.fischl5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory fischl5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.fischlHighness5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory fischlHighness5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ganyu5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ganyu5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.ganyuTwilight5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ganyuTwilight5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kirara5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kirara5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.kiraraBoots5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory kiraraBoots5_7();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Kirara onto KiraraBoots** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Three objects one-to-one, and the first fix to use a CONDITIONAL register shift: her
             dress moves down a slot, but only where what is bound there still looks like the
             texture being moved, so a mod whose author already hand-fixed it is left alone. See
             ``data/IniFixData/Kirara/KiraraFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory kirara6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **KiraraBoots onto Kirara** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The mirror of :cpp:func:`kirara6_1`: her HEAD shifts up rather than her dress down, and
             the fix INVENTS the normal map Kirara reads and this skin never had -- a yellow one, not
             the flat blue every other character here creates. See
             ``data/IniFixData/KiraraBoots/KiraraBootsFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory kiraraBoots6_1();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.lisa5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory lisa5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Nilou onto NilouBreeze** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Three objects one-to-one, and registered at 5.7 with **no 6.1 row** for the same reason
             :cpp:func:`xiangling4_0` has none: every object re-issues ``ORFix``, which carries the GI
             6.1 register swap. Its normal-map removal is conditional on the register's VALUE, so a mod
             whose author already hand-fixed it is left alone. See
             ``data/IniFixData/Nilou/NilouFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory nilou5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.nilouBreeze5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze5_7();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **NilouBreeze onto Nilou** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The plainest fix of its batch: three objects one-to-one, one unread register dropped from
             each, and ``NNFix`` re-issued everywhere (the template's default, so the fix does not even
             name it). See ``data/IniFixData/NilouBreeze/NilouBreezeFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze6_1();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.shenheFrostFlower5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower5_7();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.raiden6_1`` -- **not** a stub, unlike every other
             method here :raw-html:`<br />` :raw-html:`<br />`

             Builds a :cpp:class:`GIMIFixer` over the four mod objects
             :cpp:func:`IniParseBuilderFuncs::raiden6_1` classifies, doing three things:

             #. **Remaps the** ``Blend.buf``. A :cpp:class:`ResRegCollect` over the
                ``("", "blend")`` graph collects every ``vb1`` reference and hands it to a
                :cpp:class:`RemapBlendReplace`. Its ``partPredicates`` entry windows collection to
                the order indices governed by a ``hash`` naming Raiden's own ``blend_vb``, so a
                `section`_ carrying several hashes contributes only the references belonging to
                this one
             #. **Reissues the texture fixes on head/body/dress.** A :cpp:class:`GraphGroupEdit`
                first drops every ``run =`` into the external ``ORFix`` library
                (:cpp:member:`IniKeywords::ORFixPath` and :cpp:member:`IniKeywords::NNFixPath`
                both), then uses :cpp:class:`RegDelimitedAdd` to reissue ``NNFix`` immediately
                before every ``drawindexed`` and once at the end of any path that draws nothing
             #. **Hides the originals.** ``head``/``body``/``dress`` go into
                :cpp:member:`GIMIFixer::hiddenModObjs`, since this fix rewrites them in place
                rather than adding beside them. ``blend`` deliberately does not -- its `section`_
                is what the remapped ``Blend.buf`` is referenced from
             @endrst
             */
            static IniFixBuilder::Factory raiden6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.amber6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The same skeleton as \ref raiden6_1, differing where remapping onto a **CN skin**
             differs from remapping onto a boss that shares the source's geometry:

             #. **Both** the ``hash`` and the ``match_first_index`` are remapped, on every mod
                object -- the two models are genuinely different, where Raiden's boss draws the same
                geometry and only her blend weights change
             #. The shared ``drawindexed`` is removed from ``("", "ib")`` and re-issued per drawn
                object with :cpp:class:`RegFillMissing`, since each remapped object now draws its
                own geometry
             #. ``position`` and ``texcoord`` are remapped too
             @endrst
             */
            static IniFixBuilder::Factory amber6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.amberCN6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             AmberCN remapped onto Amber. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with AmberCN's own choices in
             ``data/IniFixData/AmberCN/AmberCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory amberCN6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.mona6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Mona remapped onto MonaCN. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with Mona's own choices in
             ``data/IniFixData/Mona/MonaFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory mona6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Jean onto JeanCN** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The ordinary CN-skin remap. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with Jean's own choices in
             ``data/IniFixData/Jean/JeanFixer.cpp`` :raw-html:`<br />` :raw-html:`<br />`

             Jean is the first character with **two** fixers rather than one, because her two
             targets are different shapes -- see :cpp:class:`JeanFixer`
             @endrst
             */
            static IniFixBuilder::Factory jean6_1ToJeanCN();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Jean onto JeanSea** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The same shape plus a **split**: JeanSea draws a ``dress`` Jean has no geometry for, so
             Jean's ``body`` graph is emitted once as each. See
             :cpp:member:`GIMICharFixerConfig::objSplits` and ``data/IniFixData/Jean/JeanFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory jean6_1ToJeanSea();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **JeanCN onto Jean** -- **not** a stub. The mirror of
             :cpp:func:`jean6_1ToJeanCN`, in ``data/IniFixData/JeanCN/JeanCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory jeanCN6_1ToJean();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **JeanCN onto JeanSea** -- **not** a stub. The same split as
             :cpp:func:`jean6_1ToJeanSea`, in ``data/IniFixData/JeanCN/JeanCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory jeanCN6_1ToJeanSea();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **JeanSea onto Jean** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The **merge**: JeanSea's ``body`` and ``dress`` both become Jean's ``body``, which takes
             a second generated ``.ini`` file. See :cpp:class:`JeanSeaFixer` and
             :cpp:member:`GIMICharFixerConfig::objSplits`
             @endrst
             */
            static IniFixBuilder::Factory jeanSea6_1ToJean();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **JeanSea onto JeanCN** -- **not** a stub. The same merge as
             :cpp:func:`jeanSea6_1ToJean`, in ``data/IniFixData/JeanSea/JeanSeaFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory jeanSea6_1ToJeanCN();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Ningguang onto NingguangOrchid** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The standard GIMI character shape -- see :cpp:func:`makeGIMICharFixer` -- plus a
             ``ps-t3`` strip the target does not read and the ``DarkDiffuse`` head edit. Ningguang's
             own choices are in ``data/IniFixData/Ningguang/NingguangFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory ningguang6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **NingguangOrchid onto Ningguang** -- **not** a stub. The plain
             shape, with none of Ningguang's extras; see ``data/IniFixData/NingguangOrchid/NingguangOrchidFixer.cpp``
             for why the asymmetry is real
             @endrst
             */
            static IniFixBuilder::Factory ningguangOrchid6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Ganyu onto GanyuTwilight** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The direction that **gains a normal map**, and the exact mirror of
             :cpp:func:`ganyuTwilight6_1`: Ganyu predates GI 3.x's normal maps, GanyuTwilight
             has one, so the head's registers shift **up** a slot and the fix invents the
             texture that fills the hole. See ``data/IniFixData/Ganyu/GanyuFixer.cpp`` and
             :cpp:member:`GIMICharFixerConfig::texAdds`
             @endrst
             */
            static IniFixBuilder::Factory ganyu6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **XianglingCheer onto Xiangling** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The **split** that undoes :cpp:func:`xiangling4_0`'s merge, and the direction that loses a
             normal map. See ``data/IniFixData/XianglingCheer/XianglingCheerFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory xianglingCheer6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **HuTao onto CherryHuTao** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             A **split of both objects**, two becoming four, plus a created normal map and a moved draw
             call. See ``data/IniFixData/HuTao/HuTaoFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory hutao6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **CherryHuTao onto HuTao** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The most involved fix here: a **merge** of four objects onto two, three texture edits,
             reflection sections to strip, and a register shift that belongs to only half the copies --
             see :cpp:member:`GIMICharFixerConfig::srcObjRegRemaps` and
             ``data/IniFixData/CherryHuTao/CherryHuTaoFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory cherryHuTao6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Xingqiu onto XingqiuBamboo** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             A **split** of the head alone, across the skin's ``head`` and ``dress``. See
             ``data/IniFixData/Xingqiu/XingqiuFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory xingqiu6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **XingqiuBamboo onto Xingqiu** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The **merge** back onto Xingqiu, whose head takes both the skin's head and its outer robe.
             See ``data/IniFixData/XingqiuBamboo/XingqiuBambooFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory xingqiuBamboo6_1();
            /**
             * @brief
             @rst
             The 6.1 fix remapping **Keqing onto KeqingOpulent** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             A **merge**: KeqingOpulent has no ``dress``, so Keqing's dress and head both land on the
             target's head and the fix writes a second ``.ini`` file for the loser. See
             ``data/IniFixData/Keqing/KeqingFixer.cpp`` and
             :cpp:member:`GIMICharFixerConfig::objSplits`
             @endrst
             */
            static IniFixBuilder::Factory keqing6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **KeqingOpulent onto Keqing** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The **split** that undoes :cpp:func:`keqing6_1`'s merge: one body mesh emitted twice, at
             Keqing's body and dress indices. See
             ``data/IniFixData/KeqingOpulent/KeqingOpulentFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory keqingOpulent6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Shenhe onto ShenheFrostFlower** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             A **split** of the dress alone, across the skin's ``dress`` and ``extra``, plus the
             dress's ``ps-t2`` / ``ps-t3`` shift. See ``data/IniFixData/Shenhe/ShenheFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory shenhe6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **ShenheFrostFlower onto Shenhe** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The **widest merge** here: the skin's head, body and extra all come through Shenhe's one
             body draw call, so the fix writes THREE ``.ini`` files. See
             ``data/IniFixData/ShenheFrostFlower/ShenheFrostFlowerFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **GanyuTwilight onto Ganyu** -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The direction that **loses a normal map**: GanyuTwilight is a post-GI-3.x model with one
             on ``ps-t0``, Ganyu predates it, so the head's registers shift down a slot. See
             ``data/IniFixData/GanyuTwilight/GanyuTwilightFixer.cpp`` and
             :cpp:member:`GIMICharFixerConfig::objRegRemaps`
             @endrst
             */
            static IniFixBuilder::Factory ganyuTwilight6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.monaCN6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             MonaCN remapped onto Mona. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with MonaCN's own choices in
             ``data/IniFixData/MonaCN/MonaCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory monaCN6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.rosaria6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Rosaria remapped onto RosariaCN. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with Rosaria's own choices in
             ``data/IniFixData/Rosaria/RosariaFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory rosaria6_1();

            /**
             * @brief
             @rst
             The pure-Python ``IniFixBuilderFuncs.rosariaCN6_1`` -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             RosariaCN remapped onto Rosaria. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with RosariaCN's own choices in
             ``data/IniFixData/RosariaCN/RosariaCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory rosariaCN6_1();

    };

    /**
     * @brief
     @rst
     The version-keyed table of :cpp:class:`IniFixBuilder` factories -- the C++ counterpart
     to the pure-Python ``IniFixBuilderData`` dictionary (``data/IniFixBuilderData.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     73 rows across 10 game versions (4.0, 4.4, 4.6, 4.8, 5.0, 5.3, 5.4, 5.5, 5.6, 5.7), each mapping a
     ``(version, mod name)`` pair to one :cpp:class:`IniFixBuilderFuncs` method
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Mod names come from :cpp:func:`ModTypeIdTools::getName` rather than being spelled out as
        string literals, exactly as the original's own
        ``ModTypeIdTools.getName(ModTypeId.Amber)`` keys do -- so a rename in the registry
        cannot silently desync this table from it

     .. note::
        A mod only needs a row at the version its fixer *changed*.  
        :cpp:func:`ModDictAssets::get`'s inclusive floor-match means that row keeps applying to
        every later version until a newer one supersedes it, which is why most mods appear only
        once, at 4.0
     @endrst
     */
    class IniFixBuilderData {
        public:

            IniFixBuilderData() = delete;

            /**
             * @brief
             @rst
             The shared table, lazily built on first access and reused afterwards -- the same
             lazy, build-once pattern as :cpp:func:`GlobalIniClassifiers::classifier`
             :raw-html:`<br />` :raw-html:`<br />`

             Held by ``shared_ptr`` because that is what
             :cpp:func:`IniFixBuilder::IniFixBuilder` takes -- every
             :cpp:class:`ModType` of the game shares this one table
             @endrst
             *
             * @return The shared args table
             */
            static const std::shared_ptr<const IniFixBuilder::ArgsRepo>& repo();
    };
}

#endif
