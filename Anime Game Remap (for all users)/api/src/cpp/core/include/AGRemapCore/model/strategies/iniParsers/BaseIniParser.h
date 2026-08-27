#ifndef AGRemapCore_BaseIniParser_H
#define AGRemapCore_BaseIniParser_H

#include <vector>

#include "AGRemapCore/model/IniGraphGroup.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     Base class to parse a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Two deliberate divergences from the pure-Python
        ``FixRaidenBoss2.model.strategies.iniParsers.BaseIniParser`` original:

        * There is no ``_modsToFix`` attribute (and so #clear has nothing of its own to reset, and
          is a no-op by default). The pure-Python original's ``clear`` exists *only* to clear that
          set, so subclasses here own whatever state they accumulate and reset it themselves
        * #parse returns the parsed :cpp:class:`IniGraphGroup`\\s instead of ``None``, so a parser
          hands its results back to its caller as a real value rather than by mutating the
          :cpp:class:`IniFile` it was given
     @endrst
     */
    class BaseIniParser {
        public:

            /**
             * @brief Constructs a new parser
             *
             * @param iniFile
             @rst
             The ``.ini`` file to parse :raw-html:`<br />` :raw-html:`<br />`

             This is a non-owning pointer to a file owned elsewhere -- it must outlive this parser
             :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` is allowed so a parser can exist before it's bound to any particular file
             -- :cpp:member:`ModType::iniParser` holds one this way, since a
             :cpp:class:`ModType` describes a *kind* of mod rather than one specific ``.ini`` file.
             Call #setIniFile before #parse in that case :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit BaseIniParser(IniFile* iniFile = nullptr);

            virtual ~BaseIniParser() = default;

            /**
             * @brief The .ini file this parser reads from, or ``nullptr`` if it hasn't been bound to one
             */
            IniFile* getIniFile() const;

            /**
             * @brief
             @rst
             Binds this parser to the ``.ini`` file it should read from -- non-owning, see the
             constructor
             @endrst
             *
             * @param iniFile The .ini file to parse, or ``nullptr`` to unbind
             */
            void setIniFile(IniFile* iniFile);

            /**
             * @brief
             @rst
             Clears any saved data. No-op by default -- see this class's own note on why there's
             nothing here to clear, unlike the pure-Python original
             @endrst
             */
            virtual void clear();

            /**
             * @brief
             @rst
             Parses the ``.ini`` file. Returns an empty vector by default, matching the pure-Python
             original's ``pass`` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                :cpp:class:`IniGraphGroup` is move-only (it holds move-only
                :cpp:class:`IniSectionGraph`\\s), so the returned vector is likewise move-only --
                take it by value and ``std::move`` it around, don't try to copy it
             @endrst
             *
             * @return The parsed groups of caller/callee graphs found in the .ini file
             */
            virtual std::vector<IniGraphGroup> parse();

        protected:

            /**
             * @brief
             @rst
             The ``.ini`` file that will be parsed -- non-owning, see the constructor
             @endrst
             */
            IniFile* iniFile_;
    };
}

#endif
