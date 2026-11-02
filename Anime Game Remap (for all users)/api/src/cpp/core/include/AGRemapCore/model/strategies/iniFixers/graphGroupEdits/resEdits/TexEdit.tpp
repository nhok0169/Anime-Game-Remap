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

#ifndef AGRemapCore_TexEdit_TPP
#define AGRemapCore_TexEdit_TPP

#include <memory>
#include <utility>
#include <vector>

#include <filesystem>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/tools/TextTools.h"
#include "AGRemapCore/tools/files/FileService.h"
#include "AGRemapCore/tools/hashing/HashTools.h"


namespace AGRemapCore {
    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexCreate<K, V, KeyHash, KeyEqual>::TexCreate(GraphId resModObj, std::string texName, std::shared_ptr<BaseTexEditor> texCreator,
                                                   ResEditConfig config, std::string resType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), texName(std::move(texName)),
        texCreator(std::move(texCreator)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<RemapTexAddResource> TexCreate<K, V, KeyHash, KeyEqual>::makeResModel(
            const std::string& iniFolder, const std::string& path, const std::string& resType) const {
        return std::make_unique<RemapTexAddResource>(iniFolder, path, texCreator, resType, fixFunc);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexCreate<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                           const std::string& fixedPath, const std::string& modName,
                                                           const std::string& fileKey, Context& ctx) {
        (void)fixedPath;
        (void)modName;
        // The resType ARGUMENT, not this->resType (unlike TexReplace). 'srcPath' is where the created texture is
        // written: a created resource has no separate source, which is why ResCreate::collectResourceName returns the
        // fixed name for both halves.
        std::unique_ptr<RemapTexAddResource> resource = makeResModel(ctx.iniFolder(), srcPath, resType);
        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    typename TexCreate<K, V, KeyHash, KeyEqual>::Section* TexCreate<K, V, KeyHash, KeyEqual>::buildSection(
            const std::string& sectionName, const std::string& modName) {
        const std::optional<std::string> file = this->fileOfSection(sectionName, modName);
        if (!file.has_value()) {
            return nullptr;
        }

        std::vector<std::pair<K, V>> src;
        src.emplace_back(this->config.filenameKey, this->config.valOfFile(*file));
        std::vector<std::unique_ptr<IfTemplatePart>> parts;
        parts.push_back(std::make_unique<IfContentPart<K, V, KeyHash, KeyEqual>>(src, 0));

        // A created resource section is a single `filename =` KVP and holds no `run =`, so nothing consults this
        // run config -- but IfTemplate takes one, and ResEditConfig deliberately does not carry it
        IfTemplateRunConfig<K, V> runConfig{
            IniKeywords::Run,
            [](const V& val) { return val; },
            [](const std::string& name) { return name; }
        };
        sections_.push_back(std::make_unique<Section>(std::move(parts), std::move(runConfig), sectionName));
        return sections_.back().get();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexCreate<K, V, KeyHash, KeyEqual>::clear() {
        texInd_ = 0;
        names_.clear();
        // The sections go with the counter: a fixer is reused across .ini files, and a section built for the last
        // one names a file the next one has nothing to do with
        sections_.clear();
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> TexCreate<K, V, KeyHash, KeyEqual>::fileOfSection(const std::string& sectionName,
                                                                                const std::string& modName) const {
        if (!files.empty()) {
            const auto found = files.find(sectionName);
            return found == files.end() ? std::nullopt : std::optional<std::string>(found->second);
        }

        // The leading "Resource" comes off to recover the file's base name -- the pure-Python
        // original's own sectionName[len(IniKeywords.Resource):] slice
        std::string fileBaseName = sectionName;
        if (sectionName.rfind(IniKeywords::Resource, 0) == 0) {
            fileBaseName = sectionName.substr(IniKeywords::Resource.size());
        }
        return this->getFixFile(fileBaseName + FileExt::DDS, modName);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> TexCreate<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                       const std::string& modName) const {
        // the fix names its own textures: the resource keeps the name its register gives it
        if (!files.empty()) {
            return std::nullopt;
        }

        // 'resource' is deliberately discarded: a created texture has no original resource name to
        // build on, so the name comes entirely from the mod being fixed to and the texture type.
        (void)resource;

        // ALREADY NAMED FOR THIS MOD: hand back the same name rather than minting a second.
        // Everything this class creates comes out of one TexCreator, so two names here mean two
        // identical files -- see the note on this method.
        auto cached = names_.find(modName);
        if (cached != names_.end()) {
            return cached->second;
        }

        std::string result = TextTools::capitalize(modName) + texName;

        // The first texture gets no suffix at all -- matching the pure-Python original's own
        // "if (self._texInd):" truthiness check on a zero-based counter.
        if (texInd_ != 0) {
            result += std::to_string(texInd_);
        }

        result = IniNamingTools::getRemapTexResourceName(result);
        ++texInd_;
        names_[modName] = result;
        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexCreate<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                const std::string& graphId) const {
        (void)modName;

        std::string result = IniNamingTools::getFixedTexFile(file);
        if (graphId.empty()) {
            return result;
        }

        // 'file', not 'result' -- see this method's own note. Preserved from the pure-Python
        // original rather than corrected.
        return Base::fileAddGraphId(file, graphId);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    int TexCreate<K, V, KeyHash, KeyEqual>::texInd() const {
        return texInd_;
    }

    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    TexReplace<K, V, KeyHash, KeyEqual>::TexReplace(GraphId resModObj, std::shared_ptr<BaseTexEditor> texEditor, ResEditConfig config,
                                                     std::string resType, std::optional<std::string> resSubType):
        Base(std::move(resType), std::move(resModObj), std::move(config)), resSubType(std::move(resSubType)),
        texEditor(std::move(texEditor)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::unique_ptr<RemapTexEditResource> TexReplace<K, V, KeyHash, KeyEqual>::makeResModel(
            const std::string& iniFolder, const std::string& srcPath, const std::string& fixedPath, const std::string& resType) const {
        return std::make_unique<RemapTexEditResource>(iniFolder, srcPath, fixedPath, texEditor, resType, fixFunc);
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    void TexReplace<K, V, KeyHash, KeyEqual>::buildResModel(const std::string& resType, const std::string& srcPath,
                                                            const std::string& fixedPath, const std::string& modName,
                                                            const std::string& fileKey, Context& ctx) {
        (void)resType;
        (void)modName;
        std::unique_ptr<RemapTexEditResource> resource = makeResModel(ctx.iniFolder(), srcPath, fixedPath, this->resType);
        resource->logger = ctx.logger();
        ctx.storeResource(fileKey, std::move(resource));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexReplace<K, V, KeyHash, KeyEqual>::subTypedModName(const std::string& modName) const {
        std::string result = TextTools::capitalize(modName);
        if (resSubType.has_value()) {
            result += TextTools::capitalize(*resSubType);
        }

        return result;
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::optional<std::string> TexReplace<K, V, KeyHash, KeyEqual>::getFixResourceName(const std::string& resource,
                                                                                         const std::string& modName) const {
        return IniNamingTools::getRemapTexResourceName(resource, subTypedModName(modName));
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual>
    std::string TexReplace<K, V, KeyHash, KeyEqual>::getFixFile(const std::string& file, const std::string& modName,
                                                                 const std::string& graphId) const {
        // ONE FILE PER EDIT, NOT PER SOURCE TEXTURE -- and that distinction is the whole point.
        //
        // Named from the source resource alone (the old behaviour, kept below for a caller that
        // names no mod object), TWO edits of one texture write to ONE path. The .ini file is
        // fine -- two resource sections, two names -- and both name the same file, so the second
        // edit lands on top of the first. Seen in game on AyakaSpringbloom -> Ayaka: her neck
        // rendered pale and flat because a lightmap had been given both a colour replacement AND
        // an alpha of 1, and the pixels say so (18a100ff -> 015d0001).
        //
        // So the name follows the pure-Python original's:
        //
        //     <target mod><Obj>RemapTex<hash of the source file> <hash of the edit name>.dds
        //
        // TWO hashes, space-separated, exactly as _getFixedTexFile builds them. The first
        // separates edits of different sources, the second separates different edits of the SAME
        // source -- which is the collision above, and why one hash would not have been enough.
        if (!modObj.empty()) {
            const std::string texName = resSubType.has_value() ? *resSubType : std::string();
            const std::string baseName = FileService::pathToStr(
                std::filesystem::path(FileService::strToPath(file)).filename());

            // STABLE, not fresh-per-call. The same source texture given the same edit is one
            // file, however many target objects end up pointing at it -- see the note in
            // GIMICharFixer::buildTexEdits, and HashTools::getStableShortHashStr for why the
            // ordinary short hash would hand out HfW, HfW_B, HfW_C for three identical requests.
            const std::string ind = HashTools::getStableShortHashStr(baseName) + " "
                                    + HashTools::getStableShortHashStr(texName);

            const std::string folder = FileService::pathToStr(
                std::filesystem::path(FileService::strToPath(file)).parent_path());
            const std::string named = IniNamingTools::getRemapTexName(
                "", TextTools::capitalize(modName) + TextTools::capitalize(modObj)) + ind + ".dds";

            return folder.empty() ? named
                                  : FileService::pathToIniStr(FileService::strToPath(folder)
                                                               / FileService::strToPath(named));
        }

        std::string result = IniNamingTools::getFixedTexFile(file);
        if (graphId.empty()) {
            return result;
        }

        return Base::fileAddGraphId(result, graphId);
    }
}

#endif
