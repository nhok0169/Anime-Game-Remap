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

#ifndef AGRemapCore_FileDownloadEdit_H
#define AGRemapCore_FileDownloadEdit_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`ResCreate`

     Downloads a file the fix needs -- eg. the source character's own game texture for a role a mod
     ships no file for -- together with the ``[Resource...]`` `section`_ that names it
     :raw-html:`<br />` :raw-html:`<br />`

     The download counterpart of :cpp:class:`FileAddCreate`, and used the same way, through
     :cpp:class:`ResRegCollect`: the collect finds the register naming the resource (eg.
     ``this = ResourceMiyabiBlossomBodyRemapDLDiffuse``), and this edit builds the `section`_ of that
     name from \ref files and a :cpp:class:`RemapIniDownload` that fetches the file

     .. code-block:: cpp

        fallbackCollect_.srcRegs  = {{fallbackGraph, "this"}};
        fallbackCollect_.resEdits = {{"download", fallbackDownload_.get()}};

     .. note::
        The `section`_ keeps the name the register gives it, and a file keeps the name \ref files
        gives it. Name a download with :cpp:func:`DownloadTools::fixedFileName` (``...RemapDL...``),
        the naming an undo recognises a download by. A name \ref files does not list builds no
        `section`_

     .. note::
        A download is fetched in the order the resources were collected. An edit of the downloaded
        texture itself is ef File::texEdit: a second collect cannot reach a `section`_ this one
        created
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class FileDownloadCreate: public ResCreate<K, V, KeyHash, KeyEqual> {
        public:
            using Base = ResCreate<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;
            using Section = typename Base::Section;

            /**
             * @brief One downloaded file and the `section`_ that names it
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
                 * @brief Where the file is downloaded from -- see :cpp:func:`DownloadTools::urlPath`
                 */
                std::string url;

                /**
                 * @brief
                 @rst
                 An edit of the downloaded texture -- eg. a light map's bands moved for the target. Set,
                 the download goes to ef rawFile and the edit writes ef file from it, so the
                 `section`_ names the EDITED file. **Default**: none, the download is ef file itself
                 @endrst
                 */
                std::optional<TexEditor> texEdit;

                /**
                 * @brief
                 @rst
                 Where the download goes when there is a ef texEdit, relative to the ``.ini`` file. Name
                 it ``...RemapDL...`` too: no `section`_ names it, and the undo recognises it by that name
                 @endrst
                 */
                std::string rawFile;
            };

            /**
             * @brief The files to download, keyed by the name of the `section`_ that names each
             */
            std::unordered_map<std::string, File> files;

            /**
             * @brief Constructs a new file-downloading resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resources
             * @param files The files to download, keyed by the name of the `section`_ that names each
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"download"``
             */
            FileDownloadCreate(GraphId resModObj, std::unordered_map<std::string, File> files, ResEditConfig config,
                               std::string resType = "download");

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
             * @brief Builds the :cpp:class:`RemapIniDownload` that fetches 'srcPath'
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
}

#include "FileDownloadEdit.tpp"

#endif
