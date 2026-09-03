#include "PyRegAdd.h"

#include <memory>
#include <utility>
#include <vector>


namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyRegAdd::PyRegAdd(py::object valsObj, bool latest): Core({}, latest), valsObj(std::move(valsObj)) {}

void PyRegAdd::refresh(const py::object &modType) {
    (void)modType;
    vals = py::cast<std::vector<std::pair<std::string, std::string>>>(valsObj);
}


void initCppRegAdd(pybind11::module_ &m) {
    py::class_<PyRegAdd, PyBaseRegEdit, py::smart_holder> cls(m, "RegAdd", R"doc(
This class inherits from :class:`BaseRegEdit`

Bulk adds some `KVPs`_ into some :class:`IfContentPart`

Parameters
----------
vals: List[Tuple[:class:`str`, :class:`str`]]
    The `KVPs`_ to add, in the order given

latest: :class:`bool`
    Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if 'partRanges' is
    provided to :meth:`edit`, at the end of that window), instead of at the beginning :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    // py::init(factory) rather than py::init<py::object, bool>(): AGRC::RegAdd owns std::function
    // and std::variant members through its base's typedefs, and a factory returning a unique_ptr
    // avoids ever needing to move-construct the class itself -- the same pattern BaseSLR1Parser's
    // constructor bindings use.
    cls.def(py::init([](py::object vals, bool latest) {
        return std::make_unique<PyRegAdd>(std::move(vals), latest);
    }), py::arg("vals"), py::arg("latest") = true);

    cls.def_property("vals", [](const PyRegAdd &self) {
        return self.valsObj;
    }, [](PyRegAdd &self, py::object vals) {
        self.valsObj = std::move(vals);
    }, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`str`]]: The `KVPs`_ to add, in the order given
    )doc"));

    cls.def_readwrite("latest", &PyRegAdd::latest, py::doc(R"doc(
:class:`bool`: Whether to add :attr:`vals` at the end of the :class:`IfContentPart` (or, if
'partRanges' is provided to :meth:`edit`, at the end of that window), instead of at the beginning
    )doc"));

    bindRegEditEdit<PyRegAdd>(cls, R"doc(
Adds every `KVP`_ in :attr:`vals` into 'part'

With no 'partRanges', the `KVPs`_ go straight to the true beginning/end of 'part' (based on
:attr:`latest`). With a 'partRanges' window, they instead go right after the last valid index of
that window (or right before its first valid index, when :attr:`latest` is ``False``) -- an
unbounded window edge falls back to the true end/beginning of 'part'. An empty :attr:`vals` or an
empty 'partRanges' leaves 'part' untouched

Parameters
----------
part: :class:`IfContentPart`
    The part of the `IfTemplate` that is being editted

sectionName: :class:`str`
    The name of the `section`_ that is being editted. Unused by this edit

modType: Optional[:class:`ModType`]
    The type of mod to fix. Unused by this edit

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
