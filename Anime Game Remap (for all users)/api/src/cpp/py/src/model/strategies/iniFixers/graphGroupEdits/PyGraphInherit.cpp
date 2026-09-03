#include "PyGraphInherit.h"

#include <memory>
#include <utility>

#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the same "a bound Ranges, or a raw
                                        // list of bounds" resolver every reg edit already uses)


PyGraphInherit::PyGraphInherit(py::object srcObj, py::object dstObj, std::string reg, bool latest, py::object partFilterObj):
    Core(GraphId(), GraphId(), std::move(reg), latest, {}),
    srcObj(std::move(srcObj)), dstObj(std::move(dstObj)), partFilterObj(std::move(partFilterObj)) {}


void PyGraphInherit::refresh(const py::object &modType) {
    src = parseGraphId(srcObj);
    dst = parseGraphId(dstObj);

    if (partFilterObj.is_none()) {
        partFilter = {};
        return;
    }

    py::object filterObj = partFilterObj;

    // 'modType' is captured from the Python side rather than travelling through the C++ signature:
    // the core takes a nullable AGRC::ModType*, and the Python-side ModType is an unrelated
    // pure-Python class with nothing castable to hand over. The third argument is always None --
    // GraphInherit's own edit() passes no .ini file, exactly as the pure-Python original did.
    partFilter = [filterObj, modType](const IterData &iterData, const AGRC::ModType *, AGRC::IniFile *) -> OrderRanges {
        py::object result = filterObj(py::cast(&iterData, py::return_value_policy::reference), modType, py::none());

        PyPartRanges parsed(result);
        const OrderRanges *ranges = parsed.get();
        if (ranges == nullptr) {
            throw py::type_error("A GraphInherit partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }

        return *ranges;
    };
}


void initCppGraphInherit(pybind11::module_ &m) {
    py::class_<PyGraphInherit, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphInherit", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Merges the graph at 'dst' into the graph at 'src', by inserting consecutive `KVPs`_ into 'src' that
reference every root `section`_ of the graph at 'dst'

.. note::
    This only inserts the reference `KVPs`_ into 'src' -- the `sections`_ of 'dst' themselves are
    left untouched (and still need to be reachable/present elsewhere for the reference to resolve,
    the same way a plain ``run =`` reference to another `section`_ works)

.. note::
    If either the graph at 'src' or the graph at 'dst' cannot be found, nothing is inserted and the
    original 'graphGroups' is returned as-is -- no exception is raised

Parameters
----------
src: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the source :class:`IniSectionGraph` to insert the reference `KVPs`_ into. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

dst: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the :class:`IniSectionGraph` to merge into 'src'. Same tuple format as 'src'

reg: :class:`str`
    The name of the register used to reference the root `sections`_ of the graph at 'dst'

latest: :class:`bool`
    Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the front :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
    The filter used to indicate which areas of some :class:`IfContentPart` within the graph at 'src'
    are valid to insert the `KVPs`_ :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then the `KVPs`_ are instead inserted directly at the very
    front/back (based on 'latest') of every root `section`_ of the graph at 'src', instead of being
    filtered through every :class:`IfContentPart` of the graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members, and
    // a factory returning a unique_ptr avoids ever needing to move-construct the class itself --
    // see PyRegAdd.cpp's identical note.
    cls.def(py::init([](py::object src, py::object dst, std::string reg, bool latest, py::object partFilter) {
        return std::make_unique<PyGraphInherit>(std::move(src), std::move(dst), std::move(reg), latest, std::move(partFilter));
    }), py::arg("src"), py::arg("dst"), py::arg("reg"), py::arg("latest") = true, py::arg("partFilter") = py::none());

    cls.def_property("src", [](const PyGraphInherit &self) {
        return self.srcObj;
    }, [](PyGraphInherit &self, py::object src) {
        self.srcObj = std::move(src);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the source :class:`IniSectionGraph` to
insert the reference `KVPs`_ into
    )doc"));

    cls.def_property("dst", [](const PyGraphInherit &self) {
        return self.dstObj;
    }, [](PyGraphInherit &self, py::object dst) {
        self.dstObj = std::move(dst);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the :class:`IniSectionGraph` to merge
into :attr:`src`
    )doc"));

    cls.def_readwrite("reg", &PyGraphInherit::reg, py::doc(R"doc(
:class:`str`: The name of the register used to reference the root `sections`_ of the graph at
:attr:`dst`
    )doc"));

    cls.def_readwrite("latest", &PyGraphInherit::latest, py::doc(R"doc(
:class:`bool`: Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the
front
    )doc"));

    cls.def_property("partFilter", [](const PyGraphInherit &self) {
        return self.partFilterObj;
    }, [](PyGraphInherit &self, py::object partFilter) {
        self.partFilterObj = std::move(partFilter);
    }, py::doc(R"doc(
Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]:
The filter used to indicate which areas of some :class:`IfContentPart` within the graph at
:attr:`src` are valid to insert the `KVPs`_
    )doc"));

    cls.def("edit", [](PyGraphInherit &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        self.refresh(modType);

        PyIniGraphGroups groups(graphGroups);
        // nullptr for modType: the real Python object is captured by the rebuilt partFilter
        // instead -- see PyGraphInherit::refresh.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Inserts the reference `KVPs`_ from the graph at :attr:`dst` into the graph at :attr:`src`

With no :attr:`partFilter`, the `KVPs`_ go straight to the very front/back (based on :attr:`latest`)
of every root `section`_ of the graph at :attr:`src`. With one, they instead go at the
earliest/latest valid index of every :class:`IfContentPart` the filter accepts

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix. Only ever handed to :attr:`partFilter`

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));
}
