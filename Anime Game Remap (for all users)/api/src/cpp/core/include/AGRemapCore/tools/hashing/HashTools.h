#ifndef AGRemapCore_HashTools_H
#define AGRemapCore_HashTools_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "AGRemapCore/tools/hashing/Hash128.h"


namespace AGRemapCore {
    /**
     * @brief Tools for deterministically hashing data
     */
    class HashTools {
        public:
            /**
             * @brief Deterministically hashes a buffer of bytes
             *
             * @param data Pointer to the start of the buffer to hash
             * @param size The number of bytes in the buffer
             *
             * @return The resultant deterministic hash
             */
            static Hash128 getDeterministicHash(const void *data, std::size_t size) noexcept;

            /**
             * @brief Deterministically hashes a string
             *
             * @param str The string to hash
             *
             * @return The resultant deterministic hash
             */
            static Hash128 getDeterministicHash(std::string_view str) noexcept;

            /**
             * @brief Deterministically hashes a buffer of bytes
             *
             * @param data Pointer to the start of the buffer to hash
             * @param size The number of bytes in the buffer
             *
             * @return The resultant deterministic hash, as a base64 string (see :cpp:func:`Hash128::toBase64`)
             */
            static std::string getDeterministicHashStr(const void *data, std::size_t size);

            /**
             * @brief Deterministically hashes a string
             *
             * @param str The string to hash
             *
             * @return The resultant deterministic hash, as a base64 string (see :cpp:func:`Hash128::toBase64`)
             */
            static std::string getDeterministicHashStr(std::string_view str);

            /**
             * @brief
             @rst
             Deterministically hashes a buffer of bytes into a short, compact base64 string.

             The hash is reduced modulo :math:`2^{16}` before being converted to base64, so
             unlike :cpp:func:`getDeterministicHashStr`, collisions across different inputs are
             expected. To disambiguate a collision, every occurrence of a short hash value after
             the first has ``_<frequency>`` appended, where ``<frequency>`` (itself base64-encoded)
             counts how many times that short hash value has already been produced by this method.
             @endrst
             *
             * @param data Pointer to the start of the buffer to hash
             * @param size The number of bytes in the buffer
             *
             * @return The resultant short, possibly-colliding hash, as a base64 string
             */
            static std::string getShortDeterministicHashStr(const void *data, std::size_t size);

            /**
             * @brief
             @rst
             Deterministically hashes a string into a short, compact base64 string. See the
             ``(const void*, std::size_t)`` overload of this method for the full explanation of
             the collision-disambiguation behaviour.
             @endrst
             *
             * @param str The string to hash
             *
             * @return The resultant short, possibly-colliding hash, as a base64 string
             */
            static std::string getShortDeterministicHashStr(std::string_view str);

            /**
             * @brief
             @rst
             The STABLE short hash of a string: the same input always comes back as the same
             token :raw-html:`<br />` :raw-html:`<br />`

             :cpp:func:`getShortDeterministicHashStr` deliberately does NOT do this. It hands out a
             FRESH token on every call -- ask it twice for the same string and you get ``HfW`` then
             ``HfW_B`` -- because its job is to name a series of distinct things that may happen to
             collide. That is right for a caller naming each of N sections; it is wrong for a caller
             asking "what is the name for THIS texture", which is a question with one answer
             :raw-html:`<br />` :raw-html:`<br />`

             Collision safety is kept: two DIFFERENT strings that hash to the same short value are
             still disambiguated, because only a repeat of the same input is served from the memo
             @endrst
             *
             * @param str The string to hash
             *
             * @return The resultant short hash, stable for the lifetime of the process or until
             *         :cpp:func:`clear` is called
             */
            static std::string getStableShortHashStr(std::string_view str);

            /**
             * @brief
             @rst
             Clears any saved internal state this class accumulates across calls -- the
             ``shortHashFrequency_`` collision-disambiguation counts used by
             ``getShortDeterministicHashStr()``, and the ``stableShortHash_`` memo behind
             ``getStableShortHashStr()``.
             @endrst
             */
            static void clear();

        private:
            /**
             * @brief The exclusive upper bound (:math:`2^{16}`) for a value produced by
             * getShortDeterministicHashStr(), before collision disambiguation
             */
            static constexpr std::uint64_t ShortHashMaxVal = 1ull << 16;

            /**
             * @brief The token already handed out for a given input string, so
             * getStableShortHashStr() can answer a repeat with the same name
             */
            static std::unordered_map<std::string, std::string> stableShortHash_;

            /**
             * @brief
             @rst
             How many times each short hash value has been produced so far by
             ``getShortDeterministicHashStr()``, keyed by that short hash value.

             Not encountering a key yet implies a frequency of 0 (relies on
             ``std::unordered_map``'s own default-insertion behaviour for missing keys).
             @endrst
             */
            static std::unordered_map<std::uint64_t, std::uint64_t> shortHashFrequency_;

            /**
             * @brief Encodes 'shortHash' as a base64 string, appending a base64-encoded,
             * disambiguating frequency suffix if 'shortHash' has been produced before, then
             * records this occurrence
             *
             * @param shortHash The short hash value (already reduced modulo #ShortHashMaxVal)
             *
             * @return The resultant, possibly disambiguated, base64 string
             */
            static std::string disambiguateShortHash(std::uint64_t shortHash);
    };
}

#endif
