#include "AGRemapCore/RemapServiceCLIErrors.h"

#include <utility>


namespace AGRemapCore {

    // The "ERROR: " prefix is the pure-Python 'Error' base class's, not this exception's own --
    // every exception in that hierarchy carries it, so a message built here without it would read
    // differently from the same failure raised on the Python side.
    std::string InvalidModType::buildMessage(const std::string& modType) {
        return "ERROR: Unable to find the type of mod by the search string, '" + modType + "'";
    }

    InvalidModType::InvalidModType(std::string modType):
        std::runtime_error(buildMessage(modType)), modType_(std::move(modType)) {}

    const std::string& InvalidModType::modType() const {
        return modType_;
    }


    std::string InvalidDownloadMode::buildMessage(const std::string& downloadMode) {
        return "ERROR: Unable to find the download mode by the string, '" + downloadMode + "'";
    }

    InvalidDownloadMode::InvalidDownloadMode(std::string downloadMode):
        std::runtime_error(buildMessage(downloadMode)), downloadMode_(std::move(downloadMode)) {}

    const std::string& InvalidDownloadMode::downloadMode() const {
        return downloadMode_;
    }


    // No "ERROR: " prefix and no mention of the offending string, both deliberate: the pure-Python
    // original raised a plain ValueError with exactly this sentence, and it is worth staying
    // word-for-word since it is the one that tells a user WHAT a valid version looks like.
    InvalidVersion::InvalidVersion(std::string version):
        std::runtime_error("Please enter a valid version that conforms to PEP 440 for the game version"),
        version_(std::move(version)) {}

    const std::string& InvalidVersion::version() const {
        return version_;
    }
}
