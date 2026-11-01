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

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegDrawRangeRemap.h"
#include "AGRemapCore/tools/ListTools.h"
#include "AGRemapCore/tools/StringTools.h"

#include <algorithm>
#include <tuple>


namespace AGRemapCore {
    void IniParseData::fileAnatomy(const AnatomyRules& rules) {
        // the files of the resources the graphs bind -- each section's first `filename`
        if (!rules.resources.empty()) {
            std::set<std::string> filed;
            for (const auto& row : collected(rules.resources)) {
                if (filed.insert(row.section).second) {
                    setResourceFile(row.section, std::string(StringTools::strip(row.value)));
                }
            }
        }

        // every row of each object, once each
        std::map<ModObj, std::vector<const CollectedRow*>> rowsOf;
        std::set<std::tuple<ModObj, std::string, std::size_t, long long, std::string>> seen;
        for (const auto& row : collected(rules.rows)) {
            if (seen.emplace(row.graphId.modObj, row.section, row.partId, row.line, row.key).second) {
                rowsOf[row.graphId.modObj].push_back(&row);
            }
        }

        const std::string drawKey = StringTools::toLower(IniKeywords::DrawIndexed);
        const std::string runKey = StringTools::toLower(IniKeywords::Run);
        const std::string regPrefix = StringTools::toLower(rules.texRegPrefix);
        std::size_t read = 0;
        std::vector<ModObj> hidden;
        for (const auto& [obj, roots] : rules.roots) {
            if (roots.empty()) {
                continue;
            }
            ++read;

            // the rows in the object's own root sections, in the order they are written: by root, then part, then line
            std::map<std::string, std::size_t> rootIndex;
            for (std::size_t i = 0; i < roots.size(); ++i) {
                rootIndex.emplace(roots[i], i);
            }

            std::vector<std::tuple<std::size_t, std::size_t, long long, const CollectedRow*>> inRoots;
            bool draws = false;
            for (const CollectedRow* row : rowsOf[obj]) {
                const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));
                if (StringTools::startsWith(key, "draw")) {
                    draws = true;
                } else if (key == runKey && rules.runDraws) {
                    draws = draws || rules.runDraws(std::string(StringTools::strip(row->value)));
                }

                const auto root = rootIndex.find(row->section);
                if (root != rootIndex.end()) {
                    inRoots.emplace_back(root->second, row->position + 1, row->line, row);
                }
            }
            std::sort(inRoots.begin(), inRoots.end(), [](const auto& a, const auto& b) {
                return std::make_tuple(std::get<0>(a), std::get<1>(a), std::get<2>(a))
                       < std::make_tuple(std::get<0>(b), std::get<1>(b), std::get<2>(b));
            });

