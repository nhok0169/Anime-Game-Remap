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

#include "PyConstantEnums.h"

#include <optional>
#include <string>

#include <pybind11/stl.h>


AGRC::DownloadMode toDownloadMode(const py::object &mode, bool strict) {
    if (mode.is_none()) {
        return AGRC::DownloadMode::Normal;
    }
    if (py::isinstance<AGRC::DownloadMode>(mode)) {
        return mode.cast<AGRC::DownloadMode>();
    }

    std::string name = py::str(mode).cast<std::string>();
    std::optional<AGRC::DownloadMode> result = AGRC::DownloadModeTools::findByName(name);
    if (result.has_value()) {
        return *result;
    }
    if (strict) {
        throw py::value_error("Unknown download mode: '" + name + "'");
    }
    return AGRC::DownloadMode::Normal;
}


AGRC::RegFillMissingMode toRegFillMissingMode(const py::object &mode) {
    if (mode.is_none()) {
        return AGRC::RegFillMissingMode::FillMissing;
    }
    if (py::isinstance<AGRC::RegFillMissingMode>(mode)) {
        return mode.cast<AGRC::RegFillMissingMode>();
    }

    std::string name = py::str(mode).cast<std::string>();
    if (name == "topdownCover") {
        return AGRC::RegFillMissingMode::TopdownCover;
    }
    if (name == "bottomCover") {
        return AGRC::RegFillMissingMode::BottomCover;
    }
    return AGRC::RegFillMissingMode::FillMissing;
}


AGRC::IniGraphReplaceMode toIniGraphReplaceMode(const py::object &mode) {
    if (mode.is_none()) {
        return AGRC::IniGraphReplaceMode::Ignore;
    }
    if (py::isinstance<AGRC::IniGraphReplaceMode>(mode)) {
        return mode.cast<AGRC::IniGraphReplaceMode>();
    }

    std::string name = py::str(mode).cast<std::string>();
    if (name == "replace") {
        return AGRC::IniGraphReplaceMode::Replace;
    }
    if (name == "combine") {
        return AGRC::IniGraphReplaceMode::Combine;
    }
    return AGRC::IniGraphReplaceMode::Ignore;
}


AGRC::IfPredPartType toIfPredPartType(const py::object &type) {
    if (py::isinstance<AGRC::IfPredPartType>(type)) {
        return type.cast<AGRC::IfPredPartType>();
    }

    // An exact match on the keyword, deliberately NOT IfPredPartTypeTools::getType -- that is a
    // permissive classifier over raw predicate text ("iffy" reads as If), not a name lookup.
    std::string name = py::str(type).cast<std::string>();
    for (AGRC::IfPredPartType value : {AGRC::IfPredPartType::If, AGRC::IfPredPartType::Else,
                                       AGRC::IfPredPartType::Elif, AGRC::IfPredPartType::EndIf}) {
        if (AGRC::IfPredPartTypeTools::getName(value) == name) {
            return value;
        }
    }
    throw py::value_error("Unrecognized IfPredPartType: '" + name + "'");
}


