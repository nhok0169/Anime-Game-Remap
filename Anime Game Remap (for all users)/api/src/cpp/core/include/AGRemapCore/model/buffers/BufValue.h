#ifndef AGRemapCore_BufValue_H
#define AGRemapCore_BufValue_H

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
     * @brief A raw sequence of bytes -- used throughout the ``.buf`` file model instead of
     *      Python's ``bytes``
     */
    using ByteVec = std::vector<std::uint8_t>;
}

#endif
