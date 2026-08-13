#include "AGRemapCore/tools/Ranges.h"

#include <algorithm>
#include <limits>


namespace AGRemapCore {

template <typename T>
std::optional<T> Ranges<T>::successor(T value) {
    if (value == std::numeric_limits<T>::max()) {
        return std::nullopt;
    }
    return value + 1;
}

template <typename T>
bool Ranges<T>::Bound::operator<(const Bound& other) const {
    if (kind != other.kind) {
        return static_cast<std::int8_t>(kind) < static_cast<std::int8_t>(other.kind);
    }
    if (kind == BoundKind::Finite) {
        return value < other.value;
    }
    // Both NegInf or both PosInf: neither is less than the other.
    return false;
}

template <typename T>
bool Ranges<T>::Bound::operator<=(const Bound& other) const {
    return !(other < *this);
}

template <typename T>
typename Ranges<T>::Bound Ranges<T>::startBound(const std::optional<T>& s) {
    return s.has_value() ? Bound{BoundKind::Finite, *s} : Bound{BoundKind::NegInf, T{}};
}

template <typename T>
typename Ranges<T>::Bound Ranges<T>::endBound(const std::optional<T>& e) {
    return e.has_value() ? Bound{BoundKind::Finite, *e} : Bound{BoundKind::PosInf, T{}};
}

template <typename T>
std::optional<T> Ranges<T>::boundToOptional(const Bound& b) {
    return (b.kind == BoundKind::Finite) ? std::optional<T>(b.value) : std::nullopt;
}

template <typename T>
bool Ranges<T>::boundRangeNonEmpty(const Bound& start, const Bound& end) {
    if (!(start < end)) {
        return false;
    }
    // (-infinity, T::min()) is the one case where 'start < end' holds under
    // Bound's extended-number-line ordering, yet no representable T value
    // actually falls within it, since nothing is less than T::min().
    if (start.kind == BoundKind::NegInf && end.kind == BoundKind::Finite
        && end.value == std::numeric_limits<T>::min()) {
        return false;
    }
    return true;
}

template <typename T>
Ranges<T>::Ranges(std::vector<Range> ranges, bool normalize)
    : ranges(normalize ? normalizeRanges(ranges) : std::move(ranges)) {}

template <typename T>
Ranges<T>::Ranges(const std::vector<T>& values) : ranges(createFromList(values).ranges) {}

template <typename T>
Ranges<T>::Ranges(const std::unordered_set<T>& values)
    : ranges(createFromSet(std::set<T>(values.begin(), values.end())).ranges) {}

template <typename T>
Ranges<T>::Ranges(const std::set<T>& values) : ranges(createFromSet(values).ranges) {}

template <typename T>
std::vector<typename Ranges<T>::Range> Ranges<T>::normalizeRanges(const std::vector<Range>& ranges) {
    if (ranges.empty()) {
        return {};
    }

    // Silently drop degenerate/empty ranges -- either ones with a concrete
    // start and end where start >= end (e.g. [3,3)), or the one edge case
    // where an unbounded start pairs with an end of exactly T::min()
    // (nothing is less than T::min(), so it contains no values either).
    std::vector<Range> filtered;
    filtered.reserve(ranges.size());
    for (const auto& r : ranges) {
        if (!boundRangeNonEmpty(startBound(r.first), endBound(r.second))) {
            continue;
        }
        filtered.push_back(r);
    }

    if (filtered.empty()) {
        return {};
    }

    // Sort by start. std::optional<T>::operator< already treats
    // std::nullopt as less than any concrete value, which is exactly the
    // -infinity semantics we want here.
    std::vector<Range> sortedRanges = std::move(filtered);
    std::sort(sortedRanges.begin(), sortedRanges.end(),
              [](const Range& a, const Range& b) { return a.first < b.first; });

    std::vector<Range> merged;
    for (const auto& r : sortedRanges) {
        const std::optional<T>& start = r.first;
        const std::optional<T>& end = r.second;

        if (merged.empty()) {
            merged.push_back({start, end});
            continue;
        }

        Range& last = merged.back();
        const std::optional<T>& lastEnd = last.second;

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

template <typename T>
bool Ranges<T>::has(T value) const {
    for (const auto& r : ranges) {
        bool afterStart = !r.first.has_value() || *r.first <= value;
        bool beforeEnd = !r.second.has_value() || value < *r.second;
        if (afterStart && beforeEnd) {
            return true;
        }
    }
    return false;
}

template <typename T>
bool Ranges<T>::isEmpty() const {
    return ranges.empty();
}

template <typename T>
bool Ranges<T>::isFull() const {
    for (const auto& r : ranges) {
        if (!r.first.has_value() && !r.second.has_value()) {
            return true;
        }
    }
    return false;
}

template <typename T>
std::int8_t Ranges<T>::compareRangeToValue(const Range& r, const Range& valueProbe) {
    Bound value{BoundKind::Finite, *valueProbe.first};
    Bound start = startBound(r.first);
    Bound end = endBound(r.second);

    if (value < start) {
        // 'r' starts after 'value', so it belongs later in the sorted list.
        return 1;
    }
    if (!(value < end)) {
        // 'r' ends at or before 'value', so it belongs earlier in the list.
        return -1;
    }
    // 'value' falls within [start, end).
    return 0;
}

template <typename T>
void Ranges<T>::add(T value, bool normalize) {
    if (normalize) {
        ranges = normalizeRanges(ranges);
    }

    // The probe's 'second' field is never read by compareRangeToValue, so
    // std::nullopt is a safe placeholder that avoids computing 'value + 1'
    // (which would overflow if value is T's maximum).
    Range probe = {value, std::nullopt};
    bool found = false;
    size_t idx = Algo::binarySearch(ranges, probe, &Ranges<T>::compareRangeToValue, found);

    if (found) {
        // 'value' is already covered by an existing range.
        return;
    }

    std::optional<T> succ = successor(value);

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
        // Likewise, 'nextRange.first' can't be std::nullopt here. If 'succ'
        // is std::nullopt (value is T's maximum), nothing can touch from
        // above, so this correctly stays false.
        nextTouches = succ.has_value() && nextRange.first.has_value() && *nextRange.first == *succ;
    }

    if (prevTouches && nextTouches) {
        // 'value' bridges the gap between two ranges; merge them into one.
        ranges[idx - 1].second = ranges[idx].second;
        ranges.erase(ranges.begin() + idx);
    } else if (prevTouches) {
        ranges[idx - 1].second = succ;
    } else if (nextTouches) {
        ranges[idx].first = value;
    } else {
        ranges.insert(ranges.begin() + idx, {value, succ});
    }
}

template <typename T>
void Ranges<T>::remove(T value, bool normalize) {
    if (normalize) {
        ranges = normalizeRanges(ranges);
    }

    if (ranges.empty()) {
        return;
    }

    // The probe's 'second' field is never read by compareRangeToValue, so
    // std::nullopt is a safe placeholder that avoids computing 'value + 1'
    // (which would overflow if value is T's maximum).
    Range probe = {value, std::nullopt};
    bool found = false;
    size_t idx = Algo::binarySearch(ranges, probe, &Ranges<T>::compareRangeToValue, found);

    if (!found) {
        // 'value' isn't covered by any stored range.
        return;
    }

    const Range r = ranges[idx];

    // Removing a single value from [start, end) leaves up to two pieces:
    // [start, value) on the left and [value + 1, end) on the right. Each is
    // kept only if it's non-empty.
    //
    // If value is T's minimum, there's no representable value below it, so
    // the left piece is always empty regardless of 'start' -- even though
    // 'start' being std::nullopt (-infinity) would otherwise make the
    // naive check look non-empty.
    bool leftNonEmpty = (value != std::numeric_limits<T>::min())
        && (!r.first.has_value() || (*r.first < value));

    // Symmetrically, if value is T's maximum, there's no representable
    // successor, so the right piece is always empty.
    std::optional<T> succ = successor(value);
    bool rightNonEmpty = succ.has_value() && (!r.second.has_value() || (*r.second > *succ));

    std::vector<Range> replacement;
    if (leftNonEmpty) {
        replacement.push_back({r.first, value});
    }
    if (rightNonEmpty) {
        replacement.push_back({succ, r.second});
    }

    ranges.erase(ranges.begin() + idx);
    ranges.insert(ranges.begin() + idx, replacement.begin(), replacement.end());
}

template <typename T>
bool Ranges<T>::operator==(const Ranges& other) const {
    return ranges == other.ranges;
}

template <typename T>
bool Ranges<T>::operator!=(const Ranges& other) const {
    return !(*this == other);
}

template <typename T>
Ranges<T> Ranges<T>::deepcopy() const {
    // std::vector's copy constructor already deep-copies its elements, and
    // Range (a pair of std::optional<T>) holds no pointers or shared
    // state, so a plain value copy of 'ranges' is already a full deep copy.
    return Ranges(ranges, false);
}

template <typename T>
void Ranges<T>::update(const std::vector<Ranges>& ranges) {
    *this = unionWith(ranges);
}

template <typename T>
Ranges<T> Ranges<T>::unionWith(const std::vector<Ranges>& ranges) const {
    std::vector<Range> combined = this->ranges;
    for (const auto& r : ranges) {
        combined.insert(combined.end(), r.ranges.begin(), r.ranges.end());
    }
    return Ranges(normalizeRanges(combined), false);
}

template <typename T>
Ranges<T>& Ranges<T>::operator+=(const Ranges& other) {
    update({other});
    return *this;
}

template <typename T>
Ranges<T> Ranges<T>::operator+(const Ranges& other) const {
    return unionWith({other});
}

template <typename T>
Ranges<T> Ranges<T>::createFull() {
    return Ranges({{std::nullopt, std::nullopt}}, false);
}

template <typename T>
Ranges<T> Ranges<T>::createEmpty() {
    return Ranges({}, false);
}

template <typename T>
Ranges<T> Ranges<T>::createFromList(const std::vector<T>& values) {
    std::vector<T> sortedUnique = values;
    std::sort(sortedUnique.begin(), sortedUnique.end());
    sortedUnique.erase(std::unique(sortedUnique.begin(), sortedUnique.end()), sortedUnique.end());
    return buildFromSortedUnique(sortedUnique);
}

template <typename T>
Ranges<T> Ranges<T>::createFromSet(const std::set<T>& values) {
    // std::set is already sorted and unique.
    std::vector<T> sortedUnique(values.begin(), values.end());
    return buildFromSortedUnique(sortedUnique);
}

template <typename T>
Ranges<T> Ranges<T>::buildFromSortedUnique(const std::vector<T>& sortedUnique) {
    std::vector<Range> result;

    size_t i = 0;
    while (i < sortedUnique.size()) {
        size_t j = i;
        // sortedUnique[j] can only equal T's maximum on the last iteration
        // (nothing sorts after it), so 'j + 1 < sortedUnique.size()' being
        // false short-circuits before 'sortedUnique[j] + 1' would overflow.
        while (j + 1 < sortedUnique.size() && sortedUnique[j + 1] == sortedUnique[j] + 1) {
            ++j;
        }
        // [sortedUnique[i], sortedUnique[j]] is a run of consecutive
        // values; store it as the half-open range [start, end).
        result.push_back({sortedUnique[i], successor(sortedUnique[j])});
        i = j + 1;
    }

    // 'result' is already sorted and disjoint by construction.
    return Ranges(result, false);
}

template <typename T>
Ranges<T> Ranges<T>::difference(const Ranges& other) const {
    std::vector<Range> a = normalizeRanges(this->ranges);
    std::vector<Range> b = normalizeRanges(other.ranges);
    return Ranges(differenceRanges(a, b), false);
}

template <typename T>
Ranges<T> Ranges<T>::operator-(const Ranges& other) const {
    return difference(other);
}

template <typename T>
Ranges<T> Ranges<T>::negate() const {
    return createFull().difference(*this);
}

template <typename T>
Ranges<T> Ranges<T>::operator!() const {
    return negate();
}

template <typename T>
Ranges<T> Ranges<T>::getOverlaps(const std::vector<Ranges>& ranges, bool requireAll, bool normalizeSelf, bool normalizeOthers) const {
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

template <typename T>
Ranges<T> Ranges<T>::intersect(const std::vector<Ranges>& ranges, bool normalizeSelf, bool normalizeOthers) const {
    return getOverlaps(ranges, true, normalizeSelf, normalizeOthers);
}

template <typename T>
void Ranges<T>::intersectUpdate(const std::vector<Ranges>& ranges, bool normalizeSelf, bool normalizeOthers) {
    *this = intersect(ranges, normalizeSelf, normalizeOthers);
}

template <typename T>
std::vector<typename Ranges<T>::Range> Ranges<T>::intersectRanges(const std::vector<Range>& a, const std::vector<Range>& b) {
    std::vector<Range> result;

    size_t i = 0;
    size_t j = 0;
    while (i < a.size() && j < b.size()) {
        Bound aStart = startBound(a[i].first);
        Bound aEnd = endBound(a[i].second);
        Bound bStart = startBound(b[j].first);
        Bound bEnd = endBound(b[j].second);

        Bound overlapStart = (aStart < bStart) ? bStart : aStart;
        Bound overlapEnd = (bEnd < aEnd) ? bEnd : aEnd;

        if (boundRangeNonEmpty(overlapStart, overlapEnd)) {
            result.push_back({boundToOptional(overlapStart), boundToOptional(overlapEnd)});
        }

        if (aEnd < bEnd) {
            ++i;
        } else {
            ++j;
        }
    }

    return result;
}

template <typename T>
std::vector<typename Ranges<T>::Range> Ranges<T>::differenceRanges(const std::vector<Range>& a, const std::vector<Range>& b) {
    std::vector<Range> result;

    size_t j = 0;
    for (const auto& r : a) {
        Bound curStart = startBound(r.first);
        Bound curEnd = endBound(r.second);

        // Ranges in 'b' that end at or before this range's remaining
        // portion starts can never matter again, since 'a' and 'b' are
        // both sorted ascending.
        while (j < b.size() && endBound(b[j].second) <= curStart) {
            ++j;
        }

        size_t k = j;
        while (k < b.size() && curStart < curEnd && startBound(b[k].first) < curEnd) {
            Bound bStart = startBound(b[k].first);
            Bound bEnd = endBound(b[k].second);

            if (boundRangeNonEmpty(curStart, bStart)) {
                result.push_back({boundToOptional(curStart), boundToOptional(bStart)});
            }
            if (curStart < bEnd) {
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

        if (boundRangeNonEmpty(curStart, curEnd)) {
            result.push_back({boundToOptional(curStart), boundToOptional(curEnd)});
        }
    }

    return result;
}

}
