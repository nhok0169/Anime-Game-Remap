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

#include "PyBufEdit.h"

#include <memory>
#include <optional>
#include <utility>

namespace {
std::optional<std::string> parseOptionalStr(const py::object &value) {
    if (value.is_none()) {
        return std::nullopt;
    }
    return py::str(value).cast<std::string>();
}

py::object optionalStrToPy(const std::optional<std::string> &value) {
    if (!value.has_value()) {
        return py::none();
    }
    return py::str(*value);
}
}


PyBufReplace::PyBufReplace(py::object resModObj, std::string kind, const py::object &resSubType):
    PyResEditMixin<AGRC::BufReplace<std::string, std::string>>(
        PyBaseResEditCore::GraphId(), makeResEditConfig(), std::move(kind), parseOptionalStr(resSubType)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyBufReplace::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyBufReplace::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                  const std::string &modName, const std::string &fileKey, Context &ctx) {
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), py::str(fixedPath), modTypeObj),
                           modName, fileKey, ctx);
}


void initCppBufReplace(pybind11::module_ &m) {
    auto cls = py::class_<PyBufReplace, PyBaseResEditCore, py::smart_holder>(m, "BufReplace", R"doc(
This class inherits from :class:`ResReplace`

Names the replacement of one of a mod's buffers -- a ``Blend.buf``, ``Position.buf``, ``Texcoord.buf``
or ``.ib`` -- by its **kind**, and builds a :class:`RemapIniFixResource` of that kind for it

Nothing here writes a file. A buffer that has to be fixed together with the others (see
:class:`VGSplitGroupResource`) is collected by a :class:`ResGroupCollect` through one of these per
buffer, and the grouped resource does the writing; a buffer fixed on its own wants
:class:`RemapBlendReplace` instead

The kind is the resource's ``type`` -- ``blend`` / ``position`` / ``texcoord`` / ``buf`` (an index
buffer) -- which is what the remap's stats count under and what a grouped fix tells its members apart
by. The naming follows the kind too: ``...RemapBlend``, ``...RemapPosition``, ``...RemapTexcoord``,
``...RemapIB``

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource

kind: :class:`str`
    ``"blend"``, ``"position"``, ``"texcoord"`` or ``"ib"``

resSubType: Optional[:class:`str`]
    An extra name between the mod's and the kind's, for a second buffer of the same kind and mod :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object resModObj, std::string kind, const py::object &resSubType) {
        return std::make_unique<PyBufReplace>(std::move(resModObj), std::move(kind), resSubType);
    }), py::arg("resModObj"), py::arg("kind"), py::arg("resSubType") = py::none());

    cls.def_property_readonly("kind", [](const PyBufReplace &self) { return self.kind; },
                              py::doc(":class:`str`: The kind of buffer"));

    cls.def_property("resSubType", [](const PyBufReplace &self) {
        return optionalStrToPy(self.resSubType);
    }, [](PyBufReplace &self, const py::object &resSubType) {
        self.resSubType = parseOptionalStr(resSubType);
    }, py::doc(R"doc(Optional[:class:`str`]: An extra name between the mod's and the kind's)doc"));

    cls.def("buildResModel", [](PyBufReplace &self, const std::string &resType, const py::object &ini,
                                const std::string &srcPath, const std::string &fixedPath, const py::object &modType,
                                const py::args &, const std::string &modName, const py::kwargs &) -> py::object {
        (void)resType;
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        // Typed by KIND, whatever 'resType' says: the grouped fix tells its members apart by it
        return pyCoreModule().attr("RemapIniFixResource")(py::str(self.resType), ini.attr("folder"), py::str(srcPath), py::str(fixedPath));
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("fixedPath"), py::arg("modType") = py::none(),
       py::kw_only(), py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource -- a :class:`RemapIniFixResource` typed by :attr:`kind`

Parameters
----------
resType: :class:`str`
    The name for the type of resource. Unused: the kind decides

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original buffer

fixedPath: :class:`str`
    The file path to the fixed buffer

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
Optional[:class:`RemapIniFixResource`]
    The built resource, or ``None`` without a .ini file
        )doc"));
}