            bool drew = false;
            bool skipped = false;
            std::vector<std::string> late;
            for (const auto& [rootAt, partAt, line, row] : inRoots) {
                (void)rootAt;
                (void)line;
                const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));

                // its draws, and which part each sits in
                if (row->key == IniKeywords::DrawIndexed) {
                    addDrawPart(obj, partAt);
                    const std::string& draw = row->value;
                    if (std::count(draw.begin(), draw.end(), ',') >= 2) {
                        const std::optional<std::vector<long long>> numbers = RegDrawRangeRemap<>::parseRange(draw);
                        if (numbers.has_value() && numbers->size() >= 3) {
                            addDrawRange(obj, {(*numbers)[0], (*numbers)[1]});
                        } else if (rules.log) {
                            // a dropped draw range is a piece of the mesh that stops being remapped
                            rules.log("could not read the draw range `" + std::string(StringTools::strip(draw)) + "` in ["
                                      + row->section + "], so it is not remapped");
                        }
                    }
                }

                // the texture registers it binds only after it has drawn
                if (key == drawKey) {
                    drew = true;
                } else if (drew && StringTools::startsWith(key, regPrefix)) {
                    ListTools::pushDistinct(late, key);
                }

                // the libraries it names, on any line
                for (const auto& [library, names] : rules.libraries) {
                    if (names && (names(row->key) || names(row->value))) {
                        addLibrary(obj, library);
                    }
                }

                if (row->key == IniKeywords::Handling
                        && StringTools::equalsIgnoreCase(std::string(StringTools::strip(row->value)), IniKeywords::Skip)) {
                    skipped = true;
                }
            }

            if (!late.empty() && !replacesTexturesByHash()) {
                setLateBindings(obj, std::move(late));
            }

            // AN OBJECT WHOSE SECTIONS DRAW NOTHING AND SKIP THE GAME'S DRAW IS HIDDEN: the mod hides it. One that draws
            // nothing and leaves the game's geometry on screen is not.
            if (!draws && skipped) {
                hidden.push_back(obj);
            }
        }

        if (!rules.noneUndrawnWhenAll || hidden.size() < read) {
            for (const ModObj& obj : hidden) {
                addUndrawn(obj);
            }
        }
    }

    void IniParseData::clear() {
        remappable_ = true;
        recolourOnly_ = false;
        carriedSections_.clear();
        undrawn_.clear();
        drawRanges_.clear();
        drawParts_.clear();
        libraries_.clear();
        lateBindings_.clear();
        replacesByHash_ = false;
        resourceFiles_.clear();
        textureResources_.clear();
        blendVersion_.reset();
        roles_.reset();
        candidates_.clear();
        roleFiles_.clear();
        rejections_.clear();
        thisToggles_.clear();
        registerToggles_.clear();
        collected_.clear();
        fill_ = nullptr;
    }

    bool IniParseData::hasRemappableContent() const {
        return remappable_;
    }

    void IniParseData::setRemappableContent(bool remappable) {
        remappable_ = remappable;
    }

    bool IniParseData::isRecolourOnly() const {
        return recolourOnly_;
    }

    void IniParseData::setRecolourOnly(bool recolourOnly) {
        recolourOnly_ = recolourOnly;
    }

    const std::string& IniParseData::carriedSections() const {
        return carriedSections_;
    }

    void IniParseData::setCarriedSections(std::string sections) {
        carriedSections_ = std::move(sections);
    }

    bool IniParseData::isUndrawn(const ModObj& modObj) const {
        fill();
        return undrawn_.count(modObj) > 0;
    }

    void IniParseData::addUndrawn(const ModObj& modObj) {
        undrawn_.insert(modObj);
    }

    void IniParseData::clearUndrawn() {
        undrawn_.clear();
    }

    const std::set<IniParseData::ModObj>& IniParseData::undrawnObjs() const {
        fill();
        return undrawn_;
    }

    const std::vector<IniParseData::DrawRange>& IniParseData::drawRanges(const ModObj& modObj) const {
        static const std::vector<DrawRange> none;
        fill();
        const auto found = drawRanges_.find(modObj);
        return found == drawRanges_.end() ? none : found->second;
    }

    void IniParseData::addDrawRange(const ModObj& modObj, DrawRange range) {
        drawRanges_[modObj].push_back(range);
    }

    const std::set<std::size_t>& IniParseData::drawParts(const ModObj& modObj) const {
        static const std::set<std::size_t> none;
        fill();
        const auto found = drawParts_.find(modObj);
        return found == drawParts_.end() ? none : found->second;
    }

    void IniParseData::addDrawPart(const ModObj& modObj, std::size_t part) {
        drawParts_[modObj].insert(part);
    }

    const std::set<std::string>& IniParseData::libraries(const ModObj& modObj) const {
        static const std::set<std::string> none;
        fill();
        const auto found = libraries_.find(modObj);
        return found == libraries_.end() ? none : found->second;
    }

    void IniParseData::addLibrary(const ModObj& modObj, std::string library) {
        libraries_[modObj].insert(std::move(library));
    }

    const std::vector<std::string>& IniParseData::lateBindings(const ModObj& modObj) const {
        static const std::vector<std::string> none;
        fill();
        const auto found = lateBindings_.find(modObj);
        return found == lateBindings_.end() ? none : found->second;
    }

    void IniParseData::setLateBindings(const ModObj& modObj, std::vector<std::string> regs) {
        lateBindings_[modObj] = std::move(regs);
    }

    bool IniParseData::replacesTexturesByHash() const {
        fill();
        return replacesByHash_;
    }

    void IniParseData::setReplacesTexturesByHash(bool replaces) {
        replacesByHash_ = replaces;
    }

    std::optional<std::string> IniParseData::resourceFile(const std::string& section) const {
        fill();
        const auto found = resourceFiles_.find(StringTools::toLower(section));
        if (found == resourceFiles_.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    const std::map<std::string, std::string>& IniParseData::resourceFiles() const {
        fill();
        return resourceFiles_;
    }

    const std::optional<std::string>& IniParseData::blendVersion() const {
        fill();
        return blendVersion_;
    }

    void IniParseData::setBlendVersion(std::string version) {
        blendVersion_ = std::move(version);
    }

    void IniParseData::setTextureResource(const std::string& resource, std::string file) {
        textureResources_[StringTools::toLower(resource)] = std::move(file);
    }

    std::vector<IniParseData::ResourceRole> IniParseData::rolesOfResource(const std::string& resource) const {
        fill();
        std::vector<ResourceRole> result;
        const auto file = textureResources_.find(StringTools::toLower(resource));
        if (file == textureResources_.end()) {
            return result;
        }

        for (const auto& [key, chosen] : roleFiles_) {
            if (chosen.file == file->second) {
                result.push_back(ResourceRole{key.first, key.second});
            }
        }
        return result;
    }

    std::optional<std::string> IniParseData::refusedRoleOf(const std::string& resource) const {
        fill();
        const auto file = textureResources_.find(StringTools::toLower(resource));
        if (file == textureResources_.end()) {
            return std::nullopt;
        }

        for (const Rejection& rejection : rejections_) {
            if (rejection.file == file->second) {
                return rejection.role;
            }
        }
        return std::nullopt;
    }

    void IniParseData::setResourceFile(const std::string& section, std::string file) {
        resourceFiles_.emplace(StringTools::toLower(section), std::move(file));
    }

    // Taken out before it runs: what it fills is set through the same setters a caller would use,
    // and an ask made from inside it must not run it again.
    void IniParseData::fill() const {
        if (fill_) {
            Filler fill = std::move(fill_);
            fill_ = nullptr;
            fill(const_cast<IniParseData&>(*this));
        }
    }

    const TextureRoles* IniParseData::textureRoles() const {
        fill();
        return roles_.get();
    }

    void IniParseData::setTextureRoles(TextureRoles roles) {
        roles_ = std::make_unique<TextureRoles>(std::move(roles));
    }

    const std::vector<IniParseData::CollectedRow>& IniParseData::collected(const std::string& name) const {
        static const std::vector<CollectedRow> none;
        fill();
        const auto found = collected_.find(name);
        return found == collected_.end() ? none : found->second;
    }

    const IniParseData::CollectedRow* IniParseData::firstCollected(const std::string& name, const std::string& section,
                                                                   const std::string& key) const {
        for (const CollectedRow& row : collected(name)) {
            if (row.key == key && StringTools::equalsIgnoreCase(row.section, section)) {
                return &row;
            }
        }
        return nullptr;
    }

    void IniParseData::setCollected(const std::string& name, std::vector<CollectedRow> rows) {
        collected_.insert_or_assign(name, std::move(rows));
    }

    void IniParseData::defer(Filler fill) {
        fill_ = std::move(fill);
    }

    const std::vector<std::pair<std::string, std::string>>& IniParseData::roleCandidates(const std::string& role) const {
        static const std::vector<std::pair<std::string, std::string>> none;
        fill();
        const auto found = candidates_.find(role);
        return found == candidates_.end() ? none : found->second;
    }

    void IniParseData::setRoleCandidates(const std::string& role, std::vector<std::pair<std::string, std::string>> candidates) {
        candidates_[role] = std::move(candidates);
    }

    const IniParseData::RoleFile* IniParseData::roleFile(const std::string& role, const ModObj& modObj) const {
        fill();
        const auto found = roleFiles_.find({role, modObj});
        return found == roleFiles_.end() ? nullptr : &found->second;
    }

    void IniParseData::setRoleFile(const std::string& role, const ModObj& modObj, RoleFile file) {
        roleFiles_[{role, modObj}] = std::move(file);
    }

    const std::vector<IniParseData::Rejection>& IniParseData::rejections() const {
        fill();
        return rejections_;
    }

    void IniParseData::reject(const std::string& file, const std::string& role, const std::string& why) {
        rejections_.push_back(Rejection{file, role, why});
    }

    const IniParseData::TextureToggle* IniParseData::thisToggleOf(const std::string& resource) const {
        fill();
        const auto found = thisToggles_.find(resource);
        return found == thisToggles_.end() ? nullptr : &found->second;
    }

    void IniParseData::setThisToggle(const std::string& resource, TextureToggle toggle) {
        thisToggles_[resource] = std::move(toggle);
    }

    const IniParseData::TextureToggle* IniParseData::registerToggleOf(const std::string& resource) const {
        fill();
        const auto found = registerToggles_.find(resource);
        return found == registerToggles_.end() ? nullptr : &found->second;
    }

    void IniParseData::setRegisterToggle(const std::string& resource, TextureToggle toggle) {
        registerToggles_[resource] = std::move(toggle);
    }
}
