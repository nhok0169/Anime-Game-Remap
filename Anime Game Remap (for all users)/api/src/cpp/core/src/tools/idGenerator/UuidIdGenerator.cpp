#include "AGRemapCore/tools/idGenerator/UuidIdGenerator.h"

#include <cstdint>
#include <iomanip>
#include <sstream>


namespace AGRemapCore {

    UuidIdGenerator::UuidIdGenerator() {
        std::random_device rd;
        std::seed_seq seed{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
        engine_.seed(seed);
    }

    void UuidIdGenerator::reset() {
        // no persistent counter/sequence state to reset -- see the class-level note
    }

    bool UuidIdGenerator::getId(std::string& result) {
        std::uniform_int_distribution<std::uint64_t> dist;
        std::uint64_t hi = dist(engine_);
        std::uint64_t lo = dist(engine_);

        // RFC 4122: version 4 in the top nibble of time_hi_and_version (bits 12-15 of 'hi'),
        // variant 1 ("10xxxxxx") in the top 2 bits of clock_seq_hi_and_reserved (top 2 bits of 'lo')
        hi = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        lo = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(8) << static_cast<std::uint32_t>(hi >> 32) << '-'
            << std::setw(4) << static_cast<std::uint32_t>((hi >> 16) & 0xFFFFULL) << '-'
            << std::setw(4) << static_cast<std::uint32_t>(hi & 0xFFFFULL) << '-'
            << std::setw(4) << static_cast<std::uint32_t>(lo >> 48) << '-'
            << std::setw(12) << (lo & 0xFFFFFFFFFFFFULL);

        result = oss.str();
        return true;
    }
}
