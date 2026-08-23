#include "PyIfTemplate.h"

#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyIfPredPart.h"
#include "PyIfTemplateTree.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

// Shared '.ini' domain customization -- see PyIfTemplate.h's own top-level note.
AGRC::IfTemplateRunConfig<py::object, py::object> makeRunConfig() {
    return AGRC::IfTemplateRunConfig<py::object, py::object>{
        py::cast(std::string("run")),
        [](const py::object &v) { return v.cast<std::string>(); },
        [](const std::string &name) { return py::cast(name); }
    };
}

// Takes ownership of every part in 'parts' (each already an existing IfPredPart/IfContentPart
// Python object) -- same ownership-transfer contract as IfContentPart's own 'content' constructor
// parameter: the original Python objects no longer own the underlying part afterward.
std::vector<std::unique_ptr<AGRC::IfTemplatePart>> parsePartsList(const py::object &parts) {
    std::vector<std::unique_ptr<AGRC::IfTemplatePart>> result;
    for (auto item : parts) {
        py::object part = py::reinterpret_borrow<py::object>(item);

        if (py::isinstance<AGRC::IfPredPart>(part)) {
            result.push_back(part.cast<std::unique_ptr<AGRC::IfPredPart>>());
        } else {
            result.push_back(part.cast<std::unique_ptr<PyIfContentPart>>());
        }
    }
    return result;
}

py::object partToPy(AGRC::IfTemplatePart *part) {
    if (part == nullptr) {
        return py::none();
    }
    if (auto *predPart = dynamic_cast<AGRC::IfPredPart*>(part)) {
        return py::cast(predPart, py::return_value_policy::reference);
    }
    return py::cast(static_cast<PyIfContentPart*>(part), py::return_value_policy::reference);
}

PyIfTemplate::TreeKind parseTreeKind(const std::string &s) {
    if (s == "basic") return PyIfTemplate::TreeKind::Basic;
    if (s == "norm") return PyIfTemplate::TreeKind::Norm;
    return PyIfTemplate::TreeKind::NonEmptyNode;
}

}


