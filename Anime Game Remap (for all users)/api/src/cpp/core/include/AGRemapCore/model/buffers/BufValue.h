#ifndef AGRemapCore_BufValue_H
#define AGRemapCore_BufValue_H

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

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A single decoded value for one elementary data type within a ``.buf`` file
     (:cpp:class:`BufDataType`) -- either a signed integer, an unsigned integer, or a
     `floating point`_ number :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original (``BufDataType.decode``) returns a plain ``Any``, since Python's
        ``int`` is arbitrary-precision and duck typing needs no closed set of alternatives. This
        C++ port instead caps integer decoding at 64 bits (``long long``/``unsigned long long``) --
        every ``.buf`` format actually defined in this codebase (``BufDataTypes``: ``Float32``,
        ``Int32``, ``UInt32``, ``UNorm8``) uses at most 4 bytes, so 64 bits leaves ample headroom
        without needing an arbitrary-width big-integer representation
     @endrst
     */
    using BufValue = std::variant<long long, unsigned long long, double>;

    /**
     * @brief
     @rst
     The decoded data for one line (one vertex) of a ``.buf`` file -- the keys are the element
     keys computed by :cpp:func:`BufFile::setElements` (an element's :cpp:func:`BufType::getName`,
     suffixed with an occurrence count if the same name repeats) and the values are the decoded
     :cpp:type:`BufValue`\\s for that element, one per :cpp:class:`BufDataType` composing it
     @endrst
     */
    using BufLineData = std::unordered_map<std::string, std::vector<BufValue>>;

    /**
     * @brief
     @rst
     One :cpp:type:`BufValue` as an integer, whatever alternative it actually holds
     :raw-html:`<br />` :raw-html:`<br />`

     Here rather than in each reader because a caller that tests for ONE alternative --
     ``std::holds_alternative<unsigned long long>(v) ? std::get<unsigned long long>(v) : 0`` -- is
     correct only while the :cpp:class:`BufElementType`\s it read the buffer with produce that
     alternative, and silently wrong otherwise. Against a `blend`_ buffer read with the float
     weights of :cpp:func:`BlendFile::defaultElements` such a test reads every influence as
     weight-zero, so a remap does nothing while reporting success. Six sites hand-rolled it
     @endrst
     * @param value The decoded value
     * @return It as an integer
     */
    inline long long bufValueAsInt(const BufValue& value) {
        return std::visit([](auto&& v) -> long long { return static_cast<long long>(v); }, value);
    }

    /**
     * @brief One :cpp:type:`BufValue` as a `floating point`_ number, whatever alternative it holds
     *      -- see \ref bufValueAsInt
     * @param value The decoded value
     * @return It as a number
     */
    inline double bufValueAsFloat(const BufValue& value) {
        return std::visit([](auto&& v) -> double { return static_cast<double>(v); }, value);
    }

    /**
     * @brief A raw sequence of bytes -- used throughout the ``.buf`` file model instead of
     *      Python's ``bytes``
     */
    using ByteVec = std::vector<std::uint8_t>;
}

#endif
