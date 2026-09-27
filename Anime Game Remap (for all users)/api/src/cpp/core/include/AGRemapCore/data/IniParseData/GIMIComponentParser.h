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

#ifndef AGRemapCore_GIMIComponentParser_H
#define AGRemapCore_GIMIComponentParser_H

#include <string>
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     What :cpp:func:`makeGIMIComponentParser` needs to read a mod built for a skin of SEVERAL
     components -- the parser counterpart of :cpp:struct:`GIMICharParserConfig`
     @endrst
     */
    struct GIMIComponentParserConfig {
        /**
         * @brief The mod type a ``.ini`` of this skin classifies as, eg. ``YelanTranquil``
         */
        ModTypeId modTypeId;

        /**
         * @brief The folder under ``Data/Mod Downloads/GI/``
         */
        std::string downloadCharFolder;

        /**
         * @brief The version subfolder, eg. ``5_7``
         */
        std::string downloadVersionFolder;

        /**
         * @brief The download file prefix -- NOT derivable from #downloadCharFolder
         */
        std::string downloadPrefix;

        /**
         * @brief One draw slot of one component
         */
        struct Slot {
            /**
             * @brief The slot's name, eg. ``A``
             */
            std::string name;

            /**
             * @brief
             @rst
             The slot's ``match_first_index``, as a LITERAL :raw-html:`<br />` :raw-html:`<br />`

             Deliberately not an :cpp:class:`Indices` row, for the reason ``IndexData.cpp``'s own
             note records: :cpp:func:`ModMappedAssets::getKey` buckets every row holding a value by
             version and searches only the newest bucket at or below the version asked, so a slot
             filed as ``"0"`` at 5.7 would make the 5.7 bucket THE bucket for ``"0"`` and every
             classic character's head -- index 0, filed at 4.0 -- would reverse-resolve to a slot
             named ``A`` and fall out of its own parser. So the classifier here resolves a slot from
             this literal, per component, rather than through the table
             @endrst
             */
            std::string index;

            std::string diffuseReg = "ps-t0";
            std::string lightMapReg = "ps-t1";

            /**
             * @brief Empty when the slot's shader reads no normal map
             */
            std::string normalMapReg;

            /**
             * @brief ``true`` when this slot has no textures of its own and reads another's
             */
            bool noTextures = false;

            /**
             * @brief
             @rst
             The ``<Component>;<Slot>`` whose textures the GAME draws this slot with, for a slot
             that binds none of its own -- empty for a slot that always has them
             :raw-html:`<br />` :raw-html:`<br />`

             A GIMI ``TextureOverride`` binds registers for the draw call its hash matches and no
             other, so a slot whose own `section`_ declares no ``ps-t`` renders with whatever the
             GAME had bound, however thoroughly the mod repainted the donor's atlas. Naming the
             donor here is what lets those textures be DOWNLOADED rather than read out of the mod
             :raw-html:`<br />` :raw-html:`<br />`

             Measured over five YelanTranquil mods: three of them leave a slot textureless, and
             reading the mod's donor instead put a repainted atlas under the game's texture
             coordinates -- one mod's hair and another's eyes came out sampling other islands
             entirely (2026-09-14)
             @endrst
             */
            std::string textureDonor;

            /**
             * @brief
             @rst
             Whether the donor's NORMAL MAP is downloaded too, at :cpp:member:`normalMapReg`
             :raw-html:`<br />` :raw-html:`<br />`

             Needed when the TARGET reads normal maps: CitlaliWhisperofStars -> Citlali keeps every
             slot's normal map (``GIMIMergeFixerConfig::TargetLayout::NormalMap``), and a borrowing
             slot fetched only the donor's diffuse and light map, so its draw read whatever normal
             map the game had bound (2026-09-22). Off by default: a plain target drops the normal
             map, and fetching one only to drop it is a download and a resource section for nothing
             @endrst
             */
            bool donorNormalMap = false;
        };

        /**
         * @brief One component of the skin -- its own buffers, its own hashes, its own draw slots
         */
        struct Component {
            /**
             * @brief The component's name, eg. ``Body``
             */
            std::string name;

            /**
             * @brief
             @rst
             The mod type NAME its hashes are filed under in ``HashData``, eg. ``YelanTranquilBody``
             :raw-html:`<br />` :raw-html:`<br />`

             Not the same as the skin's own name: each component of a multi-component skin is a
             :cpp:enum:`ModTypeId` of its own so the forward direction can target it, and its hashes
             live under that name. This is why one classifier per component is needed rather than one
             for the parser: a single hash filter can only name one of them
             @endrst
             */
            std::string modTypeName;

            /**
             * @brief The component's draw slots, in draw order
             */
            std::vector<Slot> slots;

            /**
             * @brief This component's ``Texcoord.buf`` stride -- 20 with a second UV set, 12 without
             */
            int texcoordStride = 20;

            /**
             * @brief
             @rst
             The GAME model's vertex count for this component, for the downloaded blend's ``draw``
             line. ``0`` leaves the line out :raw-html:`<br />` :raw-html:`<br />`

             In the config rather than :cpp:class:`VertexCounts` because that table's rows all carry
             an empty component column, and ``VertexCounts_test`` asserts so deliberately -- the day
             a real per-component count is filed there, that tripwire is the thing that says so
             @endrst
             */
            long long vertexCount = 0;
        };

        /**
         * @brief Every component of the skin
         */
        std::vector<Component> components;

        int positionStride = 40;
        int blendStride = 32;
    };

    /**
     * @brief
     @rst
     Builds the parser for a mod of a skin made of SEVERAL components :raw-html:`<br />`
     :raw-html:`<br />`

     :cpp:func:`makeGIMICharParser` cannot express one: its mod objects are all ``("", obj)``, its
     index map has the single key ``ib``, and its hash filter names one mod type -- while a
     multi-component skin files each component's hashes under that COMPONENT's mod type name. So this
     builds **one classifier per component**, each filtered to its own name, and a section is offered
     to each in turn; a hash value is unique to one character, so at most one answers
     :raw-html:`<br />` :raw-html:`<br />`

     The mod objects it produces are ``(component, kind)`` -- ``("Body", "blend")``,
     ``("Bang", "position")`` -- and ``(component, slot)`` for a drawn object, which is what lets a
     fixer address one component's buffers without touching another's. The ``match_first_index`` of a
     drawn object is resolved from :cpp:member:`GIMIComponentParserConfig::Slot::index` rather than
     through :cpp:class:`Indices`; see there for why
     @endrst
     */
    IniParseBuilder::Factory makeGIMIComponentParser(GIMIComponentParserConfig config);

    /**
     * @brief
     @rst
     What a parser from :cpp:func:`makeGIMIComponentParser` learned about a ``.ini`` file that its
     merge fixer needs, and cannot read off the sections :raw-html:`<br />` :raw-html:`<br />`

     The fixer reaches it by ``dynamic_cast`` from the parser it is handed. Absent (another parser),
     everything is assumed to be there -- the behaviour before this existed
     @endrst
     */
    class GIMIComponentParseFacts {
        public:
            virtual ~GIMIComponentParseFacts() = default;

            /**
             * @brief
             @rst
             Whether the ``.ini`` file carries anything of the skin to remap: a section of its own
             that binds a buffer, an index buffer, a texture or a draw, or a texture override by one
             of the skin's slot texture hashes :raw-html:`<br />` :raw-html:`<br />`

             ``false`` for a file that only WATCHES the skin -- a help overlay whose one section
             matches the position hash to know she is on screen (2026-09-24). Everything the parser
             would give such a file comes from downloads, and remapping it puts a second full copy
             of the skin over the real mod's
             @endrst
             */
            virtual bool hasRemappableContent() const = 0;

            /**
             * @brief
             @rst
             Whether the mod leaves this source slot UNDRAWN: it carries the component's own buffers
             and skips the component's index buffer (``handling = skip``), and declares no section for
             the slot -- so on its own character the slot is never drawn :raw-html:`<br />` :raw-html:`<br />`

             Such a slot gets no download and brings nothing to the merge. The game's index buffer
             for it counts in the GAME's vertex order, and drawn over the mod's own buffers it is
             shards of stretched triangles: CharlotteHurlock1 draws its slot C geometry inside its
             slot B section and has no C or D section, and the downloaded C and D put its skirt up to
             its chest (2026-09-24)
             @endrst

             @param[in] component The source component, eg. ``Body``
             @param[in] slot The source slot, eg. ``C``
             */
            virtual bool isSlotUndrawn(const std::string& component, const std::string& slot) const = 0;
    };
}

#endif
