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

#ifndef AGRemapCore_BufEdit_H
#define AGRemapCore_BufEdit_H

#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResReplace`

     Names the replacement of one of a mod's buffers -- a ``Blend.buf``, ``Position.buf``,
     ``Texcoord.buf`` or ``.ib`` -- by its **kind**, and builds a plain :cpp:class:`IniFixResource`
     of that kind for it :raw-html:`<br />` :raw-html:`<br />`

     Nothing here writes a file. A buffer that has to be fixed together with the others (see
     :cpp:class:`VGSplitGroupResource`) is collected by a :cpp:class:`ResGroupCollect` through one of
     these per buffer, and the grouped resource does the writing; a buffer fixed on its own wants
     :cpp:class:`VGRemapBlendReplace` or :cpp:class:`PositionEditReplace` instead :raw-html:`<br />`
     :raw-html:`<br />`

     The kind is the resource's :cpp:member:`IniResource::type` -- ``blend`` / ``position`` /
     ``texcoord`` / ``buf`` (an index buffer) -- which is what :cpp:func:`RemapStats::get` counts
     under and what a grouped fix tells its members apart by. The naming follows the kind too:
     ``...RemapBlend``, ``...RemapPosition``, ``...RemapTexcoord``, ``...RemapIB``
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BufReplace: public ResReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;

            /**
             * @brief The kind of buffer -- ``blend``, ``position``, ``texcoord`` or ``ib``
             */
            std::string kind;

            /**
             * @brief An extra name between the mod's and the kind's, for a second buffer of the same kind and mod
             */
            std::optional<std::string> resSubType;

            /**
             * @brief Constructs a new buffer replacement
             *
             * @param resModObj The mod object to hold the resource's graph
             * @param config The edit's configuration
             * @param kind The kind of buffer (see #kind)
             * @param resSubType See #resSubType
             *
             * @throw std::invalid_argument If 'kind' is not one of the four
             */
            BufReplace(GraphId resModObj, ResEditConfig config, std::string kind, std::optional<std::string> resSubType = std::nullopt);

            /**
             * @brief The element name the kind is written into a name as -- ``Blend``, ``Position``, ``Texcoord`` or ``IB``
             */
            static std::string elementName(const std::string& kind);

            /**
             * @brief The resource type a kind is counted under -- the kind itself, except ``ib`` which is ``buf``
             */
            static std::string resTypeOf(const std::string& kind);

            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;
            std::string getFixFile(const std::string& file, const std::string& modName = "", const std::string& graphId = "") const override;

            /**
             * @brief Builds an :cpp:class:`IniFixResource` of this edit's kind, so a grouped fix can find and write it
             */
            void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                const std::string& modName, const std::string& fileKey, Context& ctx) override;

        protected:
            std::string subTypedModName(const std::string& modName) const;
    };
}

#include "BufEdit.tpp"

#endif
