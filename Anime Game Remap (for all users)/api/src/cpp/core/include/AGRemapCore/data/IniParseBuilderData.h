#ifndef AGRemapCore_IniParseBuilderData_H
#define AGRemapCore_IniParseBuilderData_H

#include <memory>

#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Defines how the :cpp:class:`IniParseBuilder` arguments for some mod are built for a
     particular game version -- the C++ counterpart to the pure-Python ``IniParseBuilderFuncs``
     class (``data/IniParseBuilderData.py``) :raw-html:`<br />` :raw-html:`<br />`

     One static method per (mod, version-it-changed-at) pair, named exactly as in the original,
     each returning the :cpp:type:`IniParseBuilder::Factory` for that pair
     :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        **Every method here is currently a stub**: they all return
        :cpp:func:`IniParseBuilder::defaultFactory`, which builds a plain
        :cpp:class:`BaseIniParser`. The real pure-Python generators pick between concrete
        subclasses (``GIMIParser``, ``GIMIObjParser``) and pass per-mod
        arguments, none of which have been ported to C++ yet. The methods exist now so
        that the *table* is real and version selection genuinely works -- fill them in one at a
        time as concrete strategies land, without touching :cpp:class:`IniParseBuilderData` or
        anything downstream

     .. note::
        The pure-Python original also carries per-mod texture-edit helpers such as ``_ayakaEditDressDiffuse``
        and ``_ayakaSpringbloomEditLightMap5_6``.
        Those exist only to build the arguments the real generators pass, so while every method
        below is a stub they would be dead code -- port them alongside the first generator that
        actually needs them
     @endrst
     */
    class IniParseBuilderFuncs {
        public:

            IniParseBuilderFuncs() = delete;

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.amber4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory amber4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.amberCN4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory amberCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ayaka4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ayakaSpringbloom4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.barbara4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.barbaraSummertime4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.diluc4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.dilucFlamme4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.fischl4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.fischlHighness4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ganyu4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.hutao4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory hutao4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.jean4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory jean4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.jeanCN4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory jeanCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.jeanSea4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory jeanSea4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kaeya4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kaeyaSailwind4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.keqing4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.keqingOpulent4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kirara4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.klee4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kleeBlossomingStarlight4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.lisa4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.lisaStudent4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory lisaStudent4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.mona4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory mona4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.monaCN4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory monaCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.nilou4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory nilou4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ningguang4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ningguang4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ningguangOrchid4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ningguangOrchid4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.giDefault`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory giDefault();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.rosaria4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory rosaria4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.rosariaCN4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory rosariaCN4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.shenhe4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory shenhe4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.xiangling4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory xiangling4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.xingqiu4_0`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ganyuTwilight4_4`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ganyuTwilight4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.shenheFrostFlower4_4`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory shenheFrostFlower4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.xingqiuBamboo4_4`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory xingqiuBamboo4_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kiraraBoots4_8`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kiraraBoots4_8();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.nilouBreeze4_8`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory nilouBreeze4_8();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.cherryHutao5_3`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory cherryHutao5_3();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.xianglingCheer5_3`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory xianglingCheer5_3();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.arlecchino5_4`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory arlecchino5_4();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.jean5_5`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory jean5_5();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.jeanCN5_5`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory jeanCN5_5();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ayakaSpringbloom5_6`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ayakaSpringbloom5_6();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ayakaSpringbloom5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ayakaSpringbloom5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.ganyuTwilight5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory ganyuTwilight5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kirara5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kirara5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.kiraraBoots5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory kiraraBoots5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.lisaStudent5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory lisaStudent5_7();

            /**
             * @brief
             @rst
             Stub for the pure-Python ``IniParseBuilderFuncs.nilou5_7`` -- returns
             :cpp:func:`IniParseBuilder::defaultFactory`, see this class's own warning
             @endrst
             */
            static IniParseBuilder::Factory nilou5_7();

    };

    /**
     * @brief
     @rst
     The version-keyed table of :cpp:class:`IniParseBuilder` factories -- the C++ counterpart
     to the pure-Python ``IniParseBuilderData`` dictionary (``data/IniParseBuilderData.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     53 rows across 9 game versions (4.0, 4.4, 4.6, 4.8, 5.3, 5.4, 5.5, 5.6, 5.7), each mapping a
     ``(version, mod name)`` pair to one :cpp:class:`IniParseBuilderFuncs` method
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Mod names come from :cpp:func:`ModTypeIdTools::getName` rather than being spelled out as
        string literals, exactly as the original's own
        ``ModTypeIdTools.getName(ModTypeId.Amber)`` keys do -- so a rename in the registry
        cannot silently desync this table from it

     .. note::
        A mod only needs a row at the version its parser *changed*.  
        :cpp:func:`ModDictAssets::get`'s inclusive floor-match means that row keeps applying to
        every later version until a newer one supersedes it, which is why most mods appear only
        once, at 4.0
     @endrst
     */
    class IniParseBuilderData {
        public:

            IniParseBuilderData() = delete;

            /**
             * @brief
             @rst
             The shared table, lazily built on first access and reused afterwards -- the same
             lazy, build-once pattern as :cpp:func:`GlobalIniClassifiers::classifier`
             :raw-html:`<br />` :raw-html:`<br />`

             Held by ``shared_ptr`` because that is what
             :cpp:func:`IniParseBuilder::IniParseBuilder` takes -- every
             :cpp:class:`ModType` of the game shares this one table
             @endrst
             *
             * @return The shared args table
             */
            static const std::shared_ptr<const IniParseBuilder::ArgsRepo>& repo();
    };
}

#endif
