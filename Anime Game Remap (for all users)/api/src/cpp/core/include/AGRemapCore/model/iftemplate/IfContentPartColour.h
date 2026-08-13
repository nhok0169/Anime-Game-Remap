#ifndef AGRemapCore_IfContentPartColour_H
#define AGRemapCore_IfContentPartColour_H

#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
    class IfContentPartColouring;

    /**
     * @brief
     @rst
     Class to store the change in state of a particular key for an :cpp:class:`IfContentPartColouring`
     -- the C++ port of ``IfContentPartColourChange`` from ``IfContentPartColour.py``.
     @endrst
     *
     * @tparam V The type of the values stored by the owning :cpp:class:`IfContentPartColouring`
     */
    template <typename V>
    class IfContentPartColourChange {
        public:
            /**
             * @brief A single ``(index, value)`` occurrence, as stored by the current :cpp:class:`IfContentPart`
             */
            using IndexedValue = std::pair<long long, V>;

            /**
             * @brief
             @rst
             The state stored for a key within an :cpp:class:`IfContentPartColouring` -- either:

             * a plain ``V``, meaning the value carried over from some previous :cpp:class:`IfContentPart`, OR
             * a ``std::vector<IndexedValue>``, meaning the values come from the current :cpp:class:`IfContentPart`,
               each paired with its index of occurrence within that part
             @endrst
             */
            using StateValue = std::variant<V, std::vector<IndexedValue>>;

            /**
             * @brief Constructs a new change record
             *
             * @param old
             @rst
             The old value of the key, before the change -- ``std::nullopt`` means the key didn't
             exist beforehand :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit IfContentPartColourChange(std::optional<StateValue> old = std::nullopt);

            /**
             * @brief
             @rst
             Creates a copy of this change record, via the compiler-generated copy constructor --
             a structural copy only (any ``V`` held by :cpp:member:`old` is itself copy-constructed,
             not recursively deep-copied), matching :cpp:func:`IfContentPart::clone`'s own
             shallow-clone convention.
             @endrst
             *
             * @return A new, independent change record equal to this one
             */
            std::unique_ptr<IfContentPartColourChange<V>> clone() const;

            /**
             * @brief
             @rst
             Restores ``colouring``'s value for ``key`` back to what it was before this change
             @endrst
             *
             * @tparam K The type of the keys stored in ``colouring``
             * @tparam KeyHash A hasher for ``K``, matching ``colouring``'s own
             * @tparam KeyEqual An equality comparator for ``K``, matching ``colouring``'s own
             * @tparam ValueHash A hasher for ``V``, matching ``colouring``'s own
             * @tparam ValueEqual An equality comparator for ``V``, matching ``colouring``'s own
             *
             * @param colouring The colouring to restore a value within
             * @param key
             @rst
             The key to restore -- if ``key`` isn't currently in ``colouring``, this has no effect
             @endrst
             */
            template <typename K, typename KeyHash, typename KeyEqual, typename ValueHash, typename ValueEqual>
            void restore(IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>& colouring, const K& key) const;

            /**
             * @brief The old value of the key, before the change
             */
            std::optional<StateValue> old;
    };

    /**
     * @brief
     @rst
     Class that keeps track of the current state of the `KVPs`_ within an :cpp:class:`IfContentPart`
     -- the C++ port of ``IfContentPartColouring`` from ``IfContentPartColour.py``. :raw-html:`<br />` :raw-html:`<br />`

     A thin, ordered dict-like wrapper (backed by a `tsl::ordered_map`_) from key to
     :cpp:type:`StateValue`:

     * A plain ``V`` means the value's `KVP`_ comes from some previous :cpp:class:`IfContentPart`
     * A ``std::vector<IndexedValue>`` means the values come from the current :cpp:class:`IfContentPart`,
       each paired with its index of occurrence within that part

     Unlike the deprecated Python version's ``getVals`` (which returns either a ``list`` or a
     ``set``, chosen by its ``unique`` flag), this class splits that into two separately-typed
     methods, :cpp:func:`getVals` and :cpp:func:`getUniqueVals` -- matching how :cpp:class:`IfContentPart`
     itself already splits ``getVals``/``getKeys`` (a ``std::vector`` vs a real hash set) rather than
     returning a union type C++ can't express directly.
     @endrst
     *
     * @tparam K The type of the keys tracked by this colouring
     * @tparam V The type of the values tracked by this colouring
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     * @tparam ValueHash A hasher for ``V``, used only by :cpp:func:`getUniqueVals`. Defaults to ``std::hash<V>``
     * @tparam ValueEqual An equality comparator for ``V``, used only by :cpp:func:`getUniqueVals`. Defaults to ``std::equal_to<V>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
              typename ValueHash = std::hash<V>, typename ValueEqual = std::equal_to<V>>
    class IfContentPartColouring {
        public:
            /**
             * @brief The concrete :cpp:class:`IfContentPart` instantiation this colouring tracks state for
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc IfContentPartColourChange::IndexedValue
             */
            using IndexedValue = std::pair<long long, V>;

            /**
             * @copydoc IfContentPartColourChange::StateValue
             */
            using StateValue = std::variant<V, std::vector<IndexedValue>>;

            /**
             * @brief The change-record type produced/consumed by :cpp:func:`updateColouring`/:cpp:func:`restore`
             */
            using Change = IfContentPartColourChange<V>;

            /**
             * @brief
             @rst
             A predicate over an occurrence -- its index (``std::nullopt`` if the value was carried
             over from a previous part) and value
             @endrst
             */
            using Filter = std::function<bool(std::optional<long long>, const V&)>;

            IfContentPartColouring() = default;

            /**
             * @brief
             @rst
             Creates a copy of this colouring, via the compiler-generated copy constructor -- a
             structural copy only (every tracked ``V`` is itself copy-constructed, not recursively
             deep-copied), matching :cpp:func:`IfContentPart::clone`'s own shallow-clone convention.
             @endrst
             *
             * @return A new, independent colouring equal to this one
             */
            std::unique_ptr<IfContentPartColouring<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual>> clone() const;

            /**
             * @brief Checks whether ``key`` currently has a tracked state
             */
            bool contains(const K& key) const;

            /**
             * @brief Retrieves the number of keys currently tracked
             */
            size_t size() const;

            /**
             * @brief Checks whether no keys are currently tracked
             */
            bool empty() const;

            /**
             * @brief Retrieves the current state for ``key``, if any
             *
             * @param key The key to look up
             *
             * @return The current state for ``key``, or ``std::nullopt`` if ``key`` isn't tracked
             */
            std::optional<StateValue> get(const K& key) const;

            /**
             * @brief Retrieves the current state for ``key``
             *
             * @param key The key to look up
             *
             * @throw std::out_of_range Thrown when ``key`` isn't currently tracked
             *
             * @return The current state for ``key``
             */
            const StateValue& at(const K& key) const;

            /**
             * @brief Sets the current state for ``key``, inserting it if not already tracked
             */
            void set(const K& key, StateValue value);

            /**
             * @brief Removes the current state for ``key``, if any
             *
             * @return Whether ``key`` was actually tracked (and so removed)
             */
            bool erase(const K& key);

            /**
             * @brief Removes every tracked key
             */
            void clear();

            /**
             * @brief Retrieves every currently-tracked key, in insertion order
             */
            std::vector<K> keys() const;

            /**
             * @brief Retrieves every currently-tracked ``(key, state)`` pair, in insertion order
             */
            std::vector<std::pair<K, StateValue>> items() const;

            /**
             * @brief
             @rst
             Updates the current state of the `KVPs`_ based on the current :cpp:class:`IfContentPart`
             @endrst
             *
             * @param ifContentPart The part to update the new `KVPs`_ from
             * @param targetKeys
             @rst
             The target keys to keep track of -- if ``std::nullopt`` (the default), every key in
             ``ifContentPart`` is tracked
             @endrst
             * @param updatePreviousKVPs
             @rst
             Whether to also update the `KVP`_ values from previous :cpp:class:`IfContentPart`\\s
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             *
             * @return
             @rst
             The change in state -- the keys are the names of the changed keys and the values are
             the state each one had before this call
             @endrst
             */
            std::unordered_map<K, Change, KeyHash, KeyEqual> updateColouring(
                const ContentPart& ifContentPart,
                const std::optional<std::unordered_set<K, KeyHash, KeyEqual>>& targetKeys = std::nullopt,
                bool updatePreviousKVPs = true);

            /**
             * @brief Restores this colouring to a previous state
             *
             * @param colourChange The change in state to undo, as returned by :cpp:func:`updateColouring`
             */
            void restore(const std::unordered_map<K, Change, KeyHash, KeyEqual>& colourChange);

            /**
             * @brief
             @rst
             Retrieves both the corresponding values and the index of where the value occurs :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike :cpp:func:`getVals`, ``filter`` is only ever applied when ``key``'s state
                comes from the current :cpp:class:`IfContentPart` (a list of indexed occurrences) --
                a value carried over from a previous part is always returned unfiltered, as
                ``(std::nullopt, value)``. This intentionally mirrors an asymmetry already present
                in the original Python source between its own ``getIndVals``/``getVals``.
             @endrst
             *
             * @param key The key to search for
             * @param filter
             @rst
             A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``, meaning every value is returned
             @endrst
             *
             * @return
             @rst
             Both the values and their index within the current :cpp:class:`IfContentPart`. If an
             index is ``std::nullopt``, the value appeared before the current part. Empty if ``key``
             isn't tracked.
             @endrst
             */
            std::vector<std::pair<std::optional<long long>, V>> getIndVals(const K& key, const std::optional<Filter>& filter = std::nullopt) const;

            /**
             * @brief Retrieves the values for a given key, keeping duplicates and occurrence order
             *
             * @param key The key to search for
             * @param filter
             @rst
             A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``, meaning every value is returned
             @endrst
             *
             * @return The resultant values. Empty if ``key`` isn't tracked.
             */
            std::vector<V> getVals(const K& key, const std::optional<Filter>& filter = std::nullopt) const;

            /**
             * @brief
             @rst
             Same as :cpp:func:`getVals`, except the result is deduplicated into a real hash set
             (using this instantiation's own ``ValueHash``/``ValueEqual``)
             @endrst
             *
             * @param key The key to search for
             * @param filter
             @rst
             A predicate to filter certain values returned :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``, meaning every value is returned
             @endrst
             *
             * @return The resultant unique values. Empty if ``key`` isn't tracked.
             */
            std::unordered_set<V, ValueHash, ValueEqual> getUniqueVals(const K& key, const std::optional<Filter>& filter = std::nullopt) const;

            /**
             * @brief
             @rst
             Retrieves the ranges of indices within the current part that satisfy specified
             conditions for each key
             @endrst
             *
             * @param keysExists
             @rst
             Checks whether a key exists or does not exist -- the keys are the names of the
             registers and the values are whether to check for the existence/non-existence of the
             register :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param keyFilters
             @rst
             The conditions to satisfy for each key -- the keys are the names of the registers and
             the values are the predicates :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param existsRequireAll
             @rst
             Whether the retrieved ranges must satisfy all existence checks at ``keysExists`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             * @param filtersRequireAll
             @rst
             Whether the retrieved ranges must satisfy all the predicates specified at ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             * @param globalRequireAll
             @rst
             Whether the retrieved ranges must satisfy checks in both ``keysExists`` and ``keyFilters`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             * @param includeKeyDefs
             @rst
             Whether to include indices where the values for the keys specified at ``keysExists``
             or ``keyFilters`` are being (re)defined :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             *
             * @return The valid ranges that satisfy the specified conditions
             */
            Ranges<long long> getRanges(
                const std::optional<std::unordered_map<K, bool, KeyHash, KeyEqual>>& keysExists = std::nullopt,
                const std::optional<std::unordered_map<K, Filter, KeyHash, KeyEqual>>& keyFilters = std::nullopt,
                bool existsRequireAll = true, bool filtersRequireAll = true, bool globalRequireAll = true,
                bool includeKeyDefs = true) const;

        private:
            // Common lookup used by both getIndVals() and the getVals()/getUniqueVals() pair --
            // NOT shared verbatim with getIndVals() itself, since getIndVals() intentionally
            // skips 'filter' for a flat/carried-over state value while getVals()/getUniqueVals()
            // intentionally apply it there too (see getIndVals()'s doc comment) -- this helper
            // only builds the raw (index, value) pairs, unfiltered.
            std::vector<std::pair<std::optional<long long>, V>> rawIndVals(const K& key) const;

            tsl::ordered_map<K, StateValue, KeyHash, KeyEqual> data_;
    };

}

#include "IfContentPartColour.tpp"

#endif
