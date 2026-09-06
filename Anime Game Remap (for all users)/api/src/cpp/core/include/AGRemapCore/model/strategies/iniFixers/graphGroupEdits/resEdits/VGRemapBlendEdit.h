#ifndef AGRemapCore_VGRemapBlendEdit_H
#define AGRemapCore_VGRemapBlendEdit_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/BlendEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapBlendReplace`

     A ``Blend.buf`` replacement that actually **remaps the vertex groups** :raw-html:`<br />`
     :raw-html:`<br />`

     :cpp:class:`RemapBlendReplace` deliberately does not, for a plain C++ caller: its own class
     note says it inherits :cpp:func:`ResReplace::buildResModel`, which builds a plain
     :cpp:class:`IniFixResource` -- a straight **copy** of the ``Blend.buf``, with the source mod's
     vertex-group weights left exactly as they were. Only the `pybind11`_ layer overrides it, and it
     does so by reading the remap off a *`Python`_* ``ModType``, which a C++ caller has nothing to
     hand it. This class is that missing override, done against
     :cpp:func:`ModType::getVGRemap` :raw-html:`<br />` :raw-html:`<br />`

     **Every fix that touches a** ``Blend.buf`` **wants this one, not the base.** The inherited
     behaviour is not a milder version of the same thing -- it is the bug it looks like in game. The
     boss reads the character's blend weights against its own skeleton, every vertex binds to a bone
     that means something else on the target rig, and the mesh scatters into loose polygons (see
     ``AI Agent Help/CreatingRemaps/Images/Raiden/6_1/RaidenBlendBroken.jpg``). A remapped ``.ini``
     pointing at an unremapped ``.buf`` is *worse* than emitting nothing, because every observable
     signal -- the section names, the file on disk, the fix's own summary line -- says it worked

     :raw-html:`<br />`

     .. note::
        The :cpp:class:`ModType` and both versions are handed in at construction rather than read
        from the :cpp:class:`IniResEditContext` that :cpp:func:`buildResModel` receives, which
        exposes neither. A fixer that owns its own :cpp:class:`IniFileFixContext` has all three to
        hand (:cpp:func:`IniFileFixContext::modType`, :cpp:func:`IniFileFixContext::version`, and
        the ``.ini`` file's own ``toVersion``)

     .. note::
        When the :cpp:class:`ModType` has no remap for this pair of mods, this falls back to the
        base rather than writing a plain copy under a remapped name. A missing remap is a real
        answer -- some pairs genuinely share a skeleton -- but it is not the same as *having*
        applied one, and the two should not be indistinguishable on disk
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class VGRemapBlendReplace: public RemapBlendReplace<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = RemapBlendReplace<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseResEdit::GraphId
             */
            using GraphId = typename Base::GraphId;

            /**
             * @copydoc BaseResEdit::ResEditConfig
             */
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @copydoc BaseResEdit::Context
             */
            using Context = typename Base::Context;

            /**
             * @brief Constructs a new vertex-group-remapping ``Blend.buf`` replacement
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param modType
             @rst
             The mod type being fixed **from** -- where the vertex group remap comes from.
             **Borrowed**, and a ``nullptr`` falls back to the base
             @endrst
             * @param fromVersion The version of the ``.ini`` file being fixed. **Default**: ``std::nullopt``
             * @param toVersion The version being fixed to. **Default**: ``std::nullopt``
             * @param resType
             @rst
             The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``"blend"``, **not** :cpp:class:`RemapBlendReplace`'s own
             ``"resourceRemapBlend"``. This string becomes :cpp:member:`IniResource::type`, which is
             what :cpp:func:`RemapStats::get` looks the resource up by -- and that only knows the
             short kind names (``blend``/``position``/``texcoord``/...). Anything else resolves to
             ``nullptr``, and the resource is then fixed correctly but counted nowhere: the summary
             reports ``0 Blend.buf files`` while the files sit on disk
             @endrst
             * @param resSubType The name of the subtype of the resource. **Default**: none
             * @param fromComp The specific component to remap from. **Default**: none
             * @param toComp The specific component to remap to. **Default**: none
             */
            VGRemapBlendReplace(GraphId resModObj, ResEditConfig config, const ModType* modType,
                                 std::optional<Version> fromVersion = std::nullopt,
                                 std::optional<Version> toVersion = std::nullopt,
                                 std::string resType = "blend",
                                 std::optional<std::string> resSubType = std::nullopt,
                                 std::optional<std::string> fromComp = std::nullopt,
                                 std::optional<std::string> toComp = std::nullopt);

            /**
             * @brief The mod type the vertex group remap comes from -- borrowed, may be ``nullptr``
             */
            const ModType* modType;

            /**
             * @brief The version of the ``.ini`` file being fixed
             */
            std::optional<Version> fromVersion;

            /**
             * @brief The version being fixed to
             */
            std::optional<Version> toVersion;

            /**
             * @brief
             @rst
             Builds a real :cpp:class:`RemapBlendResource`, carrying the vertex group remap
             :cpp:func:`ModType::getVGRemap` gives for this pair of mods :raw-html:`<br />`
             :raw-html:`<br />`

             The resource's ``type`` is this edit's own ``resType``, not the 'resType' argument --
             faithful to both the pure-Python original and the `pybind11`_ override
             @endrst
             *
             * @param resType The name for the type of resource. Unused -- see above
             * @param srcPath The file path to the original resource
             * @param fixedPath The file path to the fixed resource
             * @param modName The name of the mod to fix to
             * @param fileKey The assigned id for the source file
             * @param ctx The .ini file the resource is being built for
             */
            void buildResModel(const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
                                const std::string& modName, const std::string& fileKey, Context& ctx) override;
    };
}

#include "VGRemapBlendEdit.tpp"

#endif
