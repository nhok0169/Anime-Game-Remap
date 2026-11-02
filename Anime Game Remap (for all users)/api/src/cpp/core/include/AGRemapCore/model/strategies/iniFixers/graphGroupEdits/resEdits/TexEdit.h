#ifndef AGRemapCore_TexEdit_H
#define AGRemapCore_TexEdit_H

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

#include <functional>
#include <optional>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResCreate`

     Class that builds the necessary parts to create some new texture file :raw-html:`<br />`
     :raw-html:`<br />`

     The texture is produced by \ref texCreator, and the resource model that carries it is a
     :cpp:class:`RemapTexAddResource` (#makeResModel). The creator is held through a ``std::shared_ptr`` to its
     base and shared by every resource built, so a subclass's own :cpp:func:`BaseTexEditor::fix` -- a
     `Python`_ :class:`TexCreator`'s included -- is the one that runs
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
            using Context = typename Base::Context;
            using Section = typename Base::Section;

            /**
             * @brief The name for the type of texture
             */
            std::string texName;

            /**
             * @brief
             @rst
             The textures to create under the names their registers already give them, as
             ``section name -> file`` (relative to the ``.ini`` file) :raw-html:`<br />` :raw-html:`<br />`

             Set, a collected resource keeps its name (:cpp:func:`getFixResourceName` answers ``std::nullopt``),
             only the `sections`_ listed here are built, and each names its file -- for a fix that names its
             created textures itself. Empty (the default), every collected resource is renamed from the mod's
             name and the texture's, and its file from that
             @endrst
             */
            std::unordered_map<std::string, std::string> files;

            /**
             * @brief
             @rst
             The file the `section`_ 'sectionName' names: its entry in \ref files when that is set (and
             ``std::nullopt`` for a name it does not list, which builds no `section`_), otherwise
             :cpp:func:`getFixFile` of the `section`_'s name without its leading ``Resource``, as a ``.dds``
             @endrst
             *
             * @param sectionName The name of the `section`_ to build
             * @param modName The name of the mod to fix to
             */
            std::optional<std::string> fileOfSection(const std::string& sectionName, const std::string& modName) const;

            /**
             * @brief
             @rst
             The creator that produces every texture this resource edit adds -- held through its base, so a
             subclass's :cpp:func:`BaseTexEditor::fix` (a `Python`_ one included) is the one that runs, and
             shared by every resource built
             @endrst
             */
            std::shared_ptr<BaseTexEditor> texCreator;

            /**
             * @brief What fixes each built resource in place of running \ref texCreator, or empty. **Default**: empty
             */
            std::function<bool(RemapTexAddResource&)> fixFunc;

            /**
             * @brief Constructs a new texture-creating resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param texName The name the created texture is filed under, eg. ``"NormalMap"``
             * @param texCreator The creator that produces each texture
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexAdd"``
             */
            TexCreate(GraphId resModObj, std::string texName, std::shared_ptr<BaseTexEditor> texCreator, ResEditConfig config,
                      std::string resType = "resourceRemapTexAdd");

            /**
             * @brief
             @rst
             The :cpp:class:`RemapTexAddResource` that runs \ref texCreator (or \ref fixFunc) and writes 'path'
             @endrst
             *
             * @param iniFolder The folder of the ``.ini`` file the resource belongs to
             * @param path Where the created texture is written
             * @param resType The name of the type of resource
             *
             * @return The resource
             */
            std::unique_ptr<RemapTexAddResource> makeResModel(const std::string& iniFolder, const std::string& path,
                                                              const std::string& resType) const;

            /**
             * @brief
             @rst
             Builds the ``[Resource...]`` `section`_ for a created texture --- a single ``filename =`` `KVP`_ naming
             the ``.dds`` this edit writes (#fileOfSection), or nothing for a name \ref files does not list
             @endrst
             *
             * @param sectionName The name of the section to build
             * @param modName The name of the mod being fixed to
             *
             * @return The section, owned by this edit
             */
            Section* buildSection(const std::string& sectionName, const std::string& modName) override;

            /**
             * @brief Builds the resource of #makeResModel and files it with 'ctx'
             */
            void buildResModel(const std::string& resType, const std::string& srcPath,
                                const std::string& fixedPath, const std::string& modName,
                                const std::string& fileKey, Context& ctx) override;

            /**
             * @brief Resets the texture counter \ref getFixResourceName advances
             */
            void clear() override;

            /**
             * @brief
             @rst
             The fixed name of the resource `section`_ -- ``<capitalized modName><texName>``, with a
             counter suffix from the second DISTINCT texture onwards, run through
             :cpp:func:`IniNamingTools::getRemapTexResourceName` :raw-html:`<br />` :raw-html:`<br />`

             **Memoised per 'modName'**: asked for the same mod twice, this hands back the same
             name and does not advance the counter :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                That memoisation is the whole point, and it is not a micro-optimisation. A
                :cpp:class:`TexCreate` holds ONE creator, so every texture it makes is identical
                by construction -- there is nothing for a counter to tell apart. Without the
                memo, a mod whose graph binds the register in several branches asks once per
                branch and gets ``NormalMap``, ``NormalMap1``, ``NormalMap2``, ``NormalMap3``:
                four byte-identical 4MB ``.dds`` files where one would do.

             .. note::
                The counter still exists and still numbers genuinely different textures apart --
                a second mod name mints a second name. \ref clear resets both it and the memo
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
                fixed path just computed -- so the fixed name is discarded in that case. Every
                sibling appends it to the fixed path instead; this one is kept as is because
                changing it would rename real output files
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
            // Both mutable because getFixResourceName is const (it overrides a const virtual) yet
            // has to fill the memo and advance the counter -- see its own notes.
            mutable int texInd_ = 0;

            // modName -> the name already minted for it. One entry in every real fix; the map is
            // what makes asking twice cost nothing rather than costing a file.
            mutable std::unordered_map<std::string, std::string> names_;

            // The sections #buildSection built, cleared with the counter
            std::vector<std::unique_ptr<Section>> sections_;
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

     The texture is edited by \ref texEditor, held through a ``std::shared_ptr`` to its base and shared by every
     resource built, so a subclass's own :cpp:func:`BaseTexEditor::fix` -- a `Python`_ :class:`TexEditor`'s
     included -- is the one that runs (#makeResModel)
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
            using Context = typename Base::Context;

            /**
             * @brief The name of the subtype of the resource, folded into every fixed name
             */
            std::optional<std::string> resSubType;

            /**
             * @brief
             @rst
             The editor run over every texture this resource edit collects -- held through its base, so a
             subclass's :cpp:func:`BaseTexEditor::fix` (a `Python`_ one included) is the one that runs, and
             shared by every resource built
             @endrst
             */
            std::shared_ptr<BaseTexEditor> texEditor;

            /**
             * @brief What fixes each built resource in place of running \ref texEditor, or empty. **Default**: empty
             */
            std::function<bool(RemapTexEditResource&)> fixFunc;

            /**
             * @brief Constructs a new editing texture resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param texEditor The editor to run over each collected texture
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexEdit"``
             * @param resSubType The name of the subtype of the resource, folded into every fixed name. **Default**: none
             */
            TexReplace(GraphId resModObj, std::shared_ptr<BaseTexEditor> texEditor, ResEditConfig config,
                       std::string resType = "resourceRemapTexEdit", std::optional<std::string> resSubType = std::nullopt);

            /**
             * @brief
             @rst
             The :cpp:class:`RemapTexEditResource` that reads 'srcPath', runs \ref texEditor (or \ref fixFunc) over it
             and writes 'fixedPath'
             @endrst
             *
             * @param iniFolder The folder of the ``.ini`` file the resource belongs to
             * @param srcPath The texture to read
             * @param fixedPath Where the edited texture is written
             * @param resType The name of the type of resource
             *
             * @return The resource
             */
            std::unique_ptr<RemapTexEditResource> makeResModel(const std::string& iniFolder, const std::string& srcPath,
                                                               const std::string& fixedPath, const std::string& resType) const;

            /**
             * @brief Builds the resource of #makeResModel and files it with 'ctx'
             */
            void buildResModel(const std::string& resType, const std::string& srcPath,
                                const std::string& fixedPath, const std::string& modName,
                                const std::string& fileKey, Context& ctx) override;

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
             * @brief 'modName' capitalized, extended by \ref resSubType when there is one
             */
            std::string subTypedModName(const std::string& modName) const;

            /**
             * @brief
             @rst
             The mod object this edit belongs to, which the fixed FILE name carries -- see
             \ref getFixFile :raw-html:`<br />` :raw-html:`<br />`

             Optional, and only the character template sets it. Left empty the file name falls back
             to being built from the source resource, which is what every caller got before edits
             could collide
             @endrst
             */
            std::string modObj;
    };
}

#include "TexEdit.tpp"

#endif
