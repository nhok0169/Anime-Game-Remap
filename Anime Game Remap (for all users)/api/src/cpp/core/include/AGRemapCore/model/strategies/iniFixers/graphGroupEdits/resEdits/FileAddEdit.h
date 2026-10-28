#ifndef AGRemapCore_FileAddEdit_H
#define AGRemapCore_FileAddEdit_H

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

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iniresources/RemapFileResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResCreate`

     Creates a brand new file of contents the fix already knows -- eg. the constant buffer a
     ``CustomShader`` reads -- together with the ``[Resource...]`` `section`_ that names it
     :raw-html:`<br />` :raw-html:`<br />`

     Use it through :cpp:class:`ResRegCollect`, as :cpp:class:`TexCreatorCreate` is: the collect finds
     the register naming the resource (eg. ``ps-cb4 = ResourceShaderCB4``), and this edit builds the
     `section`_ of that name from \ref files and a :cpp:class:`RemapFileAddResource` that writes the
     file

     .. code-block:: cpp

        bufferCollect_.srcRegs  = {{shaderGraph, "ps-cb4"}};
        bufferCollect_.resEdits = {{"buffer", bufferCreate_.get()}};

     .. note::
        The `section`_ keeps the name the register gives it (\ref getFixResourceName answers
        ``std::nullopt``): the caller names the resource when it writes the register, so there is
        nothing left to rename. A name \ref files does not list builds no `section`_
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class FileAddCreate: public ResCreate<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResCreate<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;
            using Section = typename Base::Section;

            /**
             * @brief One created file and the `section`_ that names it
             */
            struct File {
                /**
                 * @brief The `section`_'s `KVPs`_ other than its ``filename``, eg. ``type = Buffer``
                 */
                std::vector<std::pair<K, V>> keys;

                /**
                 * @brief The file's path, relative to the ``.ini`` file
                 */
                std::string file;

                /**
                 * @brief The file's contents
                 */
                std::vector<std::uint8_t> bytes;

                /**
                 * @brief
                 @rst
                 The contents computed when the file is written -- see
                 :cpp:member:`RemapFileAddResource::makeBytes` (a buffer made from files the run downloads
                 first). Set, it replaces ef bytes. **Default**: empty
                 @endrst
                 */
                std::function<std::vector<std::uint8_t>()> makeBytes;
            };

            /**
             * @brief The files to create, keyed by the name of the `section`_ that names each
             */
            std::unordered_map<std::string, File> files;

            /**
             * @brief Constructs a new file-creating resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resources
             * @param files The files to create, keyed by the name of the `section`_ that names each
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapFileAdd"``
             */
            FileAddCreate(GraphId resModObj, std::unordered_map<std::string, File> files, ResEditConfig config,
                          std::string resType = "resourceRemapFileAdd");

            /**
             * @brief ``std::nullopt``: the resource keeps the name its register gives it
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief
             @rst
             Builds the `section`_ \ref files lists under ``sectionName`` -- its \ref File::keys, then
             a ``filename =`` naming \ref File::file
             @endrst
             *
             * @param sectionName The name of the section to build
             * @param modName The name of the mod being fixed to
             *
             * @return The section, owned by this edit, or ``nullptr`` when \ref files does not list it
             */
            Section* buildSection(const std::string& sectionName, const std::string& modName) override;

            /**
             * @brief Builds the :cpp:class:`RemapFileAddResource` that writes 'srcPath'
             */
            void buildResModel(const std::string& resType, const std::string& srcPath,
                               const std::string& fixedPath, const std::string& modName,
                               const std::string& fileKey, Context& ctx) override;

            /**
             * @brief Clears every section this edit built
             */
            void clear() override;

        private:
            std::vector<std::unique_ptr<Section>> sections_;
    };


    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseResEdit`

     Creates a brand new file that a register names DIRECTLY by its path -- 3DMigoto's
     ``ps = <file>.hlsl`` in a ``CustomShader``, which takes no ``[Resource...]`` `section`_
     :raw-html:`<br />` :raw-html:`<br />`

     Use it through :cpp:class:`ResRegCollect` like any other resource edit: the collect hands over
     each value of the register, which here is the file itself, and this edit builds a
     :cpp:class:`RemapFileAddResource` for every value \ref files lists. It builds no graph and
     changes no register

     .. code-block:: cpp

        shaderCollect_.srcRegs  = {{shaderGraph, "ps"}};
        shaderCollect_.resEdits = {{"shader", shaderAdd_.get()}};
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class FileRefAdd: public BaseResEdit<K, V, KeyHash, KeyEqual> {
        public:
            using Base = BaseResEdit<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;
            using CollectedSections = typename Base::CollectedSections;
            using GraphGroups = typename Base::GraphGroups;
            using ResourceFilter = typename Base::ResourceFilter;

            /**
             * @brief The contents of each file to create, keyed by its path as the register gives it (relative to the ``.ini`` file)
             */
            std::unordered_map<std::string, std::vector<std::uint8_t>> files;

            /**
             * @brief Constructs a new file-adding resource edit
             *
             * @param files The contents of each file to create, keyed by its path as the register gives it
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapFileAdd"``
             */
            FileRefAdd(std::unordered_map<std::string, std::vector<std::uint8_t>> files, ResEditConfig config,
                       std::string resType = "resourceRemapFileAdd");

            /**
             * @brief ``std::nullopt``: the register's value is the file, and stays as it is
             */
            std::optional<std::string> getFixResourceName(const std::string& resource, const std::string& modName = "") const override;

            /**
             * @brief
             @rst
             Builds a :cpp:class:`RemapFileAddResource` for each collected file \ref files lists, and no
             graph
             @endrst
             */
            GraphGroups& buildResources(const CollectedSections& collectedSections, Context& ctx, GraphGroups& graphGroups,
                                        const std::string& modName = "", const ResourceFilter& resourceFilter = {},
                                        bool copySections = false) override;
    };
}

#include "FileAddEdit.tpp"

namespace AGRemapCore {
    // note: compiled once, in
    //   src/model/strategies/iniFixers/graphGroupEdits/resEdits/FileAddEditInstantiation.cpp, for every
    //   translation unit that uses these with <std::string, std::string>
    extern template class FileAddCreate<std::string, std::string>;
    extern template class FileRefAdd<std::string, std::string>;
}

#endif