void initCppIfTemplate(pybind11::module_ &m) {
    py::class_<PyIfTemplate>(m, "IfTemplate", R"doc(
Data for storing information about a `section`_ in a .ini file

.. container:: operations

    **Supported Operations:**

    .. describe:: for element in x

        Iterates over all the parts of the :class:`IfTemplate`, ``x``

    .. describe:: len(x)

        Retrieves the number of parts

    .. describe:: x[num]

        Retrieves the part at index ``num`` (``None`` if that slot has been cleared)

    .. describe:: x[num] = newPart

        Sets the part at index ``num`` -- pass ``None`` to clear the slot

Parameters
----------
parts: List[Optional[:class:`IfTemplatePart`]]
    The individual parts of the :class:`IfTemplate` -- ownership of each is taken from the passed-in
    Python objects (the same contract as :class:`IfContentPart`'s own ``content`` parameter)

name: :class:`str`
    The name of the `section`_. **Default**: ``""``

prefix: :class:`str`
    Any prefix that precedes the content. **Default**: ``""``

suffix: :class:`str`
    Any suffix that follows the content. **Default**: ``""``
    )doc")

        .def(py::init([](py::object parts, std::string name, std::string prefix, std::string suffix) {
            return std::make_unique<PyIfTemplate>(parsePartsList(parts), makeRunConfig(), std::move(name),
                                                   PyIfTemplate::TreeKind::NonEmptyNode, std::move(prefix), std::move(suffix));
        }), py::arg("parts"), py::arg("name") = "", py::arg("prefix") = "", py::arg("suffix") = "")

        .def_readwrite("name", &PyIfTemplate::name, py::doc(R"doc(:class:`str`: The name of the `section`_)doc"))
        .def_readwrite("prefix", &PyIfTemplate::prefix, py::doc(R"doc(:class:`str`: Any prefix that precedes the content)doc"))
        .def_readwrite("suffix", &PyIfTemplate::suffix, py::doc(R"doc(:class:`str`: Any suffix that follows the content)doc"))

        .def_property("parts", [](PyIfTemplate &self) {
            py::list result;
            for (const auto &part : self.parts()) {
                result.append(partToPy(part.get()));
            }
            return result;
        }, [](PyIfTemplate &self, py::object newParts) {
            self.parts() = parsePartsList(newParts);
        }, py::doc(R"doc(List[Optional[:class:`IfTemplatePart`]]: The individual parts -- reassigning this does not itself update :attr:`tree`/:attr:`partsById`/:attr:`calledSubCommands`; call :meth:`rebuild` afterward if needed)doc"))

        .def_property_readonly("partsById", [](PyIfTemplate &self) {
            py::dict result;
            for (const auto &entry : self.partsById()) {
                result[py::cast(entry.first)] = partToPy(entry.second);
            }
            return result;
        }, py::doc(R"doc(Dict[:class:`int`, :class:`IfTemplatePart`]: The parts, keyed by their id)doc"))

        .def_property_readonly("calledSubCommands", [](PyIfTemplate &self) {
            py::dict result;
            for (const auto &entry : self.calledSubCommands()) {
                result[py::cast(entry.first)] = py::cast(entry.second);
            }
            return result;
        }, py::doc(R"doc(Dict[:class:`int`, List[:class:`str`]]: Any other `sections`_ this :class:`IfTemplate` references, by ``run =``)doc"))

        .def_property_readonly("tree", [](PyIfTemplate &self) -> PyIfTemplateTree* {
            return self.tree();
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(:class:`IfTemplateTree`: The parse tree for this :class:`IfTemplate`)doc"))

        .def("rebuild", &PyIfTemplate::rebuild,
    py::doc(R"doc(Updates the parse tree and the reference to other sections that this object calls)doc"))

        .def("refreshPartIds", &PyIfTemplate::refreshPartIds,
    py::doc(R"doc(Regenerates the ids for the parts)doc"))

        .def("deepcopy", &PyIfTemplate::deepcopy, py::arg("newPartIds") = true,
    py::doc(R"doc(
Performs a deep copy on the object

Parameters
----------
newPartIds: :class:`bool`
    Whether to refresh the ids for each part. **Default**: ``True``

Returns
-------
:class:`IfTemplate`
    The copied object
        )doc"))
        .def("__copy__", [](PyIfTemplate &self) { return self.deepcopy(true); })
        .def("__deepcopy__", [](PyIfTemplate &self, py::dict) { return self.deepcopy(true); }, py::arg("memo"))

        .def("normalize", &PyIfTemplate::normalize,
    py::doc(R"doc(Normalizes the branching structure within this :class:`IfTemplate` to follow :class:`IfTemplateNormTree`'s structure)doc"))

        .def("add", [](PyIfTemplate &self, py::object part, bool updateTree) {
            std::unique_ptr<AGRC::IfTemplatePart> owned;
            if (py::isinstance<AGRC::IfPredPart>(part)) {
                owned = part.cast<std::unique_ptr<AGRC::IfPredPart>>();
            } else {
                owned = part.cast<std::unique_ptr<PyIfContentPart>>();
            }
            return partToPy(self.add(std::move(owned), updateTree));
        }, py::arg("part"), py::arg("updateTree") = false,
    py::doc(R"doc(
Adds a part to the :class:`IfTemplate` -- ownership of 'part' is taken (same contract as the
constructor's own ``parts``)

Parameters
----------
part: :class:`IfTemplatePart`
    The part to add

updateTree: :class:`bool`
    Whether to update the parse tree. **Default**: ``False``
        )doc"))

        .def("find", [](PyIfTemplate &self, const py::object &pred, const py::object &postProcessor) {
            std::function<bool(PyIfTemplate&, size_t, AGRC::IfTemplatePart&)> predFn = nullptr;
            if (!pred.is_none()) {
                py::object predObj = pred;
                predFn = [predObj](PyIfTemplate &ifTemplate, size_t ind, AGRC::IfTemplatePart &part) {
                    return predObj(&ifTemplate, ind, partToPy(&part)).cast<bool>();
                };
            }

            std::function<py::object(PyIfTemplate&, size_t, AGRC::IfTemplatePart&)> postFn;
            if (postProcessor.is_none()) {
                postFn = [](PyIfTemplate&, size_t, AGRC::IfTemplatePart &part) {
                    return partToPy(&part);
                };
            } else {
                py::object postObj = postProcessor;
                postFn = [postObj](PyIfTemplate &ifTemplate, size_t ind, AGRC::IfTemplatePart &part) {
                    return postObj(&ifTemplate, ind, partToPy(&part));
                };
            }

            auto found = self.find<py::object>(predFn, postFn);
            py::dict result;
            for (auto &entry : found) {
                result[py::cast(entry.first)] = entry.second;
            }
            return result;
        }, py::arg("pred") = py::none(), py::arg("postProcessor") = py::none(),
    py::doc(R"doc(
Searches the :class:`IfTemplate` for parts that meet a certain condition

Parameters
----------
pred: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], :class:`bool`]]
    The predicate used to filter the parts. If ``None``, every part matches. **Default**: ``None``

postProcessor: Optional[Callable[[:class:`IfTemplate`, :class:`int`, :class:`IfTemplatePart`], Any]]
    Post-processes each matching part. If ``None``, returns the found part itself. **Default**: ``None``

Returns
-------
Dict[:class:`int`, Any]
    The filtered parts, keyed by their index
        )doc"))

        .def("addTopContentPart", [](PyIfTemplate &self) {
            return self.addTopContentPart();
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Adds a new :class:`IfContentPart` at the root of this :class:`IfTemplate`, if needed)doc"))

        .def("addKVPsToFront", [](PyIfTemplate &self, const std::vector<std::pair<py::object, py::object>> &kvps) {
            self.addKVPsToFront(kvps);
        }, py::arg("kvps"), py::doc(R"doc(Adds some KVPs to the top of this :class:`IfTemplate`)doc"))

        .def("addKVPToFront", &PyIfTemplate::addKVPToFront, py::arg("key"), py::arg("val"),
    py::doc(R"doc(Adds a KVP to the top of this :class:`IfTemplate`)doc"))

        .def("addBottomContentPart", [](PyIfTemplate &self) {
            return self.addBottomContentPart();
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Adds a new :class:`IfContentPart` at the very end of this :class:`IfTemplate`, if needed)doc"))

        .def("addKVPsToBack", [](PyIfTemplate &self, const std::vector<std::pair<py::object, py::object>> &kvps) {
            self.addKVPsToBack(kvps);
        }, py::arg("kvps"), py::doc(R"doc(Adds some KVPs to the bottom of this :class:`IfTemplate`)doc"))

        .def("addKVPToBack", &PyIfTemplate::addKVPToBack, py::arg("key"), py::arg("val"),
    py::doc(R"doc(Adds a KVP to the bottom of this :class:`IfTemplate`)doc"))

        .def("toStr", [](PyIfTemplate &self, const std::string &linePrefix, bool autoindent) {
            std::vector<std::string> lines;
            int tabCount = 0;

            if (!self.prefix.empty()) {
                lines.push_back(self.prefix);
            }
            lines.push_back("[" + self.name + "]");

            for (const auto &part : self.parts()) {
                if (part == nullptr) {
                    continue;
                }

                auto *predPart = dynamic_cast<AGRC::IfPredPart*>(part.get());
                bool isPredPart = predPart != nullptr;

                if (autoindent && isPredPart && predPart->type != AGRC::IfPredPartType::If) {
                    --tabCount;
                }

                std::string tabs(static_cast<size_t>(tabCount < 0 ? 0 : tabCount), '\t');
                std::string currentLinePrefix = linePrefix + tabs;

                if (isPredPart) {
                    lines.push_back(predPart->toStr(currentLinePrefix));
                } else {
                    auto *contentPart = static_cast<PyIfContentPart*>(part.get());
                    py::object pyPart = py::cast(contentPart, py::return_value_policy::reference);
                    lines.push_back(pyPart.attr("toStr")(py::arg("linePrefix") = currentLinePrefix).cast<std::string>());
                }

                if (autoindent && isPredPart && predPart->type != AGRC::IfPredPartType::EndIf) {
                    ++tabCount;
                }
            }

            if (!self.suffix.empty()) {
                lines.push_back(self.suffix);
            }

            std::string joined;
            for (size_t i = 0; i < lines.size(); ++i) {
                if (i > 0) joined += "\n";
                joined += lines[i];
            }
            return joined;
        }, py::arg("linePrefix") = "", py::arg("autoindent") = true,
    py::doc(R"doc(
Converts this :class:`IfTemplate` to a string

Parameters
----------
linePrefix: :class:`str`
    The string that will prefix every line. **Default**: ``""``

autoindent: :class:`bool`
    Whether to compute the proper tab indent. **Default**: ``True``

Returns
-------
:class:`str`
    The string representation
        )doc"))

        .def_static("build", [](const py::list &rawParts, std::string name, py::object ctx, py::object z3Ctx) {
            std::vector<std::pair<int, std::variant<std::string, tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual>>>> parsed;
            parsed.reserve(rawParts.size());

            for (auto item : rawParts) {
                auto tup = py::reinterpret_borrow<py::object>(item);
                int lineNo = tup.attr("__getitem__")(0).cast<int>();
                py::object rawPart = tup.attr("__getitem__")(1);

                if (py::isinstance<py::str>(rawPart)) {
                    parsed.emplace_back(lineNo, rawPart.cast<std::string>());
                    continue;
                }

                py::dict srcDict = rawPart.cast<py::dict>();
                tsl::ordered_map<py::object, std::vector<std::pair<long long, py::object>>, PyObjectHash, PyObjectEqual> src;
                for (auto srcItem : srcDict) {
                    py::object key = py::reinterpret_borrow<py::object>(srcItem.first);
                    auto vals = py::reinterpret_borrow<py::object>(srcItem.second).cast<std::vector<std::pair<long long, py::object>>>();
                    src[key] = std::move(vals);
                }
                parsed.emplace_back(lineNo, std::move(src));
            }

            AGRC::ParseContext *ctxPtr = ctx.is_none() ? nullptr : ctx.cast<AGRC::ParseContext*>();
            AGRC::Z3Context *z3CtxPtr = z3Ctx.is_none() ? nullptr : z3Ctx.cast<AGRC::Z3Context*>();

            return PyIfTemplate::build(parsed, makeRunConfig(), std::move(name), ctxPtr, z3CtxPtr);
        }, py::arg("rawParts"), py::arg("name") = "", py::arg("ctx") = py::none(), py::arg("z3Ctx") = py::none(),
    py::doc(R"doc(
Builds the :class:`IfTemplate`

Parameters
----------
rawParts: List[Tuple[:class:`int`, Union[:class:`str`, Dict[Any, List[Tuple[:class:`int`, Any]]]]]]
    The list of raw parts found -- each tuple is a starting line number paired with either a
    conditional-predicate string (parsed via :class:`IfPredPart`) or an :class:`IfContentPart`-shaped
    ``src`` dict

name: :class:`str`
    The name of the `section`_. **Default**: ``""``

ctx: Optional[:class:`ParseContext`]
    The context for parsing conditional predicates. **Default**: ``None``

z3Ctx: Optional[:class:`Z3Context`]
    The `Z3`_ context every parsed :class:`IfPredPart` will share. **Default**: ``None``
        )doc"))

        .def("__iter__", [](PyIfTemplate &self) {
            py::list result;
            for (const auto &part : self.parts()) {
                result.append(partToPy(part.get()));
            }
            return py::iter(result);
        })

        .def("__len__", [](PyIfTemplate &self) { return self.parts().size(); })

        .def("__getitem__", [](PyIfTemplate &self, size_t index) {
            return partToPy(self.parts().at(index).get());
        })

        .def("__setitem__", [](PyIfTemplate &self, size_t index, py::object value) {
            if (value.is_none()) {
                self.parts().at(index) = nullptr;
                return;
            }

            if (py::isinstance<AGRC::IfPredPart>(value)) {
                self.parts().at(index) = value.cast<std::unique_ptr<AGRC::IfPredPart>>();
            } else {
                self.parts().at(index) = value.cast<std::unique_ptr<PyIfContentPart>>();
            }
        });
}
