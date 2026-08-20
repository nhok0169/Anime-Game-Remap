#ifndef AGRemapPyBind_PyVersion_H
#define AGRemapPyBind_PyVersion_H

#include <optional>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/Version.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 Converts a flexible Python "version" argument (``None``, a :class:`str`, an :class:`int`/
 :class:`float`, or an already-bound :cpp:class:`AGRC::Version` instance) into an
 :cpp:class:`AGRC::Version`, the same flexibility the pure-Python ``Version.getVersion``
 accepts :raw-html:`<br />` :raw-html:`<br />`

 Shared by every :cpp:class:`CppModDictAssets`/:cpp:class:`CppModMappedAssets` method taking a
 version argument, so all of them accept the same range of Python inputs consistently
 @endrst
 *
 * @param raw The raw Python version argument
 *
 * @throws py::value_error If 'raw' is not ``None`` and doesn't parse as a valid version (see
 *      :cpp:func:`AGRC::Version::parse`)
 *
 * @return The parsed version, or ``std::nullopt`` if 'raw' is ``None``
 */
std::optional<AGRC::Version> parseVersionArg(const py::object &raw);

void initCppVersion(pybind11::module_ &m);

#endif
