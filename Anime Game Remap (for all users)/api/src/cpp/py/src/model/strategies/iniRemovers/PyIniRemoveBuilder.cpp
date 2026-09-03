#include "PyIniRemoveBuilder.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "PyBaseIniRemover.h"
#include "../PyStrategyFactory.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/files/IniFile.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


namespace {

    /**
     * Wraps a Python callable as a core Factory.
     *
     * The callable is held by value in the closure, so the builder keeps it alive for as long as
     * it lives -- a builder outlives the expression that constructed it (ModType holds one in a
     * shared_ptr), so borrowing would dangle.
     */
    AGRC::IniRemoveBuilder::Factory factoryFromPy(py::object factory) {
        return [factory = std::move(factory)](AGRC::IniFile* iniFile) -> std::shared_ptr<AGRC::BaseIniRemover<>> {
            // The GIL is not held here: build() is a plain C++ call that the core may make from
            // anywhere, so re-acquire before touching the callable.
            py::gil_scoped_acquire gil;

            py::object result = factory(py::cast(iniFile, py::return_value_policy::reference));

            // holdPyStrategy, not a plain shared_ptr cast -- see its own note on why that loses a
            // Python subclass's identity.
            return holdPyStrategy<PyBaseIniRemover, AGRC::BaseIniRemover<>>(std::move(result));
        };
    }

}


void initCppIniRemoveBuilder(py::module_ &m) {
    // Opaque on purpose: nothing on the Python side can build one of these tables yet (the rows
    // are C++ factories, and the table that exists is IniRemoveBuilderData's). Registering it
    // anyway is what lets getBuilderArgs() hand back the real thing instead of a bool.
    py::class_<AGRC::IniRemoveBuilder::ArgsRepo, py::smart_holder>(m, "CppIniRemoveBuilderArgs", R"doc(
The version-dependent lookup table a :class:`IniRemoveBuilder` resolves its factory from

Opaque: there is no way to build one from Python yet. It is exposed so that a builder which
*has* one -- every mod type from :meth:`GIBuilder.all` does -- can say so
    )doc");


    py::class_<AGRC::IniRemoveBuilder, py::smart_holder>(m, "IniRemoveBuilder", R"doc(
A factory that builds the :class:`CppBaseIniRemover` for one .ini file

What :attr:`ModType.iniRemoveBuilder` holds, and what the pure-Python builder of this name was
replaced by. It comes in two flavours:

* **Fixed** -- one factory used for every .ini file, whatever its version
* **Version-dependent** -- a lookup table consulted by ``(modName, version)`` on every
  :meth:`build`

Parameters
----------
factory: Optional[Callable[[:class:`IniFile`], Optional[:class:`BaseIniRemover`]]]
    Called to build each remover, with the .ini file the remover will act on :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which uses :meth:`defaultFactory`

.. note::
    Only the fixed flavour is constructible from Python -- see :class:`CppIniRemoveBuilderArgs`
    )doc")

        .def(py::init<>())

        .def(py::init([](py::object factory) {
                 if (factory.is_none()) {
                     return std::make_unique<AGRC::IniRemoveBuilder>();
                 }

                 return std::make_unique<AGRC::IniRemoveBuilder>(factoryFromPy(std::move(factory)));
             }),
             py::arg("factory") = py::none())

        .def_static("defaultFactory", []() {
                        AGRC::IniRemoveBuilder::Factory factory = AGRC::IniRemoveBuilder::defaultFactory();

                        // Wrapped rather than handed over raw: the raw factory returns a
                        // shared_ptr<BaseIniRemover<>>, and casting that is exactly what
                        // CppBaseIniRemover was registered for.
                        return py::cpp_function(
                            [factory](AGRC::IniFile* iniFile) { return factory(iniFile); },
                            py::arg("iniFile"));
                    },
                    R"doc(
The factory used when none is supplied -- builds a :class:`RemapIniRemover`

Returns
-------
Callable[[:class:`IniFile`], :class:`CppBaseIniRemover`]
    The default factory
        )doc")

        .def_property_readonly("builderArgs",
                               [](const AGRC::IniRemoveBuilder& self) { return self.getBuilderArgs().get(); },
                               py::return_value_policy::reference_internal,
                               py::doc(R"doc(
The lookup table this builder resolves factories from, or ``None`` if it is a fixed-factory builder

:class:`CppIniRemoveBuilderArgs`
        )doc"))

        .def_property_readonly("errorOnNotFound", &AGRC::IniRemoveBuilder::getErrorOnNotFound,
                               py::doc(R"doc(
Whether :meth:`build` raises rather than falling back when the mod name has no row

:class:`bool`
        )doc"))

        .def("build", &AGRC::IniRemoveBuilder::build,
             py::arg("iniFile"), py::arg("modName") = "", py::arg("version") = py::none(),
             py::doc(R"doc(
Builds the remover for one .ini file

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file the built remover will act on

modName: :class:`str`
    The name of the mod to build the remover for :raw-html:`<br />` :raw-html:`<br />`

    Ignored entirely by a fixed-factory builder :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

version: Optional[:class:`CppVersion`]
    The game version the .ini file originates from :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, meaning the latest listed version

Returns
-------
:class:`CppBaseIniRemover`
    The built remover -- a :class:`BaseIniRemover` when the factory came from Python
        )doc"));
}
