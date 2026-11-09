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

#include "PyVGRemapBlendEdit.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/iniresources/RemapBlendResource.h"
#include "AGRemapCore/model/strategies/ModType.h"
#include "../../../../files/PyBufFile.h"
#include "../../../../../tools/PyRefFunction.h"


namespace {

using VGReplace = AGRC::VGRemapBlendReplace<std::string, std::string>;


// A Python callable held under the GIL: the functions below are copied and destroyed by C++ code that
// does not hold it
std::shared_ptr<py::object> holdCallable(const py::object &fn) {
    if (!PyCallable_Check(fn.ptr())) {
        throw py::type_error("Expected a callable or None");
    }

    return std::shared_ptr<py::object>(new py::object(fn), [](py::object* obj) {
        if (!Py_IsInitialized()) {
            return;
        }
        py::gil_scoped_acquire gil;
        delete obj;
    });
}

// the setters the properties and the constructor share
void setFixFunc(PyVGRemapBlendReplace &self, py::object fixFunc) {
    self.fixFunc = toPyRefFunction<bool(AGRC::RemapBlendResource&)>(fixFunc);
    self.fixFuncObj = std::move(fixFunc);
}

void setElements(PyVGRemapBlendReplace &self, py::object elements) {
    if (elements.is_none()) {
        self.elements = {};
    } else {
        std::shared_ptr<py::object> held = holdCallable(elements);
        self.elements = [held](const std::string &srcPath) {
            py::gil_scoped_acquire gil;
            return parseBufElementTypes((*held)(py::str(srcPath)));
        };
    }
    self.elementsObj = std::move(elements);
}

void setPairedIds(PyVGRemapBlendReplace &self, py::object pairedIds) {
    if (pairedIds.is_none()) {
        self.pairedIds = {};
    } else {
        std::shared_ptr<py::object> held = holdCallable(pairedIds);
        self.pairedIds = [held](const std::string &srcPath) {
            py::gil_scoped_acquire gil;
            return (*held)(py::str(srcPath)).cast<std::vector<VGReplace::PairedIds>>();
        };
    }
    self.pairedIdsObj = std::move(pairedIds);
}

}


