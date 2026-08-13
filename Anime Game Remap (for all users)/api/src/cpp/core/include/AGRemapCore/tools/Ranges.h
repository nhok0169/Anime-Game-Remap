#ifndef AGRemapCore_Ranges_H
#define AGRemapCore_Ranges_H

#include <optional>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "AGRemapCore/tools/Algo.h"


namespace AGRemapCore {

/**
 * @brief
 @rst
 A collection of integer ranges, optionally kept normalized (sorted, merged,
 and reduced to the minimal set of disjoint ranges).
 @endrst
 *
 @rst
 Each range is a pair ``(start, end)`` where:

 * ``start`` is inclusive; ``std::nullopt`` means -infinity
 * ``end`` is exclusive; ``std::nullopt`` means +infinity
 @endrst
 *
 * @tparam T
 @rst
 The integer type used for range boundaries (e.g. ``int``, ``long long``).
 Internally, -infinity/+infinity are represented with a tag rather than a
 numerically wider sentinel value, so ``T`` may safely be the widest integer
 type available
 @endrst
 */
template <typename T>
class Ranges {
    static_assert(std::is_integral_v<T>, "AGRemapCore::Ranges<T> requires T to be an integral type");

    public:
        /**
         * @brief
         @rst
         A single range, represented as a ``(start, end)`` pair
         @endrst
         */
        using Range = std::pair<std::optional<T>, std::optional<T>>;

        /**
         * @brief Constructs a Ranges instance from a list of ranges
         *
         * @param ranges The ranges to store
         * @param normalize
         @rst
         Whether to normalize ``ranges`` before storing it. When ``true``
         (the default), the ranges are sorted, merged, and reduced to the
         minimal set of disjoint ranges. When ``false``, ``ranges`` is
         stored as-is
         @endrst
         */
        explicit Ranges(std::vector<Range> ranges, bool normalize = true);

        /**
         * @brief
         @rst
         Constructs a Ranges instance representing exactly the values in
         ``values`` :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``createFromList(values)``
         @endrst
         *
         * @param values The values to include
         */
        explicit Ranges(const std::vector<T>& values);

        /**
         * @brief
         @rst
         Constructs a Ranges instance representing exactly the values in
         ``values`` :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``createFromSet(std::set<T>(values.begin(), values.end()))``
         @endrst
         *
         * @param values The values to include
         */
        explicit Ranges(const std::unordered_set<T>& values);

        /**
         * @brief
         @rst
         Constructs a Ranges instance representing exactly the values in
         ``values`` :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``createFromSet(values)``
         @endrst
         *
         * @param values The values to include
         */
        explicit Ranges(const std::set<T>& values);

        /**
         * @brief Destroys this Ranges instance
         */
        virtual ~Ranges() = default;

        /**
         * @brief
         @rst
         Checks whether ``value`` falls within any of the stored ranges
         @endrst
         *
         * @param value The value to check
         *
         * @return Whether ``value`` is contained within any of the stored ranges
         */
        virtual bool has(T value) const;

        /**
         * @brief
         @rst
         Checks whether there are no stored ranges
         @endrst
         *
         * @return Whether there are no stored ranges
         */
        virtual bool isEmpty() const;

        /**
         * @brief
         @rst
         Checks whether the stored ranges span from -infinity to +infinity
         @endrst
         *
         @rst
         .. note::
            This checks for a single stored range of ``(None, None)``. If
            this instance was constructed with ``normalize=False``, several
            ranges could collectively cover -infinity to +infinity without
            having been merged into one, in which case this returns
            ``false``
         @endrst
         *
         * @return Whether the stored ranges span from -infinity to +infinity
         */
        virtual bool isFull() const;

        /**
         * @brief
         @rst
         Adds ``value`` to the stored ranges, extending or merging existing
         ranges as needed :raw-html:`<br />` :raw-html:`<br />`

         If ``value`` is already contained within the stored ranges, this
         has no effect
         @endrst
         *
         * @param value The value to add
         * @param normalize
         @rst
         Whether to (re)normalize ``this->ranges`` before adding ``value``
         :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning ``this->ranges`` is assumed to
         already be sorted and disjoint (this method relies on that to
         locate the insertion point efficiently). If that assumption
         doesn't hold, pass ``true``, or the result may be incorrect
         @endrst
         */
        void add(T value, bool normalize = false);

