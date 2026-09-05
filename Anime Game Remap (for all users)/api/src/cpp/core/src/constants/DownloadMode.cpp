#include "AGRemapCore/constants/DownloadMode.h"

#include "AGRemapCore/tools/StringTools.h"


namespace AGRemapCore {

    std::string DownloadModeTools::getName(DownloadMode value) {
        switch (value) {
            case DownloadMode::Disabled: return "disabled";
            case DownloadMode::Always: return "always";
            case DownloadMode::Normal: return "normal";
        }

        // Unreachable for a valid enumerator, but a switch over an enum class is not a promise: an
        // int cast into one lands here rather than falling off the end of the function.
        return "normal";
    }

    std::optional<DownloadMode> DownloadModeTools::findByName(const std::string& name) {
        const std::string normalized = StringTools::toLower(StringTools::strip(name));

        if (normalized == "disabled") {
            return DownloadMode::Disabled;
        }

        if (normalized == "normal") {
            return DownloadMode::Normal;
        }

        if (normalized == "always") {
            return DownloadMode::Always;
        }

        return std::nullopt;
    }
}
