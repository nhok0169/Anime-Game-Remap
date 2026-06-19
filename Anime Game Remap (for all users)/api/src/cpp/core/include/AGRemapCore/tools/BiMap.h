#ifndef AGRemapCore_BiMap_H
#define AGRemapCore_BiMap_H

#include <unordered_map>
#include <stdexcept>
#include <optional>


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
             * @throw std::out_of_range Thrown if no value is found
             * 
             * @return the corresponding value
             */
            const V& getValue(const K& key) const;

            /**
             * @brief Retrieves the reference to the corresponding key
             * 
             * @throw std::out_of_range Thrown if no key is found
             * 
             * @return the corresponding key
             */
            const K& getKey(const V& val) const;

            /**
             * @brief Retrieves the number of elements in the map
             * 
             * @return The number of elements
             */
            size_t size() const;

            /**
             * @brief Retrieves the pointer to the corresponding value
             * 
             * @return The pointer to the value if available, otherwise returns the null pointer
             */
            const V* findValuePtr(const K& key) const;

            /**
             * @brief Retrieves the corresponding value
             * 
             * @return The corresponding value if available, otherwise returns `std::nullopt`
             */
            std::optional<V> findValue(const K& key) const;

            /**
             * @brief Retrieves the pointer to the corresponding key
             * 
             * @return The pointer to the key if available, otherwise returns the null pointer
             */
            const K* findKeyPtr(const V& val) const;

            /**
             * @brief Retrieves the corresponding key
             * 
             * @return The corresponding key if available, otherwise returns `std::nullopt`
             */
            std::optional<K> findKey(const V& val) const;

            /**
             * @brief Whether the map contains the corresponding key
             * 
             * @return Whether the key is in the map
             */
            bool containsKey(const K& key) const;

            /**
             * @brief Whether the map contains the corresponding value
             * 
             * @return Whether the value is in the map
             */
            bool containsValue(const V& val) const;

    };
}

#include "BiMap.tpp"

#endif