#ifndef AGRemapPyBind_PyMultiModFixer_H
#define AGRemapPyBind_PyMultiModFixer_H

#include <string>
#include <vector>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniFixers/MultiModFixer.h"

#include "PyBaseIniFixer.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;

// Spliced on top of PyBaseIniFixer, the same way PyGIMIFixerCore is: that is what makes
// py::class_<PyMultiModFixer, PyBaseIniFixer> genuine C++ inheritance, and it is also what lets a
// Python-side fixer be one of this class's children.
using PyMultiModFixerCore = AGRC::MultiModFixer<std::string, std::string, std::hash<std::string>,
                                                 std::equal_to<std::string>, PyBaseIniFixer>;

class PyMultiModFixer: public PyMultiModFixerCore {
    public:
        using Core = PyMultiModFixerCore;

        // 'parser' is a py::object rather than a Parser*, matching PyGIMIFixer: a Python-side
        // fixer's parser is a Python object with no C++ counterpart to point at, and the bound
        // BaseIniParser is a PyBaseIniParser, not the raw core instantiation this class's
        // Child::Parser names.
        explicit PyMultiModFixer(Children children = {}, py::object parser = py::none());

    protected:
        // Overridden to forward through Python attribute lookup instead of getIniFile(). The core
        // implementation reads AGRemapCore::IniFile::filteredToModTypeIds off a C++ pointer, and a
        // Python-side fixer has none -- its .ini file is '_iniFile', a py::object. Same reason
        // every Py* strategy context forwards rather than reaching for a core pointer.
        std::vector<int> selectedChildIds() const override;
};

void initCppMultiModFixer(pybind11::module_ &m);

#endif
