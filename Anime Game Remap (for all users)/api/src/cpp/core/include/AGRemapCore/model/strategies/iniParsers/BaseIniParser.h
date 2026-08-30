#ifndef AGRemapCore_BaseIniParser_H
#define AGRemapCore_BaseIniParser_H

#include <functional>
#include <string>
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
        ``FixRaidenBoss2.model.strategies.iniParsers.BaseIniParser`` original (which has since been
        deleted outright -- this class *is* the ``BaseIniParser`` the `Python`_ API exposes now):

        * There is no ``_modsToFix`` attribute (and so #clear has nothing of its own to reset, and
          is a no-op by default). The pure-Python original's ``clear`` exists *only* to clear that
          set, so subclasses here own whatever state they accumulate and reset it themselves
        * #parse returns the parsed :cpp:class:`IniGraphGroup`\\s instead of ``None``, so a parser
          hands its results back to its caller as a real value rather than by mutating the
          :cpp:class:`IniFile` it was given

     .. note::
        This is a class template over the same ``K``/``V``/``KeyHash``/``KeyEqual`` as the
        :cpp:class:`IfContentPart`\\s the `sections`_ it parses are made of, defaulting to
        ``<std::string, std::string>``. It has to be, for the same reason
        :cpp:class:`BaseRegEdit`/:cpp:class:`BaseIniGraphEdit` do: the `pybind11`_ layer's
        `sections`_ are ``IfTemplate<py::object, py::object, ...>``, so a parser pinned to
        ``<std::string, std::string>`` would be unreachable from any binding -- and
        :cpp:class:`GIMIParser`, the one concrete parser this base has, is reached from exactly
        there.

     .. note::
        :cpp:class:`IniParseBuilder` (and, through it, :cpp:member:`ModType::iniParseBuilder` and
        :cpp:func:`IniFile::getParser`) deliberately stays pinned to ``BaseIniParser<>`` rather
        than becoming a template of its own: nothing on the `Python`_ side builds parsers through
        it (the `Python`_ API has its own pure-Python ``IniParseBuilder``), so the only instantiation
        it ever needs is the plain-``std::string`` one
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseIniParser {
        public:

            /**
             * @brief The type of group of caller/callee graphs this parser produces
             */
            using GraphGroup = IniGraphGroup<K, V, KeyHash, KeyEqual>;

            /**
             * @brief Constructs a new parser
             *
             * @param iniFile
             @rst
             The ``.ini`` file to parse :raw-html:`<br />` :raw-html:`<br />`

             This is a non-owning pointer to a file owned elsewhere -- it must outlive this parser
             :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` is allowed so a parser can exist before it's bound to any particular file;
             call #setIniFile before #parse in that case :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A parser reached through a :cpp:class:`ModType` is never in that state --
                :cpp:member:`ModType::iniParseBuilder` builds one per ``.ini`` file, already bound
                to it (see :cpp:func:`IniParseBuilder::build`), rather than sharing one unbound
                instance across every file of that mod type

             .. note::
                :cpp:class:`GIMIParser` -- the one concrete parser in this hierarchy -- does **not**
                read the ``.ini`` file through this pointer at all; it goes through a
                :cpp:class:`IniParseContext` instead, because the ``.ini`` file its real callers
                hand it is the *`Python`_* ``IniFile``, an unrelated class to
                :cpp:class:`AGRemapCore::IniFile`. See that class's own note

             :raw-html:`<br />`

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
            virtual std::vector<GraphGroup> parse();

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

#include "BaseIniParser.tpp"

#endif
