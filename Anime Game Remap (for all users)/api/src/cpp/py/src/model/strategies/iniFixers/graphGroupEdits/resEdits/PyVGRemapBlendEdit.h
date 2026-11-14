#ifndef AGRemapPyBind_PyVGRemapBlendEdit_H
#define AGRemapPyBind_PyVGRemapBlendEdit_H

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

#include <optional>
#include <string>

#include <pybind11/pybind11.h>

#include "PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/VGRemapBlendEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing :cpp:class:`AGRemapCore::VGRemapBlendReplace` -- the core class builds the
 :class:`RemapBlendResource` itself, from the :cpp:class:`AGRemapCore::ModType` it holds, so the mixin
 only gives `Python`_ the shared resource-edit surface
 @endrst
 */
class PyVGRemapBlendReplace: public PyResEditMixin<AGRC::VGRemapBlendReplace<std::string, std::string>> {
    public:
        /**
         * @brief The exact Python ``ModType`` given, kept alive for as long as the core class points at it
         */
        py::object sourceModType;

        /**
         * @brief The exact Python callables given for the three function members, handed back as given
         */
        py::object fixFuncObj = py::none();
        py::object elementsObj = py::none();
        py::object pairedIdsObj = py::none();

        /**
         * @brief Constructs a new vertex-group-remapping `Blend`_ resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param modType The Python ``ModType`` being fixed from
         * @param fromVersion The game version the remap is read from, or ``std::nullopt``
         * @param toVersion The game version the remap is read to, or ``std::nullopt``
         * @param resType The name of the type of resource
         * @param resSubType The name of the subtype of the resource, or ``std::nullopt``
         * @param fromComp The specific component to remap from, or ``std::nullopt``
         * @param toComp The specific component to remap to, or ``std::nullopt``
         */
        PyVGRemapBlendReplace(py::object resModObj, py::object modType, std::optional<AGRC::Version> fromVersion,
                              std::optional<AGRC::Version> toVersion, std::string resType,
                              std::optional<std::string> resSubType, std::optional<std::string> fromComp,
                              std::optional<std::string> toComp);

        // Not copyable, like every other resource edit binding
        PyVGRemapBlendReplace(const PyVGRemapBlendReplace&) = delete;
        PyVGRemapBlendReplace& operator=(const PyVGRemapBlendReplace&) = delete;

        py::object pySelf() const override;
};


void initCppVGRemapBlendReplace(pybind11::module_ &m);

#endif
