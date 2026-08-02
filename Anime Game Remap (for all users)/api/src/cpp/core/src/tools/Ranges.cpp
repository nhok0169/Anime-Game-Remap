#include "AGRemapCore/tools/Ranges.h"
 
#include <algorithm>
#include <limits>
 

namespace AGRemapCore {
    Ranges::Ranges(std::vector<Range> ranges, bool normalize)
        : ranges(normalize ? normalizeRanges(ranges) : std::move(ranges)) {}
    
    Ranges::Ranges(const std::vector<int>& values) : ranges(createFromList(values).ranges) {}
    
    Ranges::Ranges(const std::unordered_set<int>& values)
        : ranges(createFromSet(std::set<int>(values.begin(), values.end())).ranges) {}
    
    Ranges::Ranges(const std::set<int>& values) : ranges(createFromSet(values).ranges) {}
    
    std::vector<Ranges::Range> Ranges::normalizeRanges(const std::vector<Range>& ranges) {
        if (ranges.empty()) {
            return {};
        }
    
        // Silently drop degenerate/invalid ranges, i.e. ones with a concrete
        // start and end where start >= end (an empty or backwards range). A
        // range with an unbounded side (std::nullopt) can never be degenerate,
        // since one side being infinite always leaves room on the other.
        std::vector<Range> filtered;
        filtered.reserve(ranges.size());
        for (const auto& r : ranges) {
            if (r.first.has_value() && r.second.has_value() && *r.first >= *r.second) {
                continue;
            }
            filtered.push_back(r);
        }
    
        if (filtered.empty()) {
            return {};
        }
    
        // Sort by start, treating std::nullopt as -infinity.
        std::vector<Range> sortedRanges = std::move(filtered);
        auto startKey = [](const Range& r) -> long long {
            return r.first.has_value() ? static_cast<long long>(*r.first)
                                        : std::numeric_limits<long long>::min();
        };
        std::sort(sortedRanges.begin(), sortedRanges.end(),
                [&](const Range& a, const Range& b) { return startKey(a) < startKey(b); });
    
        std::vector<Range> merged;
        for (const auto& r : sortedRanges) {
            const std::optional<int>& start = r.first;
            const std::optional<int>& end = r.second;
    
            if (merged.empty()) {
                merged.push_back({start, end});
                continue;
            }
    
            Range& last = merged.back();
            const std::optional<int>& lastEnd = last.second;
    
            // lastEnd being nullopt (+infinity) or start being nullopt
            // (-infinity) both guarantee the current range overlaps/touches
            // the last one.
            bool overlaps = !lastEnd.has_value() || !start.has_value() || (*start <= *lastEnd);
    
            if (overlaps) {
                if (!end.has_value() || !lastEnd.has_value()) {
                    last.second = std::nullopt;
                } else {
                    last.second = std::max(*lastEnd, *end);
                }
            } else {
                merged.push_back({start, end});
            }
        }
    
        return merged;
    }
    
    bool Ranges::has(int value) const {
        for (const auto& r : ranges) {
            bool afterStart = !r.first.has_value() || *r.first <= value;
            bool beforeEnd = !r.second.has_value() || value < *r.second;
            if (afterStart && beforeEnd) {
                return true;
            }
        }
        return false;
    }
    
    bool Ranges::isEmpty() const {
        return ranges.empty();
    }
    
    bool Ranges::isFull() const {
        for (const auto& r : ranges) {
            if (!r.first.has_value() && !r.second.has_value()) {
                return true;
            }
        }
        return false;
    }
    
    std::int8_t Ranges::compareRangeToValue(const Range& r, const Range& valueProbe) {
        int value = *valueProbe.first;
        long long start = r.first.has_value() ? static_cast<long long>(*r.first) : std::numeric_limits<long long>::min();
        long long end = r.second.has_value() ? static_cast<long long>(*r.second) : std::numeric_limits<long long>::max();
    
        if (value < start) {
            // 'r' starts after 'value', so it belongs later in the sorted list.
            return 1;
        }
        if (value >= end) {
            // 'r' ends at or before 'value', so it belongs earlier in the list.
            return -1;
        }
        // 'value' falls within [start, end).
        return 0;
    }
    
    void Ranges::add(int value, bool normalize) {
        if (normalize) {
            ranges = normalizeRanges(ranges);
        }
    
        Range probe = {value, value + 1};
        bool found = false;
        size_t idx = Algo::binarySearch(ranges, probe, &Ranges::compareRangeToValue, found);
    
        if (found) {
            // 'value' is already covered by an existing range.
            return;
        }
    
        bool prevTouches = false;
        bool nextTouches = false;
        if (idx > 0) {
            const Range& prevRange = ranges[idx - 1];
            // 'prevRange.second' can't be std::nullopt here: an unbounded end
            // would mean 'value' was already found above.
            prevTouches = prevRange.second.has_value() && *prevRange.second == value;
        }
        if (idx < ranges.size()) {
            const Range& nextRange = ranges[idx];
            // Likewise, 'nextRange.first' can't be std::nullopt here.
            nextTouches = nextRange.first.has_value() && *nextRange.first == value + 1;
        }
    
        if (prevTouches && nextTouches) {
            // 'value' bridges the gap between two ranges; merge them into one.
            ranges[idx - 1].second = ranges[idx].second;
            ranges.erase(ranges.begin() + idx);
        } else if (prevTouches) {
            ranges[idx - 1].second = value + 1;
        } else if (nextTouches) {
            ranges[idx].first = value;
        } else {
            ranges.insert(ranges.begin() + idx, {value, value + 1});
        }
    }
    
