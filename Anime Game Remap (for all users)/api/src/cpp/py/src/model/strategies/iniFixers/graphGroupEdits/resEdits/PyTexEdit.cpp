#include "PyTexEdit.h"

#include <memory>
#include <utility>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"


PyTexCreate::PyTexCreate(py::object resModObj, std::string texName, py::object texCreator, std::string resType,
                          py::object fixFunc):
    PyResCreateMixin<AGRC::TexCreate<py::object, py::object, PyObjectHash, PyObjectEqual>>(
        PyBaseResEditCore::GraphId(), std::move(texName), makeResEditConfig(), std::move(resType)),
    texCreator(std::move(texCreator)), fixFunc(std::move(fixFunc)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyTexCreate::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyTexCreate::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                 const std::string &modName, const std::string &fileKey, Context &ctx) {
    (void)fixedPath;
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), modTypeObj), modName, fileKey, ctx);
}


void initCppTexCreate(pybind11::module_ &m) {
    auto cls = py::class_<PyTexCreate, PyBaseResEditCore, py::smart_holder>(m, "TexCreate", R"doc(
This class inherits from :class:`ResCreate`

Class that builds the necessary parts to create some new texture file

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

texName: :class:`str`
    The name for the type of texture

texCreator: :class:`TexCreator`
    The editor for the texture file

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``resourceRemapTexAdd``

fixFunc: Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]
    The custom function for creating the texture :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object resModObj, std::string texName, py::object texCreator, std::string resType,
                        py::object fixFunc) {
        return std::make_unique<PyTexCreate>(std::move(resModObj), std::move(texName), std::move(texCreator),
                                              std::move(resType), std::move(fixFunc));
    }), py::arg("resModObj"), py::arg("texName"), py::arg("texCreator"), py::arg("resType") = "resourceRemapTexAdd",
        py::arg("fixFunc") = py::none());

    cls.def_readwrite("texName", &PyTexCreate::texName, py::doc(R"doc(
:class:`str`: The name for the type of texture
    )doc"));

    cls.def_property("texCreator", [](const PyTexCreate &self) {
        return self.texCreator;
    }, [](PyTexCreate &self, py::object texCreator) {
        self.texCreator = std::move(texCreator);
    }, py::doc(R"doc(:class:`TexCreator`: The editor for the texture file)doc"));

    cls.def_property("fixFunc", [](const PyTexCreate &self) {
        return self.fixFunc;
    }, [](PyTexCreate &self, py::object fixFunc) {
        self.fixFunc = std::move(fixFunc);
    }, py::doc(R"doc(
Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]: The custom function for creating
the texture
    )doc"));

    cls.def("buildResModel", [](PyTexCreate &self, const std::string &resType, const py::object &ini,
                                const std::string &srcPath, const py::object &modType, const py::args &,
                                const std::string &modName, const py::kwargs &) -> py::object {
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        // Unlike RemapBlendReplace's, this one really does use the 'resType' *argument* rather than
        // the attribute -- faithful to the pure-Python original, which differs between the two.
        return pyCoreModule().attr("RemapTexAddResource")(ini.attr("folder"), py::str(srcPath), self.texCreator,
                                                           py::arg("type") = py::str(resType),
                                                           py::arg("fixFunc") = self.fixFunc);
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("modType") = py::none(), py::kw_only(),
       py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource

Parameters
----------
resType: :class:`str`
    The name for the type of resource

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`RemapTexAddResource`
    The built resource
    )doc"));

    cls.def("buildSection", [](PyTexCreate &self, const std::string &sectionName, const py::object &modType,
                               const std::string &modName) -> py::object {
        // Strips the leading "Resource" from the section name to recover the file's base name --
        // the pure-Python original's own sectionName[len(IniKeywords.Resource.value):] slice.
        const std::string &resourcePrefix = AGRC::IniKeywords::Resource;
        std::string fileBaseName = sectionName;
        if (sectionName.rfind(resourcePrefix, 0) == 0) {
            fileBaseName = sectionName.substr(resourcePrefix.size());
        }

        py::object file = self.pySelf().attr("getFixFile")(py::str(fileBaseName + AGRC::FileExt::DDS), modType,
                                                            py::arg("modName") = modName);

        py::dict src;
        src[py::cast(std::string("filename"))] = py::make_tuple(py::make_tuple(0, file));

        py::object contentPart = pyCoreModule().attr("IfContentPart")(src, 0);
        return pyCoreModule().attr("IfTemplate")(py::make_tuple(contentPart), py::arg("name") = py::str(sectionName));
    }, py::arg("sectionName"), py::arg("modType") = py::none(), py::arg("modName") = "", py::doc(R"doc(
Builds a `section`_ for the resource -- a single ``filename =`` `KVP`_ pointing at the ``.dds`` file
this edit creates

Parameters
----------
sectionName: :class:`str`
    The name for the `section`_

modType: Optional[:class:`ModType`]
    The type of mod to fix from

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`IfTemplate`
    The generated `section`_
    )doc"));

    bindResEditCommonMethods(cls);
}
