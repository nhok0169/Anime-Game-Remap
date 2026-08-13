#ifndef AGRemapCore_OrderedMultiMapAdapter_H
#define AGRemapCore_OrderedMultiMapAdapter_H

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include "AGRemapCore/tools/orderedMultiMap/IOrderedMultiMap.h"
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMap.h"
#include "AGRemapCore/tools/orderedMultiMap/OrderedMultiMapSqrt.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IOrderedMultiMap`

     Wraps any `CRTP`_-shaped ordered-multimap implementation (:cpp:class:`OrderedMultiMap`,
     :cpp:class:`OrderedMultiMapSqrt`, or a user's own ``Derived`` implementing
     :cpp:class:`BaseOrderedMultiMap`'s ``rawXxx`` primitive set) to satisfy
     :cpp:class:`IOrderedMultiMap`, so it can be used anywhere the virtual interface is needed
     -- without adding a vtable to ``Impl`` itself. Every call is a thin forward onto the owned
     ``Impl`` instance; the only real work here is converting between the interface's
     hash-free ``std::vector``-based parameter shapes and ``Impl``'s own
     `tsl::ordered_map`_/``std::unordered_map`` ones. :raw-html:`<br />` :raw-html:`<br />`

     ``KeyHash``/``KeyEqual`` must match ``Impl``'s own exactly (they can't be discovered from
     ``Impl`` automatically -- its ``Base`` alias is private) -- getting this wrong is a compile
     error, not a silent bug: the forwarding calls simply won't find a matching overload on
     ``Impl``. :cpp:type:`OrderedMultiMapListAdapter`/:cpp:type:`OrderedMultiMapSqrtAdapter`
     below cover the common case of wrapping this project's own two backing structures without
     having to repeat ``Impl``'s full template argument list at every use site.
     @endrst
     *
     * @tparam Impl The concrete backing-structure type to wrap (e.g. ``OrderedMultiMap<K, V, KeyHash, KeyEqual>``)
     * @tparam K The type of the keys stored in the map
     * @tparam V The type of the values stored in the map
     * @tparam KeyHash A hasher for ``K``, matching ``Impl``'s own. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, matching ``Impl``'s own. Defaults to ``std::equal_to<K>``
     */
    template <typename Impl, typename K, typename V,
              typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class OrderedMultiMapAdapter: public IOrderedMultiMap<K, V> {
        public:
            #ifdef AGREMAPCORE_DOCS_PARSE
            #define Interface IOrderedMultiMap<K, V>
            #define RemoveKeyCheck std::function<bool(long long, const V&)>
            #define Predicate std::function<bool(const V&)>
            #define ReplaceSpec std::variant<V, std::vector<V>, std::pair<V, Predicate>>
            #define KeyRemapList RemapList<K, V>
            #define KeyRemapValue std::variant<KeyRemapList, KeyRemapData<K, V>>
            #define RangeSpec std::vector<Ranges<long long>::Range>
            #define Item IOrderedMultiMap<K, V>::Item
            #else
            using Interface = IOrderedMultiMap<K, V>;
            using RemoveKeyCheck = typename Interface::RemoveKeyCheck;
            using ReplaceSpec = typename Interface::ReplaceSpec;
            using KeyRemapValue = typename Interface::KeyRemapValue;
            using RangeSpec = typename Interface::RangeSpec;
            using Item = typename Interface::Item;
            #endif

            /**
             * @brief Wraps a default-constructed ``Impl`` (an empty map)
             */
            OrderedMultiMapAdapter() = default;

            /**
             * @brief Wraps the given ``Impl`` instance, taking ownership of it
             *
             * @param impl The instance to wrap
             */
            explicit OrderedMultiMapAdapter(Impl impl): impl_(std::move(impl)) {}

            /**
             * @brief Retrieves the wrapped instance
             *
             * @return The wrapped instance
             */
            const Impl& impl() const { return impl_; }

            /**
             * @copydoc impl() const
             */
            Impl& impl() { return impl_; }

            void insert(const K& key, const V& value) override { impl_.insert(key, value); }
            void insertStart(const K& key, const V& value) override { impl_.insertStart(key, value); }
            void insertAt(long long index, const K& key, const V& value) override { impl_.insertAt(index, key, value); }
            void insertAllEnd(const std::vector<std::pair<K, V>>& items) override { impl_.insertAllEnd(items); }
            void insertAllStart(const std::vector<std::pair<K, V>>& items) override { impl_.insertAllStart(items); }

            size_t insertAllAt(const std::vector<std::pair<long long, std::pair<K, V>>>& items,
                                bool sortIndices,
                                const std::optional<RangeSpec>& ranges) override {
                tsl::ordered_map<long long, std::pair<K, V>> converted(items.begin(), items.end());
                return impl_.insertAllAt(converted, sortIndices, toRanges(ranges));
            }

            void reorder(const std::vector<std::pair<long long, long long>>& orderMap,
                         const std::optional<RangeSpec>& ranges) override {
                tsl::ordered_map<long long, long long> converted(orderMap.begin(), orderMap.end());
                impl_.reorder(converted, toRanges(ranges));
            }

            bool removeAt(size_t pos, const std::optional<RangeSpec>& ranges) override {
                return impl_.removeAt(pos, toRanges(ranges));
            }

            size_t removeKey(const K& key, const std::optional<RangeSpec>& ranges,
                              const std::optional<RemoveKeyCheck>& check) override {
                return impl_.removeKey(key, toRanges(ranges), check);
            }

            void remapKeys(const std::vector<std::pair<K, KeyRemapValue>>& keyRemap,
                            const std::optional<RangeSpec>& ranges) override {
                std::unordered_map<K, KeyRemapValue, KeyHash, KeyEqual> converted(keyRemap.begin(), keyRemap.end());
                impl_.remapKeys(converted, toRanges(ranges));
            }

            void replaceVals(const std::vector<std::pair<K, ReplaceSpec>>& newVals, bool addNew,
                              const std::optional<RangeSpec>& ranges) override {
                tsl::ordered_map<K, ReplaceSpec, KeyHash, KeyEqual> converted(newVals.begin(), newVals.end());
                impl_.replaceVals(converted, addNew, toRanges(ranges));
            }

            bool contains(const K& key) const override { return impl_.contains(key); }
            bool containsKey(const K& key) const override { return impl_.containsKey(key); }
            size_t count(const K& key) const override { return impl_.count(key); }
            size_t size() const override { return impl_.size(); }
            size_t length() const override { return impl_.length(); }
            size_t keySize() const override { return impl_.keySize(); }
            bool empty() const override { return impl_.empty(); }

            std::vector<V> getAll(const K& key, bool ordered, const std::optional<RangeSpec>& ranges) const override {
                return impl_.getAll(key, ordered, toRanges(ranges));
            }

            std::vector<std::pair<long long, V>> getAllWithInds(const K& key, bool ordered, const std::optional<RangeSpec>& ranges) const override {
                return impl_.getAll(key, ordered, true, toRanges(ranges));
            }

            std::vector<K> getKeys() const override {
                auto keys = impl_.getKeys();
                return std::vector<K>(keys.begin(), keys.end());
            }

            std::pair<K, V> getByInd(long long index) const override {
                auto result = impl_.getByInd(index);
                return {result.first, result.second};
            }

            std::pair<long long, V> getByIndWithOccurrence(long long index) const override {
                return impl_.getByInd(index, true);
            }

            void setValByInd(long long index, const V& value) override {
                impl_.setValByInd(index, value);
            }

            // entries()/items() walk impl_ via its Iterator (common to every BaseOrderedMultiMap
            // Derived) rather than calling impl_.entries() directly -- OrderedMultiMap's and
            // OrderedMultiMapSqrt's entries() return genuinely different shapes (a reference into
            // a std::list vs. a fresh std::vector), so the Iterator is the one thing guaranteed
            // to look the same across every possible Impl.
            std::vector<std::pair<K, V>> entries() const override {
                std::vector<std::pair<K, V>> result;
                result.reserve(impl_.size());
                for (auto it = impl_.begin(); it != impl_.end(); ++it) {
                    auto item = *it;
                    result.emplace_back(item.key, item.value);
                }
                return result;
            }

            std::vector<Item> items() const override {
                std::vector<Item> result;
                result.reserve(impl_.size());
                for (auto it = impl_.begin(); it != impl_.end(); ++it) {
                    auto item = *it;
                    result.push_back(Item{item.key, item.value, item.occurrenceIndex, item.orderIndex});
                }
                return result;
            }

            std::vector<std::unique_ptr<Interface>> splitByInds(
                const std::vector<long long>& inds,
                bool includeSplitKVP,
                bool includeEmptyParts,
                bool sortIndices) const override {
                tsl::ordered_set<long long> converted(inds.begin(), inds.end());
                std::vector<Impl> parts = impl_.splitByInds(converted, includeSplitKVP, includeEmptyParts, sortIndices);

                std::vector<std::unique_ptr<Interface>> result;
                result.reserve(parts.size());
                for (Impl& part : parts) {
                    result.push_back(std::make_unique<OrderedMultiMapAdapter>(std::move(part)));
                }
                return result;
            }

            std::unique_ptr<Interface> clone() const override {
                return std::make_unique<OrderedMultiMapAdapter>(impl_);
            }

        private:
            // Rebuilds a real, polymorphic Ranges<long long> from the interface's plain
            // RangeSpec shape -- see IOrderedMultiMap::RangeSpec's doc comment for why the
            // interface itself can't take Ranges<long long> directly. normalize=true since,
            // unlike PyRanges.h's toCoreRanges() (which slices an already-normalized bound
            // PyRanges), a RangeSpec arriving here carries no such guarantee.
            static std::optional<Ranges<long long>> toRanges(const std::optional<RangeSpec>& ranges) {
                if (!ranges.has_value()) {
                    return std::nullopt;
                }
                return Ranges<long long>(*ranges, true);
            }

            Impl impl_;
    };


    /**
     * @brief
     @rst
     Shorthand for wrapping this project's own list-backed :cpp:class:`OrderedMultiMap` in an
     :cpp:class:`OrderedMultiMapAdapter`
     @endrst
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    using OrderedMultiMapListAdapter = OrderedMultiMapAdapter<OrderedMultiMap<K, V, KeyHash, KeyEqual>, K, V, KeyHash, KeyEqual>;

    /**
     * @brief
     @rst
     Shorthand for wrapping this project's own sqrt-decomposed :cpp:class:`OrderedMultiMapSqrt`
     in an :cpp:class:`OrderedMultiMapAdapter`
     @endrst
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    using OrderedMultiMapSqrtAdapter = OrderedMultiMapAdapter<OrderedMultiMapSqrt<K, V, KeyHash, KeyEqual>, K, V, KeyHash, KeyEqual>;

}

#endif
