#ifndef AGRemapCore_BaseOrderedMultiMap_H
#define AGRemapCore_BaseOrderedMultiMap_H

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A single rule inside a :cpp:func:`remapKeys() <BaseOrderedMultiMap::remapKeys>` remap
     list, expressing a *conditional* and/or *repositioned* rename (as opposed to a plain
     key, which always fires and stays in place).
     @endrst
     *
     * @tparam K The type of the keys stored in the owning ordered multimap
     * @tparam V The type of the values stored in the owning ordered multimap
     */
    template <typename K, typename V>
    class RemappedKeyData {
        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define CheckPredicate std::function<bool(const K&, const V&)>
            #else
            /**
             * @brief
             @rst
             `(oldKey, oldValue) -> should this rule fire for this occurrence?`_
             @endrst
             */
            using CheckPredicate = std::function<bool(const K&, const V&)>;
            #endif

            /**
             * @brief Constructs a remap rule
             *
             * @param key
             @rst
             The new key to remap matching occurrences to
             @endrst
             * @param check
             @rst
             An optional predicate over ``(oldKey, oldValue)``. If omitted, this rule always
             fires. If present, it fires only when the predicate returns ``true`` for a given
             occurrence.
             @endrst
             * @param toInd
             @rst
             An optional target index. If present, every entry this rule produces (across all
             occurrences it fires for) is moved, as a group, to this index once the remap is
             complete -- using the exact same index semantics, conflict resolution, and
             front/back overflow-clustering rules as :cpp:func:`reorder() <BaseOrderedMultiMap::reorder>`'s
             target index. If omitted, produced entries stay wherever the remap naturally
             places them (in place of the occurrence that produced them).
             @endrst
             */
            explicit RemappedKeyData(K key,
                                      std::optional<CheckPredicate> check = std::nullopt,
                                      std::optional<long long> toInd = std::nullopt);

            /**
             * @brief Retrieves the new key to remap matching occurrences to
             *
             * @return The new key
             */
            const K& key() const;

            /**
             * @brief Retrieves the optional firing predicate
             *
             * @return The predicate, or an empty optional if this rule always fires
             */
            const std::optional<CheckPredicate>& check() const;

            /**
             * @brief Retrieves the optional repositioning target index
             *
             * @return The target index, or an empty optional if produced entries should stay in place
             */
            const std::optional<long long>& toInd() const;

        private:
            K key_;
            std::optional<CheckPredicate> check_;
            std::optional<long long> toInd_;
    };

    /**
     * @brief
     @rst
     A remap list: a sequence of remap rules for one old key, each either a plain new key
     (always fires, in place) or a :cpp:class:`RemappedKeyData` (conditional and/or
     repositioned). Shared by :cpp:class:`KeyRemapData` and
     :cpp:func:`BaseOrderedMultiMap::remapKeys` so both refer to the same type.
     @endrst
     *
     * @tparam K The type of the keys stored in the owning ordered multimap
     * @tparam V The type of the values stored in the owning ordered multimap
     */
    template <typename K, typename V>
    using RemapList = std::vector<std::variant<K, RemappedKeyData<K, V>>>;

    /**
     * @brief
     @rst
     A :cpp:func:`remapKeys() <BaseOrderedMultiMap::remapKeys>` ``keyRemap`` value (alongside
     a bare :cpp:type:`RemapList`) that additionally controls what happens to an occurrence
     when none of its rules fire.
     @endrst
     *
     * @tparam K The type of the keys stored in the owning ordered multimap
     * @tparam V The type of the values stored in the owning ordered multimap
     */
    template <typename K, typename V>
    class KeyRemapData {
        public:
            /**
             * @brief Constructs a keyed remap configuration
             *
             * @param remappedKeys
             @rst
             The remap rules for this key -- identical in meaning to passing a bare
             :cpp:type:`RemapList` directly.
             @endrst
             * @param keepKeyWithoutRemap
             @rst
             If ``true``, an occurrence for which zero rules fired (an empty list, or every
             rule's ``check()`` false) retains its original ``(key, value)`` pair instead of
             being removed. Evaluated per occurrence. Defaults to ``false``, matching a bare
             :cpp:type:`RemapList`'s behavior: zero firings means removal.
             @endrst
             */
            explicit KeyRemapData(RemapList<K, V> remappedKeys, bool keepKeyWithoutRemap = false);

            /**
             * @brief Retrieves the remap rules for this key
             *
             * @return The remap rules
             */
            const RemapList<K, V>& remappedKeys() const;

            /**
             * @brief Retrieves whether a non-firing occurrence retains its original pair
             *
             * @return Whether to keep a non-firing occurrence's original key-value pair
             */
            bool keepKeyWithoutRemap() const;

        private:
            RemapList<K, V> remappedKeys_;
            bool keepKeyWithoutRemap_;
    };

    /**
     * @brief
     @rst
     The `CRTP`_ (Curiously Recurring Template Pattern) base class implementing the entire
     ordered-multimap API in terms of a small set of "raw" primitives that each concrete
     backing structure provides. This preserves insertion/positional order, allows duplicate
     keys, and gives both fast key-based access and fast positional access, regardless of
     which concrete backing structure (e.g. a plain linked list, or a sqrt-decomposed block
     structure) is actually used underneath. :raw-html:`<br />` :raw-html:`<br />`

     ``Derived`` supplies a small set of "raw" primitive operations as ordinary member
     functions; every method here is implemented purely in terms of those primitives, then
     called via ``static_cast<Derived&>(*this)``. Unlike a classic virtual-dispatch base, the
     compiler inlines through that cast just like it would inline a direct call, so there is
     no vtable and no barrier to cross-function optimization at the primitive boundary. :raw-html:`<br />` :raw-html:`<br />`

     ``Handle`` and ``HandleHash`` are explicit template parameters here rather than nested
     ``typename Derived::Handle``/``typename Derived::HandleHash`` aliases: by the time this
     class template is instantiated (as part of processing ``Derived``'s own definition,
     since ``Derived`` inherits from it), ``Derived`` is still an incomplete type, so none of
     its members are visible yet -- and unlike a reference used only inside a function body
     (which is resolved lazily, in complete-class context), any type named directly in a
     function's *signature* is resolved eagerly during class template instantiation. Making
     ``Handle``/``HandleHash`` their own template parameters sidesteps this entirely. :raw-html:`<br />` :raw-html:`<br />`

     For the same reason, the per-entry storage (key, value, position label, backlink into
     its key's bucket) is never defined in this class -- each ``Derived`` owns its own
     entry-shaped storage entirely, and exposes it to this class only through the narrow
     accessor primitives listed below.

     .. rubric:: Primitive interface Derived must provide

     .. code-block:: cpp

        using Handle = ...; // derived-specific, copyable, equality-comparable
        struct HandleHash { size_t operator()(Handle) const; };

        size_t rawSize() const;
        Handle rawBegin() const;                             // first entry, or rawEnd() if empty
        Handle rawEnd() const;                                // one-past-last sentinel
        Handle rawNext(Handle h) const;
        Handle rawPrev(Handle h) const;
        Handle rawAtIndex(size_t idx) const;                   // 0 <= idx < rawSize()
        Handle rawInsertBefore(Handle pos, const K&, const V&); // returns handle to the new entry
        void   rawErase(Handle h);
        void   rawClear();
        void   rawRelinkInOrder(const std::vector<Handle>& order);

        const K& rawKey(Handle h) const;
        const V& rawValue(Handle h) const;
        void     rawSetValue(Handle h, const V& value);
        double   rawPos(Handle h) const;
        void     rawSetPos(Handle h, double p);
        typename std::list<Handle>::iterator rawSelf(Handle h) const;
        void     rawSetSelf(Handle h, typename std::list<Handle>::iterator self);

     ``rawAtIndex`` is the one primitive whose complexity is actually meant to differ between
     backing structures (:math:`O(\\min(idx, n-idx))` for a plain list, :math:`O(\\sqrt{n})` for
     block-decomposed storage, :math:`O(\\log n)` for a balanced tree, etc.) -- every other
     primitive is :math:`O(1)` (or amortized :math:`O(1)`/:math:`O(\\sqrt{n})` for insert/erase)
     regardless of backing structure, by design.
     @endrst
     *
     * @tparam Derived The concrete backing-structure class inheriting from this base (CRTP)
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     * @tparam Handle An opaque, copyable, equality-comparable handle type identifying a single stored entry, provided by ``Derived``
     * @tparam HandleHash A hasher for ``Handle``, provided by ``Derived``
     * @tparam KeyHash A hasher for ``K``, used by the internal key -> bucket index. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, used by the internal key -> bucket index. Defaults to ``std::equal_to<K>``
     */
    template <typename Derived, typename K, typename V, typename Handle, typename HandleHash,
              typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseOrderedMultiMap {
        protected:
            /**
             * @brief Casts this object to its concrete derived type
             *
             * @return This object, viewed as ``Derived&``
             */
            Derived& self();

            /**
             * @copydoc self()
             */
            const Derived& self() const;

            /**
             * @brief A list of handles, used as the per-key bucket's call-order storage
             */
            using HandleList = std::list<Handle>;

            /**
             * @brief
             @rst
             Per-key bookkeeping: every entry sharing one key, plus a lazily-rebuilt cache of
             those handles sorted by true position.
             @endrst
             */
            struct KeyBucket {
                /**
                 * @brief Every entry with this key, in the order they were added to this key (call order)
                 */
                HandleList items;

                /**
                 * @brief A lazily-rebuilt cache of #items sorted by true position
                 */
                mutable std::vector<Handle> sortedCache;

                /**
                 * @brief Whether #sortedCache must be rebuilt before its next use
                 */
                mutable bool dirty = true;
            };

            /**
             * @brief The key -> bucket index, giving fast key-based access
             */
            std::unordered_map<K, KeyBucket, KeyHash, KeyEqual> index_;

            /**
             * @brief A lazily-rebuilt cache of #buildIndexMap's own result
             */
            mutable std::unordered_map<Handle, long long, HandleHash> indexMapCache_;

            /**
             * @brief Whether #indexMapCache_ must be rebuilt before its next use
             */
            mutable bool indexMapDirty_ = true;

        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define RemoveKeyCheck std::function<bool(long long, const V&)>
            #define Predicate std::function<bool(const V&)>
            #define ReplaceSpec std::variant<V, std::vector<V>, std::pair<V, Predicate>>
            #define RemapListItem std::variant<K, RemappedKeyData<K, V>>
            #define KeyRemapList RemapList<K, V>
            #define KeyRemapValue std::variant<KeyRemapList, KeyRemapData<K, V>>
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
             positionally, or a ``(value, predicate)`` pair to conditionally assign
             @endrst
             */
            using ReplaceSpec = std::variant<V, std::vector<V>, std::pair<V, Predicate>>;

            /**
             * @brief
             @rst
             Used by :cpp:func:`remapKeys`: a single element of a remap list -- either a plain
             new key (always fires, in place), or a :cpp:class:`RemappedKeyData` for a
             conditional and/or repositioned rename
             @endrst
             */
            using RemapListItem = std::variant<K, RemappedKeyData<K, V>>;

            /**
             * @brief
             @rst
             Used by :cpp:func:`remapKeys`: a list of :cpp:type:`RemapListItem`\\ s for one old key
             @endrst
             */
            using KeyRemapList = RemapList<K, V>;

            /**
             * @brief
             @rst
             Used by :cpp:func:`remapKeys`: a ``keyRemap`` value -- either a bare
             :cpp:type:`KeyRemapList` (equivalent to a ``KeyRemapData`` constructed with
             ``keepKeyWithoutRemap`` left at its default, ``false``), or a
             :cpp:class:`KeyRemapData` for control over the zero-rules-fired case
             @endrst
             */
            using KeyRemapValue = std::variant<KeyRemapList, KeyRemapData<K, V>>;
            #endif

            /**
             * @brief Appends a key-value pair to the end
             *
             * @param key The key of the pair to insert
             * @param value The value of the pair to insert
             */
            void insert(const K& key, const V& value);

            /**
             * @brief Inserts a key-value pair at the beginning
             *
             * @param key The key of the pair to insert
             * @param value The value of the pair to insert
             */
            void insertStart(const K& key, const V& value);

            /**
             * @brief
             @rst
             Inserts a key-value pair so it ends up at position ``index`` (0-based)
             @endrst
             *
             * @param index
             @rst
             The target position. Supports Python-style negative indices: ``-1`` means "insert
             at the end", ``-(size()+1)`` means "insert at the front" (i.e. counting backwards
             over the ``size()+1`` valid insertion slots ``0..size()``). Out-of-range indices
             are clamped rather than rejected:

             * ``index >  size()``       is treated as ``size()``        (append)
             * ``index < -(size()+1)``   is treated as ``-(size()+1)``   (front)
             @endrst
             * @param key The key of the pair to insert
             * @param value The value of the pair to insert
             */
            void insertAt(long long index, const K& key, const V& value);

            /**
             * @brief
             @rst
             Appends a batch of key-value pairs to the end, in the order given. Every one of
             these is a true-tail append, so no key's sorted cache is ever invalidated by this
             call.
             @endrst
             *
             * @param items The key-value pairs to append, in order
             */
            void insertAllEnd(const std::vector<std::pair<K, V>>& items);

            /**
             * @brief
             @rst
             Inserts a batch of key-value pairs at the beginning, in the order given --
             ``items[0]`` ends up first, ``items[1]`` right after it, and so on, all before
             whatever was originally at the front.
             @endrst
             *
             * @param items The key-value pairs to insert, in order
             */
            void insertAllStart(const std::vector<std::pair<K, V>>& items);

            /**
             * @brief
             @rst
             Bulk indexed insert: inserts many key-value pairs at their own target indices in
             a single pass. Index semantics match :cpp:func:`insertAt` (Python-style negative
             indices, clamping), but with "original position" (numpy-style) semantics: each
             index refers to a position in the sequence as it was *before* this call, not a
             position in the growing result.
             @endrst
             *
             * @param items
             @rst
             Maps an index to insert at -> the key-value pair to insert there
             @endrst
             * @param sortIndices
             @rst
             If ``true`` (the default), ``items`` is stable-sorted by normalized index first,
             so the single :math:`O(n+m)` cursor pass afterward can advance strictly forward.
             If you already know ``items`` iterates in ascending normalized-index order, pass
             ``false`` to skip that sort and drop the cost to :math:`O(n+m)` -- **this
             precondition is unchecked**, and violating it produces a silently wrong (not
             crashing) result.
             @endrst
             * @param ranges
             @rst
             If provided, an entry is only inserted when its normalized target index falls
             within ``ranges``; filtered entries are dropped before sorting/the cursor pass,
             so they never affect tie-breaking among the entries that do get inserted.
             @endrst
             *
             * @return How many entries were actually inserted
             */
            size_t insertAllAt(const tsl::ordered_map<long long, std::pair<K, V>>& items,
                                bool sortIndices = true,
                                const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief
             @rst
             Same as the ``tsl::ordered_map``-keyed overload above, for callers with a plain
             ``std::unordered_map`` instead. This overload's iteration order (and therefore
             tie-breaking between conflicting target indices) is unspecified, since
             ``std::unordered_map``'s own iteration order is unspecified. There is no
             ``sortIndices`` flag here: skipping the sort would depend on knowing that
             iteration order, which isn't available.
             @endrst
             *
             * @param items Same shape/semantics as the ``tsl::ordered_map``-keyed overload's ``items``
             * @param ranges Same meaning/default as the ``tsl::ordered_map``-keyed overload's ``ranges``
             *
             * @return How many entries were actually inserted
             */
            size_t insertAllAt(const std::unordered_map<long long, std::pair<K, V>>& items,
                                const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief
             @rst
             Reorders existing entries in place. ``orderMap`` maps an old index -> new index
             for a subset (or all) of the current entries; every entry not mentioned keeps its
             relative order and fills whatever slots are left over. :raw-html:`<br />` :raw-html:`<br />`

             **Old-index (key) semantics:** must be in ``[-size(), size()-1]`` -- anything
             outside that throws. This range has two representations of every position (e.g.
             for ``size()==5``, key ``0`` and key ``-5`` both mean "the first element"), so two
             different raw keys can refer to the same physical entry -- see conflict rule 1
             below. :raw-html:`<br />` :raw-html:`<br />`

             **New-index (value) semantics:** also Python-style, but out-of-range values are
             bucketed rather than rejected:

             * a value in ``[-size(), size()-1]`` is that entry's literal target index in the
               result (Python wrap: negative -> ``value + size()``)
             * a value ``>= size()`` goes in a trailing cluster at the very end
             * a value ``< -size()`` goes in a leading cluster at the very front
             * within a cluster, a smaller raw value sorts earlier in the cluster

             **Conflicts** (for two distinct entries K1, K2 of ``orderMap``):

             #. Same physical old entry: whichever comes first in ``orderMap``'s iteration
                order wins; the later one is ignored entirely.
             #. Different old entries but the same effective new-index target (including ties
                within the same overflow cluster): broken by ``orderMap``'s iteration order.
             #. If both key and value conflict, rule 1 applies.
             @endrst
             *
             * @param orderMap The old index -> new index mapping to apply
             * @param ranges
             @rst
             If provided, an ``orderMap`` entry only takes effect when its old index falls
             within ``ranges``; otherwise it's ignored entirely, and the old position it would
             have pinned is treated as unmentioned (floating) instead.
             @endrst
             */
            void reorder(const tsl::ordered_map<long long, long long>& orderMap,
                         const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @copydoc reorder(const tsl::ordered_map<long long, long long>&, const std::optional<Ranges<long long>>&)
             *
             @rst
             This overload's iteration order (and therefore tie-breaking for conflict rules 1
             and 2) is unspecified, since ``std::unordered_map``'s own iteration order is
             unspecified.
             @endrst
             */
            void reorder(const std::unordered_map<long long, long long>& orderMap,
                         const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief Removes the entry currently at position `pos`
             *
             * @param pos The position of the entry to remove
             * @param ranges
             @rst
             If provided, removal only proceeds when ``pos`` falls within ``ranges``;
             otherwise this call is a no-op, same as an out-of-bounds ``pos``.
             @endrst
             *
             * @return Whether an entry was actually removed
             */
            bool removeAt(size_t pos, const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief
             @rst
             Removes every entry with this key, subject to two independent, optional filters
             -- both must hold (where provided) for a given occurrence to actually be removed.
             With neither filter provided, this is unconditional removal of every entry with
             this key.
             @endrst
             *
             * @param key The key whose entries to remove
             * @param ranges
             @rst
             If provided, the occurrence's true positional index (same convention as
             :cpp:func:`getByInd`) must fall within ``ranges``.
             @endrst
             * @param check
             @rst
             If provided, ``check(index, value)`` must return ``true``, given that
             occurrence's true positional index and value.
             @endrst
             *
             * @return How many entries were actually removed
             */
            size_t removeKey(const K& key,
                              const std::optional<Ranges<long long>>& ranges = std::nullopt,
                              const std::optional<RemoveKeyCheck>& check = std::nullopt);

            /**
             * @brief
             @rst
             Bulk-renames keys. ``keyRemap`` maps an old key -> either a bare
             :cpp:type:`KeyRemapList`, or a :cpp:class:`KeyRemapData` (a remap list plus a
             ``keepKeyWithoutRemap`` flag). :raw-html:`<br />` :raw-html:`<br />`

             For every existing entry, walked in true positional order: if its key is not a
             key in ``keyRemap``, it's left completely unchanged. Otherwise, mapped to a list
             of N rules, each rule is evaluated independently against this occurrence:

             * a plain key ``K`` always fires
             * a :cpp:class:`RemappedKeyData` fires if it has no ``check()``, or if
               ``check()(oldKey, oldValue)`` is ``true``

             Every rule that fires produces one new entry (that rule's key, this occurrence's
             original value). A rule with no ``toInd()`` places its entry in the same spot as
             before. A rule with ``toInd()`` instead moves its entry (as part of a group with
             every other entry sharing that same ``toInd()`` across every occurrence) to that
             target index, using :cpp:func:`reorder`'s exact index semantics, overflow
             clustering, and tie-break rules. :raw-html:`<br />` :raw-html:`<br />`

             If zero rules fire for a given occurrence: with a bare
             :cpp:type:`KeyRemapList`, or ``keepKeyWithoutRemap = false`` (the default), that
             occurrence is removed entirely. With ``keepKeyWithoutRemap = true``, it retains
             its original ``(key, value)`` pair instead. This is judged per occurrence,
             independently. :raw-html:`<br />` :raw-html:`<br />`

             Old keys mentioned in ``keyRemap`` that don't actually exist right now are simply
             never triggered -- no error, nothing happens. This is a single pass over the
             original entries: newly-created entries are never looked up in ``keyRemap``
             again, so there's no cascading/recursive re-application even when a new name also
             happens to be a source key elsewhere in ``keyRemap``.
             @endrst
             *
             * @param keyRemap The old key -> remap rules mapping to apply
             * @param ranges
             @rst
             If provided, an occurrence outside ``ranges`` is treated exactly as if its key
             were never mentioned in ``keyRemap`` at all -- a pure pass-through, not subject to
             ``keepKeyWithoutRemap`` or anything else.
             @endrst
             */
            void remapKeys(const std::unordered_map<K, KeyRemapValue, KeyHash, KeyEqual>& keyRemap,
                            const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief
             @rst
             Bulk-updates values by key. ``newVals`` maps a key -> a :cpp:type:`ReplaceSpec`,
             which is one of:

             * ``V`` sets every entry with this key to this value
             * ``std::vector<V>`` updates entries with this key positionally (true
               left-to-right order): the i-th existing entry gets ``newVals[key][i]``, for
               ``i`` in ``[0, min(count(key), list.size()))``. A list shorter than
               ``count(key)`` leaves the remaining entries untouched; a list longer than
               ``count(key)`` just has its extra values unused (not appended as new entries --
               ``addNew`` below is strictly about keys absent from the map)
             * ``std::pair<V, Predicate>`` replaces the value with the pair's ``V`` wherever
               ``predicate(oldValue)`` is ``true``, for entries with this key
             @endrst
             *
             * @param newVals The key -> replace spec mapping to apply
             * @param addNew
             @rst
             What to do when a key in ``newVals`` doesn't currently exist. If ``true`` (the
             default), it's added, appended at the end (``V`` -> one entry; ``vector<V>`` -> one
             entry per element, in order; ``pair<V,Predicate>`` -> one entry with just the
             ``V``, predicate ignored since there's nothing existing to test it against). If
             ``false``, the key is skipped entirely; no error.
             @endrst
             * @param ranges
             @rst
             If provided, gates whether an existing entry's value actually gets replaced, on
             top of whatever the spec itself already decides -- both must hold. Not consulted
             for ``addNew``: a brand-new entry has no existing position to filter by, so it's
             always appended regardless. For the ``vector<V>`` case, ``ranges`` gates whether an
             already-paired (i-th entry, ``list[i]``) update fires -- it does not reindex to
             skip ineligible entries and shift ``list[i]`` onto the next eligible one.
             @endrst
             */
            void replaceVals(const tsl::ordered_map<K, ReplaceSpec, KeyHash, KeyEqual>& newVals, bool addNew = true,
                              const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @copydoc replaceVals(const tsl::ordered_map<K, std::variant<V, std::vector<V>, std::pair<V, std::function<bool(const V&)>>>, KeyHash, KeyEqual>&, bool, const std::optional<Ranges<long long>>&)
             */
            void replaceVals(const std::unordered_map<K, ReplaceSpec, KeyHash, KeyEqual>& newVals, bool addNew = true,
                              const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief Checks whether a key exists
             *
             * @param key The key to check for
             *
             * @return Whether the key exists
             */
            bool contains(const K& key) const;

            /**
             * @copydoc contains(const K&) const
             */
            bool containsKey(const K& key) const;

            /**
             * @brief Retrieves how many entries share a given key
             *
             * @param key The key to count
             *
             * @return The number of entries with this key
             */
            size_t count(const K& key) const;

            /**
             * @brief Retrieves the number of entries in the map
             *
             * @return The number of entries
             */
            size_t size() const;

            /**
             * @copydoc size() const
             */
            size_t length() const;

            /**
             * @brief Retrieves the number of distinct keys in the map
             *
             * @return The number of distinct keys -- ``index_``'s own size
             */
            size_t keySize() const;

            /**
             * @brief Checks whether the map is empty
             *
             * @return Whether the map is empty
             */
            bool empty() const;

            /**
             * @brief Retrieves all values currently stored under a key
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
            std::vector<V> getAll(const K& key, bool ordered = true, const std::optional<Ranges<long long>>& ranges = std::nullopt) const;

            /**
             * @brief
             @rst
             Same as the overload just above, except each value is paired with its true
             positional index. A separate overload rather than a 4th default parameter on the
             one above, since C++ can't vary a function's return type based on a runtime bool.
             @endrst
             *
             * @param key The key to look up
             * @param ordered
             @rst
             If ``true`` (the default), returned in true left-to-right positional order. If
             ``false``, returned in whatever order they were added to this key.
             @endrst
             * @param withInds
             @rst
             Selects this overload; its actual runtime value does not change the behavior.
             @endrst
             * @param ranges
             @rst
             If provided, only occurrences whose true positional index (same convention as
             :cpp:func:`getByInd`) falls within ``ranges`` are included. If omitted (the
             default), every occurrence is included.
             @endrst
             *
             * @return The (order index, value) pairs for this key, in the requested order
             */
            std::vector<std::pair<long long, V>> getAll(const K& key, bool ordered, bool withInds,
                                                          const std::optional<Ranges<long long>>& ranges = std::nullopt) const;

            /**
             * @brief Retrieves every distinct key currently in the map
             *
             * @return Every distinct key, as a set (unordered -- ``index_``'s own keys, copied out directly)
             */
            std::unordered_set<K, KeyHash, KeyEqual> getKeys() const;

            /**
             * @brief Retrieves the entry at a true positional index
             *
             * @param index
             @rst
             The position to retrieve. Python-style negative indices are supported: valid
             range is ``[-size(), size()-1]`` (``-1`` = last entry, ``-size()`` = first).
             @endrst
             *
             * @return The (key, value) pair at that position
             *
             * @throw std::out_of_range Thrown when ``index`` is out of range
             */
            std::pair<const K&, const V&> getByInd(long long index) const;

            /**
             * @copydoc getByInd(long long) const
             *
             @rst
             This overload additionally pairs the entry's value with its occurrence index
             (how many times this same key already appeared earlier in the sequence,
             0-based) instead of its key. A separate overload rather than a 2nd default
             parameter on the one above, for the same reason as :cpp:func:`getAll`'s
             ``withInds`` overload.
             @endrst
             *
             * @param withInds
             @rst
             Selects this overload; its actual runtime value does not change the behavior.
             @endrst
             *
             * @return The (occurrence index, value) pair at that position
             *
             * @throw std::out_of_range Thrown when ``index`` is out of range
             */
            std::pair<long long, V> getByInd(long long index, bool withInds) const;

            /**
             * @brief Sets the value of the entry at a true positional index, leaving its key untouched
             *
             * @param index
             @rst
             The position to update. Python-style negative indices are supported: valid range
             is ``[-size(), size()-1]`` (``-1`` = last entry, ``-size()`` = first) -- same
             convention as :cpp:func:`getByInd`.
             @endrst
             * @param value The new value for that entry
             *
             * @throw std::out_of_range Thrown when ``index`` is out of range
             */
            void setValByInd(long long index, const V& value);

            /**
             * @brief
             @rst
             A forward iterator walking the whole map in true positional order, yielding an
             ``Item`` per entry: its key, its value, its occurrence index (how many times this
             same key has already been seen earlier in this iteration, 0-based), and its order
             index (its own 0-based position, same convention as :cpp:func:`getByInd`).
             ``Item`` is a plain aggregate, so structured bindings work directly on the
             dereferenced value. :raw-html:`<br />` :raw-html:`<br />`

             Copyable and safe for independent multi-pass use, but each copy carries its own
             per-key running-occurrence-count map, so copying is not :math:`O(1)` the way a
             plain index/pointer iterator's copy would be.
             @endrst
             */
            class Iterator {
                public:
                    /**
                     * @brief A single yielded element of an #Iterator
                     */
                    struct Item {
                        /**
                         * @brief This entry's key
                         */
                        const K& key;

                        /**
                         * @brief This entry's value
                         */
                        const V& value;

                        /**
                         * @brief How many times this same key has already been seen earlier in this iteration (0-based)
                         */
                        size_t occurrenceIndex;

                        /**
                         * @brief This entry's own 0-based position in the overall sequence
                         */
                        size_t orderIndex;
                    };

                    using iterator_category = std::forward_iterator_tag;
                    using value_type        = Item;
                    using difference_type   = std::ptrdiff_t;
                    using pointer           = void;
                    using reference         = Item;

                    /**
                     * @brief Dereferences this iterator
                     *
                     * @return The #Item at the current position
                     */
                    Item operator*() const;

                    /**
                     * @brief Advances this iterator, pre-increment
                     *
                     * @return This iterator, after advancing
                     */
                    Iterator& operator++();

                    /**
                     * @brief Advances this iterator, post-increment
                     *
                     * @return A copy of this iterator, before advancing
                     */
                    Iterator operator++(int);

                    /**
                     * @brief Checks two iterators for equality
                     *
                     * @param other The iterator to compare against
                     *
                     * @return Whether both iterators refer to the same position
                     */
                    bool operator==(const Iterator& other) const;

                    /**
                     * @copydoc operator==(const Iterator&) const
                     *
                     * @return Whether the iterators refer to different positions
                     */
                    bool operator!=(const Iterator& other) const;

                private:
                    friend class BaseOrderedMultiMap;
                    Iterator(const BaseOrderedMultiMap* owner, Handle h, size_t orderIndex);

                    const BaseOrderedMultiMap* owner_;
                    Handle h_;
                    size_t orderIndex_;
                    std::unordered_map<K, size_t, KeyHash, KeyEqual> seenCount_;
            };

            /**
             * @brief Retrieves an #Iterator to the first entry
             *
             * @return An #Iterator to the first entry, or to end() if the map is empty
             */
            Iterator begin() const;

            /**
             * @brief Retrieves the past-the-end #Iterator
             *
             * @return The past-the-end #Iterator
             */
            Iterator end() const;

            /**
             * @brief
             @rst
             Splits this map into several smaller maps at the given indices, preserving
             relative order both within each part and across parts (part *i* always comes
             from earlier in the original sequence than part *i+1*). :raw-html:`<br />` :raw-html:`<br />`

             ``inds`` uses the same index convention as :cpp:func:`getByInd`: Python-style
             negative indices over the existing ``size()`` elements. Duplicate raw indices
             that normalize to the same position collapse to one split point. Each index in
             ``inds`` becomes a boundary using standard slice semantics: everything before it
             goes in the earlier part, everything from it onward starts the next part. So for
             sorted, deduplicated split points :math:`i_1 < i_2 < ... < i_k`, the parts are
             :math:`[0,i_1), [i_1,i_2), ..., [i_k,n)` -- :math:`k+1` parts total, before
             filtering.
             @endrst
             *
             * @param inds The indices at which to split
             * @param includeSplitKVP
             @rst
             What happens to the entry at each split point. If ``true`` (the default), it
             starts the later part (the slice semantics above). If ``false``, it's dropped
             entirely, belonging to neither part.
             @endrst
             * @param includeEmptyParts
             @rst
             Whether empty parts (which can arise from a split point at index 0, at the very
             end, or two split points adjacent enough that nothing falls between them) are
             included in the result or silently dropped. Defaults to ``false``.
             @endrst
             * @param sortIndices
             @rst
             If ``true`` (the default), ``inds`` is normalized, deduplicated, and sorted
             ascending first. If you already know ``inds`` iterates in that exact order, pass
             ``false`` to skip that pass -- **this precondition is unchecked**, and violating
             it produces a silently wrong (not crashing) result.
             @endrst
             *
             * @return Each computed handle group, ready for a derived class to turn into actual new instances of itself
             *
             * @throw std::out_of_range Thrown when an index in ``inds`` is out of range
             */
            std::vector<std::vector<Handle>> computeSplitGroups(const std::vector<long long>& inds,
                                                                  bool includeSplitKVP,
                                                                  bool includeEmptyParts,
                                                                  bool sortIndices) const;

        protected:
            /**
             * @brief
             @rst
             Lets an :cpp:class:`Iterator` reach ``Derived::rawKey`` despite friendship not
             extending to a friend's nested classes
             @endrst
             *
             * @param h The handle to look up
             *
             * @return The key at that handle
             */
            const K& iterKey(Handle h) const;

            /**
             * @brief
             @rst
             Lets an :cpp:class:`Iterator` reach ``Derived::rawKey`` despite friendship not
             extending to a friend's nested classes
             @endrst
             *
             * @param h The handle to look up
             *
             * @return The value at that handle
             */
            const V& iterValue(Handle h) const;

            /**
             * @brief
             @rst
             Lets an :cpp:class:`Iterator` reach ``Derived::rawKey`` despite friendship not
             extending to a friend's nested classes
             @endrst
             *
             * @param h The handle to look up
             *
             * @return The handle immediately after ``h``
             */
            Handle iterNext(Handle h) const;

            /**
             * @brief
             @rst
             Shared implementation behind the "fully-indexed" constructors (a map of key ->
             list of ``(index, value)``). The index is treated as a sort key, not a strict
             absolute position: every ``(index, key, value)`` triple across every key is
             gathered, stable-sorted by index ascending, and inserted in that order. This is
             deliberately more forgiving than requiring a dense ``0..n-1`` permutation -- gaps
             and out-of-order values just determine relative order, and duplicate indices land
             consecutively (tie-broken by encounter order: list order within a key; the outer
             container's own iteration order across different keys).
             @endrst
             *
             * @tparam IndexedMap The type of the indexed map (e.g. ``std::unordered_map<K, std::vector<std::pair<long long, V>>>``)
             *
             * @param indexed The indexed data to build from
             */
            template <typename IndexedMap>
            void buildFromIndexed(const IndexedMap& indexed);

            /**
             * @brief
             @rst
             Normalizes a possibly-negative, possibly-out-of-range raw index onto a valid slot
             in ``[0, n]``:

             * an index in ``[0, n]`` is unchanged
             * an index ``> n`` is clamped to ``n`` (append)
             * an index in ``[-(n+1), -1]`` becomes ``index + n + 1`` (counts back from the
               end over the ``n+1`` slots)
             * an index ``< -(n+1)`` is clamped to ``0`` (front)
             @endrst
             *
             * @param index The raw index to normalize
             * @param n The current size to normalize against
             *
             * @return The normalized index
             */
            static size_t normalizeIndex(long long index, size_t n);

            /**
             * @brief Ensures a key bucket's sorted cache is valid, rebuilding it if dirty
             *
             * @param bucket The bucket to validate
             *
             * @return The bucket's cache, sorted by true position
             */
            const std::vector<Handle>& ensureSortedCache(const KeyBucket& bucket) const;

            /**
             * @brief
             @rst
             Computes every entry's true positional index, via one :math:`O(n)` walk. There is
             no backing structure that maintains a dense literal index per entry directly, so
             any method needing many entries' positions at once builds this once up front --
             lazily cached in ``indexMapCache_`` (mirroring :cpp:func:`ensureSortedCache`'s own
             per-bucket ``sortedCache``/``dirty`` pattern, just at the whole-map level instead of
             per key), so back-to-back calls (e.g. repeated :cpp:func:`getAll` with
             ``withInds=true``) between which nothing actually mutated this map only pay the
             :math:`O(n)` cost once. Any operation that could change a handle's true positional
             index (insert, erase, or a physical relink such as :cpp:func:`reorder`) sets
             ``indexMapDirty_``, forcing the next call to rebuild.
             @endrst
             *
             * @return A map from handle to its true positional index
             */
            const std::unordered_map<Handle, long long, HandleHash>& buildIndexMap() const;

            /**
             * @brief
             @rst
             Inserts ``(key, value)`` immediately before handle ``pos``, assigning it a
             position label strictly between its new neighbors' labels (relabelling
             everything first, if that pair of neighbors has no room left), and updating that
             key's bucket, sorted cache, and dirty flag.
             @endrst
             *
             * @param pos The handle to insert before
             * @param key The key of the pair to insert
             * @param value The value of the pair to insert
             *
             * @return The handle to the newly-inserted entry
             */
            Handle insertBefore(Handle pos, const K& key, const V& value);

            /**
             * @brief Shared implementation behind both insertAllAt() overloads
             *
             * @param raw
             @rst
             The (index, key-value pair) entries to insert, already in the calling
             container's natural iteration order
             @endrst
             * @param sortIndices @copybrief insertAllAt(const tsl::ordered_map<long long, std::pair<K, V>>&, bool, const std::optional<Ranges<long long>>&)
             * @param ranges @copybrief insertAllAt(const tsl::ordered_map<long long, std::pair<K, V>>&, bool, const std::optional<Ranges<long long>>&)
             *
             * @return How many entries were actually inserted
             */
            size_t insertAllAtImpl(const std::vector<std::pair<long long, std::pair<K, V>>>& raw,
                                    bool sortIndices,
                                    const std::optional<Ranges<long long>>& ranges);

            /**
             * @brief Shared implementation behind both reorder() overloads
             *
             * @param raw
             @rst
             The (old index, new index) entries to apply, already in the calling container's
             natural iteration order
             @endrst
             * @param ranges @copybrief reorder(const tsl::ordered_map<long long, long long>&, const std::optional<Ranges<long long>>&)
             */
            void reorderImpl(const std::vector<std::pair<long long, long long>>& raw,
                              const std::optional<Ranges<long long>>& ranges = std::nullopt);

            /**
             * @brief Shared implementation behind both replaceVals() overloads
             *
             * @param raw
             @rst
             The (key, replace spec) entries to apply, already in the calling container's
             natural iteration order (this is what determines the append order for keys that
             don't exist yet)
             @endrst
             * @param addNew @copybrief replaceVals(const tsl::ordered_map<K, std::variant<V, std::vector<V>, std::pair<V, std::function<bool(const V&)>>>, KeyHash, KeyEqual>&, bool, const std::optional<Ranges<long long>>&)
             * @param ranges @copybrief replaceVals(const tsl::ordered_map<K, std::variant<V, std::vector<V>, std::pair<V, std::function<bool(const V&)>>>, KeyHash, KeyEqual>&, bool, const std::optional<Ranges<long long>>&)
             */
            void replaceValsImpl(const std::vector<std::pair<K, ReplaceSpec>>& raw, bool addNew,
                                  const std::optional<Ranges<long long>>& ranges);

            /**
             * @brief Erases the entry at a handle, updating its key's bucket accordingly
             *
             * @param h The handle to erase
             */
            void eraseHandle(Handle h);

        private:
            /**
             * @brief Computes a fresh position label for an about-to-be-inserted entry
             *
             * @param pos The handle the new entry will be inserted before
             *
             * @return The new label, strictly between its neighbors' labels
             */
            double computeLabel(Handle pos);

            /**
             * @brief Reassigns fresh, evenly-spaced position labels to every entry
             */
            void renumberAll();

            /**
             * @brief
             @rst
             Resolves an insertion slot (0..size(), size()+1 possible slots) to a handle,
             correctly resolving ``pos == size()`` to the end sentinel rather than walking
             into a nonexistent entry
             @endrst
             *
             * @param pos The insertion slot to resolve
             *
             * @return The handle to insert before
             */
            Handle handleAtInsertionSlot(size_t pos) const;

            /**
             * @brief
             @rst
             Resolves a query index to a handle of an *existing* entry, using
             :cpp:func:`getByInd`'s Python-negative-index-and-throw-on-range convention
             @endrst
             *
             * @param index The index to resolve
             *
             * @return The handle at that index
             *
             * @throw std::out_of_range Thrown when ``index`` is out of range
             */
            Handle handleAtQueryIndex(long long index) const;
    };
}

#include "BaseOrderedMultiMap.tpp"

#endif