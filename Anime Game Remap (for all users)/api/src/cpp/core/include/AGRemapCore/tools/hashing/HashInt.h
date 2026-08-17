#ifndef AGRemapCore_HashInt_H
#define AGRemapCore_HashInt_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>


namespace AGRemapCore {

    // Internal helper for HashInt -- not part of the public API, kept in a `detail` namespace
    // with plain (non-Doxygen) comments.
    namespace detail {
        // Encodes a byte buffer into an (unpadded) base64 string, using this project's base64
        // alphabet: standard base64 (A-Za-z0-9+/) with the 63rd digit ('/') replaced by '_'
        // (see IntTools::toBase64).
        std::string encodeBase64(const std::uint8_t *bytes, std::size_t size);
    }

    /**
     * @brief
     @rst
     CRTP base class for a deterministic, fixed-width hash id that's fundamentally just a block
     of raw bytes (eg. :cpp:class:`Hash64`, :cpp:class:`Hash128`).

     Holds the formatting/comparison logic that's the same regardless of which hashing algorithm
     produced the bytes (hex/base64 string conversion, equality, ordering) so a concrete hash
     type only has to know how to fill :cpp:member:`bytes_` and how to expose its own natively-typed
     accessors (eg. :cpp:func:`Hash64::getValue`, :cpp:func:`Hash128::getHigh`/
     :cpp:func:`Hash128::getLow`).

     Uses CRTP rather than virtual dispatch, since these are meant to be cheap, hot-path value
     types (eg. usable directly as ``unordered_map``/``unordered_set`` keys) -- see this
     project's other CRTP base, ``BaseOrderedMultiMap``, for the same rationale.
     @endrst
     *
     * @tparam Derived The concrete hash type inheriting from this class
     * @tparam ByteSize The number of raw bytes making up the hash
     */
    template <typename Derived, std::size_t ByteSize>
    class HashInt {
        protected:
            /**
             * @brief The raw bytes of the hash, big-endian (most-significant byte first)
             */
            std::array<std::uint8_t, ByteSize> bytes_{};

        public:
            /**
             * @brief The hash as a fixed-length, lowercase hex string (2 characters per byte)
             */
            std::string toHexString() const;

            /**
             * @brief The hash as a fixed-length base64 string
             *
             * @rst
             * Uses this project's base64 alphabet -- standard base64 (``A-Za-z0-9+/``) with the
             * 63rd digit (``/``) replaced by ``_`` (see :cpp:func:`IntTools::toBase64`) -- with
             * no padding character, applied over the hash's raw bytes.
             * @endrst
             */
            std::string toBase64() const;

            bool operator==(const HashInt &other) const noexcept;
            bool operator!=(const HashInt &other) const noexcept;

            /**
             * @brief An arbitrary but consistent (and deterministic) total ordering, so that this
             * hash type can be used as a key in ordered containers (eg. ``std::map``, ``std::set``)
             */
            bool operator<(const HashInt &other) const noexcept;

            /**
             * @brief
             @rst
             A ``std::hash``-compatible hash of this value's raw bytes (`FNV-1a`_).

             Used to implement ``std::hash<Derived>`` for each concrete hash type, so that type
             is directly usable as a key in unordered containers (eg. ``std::unordered_map``,
             ``std::unordered_set``).
             @endrst
             */
            std::size_t hashCode() const noexcept;
    };
}

#include "AGRemapCore/tools/hashing/HashInt.tpp"

#endif
