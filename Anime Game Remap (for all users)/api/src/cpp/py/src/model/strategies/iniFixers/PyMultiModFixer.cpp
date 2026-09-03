#include "PyMultiModFixer.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include <pybind11/stl.h>

namespace py = pybind11;


PyMultiModFixer::PyMultiModFixer(Children children, py::object parser): Core(std::move(children), nullptr) {
    // Same shape as PyGIMIFixer's own constructor: the parser is held as a Python object, and the
    // .ini file is taken off it rather than out of a C++ pointer.
    this->parserObj = std::move(parser);
    if (!this->parserObj.is_none() && py::hasattr(this->parserObj, "_iniFile")) {
        this->iniFileObj = this->parserObj.attr("_iniFile");
    }
}


std::vector<int> PyMultiModFixer::selectedChildIds() const {
    std::optional<std::unordered_set<int>> filter;

    // Read through genuine Python attribute lookup -- see this method's note in the header. A
    // missing attribute is treated the same as None: no filter.
    if (!iniFileObj.is_none() && py::hasattr(iniFileObj, "filteredToModTypeIds")) {
        py::object value = iniFileObj.attr("filteredToModTypeIds");
        if (!value.is_none()) {
            filter = value.cast<std::unordered_set<int>>();
        }
    }

    std::vector<int> result;
    result.reserve(getChildren().size());

    for (const auto &entry : getChildren()) {
        if (entry.second == nullptr) {
            continue;
        }

        if (filter.has_value() && filter->count(entry.first) == 0) {
            continue;
        }

        result.push_back(entry.first);
    }

    // Left in insertion order, matching the core implementation this overrides -- Children is a
    // tsl::ordered_map so the caller decides which child holds the .ini file's first and last word.
    // This used to sort ascending, back when the core did too; sorting here now would make the same
    // class order its children differently depending on which side of the binding you used it from.
    return result;
}


namespace {

    // Children cross this boundary one entry at a time rather than through a cast<Children>().
    // Children's value type is shared_ptr<BaseIniFixer<std::string, std::string>> -- the CORE
    // instantiation, which is not a registered pybind11 type. Only PyBaseIniFixer is, so the cast
    // has to name that and rely on the derived-to-base shared_ptr conversion afterwards.
    PyMultiModFixer::Children parseChildren(const py::object &children) {
        PyMultiModFixer::Children result;
        if (children.is_none()) {
            return result;
        }

        for (auto item : children.cast<py::dict>()) {
            int modTypeId = item.first.cast<int>();

            if (item.second.is_none()) {
                result.emplace(modTypeId, nullptr);
                continue;
            }

            result.emplace(modTypeId, item.second.cast<std::shared_ptr<PyBaseIniFixer>>());
        }

        return result;
    }


    py::dict childrenToPy(const PyMultiModFixer::Children &children) {
        py::dict result;

        for (const auto &entry : children) {
            // dynamic_pointer_cast rather than static: a child could in principle be a core-side
            // fixer with no Python object behind it, and there would be nothing to hand back for
            // it. Every child a Python caller supplied is a PyBaseIniFixer.
            std::shared_ptr<PyBaseIniFixer> asPy = std::dynamic_pointer_cast<PyBaseIniFixer>(entry.second);
            result[py::cast(entry.first)] = asPy == nullptr ? py::none() : py::cast(asPy);
        }

        return result;
    }

}


void initCppMultiModFixer(pybind11::module_ &m) {
    // Plain "MultiModFixer": the pure-Python class of that name has been renamed to
    // MultiModFixerOld, which frees this one -- the same GIMIFixer / GIMIFixerOld arrangement.
    py::class_<PyMultiModFixer, PyBaseIniFixer, py::smart_holder>(m, "MultiModFixer", R"doc(
This class inherits from :class:`BaseIniFixer`

A fixer that owns no fixing logic of its own and instead **delegates to one child fixer per mod
type**, keyed by the :class:`int` value of that mod type's :class:`ModTypeId`

Which children run is decided by the .ini file's :attr:`IniFile.filteredToModTypeIds`: a child
whose id is absent from that filter is skipped. ``None`` there means no filter, so every child
runs -- an **empty set** is deliberately different, and selects nothing

.. note::
    A child is a plain :class:`BaseIniFixer`, and this class *is* one, so a :class:`MultiModFixer`
    can be a child of another and the nesting composes. The :class:`IniFixingContext` handed to a
    child is **narrowed** rather than replaced, so a nested fixer's first child holds the .ini
    file's first word only if its parent was told the same

.. note::
    Children run in **ascending id order**, not the order of the dict they were given in. Only one
    child may take the .ini file's backup and only one may hide the original mod, so which is
    "first" and which is "last" has to be answerable the same way twice

Parameters
----------
children: Optional[Dict[:class:`int`, :class:`BaseIniFixer`]]
    The child fixers, keyed by the :class:`int` value of the :class:`ModTypeId` each one fixes. A
    ``None`` child is skipped rather than treated as an error :raw-html:`<br />`
    :raw-html:`<br />`

    **Default**: ``None``, meaning no children -- which makes fixing a no-op

parser: Optional[:class:`BaseIniParser`]
    The parser to retrieve the data to fix from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc")

        .def(py::init([](const py::object &children, py::object parser) {
            // Built fresh per call from py::none() rather than defaulted to a dict literal --
            // py::arg("x") = <mutable container> is pybind11's version of Python's
            // mutable-default-argument bug, and every caller would share the one instance.
            return std::make_unique<PyMultiModFixer>(parseChildren(children), std::move(parser));
        }), py::arg("children") = py::none(), py::arg("parser") = py::none())

        .def_property("children",
                       [](const PyMultiModFixer &self) { return childrenToPy(self.getChildren()); },
                       [](PyMultiModFixer &self, const py::object &children) {
                           self.setChildren(parseChildren(children));
                       }, py::doc(R"doc(
Dict[:class:`int`, :class:`BaseIniFixer`]: The child fixers, keyed by mod type id
        )doc"));
}
