#ifndef AGRemapCore_TexCreatorEdit_TPP
#define AGRemapCore_TexCreatorEdit_TPP

#include <functional>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"

#include "TexCreatorEdit.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexCreatorCreate<K, V, KeyHash, KeyEqual>::TexCreatorCreate(
            GraphId resModObj, std::string texName, TexCreator texCreator, ResEditConfig config,
            std::string resType):
        Base(std::move(resModObj), std::move(texName), std::move(config), std::move(resType)),
        texCreator(std::move(texCreator)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename TexCreatorCreate<K, V, KeyHash, KeyEqual>::Section*
            TexCreatorCreate<K, V, KeyHash, KeyEqual>::buildSection(const std::string& sectionName,
                                                                     const std::string& modName) {
        // The leading "Resource" comes off to recover the file's base name -- the pure-Python
        // original's own sectionName[len(IniKeywords.Resource):] slice, and what the pybind11
        // override does too.
        std::string fileBaseName = sectionName;
        if (sectionName.rfind(IniKeywords::Resource, 0) == 0) {
            fileBaseName = sectionName.substr(IniKeywords::Resource.size());
        }

        std::vector<std::pair<K, V>> src;
        src.emplace_back(this->config.filenameKey,
                          this->config.valOfFile(this->getFixFile(fileBaseName + FileExt::DDS, modName)));

        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::make_unique<IfContentPart<K, V, KeyHash, KeyEqual>>(src, 0));

        // A default run config: a created resource section is a single `filename =` KVP and holds
        // no `run =` at all, so nothing ever consults these -- but IfTemplate takes one, and
        // ResEditConfig deliberately does not carry it (see its own note on what it is for).
        IfTemplateRunConfig<K, V> runConfig{
            IniKeywords::Run,
            [](const V& val) { return val; },
            [](const std::string& name) { return name; }
        };

        sections_.push_back(std::make_unique<Section>(std::move(parts), std::move(runConfig), sectionName));
        return sections_.back().get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexCreatorCreate<K, V, KeyHash, KeyEqual>::buildResModel(
            const std::string& resType, const std::string& srcPath, const std::string& fixedPath,
            const std::string& modName, const std::string& fileKey, Context& ctx) {
        (void)fixedPath;
        (void)modName;

        // The resType ARGUMENT rather than this->resType, unlike VGRemapBlendReplace -- faithful to
        // the pure-Python original, which differs between the two, and to the pybind11 override
        // that already reproduces it.
        //
        // 'srcPath' is where the created texture is written: a created resource has no separate
        // source, which is why ResCreate::collectResourceName returns the fixed name for both
        // halves.
        auto resource = std::make_unique<RemapTexAddResource>(
            ctx.iniFolder(), srcPath, texCreator, resType,
            std::function<bool(RemapTexAddResource&)>{});

        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexCreatorCreate<K, V, KeyHash, KeyEqual>::clear() {
        Base::clear();

        // The sections go with the counter. A fixer is reused across .ini files, and a section built
        // for the last one names a file the next one has nothing to do with.
        sections_.clear();
    }
}

#endif
