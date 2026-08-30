#ifndef AGRemapCore_BaseIniFixer_H
#define AGRemapCore_BaseIniFixer_H

#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     Base class to fix a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Only :cpp:class:`GIMIFixer` is ported so far -- none of the pure-Python original's other
        concrete subclasses (``GIMIObjSplitFixer``, ``GIMIObjMergeFixer``, ``MultiModFixer``, ...)
        exist in C++ yet

     .. note::
        This is a class template over the same ``K``/``V``/``KeyHash``/``KeyEqual`` as the
        :cpp:class:`IfContentPart`\s the `sections`_ it fixes are made of, defaulting to
        ``<std::string, std::string>``. It has to be, for the same reason
        :cpp:class:`BaseIniParser` does: the `pybind11`_ layer's `sections`_ are
        ``IfTemplate<py::object, py::object, ...>``, so a fixer pinned to
        ``<std::string, std::string>`` would be unreachable from any binding

     .. note::
        :cpp:class:`IniFixBuilder` (and, through it, :cpp:member:`ModType::iniFixBuilder` and
        :cpp:func:`IniFile::getFixers`) deliberately stays pinned to ``BaseIniFixer<>``, for the
        same reason :cpp:class:`IniParseBuilder` does -- see that class's own note

     .. note::
        Divergences from the pure-Python
        ``FixRaidenBoss2.model.strategies.iniFixers.BaseIniFixer`` original:

        * #fix always returns a map of file path to new content. The original returns
          ``Union[str, Dict[Union[str, int], str]]`` -- either a single ``.ini`` file's new content,
          or several keyed by path *or* by an integer id when a path isn't available. The single-file
          case is just a one-entry map here, and the integer-id fallback is dropped, so callers
          never have to unpack a union
        * The pure-Python original's ``fix`` sets ``self._iniFile._isFixed = True`` after running.
          That's deliberately **not** done here -- :cpp:member:`IniFile::isFixed` is currently owned
          by :cpp:func:`IniFile::classify` (it means "this .ini file was *detected* as already
          fixed"), so having a fixer write to it would conflate two different meanings. The
          `pybind11`_ layer *does* set it, because the `Python`_ ``IniFile`` uses the flag the
          original's way
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseIniFixer {
        public:

            /**
             * @brief The type of parser this fixer retrieves its data from
             */
            using Parser = BaseIniParser<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The result of a fix -- the new content of the fixed ``.ini`` file(s), keyed by the file
             path each one should be written to
             @endrst
             */
            using FixResult = std::unordered_map<std::string, std::string>;

            /**
             * @brief
             @rst
             The parse data a fix works from -- exactly what this fixer's associated
             :cpp:func:`BaseIniParser::parse` produced, one :cpp:class:`IniGraphGroup` per ``.ini``
             file :raw-html:`<br />` :raw-html:`<br />`

             Passed by **non-const** reference throughout: a fixer edits the graphs in place (that is
             what every ``graphEdits``/``graphGroupEdits`` filter does), and
             :cpp:class:`IniGraphGroup` is move-only so it could not be passed by value anyway
             @endrst
             */
            using ParseData = std::vector<IniGraphGroup<K, V, KeyHash, KeyEqual>>;

            /**
             * @brief Constructs a new fixer
             *
             * @param parser
             @rst
             The associated parser to retrieve data for the fix :raw-html:`<br />` :raw-html:`<br />`

             This is a non-owning pointer to a parser owned elsewhere -- it must outlive this fixer.
             #getIniFile is taken from it, matching the pure-Python original's own
             ``self._iniFile = parser._iniFile`` :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` is allowed so a fixer can exist before it's bound to a parser; call
             #setParser before #fix in that case :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A fixer reached through a :cpp:class:`ModType` is never in that state --
                :cpp:member:`ModType::iniFixBuilder` builds one per ``.ini`` file, already bound to
                that file's own parser (see :cpp:func:`IniFixBuilder::build`), rather than sharing
                one unbound instance across every file of that mod type

             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit BaseIniFixer(Parser* parser = nullptr);

            virtual ~BaseIniFixer() = default;

            /**
             * @brief The associated parser this fixer retrieves data from, or ``nullptr`` if unbound
             */
            Parser* getParser() const;

            /**
             * @brief
             @rst
             Binds this fixer to the parser it retrieves data from, and re-reads #getIniFile from
             it -- non-owning, see the constructor
             @endrst
             *
             * @param parser The parser to use, or ``nullptr`` to unbind
             */
            void setParser(Parser* parser);

            /**
             * @brief The .ini file this fixer fixes -- taken from #getParser
             */
            IniFile* getIniFile() const;

            /**
             * @brief Resets any saved states within the fixer. No-op by default
             */
            virtual void clear();

            /**
             * @brief
             @rst
             Fixes the ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

             Thin non-virtual wrapper over #fixImpl, matching how the pure-Python original's public
             ``fix`` delegates to its own ``_fix`` with ``withBoilerPlate``/``withSrc`` both forced
             to ``true`` -- those two are internal knobs only the fixing pipeline itself varies, so
             they aren't part of this public entry point. Override #fixImpl, not this
             @endrst
             *
             * @param parseData The parse data to fix from -- see #ParseData
             * @param keepBackup Whether to keep backups for the .ini file. **Default**: ``true``
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes. **Default**: ``false``
             * @param hideOrig Whether to hide the mod for the original character. **Default**: ``false``
             *
             * @return The new content of the fixed .ini file(s), keyed by file path
             */
            FixResult fix(ParseData& parseData, bool keepBackup = true, bool fixOnly = false, bool hideOrig = false);

        protected:

            /**
             * @brief
             @rst
             Does the actual fixing -- the customization point behind #fix, and the direct
             equivalent of the pure-Python original's ``_fix``. Returns an empty map by default,
             matching that original's ``pass``
             @endrst
             *
             * @param parseData The parse data to fix from -- see #ParseData
             * @param keepBackup Whether to keep backups for the .ini file
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes
             * @param hideOrig Whether to hide the mod for the original character
             * @param withBoilerPlate Whether to include the surrounding boilerplate in the result
             * @param withSrc Whether to include the .ini file's original content in the result
             *
             * @return The new content of the fixed .ini file(s), keyed by file path
             */
            virtual FixResult fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig, bool withBoilerPlate, bool withSrc);

            /**
             * @brief The associated parser to retrieve data for the fix -- non-owning, see the constructor
             */
            Parser* parser_;

            /**
             * @brief The .ini file that will be fixed -- non-owning, taken from #parser_
             */
            IniFile* iniFile_;
    };
}

#include "BaseIniFixer.tpp"

#endif
