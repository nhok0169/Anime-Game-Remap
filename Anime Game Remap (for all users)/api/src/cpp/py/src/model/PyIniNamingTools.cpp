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

#include "PyIniNamingTools.h"

#include <optional>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "AGRemapCore/model/IniNamingTools.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniNamingTools(pybind11::module_ &m) {
    py::class_<AGRC::IniNamingTools> cls(m, "IniNamingTools", R"doc(
The naming conventions a fix follows -- the ones every compiled fix uses

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

    cls.def_static("isDisabled", &AGRC::IniNamingTools::isDisabled, py::arg("name"),
                    py::doc("Whether 'name' -- a file, folder or `section`_ name -- is one a modder turned OFF: it starts with :attr:`FilePrefixes.DisabledPrefix`, ignoring case"));

    cls.def_static("isRemapCopyFile", &AGRC::IniNamingTools::isRemapCopyFile, py::arg("file"),
                    py::doc("Whether 'file' is a .ini file a fix GENERATED beside the mod's own -- ``<name>RemapFix<N>.ini``, ignoring case, with only the LAST ``RemapFix`` in the name counting"));

    cls.def_static("hasKeywordPrefix", &AGRC::IniNamingTools::hasKeywordPrefix, py::arg("value"), py::arg("keyword"),
                    py::doc("Whether a register's value is a 3dmigoto statement opening with 'keyword' -- ``ref``, :attr:`IniKeywords.Copy`, :attr:`IniKeywords.CopyDesc` -- rather than a resource name"));

    cls.def_static("sameKey", &AGRC::IniNamingTools::sameKey, py::arg("key1"), py::arg("key2"), py::doc(R"doc(
Whether two keys of a `section`_ are the same key as a mod's author may write it: without regard to case, as
3dmigoto reads a key, and with ``\\`` and ``/`` alike in a namespaced key (``Resource\\RabbitFX\\Lightmap`` is
spelled with either). An exact comparison that misses such a key reads exactly like a mod that binds nothing there

Parameters
----------
key1: :class:`str`
    A key

key2: :class:`str`
    Another key

Returns
-------
:class:`bool`
    Whether they are the same key
    )doc"));

    cls.def_static("boundResource", &AGRC::IniNamingTools::boundResource, py::arg("value"), py::doc(R"doc(
The resource a register's value binds: the value itself, or empty when there is no value or it is ``null`` (which
unbinds the register)

Parameters
----------
value: Optional[:class:`str`]
    The register's value, if it has one

Returns
-------
:class:`str`
    The resource, or empty
    )doc"));

    cls.def_static("removeRefPrefix", &AGRC::IniNamingTools::removeRefPrefix, py::arg("value"),
                    py::doc(R"doc(
The resource `section`_ a register binding names, with any ``ref`` prefix taken off: ``ref ResourceX`` and
``ResourceX`` both give ``ResourceX``

Both spellings bind the register by reference, so the two mean the same thing. The prefix is matched
without regard to case, as 3dmigoto reads it, and only as a whole word (``reference`` keeps its name).
:meth:`hasKeywordPrefix` with :attr:`IniKeywords.Ref` tells the two spellings apart

Parameters
----------
value: :class:`str`
    The value of the register binding

Returns
-------
:class:`str`
    The `section`_ name it refers to, stripped
    )doc"));

    cls.def_static("looksRemapped", &AGRC::IniNamingTools::looksRemapped,
                    py::arg("sectionName"), py::arg("modNames"), py::arg("remapKeyword") = AGRC::IniKeywords::Remap,
                    py::doc(R"doc(
Whether 'sectionName' is one a previous run of the fix wrote -- the shape :meth:`getRemapFixName` and
the names built on it produce

An undo asks it to decide what it removes outside the fix's own boilerplate, and a parser asks it
because the undo runs before the fix: a resource `section`_ a previous fix declared is about to be
removed, so a fix that binds its name would point at a `section`_ that is no longer there

.. note::
    ``Remap`` anywhere in the name is not the test: WWMI's own blend remap declares
    ``ResourceBlendRemapVertexVGBuffer`` on a mod that has never been fixed. The name has to carry
    ``<modName><remapKeyword>`` for one of 'modNames'

Parameters
----------
sectionName: :class:`str`
    The `section`_ name to test

modNames: List[:class:`str`]
    The mod names a fix of this file could have been written for -- its own and the ones it remaps
    onto. An empty list tests for 'remapKeyword' anywhere in the name instead

remapKeyword: :class:`str`
    The keyword a fix's names are built around :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``"Remap"``

Returns
-------
:class:`bool`
    Whether a fix of this file could have written 'sectionName'
    )doc"));

    cls.def_static("isDownloadFile", &AGRC::IniNamingTools::isDownloadFile, py::arg("path"),
                    py::doc("Whether a file is one the library DOWNLOADS rather than one a mod ships: its name carries :attr:`IniKeywords.RemapDL`"));

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

    cls.def_static("getRegTag", &AGRC::IniNamingTools::getRegTag, py::arg("reg"), py::doc(R"doc(
A register as it is written into a name: its dashes dropped and its first letter capitalised -- ``ps-t0`` is ``Pst0``

Parameters
----------
reg: :class:`str`
    The register

Returns
-------
:class:`str`
    The tag
    )doc"));

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
