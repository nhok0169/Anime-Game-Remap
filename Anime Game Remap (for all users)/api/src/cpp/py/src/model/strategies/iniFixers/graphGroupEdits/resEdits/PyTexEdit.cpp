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

#include "PyTexEdit.h"

#include <memory>
#include <optional>
#include <utility>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "../../../../../tools/PyRefFunction.h"


namespace {

    // The C++ editor a Python TexEditor / TexCreator is -- the same object, held through its base
    std::shared_ptr<AGRC::BaseTexEditor> editorOf(const py::object& editor) {
        return editor.is_none() ? nullptr : editor.cast<std::shared_ptr<AGRC::BaseTexEditor>>();
    }

    // A Python function as the core class's fixFunc, or none
    template <typename Resource>
    std::function<bool(Resource&)> fixFuncOf(const py::object& fixFunc) {
        if (fixFunc.is_none()) {
            return {};
        }
        return toPyRefFunction<bool(Resource&)>(fixFunc);
    }

}


PyTexCreate::PyTexCreate(py::object resModObj, std::string texName, py::object texCreator, std::string resType,
                          py::object fixFunc):
    PyResCreateMixin<AGRC::TexCreate<std::string, std::string>>(
        PyBaseResEditCore::GraphId(), std::move(texName), editorOf(texCreator), makeResEditConfig(), std::move(resType)),
    texCreatorObj(std::move(texCreator)) {
    setFixFunc(std::move(fixFunc));
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


void PyTexCreate::setTexCreator(py::object creator) {
    this->texCreator = editorOf(creator);
    texCreatorObj = std::move(creator);
}


void PyTexCreate::setFixFunc(py::object fixFunc) {
    this->fixFunc = fixFuncOf<AGRC::RemapTexAddResource>(fixFunc);
    fixFuncObj = std::move(fixFunc);
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
        return self.texCreatorObj;
    }, [](PyTexCreate &self, py::object texCreator) {
        self.setTexCreator(std::move(texCreator));
    }, py::doc(R"doc(:class:`TexCreator`: The editor for the texture file)doc"));

    cls.def_property("fixFunc", [](const PyTexCreate &self) {
        return self.fixFuncObj;
    }, [](PyTexCreate &self, py::object fixFunc) {
        self.setFixFunc(std::move(fixFunc));
    }, py::doc(R"doc(
Optional[Callable[[:class:`RemapTexAddResource`], :class:`bool`]]: The custom function for creating
the texture
    )doc"));

    cls.def_readwrite("files", &PyTexCreate::files, py::doc(R"doc(
Dict[:class:`str`, :class:`str`]: The textures to create under the names their registers already give them,
as ``section name -> file`` (relative to the ``.ini`` file) :raw-html:`<br />` :raw-html:`<br />`

Set, a collected resource keeps its name (:meth:`getFixResourceName` answers ``None``), only the `sections`_
listed here are built, and each names its file -- for a fix that names its created textures itself. Empty (the
default), every collected resource is renamed from the mod's name and the texture's, and its file from that
    )doc"));

    cls.def("buildResModel", [](PyTexCreate &self, const std::string &resType, const py::object &ini,
                                const std::string &srcPath, const py::object &modType, const py::args &,
                                const std::string &modName, const py::kwargs &) -> py::object {
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        // The 'resType' ARGUMENT, as the core class's own buildResModel uses it
        return py::cast(self.makeResModel(py::str(ini.attr("folder")).cast<std::string>(), srcPath, resType));
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
        // The core's answer (TexCreate::fileOfSection: `files`, else getFixFile of the name without its
        // "Resource"), asked under the modType this call was given -- getFixFile reaches Python through it
        py::object previous = self.modTypeObj;
        self.modTypeObj = modType;
        std::optional<std::string> file;
        try {
            file = self.fileOfSection(sectionName, modName);
        } catch (...) {
            self.modTypeObj = previous;
            throw;
        }
        self.modTypeObj = previous;

        if (!file.has_value()) {
            return py::none();
        }

        py::dict src;
        src[py::cast(std::string("filename"))] = py::make_tuple(py::make_tuple(0, py::str(*file)));

        py::object contentPart = pyCoreModule().attr("IfContentPart")(src, 0);
        return pyCoreModule().attr("IfTemplate")(py::make_tuple(contentPart), py::arg("name") = py::str(sectionName));
    }, py::arg("sectionName"), py::arg("modType") = py::none(), py::arg("modName") = "", py::doc(R"doc(
Builds a `section`_ for the resource -- a single ``filename =`` `KVP`_ pointing at the ``.dds`` file
this edit creates: its entry in :attr:`files` when that is set, otherwise :meth:`getFixFile` of the
`section`_'s name without its leading ``Resource``

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
Optional[:class:`IfTemplate`]
    The generated `section`_, or ``None`` for a name :attr:`files` does not list
    )doc"));

    bindResEditCommonMethods(cls);
}


namespace {

    // Local copy of PyBlendEdit.cpp's own helper -- it lives in that file's anonymous namespace.
    std::optional<std::string> parseOptionalSubType(const py::object &value) {
        if (value.is_none()) {
            return std::nullopt;
        }

        return py::str(value).cast<std::string>();
    }

}


PyTexReplace::PyTexReplace(py::object resModObj, py::object texEditor, std::string resType, py::object fixFunc,
                            const py::object &resSubType):
    PyResEditMixin<AGRC::TexReplace<std::string, std::string>>(
        PyBaseResEditCore::GraphId(), editorOf(texEditor), makeResEditConfig(), std::move(resType), parseOptionalSubType(resSubType)),
    texEditorObj(std::move(texEditor)) {
    setFixFunc(std::move(fixFunc));
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


void PyTexReplace::setTexEditor(py::object editor) {
    this->texEditor = editorOf(editor);
    texEditorObj = std::move(editor);
}


void PyTexReplace::setFixFunc(py::object fixFunc) {
    this->fixFunc = fixFuncOf<AGRC::RemapTexEditResource>(fixFunc);
    fixFuncObj = std::move(fixFunc);
}


py::object PyTexReplace::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyTexReplace::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                  const std::string &modName, const std::string &fileKey, Context &ctx) {
    // Both paths travel, unlike PyTexCreate's -- an edit reads one file and writes another.
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), py::str(fixedPath), modTypeObj),
                           modName, fileKey, ctx);
}


