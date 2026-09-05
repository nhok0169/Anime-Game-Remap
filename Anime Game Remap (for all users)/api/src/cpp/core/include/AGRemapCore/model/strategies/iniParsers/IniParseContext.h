#ifndef AGRemapCore_IniParseContext_H
#define AGRemapCore_IniParseContext_H

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"
#include "AGRemapCore/tools/z3/Z3Context.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The ``.ini`` file a parser is parsing, behind an interface :raw-html:`<br />`
     :raw-html:`<br />`

     **Why this isn't just an** :cpp:class:`IniFile` **pointer.** Exactly the same reason
     :cpp:class:`IniResEditContext` isn't: the ``.ini`` file every real caller of
     :cpp:class:`GIMIParser` passes is the *`Python`_* ``IniFile`` (``model/files/IniFile.py``),
     an unrelated class to :cpp:class:`AGRemapCore::IniFile` with nothing castable between them --
     so a plain ``IniFile*`` parameter would always be ``nullptr`` here and the parser would be
     inert. Unlike the ``regEdits/`` family (where ``nullptr`` *is* the right answer, because those
     edits genuinely never read the ``.ini`` file), reading -- and writing -- the ``.ini`` file's
     `sections`_ is most of what a parser does.

     :raw-html:`<br />`

     .. note::
        Graph *creation* deliberately lives behind #graphGroups rather than being something
        :cpp:class:`GIMIParser` does for itself, for the reason
        :cpp:class:`IIniGraphGroups`'s own ownership contract spells out: the `Python`_
        implementation's graphs are `Python`_ objects with their own keep-alive bookkeeping, which
        cannot be reconstructed from a bare C++ graph after the fact. Every graph a parser
        produces is owned by the #graphGroups implementation and only ever *borrowed* by the parser
        :raw-html:`<br />` :raw-html:`<br />`

        Routing it through :cpp:class:`IIniGraphGroups` specifically (rather than a bespoke
        factory) is what makes :cpp:func:`GIMIParser::editCommands` work at all: the
        ``graphGroupEdits/`` edit it hands the command graphs to already speaks exactly that
        interface, so the parser's own graphs *are* the group being edited -- no conversion, and
        the `Python`_ dict aliasing ``GIMIParser.py``'s
        ``self.commandGraphs = graphGroups[0].graphs`` depends on is preserved for free
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniParseContext {
        public:

            /**
             * @brief The type of `section`_ a ``.ini`` file is made of
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of caller/callee graph a parser builds
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Where every graph a parser builds lives -- see this class's own note
             */
            using GraphGroups = IIniGraphGroups<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The kind of asset table :cpp:member:`ModType::hashes`/:cpp:member:`ModType::indices`
             are, as reached from here :raw-html:`<br />` :raw-html:`<br />`

             The ``ValueHash``/``ValueEqual`` parameters reuse ``KeyHash``/``KeyEqual``, matching
             what :cpp:class:`IniSectionGraph` already does for its own
             :cpp:class:`IfContentPartColouring` -- for both real instantiations
             (``<std::string, std::string>`` and ``<py::object, py::object>``) ``K`` and ``V`` are
             the same type, so one hasher genuinely serves both
             @endrst
             */
            using Assets = ModMappedAssets<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            virtual ~IniParseContext() = default;

            /**
             * @brief
             @rst
             Whether there is a real ``.ini`` file behind this context :raw-html:`<br />`
             :raw-html:`<br />`

             ``false`` stands in for the pure-Python original's ``ini = None``
             @endrst
             */
            virtual bool hasIni() const = 0;

            /**
             * @brief
             @rst
             The folder the ``.ini`` file lives in (the equivalent of the pure-Python original's
             ``ini.folder``), or an empty string when there is no ``.ini`` file
             @endrst
             */
            virtual std::string iniFolder() const = 0;

            /**
             * @brief
             @rst
             The game version the ``.ini`` file was written for (``ini.version``), or
             ``std::nullopt`` for "the latest"
             @endrst
             */
            virtual std::optional<Version> version() const = 0;

            /**
             * @brief The ``.ini`` file's download policy (``ini.downloadMode``)
             */
            virtual DownloadMode downloadMode() const = 0;

            /**
             * @brief
             @rst
             The one `Z3`_ context this ``.ini`` file owns (``ini._z3Ctx``), or ``nullptr``
             :raw-html:`<br />` :raw-html:`<br />`

             There is deliberately one context per ``.ini`` file, not one per
             :cpp:class:`IniSectionGraph` -- every graph built here borrows this one
             @endrst
             */
            virtual Z3Context* z3Ctx() const = 0;

            /**
             * @brief
             @rst
             Every `section`_ parsed out of the ``.ini`` file, keyed by name -- the equivalent of
             the pure-Python original's ``ini.sectionIfTemplates``. Borrowed, not owned
             @endrst
             */
            virtual std::unordered_map<std::string, Section*> sectionIfTemplates() const = 0;

            /**
             * @brief
             @rst
             The names of every `section`_, **in the order the ``.ini`` file declared them**
             :raw-html:`<br />` :raw-html:`<br />`

             Separate from #sectionIfTemplates specifically because that one is an
             ``std::unordered_map`` and this order is load-bearing: the pure-Python original
             iterates ``ini.sectionIfTemplates`` (a `Python`_ ``dict``, ie. insertion-ordered) both
             to classify `sections`_ by name and to seed :cpp:func:`GIMIParser::buildGlobalGraph`'s
             target list, and the rendered output of every graph built from it inherits that order
             @endrst
             */
            virtual std::vector<std::string> sectionNames() const = 0;

            /**
             * @brief The `section`_ named 'name', or ``nullptr`` if the ``.ini`` file has none
             *
             * @param name The name of the `section`_ to look up
             */
            virtual Section* getSection(const std::string& name) const = 0;

            /**
             * @brief
             @rst
             Adds a newly-built `section`_ to the ``.ini`` file, replacing any `section`_ already
             stored under 'name' :raw-html:`<br />` :raw-html:`<br />`

             The ``.ini`` file takes ownership; the returned pointer is borrowed and stays valid
             until the `section`_ is replaced or removed
             @endrst
             *
             * @param name The name to store it under
             * @param section The `section`_ to add
             *
             * @return The added `section`_
             */
            virtual Section* addSection(const std::string& name, std::unique_ptr<Section> section) = 0;

            /**
             * @brief Removes the `section`_ named 'name' from the ``.ini`` file, if there is one
             *
             * @param name The name of the `section`_ to remove
             */
            virtual void removeSection(const std::string& name) = 0;

            /**
             * @brief
             @rst
             Records one file the ``.ini`` file needs downloaded -- the equivalent of the
             pure-Python original's ``ini.fileDownloads.append(...)``
             @endrst
             *
             * @param download The download to record
             */
            virtual void addFileDownload(std::unique_ptr<IniResource> download) = 0;

            /**
             * @brief
             @rst
             Whether the ``.ini`` file was classified as some mod type at all (``ini.availableType
             is not None``)
             @endrst
             */
            virtual bool hasModType() const = 0;

            /**
             * @brief
             @rst
             The name of the mod type the ``.ini`` file was classified as
             (``ini.availableType.name``), or an empty string when there is none -- exactly the
             pure-Python original's own
             ``modTypeName = "" if (modType is None) else modType.name``
             @endrst
             */
            virtual std::string modTypeName() const = 0;

            /**
             * @brief
             @rst
             Reports one line of progress, wherever this environment's ``.ini`` file sends such
             things -- which may be nowhere at all :raw-html:`<br />` :raw-html:`<br />`

             The counterpart to :cpp:func:`IniFixContext::log`, and the same contract: a parser
             narrates through this rather than holding a view of its own, so it works identically
             whether it was reached from `Python`_ or from a plain C++ caller
             @endrst
             *
             * @param message The line to report
             */
            virtual void log(const std::string& message) = 0;

            /**
             * @brief
             @rst
             The mod type's ``hash`` assets (``ini.availableType.hashes``), or ``nullptr`` when the
             ``.ini`` file was not classified. Borrowed, not owned
             @endrst
             */
            virtual Assets* modTypeHashes() const = 0;

            /**
             * @brief
             @rst
             The mod type's ``match_first_index`` assets (``ini.availableType.indices``), or
             ``nullptr`` when the ``.ini`` file was not classified. Borrowed, not owned
             @endrst
             */
            virtual Assets* modTypeIndices() const = 0;

            /**
             * @brief
             @rst
             Where every graph a parser builds lives -- see this class's own note on why graph
             creation is here rather than in the parser
             @endrst
             */
            virtual GraphGroups& graphGroups() = 0;
    };
}

#endif
