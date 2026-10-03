#ifndef AGRemapCore_IniRemoveBuilderData_H
#define AGRemapCore_IniRemoveBuilderData_H

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

#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Defines how the :cpp:class:`IniRemoveBuilder` arguments for some mod are built for a
     particular game version :raw-html:`<br />` :raw-html:`<br />`

     One static method per mod, each returning the :cpp:type:`IniRemoveBuilder::Factory` for it
     :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        The `Python`_ package has no per-mod remover table: it uses exactly one
        ``IniRemoveBuilder``, globally, from ``constants/GlobalIniRemoveBuilders.py``, with no
        per-mod or per-version variation at all. This table exists so per-mod removers *can* be
        expressed in C++ when they are needed

     .. warning::
        **Every method here is currently the same**: they all return
        :cpp:func:`IniRemoveBuilder::defaultFactory`, which builds an :cpp:class:`RemapIniRemover`.
        That is the real remover, and it is the only one there is, exactly as on the `Python`_ side where every mod type shares one. The rows
        exist so a mod that eventually needs its *own* remover can be given one here, without
        touching :cpp:class:`IniRemoveBuilderData` or anything downstream

     .. note::
        The method names follow the ``<mod><version>`` convention the other two tables use, and
        every one currently sits at 4.0 -- not because a remover changed at 4.0, but because
        that is the baseline version those tables use for "has not changed since". A remover
        that genuinely starts differing at some later version gets a new method and a new row,
        exactly as on the parse and fix sides
     @endrst
     */
    class IniRemoveBuilderFuncs {
        public:

            IniRemoveBuilderFuncs() = delete;

            /**
             * @brief
             @rst
             The remover for the ``amber4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory amber4_0();

            /**
             * @brief
             @rst
             The remover for the ``amberCN4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory amberCN4_0();

            /**
             * @brief
             @rst
             The remover for the ``ayaka4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             The remover for the ``ayakaSpringbloom4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             The remover for the ``arlecchino4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory arlecchino4_0();

            /**
             * @brief
             @rst
             The remover for the ``barbara4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             The remover for the ``barbaraSummertime4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             The remover for the ``bennett4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory bennett4_0();

            /**
             * @brief
             @rst
             The remover for the ``bennettAdventure4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory bennettAdventure4_0();

            /**
             * @brief
             @rst
             The remover for the ``charlotte4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory charlotte4_0();

            /**
             * @brief
             @rst
             The remover for the ``charlotteHurlock4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory charlotteHurlock4_0();

            /**
             * @brief
             @rst
             The remover for the ``cherryHuTao4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory cherryHuTao4_0();

            /**
             * @brief
             @rst
             The remover for the ``citlali4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory citlali4_0();

            /**
             * @brief
             @rst
             The remover for the ``citlaliWhisperofStars4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory citlaliWhisperofStars4_0();

            /**
             * @brief
             @rst
             The remover for the ``diluc4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             The remover for the ``dilucFlamme4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             The remover for the ``fischl4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             The remover for the ``fischlHighness4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             The remover for the ``ganyu4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             The remover for the ``ganyuTwilight4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ganyuTwilight4_0();

            /**
             * @brief
             @rst
             The remover for the ``huTao4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory huTao4_0();

            /**
             * @brief
             @rst
             The remover for the ``jean4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jean4_0();

            /**
             * @brief
             @rst
             The remover for the ``jeanCN4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jeanCN4_0();

            /**
             * @brief
             @rst
             The remover for the ``jeanSea4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jeanSea4_0();

            /**
             * @brief
             @rst
             The remover for the ``kaeya4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             The remover for the ``kaeyaSailwind4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             The remover for the ``keqing4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             The remover for the ``keqingOpulent4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             The remover for the ``kirara4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             The remover for the ``kiraraBoots4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kiraraBoots4_0();

            /**
             * @brief
             @rst
             The remover for the ``klee4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             The remover for the ``kleeBlossomingStarlight4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             The remover for the ``lisa4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             The remover for the ``lisaStudent4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lisaStudent4_0();

            /**
             * @brief
             @rst
             The remover for the ``lumine4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lumine4_0();

            /**
             * @brief
             @rst
             The remover for the ``lumineHeaven4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lumineHeaven4_0();

            /**
             * @brief
             @rst
             The remover for the ``mona4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory mona4_0();

            /**
             * @brief
             @rst
             The remover for the ``monaCN4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory monaCN4_0();

            /**
             * @brief
             @rst
             The remover for the ``neuvillette4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory neuvillette4_0();

            /**
             * @brief
             @rst
             The remover for the ``neuvilletteMelusent4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory neuvilletteMelusent4_0();

            /**
             * @brief
             @rst
             The remover for the ``nilou4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory nilou4_0();

            /**
             * @brief
             @rst
             The remover for the ``nilouBreeze4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory nilouBreeze4_0();

            /**
             * @brief
             @rst
             The remover for the ``ningguang4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ningguang4_0();

            /**
             * @brief
             @rst
             The remover for the ``ningguangOrchid4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ningguangOrchid4_0();

            /**
             * @brief
             @rst
             The remover for the ``raiden4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory raiden4_0();

            /**
             * @brief
             @rst
             The remover for the ``rosaria4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory rosaria4_0();

            /**
             * @brief
             @rst
             The remover for the ``rosariaCN4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory rosariaCN4_0();

            /**
             * @brief
             @rst
             The remover for the ``shenhe4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory shenhe4_0();

            /**
             * @brief
             @rst
             The remover for the ``shenheFrostFlower4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory shenheFrostFlower4_0();

            /**
             * @brief
             @rst
             The remover for the ``xiangling4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xiangling4_0();

            /**
             * @brief
             @rst
             The remover for the ``xianglingCheer4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xianglingCheer4_0();

            /**
             * @brief
             @rst
             The remover for the ``xingqiu4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             The remover for the ``xingqiuBamboo4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xingqiuBamboo4_0();

            /**
             * @brief
             @rst
             The remover for the ``yaoyao4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory yaoyao4_0();

            /**
             * @brief
             @rst
             The remover for the ``yaoyaoBamboo4_0`` row -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory yaoyaoBamboo4_0();

            /**
             * @brief Yelan's remover -- returns :cpp:func:`IniRemoveBuilder::defaultFactory`
             */
            static IniRemoveBuilder::Factory yelan4_0();

            /**
             * @brief YelanTranquil's remover -- returns :cpp:func:`IniRemoveBuilder::defaultFactory`
             */
            static IniRemoveBuilder::Factory yelanTranquil4_0();

            /**
             * @brief The remover every WuWa row points at -- :cpp:func:`IniRemoveBuilder::defaultFactory`
             */
            static IniRemoveBuilder::Factory wwmiStub();

    };

    /**
     * @brief
     @rst
     The version-keyed table of :cpp:class:`IniRemoveBuilder` factories
     :raw-html:`<br />` :raw-html:`<br />`

     See :cpp:class:`IniRemoveBuilderFuncs`'s own warning :raw-html:`<br />` :raw-html:`<br />`

     Each row maps a
     ``(version, mod name)`` pair to one :cpp:class:`IniRemoveBuilderFuncs` method
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Mod names come from :cpp:func:`ModTypeIdTools::getName` rather than being spelled out as
        string literals, matching its sibling tables -- so a rename in the registry cannot
        silently desync this table from it

     .. note::
        A mod only needs a row at the version its remover *changed*.  
        :cpp:func:`ModDictAssets::get`'s inclusive floor-match means that row keeps applying to
        every later version until a newer one supersedes it, which is why most mods appear only
        once, at 4.0
     @endrst
     */
    class IniRemoveBuilderData {
        public:

            IniRemoveBuilderData() = delete;

            /**
             * @brief
             @rst
             The shared table, lazily built on first access and reused afterwards -- the same
             lazy, build-once pattern as :cpp:func:`GlobalIniClassifiers::classifier`
             :raw-html:`<br />` :raw-html:`<br />`

             Held by ``shared_ptr`` because that is what
             :cpp:func:`IniRemoveBuilder::IniRemoveBuilder` takes -- every
             :cpp:class:`ModType` of the game shares this one table
             @endrst
             *
             * @return The shared args table
             */
            static const std::shared_ptr<const IniRemoveBuilder::ArgsRepo>& repo();
    };
}

#endif
