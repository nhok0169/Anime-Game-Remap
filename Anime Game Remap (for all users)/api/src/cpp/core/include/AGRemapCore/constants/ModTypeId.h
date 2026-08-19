#ifndef AGRemapCore_ModTypeId_H
#define AGRemapCore_ModTypeId_H

#include <optional>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The names of the different types of mods this fix will fix from or fix to :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the keys of the pure-Python ``ModTypeNames`` enum (``constants/ModTypeNames.py``)
     @endrst
     */
    enum class ModTypeId {
        /**
         * @brief Amber from GI
         */
        Amber,

        /**
         * @brief Amber Chinese version from GI
         */
        AmberCN,

        /**
         * @brief Ayaka from GI
         */
        Ayaka,

        /**
         * @brief Ayaka Fontaine skin from GI
         */
        AyakaSpringbloom,

        /**
         * @brief Arlecchino from GI
         */
        Arlecchino,

        /**
         * @brief The first phase of the Arlecchino boss from GI
         */
        ArlecchinoBoss,

        /**
         * @brief Barbara from GI
         */
        Barbara,

        /**
         * @brief Barbara summer skin from GI
         */
        BarbaraSummertime,

        /**
         * @brief Hu Tao Lantern Rite skin from GI
         */
        CherryHuTao,

        /**
         * @brief Diluc from GI
         */
        Diluc,

        /**
         * @brief Diluc Red Dead of the Night skin from GI
         */
        DilucFlamme,

        /**
         * @brief Fischl from GI
         */
        Fischl,

        /**
         * @brief Fischl summer skin from GI
         */
        FischlHighness,

        /**
         * @brief Ganyu from GI
         */
        Ganyu,

        /**
         * @brief Ganyu Lantern Rite skin from GI
         */
        GanyuTwilight,

        /**
         * @brief HuTao from GI
         */
        HuTao,

        /**
         * @brief Jean from GI
         */
        Jean,

        /**
         * @brief Jean Chinese version from GI
         */
        JeanCN,

        /**
         * @brief Jean summer skin from GI
         */
        JeanSea,

        /**
         * @brief Kaeya from GI
         */
        Kaeya,

        /**
         * @brief KaeyaSailwind from GI
         */
        KaeyaSailwind,

        /**
         * @brief Keqing from GI
         */
        Keqing,

        /**
         * @brief Keqing Lantern Rite skin from GI
         */
        KeqingOpulent,

        /**
         * @brief Kirara from GI
         */
        Kirara,

        /**
         * @brief Kirara summer skin from GI
         */
        KiraraBoots,

        /**
         * @brief Klee from GI
         */
        Klee,

        /**
         * @brief Klee summer skin from GI
         */
        KleeBlossomingStarlight,

        /**
         * @brief Lisa from GI
         */
        Lisa,

        /**
         * @brief Lisa Sumeru skin from GI
         */
        LisaStudent,

        /**
         * @brief Mona from GI
         */
        Mona,

        /**
         * @brief Mona Chinese version from GI
         */
        MonaCN,

        /**
         * @brief Nilou from GI
         */
        Nilou,

        /**
         * @brief Nilou summer skin from GI
         */
        NilouBreeze,

        /**
         * @brief Ningguang from GI
         */
        Ningguang,

        /**
         * @brief Ningguang Lantern Rite from GI
         */
        NingguangOrchid,

        /**
         * @brief Ei from GI
         */
        Raiden,

        /**
         * @brief The first phase of the Raiden Shogun boss from GI
         */
        RaidenBoss,

        /**
         * @brief Rosaria from GI
         */
        Rosaria,

        /**
         * @brief Rosaria Chinese version from GI
         */
        RosariaCN,

        /**
         * @brief Shenhe from GI
         */
        Shenhe,

        /**
         * @brief Shenhe Lantern Rite skin from GI
         */
        ShenheFrostFlower,

        /**
         * @brief Xiangling from GI
         */
        Xiangling,

        /**
         * @brief Xiangling Lantern Rite skin from GI
         */
        XianglingCheer,

        /**
         * @brief Xingqiu from GI
         */
        Xingqiu,

        /**
         * @brief Xingqiu Lantern Rite skin from GI
         */
        XingqiuBamboo
    };

    /**
     * @brief Tools for handling :cpp:enum:`ModTypeId`
     */
    class ModTypeIdTools {
        public:

            /**
             * @brief
             @rst
             Retrieves the corresponding :cpp:enum:`ModTypeId` for some integer value, checking
             that the value actually corresponds to one of :cpp:enum:`ModTypeId`'s declared values
             @endrst
             *
             * @param value The integer value to convert
             *
             * @return The corresponding :cpp:enum:`ModTypeId`, if 'value' is valid
             */
            static std::optional<ModTypeId> getEnum(int value);
    };
}

#endif
