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
     particular game version :raw-html:`<br />` :raw-html:`<br />`

     One static method per (mod, version-it-changed-at) pair, each returning the :cpp:type:`IniFixBuilder::Factory` for that pair
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        :cpp:func:`IniFixBuilder::defaultFactory` builds a real ``GIMIFixer`` with no object
        awareness. A method that returns it is either a genuine fallback
        (:cpp:func:`IniFixBuilderFuncs::giDefault`) or a stub for a pair with no dedicated fix
        yet (:cpp:func:`IniFixBuilderFuncs::wwmiStub`). Read the method before assuming which
     @endrst
     */
    class IniFixBuilderFuncs {
        public:

            IniFixBuilderFuncs() = delete;

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Amber onto AmberCN**
             @endrst
             */
            static IniFixBuilder::Factory amber4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **AmberCN onto Amber**
             @endrst
             */
            static IniFixBuilder::Factory amberCN4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Ayaka onto AyakaSpringbloom**
             @endrst
             */
            static IniFixBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **AyakaSpringbloom onto Ayaka**
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Barbara onto BarbaraSummertime**
             @endrst
             */
            static IniFixBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **BarbaraSummertime onto Barbara**
             @endrst
             */
            static IniFixBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Diluc onto DilucFlamme**
             @endrst
             */
            static IniFixBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **DilucFlamme onto Diluc**
             @endrst
             */
            static IniFixBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Fischl onto FischlHighness**
             @endrst
             */
            static IniFixBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **FischlHighness onto Fischl**
             @endrst
             */
            static IniFixBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Ganyu onto GanyuTwilight**
             @endrst
             */
            static IniFixBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **HuTao onto CherryHuTao**
             @endrst
             */
            static IniFixBuilder::Factory hutao4_0();

            /**
             * @brief
             @rst
             The 4.0 fixes for **Jean** and **JeanCN** mods, one method per (source, target) pair:
             ``jeanCN4_0ToJeanSea``, ``jeanCN4_0ToJean``, ``jean4_0ToJeanSea`` and
             ``jean4_0ToJeanCN``
             @endrst
             */
            static IniFixBuilder::Factory jeanCN4_0ToJeanSea();
            static IniFixBuilder::Factory jeanCN4_0ToJean();
            static IniFixBuilder::Factory jean4_0ToJeanSea();
            static IniFixBuilder::Factory jean4_0ToJeanCN();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **JeanSea onto Jean** and **JeanSea onto JeanCN**
             @endrst
             */
            static IniFixBuilder::Factory jeanSea4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Kaeya onto KaeyaSailwind**
             @endrst
             */
            static IniFixBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **KaeyaSailwind onto Kaeya**
             @endrst
             */
            static IniFixBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Keqing onto KeqingOpulent**
             @endrst
             */
            static IniFixBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **KeqingOpulent onto Keqing**
             @endrst
             */
            static IniFixBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Kirara onto KiraraBoots**
             @endrst
             */
            static IniFixBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Klee onto KleeBlossomingStarlight**
             @endrst
             */
            static IniFixBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **KleeBlossomingStarlight onto Klee**
             @endrst
             */
            static IniFixBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Lisa onto LisaStudent**
             @endrst
             */
            static IniFixBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **LisaStudent onto Lisa**
             @endrst
             */
            static IniFixBuilder::Factory lisaStudent4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Mona onto MonaCN**
             @endrst
             */
            static IniFixBuilder::Factory mona4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **MonaCN onto Mona**
             @endrst
             */
            static IniFixBuilder::Factory monaCN4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Nilou onto NilouBreeze**
             @endrst
             */
            static IniFixBuilder::Factory nilou4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Ningguang onto NingguangOrchid**
             @endrst
             */
            static IniFixBuilder::Factory ningguang4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **NingguangOrchid onto Ningguang**
             @endrst
             */
            static IniFixBuilder::Factory ningguangOrchid4_0();

            /**
             * @brief
             @rst
             The fallback fix row -- returns :cpp:func:`IniFixBuilder::defaultFactory`, a plain
             GIMI fixer with no object awareness, which is genuinely the fix for the rows that use it
             @endrst
             */
            static IniFixBuilder::Factory giDefault();

            /**
             * @brief The stub a WuWa row without a WWMI fixer points at -- :cpp:func:`IniFixBuilder::defaultFactory`
             */
            static IniFixBuilder::Factory wwmiStub();

            /**
             * @brief
             @rst
             The 2.5 fix remapping **Sanhua onto SanhuaExorcist** -- the first Wuthering Waves remap,
             and the first row built by :cpp:func:`makeWWMIFixer`: a multi-component character onto
             a multi-component skin, every draw slot retargeted, the textures bound by register on the
             target's passes, the blend remapped over the WWMI layout, and a third source component
             landing on one target slot written into a further ``.ini`` file. See
             ``data/IniFixData/Sanhua/SanhuaFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory sanhuaExorcist2_5();

            /**
             * @brief
             @rst
             The fix that remaps a **SanhuaExorcist** mod onto **Sanhua** at game version ``2.5`` --
             the reverse of :cpp:func:`sanhuaExorcist2_5`, built by :cpp:func:`makeWWMIFixer` from
             ``IniFixData/SanhuaExorcist/SanhuaExorcistFixer.cpp``'s config
             @endrst
             */
            static IniFixBuilder::Factory sanhua2_5();

            /**
             * @brief
             @rst
             ``Chisa -> ChisaParfait`` at game version ``3.7``, built by :cpp:func:`makeWWMIFixer`.
             See ``data/IniFixData/Chisa/ChisaFixer.cpp``

             Filed at 3.7 rather than her release 3.5 because that is the version whose hashes it
             writes: 3.7 moved ChisaParfait's ``vb0``, so a fix built against the 3.5 bucket labels
             every remapped section with a hash the game no longer binds, and the mod renders
             nothing. The fixer itself is unchanged
             @endrst
             */
            static IniFixBuilder::Factory chisaParfait3_7();

            /**
             * @brief
             @rst
             ``Lynae -> LynaePeppermint`` at game version ``3.7``, built by :cpp:func:`makeWWMIFixer`.
             See ``data/IniFixData/Lynae/LynaeFixer.cpp``

             Filed at 3.7, the skin's version and the one whose hashes it writes. The fix reads each
             mod with the vertex group row of the skeleton numbering its own geometry is in, since her
             3.7 update renumbered her skeleton
             @endrst
             */
            static IniFixBuilder::Factory lynaePeppermint3_7();

            /**
             * @brief
             @rst
             ``LynaePeppermint -> Lynae`` at game version ``3.7``, built by :cpp:func:`makeWWMIFixer`.
             See ``data/IniFixData/LynaePeppermint/LynaePeppermintFixer.cpp``

             Filed at 3.7, the version of Lynae's live ``vb0`` and of her skeleton's numbering. The
             skin's coat and her shirt / jacket / shoes both go through Lynae's jacket slot, the second
             in a generated copy ``.ini``
             @endrst
             */
            static IniFixBuilder::Factory lynae3_7();

            /**
             * @brief
             @rst
             ``ChisaParfait -> Chisa`` at game version ``2.8``, built by :cpp:func:`makeWWMIFixer`.
             The FIRST fix onto a target past 256 merged bones, so it writes WWMI's own blend remap.
             See ``data/IniFixData/ChisaParfait/ChisaParfaitFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory chisa2_8();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Rosaria onto RosariaCN**
             @endrst
             */
            static IniFixBuilder::Factory rosaria4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **RosariaCN onto Rosaria**
             @endrst
             */
            static IniFixBuilder::Factory rosariaCN4_0();

            /**
             * @brief
             @rst
             The 4.0 fix remapping **Shenhe onto ShenheFrostFlower**
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
             The 4.0 fix remapping **Xingqiu onto XingqiuBamboo**
             @endrst
             */
            static IniFixBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             The 4.4 fix remapping **GanyuTwilight onto Ganyu**
             @endrst
             */
            static IniFixBuilder::Factory ganyuTwilight4_4();

            /**
             * @brief
             @rst
             The 4.4 fix remapping **ShenheFrostFlower onto Shenhe**
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower4_4();

            /**
             * @brief
             @rst
             The 4.4 fix remapping **XingqiuBamboo onto Xingqiu**
             @endrst
             */
            static IniFixBuilder::Factory xingqiuBamboo4_4();

            /**
             * @brief
             @rst
             The 4.8 fix remapping **KiraraBoots onto Kirara**
             @endrst
             */
            static IniFixBuilder::Factory kiraraBoots4_8();

            /**
             * @brief
             @rst
             The 4.8 fix remapping **NilouBreeze onto Nilou**
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze4_8();

            /**
             * @brief
             @rst
             The 5.0 fix remapping **Kaeya onto KaeyaSailwind**
             @endrst
             */
            static IniFixBuilder::Factory kaeya5_0();

            /**
             * @brief
             @rst
             The 5.0 fix remapping **KaeyaSailwind onto Kaeya**
             @endrst
             */
            static IniFixBuilder::Factory kaeyaSailwind5_0();

            /**
             * @brief
             @rst
             The 5.3 fix remapping **CherryHuTao onto HuTao**
             @endrst
             */
            static IniFixBuilder::Factory cherryHuTao5_3();

            /**
             * @brief
             @rst
             The 5.3 fix remapping **XianglingCheer onto Xiangling**
             @endrst
             */
            static IniFixBuilder::Factory xianglingCheer5_3();

            /**
             * @brief
             @rst
             The 5.4 fix remapping **Ayaka onto AyakaSpringbloom**
             @endrst
             */
            static IniFixBuilder::Factory ayaka5_4();

            /**
             * @brief
             @rst
             The 5.4 fix remapping **Arlecchino onto ArlecchinoBoss**
             @endrst
             */
            static IniFixBuilder::Factory arlecchino5_4();

            /**
             * @brief
             @rst
             The 5.4 fix remapping **NilouBreeze onto Nilou**
             @endrst
             */
            static IniFixBuilder::Factory nilouBreeze5_4();

            /**
             * @brief
             @rst
             The 5.4 fix remapping **Lisa onto LisaStudent**
             @endrst
             */
            static IniFixBuilder::Factory lisa5_4();

            /**
             * @brief
             @rst
             The 5.5 fixes for **Jean** and **JeanCN** mods, one method per (source, target) pair:
             ``jeanCN5_5ToJeanSea``, ``jeanCN5_5ToJean``, ``jean5_5ToJeanSea`` and
             ``jean5_5ToJeanCN``
             @endrst
             */
            static IniFixBuilder::Factory jeanCN5_5ToJeanSea();
            static IniFixBuilder::Factory jeanCN5_5ToJean();
            static IniFixBuilder::Factory jean5_5ToJeanSea();
            static IniFixBuilder::Factory jean5_5ToJeanCN();

            /**
             * @brief
             @rst
             The 5.6 fix remapping **HuTao onto CherryHuTao**
             @endrst
             */
            static IniFixBuilder::Factory hutao5_6();

            /**
             * @brief
             @rst
             The 5.6 fix remapping **Ayaka onto AyakaSpringbloom**
             @endrst
             */
            static IniFixBuilder::Factory ayaka5_6();

            /**
             * @brief
             @rst
             The 5.6 fix remapping **AyakaSpringbloom onto Ayaka**
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom5_6();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Amber onto AmberCN**
             @endrst
             */
            static IniFixBuilder::Factory amber5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **AmberCN onto Amber**
             @endrst
             */
            static IniFixBuilder::Factory amberCN5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Ayaka onto AyakaSpringbloom**
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
             The 5.7 fix remapping **AyakaSpringbloom onto Ayaka**
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
             The 5.7 fix that remaps an Arlecchino mod onto ArlecchinoBoss -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             She has no 6.1 row, so this one serves 6.1 as well --
             the same arrangement :cpp:func:`IniFixBuilderFuncs::nilou5_7` has. See
             :cpp:class:`ArlecchinoFixer`
             @endrst
             */
            static IniFixBuilder::Factory arlecchino5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Barbara onto BarbaraSummertime**
             @endrst
             */
            static IniFixBuilder::Factory barbara5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **BarbaraSummertime onto Barbara**
             @endrst
             */
            static IniFixBuilder::Factory barbaraSummertime5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Diluc onto DilucFlamme**
             @endrst
             */
            static IniFixBuilder::Factory diluc5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **DilucFlamme onto Diluc**
             @endrst
             */
            static IniFixBuilder::Factory dilucFlamme5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Fischl onto FischlHighness**
             @endrst
             */
            static IniFixBuilder::Factory fischl5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **FischlHighness onto Fischl**
             @endrst
             */
            static IniFixBuilder::Factory fischlHighness5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Ganyu onto GanyuTwilight**
             @endrst
             */
            static IniFixBuilder::Factory ganyu5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **GanyuTwilight onto Ganyu**
             @endrst
             */
            static IniFixBuilder::Factory ganyuTwilight5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **Kirara onto KiraraBoots**
             @endrst
             */
            static IniFixBuilder::Factory kirara5_7();

            /**
             * @brief
             @rst
             The 5.7 fix remapping **KiraraBoots onto Kirara**
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
             The 5.7 fix remapping **Lisa onto LisaStudent**
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
             The 5.7 fix remapping **NilouBreeze onto Nilou**
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
             The 5.7 fix remapping **ShenheFrostFlower onto Shenhe**
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower5_7();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Raiden onto RaidenBoss** :raw-html:`<br />` :raw-html:`<br />`

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
             The 6.1 fix remapping **Amber onto AmberCN** :raw-html:`<br />`
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
             The 6.1 fix remapping **AmberCN onto Amber** :raw-html:`<br />`
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
             The 6.1 fix remapping **Mona onto MonaCN** :raw-html:`<br />`
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
             The 6.1 fix that remaps a Barbara mod onto BarbaraSummertime -- see
             :cpp:class:`BarbaraFixer`
             @endrst
             */
            static IniFixBuilder::Factory barbara6_1ToBarbaraSummertime();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a Diluc mod onto DilucFlamme -- see
             :cpp:class:`DilucFixer`
             @endrst
             */
            static IniFixBuilder::Factory diluc6_1ToDilucFlamme();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a DilucFlamme mod onto Diluc -- see
             :cpp:class:`DilucFlammeFixer`
             @endrst
             */
            static IniFixBuilder::Factory dilucFlamme6_1ToDiluc();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a Fischl mod onto FischlHighness -- see
             :cpp:class:`FischlFixer`
             @endrst
             */
            static IniFixBuilder::Factory fischl6_1ToFischlHighness();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a FischlHighness mod onto Fischl -- see
             :cpp:class:`FischlHighnessFixer`
             @endrst
             */
            static IniFixBuilder::Factory fischlHighness6_1ToFischl();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a Kaeya mod onto KaeyaSailwind -- see
             :cpp:class:`KaeyaFixer`
             @endrst
             */
            static IniFixBuilder::Factory kaeya6_1ToKaeyaSailwind();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a KaeyaSailwind mod onto Kaeya -- see
             :cpp:class:`KaeyaSailwindFixer`
             @endrst
             */
            static IniFixBuilder::Factory kaeyaSailwind6_1ToKaeya();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a BarbaraSummertime mod onto Barbara -- see
             :cpp:class:`BarbaraSummertimeFixer`
             @endrst
             */
            static IniFixBuilder::Factory barbaraSummertime6_1ToBarbara();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a Klee mod onto KleeBlossomingStarlight -- see
             :cpp:class:`KleeFixer`
             @endrst
             */
            static IniFixBuilder::Factory klee6_1ToKleeBlossomingStarlight();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a KleeBlossomingStarlight mod onto Klee -- see
             :cpp:class:`KleeBlossomingStarlightFixer`
             @endrst
             */
            static IniFixBuilder::Factory kleeBlossomingStarlight6_1ToKlee();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a Lisa mod onto LisaStudent -- see
             :cpp:class:`LisaFixer`
             @endrst
             */
            static IniFixBuilder::Factory lisa6_1ToLisaStudent();

            /**
             * @brief
             @rst
             The 6.1 fix that remaps a LisaStudent mod onto Lisa -- see
             :cpp:class:`LisaStudentFixer`
             @endrst
             */
            static IniFixBuilder::Factory lisaStudent6_1ToLisa();

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
             The 6.1 fix remapping **MonaCN onto Mona** :raw-html:`<br />`
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
             The 6.1 fix remapping **Rosaria onto RosariaCN** :raw-html:`<br />`
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
             The 6.1 fix remapping **RosariaCN onto Rosaria** :raw-html:`<br />`
             :raw-html:`<br />`

             RosariaCN remapped onto Rosaria. The standard GIMI character shape -- see
             :cpp:func:`makeGIMICharFixer` -- with RosariaCN's own choices in
             ``data/IniFixData/RosariaCN/RosariaCNFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory rosariaCN6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **Yelan onto YelanTranquil's Body** -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The first remap onto a skin of SEVERAL components: one fixer per component, three
             rows, all built from one :cpp:class:`GIMIComponentFixerConfig` by
             :cpp:func:`makeGIMIComponentFixer`. See ``data/IniFixData/Yelan/YelanFixer.cpp``
             @endrst
             */
            /**
             * @brief
             @rst
             The 6.1 fix remapping **Bennett onto BennettAdventure's Body** :raw-html:`<br />`
             :raw-html:`<br />`

             The SECOND remap onto a skin of several components, after
             :cpp:func:`yelanTranquilBody6_1`. One fixer per target component, all three built from
             one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Bennett/BennettFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory bennettAdventureBody6_1();

            /**
             * @brief The 6.1 fix remapping **Bennett onto BennettAdventure's Eye** -- see :cpp:func:`bennettAdventureBody6_1`
             */
            static IniFixBuilder::Factory bennettAdventureEye6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **BennettAdventure onto Bennett** -- the inverse of
             :cpp:func:`bennettAdventureBody6_1` :raw-html:`<br />` :raw-html:`<br />`

             A skin of several components merged onto a target of one, always as ONE ``.ini`` group.
             BOTH of Bennett's objects are merged onto: his body takes her Body's two slots and his
             head takes her Bang and her Eye. See
             ``data/IniFixData/BennettAdventure/BennettAdventureFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory bennettAdventureToBennett6_1();

            /**
             * @brief
             @rst
             The 6.7 fix remapping **Citlali onto CitlaliWhisperofStars's Body** :raw-html:`<br />`
             :raw-html:`<br />`

             The THIRD remap onto a skin of several components. One fixer per target component, all
             three built from one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Citlali/CitlaliFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory citlaliWhisperofStarsBody6_7();

            /**
             * @brief The 6.7 fix remapping **Citlali onto CitlaliWhisperofStars's Bangs** -- see :cpp:func:`citlaliWhisperofStarsBody6_7`
             */
            static IniFixBuilder::Factory citlaliWhisperofStarsBangs6_7();

            /**
             * @brief The 6.7 fix remapping **Citlali onto CitlaliWhisperofStars's Eyes** -- see :cpp:func:`citlaliWhisperofStarsBody6_7`
             */
            static IniFixBuilder::Factory citlaliWhisperofStarsEyes6_7();

            /**
             * @brief
             @rst
             The 6.7 fix remapping **Charlotte onto CharlotteHurlock's Body** :raw-html:`<br />`
             :raw-html:`<br />`

             The FOURTH remap onto a skin of several components. One fixer per target component the
             mod reaches (the Body and the Eyes -- the skin's Bangs and Camera are hidden), both
             built from one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Charlotte/CharlotteFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory charlotteHurlockBody6_7();

            /**
             * @brief The 6.7 fix remapping **Charlotte onto CharlotteHurlock's Eyes** -- see :cpp:func:`charlotteHurlockBody6_7`
             */
            static IniFixBuilder::Factory charlotteHurlockEyes6_7();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **Neuvillette onto NeuvilletteMelusent's main mesh** :raw-html:`<br />`
             :raw-html:`<br />`

             A skin of FOUR components whose main mesh is unnamed. One fixer per target component, all
             four built from one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Neuvillette/NeuvilletteFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory neuvilletteMelusentMain6_3();

            /**
             * @brief The 6.3 fix remapping **Neuvillette onto NeuvilletteMelusent's Coat** -- see :cpp:func:`neuvilletteMelusentMain6_3`
             */
            static IniFixBuilder::Factory neuvilletteMelusentCoat6_3();

            /**
             * @brief The 6.3 fix remapping **Neuvillette onto NeuvilletteMelusent's Bang** -- see :cpp:func:`neuvilletteMelusentMain6_3`
             */
            static IniFixBuilder::Factory neuvilletteMelusentBang6_3();

            /**
             * @brief The 6.3 fix remapping **Neuvillette onto NeuvilletteMelusent's Eye** -- see :cpp:func:`neuvilletteMelusentMain6_3`
             */
            static IniFixBuilder::Factory neuvilletteMelusentEye6_3();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **Yaoyao onto YaoyaoBamboo's main mesh** :raw-html:`<br />`
             :raw-html:`<br />`

             A skin of THREE components whose main mesh is unnamed. One fixer per target component, all
             three built from one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Yaoyao/YaoyaoFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory yaoyaoBambooMain6_3();

            /**
             * @brief The 6.3 fix remapping **Yaoyao onto YaoyaoBamboo's Bang** -- see :cpp:func:`yaoyaoBambooMain6_3`
             */
            static IniFixBuilder::Factory yaoyaoBambooBang6_3();

            /**
             * @brief The 6.3 fix remapping **Yaoyao onto YaoyaoBamboo's Eye** -- see :cpp:func:`yaoyaoBambooMain6_3`
             */
            static IniFixBuilder::Factory yaoyaoBambooEye6_3();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **Lumine onto LumineHeaven's main mesh** :raw-html:`<br />`
             :raw-html:`<br />`

             A skin of THREE components whose main mesh is unnamed. One fixer per target component, all
             three built from one :cpp:class:`GIMIComponentFixerConfig`. See
             ``data/IniFixData/Lumine/LumineFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory lumineHeavenMain6_3();

            /**
             * @brief The 6.3 fix remapping **Lumine onto LumineHeaven's Bang** -- see :cpp:func:`lumineHeavenMain6_3`
             */
            static IniFixBuilder::Factory lumineHeavenBang6_3();

            /**
             * @brief The 6.3 fix remapping **Lumine onto LumineHeaven's Eye** -- see :cpp:func:`lumineHeavenMain6_3`
             */
            static IniFixBuilder::Factory lumineHeavenEye6_3();

            static IniFixBuilder::Factory yelanTranquilBody6_1();

            /**
             * @brief The 6.1 fix remapping **Yelan onto YelanTranquil's Bang** -- see :cpp:func:`yelanTranquilBody6_1`
             */
            static IniFixBuilder::Factory yelanTranquilBang6_1();

            /**
             * @brief The 6.1 fix remapping **Yelan onto YelanTranquil's Eye** -- see :cpp:func:`yelanTranquilBody6_1`
             */
            static IniFixBuilder::Factory yelanTranquilEye6_1();

            /**
             * @brief
             @rst
             The 6.1 fix remapping **YelanTranquil onto Yelan** -- the inverse of
             :cpp:func:`yelanTranquilBody6_1` and the first remap of a skin of SEVERAL components
             onto a target of ONE :raw-html:`<br />` :raw-html:`<br />`

             ONE row, not three: the target draws through one set of buffer hashes, so the source's
             components are merged into one ``.ini`` file over one set of buffers. See
             :cpp:func:`makeGIMIMergeFixer` and ``data/IniFixData/YelanTranquil/YelanTranquilFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory yelanTranquilToYelan6_1();

            /**
             * @brief
             @rst
             CitlaliWhisperofStars -> Citlali -- the THIRD merge of a skin of several components
             onto a target of one, and the inverse of :cpp:class:`CitlaliFixer`'s direction
             @endrst
             */
            static IniFixBuilder::Factory citlaliWhisperofStarsToCitlali6_7();

            /**
             * @brief
             @rst
             The 6.7 fix remapping **CharlotteHurlock onto Charlotte** :raw-html:`<br />`
             :raw-html:`<br />`

             The FOURTH merge of a skin of several components onto a target of one, and the
             inverse of :cpp:func:`charlotteHurlockBody6_7` / :cpp:func:`charlotteHurlockEyes6_7`.
             See ``data/IniFixData/CharlotteHurlock/CharlotteHurlockFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory charlotteHurlockToCharlotte6_7();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **NeuvilletteMelusent onto Neuvillette** :raw-html:`<br />`
             :raw-html:`<br />`

             The FIFTH merge of a skin of several components onto a target of one, and the inverse of
             :cpp:func:`neuvilletteMelusentMain6_3` and its siblings. See
             ``data/IniFixData/NeuvilletteMelusent/NeuvilletteMelusentFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory neuvilletteMelusentToNeuvillette6_3();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **YaoyaoBamboo onto Yaoyao** :raw-html:`<br />`
             :raw-html:`<br />`

             The SIXTH merge of a skin of several components onto a target of one, and the inverse of
             :cpp:func:`yaoyaoBambooMain6_3` and its siblings. See
             ``data/IniFixData/YaoyaoBamboo/YaoyaoBambooFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory yaoyaoBambooToYaoyao6_3();

            /**
             * @brief
             @rst
             The 6.3 fix remapping **LumineHeaven onto Lumine** :raw-html:`<br />`
             :raw-html:`<br />`

             The SEVENTH merge of a skin of several components onto a target of one, and the inverse of
             :cpp:func:`lumineHeavenMain6_3` and its siblings. See
             ``data/IniFixData/LumineHeaven/LumineHeavenFixer.cpp``
             @endrst
             */
            static IniFixBuilder::Factory lumineHeavenToLumine6_3();

    };

    /**
     * @brief
     @rst
     The version-keyed table of :cpp:class:`IniFixBuilder` factories :raw-html:`<br />`
     :raw-html:`<br />`

     Each row maps a ``(fromVersion, fromMod, toVersion, toMod)`` key to one
     :cpp:class:`IniFixBuilderFuncs` method
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Mod names come from :cpp:func:`ModTypeIdTools::getName` rather than being spelled out as
        string literals -- so a rename in the registry
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
