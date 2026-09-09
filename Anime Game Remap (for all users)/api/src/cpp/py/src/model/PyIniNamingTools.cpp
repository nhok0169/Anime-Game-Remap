#include "PyIniNamingTools.h"

#include <optional>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "AGRemapCore/model/IniNamingTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniNamingTools(pybind11::module_ &m) {
    // 'CppIniNamingTools', not 'IniNamingTools': the pure-Python IniNamingTools is still exported
    // under that name, and the two do NOT agree. Its getModSuffixedName returns
    // `name[:len(suffix)]` where it means `name[:-len(suffix)]`, so a name that ALREADY ends in the
    // suffix comes back truncated to its first few characters -- getRemapFixName(
    // "TextureOverrideGanyuTwilightFaceRemapFix", "Ganyu") is "TextureOGanyuRemapFix" there and
    // "TextureOverrideGanyuTwilightFaceGanyuRemapFix" here. The C++ port implements the documented
    // behaviour on the maintainer's explicit instruction; see IniNamingTools::getModSuffixedName.
    //
    // That difference is invisible until a section name reaches a rename already carrying the
    // suffix, which is exactly what happens to a section the parser INVENTS for a download.
    py::class_<AGRC::IniNamingTools> cls(m, "CppIniNamingTools", R"doc(
The naming conventions a fix follows, as the C++ core implements them

.. warning::
    Not the same as the pure-Python :class:`IniNamingTools`. That one's ``getModSuffixedName`` keeps
    the first ``len(suffix)`` characters of a name that already ends in the suffix, where it means to
    strip the suffix off the end -- a confirmed bug, contradicting its own docstring. This class
    implements the documented behaviour, and it is what every compiled fix actually uses, so a fix
    written in Python should use this one to match

Every method is static.
    )doc");

    cls.def_static("getResourceName", &AGRC::IniNamingTools::getResourceName, py::arg("name"),
                    py::doc("Retrieves the name of the resource `section`_ for some `section`_ name"));

    cls.def_static("removeResourceName", &AGRC::IniNamingTools::removeResourceName, py::arg("name"),
                    py::doc("Removes the resource prefix from some `section`_ name"));

    cls.def_static("getRemapElementName", &AGRC::IniNamingTools::getRemapElementName,
                    py::arg("name"), py::arg("elementName"), py::arg("modName") = "",
                    py::doc("The remapped name for some element (Blend/Position/Texcoord/IB) `section`_"));

    cls.def_static("getRemapBlendName", &AGRC::IniNamingTools::getRemapBlendName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``Blend.buf`` `section`_"));

    cls.def_static("getRemapPositionName", &AGRC::IniNamingTools::getRemapPositionName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``Position.buf`` `section`_"));

    cls.def_static("getRemapTexcoordName", &AGRC::IniNamingTools::getRemapTexcoordName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``Texcoord.buf`` `section`_"));

    cls.def_static("getRemapIbName", &AGRC::IniNamingTools::getRemapIbName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``.ib`` `section`_"));

    cls.def_static("getModSuffixedName", &AGRC::IniNamingTools::getModSuffixedName,
                    py::arg("name"), py::arg("suffix") = "", py::arg("modName") = "",
                    py::doc(R"doc(
Appends ``modName + suffix`` to 'name', replacing a trailing 'suffix' it already carries

.. note::
    This is the method the pure-Python :class:`IniNamingTools` gets wrong -- see this class's own
    warning
    )doc"));

    cls.def_static("getRemapFixName", &AGRC::IniNamingTools::getRemapFixName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some fixed `section`_"));

    cls.def_static("getRemapTexName", &AGRC::IniNamingTools::getRemapTexName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some texture `section`_"));

    cls.def_static("getRemapDLName", &AGRC::IniNamingTools::getRemapDLName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some downloaded resource `section`_"));

    cls.def_static("getRemapFixResourceName", &AGRC::IniNamingTools::getRemapFixResourceName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some fixed resource `section`_"));

    cls.def_static("getRemapTexResourceName", &AGRC::IniNamingTools::getRemapTexResourceName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some texture resource `section`_"));

    cls.def_static("getRemapDLResourceName", &AGRC::IniNamingTools::getRemapDLResourceName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some downloaded resource `section`_"));

    cls.def_static("getRemapBlendResourceName", &AGRC::IniNamingTools::getRemapBlendResourceName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``Blend.buf`` resource `section`_"));

    cls.def_static("getRemapPositionResourceName", &AGRC::IniNamingTools::getRemapPositionResourceName,
                    py::arg("name"), py::arg("modName") = "",
                    py::doc("The remapped name for some ``Position.buf`` resource `section`_"));

    cls.def_static("getFixedFile", &AGRC::IniNamingTools::getFixedFile,
                    py::arg("file"), py::arg("modName") = "", py::arg("fileExt") = py::none(),
                    py::doc("The name of the fixed file for some file"));

    cls.def_static("getFixedElementFile", &AGRC::IniNamingTools::getFixedElementFile,
                    py::arg("file"), py::arg("elementName"), py::arg("modName") = "",
                    py::arg("fileExt") = py::none(),
                    py::doc("The name of the fixed file for some element file"));

    cls.def_static("getFixedBlendFile", &AGRC::IniNamingTools::getFixedBlendFile,
                    py::arg("blendFile"), py::arg("modName") = "",
                    py::doc("The name of the fixed ``Blend.buf`` file"));

    cls.def_static("getFixedPositionFile", &AGRC::IniNamingTools::getFixedPositionFile,
                    py::arg("positionFile"), py::arg("modName") = "",
                    py::doc("The name of the fixed ``Position.buf`` file"));

    cls.def_static("getFixedTexFile", &AGRC::IniNamingTools::getFixedTexFile,
                    py::arg("texFile"), py::arg("modName") = "",
                    py::doc("The name of the fixed texture file"));

    cls.def_static("getTextureOverrideRemapFix", &AGRC::IniNamingTools::getTextureOverrideRemapFix,
                    py::arg("component"), py::arg("obj"), py::arg("modName") = "",
                    py::doc("The name of the ``TextureOverride`` `section`_ a fix invents for some mod object"));

    cls.def_static("getObjRemapFixName", [](const std::string &name, const std::string &modName,
                                             const py::object &objName, const py::object &newObjName) {
        auto toPair = [](const py::object &value) {
            py::sequence seq = value.cast<py::sequence>();
            return std::make_pair(py::str(seq[0]).cast<std::string>(), py::str(seq[1]).cast<std::string>());
        };

        return AGRC::IniNamingTools::getObjRemapFixName(name, modName, toPair(objName), toPair(newObjName));
    }, py::arg("name"), py::arg("modName"), py::arg("objName"), py::arg("newObjName"),
       py::doc("The remapped name for a `section`_ whose mod object is being swapped for another"));
}