        /**
         * @brief
         @rst
         Removes ``value`` from the stored ranges, shrinking, splitting, or
         erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`

         If ``value`` isn't contained within the stored ranges, this has no
         effect
         @endrst
         *
         * @param value The value to remove
         * @param normalize
         @rst
         Whether to (re)normalize ``this->ranges`` before removing ``value``
         :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning ``this->ranges`` is assumed to
         already be sorted and disjoint (this method relies on that to
         locate ``value`` efficiently). If that assumption doesn't hold,
         pass ``true``, or the result may be incorrect
         @endrst
         */
        void remove(T value, bool normalize = false);

        /**
         * @brief Checks whether two Ranges instances store the same ranges
         *
         * @param other The Ranges instance to compare against
         *
         * @return Whether `ranges` is equal to `other`'s ranges
         */
        bool operator==(const Ranges& other) const;

        /**
         * @copydoc operator==(const Ranges&) const
         */
        bool operator!=(const Ranges& other) const;

        /**
         * @brief
         @rst
         Creates a deep copy of this instance
         @endrst
         *
         * @return A new Ranges instance that is a deep copy of this one
         */
        Ranges deepcopy() const;

        /**
         * @brief
         @rst
         Computes the overlap between this instance and a list of other
         `Ranges`
         @endrst
         *
         * @param ranges The list of other Ranges to compute the overlap against
         * @param requireAll
         @rst
         When ``true`` (the default), the result is the intersection of
         ``this`` and *every* Ranges in ``ranges`` (a value must fall
         within ``this`` and all of ``ranges`` to be included) :raw-html:`<br />` :raw-html:`<br />`

         When ``false``, the result is the intersection of ``this`` and the
         *union* of ``ranges`` (a value must fall within ``this`` and at
         least one of ``ranges`` to be included)
         @endrst
         * @param normalizeSelf
         @rst
         Whether to (re)normalize ``this->ranges`` before computing the
         overlap :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning ``this->ranges`` is assumed to
         already be sorted and disjoint. If that assumption doesn't hold,
         pass ``true``, or the computed overlap may be incorrect
         @endrst
         * @param normalizeOthers
         @rst
         Whether to (re)normalize each entry of ``ranges`` before computing
         the overlap :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning every Ranges in ``ranges`` is
         assumed to already be sorted and disjoint. If that assumption
         doesn't hold, pass ``true``, or the computed overlap may be
         incorrect
         @endrst
         *
         * @return A new Ranges instance containing the computed overlap
         */
        Ranges getOverlaps(const std::vector<Ranges>& ranges, bool requireAll = true, bool normalizeSelf = false, bool normalizeOthers = false) const;

        /**
         * @brief
         @rst
         Computes the intersection of this instance and a list of other
         `Ranges` :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``getOverlaps(ranges, true, normalizeSelf, normalizeOthers)``
         @endrst
         *
         * @param ranges The list of other Ranges to intersect with
         * @param normalizeSelf
         @rst
         Whether to (re)normalize ``this->ranges`` before computing the
         intersection :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning ``this->ranges`` is assumed to
         already be sorted and disjoint. If that assumption doesn't hold,
         pass ``true``, or the computed intersection may be incorrect
         @endrst
         * @param normalizeOthers
         @rst
         Whether to (re)normalize each entry of ``ranges`` before computing
         the intersection :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning every Ranges in ``ranges`` is
         assumed to already be sorted and disjoint. If that assumption
         doesn't hold, pass ``true``, or the computed intersection may be
         incorrect
         @endrst
         *
         * @return A new Ranges instance containing the intersection of ``this`` and every entry in ``ranges``
         */
        Ranges intersect(const std::vector<Ranges>& ranges, bool normalizeSelf = false, bool normalizeOthers = false) const;

        /**
         * @brief
         @rst
         Performs the intersection of this instance with a list of other
         `Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``this``
         @endrst
         *
         * @param ranges The list of other Ranges to intersect with
         * @param normalizeSelf
         @rst
         Whether to (re)normalize ``this->ranges`` before computing the
         intersection :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning ``this->ranges`` is assumed to
         already be sorted and disjoint. If that assumption doesn't hold,
         pass ``true``, or the computed intersection may be incorrect
         @endrst
         * @param normalizeOthers
         @rst
         Whether to (re)normalize each entry of ``ranges`` before computing
         the intersection :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``, meaning every Ranges in ``ranges`` is
         assumed to already be sorted and disjoint. If that assumption
         doesn't hold, pass ``true``, or the computed intersection may be
         incorrect
         @endrst
         */
        void intersectUpdate(const std::vector<Ranges>& ranges, bool normalizeSelf = false, bool normalizeOthers = false);

