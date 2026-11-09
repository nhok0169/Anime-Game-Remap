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
#include "../PyEditScope.h"

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <variant>

#include "AGRemapCore/model/strategies/ModType.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {
    using NewVal = PyRegNewVals::NewVal;
    using NewValSpec = PyRegNewVals::NewValSpec;

    // A Python callable as each kind of C++ value function, kept as itself so 'vals' reads back the callable given. Each
    // is handed the mod type the edit runs for, as Python sees it
    struct PyValProducer {
        py::object producer;
        std::string operator()(const AGRC::ModType *modType) const {
            py::gil_scoped_acquire gil;
            return py::str(producer(editModTypeToPy(modType))).cast<std::string>();
        }
    };

    struct PyOldValProducer {
        py::object producer;
        std::string operator()(const std::string &oldValue, const AGRC::ModType *modType) const {
            py::gil_scoped_acquire gil;
            return py::str(producer(py::cast(oldValue), editModTypeToPy(modType))).cast<std::string>();
        }
    };

    struct PyPredicate {
        py::object predicate;
        bool operator()(const std::string &oldValue, const AGRC::ModType *modType) const {
            py::gil_scoped_acquire gil;
            return predicate(py::cast(oldValue), editModTypeToPy(modType)).cast<bool>();
        }
    };

    // One value as the edit holds it: a FromOldVal's callable is handed the old value too, any other callable is a
    // producer, and anything else is its text
    NewVal newValOf(const py::object &value) {
        if (py::isinstance<PyFromOldVal>(value)) {
            return NewVal(PyRegNewVals::OldValProducer(PyOldValProducer{value.cast<PyFromOldVal>().producer()}));
        }
        if (PyCallable_Check(value.ptr())) {
            return NewVal(PyRegNewVals::ValProducer(PyValProducer{value}));
        }
        return NewVal(py::str(value).cast<std::string>());
    }

    // 'vals' as the edit holds it: register -> its value, a ReplaceList of values, or a ReplaceIf
    std::vector<std::pair<std::string, NewValSpec>> parseNewVals(const py::object &vals) {
        std::vector<std::pair<std::string, NewValSpec>> result;
        for (auto item : py::cast<py::dict>(vals)) {
            std::string key = py::str(item.first).cast<std::string>();
            py::object value = py::reinterpret_borrow<py::object>(item.second);
            if (py::isinstance<PyReplaceList>(value)) {
                std::vector<NewVal> list;
                for (const py::object &entry : value.cast<PyReplaceList>().values()) {
                    list.push_back(newValOf(entry));
                }
                result.emplace_back(std::move(key), NewValSpec(std::move(list)));
            } else if (py::isinstance<PyReplaceIf>(value)) {
                PyReplaceIf spec = value.cast<PyReplaceIf>();
                result.emplace_back(std::move(key), NewValSpec(std::pair<NewVal, PyRegNewVals::ModTypePredicate>(
                                                        newValOf(spec.value()), PyPredicate{spec.predicateObj()})));
            } else {
                result.emplace_back(std::move(key), NewValSpec(newValOf(value)));
            }
        }
        return result;
    }

    // a value back as Python: its text, the callable it was given (as a FromOldVal for an old-value one), or the C++
    // function itself for one built in C++
    py::object newValToPy(const NewVal &value) {
        if (const auto *text = std::get_if<std::string>(&value)) {
            return py::str(*text);
        }
        if (const auto *producer = std::get_if<PyRegNewVals::ValProducer>(&value)) {
            const PyValProducer *own = producer->target<PyValProducer>();
            return own != nullptr ? own->producer : py::cast(*producer);
        }
        const auto &producer = std::get<PyRegNewVals::OldValProducer>(value);
        const PyOldValProducer *own = producer.target<PyOldValProducer>();
        return py::cast(PyFromOldVal(own != nullptr ? own->producer : py::object(py::cast(producer))));
    }

    py::dict newValsToPy(const PyRegNewVals &edit) {
        py::dict result;
        for (const auto &[key, spec] : edit.vals) {
            if (const auto *one = std::get_if<NewVal>(&spec)) {
                result[py::str(key)] = newValToPy(*one);
            } else if (const auto *list = std::get_if<std::vector<NewVal>>(&spec)) {
                std::vector<py::object> values;
                for (const NewVal &entry : *list) {
                    values.push_back(newValToPy(entry));
                }
                result[py::str(key)] = py::cast(PyReplaceList(std::move(values)));
            } else {
                const auto &[value, predicate] = std::get<std::pair<NewVal, PyRegNewVals::ModTypePredicate>>(spec);
                const PyPredicate *own = predicate.target<PyPredicate>();
                result[py::str(key)] = py::cast(PyReplaceIf(newValToPy(value), own != nullptr ? own->predicate
                                                                                               : py::object(py::cast(predicate))));
            }
        }
        return result;
    }
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

    py::class_<PyFromOldVal, py::smart_holder>(m, "FromOldVal", R"doc(
Marks a callable in a :class:`RegNewVals` value slot as wanting the value already there

A bare callable in that slot already means ``newVal(modType)``, so the one that also reads the old
value says so by being wrapped in this -- the way :class:`ReplaceList` and :class:`ReplaceIf` say
which of :meth:`IfContentPart.replaceVals`' forms they are. Wrapped, it is called as
``newVal(oldValue, modType)``

Nothing else in the `regEdits` family can write a value derived from the old one:
:class:`RegRemap` moves the KEY (its :class:`RemappedKeyData` may *test* the value and cannot change
it), and :class:`RegAssetRemap` maps a value through :class:`ModMappedAssets`, which is the right
answer for a hash or an index and no help for anything else

.. note::
    A register the :class:`IfContentPart` does not have has no old value to read, so ``addNewKVPs``
    does not apply to one of these -- an absent register stays absent

Parameters
----------
producer: Callable[[:class:`str`, Optional[:class:`ModType`]], :class:`str`]
    Called as ``producer(oldValue, modType)`` when :meth:`RegNewVals.edit` runs, to produce the
    value to write :raw-html:`<br />` :raw-html:`<br />`

    eg. :raw-html:`<br />`
    ``{"ps-t1": FromOldVal(lambda old, modType: old + "Edited")}``
    )doc")
        .def(py::init([](py::object producer) {
            return std::make_unique<PyFromOldVal>(std::move(producer));
        }), py::arg("producer"))
        .def_property_readonly("producer", [](const PyFromOldVal &self) {
            return self.producer();
        }, py::doc(R"doc(
Callable[[:class:`str`, Optional[:class:`ModType`]], :class:`str`]: The callable to invoke, as
``producer(oldValue, modType)``
        )doc"));

    cls.def(py::init([](py::object vals, bool addNewKVPs) {
        return std::make_unique<PyRegNewVals>(parseNewVals(vals), addNewKVPs);
    }), py::arg("vals"), py::arg("addNewKVPs") = false);

    cls.def_property("vals", [](const PyRegNewVals &self) {
        return newValsToPy(self);
    }, [](PyRegNewVals &self, py::object vals) {
        self.vals = parseNewVals(vals);
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
