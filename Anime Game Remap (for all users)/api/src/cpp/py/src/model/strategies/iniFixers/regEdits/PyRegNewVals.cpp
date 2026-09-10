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

    // A callable in any value slot is a ValProducer -- called as producer(modType) when 'edit'
    // runs, rather than being stringified the way every other object here is. 'modType' arrives by
    // capture, not through the core ValProducer's own parameter, for the same reason a ReplaceIf's
    // predicate does; see PyRegNewVals::refresh's doc comment.
    auto toNewVal = [&modType](const py::object &value) -> NewVal {
        if (PyCallable_Check(value.ptr())) {
            py::object producer = value;
            return NewVal(ValProducer([producer, modType](const AGRC::ModType *) {
                return py::str(producer(modType)).cast<std::string>();
            }));
        }

        return NewVal(py::str(value).cast<std::string>());
    };

    auto toNewVals = [&toNewVal](const std::vector<py::object> &raw) {
        std::vector<NewVal> out;
        out.reserve(raw.size());
        for (const py::object &v : raw) {
            out.push_back(toNewVal(v));
        }
        return out;
    };

    std::vector<std::pair<std::string, NewValSpec>> result;
    result.reserve(src.size());

    for (auto item : src) {
        std::string key = py::str(item.first).cast<std::string>();
        py::object value = py::reinterpret_borrow<py::object>(item.second);

        if (py::isinstance<PyReplaceList>(value)) {
            result.emplace_back(std::move(key), NewValSpec(toNewVals(value.cast<PyReplaceList>().values())));
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

            result.emplace_back(std::move(key), NewValSpec(std::pair<NewVal, ModTypePredicate>(toNewVal(spec.value()), std::move(boundPredicate))));
            continue;
        }

        result.emplace_back(std::move(key), NewValSpec(toNewVal(value)));
    }

    vals = std::move(result);
}


void initCppRegNewVals(pybind11::module_ &m) {
    py::class_<PyRegNewVals, PyBaseRegEdit, py::smart_holder> cls(m, "RegNewVals", R"doc(
This class inherits from :class:`BaseRegEdit`

Class for assigning new values to specific registers for some :class:`IfContentPart`

.. note::
    Both of the callbacks this class accepts get handed the :class:`ModType` being fixed, since a
    register edit always knows which one it is running for and deciding what to write based on
    that is the whole point of this class over a plain :meth:`IfContentPart.replaceVals` call:

    * a **new value** may be a callable, called as ``newVal(modType)`` to produce the value to
      write, and
    * a :class:`ReplaceIf` value's predicate is called as ``predicate(oldValue, modType)`` --
      one argument wider than every ``replaceVals`` calls it with, so a single-argument predicate
      will raise :class:`TypeError` when :meth:`edit` runs

Parameters
----------
vals: Dict[:class:`str`, Union[:class:`str`, Callable[[Optional[:class:`ModType`]], :class:`str`], :class:`ReplaceList`, :class:`ReplaceIf`]]
    Defines which registers will have their values changed :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names of the register and the values are the new values. Each value also
    accepts the richer forms :meth:`IfContentPart.replaceVals` takes -- a :class:`ReplaceList`
    (positional, by existing true left-to-right order) or a :class:`ReplaceIf` (conditional, by
    the wider predicate described above) :raw-html:`<br />` :raw-html:`<br />`

    Anywhere a new value is expected -- on its own, inside a :class:`ReplaceList`, or as a
    :class:`ReplaceIf`'s value -- a callable may be given instead, and is called as
    ``newVal(modType)`` when :meth:`edit` runs to produce the value :raw-html:`<br />` :raw-html:`<br />`

    eg. :raw-html:`<br />`
    ``{"ps-t1": "newVal", "ps-t2": lambda modType: f"{modType.name}Texture"}``

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
Dict[:class:`str`, Union[:class:`str`, Callable[[Optional[:class:`ModType`]], :class:`str`], :class:`ReplaceList`, :class:`ReplaceIf`]]: Defines which
registers will have their values changed, where the keys are the names of the register and the
values are the new values :raw-html:`<br />` :raw-html:`<br />`

Anywhere a new value is expected, a callable may be given instead and is called as
``newVal(modType)`` when :meth:`edit` runs
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
    The type of mod to fix. Passed through as the only argument to every callable new value in
    :attr:`vals`, and as the second argument to every :class:`ReplaceIf` predicate in it -- see
    this class's own note

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
