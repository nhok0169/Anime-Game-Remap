#ifndef AGRemapPyBind_PyGraphGroupEdit_H
#define AGRemapPyBind_PyGraphGroupEdit_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/GraphGroupEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GraphGroupEdit`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Keeps the **exact** `Python`_ objects given for
 ``edits``/``trackKeys``/``keysToTrack``/``keyFilters`` and re-derives the C++ members from them at
 the start of every ``edit`` (see #refresh) -- the same identity/in-place-mutation contract every
 other edit in this family keeps. Here it is doubly necessary: the values inside ``edits`` are the
 caller's own edit objects (some C++-backed, some pure `Python`_) and the values inside
 ``keyFilters`` are `Python`_ callables, neither of which survives a round trip through a parsed
 C++ member
 @endrst
 */
class PyGraphGroupEdit: public AGRC::GraphGroupEdit<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GraphGroupEdit<std::string, std::string>;

        /**
         * @brief The exact Python object given for ``edits``
         */
        py::object editsObj;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``trackKeys`` -- either a plain ``bool`` (applies to
         every graph) or the granular per-``.ini``-file list
         @endrst
         */
        py::object trackKeysObj;

        /**
         * @brief The exact Python object given for ``keysToTrack``
         */
        py::object keysToTrackObj;

        /**
         * @brief The exact Python object given for ``keyFilters``
         */
        py::object keyFiltersObj;

        /**
         * @brief Constructs a new graph-group edit
         *
         * @param editsObj The Python edits, one entry per .ini file
         * @param trackKeysObj The Python key-tracking flag(s)
         * @param keysToTrackObj The Python keys to track
         * @param keyFiltersObj The Python part filters
         */
        PyGraphGroupEdit(py::object editsObj, py::object trackKeysObj, py::object keysToTrackObj, py::object keyFiltersObj);

        /**
         * @brief
         @rst
         Re-derives every inherited C++ member from its `Python`_ counterpart, building one
         ``PartEdit`` adapter per edit object :raw-html:`<br />` :raw-html:`<br />`

         The adapters live until the next #refresh, which is long enough: they are only ever
         reached from inside the ``edit`` call that built them
         @endrst
         *
         * @param groups The graph-groups view the current edit is running against -- adapters adopt any graph a Python edit returns into it
         * @param ini The ``ini`` the current edit was called with (``None`` for plain ``edit``)
         * @param modType The ``modType`` the current edit was called with
         */
        void refresh(PyIniGraphGroups &groups, py::object ini, py::object modType);

        /**
         * @brief
         @rst
         The original `Python`_ callable a parsed part filter was built from, or the shared
         "everything is valid" default when there is none :raw-html:`<br />` :raw-html:`<br />`

         Keyed by the address of the stored ``std::function`` -- see
         :cpp:func:`AGRemapCore::GraphGroupEdit::PartEdit::editGraph`'s own note on why the address
         is what makes this recoverable at all
         @endrst
         *
         * @param keyFilter The parsed part filter to look up, or ``nullptr``
         */
        py::object partFilterToPy(const PartFilter *keyFilter) const;

    private:
        std::unordered_map<const void*, py::object> partFilterObjs_;
        std::vector<std::unique_ptr<PartEdit>> partEdits_;
};


void initCppGraphGroupEdit(pybind11::module_ &m);

#endif
