#ifndef AGRemapCore_IniFixContext_H
#define AGRemapCore_IniFixContext_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/IIniGraphGroups.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The ``.ini`` file a fixer is fixing, behind an interface :raw-html:`<br />` :raw-html:`<br />`

     The fixing counterpart of :cpp:class:`IniParseContext`, and it exists for exactly the same
     reason -- see that class's own note. Where a parser mostly *reads* `sections`_, a fixer mostly
     drives the ``.ini`` file's own surrounding machinery: the boilerplate it wraps a fix in, the
     backup it disables, the text it hides, and the files it writes. All of that lives on the
     *`Python`_* ``IniFile``, which has no C++ counterpart to call into.

     :raw-html:`<br />`

     .. note::
        #makeGraphGroups is here rather than being something :cpp:class:`GIMIFixer` does for itself
        for the reason :cpp:class:`IIniGraphGroups`'s own ownership contract spells out: a plain
        C++ caller's groups are owned C++ values, while the `Python`_-facing ones are a `Python`_
        ``list`` of ``IniGraphGroup`` objects whose ``graphs`` really is a `Python`_ ``dict``.
        Each fixer owns the one it is handed, so handing a chain of fixers' groups along
        (:cpp:member:`GIMIFixer::prevFixer`) is a pointer move rather than a copy
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniFixContext {
        public:

            /**
             * @brief Where the groups a fixer edits live -- see this class's own note
             */
            using GraphGroups = IIniGraphGroups<K, V, KeyHash, KeyEqual>;

            virtual ~IniFixContext() = default;

            /**
             * @brief Whether there is a real ``.ini`` file behind this context
             */
            virtual bool hasIni() const = 0;

            /**
             * @brief
             @rst
             The names of the mods this ``.ini`` file should be fixed to -- the equivalent of the
             pure-Python original's ``ini.availableType.getModsToFix()``, empty when the ``.ini``
             file was never classified :raw-html:`<br />` :raw-html:`<br />`

             Only consulted when :cpp:member:`GIMIFixer::modsToFix` was not set explicitly
             @endrst
             */
            virtual std::vector<std::string> modsToFix() const = 0;

            /**
             * @brief
             @rst
             Where the fix for the group at 'groupInd' should be written, or ``std::nullopt`` when
             the ``.ini`` file has no path at all :raw-html:`<br />` :raw-html:`<br />`

             Group ``0`` is the ``.ini`` file's own path; every later group is a *copy*, named by
             appending the ``RemapFixCopy`` suffix and the index to the base name -- the equivalent
             of the pure-Python original mutating ``iniFilePath.baseName`` as it walks the groups
             @endrst
             *
             * @param groupInd Which group's destination to build
             */
            virtual std::optional<std::string> fixedFilePath(std::size_t groupInd) const = 0;

            /**
             * @brief Whether the ``.ini`` file already exists on disk (``os.path.exists(filePath.path)``)
             */
            virtual bool fixedFileExists() const = 0;

            /**
             * @brief The text content of the ``.ini`` file (``ini.fileTxt``)
             */
            virtual std::string fileTxt() const = 0;

            /**
             * @copydoc fileTxt() const
             *
             * @param txt The new text content
             */
            virtual void setFileTxt(std::string txt) = 0;

            /**
             * @brief
             @rst
             Comments out the original mod's `sections`_ so only the remapped mod is displayed
             (``ini.hideOriginalSections()``) :raw-html:`<br />` :raw-html:`<br />`

             Which `sections`_ those are is the *fixer's* answer, not the ``.ini`` file's: they are
             the ones the fix it just built actually touched. The pure-Python original splits the
             same two halves across two objects -- its ``GIMIFixer`` fills the ``.ini`` file's
             ``_remappedSectionNames`` as it renders, and ``ini.hideOriginalSections()`` then
             comments out whatever ended up in there -- so an implementation of this that forwards
             to a `Python`_ ``IniFile`` fills that set first

             :raw-html:`<br />`

             .. note::
                'sectionNames' never holds a resource `section`_, only the ``run =`` command chains
                the fix replaces. That is deliberate and load-bearing: a fix can carry a register
                over verbatim, pointing at one of the original mod's own resource `sections`_, and
                commenting that out would break the very fix this is protecting
             @endrst
             *
             * @param sectionNames The names of the `sections`_ to comment out
             */
            virtual void hideOriginalSections(const std::unordered_set<std::string>& sectionNames) = 0;

            /**
             * @brief Disables the existing ``.ini`` file, keeping it as a backup (``ini.disIni()``)
             */
            virtual void disableIni() = 0;

            /**
             * @brief Writes one line to the ``.ini`` file's own log (``ini.print("log", ...)``)
             *
             * @param message The message to log
             */
            virtual void log(const std::string& message) = 0;

            /**
             * @brief
             @rst
             Wraps 'fix' in the header/credit/footer boilerplate that marks it as this software's
             work (``ini.addFixBoilerPlate(fix = ...)``)
             @endrst
             *
             * @param fix The content of the fix
             */
            virtual std::string addFixBoilerPlate(const std::string& fix) const = 0;

            /**
             * @brief Writes one finished ``.ini`` file to disk
             *
             * @param path Where to write it -- one of #fixedFilePath's results
             * @param content What to write
             */
            virtual void writeFixedFile(const std::string& path, const std::string& content) = 0;

            /**
             * @brief
             @rst
             Records that the ``.ini`` file has now been fixed (``ini._isFixed = True``)
             :raw-html:`<br />` :raw-html:`<br />`

             :cpp:class:`AGRemapCore::IniFile` deliberately does not let a fixer write its own
             ``isFixed`` (see :cpp:class:`BaseIniFixer`'s own note), so a plain C++ implementation
             of this is free to do nothing. The `Python`_ ``IniFile`` uses the flag the pure-Python
             original's way, and its implementation does set it
             @endrst
             *
             * @param isFixed Whether the .ini file has been fixed
             */
            virtual void setIsFixed(bool isFixed) = 0;

            /**
             * @brief
             @rst
             Builds a fresh, empty place for one fixer's graph groups to live -- see this class's
             own note on why this isn't something the fixer does for itself
             @endrst
             */
            virtual std::unique_ptr<GraphGroups> makeGraphGroups() = 0;
    };
}

#endif