    void Ranges::remove(int value, bool normalize) {
        if (normalize) {
            ranges = normalizeRanges(ranges);
        }
    
        if (ranges.empty()) {
            return;
        }
    
        Range probe = {value, value + 1};
        bool found = false;
        size_t idx = Algo::binarySearch(ranges, probe, &Ranges::compareRangeToValue, found);
    
        if (!found) {
            // 'value' isn't covered by any stored range.
            return;
        }
    
        const Range r = ranges[idx];
    
        // Removing a single value from [start, end) leaves up to two pieces:
        // [start, value) on the left and [value + 1, end) on the right. Each is
        // kept only if it's non-empty.
        bool leftNonEmpty = !r.first.has_value() || (*r.first < value);
        bool rightNonEmpty = !r.second.has_value() || (*r.second > value + 1);
    
        std::vector<Range> replacement;
        if (leftNonEmpty) {
            replacement.push_back({r.first, value});
        }
        if (rightNonEmpty) {
            replacement.push_back({value + 1, r.second});
        }
    
        ranges.erase(ranges.begin() + idx);
        ranges.insert(ranges.begin() + idx, replacement.begin(), replacement.end());
    }
    
    bool Ranges::operator==(const Ranges& other) const {
        return ranges == other.ranges;
    }
    
    bool Ranges::operator!=(const Ranges& other) const {
        return !(*this == other);
    }
    
    Ranges Ranges::deepcopy() const {
        // std::vector's copy constructor already deep-copies its elements, and
        // Range (a pair of std::optional<int>) holds no pointers or shared
        // state, so a plain value copy of 'ranges' is already a full deep copy.
        return Ranges(ranges, false);
    }
    
    void Ranges::update(const std::vector<Ranges>& ranges) {
        *this = unionWith(ranges);
    }
    
    Ranges Ranges::unionWith(const std::vector<Ranges>& ranges) const {
        std::vector<Range> combined = this->ranges;
        for (const auto& r : ranges) {
            combined.insert(combined.end(), r.ranges.begin(), r.ranges.end());
        }
        return Ranges(normalizeRanges(combined), false);
    }
    
    Ranges& Ranges::operator+=(const Ranges& other) {
        update({other});
        return *this;
    }
    
    Ranges Ranges::operator+(const Ranges& other) const {
        return unionWith({other});
    }
    
    Ranges Ranges::createFull() {
        return Ranges({{std::nullopt, std::nullopt}}, false);
    }
    
    Ranges Ranges::createEmpty() {
        return Ranges({}, false);
    }
    
    Ranges Ranges::createFromList(const std::vector<int>& values) {
        std::vector<int> sortedUnique = values;
        std::sort(sortedUnique.begin(), sortedUnique.end());
        sortedUnique.erase(std::unique(sortedUnique.begin(), sortedUnique.end()), sortedUnique.end());
        return buildFromSortedUnique(sortedUnique);
    }
    
    Ranges Ranges::createFromSet(const std::set<int>& values) {
        // std::set is already sorted and unique.
        std::vector<int> sortedUnique(values.begin(), values.end());
        return buildFromSortedUnique(sortedUnique);
    }
    
    Ranges Ranges::buildFromSortedUnique(const std::vector<int>& sortedUnique) {
        std::vector<Range> result;
    
        size_t i = 0;
        while (i < sortedUnique.size()) {
            size_t j = i;
            while (j + 1 < sortedUnique.size() && sortedUnique[j + 1] == sortedUnique[j] + 1) {
                ++j;
            }
            // [sortedUnique[i], sortedUnique[j]] is a run of consecutive
            // integers; store it as the half-open range [start, end).
            result.push_back({sortedUnique[i], sortedUnique[j] + 1});
            i = j + 1;
        }
    
        // 'result' is already sorted and disjoint by construction.
        return Ranges(result, false);
    }
    
    Ranges Ranges::difference(const Ranges& other) const {
        std::vector<Range> a = normalizeRanges(this->ranges);
        std::vector<Range> b = normalizeRanges(other.ranges);
        return Ranges(differenceRanges(a, b), false);
    }
    
    Ranges Ranges::operator-(const Ranges& other) const {
        return difference(other);
    }
    
    Ranges Ranges::negate() const {
        return createFull().difference(*this);
    }
    
    Ranges Ranges::operator!() const {
        return negate();
    }
    
