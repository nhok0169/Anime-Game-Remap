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

#ifndef AGRemapPyBind_PyVGSplitGroupResource_H
#define AGRemapPyBind_PyVGSplitGroupResource_H

#include <functional>
#include <string>

#include <pybind11/pybind11.h>

#include "PyIniGroupedResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"

/**
 * @brief
 @rst
 The `Python`_-facing :cpp:class:`AGRemapCore::VGSplitGroupResource` :raw-html:`<br />` :raw-html:`<br />`

 A :cpp:class:`PyIniGroupedResource` rather than the core class, because that is what
 ``ResGroupCollect``'s builder bridge (``PyGroupedResBuilder``) casts a built group to, and because
 its members live in the `Python`_ ``dict`` that bridge fills -- :cpp:func:`fixVGSplitGroup` reads
 them through the virtual ``memberResources()``, which is exactly why that accessor exists. The
 core class and this one share that one function, so the split is written once
 @endrst
 */
class PyVGSplitGroupResource: public PyIniGroupedResource, public AGRemapCore::RemapIniResourceMixin {
    public:
        PyVGSplitGroupResource(std::string name, pybind11::dict resources, AGRemapCore::VGSplitGroupConfig config,
                               std::function<bool(AGRemapCore::IniGroupedResource&)> fixFunc, bool isBuilt);

        AGRemapCore::VGSplitGroupConfig config;

        /**
         * @brief The `Python`_ line edits exactly as given, so they round-trip
         */
        pybind11::object texcoordLineEditObj;
        pybind11::object positionLineEditObj;

    protected:
        bool _fix() override;
};

/**
 * @brief Wraps a `Python`_ ``Callable[[bytes], bytes]`` as a line edit, or an empty one for ``None``
 */
AGRemapCore::VGSplitGroupConfig::LineEdit lineEditFromPy(const pybind11::object &edit);

void initCppVGSplitGroupResource(pybind11::module_ &m);

#endif