        /**
         * @brief
         @rst
         Performs the union of this instance with a list of other `Ranges`,
         in place
         @endrst
         *
         * @param ranges The list of other Ranges to union with
         */
        void update(const std::vector<Ranges>& ranges);

        /**
         * @brief
         @rst
         Computes the union of this instance with a list of other `Ranges`
         @endrst
         *
         @rst
         .. note::
            Named ``unionWith`` rather than ``union``, since ``union`` is a
            reserved keyword in C++
         @endrst
         *
         * @param ranges The list of other Ranges to union with
         *
         * @return A new Ranges instance containing the union of ``this`` and ``ranges``
         */
        Ranges unionWith(const std::vector<Ranges>& ranges) const;

        /**
         * @brief
         @rst
         Performs the union of this instance with ``other``, in place :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``update({other})``
         @endrst
         *
         * @param other The Ranges to union with
         *
         * @return A reference to this instance
         */
        Ranges& operator+=(const Ranges& other);

        /**
         * @brief
         @rst
         Computes the union of this instance with ``other`` :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``unionWith({other})``
         @endrst
         *
         * @param other The Ranges to union with
         *
         * @return A new Ranges instance containing the union of ``this`` and ``other``
         */
        Ranges operator+(const Ranges& other) const;

        /**
         * @brief
         @rst
         Computes the set difference between this instance and ``other``
         (``self - other``)
         @endrst
         *
         * @param other The Ranges to subtract from this instance
         *
         * @return A new Ranges instance containing the values in ``this`` that are not in ``other``
         */
        Ranges difference(const Ranges& other) const;

        /**
         * @copydoc difference(const Ranges&) const
         */
        Ranges operator-(const Ranges& other) const;

        /**
         * @brief
         @rst
         Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`

         Equivalent to ``createFull() - self``
         @endrst
         *
         * @return A new Ranges instance containing every value not in ``this``
         */
        Ranges negate() const;

        /**
         * @copydoc negate() const
         */
        Ranges operator!() const;

        /**
         * @brief
         @rst
         Creates a Ranges instance spanning from -infinity to +infinity
         @endrst
         *
         * @return A new Ranges instance containing a single range from -infinity to +infinity
         */
        static Ranges createFull();

        /**
         * @brief
         @rst
         Creates a Ranges instance with no ranges
         @endrst
         *
         * @return A new, empty Ranges instance
         */
        static Ranges createEmpty();

        /**
         * @brief
         @rst
         Creates a Ranges instance representing exactly the values in
         ``values`` :raw-html:`<br />` :raw-html:`<br />`

         Consecutive integers are merged into contiguous ranges (e.g.
         ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)
         @endrst
         *
         * @param values The values to include
         *
         * @return A new Ranges instance whose ranges cover exactly the values in ``values``
         */
        static Ranges createFromList(const std::vector<T>& values);

        /**
         * @copydoc createFromList(const std::vector<T>&)
         */
        static Ranges createFromSet(const std::set<T>& values);

        /**
         * @brief The ranges stored by this instance
         */
        std::vector<Range> ranges;

    private:
        /**
         * @brief
         @rst
         A value on the extended number line: -infinity, a finite ``T``
         value, or +infinity :raw-html:`<br />` :raw-html:`<br />`

         Used internally so range-overlap arithmetic never needs a
         numerically wider "sentinel" type to represent infinity, which
         wouldn't exist if ``T`` is already the widest integer type
         available. Supports a total order where ``NegInf < Finite < PosInf``
         @endrst
         */
        enum class BoundKind : std::int8_t { NegInf, Finite, PosInf };

        /**
         * @copydoc BoundKind
         */
        struct Bound {
            BoundKind kind;
            T value{};

            bool operator<(const Bound& other) const;
            bool operator<=(const Bound& other) const;
        };

        /**
         * @brief Converts a range's ``start`` field into a Bound, where ``std::nullopt`` means -infinity
         *
         * @param s The optional start value to convert
         *
         * @return The corresponding Bound
         */
        static Bound startBound(const std::optional<T>& s);

        /**
         * @brief Converts a range's ``end`` field into a Bound, where ``std::nullopt`` means +infinity
         *
         * @param e The optional end value to convert
         *
         * @return The corresponding Bound
         */
        static Bound endBound(const std::optional<T>& e);

