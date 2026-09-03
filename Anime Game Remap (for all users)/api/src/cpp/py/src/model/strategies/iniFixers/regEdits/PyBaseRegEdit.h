#ifndef AGRemapPyBind_PyBaseRegEdit_H
#define AGRemapPyBind_PyBaseRegEdit_H

#include <optional>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "../../../../tools/PyRanges.h"
#include "../../../iftemplate/PyIfContentPart.h"  // reuses PyIfContentPart (the exact ContentPart
                                                   // instantiation every reg edit below edits) and
                                                   // its parseRemoveKeys/parseKeyRemap/
                                                   // parseReplaceVals dict-parsing helpers
#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::BaseRegEdit`\\<py::object, py::object\\>. A plain alias,
 not a subclass -- nothing on the C++ side ever holds a reg edit through this base and calls
 ``edit`` itself (the only consumer, ``GraphGroupEdit``, is still pure `Python`_), so no
 trampoline is needed: a `Python`_ subclass overriding ``edit`` is resolved by ordinary `Python`_
 method lookup before anything reaches C++
 @endrst
 */
using PyBaseRegEdit = AGRC::BaseRegEdit<std::string, std::string>;


/**
 * @brief
 @rst
 Resolves a `Python`_ ``partRanges`` argument into the non-owning
 :cpp:class:`AGRemapCore::Ranges`\\<long long\\> pointer every :cpp:func:`AGRemapCore::BaseRegEdit::edit`
 takes :raw-html:`<br />` :raw-html:`<br />`

 Accepts a bound `Ranges` instance (the common case -- borrowed, never copied), a raw list of
 ``(start, end)`` bounds (materialized into a short-lived `Ranges` owned by this object, **not**
 normalized, matching what ``parseRanges`` does for the same shape elsewhere), or ``None``
 (``nullptr``). Keep the instance alive for as long as the pointer is in use -- it is a scope
 guard, not a value
 @endrst
 */
class PyPartRanges {
    public:

        /**
         * @brief Resolves 'partRanges'
         *
         * @param partRanges The Python value to resolve -- a bound `Ranges`, a raw list of bounds, or None
         */
        explicit PyPartRanges(const py::object &partRanges);

        /**
         * @brief Retrieves the resolved ranges
         *
         * @return The resolved ranges, or ``nullptr`` when 'partRanges' was None
         */
        const AGRC::Ranges<long long>* get() const;

    private:
        const AGRC::Ranges<long long>* ranges_ = nullptr;
        std::optional<AGRC::Ranges<long long>> owned_;
};


/**
 * @brief
 @rst
 Chains the shared ``edit`` binding onto one concrete reg edit's already-constructed
 ``py::class_`` :raw-html:`<br />` :raw-html:`<br />`

 Every concrete reg edit's ``edit`` differs only in its docstring and in which C++ ``edit``
 override ends up running, so the argument list/parsing/return-the-same-part behaviour is bound
 once here instead of copy-pasted four times. ``T`` must expose a ``refresh(modType)`` method
 (see each ``PyRegXxx`` class's own note on why the `Python`_-side argument object stays the
 source of truth, and `PyRegNewVals`'s on why ``refresh`` needs ``modType``)
 @endrst
 *
 * @param cls The class registration to chain onto
 * @param docstring The Python docstring for this class's own ``edit``
 *
 * @tparam T The concrete ``PyRegXxx`` class being bound
 * @tparam PyClass The ``py::class_<...>`` specialization for ``T``
 */
template <typename T, typename PyClass>
void bindRegEditEdit(PyClass &cls, const char *docstring) {
    cls.def("edit", [](T &self, py::object part, const std::string &sectionName, const py::object &modType,
                       const std::string &modName, const py::object &partRanges) {
        // The C++ core takes 'modType' as a nullable ModType*, and the Python-side ModType is a
        // pure-Python class with no C++ counterpart to cast to, so nullptr is the only honest
        // thing to pass down. The *Python* object is handed to refresh() instead: RegNewVals
        // needs it to call a user-supplied ReplaceIf predicate as predicate(oldValue, modType).
        // Every other reg edit ignores it, exactly as its pure-Python original did.
        self.refresh(modType);

        PyPartRanges ranges(partRanges);
        self.edit(py::cast<PyIfContentPart &>(part), sectionName, nullptr, modName, ranges.get());

        // Returns the original Python object rather than py::cast()-ing the C++ reference back,
        // so 'result is part' holds (matching the pure-Python originals, which all did a plain
        // 'return part') and a Python subclass's own wrapper identity survives the round trip.
        return part;
    }, py::arg("part"), py::arg("sectionName"), py::arg("modType"), py::arg("modName") = "",
       py::arg("partRanges") = py::none(), py::doc(docstring));
}


void initCppBaseRegEdit(pybind11::module_ &m);

#endif
