#include "PyIniFixBuilder.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyBaseIniFixer.h"
#include "../PyStrategyFactory.h"
#include "../iniParsers/PyBaseIniParser.h"
#include "AGRemapCore/model/Version.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    /**
     * Wraps a Python callable as a core Factory.
     *
     * The callable is captured by value: a builder outlives the expression that made it (ModType
     * holds one in a shared_ptr), so borrowing would dangle.
     */
    AGRC::IniFixBuilder::Factory factoryFromPy(py::object factory) {
        return [factory = std::move(factory)](AGRC::BaseIniParser<>* parser,
                                              const std::string& toModName) -> std::shared_ptr<AGRC::BaseIniFixer<>> {
            // build()/buildAll() are plain C++ calls the core may make from anywhere, so the GIL
            // is not already held.
            py::gil_scoped_acquire gil;

            py::object result = factory(py::cast(parser, py::return_value_policy::reference),
                                        py::cast(toModName));

            // holdPyStrategy, not a plain shared_ptr cast -- see its own note on why that quietly
            // loses a Python subclass's identity.
            return holdPyStrategy<PyBaseIniFixer, AGRC::BaseIniFixer<>>(std::move(result));
        };
    }

}


void initCppIniFixBuilder(py::module_ &m) {
    // Opaque on purpose: the rows are C++ factories, and the only table that exists is
    // IniFixBuilderData's. Registering it is what lets 'builderArgs' hand back the real thing
    // rather than a bool.
    py::class_<AGRC::IniFixBuilder::ArgsRepo, py::smart_holder>(m, "CppIniFixBuilderArgs", R"doc(
The version-dependent lookup table a :class:`CppIniFixBuilder` resolves its factory from

Opaque: there is no way to build one from Python yet. It is exposed so a builder that *has* one --
every mod type from :meth:`CppGIBuilder.all` does -- can say so
    )doc");


    py::class_<AGRC::IniFixBuilder, py::smart_holder>(m, "CppIniFixBuilder", R"doc(
A factory that builds the :class:`CppBaseIniFixer` that fixes one mod onto another

The C++ counterpart to :class:`IniFixBuilder`, and what :attr:`CppModType.iniFixBuilder` holds. It
comes in the same two flavours:

* **Fixed** -- one factory used for every .ini file, whatever its version
* **Version-dependent** -- a lookup table consulted on every :meth:`build`

Unlike the parse and remove builders, one source mod may be fixed onto **several** targets, which
is what :meth:`buildAll` exists for -- normally you want that rather than :meth:`build`

Parameters
----------
factory: Optional[Callable[[:class:`CppBaseIniParser`, :class:`str`], Optional[:class:`BaseIniFixer`]]]
    Called to build each fixer, with the parser that read the .ini file and the name of the mod
    being fixed **to** :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which uses :meth:`defaultFactory`

.. note::
    Only the fixed flavour is constructible from Python -- see :class:`CppIniFixBuilderArgs`
    )doc")

        .def(py::init<>())

        .def(py::init([](py::object factory) {
                 if (factory.is_none()) {
                     return std::make_unique<AGRC::IniFixBuilder>();
                 }

                 return std::make_unique<AGRC::IniFixBuilder>(factoryFromPy(std::move(factory)));
             }),
             py::arg("factory") = py::none())

        .def_static("defaultFactory", []() {
                        AGRC::IniFixBuilder::Factory factory = AGRC::IniFixBuilder::defaultFactory();

                        return py::cpp_function(
                            [factory](AGRC::BaseIniParser<>* parser, const std::string& toModName) {
                                return factory(parser, toModName);
                            },
                            py::arg("parser"), py::arg("toModName"));
                    },
                    R"doc(
The factory used when none is supplied -- builds a :class:`GIMIFixer` owning its own fix context

Returns
-------
Callable[[:class:`CppBaseIniParser`, :class:`str`], :class:`CppBaseIniFixer`]
    The default factory
        )doc")

        .def_property_readonly("builderArgs",
                               [](const AGRC::IniFixBuilder& self) { return self.getBuilderArgs().get(); },
                               py::return_value_policy::reference_internal,
                               py::doc(R"doc(
The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder

:class:`CppIniFixBuilderArgs`
        )doc"))

        .def_property_readonly("errorOnNotFound", &AGRC::IniFixBuilder::getErrorOnNotFound,
                               py::doc(R"doc(
Whether :meth:`build` raises rather than falling back when the key has no row

:class:`bool`
        )doc"))

        .def("build", &AGRC::IniFixBuilder::build,
             py::arg("parser"), py::arg("fromModName"), py::arg("toModName"),
             py::arg("fromVersion") = py::none(), py::arg("toVersion") = py::none(),
             py::doc(R"doc(
Builds the fixer for **one** target mod

Use :meth:`buildAll` when the target is not known up front, which is the normal case

Parameters
----------
parser: :class:`CppBaseIniParser`
    The parser that read the .ini file being fixed

fromModName: :class:`str`
    The name of the mod being fixed **from**

toModName: :class:`str`
    The name of the mod being fixed **to**

fromVersion: Optional[:class:`CppVersion`]
    The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toVersion: Optional[:class:`CppVersion`]
    The game version to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`CppBaseIniFixer`
    The built fixer -- the very same object a Python factory returned, when one was given
        )doc"))

        .def("buildAll", &AGRC::IniFixBuilder::buildAll,
             py::arg("parser"), py::arg("fromModName"), py::arg("fromVersion") = py::none(),
             py::arg("toVersion") = py::none(), py::arg("filteredToModNames") = py::none(),
             py::doc(R"doc(
Builds one fixer per mod 'fromModName' can be fixed onto

Parameters
----------
parser: :class:`CppBaseIniParser`
    The parser that read the .ini file being fixed

fromModName: :class:`str`
    The name of the mod being fixed **from**

fromVersion: Optional[:class:`CppVersion`]
    The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

toVersion: Optional[:class:`CppVersion`]
    The game version to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

filteredToModNames: Optional[Set[:class:`str`]]
    Only build fixers for these target mods :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning every target

Returns
-------
List[Tuple[:class:`str`, :class:`CppBaseIniFixer`]]
    One ``(toModName, fixer)`` pair per target mod, in no particular order
        )doc"));
}
