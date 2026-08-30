#ifndef AGRemapPyBind_PyResRegCollect_H
#define AGRemapPyBind_PyResRegCollect_H

#include <pybind11/pybind11.h>

#include "resEdits/PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResRegCollect.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::ResRegCollect`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Keeps the **exact** `Python`_ objects given for every constructor argument and re-derives the C++
 members from them at the start of every ``edit`` (see #refresh) -- the same
 identity/in-place-mutation contract every other edit in this family keeps, and load-bearing here
 for two of them in particular: ``resEdits``' values are the caller's own resource-edit objects
 (whose per-run state ``clear()`` resets), and ``partPredicates``/``resPredicates`` hold `Python`_
 callables, which a ``std::function`` round trip cannot hand back as the same callable
 @endrst
 */
class PyResRegCollect: public AGRC::ResRegCollect<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::ResRegCollect<py::object, py::object, PyObjectHash, PyObjectEqual>;

        /**
         * @brief The exact Python object given for ``srcRegs``
         */
        py::object srcRegsObj;

        /**
         * @brief The exact Python object given for ``resEdits``
         */
        py::object resEditsObj;

        /**
         * @brief The exact Python object given for ``partPredicates``
         */
        py::object partPredicatesObj;

        /**
         * @brief The exact Python object given for ``resPredicates``
         */
        py::object resPredicatesObj;

        /**
         * @brief The exact Python object given for ``remaps``, or ``None``
         */
        py::object remapsObj;

        /**
         * @brief The exact Python object given for ``trackKeys`` -- a ``bool`` or the granular per-graph dict
         */
        py::object trackKeysObj;

        /**
         * @brief The exact Python object given for ``keysToTrack``
         */
        py::object keysToTrackObj;

        /**
         * @brief Constructs a new resource-collecting edit
         *
         * @param srcRegs The registers that reference the resource, keyed by which graph to search
         * @param resEdits How each resource subtype is built
         * @param partPredicates Which order indices to collect from, keyed by graph
         * @param resPredicates Which references to collect, keyed by graph
         * @param remaps Whether to remap the searched graphs, or ``None``
         * @param trackKeys Whether to track `KVPs`_ -- a ``bool`` or the granular per-graph dict
         * @param keysToTrack Which keys to track, keyed by graph
         */
        PyResRegCollect(py::object srcRegs, py::object resEdits, py::object partPredicates, py::object resPredicates,
                         py::object remaps, py::object trackKeys, py::object keysToTrack);

        /**
         * @brief
         @rst
         Re-derives every inherited C++ member from its `Python`_ counterpart -- called at the start
         of every ``edit``, so an in-place mutation of any of them is honoured
         @endrst
         */
        void refresh();

        /**
         * @brief
         @rst
         \ref resCalls rebuilt in the nested ``dict`` shape the pure-Python original exposed
         @endrst
         */
        py::object resCallsToPy() const;
};


void initCppResRegCollect(pybind11::module_ &m);

#endif
