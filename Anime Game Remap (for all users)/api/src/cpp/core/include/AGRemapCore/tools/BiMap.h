#ifndef AGRemapCore_BiMap_H
#define AGRemapCore_BiMap_H

#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <tuple>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A simple `one-to-one`_ map
     @endrst
     *
     * @tparam K 
     *      The type for the keys of the map
     * 
     * @tparam V
     *      The type for the values of the map
     * 
     * @tparam KHash
     *      The hash function for the keys
     * 
     * @tparam KEqual
     *      The equality function for the keys
     * 
     * @tparam VHash
     *      The hash function for the values
     * 
     * @tparam VEqual
     *      The equality function for the values 
     */
    template <typename K, typename V, typename KHash = std::hash<K>, typename KEqual = std::equal_to<K>, typename VHash = std::hash<V>, typename VEqual = std::equal_to<V>>
    class BiMap {
        protected:
            /**
             * @brief
             @rst
             The `injective`_ map
             @endrst
             */
            std::unordered_map<K, V, KHash, KEqual> forward;

            /**
             * @brief
             @rst
             The `surjective`_ map
             @endrst
             */
            std::unordered_map<V, K, VHash, VEqual> backward;

        public:
            /**
             * @brief Clears the map
             */
            void clear();

            /**
             * @brief Determines whether the map is empty
             * 
             * @return Whether the map is empty
             */
            bool empty();

            /**
             * @brief
             @rst
             Adds a new `KVP`_
             @endrst
             *
             * @return Whether the add operation was successful
             */
            bool add(const K& key, const V& val);

            /**
             * @brief
             @rst
             Adds a new `KVP`_
             @endrst
             *
             * @throw std::runtime_error Thrown when a duplicate key or value has been inserted
             */
            void insert(const K& key, const V& val);

            /**
             * @brief Retrieves the reference to the corresponding value
             * 
             * @tparam KeyLike The type for the key
             * 
             * @throw std::out_of_range Thrown if no value is found
             * 
             * @return the corresponding value
             */
            template <typename KeyLike>
            const V& getValue(const KeyLike& key) const;

            /**
             * @brief Retrieves the reference to the corresponding key
             * 
             * @tparam ValueLike The type for the value
             * 
             * @throw std::out_of_range Thrown if no key is found
             * 
             * @return the corresponding key
             */
            template <typename ValueLike>
            const K& getKey(const ValueLike& val) const;

            /**
             * @brief Retrieves the number of elements in the map
             * 
             * @return The number of elements
             */
            size_t size() const;

            using iterator = typename std::unordered_map<K, V, KHash, KEqual>::iterator;
            using const_iterator = typename std::unordered_map<K, V, KHash, KEqual>::const_iterator;

            iterator begin();
            iterator end();
            const_iterator begin() const;
            const_iterator end() const;
            const_iterator cbegin() const;
            const_iterator cend() const;

            /**
             * @brief Retrieves the pointer to the corresponding value
             * 
             * @tparam KeyLike The type for the key
             * 
             * @return The pointer to the value if available, otherwise returns the null pointer
             */
            template <typename KeyLike>
            const V* findValuePtr(const KeyLike& key) const;

            /**
             * @brief Retrieves the pointers to both the key and value using lookup by key
             * 
             * @tparam KeyLike The type for the key
             * 
             * @return A tuple containing the pointers to the key and the value if available, otherwise both poitners are null pointers
             */
            template <typename KeyLike>
            std::tuple<const K*, const V*> findKVPPtrByKey(const KeyLike& key) const;

            /**
             * @brief Retrieves the corresponding value
             * 
             * @tparam KeyLike The type for the key
             * 
             * @return The corresponding value if available, otherwise returns `std::nullopt`
             */
            template <typename KeyLike>
            std::optional<V> findValue(const KeyLike& key) const;

            /**
             * @brief Retrieves the pointer to the corresponding key
             * 
             * @tparam ValueLike The type for the value
             * 
             * @return The pointer to the key if available, otherwise returns the null pointer
             */
            template <typename ValueLike>
            const K* findKeyPtr(const ValueLike& val) const;

            /**
             * @brief Retrieves the pointers to both the key and value using lookup by value
             * 
             * @tparam ValueLike The type for the value
             * 
             * @return A tuple containing the pointers to the key and the value if available, otherwise both poitners are null pointers
             */
            template <typename ValueLike>
            std::tuple<const K*, const V*> findKVPPtrByVal(const ValueLike& val) const;

            /**
             * @brief Retrieves the corresponding key
             * 
             * @tparam ValueLike The type for the value
             * 
             * @return The corresponding key if available, otherwise returns `std::nullopt`
             */
            template <typename ValueLike>
            std::optional<K> findKey(const ValueLike& val) const;

            /**
             * @brief Whether the map contains the corresponding key
             * 
             * @tparam KeyLike The type for the key
             * 
             * @return Whether the key is in the map
             */
            template <typename KeyLike>
            bool containsKey(const KeyLike& key) const;

            /**
             * @brief Whether the map contains the corresponding value
             * 
             * @tparam ValueLike The type for the value
             * 
             * @return Whether the value is in the map
             */
            template <typename ValueLike>
            bool containsValue(const ValueLike& val) const;

    };
}

#include "BiMap.tpp"

#endif