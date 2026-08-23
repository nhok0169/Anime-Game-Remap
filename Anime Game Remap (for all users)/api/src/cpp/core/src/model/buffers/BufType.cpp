#include "AGRemapCore/model/buffers/BufType.h"

#include <utility>


namespace AGRemapCore {
    BufType::BufType(std::string name): name_(std::move(name)) {}

    const std::string& BufType::getName() const {
        return name_;
    }

    void BufType::setName(std::string name) {
        name_ = std::move(name);
    }
}