        /**
         * @brief
         @rst
         Converts a Bound back into an optional range boundary :raw-html:`<br />` :raw-html:`<br />`

         Both ``BoundKind::NegInf`` and ``BoundKind::PosInf`` map to
         ``std::nullopt``; which infinity that represents depends on
         whether the result is used as a ``start`` or an ``end`` field
         @endrst
         *
         * @param b The Bound to convert
         *
         * @return The corresponding optional value
         */
        static std::optional<T> boundToOptional(const Bound& b);

        /**
         * @brief
         @rst
         Checks whether the half-open range represented by ``[start, end)``
         actually contains at least one representable ``T`` value :raw-html:`<br />` :raw-html:`<br />`

         This is *not* simply ``start < end``: the pair
         ``(NegInf, Finite(T::min()))`` satisfies ``start < end`` under
         Bound's extended-number-line ordering, yet no representable ``T``
         value is actually less than ``T::min()``, so it must still be
         treated as empty. There is no equivalent issue at the top end,
         since ``[Finite(T::max()), PosInf)`` always contains ``T::max()``
         itself
         @endrst
         *
         * @param start The start bound
         * @param end The end bound
         *
         * @return Whether ``[start, end)`` contains at least one representable value
         */
        static bool boundRangeNonEmpty(const Bound& start, const Bound& end);

        /**
         * @brief
         @rst
         Returns the value immediately after ``value``, or ``std::nullopt``
         if ``value`` is the maximum representable ``T`` (in which case
         there is no finite successor, and the result should be treated as
         +infinity) :raw-html:`<br />` :raw-html:`<br />`

         Used to avoid signed-overflow undefined behavior when computing
         ``value + 1`` at the edge of ``T``'s representable range
         @endrst
         *
         * @param value The value to compute the successor of
         *
         * @return The successor of ``value``, or ``std::nullopt`` if there isn't one
         */
        static std::optional<T> successor(T value);

        /**
         * @brief
         @rst
         Sorts ``ranges`` by their starting index (treating ``std::nullopt``
         as -infinity) and merges any ranges that overlap or touch (e.g.
         ``[1,2)`` and ``[2,5)`` merge into ``[1,5)``) :raw-html:`<br />` :raw-html:`<br />`

         Any degenerate range with a concrete start and end where
         ``start >= end`` (e.g. ``[3,3)``) is silently dropped
         @endrst
         *
         * @param ranges The ranges to normalize
         *
         * @return The minimal set of disjoint ranges equivalent to ``ranges``
         */
        static std::vector<Range> normalizeRanges(const std::vector<Range>& ranges);

        /**
         * @brief
         @rst
         Computes the intersection of two normalized (sorted, disjoint)
         range lists
         @endrst
         *
         * @param a The first normalized range list
         * @param b The second normalized range list
         *
         * @return The normalized range list representing the intersection of ``a`` and ``b``
         */
        static std::vector<Range> intersectRanges(const std::vector<Range>& a, const std::vector<Range>& b);

        /**
         * @brief
         @rst
         Computes the set difference between two normalized (sorted,
         disjoint) range lists (``a - b``)
         @endrst
         *
         * @param a The normalized range list to subtract from
         * @param b The normalized range list to subtract
         *
         * @return The normalized range list representing ``a - b``
         */
        static std::vector<Range> differenceRanges(const std::vector<Range>& a, const std::vector<Range>& b);

        /**
         * @brief
         @rst
         A three-way compare between a range and a probe range representing
         a single value (as produced by ``{value, value + 1}``), for use
         with :cpp:func:`AGRemapCore::Algo::binarySearch` :raw-html:`<br />` :raw-html:`<br />`

         Returns a positive value if ``r`` starts after ``valueProbe``'s
         value, a negative value if ``r`` ends at or before it, or ``0`` if
         ``r`` contains it
         @endrst
         *
         * @param r The range to compare
         * @param valueProbe A probe range whose ``first`` field holds the value being searched for
         *
         * @return A three-way comparison result, as described above
         */
        static std::int8_t compareRangeToValue(const Range& r, const Range& valueProbe);

        /**
         * @brief
         @rst
         Builds a Ranges instance from a sorted list of unique values,
         merging consecutive runs into contiguous ranges
         @endrst
         *
         * @param sortedUnique A sorted list of unique values
         *
         * @return A new Ranges instance whose ranges cover exactly the values in ``sortedUnique``
         */
        static Ranges buildFromSortedUnique(const std::vector<T>& sortedUnique);
};

}

#include "Ranges.tpp"

#endif