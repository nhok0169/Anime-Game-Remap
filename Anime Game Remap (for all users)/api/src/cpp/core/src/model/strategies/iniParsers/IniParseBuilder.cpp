#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"

#include <utility>

#include "AGRemapCore/model/strategies/iniParsers/GIMIParser.h"
#include "AGRemapCore/model/strategies/iniParsers/IniFileParseContext.h"


namespace AGRemapCore {
    namespace {
        // A GIMIParser that owns the context it reads through. GIMIParser holds a bare Context*, so
        // something has to keep one alive for exactly as long as the parser -- and a Factory hands
        // back only the parser. Declaring the context after the base means it is constructed before
        // the constructor body runs, which is where setCtx can safely take its address.
        class OwnedContextGIMIParser: public GIMIParser<> {
            public:
                explicit OwnedContextGIMIParser(IniFile* iniFile, std::optional<int> modTypeId):
                    GIMIParser<>(nullptr), ctx_(iniFile, modTypeId) {
                    this->setCtx(&ctx_);

                    // GIMIParser's own constructor calls Base() and never tells BaseIniParser which
                    // .ini file it reads -- it works through the context instead. That matters here:
                    // BaseIniFixer::setParser takes ITS .ini file from parser->getIniFile(), so
                    // leaving it null would silently blind every fixer built over this parser --
                    // MultiModFixer, which reads filteredToModTypeIds off it, included.
                    this->setIniFile(iniFile);
                }

            private:
                IniFileParseContext ctx_;
        };
    }

    IniParseBuilder::Factory IniParseBuilder::defaultFactory() {
        // A real GIMIParser rather than a do-nothing BaseIniParser: the pure-Python
        // ModType.__init__ defaults to "IniParseBuilder(GIMIParser)", so a bare base here was a
        // divergence, not a design choice. 'modTypeId' is what lets the context answer
        // modTypeName/hasModType, which GIMIParser reads when naming what it builds.
        return [](IniFile* iniFile, std::optional<int> modTypeId) {
            return std::make_shared<OwnedContextGIMIParser>(iniFile, modTypeId);
        };
    }

    IniParseBuilder::IniParseBuilder(): factory_(defaultFactory()) {}

    IniParseBuilder::IniParseBuilder(Factory factory): factory_(std::move(factory)) {
        // Mirrors the pure-Python original's "if (iniParseBuilder is None)" fallback -- an empty
        // std::function is this API's version of being handed None.
        if (!factory_) {
            factory_ = defaultFactory();
        }
    }

    IniParseBuilder::IniParseBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound):
        builderArgs_(std::move(builderArgs)), errorOnNotFound_(errorOnNotFound) {
        // The two flavours are mutually exclusive (see the private members), so factory_ is only
        // populated when there is no table to resolve against -- which is also what makes a
        // nullptr table degrade to the default-constructor behaviour rather than crashing in
        // build().
        if (builderArgs_ == nullptr) {
            factory_ = defaultFactory();
        }
    }

    const std::shared_ptr<const IniParseBuilder::ArgsRepo>& IniParseBuilder::getBuilderArgs() const {
        return builderArgs_;
    }

    bool IniParseBuilder::getErrorOnNotFound() const {
        return errorOnNotFound_;
    }

    std::shared_ptr<BaseIniParser<>> IniParseBuilder::build(IniFile* iniFile, const std::string& modName,
                                                           const std::optional<Version>& version,
                                                           std::optional<int> modTypeId) const {
        if (builderArgs_ == nullptr) {
            // The equivalent of the original's "_buildCls is not None" path, where modName/version
            // are documented as having no effect.
            return factory_(iniFile, modTypeId);
        }

        // 'modName' is the table's single non-version index value, matching the pure-Python
        // original's own "self._builderArgs.get(modName, version = version)". A missing 'version'
        // means "latest listed"; a present one floor-matches, so an older row keeps applying to
        // every later version until a newer row supersedes it.
        std::optional<Factory> found = builderArgs_->get({modName}, version, errorOnNotFound_);

        // Reached only when errorOnNotFound_ is false (get() would have thrown otherwise), or when
        // a row exists but holds an empty std::function -- both are "nothing usable was found", so
        // both degrade to a plain parser rather than dereferencing an empty callable.
        if (!found.has_value() || !*found) {
            return defaultFactory()(iniFile, modTypeId);
        }

        return (*found)(iniFile, modTypeId);
    }
}
