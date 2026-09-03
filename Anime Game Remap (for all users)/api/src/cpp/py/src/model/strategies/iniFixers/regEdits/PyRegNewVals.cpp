#include "PyRegNewVals.h"

#include <memory>
#include <utility>
#include <vector>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegNewVals::PyRegNewVals(py::object valsObj, bool addNewKVPs): Core({}, addNewKVPs), valsObj(std::move(valsObj)) {}

void PyRegNewVals::refresh(const py::object &modType) {
    // Deliberately NOT parseReplaceVals (PyIOrderedMultiMap.h), unlike PyRegRemap/PyRegRemove's
    // reuse of their own equivalents: that helper bakes a ReplaceIf's predicate down to the
    // single-argument IfContentPart::Predicate, and this class calls it with (oldValue, modType).
    // Everything else about the dispatch is identical to it, marker classes included.
    py::dict src = py::cast<py::dict>(valsObj);

    auto toIniVals = [](const std::vector<py::object> &raw) {
        std::vector<std::string> out;
        out.reserve(raw.size());
        for (const py::object &v : raw) {
            out.push_back(py::str(v).cast<std::string>());
        }
        return out;
    };

    std::vector<std::pair<std::string, NewValSpec>> result;
    result.reserve(src.size());

    for (auto item : src) {
        std::string key = py::str(item.first).cast<std::string>();
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyReplaceList>(value)) {
            result.emplace_back(std::move(key), NewValSpec(toIniVals(value.cast<PyReplaceList>().values())));
            continue;
        }

        if (py::isinstance<PyReplaceIf>(value)) {
            PyReplaceIf spec = value.cast<PyReplaceIf>();

            // predicateObj(), not predicate() -- the raw Python callable, so it can be invoked
            // with this class's own wider argument list. 'modType' is captured rather than read
            // from the ModTypePredicate's own parameter; see PyRegNewVals::refresh's doc comment.
            py::object predicate = spec.predicateObj();
            ModTypePredicate boundPredicate = [predicate, modType](const std::string &oldValue, const AGRC::ModType *) {
                return predicate(py::cast(oldValue), modType).cast<bool>();
            };

            result.emplace_back(std::move(key), NewValSpec(std::pair<std::string, ModTypePredicate>(py::str(spec.value()).cast<std::string>(), std::move(boundPredicate))));
            continue;
        }

        result.emplace_back(std::move(key), NewValSpec(py::str(value).cast<std::string>()));
    }

    vals = std::move(result);
}


void initCppRegNewVals(pybind11::module_ &m) {
    py::class_<PyRegNewVals, PyBaseRegEdit, py::smart_holder> cls(m, "RegNewVals", R"doc(
This class inherits from :class:`BaseRegEdit`

Class for assigning new values to specific registers for some :class:`IfContentPart`

.. note::
    A :class:`ReplaceIf` value's predicate is called as ``predicate(oldValue, modType)`` here --
    one argument wider than every ``replaceVals`` calls it with, since a register edit always
    knows which :class:`ModType` it is running for and deciding what to write based on that is
    the whole point of this class over a plain :meth:`IfContentPart.replaceVals` call. A
    single-argument predicate will raise :class:`TypeError` when :meth:`edit` runs

Parameters
----------
vals: Dict[:class:`str`, Union[:class:`str`, :class:`ReplaceList`, :class:`ReplaceIf`]]
    Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names of the register and the values are the new values. Each value also
    accepts the richer forms :meth:`IfContentPart.replaceVals` takes -- a :class:`ReplaceList`
    (positional, by existing true left-to-right order) or a :class:`ReplaceIf` (conditional, by
    the wider predicate described above)

addNewKVPs: :class:`bool`
    Whether to add new `KVPs`_ if the register keys do not exist in the :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``
    )doc");

    cls.def(py::init([](py::object vals, bool addNewKVPs) {
        return std::make_unique<PyRegNewVals>(std::move(vals), addNewKVPs);
    }), py::arg("vals"), py::arg("addNewKVPs") = false);

    cls.def_property("vals", [](const PyRegNewVals &self) {
        return self.valsObj;
    }, [](PyRegNewVals &self, py::object vals) {
        self.valsObj = std::move(vals);
    }, py::doc(R"doc(
Dict[:class:`str`, Union[:class:`str`, :class:`ReplaceList`, :class:`ReplaceIf`]]: Defines which
registers will have their values changed, where the keys are the names of the register and the
values are the new values
    )doc"));

    cls.def_readwrite("addNewKVPs", &PyRegNewVals::addNewKVPs, py::doc(R"doc(
:class:`bool`: Whether to add new `KVPs`_ if the register keys do not exist in the
:class:`IfContentPart`
    )doc"));

    bindRegEditEdit<PyRegNewVals>(cls, R"doc(
Assigns the new values in :attr:`vals` to 'part', by forwarding to
:meth:`IfContentPart.replaceVals`

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted. Unused by this edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Passed through as the second argument to every :class:`ReplaceIf`
    predicate in :attr:`vals` -- see this class's own note

modName: :class:`str`
    The name of the mod to fix to. Unused by this edit :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

partRanges: Optional[:class:`Ranges`]
    The ranges that indicate the valid order indices to process for the argument 'part' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`IfContentPart`
    The same part that was passed in, after editing
    )doc");
}
