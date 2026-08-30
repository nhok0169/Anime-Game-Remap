#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"

#include <utility>


namespace AGRemapCore {
    IniParseBuilder::Factory IniParseBuilder::defaultFactory() {
        return [](IniFile* iniFile) {
            return std::make_shared<BaseIniParser<>>(iniFile);
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
                                                           const std::optional<Version>& version) const {
        if (builderArgs_ == nullptr) {
            // The equivalent of the original's "_buildCls is not None" path, where modName/version
            // are documented as having no effect.
            return factory_(iniFile);
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
            return defaultFactory()(iniFile);
        }

        return (*found)(iniFile);
    }
}