void initCppTexReplace(pybind11::module_ &m) {
    auto cls = py::class_<PyTexReplace, PyBaseResEditCore, py::smart_holder>(m, "TexReplace", R"doc(
This class inherits from :class:`ResReplace`

Class that builds the necessary parts to edit an existing texture file

The counterpart to :class:`TexCreate`, which *creates* a brand new one, and the texture analogue of
:class:`RemapBlendReplace` -- an edit has an original resource to build its fixed name on, which is
what separates a :class:`ResReplace` from a :class:`ResCreate`

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

texEditor: :class:`CppTexEditor`
    The texture editor used to edit the ``.dds`` file

resType: :class:`str`
    The name of the type of resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"resourceRemapTexEdit"``

fixFunc: Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]
    A custom function for editing the texture file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resSubType: Optional[:class:`str`]
    The name of the subtype of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init<py::object, py::object, std::string, py::object, py::object>(),
            py::arg("resModObj"), py::arg("texEditor"), py::arg("resType") = "resourceRemapTexEdit",
            py::arg("fixFunc") = py::none(), py::arg("resSubType") = py::none());

    cls.def_property("texEditor", [](const PyTexReplace &self) { return self.texEditorObj; },
                     [](PyTexReplace &self, py::object editor) { self.setTexEditor(std::move(editor)); }, py::doc(R"doc(
:class:`CppTexEditor`: The texture editor used to edit the ``.dds`` file
    )doc"));

    cls.def_property("fixFunc", [](const PyTexReplace &self) { return self.fixFuncObj; },
                     [](PyTexReplace &self, py::object fixFunc) { self.setFixFunc(std::move(fixFunc)); }, py::doc(R"doc(
Optional[Callable[[:class:`RemapTexEditResource`], :class:`bool`]]: A custom function for editing the texture file
    )doc"));

    cls.def("buildResModel", [](PyTexReplace &self, const std::string &resType, const py::object &ini,
                                const std::string &srcPath, const std::string &fixedPath, const py::object &modType,
                                const py::args &, const std::string &modName, const py::kwargs &) -> py::object {
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        return py::cast(self.makeResModel(py::str(ini.attr("folder")).cast<std::string>(), srcPath, fixedPath, resType));
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("fixedPath"),
       py::arg("modType") = py::none(), py::kw_only(), py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource

Parameters
----------
resType: :class:`str`
    The name for the type of resource

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

fixedPath: :class:`str`
    The file path to the fixed resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`RemapTexEditResource`
    The built resource
    )doc"));
}

