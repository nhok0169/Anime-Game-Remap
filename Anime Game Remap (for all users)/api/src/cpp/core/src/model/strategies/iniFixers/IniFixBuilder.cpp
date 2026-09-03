#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"

#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFileFixContext.h"
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"

#include <utility>

#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace AGRemapCore {
    namespace {
        // A GIMIFixer that owns the context it fixes through -- the same arrangement
        // IniParseBuilder::defaultFactory uses for its parser, and for the same reason: GIMIFixer
        // holds a bare Context* and a Factory hands back only the fixer, so something has to keep
        // one alive for exactly as long as it. The context is declared after the base so it is
        // constructed before the constructor body, where setCtx can take its address.
        class OwnedContextGIMIFixer: public GIMIFixer<> {
            public:
                OwnedContextGIMIFixer(BaseIniParser<>* parser, const std::string& toModName):
                    GIMIFixer<>(parser, nullptr, {},
                                 // THE target for this fixer, not all of the mod type's. buildAll
                                 // fans out one fixer per target precisely so each handles one, and
                                 // GIMIFixer::getFix loops over getModsToFix() -- leaving it unset
                                 // falls through to IniFileFixContext::modsToFix(), which is empty,
                                 // and the fix loop then runs zero times.
                                 toModName.empty() ? std::optional<std::vector<std::string>>(std::nullopt)
                                                   : std::optional<std::vector<std::string>>({toModName}),
                                 nullptr, makeConfig()),
                    ctx_(parser != nullptr ? parser->getIniFile() : nullptr) {
                    this->setCtx(&ctx_);
                }

            private:
                // The .ini file comes off the parser rather than being passed in, because that is
                // all a Factory is given. IniParseBuilder's own default parser calls setIniFile for
                // exactly this to work.
                static FixerConfig makeConfig() {
                    FixerConfig config{};
                    config.sectionToStr = &renderIfTemplate;
                    return config;
                }

                IniFileFixContext ctx_;
        };
    }

    IniFixBuilder::Factory IniFixBuilder::defaultFactory() {
        // A real GIMIFixer rather than a do-nothing BaseIniFixer, mirroring the pure-Python
        // ModType.__init__'s own "IniFixBuilder(GIMIFixer)" default -- the bare base was a
        // divergence, the same one IniParseBuilder::defaultFactory had on the parse side.
        //
        // It is handed renderIfTemplate as its sectionToStr: without one, GIMIFixer::groupToStr
        // returns an empty string and the fixer builds its groups correctly and then renders
        // NOTHING. That callback is the whole reason IfTemplate has no toStr of its own.
        return [](BaseIniParser<>* parser, const std::string& toModName) {
            return std::make_shared<OwnedContextGIMIFixer>(parser, toModName);
        };
    }

    IniFixBuilder::IniFixBuilder(): factory_(defaultFactory()) {}

    IniFixBuilder::IniFixBuilder(Factory factory): factory_(std::move(factory)) {
        // Mirrors the pure-Python original's null-fallback -- an empty std::function is this API's
        // version of being handed None.
        if (!factory_) {
            factory_ = defaultFactory();
        }
    }

    IniFixBuilder::IniFixBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound):
        builderArgs_(std::move(builderArgs)), errorOnNotFound_(errorOnNotFound) {
        // The two flavours are mutually exclusive, so factory_ is only populated when there is no
        // table to resolve against -- which is also what makes a nullptr table degrade to the
        // default-constructor behaviour rather than crashing in build().
        if (builderArgs_ == nullptr) {
            factory_ = defaultFactory();
        }
    }

    const std::shared_ptr<const IniFixBuilder::ArgsRepo>& IniFixBuilder::getBuilderArgs() const {
        return builderArgs_;
    }

    bool IniFixBuilder::getErrorOnNotFound() const {
        return errorOnNotFound_;
    }

    std::shared_ptr<BaseIniFixer<>> IniFixBuilder::build(BaseIniParser<>* parser, const std::string& fromModName,
                                                        const std::string& toModName,
                                                        const std::optional<Version>& fromVersion,
                                                        const std::optional<Version>& toVersion) const {
        if (builderArgs_ == nullptr) {
            // The equivalent of the original's "_buildCls is not None" path, where the key
            // arguments are documented as having no effect.
            return factory_(parser, std::string());
        }

        // Non-version columns in relative order: fromModName, then toModName. Version columns in
        // relative order: fromVersion, then toVersion.
        std::optional<Factory> found = builderArgs_->get({fromModName, toModName}, {fromVersion, toVersion},
                                                          errorOnNotFound_);

        // Reached only when errorOnNotFound_ is false (get() would have thrown otherwise), or when
        // a row exists but holds an empty std::function -- both are "nothing usable was found".
        if (!found.has_value() || !*found) {
            return defaultFactory()(parser, toModName);
        }

        return (*found)(parser, toModName);
    }

    std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>> IniFixBuilder::buildAll(
            BaseIniParser<>* parser, const std::string& fromModName,
            const std::optional<Version>& fromVersion, const std::optional<Version>& toVersion,
            const std::optional<std::unordered_set<std::string>>& filteredToModNames) const {
        std::vector<std::pair<std::string, std::shared_ptr<BaseIniFixer<>>>> result;

        // A fixed-factory builder has no targets to fan out over. One entry under the empty name
        // keeps callers uniform -- see buildAll's own note.
        if (builderArgs_ == nullptr) {
            result.emplace_back(std::string(), factory_(parser, std::string()));
            return result;
        }

        // fromModName fixed, toModName free (std::nullopt) -- that free axis is what getAll fans
        // out over, resolving toVersion independently per target mod.
        std::vector<std::pair<std::vector<std::string>, Factory>> matches =
            builderArgs_->getAll({fromModName, std::nullopt}, {fromVersion, toVersion});

        result.reserve(matches.size());
        for (std::pair<std::vector<std::string>, Factory>& match : matches) {
            // getAll hands back the full non-version key, so the target name is the second value.
            const std::string& toModName = match.first[1];

            if (filteredToModNames.has_value() && filteredToModNames->count(toModName) == 0) {
                continue;
            }

            std::shared_ptr<BaseIniFixer<>> fixer = match.second ? match.second(parser, toModName)
                                                               : defaultFactory()(parser, toModName);
            result.emplace_back(toModName, std::move(fixer));
        }

        return result;
    }
}
