#include "PyStrategyOverrides.h"

#include <optional>
#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "AGRemapCore/constants/StrategyOverrides.h"
#include "AGRemapCore/model/Version.h"

#include "../model/PyVersion.h"
#include "../model/strategies/iniFixers/PyIniFixBuilder.h"
#include "../model/strategies/iniParsers/PyIniParseBuilder.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppStrategyOverrides(py::module_ &m) {
    py::class_<AGRC::StrategyOverrides>(m, "CppStrategyOverrides", R"doc(
Overrides a mod's parser or fixer at runtime, without rebuilding the C++ core

Every parser and fixer this library ships is compiled into ``AGRemapCore``, which is what makes a
run fast and what makes trying a new idea slow --- changing one character's fix is a rebuild.
Registering here takes precedence over the built-in row for a mod, so a new parse or fix can be
written in Python, run, and thrown away.

It is a prototyping aid. A fix worth keeping belongs in the C++ tables.

.. note::
    Version matching mirrors the built-in tables (:class:`ModDictAssets`): a request carrying a
    version takes the highest override at or below it, and a request carrying **no** version means
    "the newest", taking the highest override registered. An override registered *without* a
    version is the fallback, used only when no versioned one applied.

    Exact matching was tried first and is wrong for what this class is for: a run resolves a mod's
    version off the .ini file and normally passes no version at all, so an override registered for
    ``6.1`` --- the literal case "override Raiden 6.1" means --- fired zero times on an ordinary
    run.

.. warning::
    Not synchronised. Register and clear **around** a run, never during one.

:example:

.. code-block:: python

    import FixRaidenBoss2 as FRB

    class MyParser(FRB.GIMIParser):
        pass

    FRB.CppStrategyOverrides.setParser("Raiden", lambda iniFile, modTypeId: MyParser(iniFile), version = "6.1")
    # ... run the fix ...
    FRB.CppStrategyOverrides.clear()
)doc")
        .def_static("setParser",
                    [](std::string modName, py::object factory, const py::object &version) {
                        AGRC::StrategyOverrides::setParser(std::move(modName), parseVersionArg(version),
                                                            parseFactoryFromPy(std::move(factory)));
                    },
                    py::arg("modName"), py::arg("factory"), py::arg("version") = py::none(),
                    py::doc(R"doc(
Registers a parser factory for a mod

:param modName: The mod type the override applies to, eg. ``"Raiden"``
:type modName: :class:`str`

:param factory: Called as ``factory(iniFile, modTypeId)`` and must return a parser
:type factory: Callable[[:class:`CppIniFile`, Optional[:class:`int`]], :class:`BaseIniParser`]

:param version:
    The version to override from, or ``None`` for every version --- an override registered here
    applies to that version **and every later one**, until a higher override supersedes it, exactly
    as the built-in version tables resolve. **Default**: ``None``
:type version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
)doc"))

        .def_static("setFixer",
                    [](std::string fromModName, std::string toModName, py::object factory,
                       const py::object &version) {
                        AGRC::StrategyOverrides::setFixer(std::move(fromModName), std::move(toModName),
                                                           parseVersionArg(version),
                                                           fixFactoryFromPy(std::move(factory)));
                    },
                    py::arg("fromModName"), py::arg("toModName"), py::arg("factory"),
                    py::arg("version") = py::none(),
                    py::doc(R"doc(
Registers a fixer factory for a ``fromModName`` -> ``toModName`` remap

The target does not have to exist in the built-in table --- registering one the table has no row for
adds it, so a brand-new remap can be prototyped and not only an existing one replaced.

:param fromModName: The mod being fixed
:type fromModName: :class:`str`

:param toModName: The mod being fixed *to*
:type toModName: :class:`str`

:param factory: Called as ``factory(parser, toModName, modTypeId)`` and must return a fixer
:type factory: Callable[[:class:`BaseIniParser`, :class:`str`, Optional[:class:`int`]], :class:`BaseIniFixer`]

:param version:
    The version of 'fromModName' to override from, or ``None`` for every version --- only the
    *from* version is keyed on, since that is the one a run resolves off the ``.ini`` being fixed.
    Floor-matched, like :meth:`setParser`'s. **Default**: ``None``
:type version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
)doc"))

        .def_static("clear", &AGRC::StrategyOverrides::clear, py::doc(R"doc(
Removes every registered override
)doc"))

        .def_static("empty", &AGRC::StrategyOverrides::empty, py::doc(R"doc(
Whether nothing is registered

:rtype: :class:`bool`
)doc"));
}
