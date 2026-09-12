#ifndef AGRemapCore_IniParseBuilderData_H
#define AGRemapCore_IniParseBuilderData_H

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
        **Every method here is a stub except** \ref raiden6_1: the rest all return
        :cpp:func:`IniParseBuilder::defaultFactory`. The real pure-Python generators pick between
        concrete subclasses (``GIMIParser``, ``GIMIObjParser``) and pass per-mod arguments, and
        ``GIMIObjParser`` has not been ported to C++ yet. The methods exist now so that the
        *table* is real and version selection genuinely works -- fill them in one at a time as
        concrete strategies land, without touching :cpp:class:`IniParseBuilderData` or anything
        downstream :raw-html:`<br />` :raw-html:`<br />`

        \ref raiden6_1 is the first one filled in, and shows the shape the others take: a factory
        capturing whatever per-mod data it needs, returning a parser subclass that owns both its
        :cpp:class:`IniParseContext` and anything else it hands to the parser by borrowed pointer

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
             The parser for a 4.0-era **Ayaka** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Alone in her batch she needs no register overrides: her own downloads are already on the
             modern ``ps-t0``/``ps-t1``, and it is her SKIN that sits a slot higher. See
             ``data/IniParseData/Ayaka/AyakaParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory ayaka4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **AyakaSpringbloom** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Her head and body sit a slot higher; only the dress is on the modern layout. See
             ``data/IniParseData/AyakaSpringbloom/AyakaSpringbloomParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory ayakaSpringbloom4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Barbara** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Three drawn objects -- head, body and dress -- and her fix moves the shared
             ``drawindexed``. See ``data/IniParseData/Barbara/BarbaraParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory barbara4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **BarbaraSummertime** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The same three objects as Barbara; this pair is one-to-one. See
             ``data/IniParseData/BarbaraSummertime/BarbaraSummertimeParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory barbaraSummertime4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Diluc** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head and body, stride 12. DilucFlamme adds the dress his coat becomes. See
             ``data/IniParseData/Diluc/DilucParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory diluc4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **DilucFlamme** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head, body and dress, stride 20 where Diluc's is 12. See
             ``data/IniParseData/DilucFlamme/DilucFlammeParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory dilucFlamme4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Fischl** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head, body and dress -- she is the one WITH the dress, the opposite way round
             from the Diluc pair. See ``data/IniParseData/Fischl/FischlParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory fischl4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **FischlHighness** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head and body, stride 12. See
             ``data/IniParseData/FischlHighness/FischlHighnessParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory fischlHighness4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Ganyu** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The standard GIMI character shape. See ``data/IniParseData/Ganyu/GanyuParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory ganyu4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **HuTao** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Draws ``head``/``body``, two fewer than her skin. See
             ``data/IniParseData/HuTao/HuTaoParser.cpp``
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
             The parser for a 4.0-era **Kaeya** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Three drawn objects and FOUR indices: IndexData gives him an ``extra`` that no
             Kaeya ``.ini`` declares and that only KaeyaSailwind's dress split fills. See
             ``data/IniParseData/Kaeya/KaeyaParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kaeya4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **KaeyaSailwind** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The same three objects as Kaeya; the asymmetry is all on the fix side. See
             ``data/IniParseData/KaeyaSailwind/KaeyaSailwindParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kaeyaSailwind4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Keqing** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The standard GIMI character shape, drawing ``head``/``body``/``dress``. See
             ``data/IniParseData/Keqing/KeqingParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory keqing4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **KeqingOpulent** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Draws ``head``/``body`` only -- her Lantern Rite outfit is one mesh, which is what makes
             the remap to and from Keqing a merge in one direction and a split in the other. See
             ``data/IniParseData/KeqingOpulent/KeqingOpulentParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory keqingOpulent4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Kirara** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             THREE different download layouts in one character: her head and body carry a normal map
             on ``ps-t0`` and sit a slot higher, her dress sits a slot higher with no normal map. See
             ``data/IniParseData/Kirara/KiraraParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kirara4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Klee** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Her texcoord stride is 12 where most of this batch is 20. See
             ``data/IniParseData/Klee/KleeParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory klee4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **KleeBlossomingStarlight** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             She has no Face component in the assets repo, so no face diffuse hash. See
             ``data/IniParseData/KleeBlossomingStarlight/KleeBlossomingStarlightParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kleeBlossomingStarlight4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Lisa** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head, body and dress; the dress is what LisaStudent has nowhere to put. See
             ``data/IniParseData/Lisa/LisaParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory lisa4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **LisaStudent** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Head and body only, and her downloads bind the diffuse to ``ps-t1`` because
             ``ps-t0`` is her normal map. See
             ``data/IniParseData/LisaStudent/LisaStudentParser.cpp``
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
             The parser for a 4.0-era **Nilou** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The standard GIMI shape, with one thing spelled out: a 4.0-era shader reads its diffuse
             from ``ps-t1`` and its lightmap from ``ps-t2``, so her downloads say so. See
             ``data/IniParseData/Nilou/NilouParser.cpp``
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
             The parser for a 4.0-era **Shenhe** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             The standard GIMI character shape, drawing ``head``/``body``/``dress``. See
             ``data/IniParseData/Shenhe/ShenheParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory shenhe4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Xiangling** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Draws ``head``/``body``/``dress``, one more than her skin. See
             ``data/IniParseData/Xiangling/XianglingParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory xiangling4_0();

            /**
             * @brief
             @rst
             The parser for a 4.0-era **Xingqiu** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Draws ``head``/``body``. See ``data/IniParseData/Xingqiu/XingqiuParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory xingqiu4_0();

            /**
             * @brief
             @rst
             The parser for a 4.4-era **GanyuTwilight** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The standard GIMI character shape. See
             ``data/IniParseData/GanyuTwilight/GanyuTwilightParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory ganyuTwilight4_4();

            /**
             * @brief
             @rst
             The parser for a 4.4-era **ShenheFrostFlower** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             FOUR drawn objects -- ``head``/``body``/``dress``/``extra`` -- one more than Shenhe. See
             ``data/IniParseData/ShenheFrostFlower/ShenheFrostFlowerParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory shenheFrostFlower4_4();

            /**
             * @brief
             @rst
             The parser for a 4.4-era **XingqiuBamboo** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Draws ``head``/``body``/``dress``. See
             ``data/IniParseData/XingqiuBamboo/XingqiuBambooParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory xingqiuBamboo4_4();

            /**
             * @brief
             @rst
             The parser for a 4.8-era **KiraraBoots** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The same three-layout split her base has. See
             ``data/IniParseData/KiraraBoots/KiraraBootsParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kiraraBoots4_8();

            /**
             * @brief
             @rst
             The parser for a 4.8-era **NilouBreeze** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The standard GIMI shape with nothing added: the skin shipped after GI moved the diffuse
             and lightmap down, so the defaults are already right. See
             ``data/IniParseData/NilouBreeze/NilouBreezeParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory nilouBreeze4_8();

            /**
             * @brief
             @rst
             The parser for a 5.3-era **CherryHuTao** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             FOUR drawn objects, and a texcoord stride of 28 that nothing else in the table uses.
             See ``data/IniParseData/CherryHuTao/CherryHuTaoParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory cherryHutao5_3();

            /**
             * @brief
             @rst
             The parser for a 5.3-era **XianglingCheer** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Draws ``head``/``body`` only. See
             ``data/IniParseData/XianglingCheer/XianglingCheerParser.cpp``
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
             The parser for a 5.6-era **AyakaSpringbloom** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             Same downloads as 4.0 -- the version exists because her TEXTURE EDITS changed. See
             ``data/IniParseData/AyakaSpringbloom/AyakaSpringbloomParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory ayakaSpringbloom5_6();

            /**
             * @brief
             @rst
             The parser for a 5.7-era **AyakaSpringbloom** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             By 5.7 her head and body have moved down to ``ps-t0``/``ps-t1``, the default. See
             ``data/IniParseData/AyakaSpringbloom/AyakaSpringbloomParser.cpp``
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
             The parser for a 5.7-era **Kirara** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Her body and dress have moved to the modern ``ps-t0``/``ps-t1`` by 5.7; her HEAD has
             not. See ``data/IniParseData/Kirara/KiraraParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory kirara5_7();

            /**
             * @brief
             @rst
             The parser for a 5.7-era **KiraraBoots** ``.ini`` file -- **not** a stub
             :raw-html:`<br />` :raw-html:`<br />`

             The MIRROR of :cpp:func:`kirara5_7`: here the head and body have moved to
             ``ps-t0``/``ps-t1`` and the DRESS is the one left behind. See
             ``data/IniParseData/KiraraBoots/KiraraBootsParser.cpp``
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
            /**
             * @brief
             @rst
             LisaStudent's 5.4 parser -- the same shifted registers as 4.0, but reading her
             re-dumped assets out of their own ``5_4`` folder. See
             :cpp:func:`LisaStudentParser::v5_4`
             @endrst
             */
            static IniParseBuilder::Factory lisaStudent5_4();

            static IniParseBuilder::Factory lisaStudent5_7();

            /**
             * @brief
             @rst
             The parser for a 5.7-era **Nilou** ``.ini`` file -- **not** a stub :raw-html:`<br />`
             :raw-html:`<br />`

             Identical to :cpp:func:`nilou4_0` except that GI has moved the diffuse and lightmap down
             to ``ps-t0``/``ps-t1``, which is the default. See ``data/IniParseData/Nilou/NilouParser.cpp``
             @endrst
             */
            static IniParseBuilder::Factory nilou5_7();

            /**
             * @brief
             @rst
             The pure-Python ``IniParseBuilderFuncs.raiden6_1`` -- **not** a stub, unlike every
             other method here :raw-html:`<br />` :raw-html:`<br />`

             Builds a :cpp:class:`GIMIParser` over four mod objects, classified by a single
             :cpp:class:`GIMISectionClassifier` in the two ways it supports:
             :raw-html:`<br />` :raw-html:`<br />`

             * ``("", "head")``/``("", "body")``/``("", "dress")`` are attributed only when a
               part's ``hash`` resolves to Raiden's ``ib`` **and** a ``match_first_index``
               following it resolves to that object's own :cpp:class:`Indices` row. All three
               share the one ``ib``, so the hash alone cannot tell them apart -- which is why they
               are in the classifier's ``indexKeyToModObj`` rather than its ``hashKeyOnlyToModObj``
             * ``("", "blend")`` is the ``Blend.buf`` the fix remaps, and its ``blend_vb`` hash
               names it outright -- so it *is* in ``hashKeyOnlyToModObj``, with no index involved

             :raw-html:`<br />`

             The parser is also built with ``disjointModObjs`` **false**, so one `section`_ may be
             attributed to several mod objects -- see the constructed parser's own comment for the
             3dmigoto grammar bug behind that
             @endrst
             */
            static IniParseBuilder::Factory raiden6_1();

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