    Ranges Ranges::getOverlaps(const std::vector<Ranges>& ranges, bool requireAll, bool normalizeSelf, bool normalizeOthers) const {
        std::vector<Range> selfRanges = normalizeSelf ? normalizeRanges(this->ranges) : this->ranges;
    
        if (requireAll) {
            std::vector<Range> result = selfRanges;
            for (const auto& r : ranges) {
                std::vector<Range> rRanges = normalizeOthers ? normalizeRanges(r.ranges) : r.ranges;
                result = intersectRanges(result, rRanges);
            }
            return Ranges(result, false);
        }
    
        // Union together the passed-in ranges. Concatenating them will only be
        // sorted/disjoint on its own if every entry in 'ranges' is individually
        // normalized *and* none of them overlap one another; otherwise, this
        // must be normalized to become a valid union.
        std::vector<Range> combined;
        for (const auto& r : ranges) {
            combined.insert(combined.end(), r.ranges.begin(), r.ranges.end());
        }
        std::vector<Range> unioned = normalizeOthers ? normalizeRanges(combined) : combined;
    
        std::vector<Range> result = intersectRanges(selfRanges, unioned);
        return Ranges(result, false);
    }
    
    Ranges Ranges::intersect(const std::vector<Ranges>& ranges, bool normalizeSelf, bool normalizeOthers) const {
        return getOverlaps(ranges, true, normalizeSelf, normalizeOthers);
    }
    
    void Ranges::intersectUpdate(const std::vector<Ranges>& ranges, bool normalizeSelf, bool normalizeOthers) {
        *this = intersect(ranges, normalizeSelf, normalizeOthers);
    }
    
    std::vector<Ranges::Range> Ranges::intersectRanges(const std::vector<Range>& a, const std::vector<Range>& b) {
        std::vector<Range> result;
    
        auto startVal = [](const std::optional<int>& s) -> long long {
            return s.has_value() ? static_cast<long long>(*s) : std::numeric_limits<long long>::min();
        };
        auto endVal = [](const std::optional<int>& e) -> long long {
            return e.has_value() ? static_cast<long long>(*e) : std::numeric_limits<long long>::max();
        };
    
        size_t i = 0;
        size_t j = 0;
        while (i < a.size() && j < b.size()) {
            long long aStart = startVal(a[i].first);
            long long aEnd = endVal(a[i].second);
            long long bStart = startVal(b[j].first);
            long long bEnd = endVal(b[j].second);
    
            long long overlapStart = std::max(aStart, bStart);
            long long overlapEnd = std::min(aEnd, bEnd);
    
            if (overlapStart < overlapEnd) {
                // The overlap is only unbounded on a side if both source
                // ranges were unbounded on that same side.
                std::optional<int> resStart = (!a[i].first.has_value() && !b[j].first.has_value())
                    ? std::nullopt
                    : std::optional<int>(static_cast<int>(overlapStart));
                std::optional<int> resEnd = (!a[i].second.has_value() && !b[j].second.has_value())
                    ? std::nullopt
                    : std::optional<int>(static_cast<int>(overlapEnd));
                result.push_back({resStart, resEnd});
            }
    
            if (aEnd < bEnd) {
                ++i;
            } else {
                ++j;
            }
        }
    
        return result;
    }
    
    std::vector<Ranges::Range> Ranges::differenceRanges(const std::vector<Range>& a, const std::vector<Range>& b) {
        std::vector<Range> result;
    
        auto startVal = [](const std::optional<int>& s) -> long long {
            return s.has_value() ? static_cast<long long>(*s) : std::numeric_limits<long long>::min();
        };
        auto endVal = [](const std::optional<int>& e) -> long long {
            return e.has_value() ? static_cast<long long>(*e) : std::numeric_limits<long long>::max();
        };
        auto toOpt = [](long long v) -> std::optional<int> {
            // Both sentinels are far outside the range of an actual int, so
            // this unambiguously identifies values that originated as
            // std::nullopt (-infinity or +infinity, depending on which side
            // they're used for).
            if (v == std::numeric_limits<long long>::min() || v == std::numeric_limits<long long>::max()) {
                return std::nullopt;
            }
            return static_cast<int>(v);
        };
    
        size_t j = 0;
        for (const auto& r : a) {
            long long curStart = startVal(r.first);
            long long curEnd = endVal(r.second);
    
            // Ranges in 'b' that end at or before this range's remaining
            // portion starts can never matter again, since 'a' and 'b' are
            // both sorted ascending.
            while (j < b.size() && endVal(b[j].second) <= curStart) {
                ++j;
            }
    
            size_t k = j;
            while (k < b.size() && curStart < curEnd && startVal(b[k].first) < curEnd) {
                long long bStart = startVal(b[k].first);
                long long bEnd = endVal(b[k].second);
    
                if (bStart > curStart) {
                    result.push_back({toOpt(curStart), toOpt(bStart)});
                }
                if (bEnd > curStart) {
                    curStart = bEnd;
                }
    
                if (bEnd <= curEnd) {
                    // This 'b' range is fully consumed within the current 'a'
                    // range; move on to the next one.
                    ++k;
                } else {
                    // This 'b' range extends past the current 'a' range, so it
                    // may still overlap the next 'a' range; leave it for then.
                    break;
                }
            }
            j = k;
    
            if (curStart < curEnd) {
                result.push_back({toOpt(curStart), toOpt(curEnd)});
            }
        }
    
        return result;
    }
}