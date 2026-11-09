// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyRegIfTemplateAdd.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "../regEdits/PyBaseRegEdit.h"  // PyPartRanges, for a partFilter's return value
#include "../../../iftemplate/PyIfTemplate.h"


namespace {
// Wraps a Python partFilter callable as the core PartFilter -- same shape, and same reason, as
// PyRegBottomAdd.cpp's parsePartFilter.
PyRegIfTemplateAdd::PartFilter parsePartFilter(const py::object &partFilter, py::object modType) {
    if (partFilter.is_none() || !PyCallable_Check(partFilter.ptr())) {
        return {};
    }
    py::object heldFilter = partFilter;
    py::object heldModType = std::move(modType);
    return [heldFilter, heldModType](const PyRegIfTemplateAdd::Base::IterData &iterData, const AGRC::ModType *,
                                     AGRC::IniFile *) -> PyRegIfTemplateAdd::Base::OrderRanges {
        py::object result = heldFilter(py::cast(&iterData, py::return_value_policy::reference), heldModType, py::none());
        PyPartRanges ranges(result);
        const PyRegIfTemplateAdd::Base::OrderRanges *parsedRanges = ranges.get();
        if (parsedRanges == nullptr) {
            throw py::type_error("A RegIfTemplateAdd partFilter must return a Ranges (or a list of (start, end) bounds), not None");
        }
        return *parsedRanges;
    };
}
}


namespace {
    // a Python object kept alive by its edit (BaseIniPartEdit::owned), let go of with the GIL held
    std::shared_ptr<void> holdPy(py::object object) {
        return std::shared_ptr<void>(new py::object(std::move(object)), [](void *held) {
            py::gil_scoped_acquire gil;
            delete static_cast<py::object *>(held);
        });
    }

    // the section the edit adds, kept alive by the edit
    void setIfTemplate(PyRegIfTemplateAdd &edit, const py::object &ifTemplate) {
        edit.owned.clear();
        edit.ifTemplate = ifTemplate.is_none() ? nullptr : ifTemplate.cast<PyIfTemplate *>();
        if (!ifTemplate.is_none()) {
            edit.owned.push_back(holdPy(ifTemplate));
        }
    }

    // a section filter given in Python, kept as itself so 'sectionFilter' reads it back
    struct PySectionFilter {
        py::object filter;
        bool operator()(const std::string &name, const PyIfTemplate &section) const {
            py::gil_scoped_acquire gil;
            return filter(name, py::cast(&section, py::return_value_policy::reference)).cast<bool>();
        }
    };
}


