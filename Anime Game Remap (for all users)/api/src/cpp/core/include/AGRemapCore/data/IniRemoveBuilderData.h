#ifndef AGRemapCore_IniRemoveBuilderData_H
#define AGRemapCore_IniRemoveBuilderData_H

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
        **This class has no pure-Python counterpart**, unlike its
        :cpp:class:`IniParseBuilderFuncs`/:cpp:class:`IniFixBuilderFuncs` siblings. There is no
        ``IniRemoveBuilderData.py`` and no ``IniRemoveBuilderArgs.py``; the whole Python package
        constructs exactly one ``IniRemoveBuilder(IniRemover)``, globally, in
        ``constants/GlobalIniRemoveBuilders.py``, with no per-mod or per-version variation at
        all. This table exists so per-mod removers *can* be expressed in C++ when they are
        needed -- do not treat its method names as mirroring anything upstream

     .. warning::
        **Every method here is currently a stub**: they all return
        :cpp:func:`IniRemoveBuilder::defaultFactory`, which builds a plain
        :cpp:class:`BaseIniRemover`. No concrete C++ remover (the equivalent of the pure-Python
        ``IniRemover``) has been ported yet, so today every row resolves the same way. Fill them
        in one at a time as concrete removers land, without touching
        :cpp:class:`IniRemoveBuilderData` or anything downstream

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
             Stub for the amber4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory amber4_0();

            /**
             * @brief
             @rst
             Stub for the amberCN4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory amberCN4_0();

            /**
             * @brief
             @rst
             Stub for the ayaka4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             Stub for the ayakaSpringbloom4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             Stub for the arlecchino4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory arlecchino4_0();

            /**
             * @brief
             @rst
             Stub for the barbara4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             Stub for the barbaraSummertime4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             Stub for the cherryHuTao4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory cherryHuTao4_0();

            /**
             * @brief
             @rst
             Stub for the diluc4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             Stub for the dilucFlamme4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             Stub for the fischl4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             Stub for the fischlHighness4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             Stub for the ganyu4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             Stub for the ganyuTwilight4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ganyuTwilight4_0();

            /**
             * @brief
             @rst
             Stub for the huTao4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory huTao4_0();

            /**
             * @brief
             @rst
             Stub for the jean4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jean4_0();

            /**
             * @brief
             @rst
             Stub for the jeanCN4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jeanCN4_0();

            /**
             * @brief
             @rst
             Stub for the jeanSea4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory jeanSea4_0();

            /**
             * @brief
             @rst
             Stub for the kaeya4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             Stub for the kaeyaSailwind4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             Stub for the keqing4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             Stub for the keqingOpulent4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             Stub for the kirara4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             Stub for the kiraraBoots4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kiraraBoots4_0();

            /**
             * @brief
             @rst
             Stub for the klee4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             Stub for the kleeBlossomingStarlight4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             Stub for the lisa4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             Stub for the lisaStudent4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory lisaStudent4_0();

            /**
             * @brief
             @rst
             Stub for the mona4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory mona4_0();

            /**
             * @brief
             @rst
             Stub for the monaCN4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory monaCN4_0();

            /**
             * @brief
             @rst
             Stub for the nilou4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory nilou4_0();

            /**
             * @brief
             @rst
             Stub for the nilouBreeze4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory nilouBreeze4_0();

            /**
             * @brief
             @rst
             Stub for the ningguang4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ningguang4_0();

            /**
             * @brief
             @rst
             Stub for the ningguangOrchid4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory ningguangOrchid4_0();

            /**
             * @brief
             @rst
             Stub for the raiden4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory raiden4_0();

            /**
             * @brief
             @rst
             Stub for the rosaria4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory rosaria4_0();

            /**
             * @brief
             @rst
             Stub for the rosariaCN4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory rosariaCN4_0();

            /**
             * @brief
             @rst
             Stub for the shenhe4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory shenhe4_0();

            /**
             * @brief
             @rst
             Stub for the shenheFrostFlower4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory shenheFrostFlower4_0();

            /**
             * @brief
             @rst
             Stub for the xiangling4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xiangling4_0();

            /**
             * @brief
             @rst
             Stub for the xianglingCheer4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xianglingCheer4_0();

            /**
             * @brief
             @rst
             Stub for the xingqiu4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             Stub for the xingqiuBamboo4_0's remover -- returns
             :cpp:func:`IniRemoveBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniRemoveBuilder::Factory xingqiuBamboo4_0();

    };

    /**
     * @brief
     @rst
     The version-keyed table of :cpp:class:`IniRemoveBuilder` factories
     :raw-html:`<br />` :raw-html:`<br />`

     Has no pure-Python counterpart -- see :cpp:class:`IniRemoveBuilderFuncs`'s own warning
     :raw-html:`<br />` :raw-html:`<br />`

     43 rows across 1 game versions (4.0), each mapping a
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
