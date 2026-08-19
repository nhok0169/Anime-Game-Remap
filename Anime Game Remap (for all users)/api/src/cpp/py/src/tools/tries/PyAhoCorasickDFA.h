#ifndef AGRemapPyBind_PyAhoCorasickDFA_H
#define AGRemapPyBind_PyAhoCorasickDFA_H

#include <optional>
#include <tuple>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


extern template class AGRC::BaseAhoCorasickDFA<py::object>;


class PyAhoCorasickDFA: public AGRC::BaseAhoCorasickDFA<py::object> {
    public:
        using DupHandler2 = typename AGRC::BaseAhoCorasickDFA<py::object>::DupHandler2;

        // Independent of TrieVal (always std::function<bool(const std::string&)>), so this alias
        // is valid regardless of which BaseAhoCorasickDFA<TrieVal> it's pulled from.
        using KeywordPredicate = typename AGRC::BaseAhoCorasickDFA<py::object>::KeywordPredicate;

        PyAhoCorasickDFA(const std::optional<std::unordered_map<std::string, py::object>> &data = std::nullopt,
                         const std::optional<DupHandler2> &handler = std::nullopt);

        virtual std::tuple<std::optional<std::string>, py::object> pyOptGet(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultRes = py::none());
        virtual py::object pyGetItem(const std::string &txt);
        virtual std::tuple<std::optional<std::string>, size_t> pyFind(const std::string &txt);

        virtual std::variant<std::tuple<std::optional<std::string>, size_t>, std::tuple<std::vector<std::string>, std::vector<size_t>>> pyFindMaximal(const std::string &txt, size_t count = 1, const std::optional<KeywordPredicate> &pred = std::nullopt);
        virtual std::variant<std::tuple<std::optional<std::string>, py::object>, std::tuple<std::vector<std::string>, std::vector<py::object>>> pyGetMaximal(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object(), size_t count = 1, const std::optional<KeywordPredicate> &pred = std::nullopt);
        virtual std::tuple<std::optional<std::string>, py::object> pyGet(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object());
        virtual std::unordered_map<std::string, py::object> pyGetAll(const std::string &txt);
        virtual std::optional<std::string> pyMaximalStartsWith(const std::string &txt);
        virtual py::object pyGetKeyVal(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object());
};


class PyBindAhoCorasickDFA: public PyAhoCorasickDFA {
    public:
        using PyAhoCorasickDFA::PyAhoCorasickDFA;

        std::tuple<std::optional<std::string>, py::object> pyOptGet(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultRes = py::none()) override;
        py::object pyGetItem(const std::string &txt) override;
        std::tuple<std::optional<std::string>, size_t> pyFind(const std::string &txt) override;

        void clear() override;
        size_t size() override;
        bool contains(const std::string &txt) override;
        void build(const std::optional<std::unordered_map<std::string, py::object>> &data = std::nullopt) override;
        bool add(const std::string &key, const py::object &val) override;

        std::unordered_map<std::string, std::vector<std::tuple<size_t, size_t>>> findAll(const std::string &txt) override;
        std::unordered_map<std::string, std::tuple<size_t, size_t>> findFirstAll(const std::string &txt) override;

        std::variant<std::tuple<std::optional<std::string>, size_t>, std::tuple<std::vector<std::string>, std::vector<size_t>>> pyFindMaximal(const std::string &txt, size_t count = 1, const std::optional<KeywordPredicate> &pred = std::nullopt) override;
        std::variant<std::tuple<std::optional<std::string>, py::object>, std::tuple<std::vector<std::string>, std::vector<py::object>>> pyGetMaximal(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object(), size_t count = 1, const std::optional<KeywordPredicate> &pred = std::nullopt) override;
        std::tuple<std::optional<std::string>, py::object> pyGet(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object()) override;
        std::unordered_map<std::string, py::object> pyGetAll(const std::string &txt) override;
        std::optional<std::string> pyMaximalStartsWith(const std::string &txt) override;
        py::object pyGetKeyVal(const std::string &txt, bool errorOnNotFound = true, const py::object &defaultVal = py::object()) override;
};


void initCppAhoCorasickDFA(pybind11::module_ &m);

#endif
