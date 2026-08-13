#ifndef AGRemapCore_IfContentPart_H
#define AGRemapCore_IfContentPart_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"
#include "AGRemapCore/tools/orderedMultiMap/IOrderedMultiMap.h"
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapAdapter.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IfTemplatePart`

     The content part of an `IfTemplate` -- the C++ port of ``IfContentPart.py``, holding the
     key-value pairs (e.g. a `.ini` section's registers) for one part of the template. :raw-html:`<br />` :raw-html:`<br />`

     Unlike the deprecated Python version (which hardcoded its own list-of-buckets storage),
     this class owns its data purely through a caller-supplied :cpp:class:`IOrderedMultiMap`
     implementation -- callers pick which concrete ordered-multimap backs a given
     :cpp:class:`IfContentPart` (:cpp:class:`OrderedMultiMap`/:cpp:class:`OrderedMultiMapSqrt`
     via :cpp:class:`OrderedMultiMapAdapter`, or any custom implementation of their own,
     including one implemented from `Python`_), and every method on this class is a thin,
     renamed delegation straight to that implementation. There is deliberately no independent
     storage or logic duplicated here: the semantics for every operation are exactly
     :cpp:class:`BaseOrderedMultiMap`'s documented rules, not the deprecated Python class's old
     ones -- only the *method names* below intentionally echo ``IfContentPart.py``'s naming
     (e.g. ``insertAllAt`` -> ``addKVPsByInds``), per that migration's explicit intent. :raw-html:`<br />` :raw-html:`<br />`

     Notable deliberate departures from the old Python class, worth knowing if migrating code
     from it:

     * ``get(key: Union[str, int], default=None)``, which conflated by-key and by-index lookup
       behind one overloaded signature, is split into the unambiguous
       :cpp:func:`getVals`/:cpp:func:`getByInd`/:cpp:func:`getByIndWithOccurrence` -- matching
       how :cpp:class:`IOrderedMultiMap` itself separates these, rather than reintroducing the
       old ambiguity.
     * ``toStr()`` (string formatting for a `.ini` section) is intentionally not provided here:
       it requires ``K``/``V`` to be string-convertible, which isn't true for an arbitrary
       :cpp:class:`IfContentPart`\\<K, V\\> instantiation. It belongs at whichever layer already
       knows ``K``/``V`` concretely (e.g. the `Python`_ binding, where ``str()`` always works).
     @endrst
     *
     * @tparam K The type of the keys stored in the part
     * @tparam V The type of the values stored in the part
     * @tparam KeyHash A hasher for ``K``, used only for the map-shaped ``src`` constructor overloads and the default ``content``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same use as ``KeyHash``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IfContentPart: public IfTemplatePart {
        private:
            // Whether KeyHash/KeyEqual (used by OrderedMultiMapListAdapter<K, V, KeyHash, KeyEqual>'s
            // default 'content', and by the tsl::ordered_map/std::unordered_map 'src' overloads) are
            // themselves usable -- false for the *default* KeyHash = std::hash<K> when e.g.
            // K = pybind11::object, whose std::hash specialization is the standard's "disabled"
            // primary template (no operator(), not default-constructible). Gates
            // makeDefaultContent() below via 'if constexpr' so that branch is simply never
            // instantiated -- and therefore never triggers a compile error -- for a K/KeyHash this
            // doesn't hold for. :raw-html:`<br />`
            //
            // Deliberately checks KeyHash directly rather than
            // is_default_constructible_v<OrderedMultiMapListAdapter<K, V, KeyHash, KeyEqual>>: the
            // adapter overrides IOrderedMultiMap<K, V>'s virtual methods, and instantiating ANY
            // trait against a polymorphic class template forces the compiler to instantiate every
            // virtual override's *body* too (unlike ordinary member functions, which stay lazy) --
            // so checking the adapter type itself would trigger the exact compile error this is
            // trying to avoid, regardless of 'if constexpr'. KeyHash has no virtual functions, so
            // checking it directly carries none of that risk.
            //
            // The pybind11 binding layer never hits the 'false' case anyway: it always passes an
            // explicit 'content' of its own (using PyObjectHash/PyObjectEqual as this
            // instantiation's KeyHash/KeyEqual), so this only matters for a direct C++ caller of
            // IfContentPart<K, V, KeyHash, KeyEqual> that both omits 'content' and leaves KeyHash at
            // its default for a K with no usable std::hash.
            static constexpr bool hasDefaultHashableContent = std::is_default_constructible_v<KeyHash>;

        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define RemoveKeyCheck std::function<bool(long long, const V&)>
            #define Predicate std::function<bool(const V&)>
            #define ReplaceSpec std::variant<V, std::vector<V>, std::pair<V, Predicate>>
            #define KeyRemapList RemapList<K, V>
            #define KeyRemapValue std::variant<KeyRemapList, KeyRemapData<K, V>>
            #define RangeSpec std::vector<Ranges<long long>::Range>
            #define Item IOrderedMultiMap<K, V>::Item
            #else
            using RemoveKeyCheck = typename IOrderedMultiMap<K, V>::RemoveKeyCheck;
            using Predicate = typename IOrderedMultiMap<K, V>::Predicate;
            using ReplaceSpec = typename IOrderedMultiMap<K, V>::ReplaceSpec;
            using KeyRemapValue = typename IOrderedMultiMap<K, V>::KeyRemapValue;
            using RangeSpec = typename IOrderedMultiMap<K, V>::RangeSpec;
            using Item = typename IOrderedMultiMap<K, V>::Item;
            #endif

            /**
             * @brief Constructs a new part, optionally wrapping an existing ``content``
             *
             * @param depth
             @rst
             The depth this part is within the owning `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``0``
             @endrst
             * @param content
             @rst
             The backing ordered-multimap implementation to use, taken by ownership -- this is
             the caller's choice of which concrete :cpp:class:`IOrderedMultiMap` implementation
             backs this part. If omitted (or ``nullptr``), defaults to a fresh, empty
             :cpp:type:`OrderedMultiMapListAdapter`\\<K, V, KeyHash, KeyEqual\\> -- i.e. the
             list-backed :cpp:class:`OrderedMultiMap` using this instantiation's own
             ``KeyHash``/``KeyEqual`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``

             @endrst
             *
             * @throw std::logic_error
             @rst
             Thrown when ``content`` is omitted and ``KeyHash`` (left at its default,
             ``std::hash<K>``) has no usable specialization for ``K`` (so no sensible default
             can be constructed) -- see :cpp:member:`hasDefaultHashableContent`
             @endrst
             */
            explicit IfContentPart(int depth = 0, std::unique_ptr<IOrderedMultiMap<K, V>> content = nullptr);

            /**
             * @brief
             @rst
             Constructs a new part, populated from a fully-indexed, key-deduplicated description
             @endrst
             *
             * @param src
             @rst
             Initial data to populate ``content`` with, as key -> list of ``(index, value)``
             pairs -- one entry per occurrence of that key (each key appears at most once as an
             entry in ``src`` itself; repeat occurrences of that key live in its own list).
             ``index`` is used purely to order every occurrence relative to every other
             occurrence *across all keys* (gathered, stable-sorted by ``index`` ascending, then
             appended in that order via :cpp:func:`IOrderedMultiMap::insertAllEnd`); it is
             **not** a strict absolute position, so gaps and duplicate/out-of-order values are
             fine -- same convention as :cpp:func:`BaseOrderedMultiMap::buildFromIndexed`,
             reimplemented here against the :cpp:class:`IOrderedMultiMap` interface rather than
             requiring that ``content`` be a :cpp:class:`BaseOrderedMultiMap`-derived type. If
             ``content`` already holds data (a pre-populated instance was passed in), ``src``'s
             entries are appended after it, not merged/interleaved with it.
             @endrst
             * @param depth
             @rst
             Same meaning/default as the two-argument constructor's ``depth``
             @endrst
             * @param content
             @rst
             Same meaning/default as the two-argument constructor's ``content``
             @endrst
             */
            explicit IfContentPart(const tsl::ordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& src,
                                    int depth = 0, std::unique_ptr<IOrderedMultiMap<K, V>> content = nullptr);

            /**
             * @brief
             @rst
             Same as the ``tsl::ordered_map`` overload just above, for callers with a plain
             ``std::unordered_map`` instead
             @endrst
             *
             * @param src
             @rst
             Same shape/semantics as the ``tsl::ordered_map`` overload's ``src``, except this
             overload's outer container iterates in an *unspecified* order (since
             ``std::unordered_map``'s own iteration order is unspecified) instead of insertion
             order, so cross-key tie-breaking (among occurrences sharing the same ``index``)
             follows that unspecified order instead.
             @endrst
             * @param depth
             @rst
             Same meaning/default as the two-argument constructor's ``depth``
             @endrst
             * @param content
             @rst
             Same meaning/default as the two-argument constructor's ``content``
             @endrst
             */
            explicit IfContentPart(const std::unordered_map<K, std::vector<std::pair<long long, V>>, KeyHash, KeyEqual>& src,
                                    int depth = 0, std::unique_ptr<IOrderedMultiMap<K, V>> content = nullptr);

            /**
             * @brief
             @rst
             Constructs a new part, populated from a flat, already-ordered list of key-value
             pairs -- a thin convenience over default-constructing then calling
             :cpp:func:`addKVPs`
             @endrst
             *
             * @param src
             @rst
             The key-value pairs to populate ``content`` with, appended in the order given
             (``src[0]`` ends up first, right after whatever ``content`` already held, and so
             on) -- unlike the indexed overloads above, a key may repeat here directly, and
             there's no separate index to reconcile: list position *is* the order.
             @endrst
             * @param depth
             @rst
             Same meaning/default as the two-argument constructor's ``depth``
             @endrst
             * @param content
             @rst
             Same meaning/default as the two-argument constructor's ``content``
             @endrst
             */
            explicit IfContentPart(const std::vector<std::pair<K, V>>& src,
                                    int depth = 0, std::unique_ptr<IOrderedMultiMap<K, V>> content = nullptr);

            IfContentPart(IfContentPart&&) = default;
            IfContentPart& operator=(IfContentPart&&) = default;
            IfContentPart(const IfContentPart&) = delete;
            IfContentPart& operator=(const IfContentPart&) = delete;

            /**
             * @brief
             @rst
             Creates a deep copy of this part, via the backing implementation's own
             :cpp:func:`IOrderedMultiMap::clone`
             @endrst
             *
             * @return A new, independent part holding a clone of this part's content, at the same depth
             */
            std::unique_ptr<IfContentPart<K, V, KeyHash, KeyEqual>> clone() const;

            /**
             * @brief Retrieves the depth this part is within the owning `IfTemplate`
             */
            int depth() const;

            /**
             * @copydoc depth() const
             *
             * @param depth The new depth
             */
            void setDepth(int depth);

            /**
             * @brief Retrieves the backing ordered-multimap implementation directly
             */
            IOrderedMultiMap<K, V>& content();

            /**
             * @copydoc content()
             */
            const IOrderedMultiMap<K, V>& content() const;

            // ---- insert family ----

            /**
             * @copydoc BaseOrderedMultiMap::insert
             */
            void addKVP(const K& key, const V& value);

            /**
             * @copydoc BaseOrderedMultiMap::insertStart
             */
            void addKVPToFront(const K& key, const V& value);

            /**
             * @copydoc BaseOrderedMultiMap::insertAt
             */
            void addKVPAt(long long index, const K& key, const V& value);

            /**
             * @copydoc BaseOrderedMultiMap::insertAllEnd
             */
            void addKVPs(const std::vector<std::pair<K, V>>& kvps);

            /**
             * @copydoc BaseOrderedMultiMap::insertAllStart
             */
            void addKVPsToFront(const std::vector<std::pair<K, V>>& kvps);

            /**
             * @copydoc IOrderedMultiMap::insertAllAt
             */
            size_t addKVPsByInds(const std::vector<std::pair<long long, std::pair<K, V>>>& kvps,
                                  bool sortIndices = true,
                                  const std::optional<RangeSpec>& ranges = std::nullopt);

            // ---- removal ----

            /**
             * @copydoc BaseOrderedMultiMap::removeAt
             */
            bool removeKVPAt(size_t pos, const std::optional<RangeSpec>& ranges = std::nullopt);

            /**
             * @copydoc BaseOrderedMultiMap::removeKey
             */
            size_t removeKey(const K& key, const std::optional<RangeSpec>& ranges = std::nullopt,
                              const std::optional<RemoveKeyCheck>& check = std::nullopt);

            /**
             * @brief
             @rst
             Removes multiple, independently-specified keys -- a thin loop over
             :cpp:func:`removeKey`, one call per entry in ``keys``, sharing one ``ranges`` filter
             across all of them. Not part of :cpp:class:`IOrderedMultiMap` itself (there's no
             bulk multi-key removal primitive there); purely a convenience at this layer.
             @endrst
             *
             * @param keys
             @rst
             Each key to remove, paired with its own optional check predicate --
             ``check(index, value)`` must return ``True`` for a given occurrence of that key to
             actually be removed, if provided; if omitted, every occurrence of that key is
             removed unconditionally (subject to ``ranges`` below)
             @endrst
             * @param ranges
             @rst
             If provided, an occurrence's true positional index (same convention as
             :cpp:func:`getByInd`) must fall within ``ranges`` for it to be eligible for removal,
             for every key in ``keys``
             @endrst
             *
             * @return How many entries were actually removed, summed across every key in ``keys``
             */
            size_t removeKeys(const std::vector<std::pair<K, std::optional<RemoveKeyCheck>>>& keys,
                               const std::optional<RangeSpec>& ranges = std::nullopt);

            // ---- bulk edits ----

            /**
             * @copydoc IOrderedMultiMap::reorder
             */
            void reorder(const std::vector<std::pair<long long, long long>>& orderMap,
                         const std::optional<RangeSpec>& ranges = std::nullopt);

            /**
             * @copydoc BaseOrderedMultiMap::remapKeys
             */
            void remapKeys(const std::vector<std::pair<K, KeyRemapValue>>& keyRemap,
                            const std::optional<RangeSpec>& ranges = std::nullopt);

            /**
             * @copydoc IOrderedMultiMap::replaceVals
             */
            void replaceVals(const std::vector<std::pair<K, ReplaceSpec>>& newVals, bool addNew = true,
                              const std::optional<RangeSpec>& ranges = std::nullopt);

            // ---- lookups ----

            /**
             * @copydoc BaseOrderedMultiMap::contains
             */
            bool contains(const K& key) const;

            /**
             * @copydoc BaseOrderedMultiMap::containsKey
             */
            bool containsKey(const K& key) const;

            /**
             * @copydoc BaseOrderedMultiMap::count
             */
            size_t count(const K& key) const;

            /**
             * @copydoc BaseOrderedMultiMap::size
             */
            size_t size() const;

            /**
             * @copydoc BaseOrderedMultiMap::length
             */
            size_t length() const;

            /**
             * @copydoc BaseOrderedMultiMap::keySize
             */
            size_t keySize() const;

            /**
             * @copydoc BaseOrderedMultiMap::empty
             */
            bool empty() const;

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
             default), every occurrence is included -- equivalent to calling
             :cpp:class:`IOrderedMultiMap`'s own ``getAll`` directly.
             @endrst
             *
             * @return The values for this key, in the requested order
             */
            std::vector<V> getVals(const K& key, bool ordered = true, const std::optional<RangeSpec>& ranges = std::nullopt) const;

            /**
             * @brief
             @rst
             Same as :cpp:func:`getVals`, except each value is paired with its true positional
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
            std::vector<std::pair<long long, V>> getValsWithInds(const K& key, bool ordered = true, const std::optional<RangeSpec>& ranges = std::nullopt) const;

            /**
             * @brief
             @rst
             Retrieves every distinct key currently in this part, as a real
             ``std::unordered_set<K, KeyHash, KeyEqual>`` -- unlike :cpp:class:`IOrderedMultiMap`
             itself, which returns a plain ``std::vector`` since the interface has no
             ``KeyHash``/``KeyEqual`` of its own to build a hash-based set with, this class does
             (as its own template parameters), so it builds the real set here from
             ``content_``'s ``std::vector`` result.
             @endrst
             *
             * @return Every distinct key, as a set (unordered)
             */
            std::unordered_set<K, KeyHash, KeyEqual> getKeys() const;

            /**
             * @copydoc BaseOrderedMultiMap::getByInd(long long) const
             */
            std::pair<K, V> getByInd(long long index) const;

            /**
             * @copydoc BaseOrderedMultiMap::getByInd(long long, bool) const
             */
            std::pair<long long, V> getByIndWithOccurrence(long long index) const;

            /**
             * @copydoc BaseOrderedMultiMap::setValByInd
             */
            void setValByInd(long long index, const V& value);

            /**
             * @copydoc IOrderedMultiMap::entries
             */
            std::vector<std::pair<K, V>> entries() const;

            /**
             * @copydoc IOrderedMultiMap::items
             */
            std::vector<Item> items() const;

            /**
             * @copydoc IOrderedMultiMap::splitByInds
             *
             @rst
             Each resulting part is a fresh :cpp:class:`IfContentPart`\\<K, V\\>, at the same
             :cpp:func:`depth` as ``this``, wrapping one of the backing implementation's own
             split results.
             @endrst
             */
            std::vector<std::unique_ptr<IfContentPart<K, V, KeyHash, KeyEqual>>> splitByInds(
                const std::vector<long long>& inds, bool includeSplitKVP = true,
                bool includeEmptyParts = false, bool sortIndices = true) const;

        private:
            // Builds the default 'content' used when the constructor's own 'content' argument is
            // nullptr -- see hasDefaultHashableContent's comment for why this has to be gated
            // behind 'if constexpr' rather than just called unconditionally.
            static std::unique_ptr<IOrderedMultiMap<K, V>> makeDefaultContent();

            // Implements the two indexed 'src' constructor overloads -- see their doc comments
            // for the exact semantics. Templated on the outer container type (IndexedMap is
            // either the tsl::ordered_map or std::unordered_map overload's own 'src' type) so
            // both share one implementation, the same way BaseOrderedMultiMap::buildFromIndexed
            // is itself templated for its own two indexed-constructor overloads. Gathers every
            // (index, key, value) triple across every key, stable-sorts by index, and appends the
            // result to 'content_' via insertAllEnd(). Deliberately implemented against the
            // IOrderedMultiMap interface rather than delegating to
            // BaseOrderedMultiMap::buildFromIndexed itself, since 'content_' may be any
            // implementation (including one written in pure Python).
            template <typename IndexedMap>
            void populateFromIndexed(const IndexedMap& src);

            std::unique_ptr<IOrderedMultiMap<K, V>> content_;
            int depth_;
    };

}

#include "IfContentPart.tpp"

#endif
