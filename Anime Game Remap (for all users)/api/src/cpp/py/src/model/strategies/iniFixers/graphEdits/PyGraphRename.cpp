#include "PyGraphRename.h"

#include <memory>
#include <string>
#include <utility>


PyGraphRename::PyGraphRename(py::object renameFuncObj): Core({}), renameFuncObj(std::move(renameFuncObj)) {}


void PyGraphRename::refresh() {
    if (renameFuncObj.is_none() || !PyCallable_Check(renameFuncObj.ptr())) {
        renameFunc = {};
        return;
    }

    // Captures the Python object by value: the resulting std::function only ever runs synchronously
    // inside a binding call, so the GIL is always held when it is invoked.
    py::object func = renameFuncObj;
    renameFunc = [func](const std::string &name) {
        return py::str(func(name)).cast<std::string>();
    };
}


void initCppGraphRename(pybind11::module_ &m) {
    py::class_<PyGraphRename, PyBaseIniGraphEdit, py::smart_holder> cls(m, "GraphRename", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Renames the `sections`_ of some caller/callee graph of :class:`IniSectionGraph`

Parameters
----------
renameFunc: Callable[[:class:`str`], :class:`str`]
    Function used to rename a `section`_. The function takes in the name of the old `section`_ and
    returns the new name for the `section`_
    )doc");

    // py::init(factory) rather than py::init<py::object>(): the core class owns std::function
    // members through its own and its base's typedefs, and a factory returning a unique_ptr avoids
    // ever needing to move-construct the class itself -- see PyGraphRemove.cpp's identical note.
    cls.def(py::init([](py::object renameFunc) {
        return std::make_unique<PyGraphRename>(std::move(renameFunc));
    }), py::arg("renameFunc"));

    cls.def_property("renameFunc", [](const PyGraphRename &self) {
        return self.renameFuncObj;
    }, [](PyGraphRename &self, py::object renameFunc) {
        self.renameFuncObj = std::move(renameFunc);
    }, py::doc(R"doc(
Callable[[:class:`str`], :class:`str`]: Function used to rename a `section`_. The function takes in
the name of the old `section`_ and returns the new name for the `section`_
    )doc"));

    cls.def("edit", [](PyGraphRename &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        self.refresh();

        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);

        // The C++ core takes 'modType' as a nullable ModType*, and the Python-side ModType is a
        // pure-Python class with no C++ counterpart to cast to, so nullptr is the only honest thing
        // to pass -- this edit never reads it anyway. 'partFilter' is ignored for the same reason
        // the pure-Python original ignored it: renaming every section has no per-part window.
        (void)modType;
        (void)partFilter;
        (void)trackKeys;
        (void)keysToTrack;
        self.Core::edit(parsedGraph, nullptr, modName);

        // IniSectionGraph.rename()'s own binding does this too -- renaming relabels the sections
        // this graph's Python-side keep-alive tracks, so its keys have to be resynced.
        parsedGraph.refreshKeepAlive();

        // Returns the original Python object rather than py::cast()-ing the C++ reference back, so
        // 'result is graph' holds (matching the pure-Python original's plain 'return graph').
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Renames every `section`_ of 'graph' by :attr:`renameFunc`

Every ``run =`` reference to a renamed `section`_ is rewritten too, and the graph is rebuilt -- so a
rename never leaves a dangling caller/callee edge behind

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph to edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partFilter: Optional[Callable[[:class:`SectionIterData`, Optional[:class:`ModType`], Optional[:class:`IniFile`]], :class:`Ranges`]]
    The filter used to indicate the valid order indices to process some :class:`IfContentPart` in
    the graph. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: :class:`bool`
    The caller's key-tracking default. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Set[:class:`str`]]
    The caller's key-tracking key set. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IniSectionGraph`
    The same graph that was passed in, after every `section`_ was renamed
    )doc"));
}