void initCppRegIfTemplateAdd(pybind11::module_ &m) {
    py::class_<PyRegIfTemplateAdd, PyBaseIniGraphEdit, py::smart_holder> cls(m, "RegIfTemplateAdd", R"doc(
This class inherits from :class:`BaseIniGraphEdit`

Adds the parts of an :class:`IfTemplate` --- `KVPs`_ and whole ``if`` / ``else`` blocks --- into every
root `section`_ of a graph, at the :attr:`position` given

Where :class:`RegAdd` adds `KVPs`_ to parts that already exist and :class:`RegBottomAdd` appends one
block at the bottom, this one inserts a structure the caller wrote, wherever the fix needs it to run
--- eg. a constant buffer bound per pass BEFORE the `section`_'s draws:

.. code-block:: ini

    [TextureOverrideCharacterLegs]
    hash = a913e9a9
    match_first_index = 0
    ; position = AfterKey, key = match_first_index: the template's parts go HERE
    if ps == 51840.1
        ps-cb3 = ResourceLegsMainCb3
    else if ps == 51840.2
        ps-cb3 = ResourceLegsOutlineCb3
    endif
    ib = ResourceLegsIB
    drawindexed = auto

The template's parts are copied in, so one template serves every root and every graph. Its ``if``
lines are re-parsed under the graph's own :class:`Z3Context`, and its `KVPs`_ keep their nesting,
shifted to the depth they are inserted at. A template is most easily written as text and parsed:
the one `section`_ of an :class:`IniFile` made from it

.. note::
    At the bottom, a namespace-merged `section`_ (one guarded as a whole by
    ``if $\<Char>\Master\swapvar == n``) takes the parts inside its guard, as :class:`RegBottomAdd`
    does. With :attr:`Position.AfterKey` or :attr:`Position.BeforeTopKey`, a `section`_ with no
    :attr:`key` is left alone, as is one with no top-level ``if`` under :attr:`Position.BeforeFirstIf`.
    :attr:`everySection` and :attr:`sectionFilter` choose which `sections`_ are added to

Parameters
----------
ifTemplate: Optional[:class:`IfTemplate`]
    The template whose parts are added. ``None`` makes the edit a no-op

position: :class:`RegIfTemplateAdd.Position`
    Where in each root the parts go :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``RegIfTemplateAdd.Position.Bottom``

key: :class:`str`
    The key :attr:`Position.AfterKey` inserts after, and :attr:`Position.BeforeTopKey` before
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``
    )doc");

    py::enum_<PyRegIfTemplateAdd::Position>(cls, "Position", R"doc(
Where in each root :class:`RegIfTemplateAdd` puts the template's parts
    )doc")
        .value("Top", PyRegIfTemplateAdd::Position::Top, "Before everything the `section`_ has")
        .value("Bottom", PyRegIfTemplateAdd::Position::Bottom, "After everything the `section`_ has, at its own depth (inside a namespace-merge guard)")
        .value("AfterKey", PyRegIfTemplateAdd::Position::AfterKey, "Right after the first `KVP`_ whose key is :attr:`RegIfTemplateAdd.key`, at that `KVP`_'s depth")
        .value("BeforeTopKey", PyRegIfTemplateAdd::Position::BeforeTopKey, "Right before the first `KVP`_ at the `section`_'s top level (outside every ``if``) whose key is :attr:`RegIfTemplateAdd.key`")
        .value("BeforeFirstIf", PyRegIfTemplateAdd::Position::BeforeFirstIf, "Right before the `section`_'s first top-level ``if``");

    cls.def(py::init([](py::object ifTemplate, PyRegIfTemplateAdd::Position position, std::string key,
                        std::optional<std::string> value, bool everySection, py::object sectionFilter) {
        // every setting of the edit can be given here, as the property of the same name
        auto edit = std::make_unique<PyRegIfTemplateAdd>(nullptr, position, std::move(key));
        setIfTemplate(*edit, ifTemplate);
        edit->value = std::move(value);
        edit->everySection = everySection;
        if (!sectionFilter.is_none()) {
            edit->sectionFilter = PyRegIfTemplateAdd::SectionFilter(PySectionFilter{sectionFilter});
        }
        return edit;
    }), py::arg("ifTemplate") = py::none(), py::arg("position") = PyRegIfTemplateAdd::Position::Bottom, py::arg("key") = "",
        py::arg("value") = py::none(), py::arg("everySection") = false, py::arg("sectionFilter") = py::none());

    cls.def_property("ifTemplate", [](const PyRegIfTemplateAdd &self) {
                         return self.ifTemplate == nullptr ? py::none() : py::cast(self.ifTemplate, py::return_value_policy::reference);
                     },
                     [](PyRegIfTemplateAdd &self, py::object v) { setIfTemplate(self, v); }, py::doc(R"doc(
Optional[:class:`IfTemplate`]: The template whose parts are added
    )doc"));

    cls.def_readwrite("position", &PyRegIfTemplateAdd::position, py::doc(R"doc(
:class:`RegIfTemplateAdd.Position`: Where in each root the parts go
    )doc"));

    cls.def_readwrite("key", &PyRegIfTemplateAdd::key, py::doc(R"doc(
:class:`str`: The key :attr:`Position.AfterKey` inserts after, and :attr:`Position.BeforeTopKey` before
    )doc"));

    cls.def_readwrite("value", &PyRegIfTemplateAdd::value, py::doc(R"doc(
Optional[:class:`str`]: The value the :attr:`key` `KVP`_ must have for :attr:`Position.AfterKey` and
:attr:`Position.BeforeTopKey` to take it. ``None`` (the default) takes any value
    )doc"));

    cls.def_readwrite("everySection", &PyRegIfTemplateAdd::everySection, py::doc(R"doc(
:class:`bool`: Whether the parts go into EVERY `section`_ of the graph -- the ones its roots call
included -- rather than only its roots. **Default**: ``False``
    )doc"));

    cls.def_property("sectionFilter", [](const PyRegIfTemplateAdd &self) -> py::object {
                         const PySectionFilter *own = self.sectionFilter ? self.sectionFilter.target<PySectionFilter>() : nullptr;
                         return own != nullptr ? own->filter : (self.sectionFilter ? py::cast(self.sectionFilter) : py::none());
                     },
                     [](PyRegIfTemplateAdd &self, py::object v) {
                         self.sectionFilter = v.is_none() ? PyRegIfTemplateAdd::SectionFilter() : PyRegIfTemplateAdd::SectionFilter(PySectionFilter{v});
                     }, py::doc(R"doc(
Optional[Callable[[:class:`str`, :class:`IfTemplate`], :class:`bool`]]: Which `sections`_ the edit adds
to, asked of each with its name: one it answers ``False`` for is left alone. ``None`` (the default) takes
every `section`_. Asked only of a `section`_ that has a place for the parts
    )doc"));

    cls.def("edit", [](PyRegIfTemplateAdd &self, py::object graph, const py::object &modType,
                       const std::string &modName, const py::object &partFilter, bool trackKeys,
                       const py::object &keysToTrack) {
        (void)keysToTrack;
        PyIniSectionGraph &parsedGraph = parseGraphArg(graph);
        self.edit(parsedGraph, nullptr, modName, parsePartFilter(partFilter, modType), trackKeys, std::nullopt);
        // brand-new parts the graph's Python-side keep-alive has never seen, as RegBottomAdd's
        parsedGraph.refreshKeepAlive();
        return graph;
    }, py::arg("graph"), py::arg("modType"), py::arg("modName") = "", py::arg("partFilter") = py::none(),
       py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(),
       py::doc(R"doc(
Adds the parts of :attr:`ifTemplate` into every root `section`_ of 'graph'

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
    Asked about each root's first :class:`IfContentPart`; a root it rejects is left alone
    :raw-html:`<br />` :raw-html:`<br />`

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
    The same graph that was passed in, after editing
    )doc"));
}
