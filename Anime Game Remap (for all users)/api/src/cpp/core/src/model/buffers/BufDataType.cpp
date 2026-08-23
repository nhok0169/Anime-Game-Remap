#include "AGRemapCore/model/buffers/BufDataType.h"

#include <stdexcept>
#include <utility>


namespace AGRemapCore {
    void BufDataType::validateSize(std::size_t size) {
        if (size == 0 || size > 8) {
            throw std::invalid_argument("BufDataType size must be between 1 and 8 bytes, got " + std::to_string(size));
        }
    }

    BufDataType::BufDataType(std::string name, std::size_t size, bool isBigEndian):
        BufType(std::move(name)), size_(0), isBigEndian_(isBigEndian) {

        setSize(size);
    }

    std::size_t BufDataType::getSize() const {
        return size_;
    }

    void BufDataType::setSize(std::size_t size) {
        validateSize(size);
        size_ = size;
    }

    bool BufDataType::getIsBigEndian() const {
        return isBigEndian_;
    }

    void BufDataType::setIsBigEndian(bool isBigEndian) {
        isBigEndian_ = isBigEndian;
    }
}
