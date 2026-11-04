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
#include "AGRemapCore/model/IniNamingTools.h"
#include "AGRemapCore/model/iftemplate/ListValue.h"
#include "AGRemapCore/tools/ListTools.h"
#include "AGRemapCore/tools/StringTools.h"

#include <set>
#include <algorithm>
#include <tuple>


namespace AGRemapCore {
    void IniParseData::resourceData(const ResourceDataRules& rules) {
        // the files of the resources the graphs bind -- each section's first `filename`
        if (!rules.resources.empty()) {
            std::set<std::string> filed;
            for (const auto& row : collected(rules.resources)) {
                if (filed.insert(row.section).second) {
                    setResourceFile(row.section, std::string(StringTools::strip(row.value)));
                }
            }
        }

        // every `this =`, and each TextureOverride's first hash and first `this` -- a section's own lines once each,
        // the sections in the order they are written
        if (!rules.sections.empty()) {
            std::set<std::tuple<std::string, std::size_t, long long>> once;
            std::map<std::string, std::vector<const CollectedRow*>> own;
            std::vector<std::string> order;
            for (const auto& row : collected(rules.sections)) {
                if (!once.emplace(row.section, row.partId, row.line).second) {
                    continue;
                }
                auto& lines = own[row.section];
                if (lines.empty()) {
                    order.push_back(row.section);
                }
                lines.push_back(&row);
            }

            const auto written = [](const CollectedRow* a, const CollectedRow* b) {
                return std::make_pair(a->partId, a->line) < std::make_pair(b->partId, b->line);
            };
            for (auto& entry : own) {
                std::stable_sort(entry.second.begin(), entry.second.end(), written);
            }
            std::stable_sort(order.begin(), order.end(), [&own, &written](const std::string& a, const std::string& b) {
                return written(own.at(a).front(), own.at(b).front());
            });

            for (const std::string& section : order) {
                std::optional<std::string> hash;
                std::optional<std::string> resource;
                for (const CollectedRow* row : own.at(section)) {
                    if (row->key == IniKeywords::This) {
                        addThisBound(std::string(StringTools::strip(row->value)));
                        if (!resource.has_value()) {
                            resource = std::string(StringTools::strip(row->value));
                        }
                    } else if (row->key == IniKeywords::Hash && !hash.has_value()) {
                        hash = std::string(StringTools::strip(row->value));
                    }
                }
                if (StringTools::startsWith(section, IniKeywords::TextureOverride) && hash.has_value() && resource.has_value()) {
                    setThisOverride(*hash, *resource);
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

            // THE RESOURCES BOUND ONLY UNDER A TOGGLE: a line's path is the branch of each `if` block it sits in
            // (GraphCollectRow::blocks), and a line reaches a draw when its path encloses the draw's
            {
                using Path = std::vector<std::size_t>;
                const auto encloses = [](const Path& outer, const Path& inner) {
                    return outer.size() <= inner.size() && std::equal(outer.begin(), outer.end(), inner.begin());
                };
                const std::string ref = StringTools::toLower(IniKeywords::Ref) + " ";
                std::map<std::string, std::vector<std::pair<Path, std::string>>> occurrences;
                std::vector<Path> drawPaths;
                for (const auto& [rootAt, partAt, line, row] : inRoots) {
                    (void)rootAt;
                    (void)partAt;
                    (void)line;
                    const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));
                    if (StringTools::startsWith(key, "draw")) {
                        drawPaths.push_back(row->blocks);
                        continue;
                    }
                    if (!StringTools::startsWith(key, regPrefix)) {
                        continue;
                    }
                    std::string val(StringTools::strip(row->value));
                    if (StringTools::startsWith(StringTools::toLower(val), ref)) {
                        val = std::string(StringTools::lstrip(val.substr(ref.size())));
                    }
                    if (!val.empty()) {
                        occurrences[StringTools::toLower(val)].emplace_back(row->blocks, key);
                    }
                }

                if (!drawPaths.empty()) {
                    const auto reaches = [&drawPaths, &encloses](const Path& path) {
                        return std::any_of(drawPaths.begin(), drawPaths.end(), [&](const Path& draw) { return encloses(path, draw); });
                    };
                    std::set<std::string> defaulted;
                    for (const auto& [resource, lines] : occurrences) {
                        (void)resource;
                        for (const auto& [path, key] : lines) {
                            if (reaches(path)) {
                                defaulted.insert(key);
                            }
                        }
                    }

                    std::set<std::string> toggled;
                    for (const auto& [resource, lines] : occurrences) {
                        const bool onlyToggled = std::none_of(lines.begin(), lines.end(), [&](const auto& line) {
                            return reaches(line.first) || defaulted.count(line.second) > 0;
                        });
                        if (onlyToggled) {
                            toggled.insert(resource);
                        }
                    }
                    if (!toggled.empty()) {
                        setToggledBindings(obj, std::move(toggled));
                    }
                }
            }

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
                        const std::optional<std::vector<long long>> numbers = ListValue(draw).numbers();
                        if (numbers.has_value() && numbers->size() >= 3 && (*numbers)[0] >= 0 && (*numbers)[1] >= 0) {
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

        if (!rules.toggles.empty()) {
            fileToggles(rules);
        }
    }

    void IniParseData::fileToggles(const ResourceDataRules& rules) {
        using Roots = std::map<std::string, std::vector<ModObj>>;
        using OwnRows = std::map<std::string, std::vector<const CollectedRow*>>;

        // the objects each root section is filed under
        const auto rootsOf = [](const std::vector<std::pair<ModObj, std::vector<std::string>>>& objs, Roots& into) {
            for (const auto& [obj, sections] : objs) {
                for (const std::string& section : sections) {
                    into[section].push_back(obj);
                }
            }
        };

        // a collect's rows in a section's own parts, when the section is a root of the row's graph
        const auto ownRows = [this](const std::string& name, const Roots& roots, OwnRows& into) {
            for (const auto& row : collected(name)) {
                const auto root = roots.find(row.section);
                if (root != roots.end()
                        && std::find(root->second.begin(), root->second.end(), row.graphId.modObj) != root->second.end()) {
                    into[row.section].push_back(&row);
                }
            }
        };

        // the sections that may hold a toggle, in the order they are written; each one's rows as written
        const auto order = [&rules](OwnRows& own) {
            std::vector<std::string> sections;
            for (auto& [section, rows] : own) {
                const bool holds = rules.togglesIn ? rules.togglesIn(section) : StringTools::startsWith(section, IniKeywords::TextureOverride);
                if (!holds) {
                    continue;
                }
                std::stable_sort(rows.begin(), rows.end(), [](const CollectedRow* a, const CollectedRow* b) {
                    return std::make_tuple(a->position, a->line) < std::make_tuple(b->position, b->line);
                });
                sections.push_back(section);
            }
            std::stable_sort(sections.begin(), sections.end(), [&own](const std::string& a, const std::string& b) {
                const auto first = [&own](const std::string& section) {
                    std::pair<std::size_t, long long> at = {own.at(section).front()->partId, own.at(section).front()->line};
                    for (const CollectedRow* row : own.at(section)) {
                        at = std::min(at, std::make_pair(row->partId, row->line));
                    }
                    return at;
                };
                return first(a) < first(b);
            });
            return sections;
        };

        // the sections this file declares
        std::set<std::string> declared;
        if (!rules.sections.empty()) {
            for (const auto& row : collected(rules.sections)) {
                declared.insert(row.section);
            }
        }
        const auto resolves = [&declared](const std::vector<std::string>& vals) {
            return static_cast<std::size_t>(std::count_if(vals.begin(), vals.end(),
                                                          [&declared](const std::string& val) { return declared.count(val) > 0; }));
        };

        // ---- THIS TOGGLES: a texture override binding several resources on its branches, or one under a condition
        // that is a real toggle (an `else` is a toggle's other side) ----
        Roots textureRoots;
        rootsOf(rules.textureRoots, textureRoots);
        OwnRows own;
        ownRows(rules.toggles, textureRoots, own);

        std::map<std::string, std::string> ownerOf;
        std::map<std::string, std::vector<std::string>> variantsOf;
        std::set<std::string> single;
        for (const std::string& name : order(own)) {
            std::vector<std::string> bound;
            bool realToggle = false;
            for (const CollectedRow* row : own.at(name)) {
                for (const std::string& guard : row->guards) {
                    std::string test = StringTools::toLower(guard);
                    test.erase(std::remove_if(test.begin(), test.end(),
                                              [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }),
                               test.end());
                    if (std::find(rules.notToggles.begin(), rules.notToggles.end(), test) == rules.notToggles.end()) {
                        realToggle = true;
                    }
                }
                if (row->key == IniKeywords::This) {
                    bound.push_back(std::string(StringTools::strip(row->value)));
                }
            }

            if (bound.size() == 1 && realToggle) {
                const std::string key = StringTools::toLower(bound.front());
                if (ownerOf.count(key) == 0) {
                    ownerOf[key] = name;
                    variantsOf[name] = bound;
                    single.insert(key);
                }
                continue;
            }
            if (bound.size() < 2) {
                continue;
            }

            // two sections may bind one resource behind a toggle: the one declaring more of its own variants keeps it
            const std::size_t score = resolves(bound);
            for (const std::string& resource : bound) {
                const std::string key = StringTools::toLower(resource);
                const auto owner = ownerOf.find(key);
                if (owner != ownerOf.end()) {
                    const auto held = variantsOf.find(owner->second);
                    if (held != variantsOf.end() && resolves(held->second) >= score) {
                        continue;
                    }
                }
                ownerOf[key] = name;
            }
            variantsOf[name] = bound;
        }

        for (const auto& [resource, section] : ownerOf) {
            const auto graph = textureRoots.find(section);
            setThisToggle(resource, TextureToggle{section, variantsOf[section], single.count(resource) > 0,
                                                  graph == textureRoots.end() ? ModObj() : graph->second.front()});
        }

        // ---- REGISTER TOGGLES: a texture override binding one register to several resources on its branches -- per
        // (section, register), since two registers of one section are two roles ----
        Roots bindingRoots = textureRoots;
        rootsOf(rules.roots, bindingRoots);
        OwnRows bindings;
        ownRows(rules.toggles, bindingRoots, bindings);
        if (!rules.rows.empty()) {
            ownRows(rules.rows, bindingRoots, bindings);
        }

        const std::string prefix = StringTools::toLower(rules.texRegPrefix);
        for (const std::string& name : order(bindings)) {
            std::map<std::string, std::vector<std::string>> boundOfReg;
            for (const CollectedRow* row : bindings.at(name)) {
                const std::string key = StringTools::toLower(std::string(StringTools::strip(row->key)));
                if (!StringTools::startsWith(key, prefix)) {
                    continue;
                }
                const std::string val = IniNamingTools::removeRefPrefix(std::string(StringTools::strip(row->value)));
                if (val.empty() || StringTools::equalsIgnoreCase(val, IniKeywords::Null)) {
                    continue;
                }
                std::vector<std::string>& vals = boundOfReg[key];
                if (std::none_of(vals.begin(), vals.end(), [&val](const std::string& had) { return StringTools::equalsIgnoreCase(had, val); })) {
                    vals.push_back(val);
                }
            }
            for (const auto& [reg, vals] : boundOfReg) {
                if (vals.size() < 2) {
                    continue;
                }
                for (const std::string& val : vals) {
                    setRegisterToggle(StringTools::toLower(val), TextureToggle{name + " " + reg, vals, false});
                }
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
        toggledBindings_.clear();
        thisOverrides_.clear();
        thisBound_.clear();
        replacesByHash_ = false;
        resourceFiles_.clear();
        textureResources_.clear();
        fileResources_.clear();
        blendVersion_.reset();
        roles_.reset();
        candidates_.clear();
        roleFiles_.clear();
        rejections_.clear();
        thisToggles_.clear();
        registerToggles_.clear();
        collected_.clear();
        sectionIndexes_.clear();
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

    const std::set<std::string>& IniParseData::toggledBindings(const ModObj& modObj) const {
        static const std::set<std::string> none;
        fill();
        const auto found = toggledBindings_.find(modObj);
        return found == toggledBindings_.end() ? none : found->second;
    }

    void IniParseData::setToggledBindings(const ModObj& modObj, std::set<std::string> resources) {
        toggledBindings_[modObj] = std::move(resources);
    }

    std::optional<std::string> IniParseData::thisOverrideOf(const std::string& hash) const {
        fill();
        const auto found = thisOverrides_.find(StringTools::toLower(hash));
        return found == thisOverrides_.end() ? std::nullopt : std::optional<std::string>(found->second);
    }

    void IniParseData::setThisOverride(const std::string& hash, std::string resource) {
        thisOverrides_.emplace(StringTools::toLower(hash), std::move(resource));
    }

    bool IniParseData::isThisBound(const std::string& resource) const {
        fill();
        return thisBound_.count(StringTools::toLower(resource)) > 0;
    }

    void IniParseData::addThisBound(const std::string& resource) {
        thisBound_.insert(StringTools::toLower(resource));
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

    std::optional<std::string> IniParseData::resourceOfFile(const std::string& file) const {
        fill();
        const auto found = fileResources_.find(file);
        return found == fileResources_.end() ? std::nullopt : std::optional<std::string>(found->second);
    }

    void IniParseData::setFileResource(const std::string& file, std::string resource) {
        fileResources_[file] = std::move(resource);
    }

    std::vector<IniParseData::ResourceRole> IniParseData::rolesOfResource(const std::string& resource) const {
        fill();
        std::vector<ResourceRole> result;
        const auto file = textureResources_.find(StringTools::toLower(resource));
        if (file == textureResources_.end()) {
            return result;
        }

        for (const auto& [key, chosen] : roleFiles_) {
            if (chosen.file == file->second
                    || std::find(chosen.variants.begin(), chosen.variants.end(), file->second) != chosen.variants.end()) {
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
        sectionIndexes_.erase(name);
    }

    const IniParseData::SectionIndex& IniParseData::sectionIndex(const std::string& name) const {
        const std::vector<CollectedRow>& rows = collected(name);
        const auto found = sectionIndexes_.find(name);
        if (found != sectionIndexes_.end()) {
            return found->second;
        }

        // A collect files its rows key by key, so they are put back in the order of their parts, whose ids are
        // handed out as the file is parsed -- which orders the sections too
        SectionIndex& index = sectionIndexes_[name];
        std::set<std::tuple<std::string, std::size_t, long long>> seen;
        for (const CollectedRow& row : rows) {
            if (!seen.emplace(row.section, row.partId, row.line).second) {
                continue;
            }

            auto& sectionRows = index.rows[row.section];
            if (sectionRows.empty()) {
                index.order.push_back(row.section);
            }
            sectionRows.push_back(&row);
        }

        const auto written = [](const CollectedRow* a, const CollectedRow* b) {
            return std::make_pair(a->partId, a->line) < std::make_pair(b->partId, b->line);
        };
        for (auto& entry : index.rows) {
            std::stable_sort(entry.second.begin(), entry.second.end(), written);
        }
        std::stable_sort(index.order.begin(), index.order.end(), [&index, &written](const std::string& a, const std::string& b) {
            return written(index.rows.at(a).front(), index.rows.at(b).front());
        });
        return index;
    }

    const std::vector<std::string>& IniParseData::collectedSections(const std::string& name) const {
        return sectionIndex(name).order;
    }

    const std::vector<const IniParseData::CollectedRow*>& IniParseData::sectionRows(const std::string& name,
                                                                                  const std::string& section) const {
        static const std::vector<const CollectedRow*> none;
        const SectionIndex& index = sectionIndex(name);
        const auto found = index.rows.find(section);
        return found == index.rows.end() ? none : found->second;
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