PyVGRemapBlendReplace::PyVGRemapBlendReplace(py::object resModObj, py::object modType,
                                             std::optional<AGRC::Version> fromVersion,
                                             std::optional<AGRC::Version> toVersion, std::string resType,
                                             std::optional<std::string> resSubType, std::optional<std::string> fromComp,
                                             std::optional<std::string> toComp):
    PyResEditMixin<VGReplace>(PyBaseResEditCore::GraphId(), makeResEditConfig(),
                              modType.is_none() ? nullptr : modType.cast<const AGRC::ModType*>(),
                              std::move(fromVersion), std::move(toVersion), std::move(resType), std::move(resSubType),
                              std::move(fromComp), std::move(toComp)),
    sourceModType(std::move(modType)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyVGRemapBlendReplace::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void initCppVGRemapBlendReplace(pybind11::module_ &m) {
    auto cls = py::class_<PyVGRemapBlendReplace, PyBaseResEditCore, py::smart_holder>(m, "VGRemapBlendReplace", R"doc(
This class inherits from :class:`BaseResEdit`

A ``Blend.buf`` replacement that remaps the vertex groups, with the remap :meth:`ModType.getVGRemap` gives
for the mod type being fixed from -- the edit every template fixer uses for a `blend`_

Use it through :class:`ResRegCollect`, as the texture edits are. Beyond the remap itself it can pin chains of
vertex groups to their root (:attr:`anchorChains`), read the source's line layout off its file
(:attr:`elements`), change the blend's width to the one the target's draws read (:attr:`influences`, with the id
buffers :attr:`pairedIds` names following), or fix the resource its own way (:attr:`fixFunc`)

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

modType: Optional[:class:`ModType`]
    The mod type being fixed from -- where the vertex group remap comes from. ``None`` remaps nothing

fromVersion: Optional[:class:`Version`]
    The game version the remap is read from. **Default**: ``None``, the latest

toVersion: Optional[:class:`Version`]
    The game version the remap is read to. **Default**: ``None``, the latest

resType: :class:`str`
    The name of the type of resource -- what the run's statistics count it under. **Default**: ``"blend"``

resSubType: Optional[:class:`str`]
    The name of the subtype of the resource, folded into every fixed name. **Default**: ``None``

fromComp: Optional[:class:`str`]
    The specific component to remap from. **Default**: ``None``

toComp: Optional[:class:`str`]
    The specific component to remap to. **Default**: ``None``
    )doc");

    py::class_<VGReplace::PairedIds>(cls, "PairedIds", R"doc(
A buffer of bone ids beside the fixed ``Blend.buf`` that names the bones for it, and keeps the same slots when
:attr:`VGRemapBlendReplace.influences` changes the blend's width

Parameters
----------
path: :class:`str`
    The buffer's path

idBytes: :class:`int`
    The bytes of one id. **Default**: ``2``
    )doc")
        .def(py::init([](std::string path, std::size_t idBytes) {
                 return VGReplace::PairedIds{std::move(path), idBytes};
             }),
             py::arg("path"), py::arg("idBytes") = 2)
        .def_readwrite("path", &VGReplace::PairedIds::path, py::doc(R"doc(
:class:`str`: The buffer's path
        )doc"))
        .def_readwrite("idBytes", &VGReplace::PairedIds::idBytes, py::doc(R"doc(
:class:`int`: The bytes of one id
        )doc"));

    cls.def(py::init([](py::object resModObj, py::object modType, std::optional<AGRC::Version> fromVersion,
                        std::optional<AGRC::Version> toVersion, std::string resType, std::optional<std::string> resSubType,
                        std::optional<std::string> fromComp, std::optional<std::string> toComp,
                        std::map<long long, std::vector<long long>> anchorChains, long long influences, py::object fixFunc,
                        py::object elements, py::object pairedIds, py::object row) {
                // every setting of the edit can be given here, as the property of the same name
                auto edit = std::make_unique<PyVGRemapBlendReplace>(std::move(resModObj), std::move(modType), std::move(fromVersion),
                                                                    std::move(toVersion), std::move(resType), std::move(resSubType),
                                                                    std::move(fromComp), std::move(toComp));
                edit->anchorChains = decltype(edit->anchorChains)(anchorChains.begin(), anchorChains.end());
                edit->influences = static_cast<decltype(edit->influences)>(influences);
                if (!fixFunc.is_none()) {
                    setFixFunc(*edit, std::move(fixFunc));
                }
                setElements(*edit, std::move(elements));
                setPairedIds(*edit, std::move(pairedIds));
                if (!row.is_none()) {
                    edit->row = row.cast<AGRC::VGRemap>();
                }
                return edit;
            }),
            py::arg("resModObj"), py::arg("modType"), py::arg("fromVersion") = py::none(),
            py::arg("toVersion") = py::none(), py::arg("resType") = "blend", py::arg("resSubType") = py::none(),
            py::arg("fromComp") = py::none(), py::arg("toComp") = py::none(),
            py::arg("anchorChains") = std::map<long long, std::vector<long long>>(), py::arg("influences") = 0,
            py::arg("fixFunc") = py::none(), py::arg("elements") = py::none(), py::arg("pairedIds") = py::none(),
            py::arg("row") = py::none());

    cls.def_property("row", [](const PyVGRemapBlendReplace &self) -> py::object {
        return self.row.has_value() ? py::cast(*self.row) : py::object(py::none());
    }, [](PyVGRemapBlendReplace &self, const py::object &row) {
        if (row.is_none()) {
            self.row.reset();
        } else {
            self.row = row.cast<AGRC::VGRemap>();
        }
    }, py::doc(R"doc(
Optional[:class:`VGRemap`]: The vertex group row given outright, used in place of looking one up in :attr:`modType`'s
asset tables -- a table the caller built, eg. a row composed with another map. **Default**: ``None``, the row is looked up
    )doc"));

    cls.def_property_readonly("modType", [](const PyVGRemapBlendReplace &self) { return self.sourceModType; }, py::doc(R"doc(
Optional[:class:`ModType`]: The mod type being fixed from -- where the vertex group remap comes from
    )doc"));

    cls.def_readwrite("rowModName", &PyVGRemapBlendReplace::rowModName, py::doc(R"doc(
Optional[:class:`str`]: The mod name the vertex group row is looked up under, in place of the mod being fixed to
-- eg. the skin, for a fixer whose target is one COMPONENT of it (:attr:`toComp` names the component).
**Default**: ``None``
    )doc"));

    cls.def_readwrite("fromVersion", &PyVGRemapBlendReplace::fromVersion, py::doc(R"doc(
Optional[:class:`Version`]: The game version the remap is read from
    )doc"));

    cls.def_readwrite("toVersion", &PyVGRemapBlendReplace::toVersion, py::doc(R"doc(
Optional[:class:`Version`]: The game version the remap is read to
    )doc"));

    cls.def_readwrite("resSubType", &PyVGRemapBlendReplace::resSubType, py::doc(R"doc(
Optional[:class:`str`]: The name of the subtype of the resource, folded into every fixed name
    )doc"));

    cls.def_readwrite("fromComp", &PyVGRemapBlendReplace::fromComp, py::doc(R"doc(
Optional[:class:`str`]: The specific component to remap from
    )doc"));

    cls.def_readwrite("toComp", &PyVGRemapBlendReplace::toComp, py::doc(R"doc(
Optional[:class:`str`]: The specific component to remap to
    )doc"));

    cls.def_readwrite("anchorChains", &PyVGRemapBlendReplace::anchorChains, py::doc(R"doc(
Dict[:class:`int`, List[:class:`int`]]: Vertex groups that move as one part: each key is a ROOT group of the source,
and every group in its list is remapped to whatever the root is remapped to -- a chain the target has no
counterpart for, pinned to the bone it hangs from
    )doc"));

    cls.def_readwrite("influences", &PyVGRemapBlendReplace::influences, py::doc(R"doc(
:class:`int`: How many (weight, index) slots each vertex of the fixed ``Blend.buf`` carries -- the width the
target's draws read -- or ``0`` to keep the source's. A changed width is also written into the generated
`section`_'s ``stride``. **Default**: ``0``
    )doc"));

    cls.def_property("fixFunc", [](const PyVGRemapBlendReplace &self) { return self.fixFuncObj; },
                     [](PyVGRemapBlendReplace &self, py::object fixFunc) { setFixFunc(self, std::move(fixFunc)); }, py::doc(R"doc(
Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]: What fixes the built resource in place of
its own remap -- eg. a blend whose true bone ids live in another buffer. Answers whether the fixed file was
written
    )doc"));

    cls.def_property("elements", [](const PyVGRemapBlendReplace &self) { return self.elementsObj; },
                     [](PyVGRemapBlendReplace &self, py::object elements) { setElements(self, std::move(elements)); }, py::doc(R"doc(
Optional[Callable[[:class:`str`], List[:class:`BufElementType`]]]: The line layout of the source ``Blend.buf``,
given its path -- eg. a layout whose number of bone influences per vertex is read off the file's size
    )doc"));

    cls.def_property("pairedIds", [](const PyVGRemapBlendReplace &self) { return self.pairedIdsObj; },
                     [](PyVGRemapBlendReplace &self, py::object pairedIds) { setPairedIds(self, std::move(pairedIds)); }, py::doc(R"doc(
Optional[Callable[[:class:`str`], List[:class:`VGRemapBlendReplace.PairedIds`]]]: The id buffers beside the fixed
``Blend.buf`` that follow its slots when :attr:`influences` changes its width, given the source file's path.
The first names the bones the blend's slots are merged by
    )doc"));
}
