#ifndef AGRemapCore_IniParseDownloadData_H
#define AGRemapCore_IniParseDownloadData_H

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
     One file a mod is missing and has to download, behind an interface -- the seam a parser
     reaches the pure-Python ``DownloadData`` (``model/DownloadData.py``) through
     :raw-html:`<br />` :raw-html:`<br />`

     **Why this is an interface rather than a concrete class.** ``DownloadData`` is still pure
     `Python`_, and (unlike most collaborator types this port has met) it is genuinely
     *polymorphic* there: ``BlendDownloadData`` overrides ``addToPart`` to append its own
     ``handling``/``draw`` `KVPs`_, and a user is free to subclass it further. Converting one into
     a plain C++ struct at the binding boundary would silently run the base class's behaviour for
     every subclass, so the parser talks to this interface and the binding layer supplies an
     implementation that calls straight back into the real `Python`_ object

     :raw-html:`<br />`

     .. note::
        #createResSection and #addFileDownload both take the :cpp:class:`IniParseContext` rather
        than returning something the parser then files away. That's deliberate: what a resource
        `section`_/download *is* on the `Python`_ side is a `Python`_ object, and only an
        implementation that already holds `Python`_ objects can put one into
        ``ini.sectionIfTemplates``/``ini.fileDownloads`` without losing it. The parser only ever
        sees the borrowed :cpp:type:`Section` pointer that comes back
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

            /**
             * @brief The ``.ini`` file this download belongs to
             */
            using Context = IniParseContext<K, V, KeyHash, KeyEqual>;

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
             * @brief
             @rst
             Builds the `section`_ that holds this download's resource and registers it with the
             ``.ini`` file (see this class's own note on why 'ctx' is a parameter)
             @endrst
             *
             * @param sectionName The name of the `section`_ to build
             * @param ctx The ``.ini`` file to register the new `section`_ with
             *
             * @return The built `section`_ -- borrowed, owned by 'ctx'
             */
            virtual Section* createResSection(const std::string& sectionName, Context& ctx) = 0;

            /**
             * @brief
             @rst
             Records the actual file download with the ``.ini`` file -- the equivalent of the
             pure-Python original's
             ``ini.fileDownloads.append(RemapIniDownload(iniFolder, dd.download.filename, dd.download))``
             @endrst
             *
             * @param ctx The ``.ini`` file to record the download with
             * @param iniFolder The folder the ``.ini`` file lives in
             */
            virtual void addFileDownload(Context& ctx, const std::string& iniFolder) = 0;
    };


    /**
     * @brief
     @rst
     The plain-C++ :cpp:class:`IniParseDownloadData` -- the direct counterpart of the pure-Python
     ``DownloadData`` for callers that aren't going through `Python`_ at all :raw-html:`<br />`
     :raw-html:`<br />`

     Use this from any standalone C++ caller. The `pybind11`_ layer uses its own
     ``PyIniParseDownloadData`` implementation instead, which calls back into the real `Python`_
     ``DownloadData`` object -- see :cpp:class:`IniParseDownloadData`'s own note for why there are
     two
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
            using Context = typename Base::Context;

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
            Section* createResSection(const std::string& sectionName, Context& ctx) override;
            void addFileDownload(Context& ctx, const std::string& iniFolder) override;

            /**
             * @brief The file download to initiate
             */
            const FileDownload* download() const;

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

#endif
