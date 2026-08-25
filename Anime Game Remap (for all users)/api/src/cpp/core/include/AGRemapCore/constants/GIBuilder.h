#ifndef AGRemapCore_GIBuilder_H
#define AGRemapCore_GIBuilder_H

#include "AGRemapCore/model/strategies/ModType.h"


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
    };
}

#endif
