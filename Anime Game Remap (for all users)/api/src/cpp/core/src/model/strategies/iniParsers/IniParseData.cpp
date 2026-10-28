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


namespace AGRemapCore {
    void IniParseData::clear() {
        remappable_ = true;
        recolourOnly_ = false;
        carriedSections_.clear();
        undrawn_.clear();
        roles_.reset();
        candidates_.clear();
        roleFiles_.clear();
        rejections_.clear();
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
        return undrawn_.count(modObj) > 0;
    }

    void IniParseData::addUndrawn(const ModObj& modObj) {
        undrawn_.insert(modObj);
    }

    void IniParseData::clearUndrawn() {
        undrawn_.clear();
    }

    const std::set<IniParseData::ModObj>& IniParseData::undrawnObjs() const {
        return undrawn_;
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
}
