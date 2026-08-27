#ifndef AGRemapCore_BaseIniFixer_H
#define AGRemapCore_BaseIniFixer_H

#include <string>
#include <unordered_map>


namespace AGRemapCore {

    class IniFile;
    class BaseIniParser;

    /**
     * @brief
     @rst
     Base class to fix a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Only the **base** class is ported -- none of the pure-Python original's concrete subclasses
        (``GIMIFixer``, ``GIMIObjSplitFixer``, ...) exist in C++ yet

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
          fixed"), so having a fixer write to it would conflate two different meanings. Revisit
          if/when the rest of the fixing pipeline is ported
     @endrst
     */
    class BaseIniFixer {
        public:

            /**
             * @brief
             @rst
             The result of a fix -- the new content of the fixed ``.ini`` file(s), keyed by the file
             path each one should be written to
             @endrst
             */
            using FixResult = std::unordered_map<std::string, std::string>;

            /**
             * @brief Constructs a new fixer
             *
             * @param parser
             @rst
             The associated parser to retrieve data for the fix :raw-html:`<br />` :raw-html:`<br />`

             This is a non-owning pointer to a parser owned elsewhere -- it must outlive this fixer.
             #getIniFile is taken from it, matching the pure-Python original's own
             ``self._iniFile = parser._iniFile`` :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` is allowed so a fixer can exist before it's bound to a parser -- see
             :cpp:member:`ModType::iniFixer`. Call #setParser before #fix in that case
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit BaseIniFixer(BaseIniParser* parser = nullptr);

            virtual ~BaseIniFixer() = default;

            /**
             * @brief The associated parser this fixer retrieves data from, or ``nullptr`` if unbound
             */
            BaseIniParser* getParser() const;

            /**
             * @brief
             @rst
             Binds this fixer to the parser it retrieves data from, and re-reads #getIniFile from
             it -- non-owning, see the constructor
             @endrst
             *
             * @param parser The parser to use, or ``nullptr`` to unbind
             */
            void setParser(BaseIniParser* parser);

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
             * @param keepBackup Whether to keep backups for the .ini file. **Default**: ``true``
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes. **Default**: ``false``
             * @param hideOrig Whether to hide the mod for the original character. **Default**: ``false``
             *
             * @return The new content of the fixed .ini file(s), keyed by file path
             */
            FixResult fix(bool keepBackup = true, bool fixOnly = false, bool hideOrig = false);

        protected:

            /**
             * @brief
             @rst
             Does the actual fixing -- the customization point behind #fix, and the direct
             equivalent of the pure-Python original's ``_fix``. Returns an empty map by default,
             matching that original's ``pass``
             @endrst
             *
             * @param keepBackup Whether to keep backups for the .ini file
             * @param fixOnly Whether to only fix the .ini file without undoing any fixes
             * @param hideOrig Whether to hide the mod for the original character
             * @param withBoilerPlate Whether to include the surrounding boilerplate in the result
             * @param withSrc Whether to include the .ini file's original content in the result
             *
             * @return The new content of the fixed .ini file(s), keyed by file path
             */
            virtual FixResult fixImpl(bool keepBackup, bool fixOnly, bool hideOrig, bool withBoilerPlate, bool withSrc);

            /**
             * @brief The associated parser to retrieve data for the fix -- non-owning, see the constructor
             */
            BaseIniParser* parser_;

            /**
             * @brief The .ini file that will be fixed -- non-owning, taken from #parser_
             */
            IniFile* iniFile_;
    };
}

#endif
