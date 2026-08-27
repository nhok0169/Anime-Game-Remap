#ifndef AGRemapPyBind_PyIniGroupedResource_H
#define AGRemapPyBind_PyIniGroupedResource_H

#include <functional>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/iniresources/IniResource.h"

// A Python-facing counterpart to AGRemapCore::IniGroupedResource -- inherits #name/#fixFunc/
// #isBuilt/#fix directly from the real C++ class (these translate cleanly), but SHADOWS 'resources'
// (and 'isMissing'/'addResource', which touch it) with a real Python dict instead of the inherited
// std::unordered_map<std::string, std::unique_ptr<IniResource>>.
//
// Why: the class's own documented contract (and the inherited C++ member) is a homogeneous
// Dict[str, IniResource]. But its one real Python call site (ResGroupCollect.py) uses 'resources'
// as general-purpose scratch storage across a multi-phase algorithm -- keyed by arbitrary hashable
// (int, str, str) mod-object tuples (never plain type-name strings), holding placeholder tuples
// like (fileKey, rootSectionName, resRootLocation, partDepth) that only get overwritten with real
// resource objects much later (see ResGroupCollect._connectResGroups) -- and the whole object gets
// copy.deepcopy()'d mid-algorithm while still holding those placeholder tuples. None of that fits a
// strongly-typed unique_ptr map. A real Python dict (reference semantics -- mutations from Python
// persist directly, unlike an automatic-STL-caster property) models this correctly; the inherited
// AGRemapCore::IniGroupedResource::resources member stays there for a hypothetical pure-C++ caller
// but is never populated or read from the Python-facing surface. Don't "fix" this by deleting
// #resources and rebinding the inherited member instead.
class PyIniGroupedResource: public AGRemapCore::IniGroupedResource {
    public:
        pybind11::dict resources;

        PyIniGroupedResource(std::string name, pybind11::dict resources, std::function<bool(AGRemapCore::IniGroupedResource&)> fixFunc, bool isBuilt);

        virtual ~PyIniGroupedResource() = default;

        // Generic version of AGRemapCore::IniGroupedResource::isMissing -- checks key membership in
        // #resources for an arbitrary Python iterable of hashable keys (real usage: a Set of
        // (int, str, str) tuples), not a std::unordered_set<std::string>.
        bool isMissing(const pybind11::object& collected) const;

        // Generic version of AGRemapCore::IniGroupedResource::addResource -- stores an arbitrary
        // Python value (a placeholder tuple, or a real resource object) under an arbitrary hashable
        // Python key, instead of requiring a std::unique_ptr<IniResource>.
        void addResource(pybind11::object resType, pybind11::object resource);
};

// Shared __deepcopy__ implementation for PyIniGroupedResource and any Python-facing subclass of it
// (eg. PyRemapIniGroupedResource) -- reconstructs via 'self.__class__' (not a hardcoded type) so it
// dispatches to whichever concrete Python class 'self' actually is, and delegates to Python's own
// copy.deepcopy for #resources' contents. Safe because every real call site (ResGroupCollect.py)
// only ever deepcopy()s an instance whose #resources values are still plain placeholder tuples --
// see this header's own class-level doc comment for why. Bind this under "__deepcopy__" on each
// concrete class rather than duplicating the lambda body per class.
pybind11::object iniGroupedResourceDeepCopy(const PyIniGroupedResource& self, pybind11::object memo);

void initCppIniGroupedResource(pybind11::module_ &m);

#endif