void initCppConstantEnums(pybind11::module_ &m) {
    py::enum_<AGRC::DownloadMode>(m, "DownloadMode", R"doc(
The download mode of how the software handles file downloads

.. tip::
    A mode's name, as typed on the command line, is :meth:`DownloadModeTools.getName`;
    :meth:`DownloadModeTools.findByName` goes the other way
    )doc")
        .value("Disabled", AGRC::DownloadMode::Disabled, R"doc(Will not perform any file downloads for any mods)doc")
        .value("Normal", AGRC::DownloadMode::Normal,
               R"doc(Only perform file downloads at places in a .ini file where a resource is missing)doc")
        .value("Always", AGRC::DownloadMode::Always,
               R"doc(Will always perform file downloads for every mod, if possible, using pessimistic assumptions)doc");

    py::class_<AGRC::DownloadModeTools>(m, "DownloadModeTools", R"doc(
Tools for handling :class:`DownloadMode`
    )doc")
        .def_static("getName", &AGRC::DownloadModeTools::getName, py::arg("value"), py::doc(R"doc(
Retrieves the name a user types for a :class:`DownloadMode`

Parameters
----------
value: :class:`DownloadMode`
    The mode to retrieve the name for

Returns
-------
:class:`str`
    The name for 'value' (``"disabled"``, ``"normal"`` or ``"always"``)
        )doc"))

        .def_static("findByName", &AGRC::DownloadModeTools::findByName, py::arg("name"), py::doc(R"doc(
Finds the :class:`DownloadMode` a string names, ignoring case and surrounding whitespace

.. note::
    An **exact** match on the trimmed, lowercased text: ``"normally"`` names no mode

Parameters
----------
name: :class:`str`
    The text to look the mode up by

Returns
-------
Optional[:class:`DownloadMode`]
    The mode 'name' names, if any
        )doc"));

    py::enum_<AGRC::RegFillMissingMode>(m, "RegFillMissingMode", R"doc(
Different modes for handling :class:`IfContentPart`\s with missing registers

.. note::
    :attr:`BottomCover` is the mode for a draw call. :attr:`FillMissing` fills the FIRST content part
    of a section that lacks the register, which is the wrong end once something (a
    :class:`ResGroupCollect` splicing a collected register into an ``if`` block) has split the section
    )doc")
        .value("FillMissing", AGRC::RegFillMissingMode::FillMissing,
               R"doc(Finds every part missing the register and fills that part with it)doc")
        .value("TopdownCover", AGRC::RegFillMissingMode::TopdownCover,
               R"doc(If any part of the graph misses the register, adds it at the top of each root of the graph)doc")
        .value("BottomCover", AGRC::RegFillMissingMode::BottomCover,
               R"doc(Like TopdownCover, but adds the register at the bottom of each root, after everything the root sets up)doc");

    py::enum_<AGRC::IniGraphReplaceMode>(m, "IniGraphReplaceMode", R"doc(
Different replacement modes if the :class:`IniSectionGraph` already exists when :class:`BaseResEdit`
builds the corresponding graph
    )doc")
        .value("Ignore", AGRC::IniGraphReplaceMode::Ignore,
               R"doc(Use the previous existing graph and don't build a new graph)doc")
        .value("Replace", AGRC::IniGraphReplaceMode::Replace,
               R"doc(Replaces the existing graph with a newly built graph)doc")
        .value("Combine", AGRC::IniGraphReplaceMode::Combine,
               R"doc(Build a new graph and combine the new graph with the existing graph)doc");

    py::enum_<AGRC::IfPredPartType>(m, "IfPredPartType", R"doc(
The possible types for an :class:`IfPredPart`
    )doc")
        .value("If", AGRC::IfPredPartType::If, R"doc(The part starts with the keyword 'if')doc")
        .value("Else", AGRC::IfPredPartType::Else, R"doc(The part starts with the keyword 'else')doc")
        .value("Elif", AGRC::IfPredPartType::Elif,
               R"doc(The part starts with the keyword 'elif', or with 'else if')doc")
        .value("EndIf", AGRC::IfPredPartType::EndIf, R"doc(The part starts with the keyword 'endif')doc");

    py::class_<AGRC::IfPredPartTypeTools>(m, "IfPredPartTypeTools", R"doc(
Tools for handling :class:`IfPredPartType`
    )doc")
        .def_static("getName", &AGRC::IfPredPartTypeTools::getName, py::arg("value"), py::doc(R"doc(
Retrieves the keyword for an :class:`IfPredPartType`

Parameters
----------
value: :class:`IfPredPartType`
    The type to retrieve the keyword for

Returns
-------
:class:`str`
    The keyword for 'value' (``"if"``, ``"else"``, ``"elif"`` or ``"endif"``)
        )doc"))

        .def_static("getType", &AGRC::IfPredPartTypeTools::getType, py::arg("rawPredPart"), py::doc(R"doc(
Retrieves the type for an :class:`IfPredPart` from its raw predicate text

.. note::
    Matches by a case-insensitive *prefix* check, with no word boundary required after the keyword
    (``"iffy ..."`` still reads as :attr:`IfPredPartType.If`)

Parameters
----------
rawPredPart: :class:`str`
    The predicate string for the :class:`IfPredPart`

Returns
-------
Optional[:class:`IfPredPartType`]
    The type found based off 'rawPredPart'
        )doc"));
}
