#include "PyIniTexModel.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/IniTexModel.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

tsl::ordered_map<int, std::vector<std::string>> intVectorMapFromDict(const py::dict &d) {
    tsl::ordered_map<int, std::vector<std::string>> result;
    for (auto item : d) {
        result.emplace(item.first.cast<int>(), item.second.cast<std::vector<std::string>>());
    }
    return result;
}

tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> nestedMapFromDict(const py::dict &d) {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> result;
    for (auto outer : d) {
        tsl::ordered_map<std::string, std::vector<std::string>> inner;
        for (auto item : py::cast<py::dict>(outer.second)) {
            inner.emplace(item.first.cast<std::string>(), item.second.cast<std::vector<std::string>>());
        }
        result.emplace(outer.first.cast<int>(), std::move(inner));
    }
    return result;
}

py::dict nestedMapToDict(const tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> &m) {
    py::dict d;
    for (const auto &[outerKey, inner] : m) {
        py::dict innerDict;
        for (const auto &[innerKey, val] : inner) {
            innerDict[py::str(innerKey)] = val;
        }
        d[py::int_(outerKey)] = innerDict;
    }
    return d;
}

// Takes ownership of every CppBaseTexEditor/subclass instance found in 'd' -- same
// disown-on-construction contract as PyIfTemplate.cpp's parsePartsList (real call sites all build
// these inline, one-shot; nothing found reusing the same editor instance across multiple owners --
// re-verify this assumption if a future caller ever does).
tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<AGRC::BaseTexEditor>>>> texEditsFromDict(const py::dict &d) {
    tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<AGRC::BaseTexEditor>>>> result;

    for (auto outer : d) {
        tsl::ordered_map<std::string, std::vector<std::unique_ptr<AGRC::BaseTexEditor>>> inner;

        for (auto item : py::cast<py::dict>(outer.second)) {
            std::vector<std::unique_ptr<AGRC::BaseTexEditor>> editors;

            for (auto editorItem : py::cast<py::list>(item.second)) {
                py::object editor = py::reinterpret_borrow<py::object>(editorItem);
                editors.push_back(editor.cast<std::unique_ptr<AGRC::BaseTexEditor>>());
            }

            inner.emplace(item.first.cast<std::string>(), std::move(editors));
        }

        result.emplace(outer.first.cast<int>(), std::move(inner));
    }

    return result;
}

// NOTE (known, deliberately-scoped-out limitation): returns references into the still-owned
// unique_ptr<BaseTexEditor> objects, with no keep-alive wiring back to the owning IniTexModel
// instance (see Architecture/CLAUDE.md's "pybind11 wrapper for a raw pointer only lives as long as
// something holds a real reference to it" section for the general hazard this can cause). Left
// unaddressed here because nothing in the current Python call sites actually reads texEdits back
// from a C++-backed IniTexModel yet (IniFile.py's own makeTexModel factory -- the only real
// producer of a texEdits-bearing model today -- still builds the pure-Python IniTexModelOld, not
// this class); revisit if/when a real caller needs to read this back.
py::dict texEditsToDict(const AGRC::IniTexModel &self) {
    py::dict d;
    for (const auto &[outerKey, inner] : self.texEdits) {
        py::dict innerDict;
        for (const auto &[innerKey, editors] : inner) {
            py::list editorList;
            for (const auto &editor : editors) {
                editorList.append(py::cast(editor.get(), py::return_value_policy::reference));
            }
            innerDict[py::str(innerKey)] = editorList;
        }
        d[py::int_(outerKey)] = innerDict;
    }
    return d;
}

}


void initCppIniTexModel(pybind11::module_ &m) {
    py::class_<AGRC::IniTexModel, AGRC::IniFixResourceModel>(m, "IniTexModel", R"doc(
This class inherits from :class:`IniFixResourceModel`

Contains data for editing some texture files in a .ini file
    )doc")

        .def(py::init([](std::string iniFolderPath, const py::dict &fixedPaths, const py::dict &texEdits, const std::optional<py::dict> &origPaths) {
            std::optional<tsl::ordered_map<int, std::vector<std::string>>> convertedOrigPaths;
            if (origPaths.has_value()) {
                convertedOrigPaths = intVectorMapFromDict(*origPaths);
            }
            // py::init(factory) returning std::unique_ptr<T> by value -- same
            // neither-copyable-nor-movable reasoning as IniGroupedResource's own binding
            // (texEdits is a map of vectors of unique_ptr<BaseTexEditor>).
            return std::make_unique<AGRC::IniTexModel>(std::move(iniFolderPath), nestedMapFromDict(fixedPaths), texEditsFromDict(texEdits), std::move(convertedOrigPaths));
        }), py::arg("iniFolderPath"), py::arg("fixedPaths"), py::arg("texEdits"), py::arg("origPaths") = py::none(), py::doc(R"doc(
Constructs new data for editing a texture file in a .ini file

Parameters
----------
iniFolderPath: :class:`str`
    The folder path to where the .ini file of the resource is located

fixedPaths: Dict[:class:`int`, Dict[:class:`str`, List[:class:`str`]]]
    See :class:`IniFixResourceModel`'s constructor

texEdits: Dict[:class:`int`, Dict[:class:`str`, List[:class:`CppBaseTexEditor`]]]
    The texture editors used to edit the texture -- the outer keys are the indices to the
    :class:`IfContentPart` that the ``.dds`` file appears in the :class:`IfTemplate` for some
    texture, the inner keys are the names for the type of mod to fix to, and the inner values are
    the different texture editors used on the ``.dds`` files. Ownership of each editor is
    transferred into this model

origPaths: Optional[Dict[:class:`int`, List[:class:`str`]]]
    See :class:`IniFixResourceModel`'s constructor

    **Default**: ``None``
        )doc"))

        .def_property("texEdits", &texEditsToDict,
            [](AGRC::IniTexModel &self, const py::dict &texEdits) { self.texEdits = texEditsFromDict(texEdits); },
    py::doc(R"doc(Dict[:class:`int`, Dict[:class:`str`, List[:class:`CppBaseTexEditor`]]]: The texture editors used to edit the texture)doc"))

        .def("clear", &AGRC::IniTexModel::clear, py::doc(R"doc(
Clears out all the path/texture-editor data stored
        )doc"));
}
