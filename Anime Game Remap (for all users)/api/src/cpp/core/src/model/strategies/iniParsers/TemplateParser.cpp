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

#include "AGRemapCore/model/strategies/iniParsers/TemplateParser.h"

#include <utility>


namespace AGRemapCore {
    TemplateParser::TemplateParser(IniFile* iniFile, std::optional<int> modTypeId, std::vector<ModObj> modObjs):
        GIMIParser<>(nullptr, std::move(modObjs)), ctx_(iniFile, modTypeId) {
        // The context is a member, so it can only be handed to the base after that base is built
        this->setCtx(&ctx_);
        this->setIniFile(iniFile);
    }

    TemplateParser::Classifier& TemplateParser::ownClassifier(std::unique_ptr<Classifier> classifier) {
        classifiers_.push_back(std::move(classifier));
        return *classifiers_.back();
    }

    TemplateParser::GroupEdit& TemplateParser::ownStep(std::unique_ptr<GroupEdit> step) {
        steps_.push_back(std::move(step));
        return *steps_.back();
    }

    TemplateParser::GroupEdit& TemplateParser::addParseStep(std::unique_ptr<GroupEdit> step) {
        GroupEdit& owned = ownStep(std::move(step));
        this->parseSteps.push_back(&owned);
        return owned;
    }

    TemplateParser::GroupEdit& TemplateParser::addDeferredParseStep(std::unique_ptr<GroupEdit> step) {
        GroupEdit& owned = ownStep(std::move(step));
        this->deferredParseSteps.push_back(&owned);
        return owned;
    }
}
