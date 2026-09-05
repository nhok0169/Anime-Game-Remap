#include "PyGlobalModTypes.h"

#include <pybind11/stl.h>

#include "AGRemapCore/constants/GlobalModTypes.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppGlobalModTypes(pybind11::module_ &m) {
    py::class_<AGRC::GlobalModTypes>(m, "CppGlobalModTypes", R"doc(
Every :class:`ModType` the software ships with, and the one place that files them into
:class:`ModTypeIdTools`'s global registry

The counterpart to the pure-Python :class:`ModTypes` enum, whose ``getAll()`` likewise builds the
shipped mod types on demand

.. important::
    :meth:`registerAll` is **not** called automatically by anything in ``AGRemapCore``, and that is
    deliberate. :meth:`ModTypeIdTools.getModType` and :meth:`ModTypeIdTools.findByName` report only
    what was explicitly registered, which is what lets a caller do :meth:`ModTypeIdTools.clear`
    followed by :meth:`ModTypeIdTools.registerModType` and get a registry holding *exactly* the mod
    types it asked for. Self-populating those lookups on first use would quietly break that

    :meth:`registerMissing` **is** called automatically, by :meth:`GlobalIniClassifiers.classifier`
    -- but only ever to fill in ids nothing has registered, never to overwrite one a caller
    registered for itself
    )doc")

        .def_static("all", &AGRC::GlobalModTypes::all, py::doc(R"doc(
Retrieves every shipped :class:`ModType`, freshly built on each call

Fresh rather than shared because a :class:`ModType` owns mutable asset tables, so one caller
adding a hash must not be visible to every other one

Returns
-------
List[:class:`ModType`]
    All the shipped mod types
        )doc"))

        .def_static("registerAll", &AGRC::GlobalModTypes::registerAll, py::doc(R"doc(
Files every mod type from :meth:`all` into :class:`ModTypeIdTools`'s global registry, so
:meth:`ModTypeIdTools.getModType` can resolve them by id and :meth:`ModTypeIdTools.findByName` by
name or alias

Idempotent: registering a mod type twice replaces the existing entry rather than duplicating it.
Note it registers *in addition to* whatever is already there rather than replacing the registry --
call :meth:`ModTypeIdTools.clear` first to start from empty
        )doc"))

        .def_static("registerMissing", &AGRC::GlobalModTypes::registerMissing, py::doc(R"doc(
Files every shipped :class:`ModType` that is **not already registered** into
:class:`ModTypeIdTools`'s global registry, leaving every id the caller registered for itself alone

The difference from :meth:`registerAll` is only what happens on a collision: that one overwrites,
this one yields. This is what the implicit population behind :meth:`GlobalIniClassifiers.classifier`
uses, so that classifying a .ini file can no longer silently replace a :class:`ModType` you
registered under one of the shipped ids
        )doc"));
}
