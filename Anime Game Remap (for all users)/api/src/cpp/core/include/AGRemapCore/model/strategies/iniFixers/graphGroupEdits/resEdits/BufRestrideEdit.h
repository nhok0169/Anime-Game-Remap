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

#ifndef AGRemapCore_BufRestrideEdit_H
#define AGRemapCore_BufRestrideEdit_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/iniresources/RemapFileResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BufEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufReplace`

     Replaces one of a mod's buffers with the same buffer in ANOTHER LAYOUT -- the stride the target
     reads (:cpp:func:`BufFile::restride`) :raw-html:`<br />` :raw-html:`<br />`

     Used through :cpp:class:`ResRegCollect`, as :cpp:class:`VGRemapBlendReplace` is: the collect
     finds the register binding the buffer (eg. ``vb1 = ResourceBodyTexcoord``), and this edit names
     the replacement by its kind (``...RemapTexcoord``) and builds a
     :cpp:class:`RemapBufRestrideResource` that writes it. Fix the declared ``stride =`` of the
     resource `section`_ alongside (a :cpp:class:`RegNewVals` on it), or the game reads the new file
     at the old stride

     .. code-block:: cpp

        // Miyabi's Body texcoord (24 bytes) onto her skin's Dress (20): TEXCOORD3 dropped
        texcoordCollect_.resEdits = {{"texcoord", restride_.get()}};
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BufRestrideReplace: public BufReplace<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BufReplace<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BufReplace::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @copydoc BufReplace::ResEditConfig
             */
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @copydoc BufReplace::Context
             */
            using Context = typename Base::Context;

            /**
             * @brief The layout the mod's buffer is in, in byte order
             */
            std::vector<std::unique_ptr<BufElementType>> fromElements;

            /**
             * @brief The layout to write, in byte order
             */
            std::vector<std::unique_ptr<BufElementType>> toElements;

            /**
             * @brief See :cpp:func:`BufFile::restride`
             */
            std::unordered_map<std::string, std::string> fillFrom;

            /**
             * @brief Constructs a new restriding buffer replacement
             *
             * @param resModObj The mod object to hold the resource's graph
             * @param config The edit's configuration
             * @param kind The kind of buffer -- see :cpp:member:`BufReplace::kind`
             * @param fromElements The layout the mod's buffer is in
             * @param toElements The layout to write
             * @param fillFrom See :cpp:func:`BufFile::restride`. **Default**: none
             * @param resSubType See :cpp:member:`BufReplace::resSubType`. **Default**: none
             */
            BufRestrideReplace(GraphId resModObj, ResEditConfig config, std::string kind,
                               std::vector<std::unique_ptr<BufElementType>> fromElements,
                               std::vector<std::unique_ptr<BufElementType>> toElements,
                               std::unordered_map<std::string, std::string> fillFrom = {},
                               std::optional<std::string> resSubType = std::nullopt);

            /**
             * @brief Builds the :cpp:class:`RemapBufRestrideResource` that writes the replacement
             */
            void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                const std::string& modName, const std::string& fileKey, Context& ctx) override;

            /**
             * @brief A deep copy of a layout
             *
             * @param elements The layout to copy
             */
            static std::vector<std::unique_ptr<BufElementType>> cloneLayout(const std::vector<std::unique_ptr<BufElementType>>& elements);
    };
}

#include "BufRestrideEdit.tpp"

#endif
