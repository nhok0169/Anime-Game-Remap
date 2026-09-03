#include "PyIniParseBuilder.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyBaseIniParser.h"
#include "../PyStrategyFactory.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    /**
     * Wraps a Python callable as a core Factory.
     *
     * The callable is captured by value: a builder outlives the expression that made it (ModType
     * holds one in a shared_ptr), so borrowing would dangle.
     */
    AGRC::IniParseBuilder::Factory factoryFromPy(py::object factory) {
        return [factory = std::move(factory)](AGRC::IniFile* iniFile,
                                              std::optional<int> modTypeId) -> std::shared_ptr<AGRC::BaseIniParser<>> {
            // build() is a plain C++ call the core may make from anywhere, so the GIL is not
            // already held.
            py::gil_scoped_acquire gil;

            py::object result = factory(py::cast(iniFile, py::return_value_policy::reference),
                                        modTypeId.has_value() ? py::cast(*modTypeId) : py::none());

            // holdPyStrategy, not a plain shared_ptr cast -- see its own note on why that quietly
            // loses a Python subclass's identity.
            return holdPyStrategy<PyBaseIniParser, AGRC::BaseIniParser<>>(std::move(result));
        };
    }

}


void initCppIniParseBuilder(py::module_ &m) {
    // Opaque on purpose: the rows are C++ factories, and the only table that exists is
    // IniParseBuilderData's. Registering it is what lets 'builderArgs' hand back the real thing
    // rather than a bool.
    py::class_<AGRC::IniParseBuilder::ArgsRepo, py::smart_holder>(m, "CppIniParseBuilderArgs", R"doc(
The version-dependent lookup table a :class:`CppIniParseBuilder` resolves its factory from

Opaque: there is no way to build one from Python yet. It is exposed so a builder that *has* one --
every mod type from :meth:`CppGIBuilder.all` does -- can say so
    )doc");


    py::class_<AGRC::IniParseBuilder, py::smart_holder>(m, "CppIniParseBuilder", R"doc(
A factory that builds the :class:`CppBaseIniParser` for one .ini file

The C++ counterpart to :class:`IniParseBuilder`, and what :attr:`CppModType.iniParseBuilder` holds.
It comes in the same two flavours:

* **Fixed** -- one factory used for every .ini file, whatever its version
* **Version-dependent** -- a lookup table consulted by ``(modName, version)`` on every
  :meth:`build`, so a 5.7-era .ini file gets a different parser than a 4.0-era one

Parameters
----------
factory: Optional[Callable[[:class:`IniFile`, Optional[:class:`int`]], Optional[:class:`BaseIniParser`]]]
    Called to build each parser, with the .ini file it will read and the id of the mod type it is
    being built for :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which uses :meth:`defaultFactory`

.. note::
    Only the fixed flavour is constructible from Python -- see :class:`CppIniParseBuilderArgs`
    )doc")

        .def(py::init<>())

        .def(py::init([](py::object factory) {
                 if (factory.is_none()) {
                     return std::make_unique<AGRC::IniParseBuilder>();
                 }

                 return std::make_unique<AGRC::IniParseBuilder>(factoryFromPy(std::move(factory)));
             }),
             py::arg("factory") = py::none())

        .def_static("defaultFactory", []() {
                        AGRC::IniParseBuilder::Factory factory = AGRC::IniParseBuilder::defaultFactory();

                        return py::cpp_function(
                            [factory](AGRC::IniFile* iniFile, std::optional<int> modTypeId) {
                                return factory(iniFile, modTypeId);
                            },
                            py::arg("iniFile"), py::arg("modTypeId") = py::none());
                    },
                    R"doc(
The factory used when none is supplied -- builds a :class:`GIMIParser` owning its own parse context

Returns
-------
Callable[[:class:`IniFile`, Optional[:class:`int`]], :class:`CppBaseIniParser`]
    The default factory
        )doc")

        .def_property_readonly("builderArgs",
                               [](const AGRC::IniParseBuilder& self) { return self.getBuilderArgs().get(); },
                               py::return_value_policy::reference_internal,
                               py::doc(R"doc(
The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder

:class:`CppIniParseBuilderArgs`
        )doc"))

        .def_property_readonly("errorOnNotFound", &AGRC::IniParseBuilder::getErrorOnNotFound,
                               py::doc(R"doc(
Whether :meth:`build` raises rather than falling back when the mod name has no row

:class:`bool`
        )doc"))

        .def("build", &AGRC::IniParseBuilder::build,
             py::arg("iniFile"), py::arg("modName"), py::arg("version") = py::none(),
             py::arg("modTypeId") = py::none(),
             py::doc(R"doc(
Builds the parser for one .ini file

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file the built parser will read

modName: :class:`str`
    The name of the mod to build the parser for :raw-html:`<br />` :raw-html:`<br />`

    Ignored entirely by a fixed-factory builder

version: Optional[:class:`CppVersion`]
    The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest listed version

modTypeId: Optional[:class:`int`]
    Which of the .ini file's mod types the parser is being built for :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`CppBaseIniParser`
    The built parser -- the very same object a Python factory returned, when one was given
        )doc"));
}
