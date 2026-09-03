#include "PyBlendEdit.h"

#include <memory>
#include <optional>
#include <utility>


namespace {

// The pure-Python original's Optional[str] constructor arguments -- None means "not set", which is
// std::nullopt here rather than an empty string (an empty resSubType would still be appended).
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


PyRemapBlendReplace::PyRemapBlendReplace(py::object resModObj, std::string resType, py::object fixFunc,
                                          const py::object &resSubType, const py::object &fromComp, const py::object &toComp):
    PyResEditMixin<AGRC::RemapBlendReplace<std::string, std::string>>(
        PyBaseResEditCore::GraphId(), makeResEditConfig(), std::move(resType), parseOptionalStr(resSubType),
        parseOptionalStr(fromComp), parseOptionalStr(toComp)),
    fixFunc(std::move(fixFunc)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyRemapBlendReplace::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyRemapBlendReplace::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                         const std::string &modName, const std::string &fileKey, Context &ctx) {
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), py::str(fixedPath), modTypeObj),
                           modName, fileKey, ctx);
}


void initCppRemapBlendReplace(pybind11::module_ &m) {
    auto cls = py::class_<PyRemapBlendReplace, PyBaseResEditCore, py::smart_holder>(m, "RemapBlendReplace", R"doc(
This class inherits from :class:`ResReplace`

Class that builds the necessary part to replace some Blend.buf file

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``resourceRemapBlend``

fixFunc: Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]
    A custom function for fixing the Blend.buf file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resSubType: Optional[:class:`str`]
    The name of the subtype of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

fromComp: Optional[:class:`str`]
    The specific component to remap from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toComp: Optional[:class:`str`]
    The specific component to remap to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object resModObj, std::string resType, py::object fixFunc, const py::object &resSubType,
                        const py::object &fromComp, const py::object &toComp) {
        return std::make_unique<PyRemapBlendReplace>(std::move(resModObj), std::move(resType), std::move(fixFunc),
                                                      resSubType, fromComp, toComp);
    }), py::arg("resModObj"), py::arg("resType") = "resourceRemapBlend", py::arg("fixFunc") = py::none(),
        py::arg("resSubType") = py::none(), py::arg("fromComp") = py::none(), py::arg("toComp") = py::none());

    cls.def_property("fixFunc", [](const PyRemapBlendReplace &self) {
        return self.fixFunc;
    }, [](PyRemapBlendReplace &self, py::object fixFunc) {
        self.fixFunc = std::move(fixFunc);
    }, py::doc(R"doc(
Optional[Callable[[:class:`RemapBlendResource`], :class:`bool`]]: A custom function for fixing the
Blend.buf file
    )doc"));

    cls.def_property("resSubType", [](const PyRemapBlendReplace &self) {
        return optionalStrToPy(self.resSubType);
    }, [](PyRemapBlendReplace &self, const py::object &resSubType) {
        self.resSubType = parseOptionalStr(resSubType);
    }, py::doc(R"doc(Optional[:class:`str`]: The name of the subtype of the resource)doc"));

    cls.def_property("fromComp", [](const PyRemapBlendReplace &self) {
        return optionalStrToPy(self.fromComp);
    }, [](PyRemapBlendReplace &self, const py::object &fromComp) {
        self.fromComp = parseOptionalStr(fromComp);
    }, py::doc(R"doc(Optional[:class:`str`]: The specific component to remap from)doc"));

    cls.def_property("toComp", [](const PyRemapBlendReplace &self) {
        return optionalStrToPy(self.toComp);
    }, [](PyRemapBlendReplace &self, const py::object &toComp) {
        self.toComp = parseOptionalStr(toComp);
    }, py::doc(R"doc(Optional[:class:`str`]: The specific component to remap to)doc"));

    cls.def("buildResModel", [](PyRemapBlendReplace &self, const std::string &resType, const py::object &ini,
                                const std::string &srcPath, const std::string &fixedPath, const py::object &modType,
                                const py::args &, const std::string &modName, const py::kwargs &) -> py::object {
        if (ini.is_none() || modType.is_none()) {
            return py::none();
        }

        // Built by calling the bound classes rather than constructing the C++ types: 'vgRemap'
        // comes back from a pure-Python ModType, and RemapBlendResource's own binding already
        // knows how to accept it -- see pyCoreModule's note.
        py::object vgRemap = modType.attr("getVGRemap")(py::str(modName), py::arg("fromVersion") = ini.attr("version"),
                                                        py::arg("toVersion") = ini.attr("toVersion"),
                                                        py::arg("fromComp") = optionalStrToPy(self.fromComp),
                                                        py::arg("toComp") = optionalStrToPy(self.toComp));

        return pyCoreModule().attr("RemapBlendResource")(ini.attr("folder"), py::str(srcPath), py::str(fixedPath), vgRemap,
                                                          py::arg("type") = py::str(self.resType),
                                                          py::arg("fixFunc") = self.fixFunc);
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("fixedPath"), py::arg("modType") = py::none(),
       py::kw_only(), py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource

.. note::
    The ``type`` of the built resource comes from :attr:`resType`, not from the 'resType' argument
    -- faithful to the pure-Python original

Parameters
----------
resType: :class:`str`
    The name for the type of resource. Unused

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

fixedPath: :class:`str`
    The file path to the fixed resource

modType: :class:`ModType`
    The type of mod being fixed -- the vertex group remap comes from it

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`RemapBlendResource`
    The built resource
    )doc"));

    bindResEditCommonMethods(cls);
}
