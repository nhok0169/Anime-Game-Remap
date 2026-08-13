#ifndef AGRemapCore_IOrderedMultiMap_H
#define AGRemapCore_IOrderedMultiMap_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/tools/Ranges.h"
#include "AGRemapCore/tools/orderedMultiMap/BaseOrderedMultiMap.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     An abstract, type-erased view of the ordered-multimap API implemented by
     :cpp:class:`BaseOrderedMultiMap`'s `CRTP`_ hierarchy (:cpp:class:`OrderedMultiMap`,
     :cpp:class:`OrderedMultiMapSqrt`), letting code that needs to select or swap the backing
     structure at runtime -- or accept an entirely user-supplied implementation, including one
     implemented from `Python`_ -- do so through a single, uniform type, without forcing every
     direct user of :cpp:class:`OrderedMultiMap`/:cpp:class:`OrderedMultiMapSqrt` to pay for a
     vtable they don't need. :raw-html:`<br />` :raw-html:`<br />`

     This is deliberately **not** a base class of :cpp:class:`BaseOrderedMultiMap`: adding any
     virtual method to an ancestor of :cpp:class:`BaseOrderedMultiMap` would put a vtable
     pointer in *every* :cpp:class:`OrderedMultiMap`/:cpp:class:`OrderedMultiMapSqrt` instance
     and block the compiler from inlining through the `CRTP`_ ``self()`` cast -- exactly the
     zero-overhead property :cpp:class:`BaseOrderedMultiMap`'s own doc comment calls out as the
     reason it uses `CRTP`_ in the first place. Instead, :cpp:class:`OrderedMultiMapAdapter`
     wraps any `CRTP`_-shaped implementation to satisfy this interface only where polymorphism
     is actually needed. :raw-html:`<br />` :raw-html:`<br />`

     Every "map-like"/"set-like" bulk parameter here (`insertAllAt`'s ``items``, `reorder`'s
     ``orderMap``, `replaceVals`'s ``newVals``, `splitByInds`'s ``inds``) is a plain
     ``std::vector`` rather than a `tsl::ordered_map`_/`tsl::ordered_set`_ or
     ``std::unordered_map``: a vector already has an unambiguous order with no hashing
     requirement on ``K`` at all, so this interface -- unlike :cpp:class:`BaseOrderedMultiMap`
     -- never needs a ``KeyHash``/``KeyEqual`` template parameter, and every parameter type here
     converts cleanly through `pybind11`_'s ``<pybind11/stl.h>`` for the `Python`_ trampoline
     binding. :raw-html:`<br />` :raw-html:`<br />`

     A class implementing this interface directly (rather than via
     :cpp:class:`OrderedMultiMapAdapter`) has full latitude over its own internal storage, at
     the cost of reimplementing every method itself -- :cpp:class:`BaseOrderedMultiMap`'s
     `reorder`/`remapKeys`/`replaceVals`/etc. machinery is not available outside the `CRTP`_
     hierarchy. Implementing the small ``rawXxx`` primitive set instead (see
     :cpp:class:`BaseOrderedMultiMap`'s doc comment) and wrapping the result in
     :cpp:class:`OrderedMultiMapAdapter` gets that machinery for free.
     @endrst
     *
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     */
    template <typename K, typename V>
    class IOrderedMultiMap {
        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define RemoveKeyCheck std::function<bool(long long, const V&)>
            #define Predicate std::function<bool(const V&)>
            #define ReplaceSpec std::variant<V, std::vector<V>, std::pair<V, Predicate>>
            #define RemapListItem std::variant<K, RemappedKeyData<K, V>>
            #define KeyRemapList RemapList<K, V>
            #define KeyRemapValue std::variant<KeyRemapList, KeyRemapData<K, V>>
            #define RangeSpec std::vector<Ranges<long long>::Range>
            #else
            /**
             * @brief
             @rst
             Used by :cpp:func:`removeKey`: ``(true positional index, value) -> should this
             occurrence be removed?``
             @endrst
             */
            using RemoveKeyCheck = std::function<bool(long long, const V&)>;

            /**
             * @brief
             @rst
             Used by :cpp:func:`replaceVals`: a predicate deciding whether a given old value
             should be replaced
             @endrst
             */
            using Predicate = std::function<bool(const V&)>;

            /**
             * @brief
             @rst
             Used by :cpp:func:`replaceVals`: a value to assign, a list of values to assign
             positionally, or a ``(value, predicate)`` pair to conditionally assign :raw-html:`<br />` :raw-html:`<br />`

             Structurally identical to :cpp:type:`BaseOrderedMultiMap::ReplaceSpec` for the same
             ``V`` -- a plain alias never creates a distinct type, so values of one are usable
             directly as the other, with no conversion needed.
             @endrst
             */
            using ReplaceSpec = std::variant<V, std::vector<V>, std::pair<V, Predicate>>;

            /**
             * @copydoc RemapListItem
             */
            using RemapListItem = std::variant<K, RemappedKeyData<K, V>>;

            /**
             * @copydoc BaseOrderedMultiMap::KeyRemapList
             */
            using KeyRemapList = RemapList<K, V>;

            /**
             * @copydoc BaseOrderedMultiMap::KeyRemapValue
             */
            using KeyRemapValue = std::variant<KeyRemapList, KeyRemapData<K, V>>;

            /**
             * @brief
             @rst
             A plain, non-polymorphic stand-in for a `pybind11`_-facing ``ranges`` parameter,
             carrying the exact same information as :cpp:class:`Ranges`\\<long long\\> (a list of
             ``(start, end)`` bounds) without being :cpp:class:`Ranges` itself. :raw-html:`<br />` :raw-html:`<br />`

             :cpp:class:`Ranges` has virtual methods (``has``/``isEmpty``/``isFull``), so it's
             polymorphic -- and only the `pybind11`_-bound `PyRanges`\\<T\\> (see
             ``py/src/tools/PyRanges.h``), not :cpp:class:`Ranges`\\<T\\> itself, is registered
             with `pybind11`_. A plain :cpp:class:`Ranges`\\<long long\\> value crossing this
             interface's `Python`_ trampoline (see :cpp:class:`OrderedMultiMapAdapter`'s doc
             comment) has no registered type for `pybind11`_'s RTTI-based polymorphic lookup to
             find, so every ``ranges`` parameter here uses this plain, unambiguous shape instead
             -- constructible into a real :cpp:class:`Ranges`\\<long long\\> via its
             ``std::vector<Range>`` constructor with no information loss.
             @endrst
             */
            using RangeSpec = std::vector<Ranges<long long>::Range>;
            #endif

            /**
             * @brief A single owned entry, as yielded by :cpp:func:`items`
             @rst
             Unlike :cpp:class:`BaseOrderedMultiMap::Iterator::Item`, whose ``key``/``value``
             are references into internal storage that only a `CRTP`_-concrete type can safely
             expose, this holds owned copies -- appropriate for a virtual interface, where the
             concrete storage behind any given call is unknown.
             @endrst
             */
            struct Item {
                K key;
                V value;
                size_t occurrenceIndex;
                size_t orderIndex;
            };

            virtual ~IOrderedMultiMap() = default;

            /**
             * @copydoc BaseOrderedMultiMap::insert
             */
            virtual void insert(const K& key, const V& value) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::insertStart
             */
            virtual void insertStart(const K& key, const V& value) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::insertAt
             */
            virtual void insertAt(long long index, const K& key, const V& value) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::insertAllEnd
             */
            virtual void insertAllEnd(const std::vector<std::pair<K, V>>& items) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::insertAllStart
             */
            virtual void insertAllStart(const std::vector<std::pair<K, V>>& items) = 0;

            /**
             * @brief
             @rst
             Bulk indexed insert: inserts many key-value pairs at their own target indices in a
             single pass. Index semantics match :cpp:func:`insertAt` (Python-style negative
             indices, clamping), but with "original position" (numpy-style) semantics: each
             index refers to a position in the sequence as it was *before* this call, not a
             position in the growing result. :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Doxygen can't resolve a ``@copybrief``/``@copydoc`` cross-reference to a
                :cpp:class:`BaseOrderedMultiMap` overload disambiguated by a complex templated
                parameter list (e.g. ``tsl::ordered_map<...>``) from an unrelated class -- it
                silently renders nothing rather than erroring, so this doc (and
                :cpp:func:`reorder`/:cpp:func:`replaceVals` below) is written out directly
                instead of copied, even though the underlying behavior is identical.
             @endrst
             *
             * @param items
             @rst
             The (index, key-value pair) entries to insert, in the order they should be
             considered for tie-breaking (i.e. ``items``' own order takes the role
             `tsl::ordered_map`_'s iteration order plays in the `CRTP`_ overload)
             @endrst
             * @param sortIndices
             @rst
             If ``true`` (the default), ``items`` is stable-sorted by normalized index first, so
             the single :math:`O(n+m)` cursor pass afterward can advance strictly forward. If you
             already know ``items`` iterates in ascending normalized-index order, pass ``false``
             to skip that sort and drop the cost to :math:`O(n+m)` -- **this precondition is
             unchecked**, and violating it produces a silently wrong (not crashing) result.
             @endrst
             * @param ranges
             @rst
             If provided, an entry is only inserted when its normalized target index falls within
             ``ranges``; filtered entries are dropped before sorting/the cursor pass, so they
             never affect tie-breaking among the entries that do get inserted.
             @endrst
             *
             * @return How many entries were actually inserted
             */
            virtual size_t insertAllAt(const std::vector<std::pair<long long, std::pair<K, V>>>& items,
                                        bool sortIndices = true,
                                        const std::optional<RangeSpec>& ranges = std::nullopt) = 0;

            /**
             * @brief
             @rst
             Reorders existing entries in place. ``orderMap`` maps an old index -> new index for
             a subset (or all) of the current entries; every entry not mentioned keeps its
             relative order and fills whatever slots are left over -- see
             :cpp:func:`BaseOrderedMultiMap::reorder`'s doc comment for the full index/conflict
             semantics (identical here; see this file's own note on :cpp:func:`insertAllAt` for
             why this isn't a ``@copydoc`` of that comment directly).
             @endrst
             *
             * @param orderMap The old index -> new index entries to apply, in tie-breaking order
             * @param ranges
             @rst
             If provided, an ``orderMap`` entry only takes effect when its old index falls within
             ``ranges``; otherwise it's ignored entirely, and the old position it would have
             pinned is treated as unmentioned (floating) instead.
             @endrst
             */
            virtual void reorder(const std::vector<std::pair<long long, long long>>& orderMap,
                                  const std::optional<RangeSpec>& ranges = std::nullopt) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::removeAt
             */
            virtual bool removeAt(size_t pos, const std::optional<RangeSpec>& ranges = std::nullopt) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::removeKey
             */
            virtual size_t removeKey(const K& key,
                                      const std::optional<RangeSpec>& ranges = std::nullopt,
                                      const std::optional<RemoveKeyCheck>& check = std::nullopt) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::remapKeys
             */
            virtual void remapKeys(const std::vector<std::pair<K, KeyRemapValue>>& keyRemap,
                                    const std::optional<RangeSpec>& ranges = std::nullopt) = 0;

            /**
             * @brief
             @rst
             Bulk-updates values by key. ``newVals`` maps a key -> a :cpp:type:`ReplaceSpec`,
             which is one of: a ``V`` (sets every entry with this key to this value), a
             ``std::vector<V>`` (updates entries positionally, true left-to-right order), or a
             ``std::pair<V, Predicate>`` (replaces the value with the pair's ``V`` wherever the
             predicate returns ``true`` for the old value) -- see this file's own note on
             :cpp:func:`insertAllAt` for why this isn't a ``@copydoc`` of
             :cpp:func:`BaseOrderedMultiMap::replaceVals`'s comment directly, despite being
             identical in behavior.
             @endrst
             *
             * @param newVals The key -> replace spec entries to apply, in append-order for keys that don't exist yet
             * @param addNew
             @rst
             What to do when a key in ``newVals`` doesn't currently exist. If ``true`` (the
             default), it's added, appended at the end (``V`` -> one entry; ``vector<V>`` -> one
             entry per element, in order; ``pair<V,Predicate>`` -> one entry with just the ``V``,
             predicate ignored since there's nothing existing to test it against). If ``false``,
             the key is skipped entirely; no error.
             @endrst
             * @param ranges
             @rst
             If provided, gates whether an existing entry's value actually gets replaced, on top
             of whatever the spec itself already decides -- both must hold. Not consulted for
             ``addNew``: a brand-new entry has no existing position to filter by, so it's always
             appended regardless. For the ``vector<V>`` case, ``ranges`` gates whether an
             already-paired (i-th entry, ``list[i]``) update fires -- it does not reindex to skip
             ineligible entries and shift ``list[i]`` onto the next eligible one.
             @endrst
             */
            virtual void replaceVals(const std::vector<std::pair<K, ReplaceSpec>>& newVals, bool addNew = true,
                                      const std::optional<RangeSpec>& ranges = std::nullopt) = 0;

            /**
             * @copydoc BaseOrderedMultiMap::contains
             */
            virtual bool contains(const K& key) const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::containsKey
             */
            virtual bool containsKey(const K& key) const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::count
             */
            virtual size_t count(const K& key) const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::size
             */
            virtual size_t size() const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::length
             */
            virtual size_t length() const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::keySize
             */
            virtual size_t keySize() const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::empty
             */
            virtual bool empty() const = 0;

            /**
             * @brief
             @rst
             Retrieves all values currently stored under a key
             @endrst
             *
             * @param key The key to look up
             * @param ordered
             @rst
             If ``true`` (the default), returned in true left-to-right positional order. If
             ``false``, returned in whatever order they were added to this key.
             @endrst
             * @param ranges
             @rst
             If provided, only occurrences whose true positional index (same convention as
             :cpp:func:`getByInd`) falls within ``ranges`` are included. If omitted (the
             default), every occurrence is included.
             @endrst
             *
             * @return The values for this key, in the requested order
             */
            virtual std::vector<V> getAll(const K& key, bool ordered = true, const std::optional<RangeSpec>& ranges = std::nullopt) const = 0;

            /**
             * @brief
             @rst
             Same as :cpp:func:`getAll`, except each value is paired with its true positional
             index
             @endrst
             *
             * @param key The key to look up
             * @param ordered
             @rst
             If ``true`` (the default), returned in true left-to-right positional order. If
             ``false``, returned in whatever order they were added to this key.
             @endrst
             * @param ranges
             @rst
             If provided, only occurrences whose true positional index (same convention as
             :cpp:func:`getByInd`) falls within ``ranges`` are included. If omitted (the
             default), every occurrence is included.
             @endrst
             *
             * @return The ``(index, value)`` pairs for this key, in the requested order
             */
            virtual std::vector<std::pair<long long, V>> getAllWithInds(const K& key, bool ordered = true, const std::optional<RangeSpec>& ranges = std::nullopt) const = 0;

            /**
             * @brief
             @rst
             Retrieves every distinct key currently in the map
             @endrst
             *
             * @return
             @rst
             Every distinct key. A plain ``std::vector`` rather than a real set (unlike
             :cpp:func:`BaseOrderedMultiMap::getKeys`) -- this interface, unlike a concrete
             :cpp:class:`BaseOrderedMultiMap`-derived type, has no ``KeyHash``/``KeyEqual`` of
             its own to build a hash-based set with, for an arbitrary ``K``.
             @endrst
             */
            virtual std::vector<K> getKeys() const = 0;

            /**
             * @copybrief BaseOrderedMultiMap::getByInd(long long) const
             *
             @rst
             Returned by value rather than by reference (unlike the `CRTP`_ version) -- a
             virtual interface can't safely hand back a reference into storage of unknown
             concrete type.
             @endrst
             *
             * @param index @copybrief BaseOrderedMultiMap::getByInd(long long) const
             *
             * @return The (key, value) pair at that position
             *
             * @throw std::out_of_range Thrown when ``index`` is out of range
             */
            virtual std::pair<K, V> getByInd(long long index) const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::getByInd(long long, bool) const
             */
            virtual std::pair<long long, V> getByIndWithOccurrence(long long index) const = 0;

            /**
             * @copydoc BaseOrderedMultiMap::setValByInd
             */
            virtual void setValByInd(long long index, const V& value) = 0;

            /**
             * @brief Retrieves a copy of the full ordered sequence
             *
             * @return The full ordered sequence of (key, value) pairs
             */
            virtual std::vector<std::pair<K, V>> entries() const = 0;

            /**
             * @brief
             @rst
             Retrieves every entry, in true positional order, each paired with its occurrence
             and order index -- the virtual-interface equivalent of iterating a `CRTP`_-concrete
             instance directly via :cpp:class:`BaseOrderedMultiMap::Iterator`
             @endrst
             *
             * @return Every entry, as owned :cpp:class:`Item` values
             */
            virtual std::vector<Item> items() const = 0;

            /**
             * @copybrief BaseOrderedMultiMap::computeSplitGroups
             *
             * @param inds The indices at which to split, in tie-breaking order
             * @param includeSplitKVP @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param includeEmptyParts @copybrief BaseOrderedMultiMap::computeSplitGroups
             * @param sortIndices @copybrief BaseOrderedMultiMap::computeSplitGroups
             *
             * @return The resulting parts, left to right -- each a fresh, independent instance of the same concrete implementation as ``this``
             *
             * @throw std::out_of_range Thrown when an index in ``inds`` is out of range
             */
            virtual std::vector<std::unique_ptr<IOrderedMultiMap<K, V>>> splitByInds(
                const std::vector<long long>& inds,
                bool includeSplitKVP = true,
                bool includeEmptyParts = false,
                bool sortIndices = true) const = 0;

            /**
             * @brief
             @rst
             Creates a deep copy of this instance -- the "virtual copy constructor" idiom,
             needed since a ``std::unique_ptr<IOrderedMultiMap<K, V>>`` can't be copy-constructed
             the ordinary way (the pointed-to type is abstract)
             @endrst
             *
             * @return A new instance of the same concrete implementation as ``this``, holding an independent deep copy of its data
             */
            virtual std::unique_ptr<IOrderedMultiMap<K, V>> clone() const = 0;
    };

    /**
     * @brief
     @rst
     Appends every ``(key, value)`` pair to any :cpp:class:`IOrderedMultiMap` implementation, in
     order -- a small example of code written once against the interface, working identically
     whether ``target`` is `CRTP`_-backed (via :cpp:class:`OrderedMultiMapAdapter`) or a
     user-supplied implementation, including one implemented from `Python`_.
     @endrst
     *
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     *
     * @param target The map to append to
     * @param items The key-value pairs to append, in order
     */
    template <typename K, typename V>
    void appendAll(IOrderedMultiMap<K, V>& target, const std::vector<std::pair<K, V>>& items) {
        for (const auto& item : items) {
            target.insert(item.first, item.second);
        }
    }

}

#endif
