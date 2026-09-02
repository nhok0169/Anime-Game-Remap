#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"

#include <utility>

// The concrete remover defaultFactory hands out. Not reachable from IniRemoveBuilder.h --
// RemapIniRemover.tpp needs the whole IniFile, which reaches this header back through ModType.h --
// so the include lives here, in the .cpp, where the cycle cannot form.
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"


namespace AGRemapCore {
    IniRemoveBuilder::Factory IniRemoveBuilder::defaultFactory() {
        // RemapIniRemover<>::factory() rather than a lambda of our own: that one does the
        // default-construct-then-setIniFile dance this needs, and setIniFile is what builds the
        // remover's IniFileRemoveContext (a constructor argument cannot, since the context has to
        // outlive the call).
        return RemapIniRemover<>::factory();
    }

    IniRemoveBuilder::IniRemoveBuilder(): factory_(defaultFactory()) {}

    IniRemoveBuilder::IniRemoveBuilder(Factory factory): factory_(std::move(factory)) {
        if (!factory_) {
            factory_ = defaultFactory();
        }
    }

    IniRemoveBuilder::IniRemoveBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound):
        builderArgs_(std::move(builderArgs)), errorOnNotFound_(errorOnNotFound) {
        // The two flavours are mutually exclusive, so factory_ is only populated when there is no
        // table to resolve against -- which is also what makes a nullptr table degrade to the
        // default-constructor behaviour rather than crashing in build().
        if (builderArgs_ == nullptr) {
            factory_ = defaultFactory();
        }
    }

    const std::shared_ptr<const IniRemoveBuilder::ArgsRepo>& IniRemoveBuilder::getBuilderArgs() const {
        return builderArgs_;
    }

    bool IniRemoveBuilder::getErrorOnNotFound() const {
        return errorOnNotFound_;
    }

    IniRemoveBuilder::Factory IniRemoveBuilder::resolveFactory(const std::string& modName,
                                                                const std::optional<Version>& version) const {
        if (builderArgs_ == nullptr) {
            return factory_;
        }

        std::optional<Factory> found = builderArgs_->get({modName}, version, errorOnNotFound_);

        // Reached only when errorOnNotFound_ is false (get() would have thrown otherwise), or when
        // a row exists but holds an empty std::function -- both are "nothing usable was found".
        if (!found.has_value() || !*found) {
            return defaultFactory();
        }

        return *found;
    }

    std::shared_ptr<BaseIniRemover<>> IniRemoveBuilder::build(IniFile* iniFile, const std::string& modName,
                                                             const std::optional<Version>& version) const {
        Factory factory = resolveFactory(modName, version);

        std::shared_ptr<BaseIniRemover<>> result = factory(iniFile);

        // A caller-supplied factory is allowed to hand back nothing; the contract is that build()
        // never returns nullptr, so fall back rather than propagating it.
        if (result == nullptr) {
            result = defaultFactory()(iniFile);
        }

        // Not redundant with the factory call -- see the declaration's own note.
        result->setIniFile(iniFile);
        return result;
    }
}
