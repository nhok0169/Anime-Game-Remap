#include "PyGlobalModTypes.h"

#include <pybind11/stl.h>

#include "AGRemapCore/constants/GlobalModTypes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGlobalModTypes(pybind11::module_ &m) {
    py::class_<AGRC::GlobalModTypes>(m, "CppGlobalModTypes", R"doc(
Every :class:`CppModType` the software ships with, and the one place that files them into
:class:`ModTypeIdTools`'s global registry

The counterpart to the pure-Python :class:`ModTypes` enum, whose ``getAll()`` likewise builds the
shipped mod types on demand

.. important::
    :meth:`registerAll` is **not** called automatically by anything in ``AGRemapCore``, and that is
    deliberate. :meth:`ModTypeIdTools.getModType` and :meth:`ModTypeIdTools.findByName` report only
    what was explicitly registered, which is what lets a caller do :meth:`ModTypeIdTools.clear`
    followed by :meth:`ModTypeIdTools.registerModType` and get a registry holding *exactly* the mod
    types it asked for. Self-populating those lookups on first use would quietly break that
    )doc")

        .def_static("all", &AGRC::GlobalModTypes::all, py::doc(R"doc(
Retrieves every shipped :class:`CppModType`, freshly built on each call

Fresh rather than shared because a :class:`CppModType` owns mutable asset tables, so one caller
adding a hash must not be visible to every other one

Returns
-------
List[:class:`CppModType`]
    All the shipped mod types
        )doc"))

        .def_static("registerAll", &AGRC::GlobalModTypes::registerAll, py::doc(R"doc(
Files every mod type from :meth:`all` into :class:`ModTypeIdTools`'s global registry, so
:meth:`ModTypeIdTools.getModType` can resolve them by id and :meth:`ModTypeIdTools.findByName` by
name or alias

Idempotent: registering a mod type twice replaces the existing entry rather than duplicating it.
Note it registers *in addition to* whatever is already there rather than replacing the registry --
call :meth:`ModTypeIdTools.clear` first to start from empty
        )doc"));
}
