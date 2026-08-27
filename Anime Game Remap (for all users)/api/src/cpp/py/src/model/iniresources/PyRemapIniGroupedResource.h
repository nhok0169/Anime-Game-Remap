#ifndef AGRemapPyBind_PyRemapIniGroupedResource_H
#define AGRemapPyBind_PyRemapIniGroupedResource_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "PyIniGroupedResource.h"

// A Python-facing counterpart to AGRemapCore::RemapIniGroupedResource. Composes
// PyIniGroupedResource + AGRemapCore::RemapIniResourceMixin directly, bypassing
// AGRemapCore::RemapIniGroupedResource entirely -- that core class adds nothing over its own two
// bases (`using IniGroupedResource::IniGroupedResource;` only, no overrides at all), so this is a
// faithful, exact-equivalent simplification, not a behavior change. Reuses PyIniGroupedResource's
// 'resources' scratch-dict shadow (see that class's own doc comment) rather than duplicating it.
class PyRemapIniGroupedResource: public PyIniGroupedResource, public AGRemapCore::RemapIniResourceMixin {
    public:
        using PyIniGroupedResource::PyIniGroupedResource;

        virtual ~PyRemapIniGroupedResource() = default;
};

void initCppRemapIniGroupedResource(pybind11::module_ &m);

#endif
