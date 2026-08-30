#ifndef AGRemapCore_BlendEdit_H
#define AGRemapCore_BlendEdit_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResReplace`

     Class that builds the necessary part to replace some ``Blend.buf`` file :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The ``Blend.buf`` file's own vertex-group remap comes from the ``ModType`` being fixed
        (``modType.getVGRemap(...)``). The `Python`_-facing ``ModType`` is an unrelated pure-Python
        class with no C++ counterpart, so **this class does not override**
        :cpp:func:`BaseResEdit::buildResModel` -- it inherits :cpp:class:`ResReplace`'s, which builds
        a plain :cpp:class:`IniFixResource`. The `pybind11`_ layer overrides it to build a real
        :cpp:class:`RemapBlendResource` from the `Python`_ ``ModType``. A plain C++ caller that wants
        the same must override it too; everything else about this class (all the naming) works
        either way
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RemapBlendReplace: public ResReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @brief The name of the subtype of the resource, folded into every fixed name
             */
            std::optional<std::string> resSubType;

            /**
             * @brief The specific component to remap from
             */
            std::optional<std::string> fromComp;

            /**
             * @brief The specific component to remap to
             */
            std::optional<std::string> toComp;

            /**
             * @brief Constructs a new ``Blend.buf``-replacing resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapBlend"``
             * @param resSubType The name of the subtype of the resource. **Default**: none
             * @param fromComp The specific component to remap from. **Default**: none
             * @param toComp The specific component to remap to. **Default**: none
             */
            RemapBlendReplace(GraphId resModObj, ResEditConfig config, std::string resType = "resourceRemapBlend",
                               std::optional<std::string> resSubType = std::nullopt,
                               std::optional<std::string> fromComp = std::nullopt,
                               std::optional<std::string> toComp = std::nullopt);

            /**
             * @brief
             @rst
             The fixed name of the resource `section`_ -- :cpp:func:`IniNamingTools::getRemapBlendResourceName`
             against 'modName' extended by \ref resSubType
             @endrst
             *
             * @param resource The name of the original resource `section`_
             * @param modName The name of the mod to fix to. **Default**: ``""``
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief
             @rst
             The fixed ``Blend.buf`` file path -- :cpp:func:`IniNamingTools::getFixedBlendFile` against
             'modName' extended by \ref resSubType, with 'graphId' appended when there is one
             @endrst
             *
             * @param file The file path to the original resource
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             */
            std::string getFixFile(const std::string& file, const std::string& modName = "",
                                    const std::string& graphId = "") const override;

        protected:
            /**
             * @brief 'modName' capitalized, extended by \ref resSubType when there is one
             *
             * @param modName The name of the mod to fix to
             */
            std::string subTypedModName(const std::string& modName) const;
    };
}

#include "BlendEdit.tpp"

#endif
