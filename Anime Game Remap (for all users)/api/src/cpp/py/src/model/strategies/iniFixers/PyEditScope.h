#ifndef AGRemapPyBind_PyEditScope_H
#define AGRemapPyBind_PyEditScope_H

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

#include <functional>
#include <vector>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/files/IniFile.h"
#include "AGRemapCore/model/strategies/ModType.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 * The Python objects of an edit run FROM Python: while an edit's Python ``edit`` / ``editFromIni`` runs, the
 * ``modType`` and ``ini`` it was called with and the Python objects of its graphs. A callable an edit holds (a value
 * producer, a key filter, a graph edit of the user's own) is handed these when a scope is open -- any object a Python
 * caller passes as ``modType`` reaches it as itself -- and the C++ objects cast to Python when the edit runs in a C++
 * fixer's list, where no scope is open
 */
class PyEditScope {
    public:
        py::object modType;
        py::object ini;

        /**
         * @brief The Python object of a graph of the call's groups, and a graph a Python edit hands back adopted into them
         */
        std::function<py::object(const void *graph)> graphToPy;
        std::function<void *(py::object graph)> adoptGraph;

        PyEditScope(py::object modType, py::object ini, std::function<py::object(const void *)> graphToPy = nullptr,
                    std::function<void *(py::object)> adoptGraph = nullptr):
            modType(std::move(modType)), ini(std::move(ini)), graphToPy(std::move(graphToPy)), adoptGraph(std::move(adoptGraph)) {
            stack().push_back(this);
        }

        ~PyEditScope() {
            stack().pop_back();
        }

        PyEditScope(const PyEditScope &) = delete;
        PyEditScope &operator=(const PyEditScope &) = delete;

        /**
         * @brief The innermost open scope, or ``nullptr`` when the edit runs from C++
         */
        static PyEditScope *current() {
            return stack().empty() ? nullptr : stack().back();
        }

    private:
        static std::vector<PyEditScope *> &stack() {
            thread_local std::vector<PyEditScope *> scopes;
            return scopes;
        }
};


/**
 * @brief
 * A parser's or fixer's list running from Python: the ``.ini`` file it runs on, its mod type, and the parse data its
 * results are filed into (:class:`IParseResult`). Open while the list runs, so any callable of its edits can read them
 * (``EditScope`` in Python). Kept apart from PyEditScope, which tells an edit run from Python from one run from C++
 */
class PyListScope {
    public:
        py::object ini;
        py::object modType;
        py::object parseData;

        PyListScope(py::object ini, py::object modType, py::object parseData):
            ini(std::move(ini)), modType(std::move(modType)), parseData(std::move(parseData)) {
            stack().push_back(this);
        }

        ~PyListScope() {
            stack().pop_back();
        }

        PyListScope(const PyListScope &) = delete;
        PyListScope &operator=(const PyListScope &) = delete;

        /**
         * @brief The innermost open list, or ``nullptr`` outside one
         */
        static PyListScope *current() {
            return stack().empty() ? nullptr : stack().back();
        }

    private:
        static std::vector<PyListScope *> &stack() {
            thread_local std::vector<PyListScope *> scopes;
            return scopes;
        }
};


/**
 * @brief The mod type a callable of an edit is handed: the Python caller's own, else the C++ one cast to Python
 */
inline py::object editModTypeToPy(const AGRC::ModType *modType) {
    if (PyEditScope *scope = PyEditScope::current(); scope != nullptr) {
        return scope->modType;
    }
    return modType == nullptr ? py::none() : py::cast(modType, py::return_value_policy::reference);
}

/**
 * @brief The ``.ini`` file a callable of an edit is handed: the Python caller's own, else the C++ one cast to Python
 */
inline py::object editIniToPy(AGRC::IniFile *ini) {
    if (PyEditScope *scope = PyEditScope::current(); scope != nullptr) {
        return scope->ini;
    }
    return ini == nullptr ? py::none() : py::cast(ini, py::return_value_policy::reference);
}

/**
 * @brief A Python ``modType`` / ``ini`` argument as the C++ pointer an edit takes, or ``nullptr`` for any other object
 */
inline const AGRC::ModType *editModTypeFromPy(const py::object &modType) {
    if (modType.is_none()) {
        return nullptr;
    }
    try {
        return modType.cast<const AGRC::ModType *>();
    } catch (const py::cast_error &) {
        return nullptr;
    }
}

inline AGRC::IniFile *editIniFromPy(const py::object &ini) {
    if (ini.is_none()) {
        return nullptr;
    }
    try {
        return ini.cast<AGRC::IniFile *>();
    } catch (const py::cast_error &) {
        return nullptr;
    }
}

#endif
