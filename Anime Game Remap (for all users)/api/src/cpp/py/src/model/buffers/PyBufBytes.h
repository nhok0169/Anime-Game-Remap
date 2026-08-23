#ifndef AGRemapPyBind_PyBufBytes_H
#define AGRemapPyBind_PyBufBytes_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BinaryFile.h"


/**
 * @brief
 * Converts a Python 'bytes'-like object into an AGRC::ByteVec
 *
 * pybind11's default <pybind11/stl.h> caster for std::vector<uint8_t> treats it as a plain list
 * of ints on the Python side (converting *to* Python), which is not what any of the .buf file
 * model's decode/encode/read/fix methods should hand back -- every one of them is bound via a
 * lambda that goes through this pair of helpers instead, so the Python-facing surface works with
 * real 'bytes' objects, matching the pure-Python original exactly. A 'bytes' object also happens
 * to satisfy the generic list_caster's sequence check on the way *in* (each element iterates as
 * an int), so accepting a plain std::vector<uint8_t> parameter directly would technically compile
 * -- but would only accept a list of ints from Python callers, not a real bytes object, which is
 * why the 'in' direction goes through this same explicit conversion too.
 *
 * @param src The Python bytes-like object to convert
 *
 * @return The equivalent byte vector
 */
AGRemapCore::ByteVec pyBytesToVec(const pybind11::bytes &src);

/**
 * @brief Converts an AGRC::ByteVec into a real Python 'bytes' object
 *
 * @param src The byte vector to convert
 *
 * @return The equivalent 'bytes' object
 */
pybind11::bytes vecToPyBytes(const AGRemapCore::ByteVec &src);

/**
 * @brief
 * Converts an AGRC::ByteVec into a real Python 'bytearray' object -- used specifically for
 * BufFile::fix's no-'fixedFile' return, which the pure-Python original builds as a mutable
 * 'bytearray' (not 'bytes')
 *
 * @param src The byte vector to convert
 *
 * @return The equivalent 'bytearray' object
 */
pybind11::bytearray vecToPyByteArray(const AGRemapCore::ByteVec &src);

/**
 * @brief
 * Converts a Python ``Union[str, bytes]`` value into an AGRC::BinarySrc, for the ``src`` parameter
 * every ``.buf``/binary file class constructor accepts
 *
 * @param src The Python value to convert
 *
 * @return The equivalent AGRC::BinarySrc
 *
 * @throws pybind11::type_error if 'src' is neither a 'str' nor a 'bytes'-like object
 */
AGRemapCore::BinarySrc pyToBinarySrc(const pybind11::object &src);

/**
 * @brief Converts an AGRC::BinarySrc back into the Python ``Union[str, bytes]`` value it came from
 *
 * @param src The AGRC::BinarySrc to convert
 *
 * @return The equivalent Python value
 */
pybind11::object binarySrcToPy(const AGRemapCore::BinarySrc &src);

#endif
