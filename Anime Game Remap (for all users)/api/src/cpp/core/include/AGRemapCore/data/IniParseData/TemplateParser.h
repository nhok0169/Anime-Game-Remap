#ifndef AGRemapCore_TemplateParser_H
#define AGRemapCore_TemplateParser_H
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
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseRows.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The base of a plain-C++ parser that parses one :cpp:class:`AGRemapCore::IniFile` by its classifiers and a list of
     steps -- the parser counterpart of :cpp:class:`TemplateFixer` :raw-html:`<br />` :raw-html:`<br />`

     A subclass sets up its classifiers and steps in its constructor. This class supplies what every such parser needs
     and none of them is about:

     * the parse context: an :cpp:class:`IniFileParseContext` over the ``.ini`` file, owned here and installed as the
       parser's context
     * owning what the parser is built from: its :cpp:class:`GIMISectionClassifier`\\ s (:cpp:func:`ownClassifier`)
       and its steps (:cpp:func:`addParseStep`, :cpp:func:`addDeferredParseStep`, :cpp:func:`ownStep`), which
       :cpp:class:`GIMIParser` itself only borrows

     The steps worth sharing between parsers are edits of their own -- a :cpp:class:`GraphCollect`, a
     :cpp:class:`ParseDataStep` -- and another ``.ini`` of the mod is read through :cpp:func:`IniFile::sibling`, so a
     parser built from `Python`_ on the bound :cpp:class:`GIMIParser` has the same pieces
     @endrst
     */
    class TemplateParser: public GIMIParser<> {
        public:
            /**
             * @brief A step of the parser
             */
            using GroupEdit = BaseIniGraphGroupEdit<>;

            /**
             * @brief Constructs a new parser
             *
             * @param iniFile The .ini file to parse
             * @param modTypeId The id of the mod type the parser is for, if any
             * @param modObjs The mod objects to parse. **Default**: none
             */
            TemplateParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs = {});

            TemplateParser(const TemplateParser&) = delete;
            TemplateParser& operator=(const TemplateParser&) = delete;

        protected:
            /**
             * @brief Keeps 'classifier' alive as long as the parser
             *
             * @param classifier The classifier
             *
             * @return The classifier, now owned here
             */
            Classifier& ownClassifier(std::unique_ptr<Classifier> classifier);

            /**
             * @brief Keeps 'step' alive as long as the parser, without running it -- for a step another edit runs
             *
             * @param step The step
             *
             * @return The step, now owned here
             */
            GroupEdit& ownStep(std::unique_ptr<GroupEdit> step);

            /**
             * @brief Owns 'step' and adds it to the end of :cpp:member:`GIMIParser::parseSteps`
             *
             * @param step The step
             *
             * @return The step, now owned here
             */
            GroupEdit& addParseStep(std::unique_ptr<GroupEdit> step);

            /**
             * @brief Owns 'step' and adds it to the end of :cpp:member:`GIMIParser::deferredParseSteps`
             *
             * @param step The step
             *
             * @return The step, now owned here
             */
            GroupEdit& addDeferredParseStep(std::unique_ptr<GroupEdit> step);

            /**
             * @brief Which of the standard row sets #addRowSteps adds -- see :cpp:struct:`IniParseRows`
             */
            struct RowSteps {
                /**
                 * @brief Whether to collect :cpp:member:`IniParseRows::Resources`. **Default**: ``true``
                 */
                bool resources = true;

                /**
                 * @brief The keys :cpp:member:`IniParseRows::Declared` collects besides ``filename``, ``type``, ``format`` and ``stride``
                 */
                std::vector<std::string> declaredKeys;

                /**
                 * @brief Whether to collect :cpp:member:`IniParseRows::Sections`. **Default**: ``true``
                 */
                bool sections = true;

                /**
                 * @brief The mod objects :cpp:member:`IniParseRows::Objs` collects every key of; empty takes none
                 */
                std::function<bool(const ModObj&)> objs;
            };

            /**
             * @brief
             @rst
             Adds the standard row sets to :cpp:member:`GIMIParser::deferredParseSteps`, in this order: a copy of the
             parser's global graph is put in the groups (:cpp:class:`GraphCreate`); a :cpp:class:`ResRegCollect` over
             the keys that ever name a resource builds a graph of the resource `sections`_ bound anywhere
             (:cpp:class:`ResIdentity`: nothing renamed, no models) and a :cpp:class:`GraphCollect` reads their
             ``filename``; the declarations and every key of every `section`_ are collected off the global graph; both
             graphs are taken out again (:cpp:class:`GraphRemove`); and every key of the chosen objects is collected.
             A step a subclass adds afterwards runs after these
             @endrst
             *
             * @param rows Which row sets to add
             */
            void addRowSteps(const RowSteps& rows);

            /**
             * @brief
             @rst
             The resource `section`_ 'value' names, as a binding names one, under the name it is declared by -- or empty.
             Without regard to case, as 3DMigoto reads a section name: a mod may declare ``[ResourceTexCoordBuffer]``
             and bind ``vb2 = ResourceTexcoordBuffer``. Known once #addRowSteps' global graph has been put in the groups
             @endrst
             *
             * @param value The value
             */
            std::string resourceNamed(const std::string& value) const;

            /**
             * @brief The parse context over the .ini file being parsed
             */
            IniFileParseContext ctx_;

        private:
            Graph* globalGraphForResources();

            std::vector<std::unique_ptr<Classifier>> classifiers_;
            std::vector<std::unique_ptr<GroupEdit>> steps_;
            std::unordered_set<std::string> resourceKeys_;
            std::unordered_map<std::string, std::string> declaredResources_;     // lowercased -> as declared
            std::unique_ptr<BaseResEdit<>> resourceIdentity_;
    };
}

#endif
