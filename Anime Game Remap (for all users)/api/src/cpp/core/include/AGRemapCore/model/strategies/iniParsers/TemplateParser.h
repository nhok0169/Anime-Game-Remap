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

#include <memory>
#include <optional>
#include <vector>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


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
             * @brief The parse context over the .ini file being parsed
             */
            IniFileParseContext ctx_;

        private:
            std::vector<std::unique_ptr<Classifier>> classifiers_;
            std::vector<std::unique_ptr<GroupEdit>> steps_;
    };
}

#endif
