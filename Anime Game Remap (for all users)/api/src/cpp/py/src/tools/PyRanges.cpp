#include "PyRanges.h"
 
 
namespace py = pybind11;
namespace AGRC = AGRemapCore; 
 
 
#include "PyRanges.h"

#include <string>


namespace py = pybind11;

namespace {

// Substitutes the literal placeholder 'RangesInt' in the shared class-level
// docstring template with the actual Python class name this instantiation
// is bound under (e.g. 'Ranges' for the long long binding), so
// help(Ranges) doesn't incorrectly describe itself as 'RangesInt'.
std::string classDoc(const std::string &className, const std::string &tmpl) {
    std::string result = tmpl;
    const std::string placeholder = "RangesInt";
    size_t pos = 0;
    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
        result.replace(pos, placeholder.length(), className);
        pos += className.length();
    }
    return result;
}

}


template <typename T>
void bindRangesClass(pybind11::module_ &m, const char *className) {
    using RangesT = AGRemapCore::Ranges<T>;

    py::class_<PyRanges<T>, PyBindRanges<T>>(m, className, classDoc(className, R"doc(
A class representing a collection of integer ranges

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: value in x

        Determines whether 'value' falls within any of the ranges

    .. describe:: x == y

        Determines whether 'x' and 'y' store the same ranges

    .. describe:: x != y

        Determines whether 'x' and 'y' store different ranges

    .. describe:: x - y

        Computes the set difference between 'x' and 'y' (equivalent to 'x.difference(y)')

    .. describe:: ~x

        Computes the negation (complement) of 'x' (equivalent to 'x.negate()')

    .. describe:: x + y

        Computes the union of 'x' and 'y' (equivalent to 'x.union([y])')

    .. describe:: x += y

        Performs the union of 'x' with 'y', in place (equivalent to 'x.update([y])')

    .. describe:: x & y

        Computes the intersection of 'x' and 'y' (equivalent to 'x.intersect([y])')

    .. describe:: x &= y

        Performs the intersection of 'x' with 'y', in place (equivalent to 'x.intersectUpdate([y])')

.. note::
    Supports Python's ``copy`` module: both ``copy.copy(x)`` and ``copy.deepcopy(x)`` return a deep copy (equivalent to 'x.deepcopy()')

.. note::
    This constructor is overloaded. Instead of a list of range tuples, ``Ranges`` can also be constructed directly
    from a ``List[int]`` or a ``Set[int]``, in which case the resulting ranges cover exactly those values (equivalent
    to :meth:`createFromList` / :meth:`createFromSet`)

Parameters
----------
ranges: List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]
    The ranges to store :raw-html:`<br />` :raw-html:`<br />`

    Each range is a tuple containing the starting (inclusive) index and the ending (exclusive) index of the range :raw-html:`<br />` :raw-html:`<br />`

    If the starting index is ``None``, the range is unbounded towards -infinity. If the ending index is ``None``, the range is unbounded towards +infinity

normalize: :class:`bool`
    Whether to normalize 'ranges' before storing it :raw-html:`<br />` :raw-html:`<br />`

    If ``True``, 'ranges' is sorted and any overlapping or touching ranges are merged, producing the minimal set of disjoint ranges :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
        )doc").c_str())

        .def(py::init<std::vector<typename RangesT::Range>, bool>(), py::arg("ranges"), py::arg("normalize") = true)

        .def(py::init<const std::vector<T>&>(), py::arg("values"))

        // Python only has one 'set' type, so std::set<T> alone covers it;
        // binding AGRemapCore::Ranges(const std::unordered_set<T>&) as
        // well would be redundant (and ambiguous to pybind, since both
        // would match a Python 'set' equally well).
        .def(py::init<const std::set<T>&>(), py::arg("values"))

        .def_readwrite("ranges", &PyRanges<T>::ranges,
    py::doc(R"doc(
List[Tuple[Optional[:class:`int`], Optional[:class:`int`]]]: The stored ranges
        )doc"))

        .def("has", &PyRanges<T>::has, py::arg("value"),
    py::doc(R"doc(
Determines whether 'value' falls within any of the stored ranges

Parameters
----------
value: :class:`int`
    The value to check

Returns
-------
:class:`bool`
    Whether 'value' is contained within any of the stored ranges
        )doc"))

        .def("__contains__", &PyRanges<T>::has, py::arg("value"),
    py::doc(R"doc(Determines whether 'value' falls within any of the stored ranges)doc"))

        .def("isEmpty", &PyRanges<T>::isEmpty,
    py::doc(R"doc(
Determines whether there are no stored ranges

Returns
-------
:class:`bool`
    Whether there are no stored ranges
        )doc"))

        .def("isFull", &PyRanges<T>::isFull,
    py::doc(R"doc(
Determines whether the stored ranges span from -infinity to +infinity

.. note::
    This checks for a single stored range of ``(None, None)``. If this instance was constructed with ``normalize=False``, several ranges
    could collectively cover -infinity to +infinity without having been merged into one, in which case this returns ``False``

Returns
-------
:class:`bool`
    Whether the stored ranges span from -infinity to +infinity
        )doc"))

        .def("add", &PyRanges<T>::add, py::arg("value"), py::arg("normalize") = false,
    py::doc(R"doc(
Adds 'value' to the stored ranges, extending or merging existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`

If 'value' is already contained within the stored ranges, this has no effect

Parameters
----------
value: :class:`int`
    The value to add

normalize: :class:`bool`
    Whether to (re)normalize ``self`` before adding 'value' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
    on that to locate the insertion point efficiently). If that assumption doesn't hold, pass ``True``, or the
    result may be incorrect
        )doc"))

        .def("remove", &PyRanges<T>::remove, py::arg("value"), py::arg("normalize") = false,
    py::doc(R"doc(
Removes 'value' from the stored ranges, shrinking, splitting, or erasing existing ranges as needed :raw-html:`<br />` :raw-html:`<br />`

If 'value' isn't contained within the stored ranges, this has no effect

Parameters
----------
value: :class:`int`
    The value to remove

normalize: :class:`bool`
    Whether to (re)normalize ``self`` before removing 'value' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint (this method relies
    on that to locate 'value' efficiently). If that assumption doesn't hold, pass ``True``, or the result may
    be incorrect
        )doc"))

        .def("getOverlaps", [](const PyRanges<T> &self, const std::vector<PyRanges<T>> &ranges, bool requireAll, bool normalizeSelf, bool normalizeOthers) {
            std::vector<RangesT> baseRanges(ranges.begin(), ranges.end());
            RangesT result = self.getOverlaps(baseRanges, requireAll, normalizeSelf, normalizeOthers);
            return PyRanges<T>(result.ranges, false);
        }, py::arg("ranges"), py::arg("requireAll") = true, py::arg("normalizeSelf") = false, py::arg("normalizeOthers") = false,
    py::doc(R"doc(
Computes the overlap between this instance and a list of other :class:`Ranges`

Parameters
----------
ranges: List[:class:`Ranges`]
    The list of other ranges to compute the overlap against

requireAll: :class:`bool`
    When ``True`` (the default), the result is the intersection of ``self`` and *every* entry in 'ranges'
    (a value must fall within ``self`` and all of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`

    When ``False``, the result is the intersection of ``self`` and the *union* of 'ranges'
    (a value must fall within ``self`` and at least one of 'ranges' to be included) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

normalizeSelf: :class:`bool`
    Whether to (re)normalize ``self`` before computing the overlap :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
    assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect

normalizeOthers: :class:`bool`
    Whether to (re)normalize each entry of 'ranges' before computing the overlap :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
    that assumption doesn't hold, pass ``True``, or the computed overlap may be incorrect

Returns
-------
:class:`Ranges`
    A new instance containing the computed overlap
        )doc"))

        .def("intersect", [](const PyRanges<T> &self, const std::vector<PyRanges<T>> &ranges, bool normalizeSelf, bool normalizeOthers) {
            std::vector<RangesT> baseRanges(ranges.begin(), ranges.end());
            RangesT result = self.intersect(baseRanges, normalizeSelf, normalizeOthers);
            return PyRanges<T>(result.ranges, false);
        }, py::arg("ranges"), py::arg("normalizeSelf") = false, py::arg("normalizeOthers") = false,
    py::doc(R"doc(
Computes the intersection of this instance and a list of other :class:`Ranges` :raw-html:`<br />` :raw-html:`<br />`

Equivalent to ``self.getOverlaps(ranges, True, normalizeSelf, normalizeOthers)``

Parameters
----------
ranges: List[:class:`Ranges`]
    The list of other ranges to intersect with

normalizeSelf: :class:`bool`
    Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
    assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect

normalizeOthers: :class:`bool`
    Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
    that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect

Returns
-------
:class:`Ranges`
    A new instance containing the intersection of ``self`` and every entry in 'ranges'
        )doc"))

        .def("intersectUpdate", [](PyRanges<T> &self, const std::vector<PyRanges<T>> &ranges, bool normalizeSelf, bool normalizeOthers) {
            std::vector<RangesT> baseRanges(ranges.begin(), ranges.end());
            self.intersectUpdate(baseRanges, normalizeSelf, normalizeOthers);
        }, py::arg("ranges"), py::arg("normalizeSelf") = false, py::arg("normalizeOthers") = false,
    py::doc(R"doc(
Performs the intersection of this instance with a list of other :class:`Ranges`, in place :raw-html:`<br />` :raw-html:`<br />`

Equivalent to ``self.intersect(ranges, normalizeSelf, normalizeOthers)``, assigned back to ``self``

Parameters
----------
ranges: List[:class:`Ranges`]
    The list of other ranges to intersect with

normalizeSelf: :class:`bool`
    Whether to (re)normalize ``self`` before computing the intersection :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning ``self`` is assumed to already be sorted and disjoint. If that
    assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect

normalizeOthers: :class:`bool`
    Whether to (re)normalize each entry of 'ranges' before computing the intersection :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``, meaning every entry of 'ranges' is assumed to already be sorted and disjoint. If
    that assumption doesn't hold, pass ``True``, or the computed intersection may be incorrect
        )doc"))

        .def("__and__", [](const PyRanges<T> &self, const PyRanges<T> &other) { return PyRanges<T>(self.intersect({other}).ranges, false); }, py::arg("other"),
    py::doc(R"doc(Computes the intersection of this instance with 'other' (equivalent to 'self.intersect([other])'))doc"))

        .def("__iand__", [](PyRanges<T> &self, const PyRanges<T> &other) -> PyRanges<T>& {
            self.intersectUpdate({other});
            return self;
        }, py::arg("other"),
    py::doc(R"doc(Performs the intersection of this instance with 'other', in place (equivalent to 'self.intersectUpdate([other])'))doc"))

        .def("update", [](PyRanges<T> &self, const std::vector<PyRanges<T>> &ranges) {
            std::vector<RangesT> baseRanges(ranges.begin(), ranges.end());
            self.update(baseRanges);
        }, py::arg("ranges"),
    py::doc(R"doc(
Performs the union of this instance with a list of other :class:`Ranges`, in place

Parameters
----------
ranges: List[:class:`Ranges`]
    The list of other ranges to union with
        )doc"))

        .def("union", [](const PyRanges<T> &self, const std::vector<PyRanges<T>> &ranges) {
            std::vector<RangesT> baseRanges(ranges.begin(), ranges.end());
            return PyRanges<T>(self.unionWith(baseRanges).ranges, false);
        }, py::arg("ranges"),
    py::doc(R"doc(
Computes the union of this instance with a list of other :class:`Ranges`

Parameters
----------
ranges: List[:class:`Ranges`]
    The list of other ranges to union with

Returns
-------
:class:`Ranges`
    A new instance containing the union of ``self`` and 'ranges'
        )doc"))

        .def("__iadd__", [](PyRanges<T> &self, const PyRanges<T> &other) -> PyRanges<T>& {
            self.update({other});
            return self;
        }, py::arg("other"),
    py::doc(R"doc(Performs the union of this instance with 'other', in place (equivalent to 'self.update([other])'))doc"))

        .def("__add__", [](const PyRanges<T> &self, const PyRanges<T> &other) { return PyRanges<T>(self.unionWith({other}).ranges, false); }, py::arg("other"),
    py::doc(R"doc(Computes the union of this instance with 'other' (equivalent to 'self.union([other])'))doc"))

        .def("difference", [](const PyRanges<T> &self, const PyRanges<T> &other) { return PyRanges<T>(self.difference(other).ranges, false); }, py::arg("other"),
    py::doc(R"doc(
Computes the set difference between this instance and 'other' (``self - other``)

Parameters
----------
other: :class:`Ranges`
    The ranges to subtract from this instance

Returns
-------
:class:`Ranges`
    A new instance containing the values in ``self`` that are not in 'other'
        )doc"))

        .def("__sub__", [](const PyRanges<T> &self, const PyRanges<T> &other) { return PyRanges<T>(self.difference(other).ranges, false); }, py::arg("other"),
    py::doc(R"doc(Computes the set difference between this instance and 'other' (``self - other``))doc"))

        .def("negate", [](const PyRanges<T> &self) { return PyRanges<T>(self.negate().ranges, false); },
    py::doc(R"doc(
Computes the negation (complement) of this instance :raw-html:`<br />` :raw-html:`<br />`

Equivalent to ``Ranges.createFull() - self``

Returns
-------
:class:`Ranges`
    A new instance containing every value not in ``self``
        )doc"))

        .def("__invert__", [](const PyRanges<T> &self) { return PyRanges<T>(self.negate().ranges, false); },
    py::doc(R"doc(Computes the negation (complement) of this instance (equivalent to :meth:`negate`))doc"))

        .def_static("createFull", []() { return PyRanges<T>(RangesT::createFull().ranges, false); },
    py::doc(R"doc(
Creates a :class:`Ranges` spanning from -infinity to +infinity

Returns
-------
:class:`Ranges`
    A new instance containing a single range from -infinity to +infinity
        )doc"))

        .def_static("createEmpty", []() { return PyRanges<T>(RangesT::createEmpty().ranges, false); },
    py::doc(R"doc(
Creates a :class:`Ranges` with no ranges

Returns
-------
:class:`Ranges`
    A new, empty instance
        )doc"))

        .def_static("createFromList", [](const std::vector<T> &values) { return PyRanges<T>(RangesT::createFromList(values).ranges, false); }, py::arg("values"),
    py::doc(R"doc(
Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`

Consecutive integers are merged into contiguous ranges (e.g. ``[1, 2, 3, 5]`` becomes ``[1,4)`` and ``[5,6)``)

Parameters
----------
values: List[:class:`int`]
    The values to include

Returns
-------
:class:`Ranges`
    A new instance whose ranges cover exactly the values in 'values'
        )doc"))

        .def_static("createFromSet", [](const std::set<T> &values) { return PyRanges<T>(RangesT::createFromSet(values).ranges, false); }, py::arg("values"),
    py::doc(R"doc(
Creates a :class:`Ranges` representing exactly the values in 'values' :raw-html:`<br />` :raw-html:`<br />`

Consecutive integers are merged into contiguous ranges (e.g. ``{1, 2, 3, 5}`` becomes ``[1,4)`` and ``[5,6)``)

Parameters
----------
values: Set[:class:`int`]
    The values to include

Returns
-------
:class:`Ranges`
    A new instance whose ranges cover exactly the values in 'values'
        )doc"))

        .def("__eq__", [](const PyRanges<T> &self, const PyRanges<T> &other) { return self.ranges == other.ranges; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store the same ranges)doc"))

        .def("__ne__", [](const PyRanges<T> &self, const PyRanges<T> &other) { return self.ranges != other.ranges; }, py::arg("other"),
    py::doc(R"doc(Determines whether 'self' and 'other' store different ranges)doc"))

        .def("deepcopy", [](const PyRanges<T> &self) { return PyRanges<T>(self.deepcopy().ranges, false); },
    py::doc(R"doc(
Creates a deep copy of this instance

Returns
-------
:class:`Ranges`
    A new instance that is a deep copy of this one
        )doc"))

        .def("__copy__", [](const PyRanges<T> &self) { return PyRanges<T>(self.deepcopy().ranges, false); },
    py::doc(R"doc(Creates a copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.copy()``)doc"))

        .def("__deepcopy__", [](const PyRanges<T> &self, py::dict) { return PyRanges<T>(self.deepcopy().ranges, false); }, py::arg("memo"),
    py::doc(R"doc(Creates a deep copy of this instance (equivalent to :meth:`deepcopy`); supports ``copy.deepcopy()``)doc"));
}


void initCppRanges(pybind11::module_ &m) {
    bindRangesClass<int>(m, "RangesInt");
    bindRangesClass<long long>(m, "Ranges");
}