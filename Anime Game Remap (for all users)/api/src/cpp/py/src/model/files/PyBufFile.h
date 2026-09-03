#ifndef AGRemapPyBind_PyBufFile_H
#define AGRemapPyBind_PyBufFile_H

#include <pybind11/pybind11.h>

#include <memory>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BufFile.h"

/**
 * @brief Takes ownership of every element in 'elements' (each already an existing
 *      CppBufElementType Python object) -- same ownership-transfer contract as
 *      PyBufElementType.h's own parseBufDataTypes, one hierarchy level up
 *
 * @param elements The Python iterable of CppBufElementType instances
 *
 * @return The elements, now owned by the returned vector
 */
std::vector<std::unique_ptr<AGRemapCore::BufElementType>> parseBufElementTypes(const pybind11::object &elements);

/**
 * @brief Wraps a single Python filter callable into a :cpp:type:`AGRemapCore::BufFile::Filter`
 *
 * @param filter The Python callable, matching :cpp:type:`AGRemapCore::BufFile::Filter`'s own signature
 */
AGRemapCore::BufFile::Filter parseFilter(const pybind11::function &filter);

/**
 * @brief Wraps a list of Python filter callables into the vector :cpp:func:`AGRemapCore::BufFile::fix` takes
 *
 * @param filters The Python callables to wrap, applied in order
 */
std::vector<AGRemapCore::BufFile::Filter> parseFilters(const std::vector<pybind11::function> &filters);

/**
 * @brief Converts a :cpp:type:`AGRemapCore::BufFile::FixResult` into the equivalent Python value
 *      (a :class:`str` or a :class:`bytearray`)
 *
 * @param result The result to convert
 */
pybind11::object fixResultToPy(const AGRemapCore::BufFile::FixResult &result);

/**
 * @brief
 @rst
 The inverse of :cpp:func:`fixResultToPy` -- converts a Python ``Union[str, bytes-like]`` value
 (eg. a ``fix`` override's return value) back into a :cpp:type:`AGRemapCore::BufFile::FixResult`
 :raw-html:`<br />` :raw-html:`<br />`

 Goes through :cpp:class:`pybind11::buffer` for the non-``str`` branch rather than `pybind11`_'s
 automatic ``std::variant`` casting -- a ``bytes``/``bytearray`` object also satisfies the generic
 ``std::vector<uint8_t>`` caster's sequence check (the same ambiguity this file's own
 ``pyBytesToVec``/``vecToPyByteArray`` pair exists to avoid on the *outgoing* direction), so this
 sidesteps it explicitly rather than relying on the automatic caster to pick the right alternative
 @endrst
 *
 * @param result The Python value to convert -- a :class:`str`, or any ``bytes``-like object
 *      (``bytes``, ``bytearray``, ...)
 */
AGRemapCore::BufFile::FixResult pyToFixResult(const pybind11::object &result);

void initCppBufFile(pybind11::module_ &m);

#endif
