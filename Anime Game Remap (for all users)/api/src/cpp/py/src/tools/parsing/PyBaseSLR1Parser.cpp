#include "PyBaseSLR1Parser.h"

#include <stdexcept>
#include <string>

#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    std::string &syntaxErrModulePath() {
        static std::string path;
        return path;
    }

    // Generates ids as real Python `str(uuid.uuid4())` values -- the pybind11-layer default id
    // generator for PyBaseSLR1Parser (#Id = py::object), matching the pure-Python original's own
    // default. Neither of AGRemapCore's own built-in defaults (IncIdGenerator, which needs
    // post-increment; UuidIdGenerator, which is std::string-only) apply to py::object, so this
    // exists specifically for this binding rather than reusing either.
    class PyUuidIdGenerator: public AGRC::BaseIdGenerator<py::object> {
        public:
            void reset() override {
                // no persistent state -- every id is independently generated
            }

            bool getId(py::object &result) override {
                py::object uuidModule = py::module_::import("uuid");
                result = py::str(uuidModule.attr("uuid4")());
                return true;
            }
    };

    PyBaseSLR1Parser::Productions parseProductionsDict(const py::dict &raw) {
        PyBaseSLR1Parser::Productions result;
        result.reserve(raw.size());

        for (auto item : raw) {
            py::object prodId = py::reinterpret_borrow<py::object>(item.first);
            py::tuple production = item.second.cast<py::tuple>();

            std::string lhs = production[0].cast<std::string>();
            std::vector<std::string> rhs = production[1].cast<std::vector<std::string>>();
            result.emplace(std::move(prodId), PyBaseSLR1Parser::Production{std::move(lhs), std::move(rhs)});
        }

        return result;
    }

    std::unique_ptr<PyBaseSLR1Parser> makeBaseSLR1Parser(const py::dict &productions, std::string startSymbol,
                                                          std::string startToken, std::string endToken, std::string nullToken, bool setup) {
        try {
            return std::make_unique<PyBaseSLR1Parser>(parseProductionsDict(productions), std::move(startSymbol),
                                                        std::move(startToken), std::move(endToken), std::move(nullToken), setup,
                                                        std::make_unique<PyUuidIdGenerator>(), std::make_unique<PyUuidIdGenerator>(),
                                                        std::make_unique<PyUuidIdGenerator>());
        } catch (const std::invalid_argument &e) {
            throw py::key_error(e.what());
        }
    }

}


void initSyntaxErrModulePath(pybind11::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    std::size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }

    syntaxErrModulePath() = parentPackage.empty() ? "exceptions.SyntaxErr" : (parentPackage + ".exceptions.SyntaxErr");
}

void raisePySyntaxErr(const AGRC::SyntaxErr &e) {
    py::object syntaxErrClass = py::module_::import(syntaxErrModulePath().c_str()).attr("SyntaxErr");
    py::object excInstance = syntaxErrClass(py::cast(e.ctx()), py::cast(e.token()), py::cast(e.process()));
    PyErr_SetObject(syntaxErrClass.ptr(), excInstance.ptr());
    throw py::error_already_set();
}


void initCppBaseSLR1Parser(pybind11::module_ &m) {
    // Uses this binding's own module name for the SyntaxErr lookup (see raisePySyntaxErr) --
    // whichever init function runs first among this/SympyParser/IfPredParser sets it, since
    // they all resolve to the same path from the same module.
    initSyntaxErrModulePath(m);

    auto cls = py::class_<PyBaseSLR1Parser>(m, "BaseSLR1Parser", R"doc(
The base class used for bottom-up `SLR(1)`_ parsing

Parameters
-----------
productions: Dict[Hashable, Tuple[:class:`str`, List[:class:`str`]]]
    The production rules of the `CFG (Context Free Grammer)`_, keyed by the id of each production rule

startSymbol: Hashable
    The starting non-terminal symbol

startToken: :class:`str`
    The name of the starting token for an input string

    **Default**: ``STARTTOKEN``

endToken: :class:`str`
    The name of the ending token for an input string

    **Default**: ``ENDTOKEN``

nullToken: :class:`str`
    The name for the empty token

    **Default**: ``EPSILON``

setup: :class:`bool`
    Whether to initialize all the setup for the parser automatically by calling :meth:`setup`

    **Default**: ``True``
    )doc")

        .def(py::init(&makeBaseSLR1Parser), py::arg("productions"), py::arg("startSymbol"),
             py::arg("startToken") = "STARTTOKEN", py::arg("endToken") = "ENDTOKEN", py::arg("nullToken") = "EPSILON",
             py::arg("setup") = true);

    bindBaseSLR1ParserCommonMethods<PyBaseSLR1Parser>(cls);
}
