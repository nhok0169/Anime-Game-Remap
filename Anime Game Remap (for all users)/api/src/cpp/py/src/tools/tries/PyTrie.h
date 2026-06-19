#ifndef PyTrie_H
#define PyTrie_H

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/tries/BaseTrie.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::BaseTrie<py::object>;

class PyTrie: public AGRC::BaseTrie<py::object> {
    public:
        using DupHandler = typename AGRC::BaseTrie<py::object>::DupHandler;

        PyTrie(const std::optional<std::unordered_map<std::string, py::object>> &data = std::nullopt,
               const std::optional<DupHandler> &handler = std::nullopt);

        virtual py::object pyOptGet(const std::string &keyword, bool errorOnNotFound = true, const py::object &defaultRes = py::none());
        virtual py::object pyGetItem(const std::string &keyword);
};


class PyBindTrie: public PyTrie {
    public:
        using PyTrie::PyTrie;

        py::object pyOptGet(const std::string &keyword, bool errorOnNotFound = true, const py::object &defaultRes = py::none()) override;
        py::object pyGetItem(const std::string &keyword) override;

        void clear() override;
        void build(const std::optional<std::unordered_map<std::string, py::object>> &data) override;
        bool add(const std::string &key, const py::object &val) override;
        bool contains(const std::string &keyword) override;
};


void initCppTrie(pybind11::module_ &m);

#endif