#ifndef AGRemapCore_IniParseDownloadData_H
#define AGRemapCore_IniParseDownloadData_H

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
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     One file a mod is missing and has to download, behind an interface :raw-html:`<br />`
     :raw-html:`<br />`

     A parser asks it for three things: how a `section`_ references the download (#addToPart /
     #addToSection), the resource `section`_ that names the file (#createResSection), and the
     download itself (#createFileDownload). The last two are BUILDERS -- the parser registers what they
     return with the ``.ini`` file -- so a subclass, in C++ or in `Python`_, only says what to build.
     :cpp:class:`DownloadData` is the implementation every parser uses
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniParseDownloadData {
        public:

            /**
             * @brief The type of `section`_ a resource download lives in
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of `KVP`_ block a download reference is added to
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            virtual ~IniParseDownloadData() = default;

            /**
             * @brief The name of the download resource in the ``.ini`` file
             */
            virtual std::string name() const = 0;

            /**
             * @brief
             @rst
             Whether to add the download reference to only the top of some `section`_ (``true``),
             or to every :cpp:class:`IfContentPart` of the `section`_ that needs it (``false``)
             @endrst
             */
            virtual bool refToSection() const = 0;

            /**
             * @brief Adds a reference to this download into one :cpp:class:`IfContentPart`
             *
             * @param part The part to add the reference to
             * @param key The key of the download reference `KVP`_
             * @param val The value of the download reference `KVP`_
             */
            virtual void addToPart(ContentPart& part, const K& key, const V& val) = 0;

            /**
             * @brief Adds a reference to this download to the front of one `section`_
             *
             * @param section The `section`_ to add the reference to
             * @param key The key of the download reference `KVP`_
             * @param val The value of the download reference `KVP`_
             */
            virtual void addToSection(Section& section, const K& key, const V& val) = 0;

            /**
             * @brief Builds the `section`_ that holds this download's resource -- the parser registers it
             *
             * @param sectionName The name of the `section`_ to build
             *
             * @return The built `section`_
             */
            virtual std::unique_ptr<Section> createResSection(const std::string& sectionName) const = 0;

            /**
             * @brief Builds the download of the file -- the parser hands it to the ``.ini`` file
             *
             * @param iniFolder The folder the ``.ini`` file lives in
             *
             * @return The download, or ``nullptr`` if there is nothing to download
             */
            virtual std::unique_ptr<IniResource> createFileDownload(const std::string& iniFolder) const = 0;
    };


    /**
     * @brief
     @rst
     A file to download, and the resource `section`_ that names it :raw-html:`<br />` :raw-html:`<br />`

     The resource `section`_ is ``filename = <the file>`` after any #resourceKVPs, and every part that
     references the download also gets #downloadRefKVPs after the reference -- eg. a downloaded
     ``Blend.buf``'s ``handling = skip`` / ``draw``, which #blendRefKVPs gives
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class DownloadData: public IniParseDownloadData<K, V, KeyHash, KeyEqual> {
        public:
            using Base = IniParseDownloadData<K, V, KeyHash, KeyEqual>;
            using Section = typename Base::Section;
            using ContentPart = typename Base::ContentPart;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs, for the same reason
             :cpp:class:`IfTemplateRunConfig` exists: ``K``/``V`` are not ``std::string`` for every
             instantiation, so the ``filename`` `KVP`_ key can't be spelled as a literal here
             @endrst
             */
            struct DownloadConfig {
                /**
                 * @brief The `KVP`_ key naming the file a resource `section`_ points at
                 */
                K filenameKey;

                /**
                 * @brief Converts a file path into the `KVP`_ value that names it
                 */
                std::function<V(const std::string&)> valOfPath;

                /**
                 * @brief The run configuration any `section`_ built by this class uses
                 */
                IfTemplateRunConfig<K, V> runConfig;
            };

            /**
             * @brief Constructs new download data
             *
             * @param name The name of the download resource in the .ini file
             * @param download The file download to initiate -- ownership is transferred into this object
             * @param config The .ini-domain customization points to use
             * @param refToSection Whether to reference the download from the top of a `section`_ instead of from each of its parts. **Default**: ``false``
             * @param downloadRefKVPs Any additional `KVPs`_ to add after the download reference. **Default**: empty
             * @param resourceKVPs Any additional `KVPs`_ to add before the download's file path. **Default**: empty
             */
            DownloadData(std::string name, std::unique_ptr<FileDownload> download, DownloadConfig config, bool refToSection = false,
                          std::vector<std::pair<K, V>> downloadRefKVPs = {}, std::vector<std::pair<K, V>> resourceKVPs = {});

            std::string name() const override;
            bool refToSection() const override;
            void addToPart(ContentPart& part, const K& key, const V& val) override;
            void addToSection(Section& section, const K& key, const V& val) override;
            std::unique_ptr<Section> createResSection(const std::string& sectionName) const override;
            std::unique_ptr<IniResource> createFileDownload(const std::string& iniFolder) const override;

            /**
             * @brief The file download to initiate
             */
            const FileDownload* download() const;

            /**
             * @brief The `KVPs`_ added after each reference to the download
             */
            const std::vector<std::pair<K, V>>& downloadRefKVPs() const;

            /**
             * @brief The `KVPs`_ added before the file path in the resource `section`_
             */
            const std::vector<std::pair<K, V>>& resourceKVPs() const;

            /**
             * @brief
             @rst
             The two `KVPs`_ that must follow a reference to a downloaded ``Blend.buf``:
             ``handling = skip`` and ``draw = <vertexCount>,0``. The ``.ini`` file it is dropped into has
             no draw of its own for it, so without them the blend is bound and never drawn from
             @endrst
             *
             * @param vertexCount How many vertices the drawn model has
             */
            static std::vector<std::pair<K, V>> blendRefKVPs(long long vertexCount);

        private:
            std::string name_;
            std::unique_ptr<FileDownload> download_;
            DownloadConfig config_;
            bool refToSection_;
            std::vector<std::pair<K, V>> downloadRefKVPs_;
            std::vector<std::pair<K, V>> resourceKVPs_;
    };
}

#include "IniParseDownloadData.tpp"

namespace AGRemapCore {
    // note: every translation unit that uses these with <std::string, std::string> would otherwise
    //   instantiate all of their member bodies itself; they are compiled once, in
    //   src/model/strategies/iniParsers/IniParseDownloadDataInstantiation.cpp, instead. Any other instantiation still happens implicitly.
    extern template class IniParseDownloadData<std::string, std::string>;
    extern template class DownloadData<std::string, std::string>;
}

#endif
