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

#ifndef AGRemapPyBind_PyVGMergeGroupResource_H
#define AGRemapPyBind_PyVGMergeGroupResource_H

#include <functional>
#include <string>

#include <pybind11/pybind11.h>

#include "PyIniGroupedResource.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/iniresources/VGMergeGroupResource.h"

/**
 * @brief
 @rst
 The `Python`_-facing :cpp:class:`AGRemapCore::VGMergeGroupResource` :raw-html:`<br />` :raw-html:`<br />`

 A :cpp:class:`PyIniGroupedResource` rather than the core class, for the same reason
 :cpp:class:`PyVGSplitGroupResource` is -- see there. The core class and this one share
 :cpp:func:`fixVGMergeGroup`, so the merge is written once
 @endrst
 */
class PyVGMergeGroupResource: public PyIniGroupedResource, public AGRemapCore::RemapIniResourceMixin {
    public:
        PyVGMergeGroupResource(std::string name, pybind11::dict resources, AGRemapCore::VGMergeGroupConfig config,
                               std::function<bool(AGRemapCore::IniGroupedResource&)> fixFunc, bool isBuilt);

        AGRemapCore::VGMergeGroupConfig config;

    protected:
        bool _fix() override;
};

void initCppVGMergeGroupResource(pybind11::module_ &m);

#endif
