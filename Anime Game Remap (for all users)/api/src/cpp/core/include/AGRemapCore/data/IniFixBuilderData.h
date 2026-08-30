#ifndef AGRemapCore_IniFixBuilderData_H
#define AGRemapCore_IniFixBuilderData_H

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
        **Every method here is currently a stub**: they all return
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
             Stub for the pure-Python ``IniFixBuilderFuncs.xiangling4_0`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
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
             Stub for the pure-Python ``IniFixBuilderFuncs.ayakaSpringbloom5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory ayakaSpringbloom5_7();

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
             Stub for the pure-Python ``IniFixBuilderFuncs.lisa5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory lisa5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniFixBuilderFuncs.nilou5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
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
             Stub for the pure-Python ``IniFixBuilderFuncs.shenheFrostFlower5_7`` -- returns
             :cpp:func:`IniFixBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniFixBuilder::Factory shenheFrostFlower5_7();

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
