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

#include "AGRemapCore/model/iftemplate/ListValue.h"

#include <stdexcept>
#include <utility>

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {
    ListValue::ListValue(std::string value, char separator): original_(std::move(value)), separator_(separator) {
        if (StringTools::strip(original_).empty()) {
            return;
        }

        std::size_t pos = 0;
        while (true) {
            const std::size_t sep = original_.find(separator_, pos);
            fields_.emplace_back(StringTools::strip(original_.substr(pos, sep == std::string::npos ? std::string::npos : sep - pos)));
            if (sep == std::string::npos) {
                break;
            }
            pos = sep + 1;
        }
    }

    std::size_t ListValue::size() const {
        return fields_.size();
    }

    const std::vector<std::string>& ListValue::fields() const {
        return fields_;
    }

    std::optional<std::size_t> ListValue::indexOf(long long pos) const {
        const long long size = static_cast<long long>(fields_.size());
        const long long index = pos < 0 ? size + pos : pos;
        if (index < 0 || index >= size) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(index);
    }

    std::optional<std::string> ListValue::at(long long pos) const {
        const std::optional<std::size_t> index = indexOf(pos);
        if (!index.has_value()) {
            return std::nullopt;
        }
        return fields_[*index];
    }

    std::optional<long long> ListValue::number(long long pos) const {
        const std::optional<std::string> field = at(pos);
        if (!field.has_value()) {
            return std::nullopt;
        }

        const std::size_t digits = (!field->empty() && (*field)[0] == '-') ? 1 : 0;
        if (field->size() == digits || field->find_first_not_of("0123456789", digits) != std::string::npos) {
            return std::nullopt;
        }
        try {
            return std::stoll(*field);
        } catch (const std::out_of_range&) {
            return std::nullopt;
        }
    }

    std::optional<std::vector<long long>> ListValue::numbers() const {
        if (fields_.empty()) {
            return std::nullopt;
        }

        std::vector<long long> result;
        result.reserve(fields_.size());
        for (std::size_t i = 0; i < fields_.size(); ++i) {
            const std::optional<long long> n = number(static_cast<long long>(i));
            if (!n.has_value()) {
                return std::nullopt;
            }
            result.push_back(*n);
        }
        return result;
    }

    bool ListValue::set(long long pos, const std::string& field) {
        const std::optional<std::size_t> index = indexOf(pos);
        if (!index.has_value()) {
            return false;
        }
        fields_[*index] = std::string(StringTools::strip(field));
        changed_ = true;
        return true;
    }

    bool ListValue::setNumber(long long pos, long long number) {
        return set(pos, std::to_string(number));
    }

    void ListValue::append(const std::string& field) {
        fields_.emplace_back(StringTools::strip(field));
        changed_ = true;
    }

    bool ListValue::changed() const {
        return changed_;
    }

    std::string ListValue::str() const {
        if (!changed_) {
            return original_;
        }

        std::string out;
        for (std::size_t i = 0; i < fields_.size(); ++i) {
            if (i > 0) {
                out += separator_;
                out += ' ';
            }
            out += fields_[i];
        }
        return out;
    }
}
