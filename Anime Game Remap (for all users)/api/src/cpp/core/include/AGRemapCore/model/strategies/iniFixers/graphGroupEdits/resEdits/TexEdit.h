#ifndef AGRemapCore_TexEdit_H
#define AGRemapCore_TexEdit_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResCreate`

     Class that builds the necessary parts to create some new texture file :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The texture itself is produced by a :cpp:class:`TexCreator`, and the resource model that
        carries it is a :cpp:class:`RemapTexAddResource`. Neither is built here: the `pybind11`_
        layer overrides :cpp:func:`BaseResEdit::buildResModel` (to hold the caller's own `Python`_
        ``TexCreator``) and :cpp:func:`ResCreate::buildSection` (which has to produce a `Python`_
        -owned `section`_). Everything else -- the numbering and naming of successive textures --
        lives here
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class TexCreate: public ResCreate<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResCreate<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @brief The name for the type of texture
             */
            std::string texName;

            /**
             * @brief Constructs a new texture-creating resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param texName The name for the type of texture
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexAdd"``
             */
            TexCreate(GraphId resModObj, std::string texName, ResEditConfig config,
                       std::string resType = "resourceRemapTexAdd");

            /**
             * @brief Resets the texture counter \ref getFixResourceName advances
             */
            void clear() override;

            /**
             * @brief
             @rst
             The fixed name of the resource `section`_ -- ``<capitalized modName><texName>``, with a
             counter suffix from the second texture onwards, run through
             :cpp:func:`IniNamingTools::getRemapTexResourceName` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike every other ``getFixResourceName`` in this family, this one is **not pure** --
                each call advances the counter, so the same 'resource' asked for twice gets two
                different names. That is the pure-Python original's own behaviour (it is what
                numbers successive textures apart), and \ref clear is what resets it between runs
             @endrst
             *
             * @param resource The name of the original resource `section`_. Unused -- the name is built from 'modName' and \ref texName
             * @param modName The name of the mod to fix to. **Default**: ``""``
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief
             @rst
             The fixed texture file path -- :cpp:func:`IniNamingTools::getFixedTexFile`
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                When a 'graphId' is given, the id is appended to the **original** 'file', not to the
                fixed path just computed -- so the fixed name is discarded in that case. That is
                exactly what the pure-Python original did (``return self.fileAddGraphId(file,
                graphId = graphId)``, where every sibling passes ``result``), and it is preserved
                here rather than quietly corrected: it is not one of the bugs this port was asked to
                fix, and changing it would rename real output files
             @endrst
             *
             * @param file The file path to the original resource
             * @param modName The name of the mod to fix to. Unused. **Default**: ``""``
             * @param graphId The unique id for the graph of the resource. **Default**: ``""``
             */
            std::string getFixFile(const std::string& file, const std::string& modName = "",
                                    const std::string& graphId = "") const override;

            /**
             * @brief The number of textures named so far, since construction or the last \ref clear
             */
            int texInd() const;

        private:
            // Mutable because getFixResourceName is const (it overrides a const virtual) yet has to
            // advance the counter -- see its own note on why it is deliberately not pure.
            mutable int texInd_ = 0;
    };
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResReplace`

     Class that builds the necessary part to **edit** an existing texture file :raw-html:`<br />`
     :raw-html:`<br />`

     The counterpart to :cpp:class:`TexCreate` in this same file, and the texture analogue of
     :cpp:class:`RemapBlendReplace`: an edit has an original resource to build its fixed name on,
     which is exactly what separates a :cpp:class:`ResReplace` from a :cpp:class:`ResCreate`. The
     resource model it stands for is a :cpp:class:`RemapTexEditResource` (``srcPath`` **and**
     ``fixedPath``), not a :cpp:class:`RemapTexAddResource` :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Like :cpp:class:`RemapBlendReplace`, this **does not override**
        :cpp:func:`BaseResEdit::buildResModel` -- the texture editor that does the work reaches
        this class from the `Python`_ side, so the `pybind11`_ layer overrides it to build a real
        :cpp:class:`RemapTexEditResource`. A plain C++ caller wanting the same must override it
        too; all the naming here works either way
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class TexReplace: public ResReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;

            /**
             * @brief The name of the subtype of the resource, folded into every fixed name
             */
            std::optional<std::string> resSubType;

            /**
             * @brief Constructs a new texture-editing resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexEdit"``
             * @param resSubType The name of the subtype of the resource. **Default**: none
             */
            TexReplace(GraphId resModObj, ResEditConfig config, std::string resType = "resourceRemapTexEdit",
                        std::optional<std::string> resSubType = std::nullopt);

            /**
             * @brief
             @rst
             The fixed name for a texture resource, built from the **original** resource's name --
             the difference that makes this a :cpp:class:`ResReplace`. :cpp:class:`TexCreate`
             discards its 'resource' argument because a newly created texture has no original to
             build on
             @endrst
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief The fixed file path for an edited texture
             */
            std::string getFixFile(const std::string& file, const std::string& modName = "",
                                    const std::string& graphId = "") const override;

            /**
             * @brief 'modName' capitalized, extended by ef resSubType when there is one
             */
            std::string subTypedModName(const std::string& modName) const;
    };
}

#include "TexEdit.tpp"

#endif
