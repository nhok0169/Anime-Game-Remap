#ifndef AGRemapPyBind_PyResGroupCollect_H
#define AGRemapPyBind_PyResGroupCollect_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

#include "resEdits/PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/ResGroupCollect.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The core class the `pybind11`_-facing ``ResGroupCollect`` wraps
 */
using PyResGroupCollectCore = AGRC::ResGroupCollect<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::ResGroupCollect::GroupedResBuilder` -- the caller's
 own ``IniGroupedResBuilder`` :raw-html:`<br />` :raw-html:`<br />`

 Three things happen here rather than in the core, all for the same reason: the objects involved are
 `Python`_ ones. The builder instantiates a *user-supplied class*; the finished group is appended to
 the `Python`_ ``IniFile``'s own ``resources`` list; and each model is filed into the group's
 ``resources`` **dict** -- which for ``PyIniGroupedResource`` is a genuine `Python`_ ``dict`` keyed
 by whole mod-object tuples, not the typed ``std::string``-keyed map the C++
 :cpp:class:`IniGroupedResource` carries (see that class's own note)
 @endrst
 */
class PyGroupedResBuilder: public PyResGroupCollectCore::GroupedResBuilder {
    public:

        /**
         * @brief Constructs a builder wrapper
         *
         * @param builder The Python ``IniGroupedResBuilder``
         * @param ini The Python ``IniFile`` finished groups are appended to, or ``None``
         * @param ctx The context that knows which Python object each built model really is
         */
        PyGroupedResBuilder(py::object builder, py::object ini, PyIniResEditContext &ctx);

        AGRC::IniGroupedResource* build() override;
        void store(AGRC::IniGroupedResource &resource) override;
        void addResource(AGRC::IniGroupedResource &group, const PyResGroupCollectCore::GraphId &resType,
                          AGRC::IniResource &resource) override;

    private:
        py::object builder_;
        py::object ini_;
        PyIniResEditContext &ctx_;

        // Strong references to every group built, plus the pointer -> object map the two methods
        // above answer from -- the same bookkeeping PyIniGraphGroups keeps for graphs.
        py::list groupsKeepAlive_;
        std::unordered_map<const AGRC::IniGroupedResource*, py::object> groupHandles_;
};


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::ResGroupCollect`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 Keeps the **exact** `Python`_ objects given for every constructor argument and re-derives the C++
 members from them at the start of every ``edit`` (see #refresh), the same contract every other edit
 in this family keeps. The builder wrappers are rebuilt per call rather than once at construction:
 they capture the ``.ini`` file finished groups are appended to, which is only known per call
 @endrst
 */
class PyResGroupCollect: public PyResGroupCollectCore {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = PyResGroupCollectCore;

        /**
         * @brief The exact Python object given for ``resGroupTypes``
         */
        py::object resGroupTypesObj;

        /**
         * @brief The exact Python object given for ``srcRegs``
         */
        py::object srcRegsObj;

        /**
         * @brief The exact Python object given for ``resEdits``
         */
        py::object resEditsObj;

        /**
         * @brief The exact Python object given for ``groupedResBuilders``
         */
        py::object groupedResBuildersObj;

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
         * @brief The exact Python object given for ``trackKeys``
         */
        py::object trackKeysObj;

        /**
         * @brief The exact Python object given for ``keysToTrack``
         */
        py::object keysToTrackObj;

        /**
         * @brief Constructs a new resource-group-collecting edit
         *
         * @param resGroupTypes The unique names for the type of resource groups
         * @param srcRegs The registers that reference each resource
         * @param resEdits How each resource in a group is built
         * @param groupedResBuilders The builders for each type of grouped resource
         * @param partPredicates Which order indices to collect from
         * @param resPredicates Which references to collect
         * @param remaps Whether to remap the searched graphs, or ``None``
         * @param trackKeys Whether to track `KVPs`_ -- a ``bool`` or the granular nested dict
         * @param keysToTrack Which keys to track
         * @param resGroupTypesSameTopology Whether every resource type shares a graph topology across group types
         * @param id The unique id for this object, or ``None`` to autogenerate one
         */
        PyResGroupCollect(py::object resGroupTypes, py::object srcRegs, py::object resEdits, py::object groupedResBuilders,
                           py::object partPredicates, py::object resPredicates, py::object remaps, py::object trackKeys,
                           py::object keysToTrack, bool resGroupTypesSameTopology, const py::object &id);

        /**
         * @brief
         @rst
         Re-derives every inherited C++ member from its `Python`_ counterpart, and rebuilds the
         builder wrappers against the ``.ini`` file this call is for
         @endrst
         *
         * @param ctx The context for the call about to run, or ``nullptr`` when there is no .ini file
         */
        void refresh(PyIniResEditContext *ctx);

        /**
         * @brief
         @rst
         \ref resCalls rebuilt in the nested ``dict`` shape the pure-Python original exposed
         @endrst
         */
        py::object resCallsToPy() const;

    private:
        std::vector<std::unique_ptr<PyGroupedResBuilder>> builders_;
};


void initCppResGroupCollect(pybind11::module_ &m);

#endif
