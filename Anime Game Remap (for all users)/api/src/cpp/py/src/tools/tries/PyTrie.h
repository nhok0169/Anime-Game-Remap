#ifndef AGRemapPyBind_PyTrie_H
#define AGRemapPyBind_PyTrie_H

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

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/tries/BaseTrie.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::BaseTrie<py::object>;

class PyTrie: public AGRC::BaseTrie<py::object> {
    public:
        using DupHandler2 = typename AGRC::BaseTrie<py::object>::DupHandler2;

        PyTrie(const std::optional<std::unordered_map<std::string, py::object>> &data = std::nullopt,
               const std::optional<DupHandler2> &handler = std::nullopt);

        virtual py::object pyOptGet(const std::string &keyword, bool errorOnNotFound = true, const py::object &defaultRes = py::object());
        virtual py::object pyGetItem(const std::string &keyword);
};


class PyBindTrie: public PyTrie {
    public:
        using PyTrie::PyTrie;

        py::object pyOptGet(const std::string &keyword, bool errorOnNotFound = true, const py::object &defaultRes = py::object()) override;
        py::object pyGetItem(const std::string &keyword) override;

        void clear() override;
        size_t size() override;
        void build(const std::optional<std::unordered_map<std::string, py::object>> &data) override;
        bool add(const std::string &key, const py::object &val) override;
        bool contains(const std::string &keyword) override;
};


void initCppTrie(pybind11::module_ &m);

#endif