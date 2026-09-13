#ifndef AGRemapCore_GIBuilder_H
#define AGRemapCore_GIBuilder_H

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

#include "AGRemapCore/model/strategies/ModType.h"
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Creates new :cpp:class:`ModType` objects for GI (Genshin Impact) mods :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``GIBuilder`` class (``constants/GIBuilder.py``), but builds the
     lighter, C++-side :cpp:class:`ModType` (id, name, and aliases only) instead of the full
     pure-Python ``ModType``
     @endrst
     */
    class GIBuilder {
        public:

            /**
             * @brief Creates the :cpp:class:`ModType` for Amber
             */
            static ModType amber();

            /**
             * @brief Creates the :cpp:class:`ModType` for AmberCN
             */
            static ModType amberCN();

            /**
             * @brief Creates the :cpp:class:`ModType` for Ayaka
             */
            static ModType ayaka();

            /**
             * @brief Creates the :cpp:class:`ModType` for AyakaSpringBloom
             */
            static ModType ayakaSpringBloom();

            /**
             * @brief Creates the :cpp:class:`ModType` for Arlecchino
             */
            static ModType arlecchino();

            /**
             * @brief Creates the :cpp:class:`ModType` for Barbara
             */
            static ModType barbara();

            /**
             * @brief Creates the :cpp:class:`ModType` for BarbaraSummerTime
             */
            static ModType barbaraSummerTime();

            /**
             * @brief Creates the :cpp:class:`ModType` for CherryHuTao
             */
            static ModType cherryHutao();

            /**
             * @brief Creates the :cpp:class:`ModType` for Diluc
             */
            static ModType diluc();

            /**
             * @brief Creates the :cpp:class:`ModType` for DilucFlamme
             */
            static ModType dilucFlamme();

            /**
             * @brief Creates the :cpp:class:`ModType` for Fischl
             */
            static ModType fischl();

            /**
             * @brief Creates the :cpp:class:`ModType` for FischlHighness
             */
            static ModType fischlHighness();

            /**
             * @brief Creates the :cpp:class:`ModType` for Ganyu
             */
            static ModType ganyu();

            /**
             * @brief Creates the :cpp:class:`ModType` for GanyuTwilight
             */
            static ModType ganyuTwilight();

            /**
             * @brief Creates the :cpp:class:`ModType` for HuTao
             */
            static ModType huTao();

            /**
             * @brief Creates the :cpp:class:`ModType` for Jean
             */
            static ModType jean();

            /**
             * @brief Creates the :cpp:class:`ModType` for JeanCN
             */
            static ModType jeanCN();

            /**
             * @brief Creates the :cpp:class:`ModType` for JeanSea
             */
            static ModType jeanSea();

            /**
             * @brief Creates the :cpp:class:`ModType` for Kaeya
             */
            static ModType kaeya();

            /**
             * @brief Creates the :cpp:class:`ModType` for KaeyaSailwind
             */
            static ModType kaeyaSailwind();

            /**
             * @brief Creates the :cpp:class:`ModType` for Keqing
             */
            static ModType keqing();

            /**
             * @brief Creates the :cpp:class:`ModType` for KeqingOpulent
             */
            static ModType keqingOpulent();

            /**
             * @brief Creates the :cpp:class:`ModType` for Kirara
             */
            static ModType kirara();

            /**
             * @brief Creates the :cpp:class:`ModType` for KiraraBoots
             */
            static ModType kiraraBoots();

            /**
             * @brief Creates the :cpp:class:`ModType` for Klee
             */
            static ModType klee();

            /**
             * @brief Creates the :cpp:class:`ModType` for KleeBlossomingStarlight
             */
            static ModType kleeBlossomingStarlight();

            /**
             * @brief Creates the :cpp:class:`ModType` for Lisa
             */
            static ModType lisa();

            /**
             * @brief Creates the :cpp:class:`ModType` for LisaStudent
             */
            static ModType lisaStudent();

            /**
             * @brief Creates the :cpp:class:`ModType` for Mona
             */
            static ModType mona();

            /**
             * @brief Creates the :cpp:class:`ModType` for MonaCN
             */
            static ModType monaCN();

            /**
             * @brief Creates the :cpp:class:`ModType` for Nilou
             */
            static ModType nilou();

            /**
             * @brief Creates the :cpp:class:`ModType` for NilouBreeze
             */
            static ModType nilouBreeze();

            /**
             * @brief Creates the :cpp:class:`ModType` for Ningguang
             */
            static ModType ningguang();

            /**
             * @brief Creates the :cpp:class:`ModType` for Ningguang
             */
            static ModType ningguangOrchid();

            /**
             * @brief Creates the :cpp:class:`ModType` for Ei
             */
            static ModType raiden();

            /**
             * @brief Creates the :cpp:class:`ModType` for Rosaria
             */
            static ModType rosaria();

            /**
             * @brief Creates the :cpp:class:`ModType` for RosariaCN
             */
            static ModType rosariaCN();

            /**
             * @brief Creates the :cpp:class:`ModType` for Shenhe
             */
            static ModType shenhe();

            /**
             * @brief Creates the :cpp:class:`ModType` for ShenheFrostFlower
             */
            static ModType shenheFrostFlower();

            /**
             * @brief Creates the :cpp:class:`ModType` for Xiangling
             */
            static ModType xiangling();

            /**
             * @brief Creates the :cpp:class:`ModType` for XianglingCheer
             */
            static ModType xianglingCheer();

            /**
             * @brief Creates the :cpp:class:`ModType` for Xingqiu
             */
            static ModType xingqiu();

            /**
             * @brief Creates the :cpp:class:`ModType` for XingqiuBamboo
             */
            static ModType xingqiuBamboo();

            /**
             * @brief Creates the :cpp:class:`ModType` for Yelan
             */
            static ModType yelan();

            /**
             * @brief
             @rst
             Creates the :cpp:class:`ModType` for YelanTranquil -- the skin, a character of THREE
             components. What a ``.ini`` file built on the skin classifies as. What a Yelan mod is
             remapped ONTO is her three component ids (:cpp:enumerator:`ModTypeId::YelanTranquilBody`
             and siblings), which -- like the boss ids -- are targets only and have no factory here:
             a registered mod type with no keyword has no way to be classified, and the classifier
             population is not built to hold one
             @endrst
             */
            static ModType yelanTranquil();

            /**
             * @brief
             @rst
             Every :cpp:class:`ModType` this builder knows how to make, freshly built on each call
             :raw-html:`<br />` :raw-html:`<br />`

             The counterpart to the pure-Python ``ModTypes.getAll()`` (``constants/ModTypes.py``).
             Note this is *not* the same set as :cpp:enum:`ModTypeId`'s members: the two boss ids
             (``RaidenBoss``, ``ArlecchinoBoss``) are only ever remap targets and have no factory
             @endrst
             *
             * @return All 43 GI mod types
             */
            static std::vector<ModType> all();
    };
}

#endif
