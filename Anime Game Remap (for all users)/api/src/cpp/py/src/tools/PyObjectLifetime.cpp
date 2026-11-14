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

#include "PyObjectLifetime.h"

#include <vector>

#include "AGRemapCore/tools/ObjectLifetime.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// The Python types whose wrappers are invalidated: IfTemplate, IfTemplatePart (with IfContentPart and IfPredPart
// under it), IniSectionGraph and IniFile. Only these: a live object holding one of them as its first member sits at
// the same address, and its own wrapper must be left alone
std::vector<PyTypeObject*> watchedTypes;

bool isFinalizing() {
#if PY_VERSION_HEX >= 0x030D0000
    return Py_IsFinalizing() != 0;
#else
    return _Py_IsFinalizing() != 0;
#endif
}

bool isWatched(PyObject* wrapper) {
    for (PyTypeObject* type : watchedTypes) {
        if (PyType_IsSubtype(Py_TYPE(wrapper), type)) {
            return true;
        }
    }
    return false;
}

// Called as 'object' is freed. A wrapper that OWNS its object is taken out of pybind11's registry before the object
// is freed (pybind11's clear_instance), so whatever of a watched type is still registered at that address borrows it
void invalidateBorrowers(const void* object) {
    if (!Py_IsInitialized() || isFinalizing() || watchedTypes.empty()) {
        return;
    }

    // Every binding runs with the GIL held; a thread of the library's own freeing one of these objects takes it
    const bool acquire = PyGILState_Check() == 0;
    PyGILState_STATE state{};
    if (acquire) {
        state = PyGILState_Ensure();
    }

    std::vector<py::detail::instance*> borrowers;
    py::detail::with_instance_map(object, [&](py::detail::instance_map& instances) {
        auto range = instances.equal_range(const_cast<void*>(object));
        for (auto it = range.first; it != range.second; ++it) {
            if (!it->second->owned && isWatched(reinterpret_cast<PyObject*>(it->second))) {
                borrowers.push_back(it->second);
            }
        }
        return 0;
    });

    // Out of the registry, so the next object at this address gets a wrapper of its own, and, for a smart_holder type
    // (every watched one), pointing at nothing: pybind11 then raises on any use ("disowned") and frees it as a disowned
    // instance. A value pointer is never nulled under another holder, for which pybind11 would allocate raw storage on
    // the next use and read that instead
    for (py::detail::instance* borrower : borrowers) {
        for (auto valueAndHolder : py::detail::values_and_holders(borrower)) {
            if (!valueAndHolder) {
                continue;
            }
            if (valueAndHolder.instance_registered()) {
                py::detail::deregister_instance(borrower, valueAndHolder.value_ptr(), valueAndHolder.type);
                valueAndHolder.set_instance_registered(false);
            }
            if (valueAndHolder.type->holder_enum_v == py::detail::holder_enum_t::smart_holder && valueAndHolder.holder_constructed()) {
                valueAndHolder.holder<py::memory::smart_holder>().vptr.reset();
                valueAndHolder.value_ptr() = nullptr;
            }
        }
    }

    if (acquire) {
        PyGILState_Release(state);
    }
}

}


void initCppObjectLifetime(py::module_ &m) {
    for (const char* name : {"IfTemplate", "IfTemplatePart", "IniSectionGraph", "IniFile"}) {
        watchedTypes.push_back(reinterpret_cast<PyTypeObject*>(m.attr(name).ptr()));
    }
    AGRC::ObjectLifetime::setReleaseHook(&invalidateBorrowers);

    // Cleared as the interpreter exits: whatever the library frees after that is not wrapped by anything
    py::module_::import("atexit").attr("register")(py::cpp_function([]() {
        AGRC::ObjectLifetime::setReleaseHook(nullptr);
        watchedTypes.clear();
    }));
}
