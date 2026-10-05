#ifndef AGRemapCore_PositionEdit_H
#define AGRemapCore_PositionEdit_H

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

#include <optional>
#include <string>

#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/iniresources/RemapPositionResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResReplace`

     Names a remapped ``Position.buf`` -- the position counterpart of
     :cpp:class:`RemapBlendReplace`, and the same two overrides: the `section`_ name and the file
     name
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RemapPositionReplace: public ResReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @brief Constructs a new ``Position.buf`` replacement
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource
             */
            RemapPositionReplace(GraphId resModObj, ResEditConfig config,
                                  std::string resType = "resourceRemapPosition");

            /**
             * @brief The fixed name of the resource `section`_
             */
            std::optional<std::string> getFixResourceName(const std::string& resource,
                                                           const std::string& modName = "") const override;

            /**
             * @brief The fixed ``Position.buf`` file path
             */
            std::string getFixFile(const std::string& file, const std::string& modName = "",
                                    const std::string& graphId = "") const override;
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapPositionReplace`

     Builds a real :cpp:class:`RemapPositionResource`, so the remapped ``Position.buf`` is actually
     WRITTEN rather than merely named :raw-html:`<br />` :raw-html:`<br />`

     The same split as :cpp:class:`VGRemapBlendReplace` and :cpp:class:`TexEditorReplace`, and for
     the same reason: the base class knows what to call things and nothing about what to put in
     them, so a fix that used it alone would produce a perfectly consistent ``.ini`` file pointing
     at a file that does not exist
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class PositionEditReplace: public RemapPositionReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = RemapPositionReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;

            /**
             * @brief Constructs a new editing ``Position.buf`` replacement
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param edit What to do to each vertex
             * @param resType
             @rst
             The name of the type of resource. **Default**: ``"position"``, **not**
             :cpp:class:`RemapPositionReplace`'s own ``"resourceRemapPosition"`` -- see
             :cpp:class:`VGRemapBlendReplace`'s constructor for why the short kind name is the one
             :cpp:func:`RemapStats::get` can find
             @endrst
             */
            PositionEditReplace(GraphId resModObj, ResEditConfig config, BufFile::Filter edit,
                                 std::string resType = "position");

            /**
             * @brief What to do to each vertex
             */
            BufFile::Filter edit;

            void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                const std::string& modName, const std::string& fileKey, Context& ctx) override;
    };
}

#include "PositionEdit.tpp"

#endif
