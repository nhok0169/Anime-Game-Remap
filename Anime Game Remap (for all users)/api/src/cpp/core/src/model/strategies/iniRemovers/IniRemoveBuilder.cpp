#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveBuilder.h"

#include <utility>


namespace AGRemapCore {
    IniRemoveBuilder::Factory IniRemoveBuilder::defaultFactory() {
        return [](IniFile* iniFile) {
            return std::make_shared<BaseIniRemover>(iniFile);
        };
    }

    const std::string& IniRemoveBuilder::defaultId() {
        // The stand-in for the pure-Python original's "self._buildCls.__name__" -- see the
        // declaration for why a constant is equivalent here. Named after the class the default
        // factory builds, so the two stay in step if a concrete remover is ever swapped in.
        static const std::string id = "BaseIniRemover";
        return id;
    }

    IniRemoveBuilder::IniRemoveBuilder(): factory_(defaultFactory()), id_(defaultId()) {}

    IniRemoveBuilder::IniRemoveBuilder(Factory factory, std::string id, bool cache):
        cache(cache), factory_(std::move(factory)), id_(std::move(id)) {
        if (!factory_) {
            factory_ = defaultFactory();
        }

        if (id_.empty()) {
            id_ = defaultId();
        }
    }

    IniRemoveBuilder::IniRemoveBuilder(std::shared_ptr<const ArgsRepo> builderArgs, bool errorOnNotFound, bool cache):
        cache(cache), builderArgs_(std::move(builderArgs)), errorOnNotFound_(errorOnNotFound), id_(defaultId()) {
        // The two flavours are mutually exclusive, so factory_ is only populated when there is no
        // table to resolve against -- which is also what makes a nullptr table degrade to the
        // default-constructor behaviour rather than crashing in build().
        if (builderArgs_ == nullptr) {
            factory_ = defaultFactory();
        }
    }

    const std::string& IniRemoveBuilder::getId() const {
        return id_;
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

    std::shared_ptr<BaseIniRemover> IniRemoveBuilder::build(IniFile* iniFile, const std::string& modName,
                                                             const std::optional<Version>& version,
                                                             const std::optional<std::string>& id) {
        Factory factory = resolveFactory(modName, version);

        // The equivalent of the original's "if (not cache): return super().build(...)" early-out --
        // no cache read, no cache write.
        if (!cache) {
            std::shared_ptr<BaseIniRemover> fresh = factory(iniFile);
            if (fresh == nullptr) {
                fresh = defaultFactory()(iniFile);
            }

            fresh->setIniFile(iniFile);
            return fresh;
        }

        // See the declaration for this precedence. Keying on the mod name whenever a table is in
        // use is what stops two mod types resolving to different factories from sharing one cached
        // instance -- with id-only keying they would all collide on defaultId().
        std::string key;
        if (id.has_value()) {
            key = *id;
        } else if (builderArgs_ != nullptr && !modName.empty()) {
            key = modName;
        } else {
            key = id_;
        }

        auto it = cache_.find(key);
        if (it == cache_.end()) {
            std::shared_ptr<BaseIniRemover> built = factory(iniFile);
            if (built == nullptr) {
                built = defaultFactory()(iniFile);
            }

            it = cache_.emplace(std::move(key), std::move(built)).first;
        }

        // Deliberately outside the miss branch: rebinding on a cache *hit* is what lets one shared
        // remover serve many .ini files, and is the original's own "result.iniFile = iniFile".
        it->second->setIniFile(iniFile);
        return it->second;
    }

    void IniRemoveBuilder::clearCache() {
        cache_.clear();
    }

    std::size_t IniRemoveBuilder::getCacheSize() const {
        return cache_.size();
    }
}
