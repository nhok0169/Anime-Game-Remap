#ifndef AGRemapCore_ModType_H
#define AGRemapCore_ModType_H

#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Heavy data for a type of mod :raw-html:`<br />` :raw-html:`<br />`

     Meant to carry the full C++-side representation of a mod type -- contrast with the cheap
     :cpp:class:`ModTypeIdData` an ini classifier (e.g. :cpp:class:`IniClassifier`) holds instead.
     The Python-side ``ModType`` is meant to build itself using this data.
     @endrst
     */
    class ModType {
        public:

            /**
             * @brief Constructs new data for a type of mod
             *
             * @param gameTypeId
             @rst
             The id for the game this type of mod belongs to -- stored as-is, with no validation
             that it corresponds to one of :cpp:enum:`GameTypeId`'s declared values (see
             :cpp:class:`GameTypeIdTools` if that's needed)
             @endrst
             * @param modTypeId
             @rst
             The id for this specific type of mod -- stored as-is, with no validation that it
             corresponds to one of :cpp:enum:`ModTypeId`'s declared values (see
             :cpp:class:`ModTypeIdTools` if that's needed), so a custom mod type using some id not
             registered in :cpp:enum:`ModTypeId` can still be represented
             @endrst
             * @param name The default name for the type of mod
             * @param aliases Other alternative names for the type of mod
             * @param iniParser
             @rst
             The parser used to parse a ``.ini`` file of this type of mod -- see #iniParser
             :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a plain :cpp:class:`BaseIniParser` is constructed instead,
             mirroring the pure-Python original's own
             ``if (iniParseBuilder is None): iniParseBuilder = IniParseBuilder(GIMIParser)``
             fallback :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param iniFixer
             @rst
             The fixer used to fix a ``.ini`` file of this type of mod -- see #iniFixer
             :raw-html:`<br />` :raw-html:`<br />`

             If this is ``nullptr``, a plain :cpp:class:`BaseIniFixer` bound to #iniParser is
             constructed instead, mirroring the pure-Python original's own ``iniFixBuilder``
             fallback :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit ModType(int gameTypeId, int modTypeId, const std::string &name, const std::vector<std::string> &aliases = {},
                              std::shared_ptr<BaseIniParser> iniParser = nullptr,
                              std::shared_ptr<BaseIniFixer> iniFixer = nullptr);

            /**
             * @brief The id for the game this type of mod belongs to
             */
            int gameTypeId;

            /**
             * @brief The id for this specific type of mod
             */
            int modTypeId;

            /**
             * @brief The default name for the type of mod
             */
            std::string name;

            /**
             * @brief Other alternative names for the type of mod
             */
            std::vector<std::string> aliases;

            /**
             * @brief
             @rst
             The parser used to parse a ``.ini`` file of this type of mod :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The pure-Python original holds an ``IniParseBuilder`` (a *factory*) here, not a
                parser, so that ``IniFile._getParser`` can build a **fresh** parser per ``.ini``
                file. This holds a single parser instance instead, so every ``.ini`` file of this
                mod type shares it -- bind it to the file being worked on with
                :cpp:func:`BaseIniParser::setIniFile` before use (it is constructed unbound, since
                a :cpp:class:`ModType` describes a *kind* of mod rather than one specific file).
                Revisit if per-file parser state is ever needed

             .. note::
                A ``shared_ptr`` rather than a ``unique_ptr`` because :cpp:class:`ModType` must stay
                **copyable** -- :cpp:func:`ModTypeIdTools::getModType` returns one by value, and
                :cpp:member:`IniFile::modTypes` stores them by value
             @endrst
             */
            std::shared_ptr<BaseIniParser> iniParser;

            /**
             * @brief
             @rst
             The fixer used to fix a ``.ini`` file of this type of mod -- see #iniParser for why
             this is a shared single instance rather than the pure-Python original's
             ``IniFixBuilder`` factory
             @endrst
             */
            std::shared_ptr<BaseIniFixer> iniFixer;
    };
}

#endif
