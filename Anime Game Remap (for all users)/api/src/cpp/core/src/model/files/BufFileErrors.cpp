#include "AGRemapCore/model/files/BufFileErrors.h"

#include <utility>


namespace AGRemapCore {
    std::string BadBufData::buildMessage(const std::string& fileType) {
        return "Bytes do not corresponding to the defined format for a " + fileType + " file";
    }

    BadBufData::BadBufData(std::string fileType):
        std::runtime_error(buildMessage(fileType)), fileType_(std::move(fileType)) {}

    const std::string& BadBufData::fileType() const {
        return fileType_;
    }

    std::string BufFileNotRecognized::buildMessage(const std::string& filePath, const std::string& fileType) {
        // The pure-Python original reports only os.path.basename(filePath) in the message itself
        // (the directory is reported separately via FileException's own 'path' formatting) --
        // that split lives entirely in the Python exception class, not reimplemented here (see
        // this class's own doc comment).
        return fileType + " file format not recognized for " + filePath;
    }

    BufFileNotRecognized::BufFileNotRecognized(std::string filePath, std::string fileType):
        std::runtime_error(buildMessage(filePath, fileType)), filePath_(std::move(filePath)), fileType_(std::move(fileType)) {}

    const std::string& BufFileNotRecognized::filePath() const {
        return filePath_;
    }

    const std::string& BufFileNotRecognized::fileType() const {
        return fileType_;
    }
}
