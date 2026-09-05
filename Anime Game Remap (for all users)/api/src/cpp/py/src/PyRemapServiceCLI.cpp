#include "PyRemapServiceCLI.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/RemapService.h"
#include "AGRemapCore/RemapServiceCLIErrors.h"


namespace {

    // The fully-qualified dotted path to this 'core' module's own parent package -- derived once in
    // initCppRemapServiceCLI, the same way PyBufFile.cpp's bufFileErrorsParentPackage() is, and for
    // the same reason (this repo's Unit Tester harness imports the package as
    // 'src.py.FixRaidenBoss2' rather than a top-level 'FixRaidenBoss2').
    std::string &cliErrorsParentPackage() {
        static std::string path;
        return path;
    }

    // Re-raises one of RemapServiceCLI's conversion failures as the real Python exception, so that
    // a caller catches 'FixRaidenBoss2.exceptions.InvalidModType' rather than a bare RuntimeError.
    // The message itself is rebuilt by the Python class from the data the C++ one carried, which is
    // what keeps its wording living in exactly one place.
    [[noreturn]] void raisePyError(const std::string &moduleName, const std::string &argName, const std::string &argValue) {
        py::object cls = py::module_::import((cliErrorsParentPackage() + ".exceptions." + moduleName).c_str()).attr(moduleName.c_str());
        py::object excInstance = cls(py::arg(argName.c_str()) = py::cast(argValue));

        PyErr_SetObject(excInstance.get_type().ptr(), excInstance.ptr());
        throw py::error_already_set();
    }

    // Runs one of the CLI's virtual methods, translating whichever conversion failure it lets
    // through. Wrapped rather than registered as a global exception translator because these are
    // the only three places that can raise them, and a translator would need registering against a
    // Python class this module does not otherwise touch.
    template <typename Fn>
    void translatingErrors(Fn &&fn) {
        try {
            fn();
        } catch (const AGRC::InvalidModType &e) {
            raisePyError("InvalidModType", "type", e.modType());
        } catch (const AGRC::InvalidDownloadMode &e) {
            raisePyError("InvalidDownloadMode", "mode", e.downloadMode());
        } catch (const AGRC::InvalidVersion &e) {
            // A plain ValueError, not one of this package's own exceptions -- which is what the
            // pure-Python original raised for an unparseable version.
            throw py::value_error(e.what());
        }
    }
}


void initCppRemapServiceCLI(py::module_ &m) {
    std::string coreModuleName = m.attr("__name__").cast<std::string>();
    std::string parentPackage = coreModuleName;
    size_t lastDot = parentPackage.rfind('.');
    if (lastDot != std::string::npos) {
        parentPackage.erase(lastDot);
    } else {
        parentPackage.clear();
    }
    cliErrorsParentPackage() = parentPackage;

    py::class_<AGRC::RemapServiceCLI, PyBindRemapServiceCLI, py::smart_holder>(m, "CppRemapServiceCLI", R"doc(
The C++ half of the command-line front end for a remap

:raw-html:`<br />`

:class:`RemapService` is the model -- it takes already-typed data and knows nothing about where its
output goes. This class is the other side of that line: the view it reports through, and the log
file that output is written to

.. note::
    :meth:`addTips` is an **empty hook** here on purpose. A tip names a command-line option, and
    those belong to the argument parser, which is not in C++. Subclass this and override it -- which
    is exactly what the Python :class:`RemapServiceCLI` does

There are **two** ways to build one, and which you want depends on what you are holding

:raw-html:`<br />`

**From strings** -- what an argument parser produced. This is the one ``main.py`` uses, and it takes
the same arguments the pure-Python :class:`RemapService` did: ``path``, ``keepBackups``, ``fixOnly``,
``undoOnly``, ``hideOrig``, ``readAllInis``, ``types``, ``defaultType``, ``forcedType``, ``log``,
``verbose``, ``handleExceptions``, ``version``, ``remappedTypes``, ``proxy``, ``downloadMode`` and
``gameTypeId``. Mod type names/aliases become :class:`ModTypeId` ints (ignoring case and surrounding
whitespace), a `PEP 440`_ string becomes a :class:`Version`, and a mode name becomes a
:class:`DownloadMode`

.. note::
    Naming **no** types -- ``None`` or an empty list -- means *every* type, not none of them. That is
    the opposite of what an empty set means on :attr:`RemapService.fromModTypeIds`, and this
    constructor is where the ambiguity gets resolved

.. note::
    A string that resolves to nothing does **not** raise from the constructor. It is stored, and
    :meth:`fix` raises it -- see :attr:`hasErrorsBeforeFix`

:raw-html:`<br />`

**From an already-built model** -- ``service``, ``log``, ``verbose``. For a caller that has a
:class:`RemapService` in hand and only wants the log file and the reporting around it. The service's
``logger`` is overwritten with this object's own

:raw-html:`<br />`

In both, ``log`` is the **folder** to write the log file into, or ``None`` for no log -- the file's
own name is always ``RemapFixLog.txt`` and is never the caller's. ``verbose`` is independent of it:
a quiet run can still write a full log file
    )doc")

        .def(py::init<AGRC::RemapService, std::optional<std::string>, bool>(),
             py::arg("service"), py::arg("log") = py::none(), py::arg("verbose") = true)

        .def(py::init<std::optional<std::string>, bool, bool, bool, bool, bool,
                      std::optional<std::vector<std::string>>, std::optional<std::string>,
                      std::optional<std::string>, std::optional<std::string>, bool, bool,
                      std::optional<std::string>, std::optional<std::vector<std::string>>,
                      std::optional<std::string>, std::optional<std::string>, std::optional<int>>(),
             py::arg("path") = py::none(), py::arg("keepBackups") = true,
             py::arg("fixOnly") = false, py::arg("undoOnly") = false, py::arg("hideOrig") = false,
             py::arg("readAllInis") = false, py::arg("types") = py::none(),
             py::arg("defaultType") = py::none(), py::arg("forcedType") = py::none(),
             py::arg("log") = py::none(), py::arg("verbose") = true,
             py::arg("handleExceptions") = false, py::arg("version") = py::none(),
             py::arg("remappedTypes") = py::none(), py::arg("proxy") = py::none(),
             py::arg("downloadMode") = py::none(), py::arg("gameTypeId") = py::none())

        .def_property_readonly("hasErrorsBeforeFix", &AGRC::RemapServiceCLI::hasErrorsBeforeFix,
    py::doc(R"doc(:class:`bool`: Whether a string handed to the string constructor could not be converted

A conversion failure is **stored rather than raised**, and :meth:`fix` raises it. A half-built
object is still worth inspecting, a caller that never runs the fix never had a problem, and
:meth:`printModsToFix` has nothing truthful to print about mod types that did not resolve -- so it
is skipped entirely

Only the **first** failure is kept)doc"))

        .def("raiseErrorsBeforeFix", [](const AGRC::RemapServiceCLI &self) {
            translatingErrors([&self]() { self.raiseErrorsBeforeFix(); });
        }, py::doc(R"doc(
Raises the stored conversion failure, or does nothing when there was none
        )doc"))

        .def_readwrite("service", &AGRC::RemapServiceCLI::service,
    py::doc(R"doc(:class:`RemapService`: The remap this drives

Public rather than forwarded: every model-side option lives on it)doc"))

        .def_readwrite("logger", &AGRC::RemapServiceCLI::logger,
    py::doc(R"doc(:class:`Logger`: The view everything is reported through, shared with :attr:`service`)doc"))

        .def_property("verbose", &AGRC::RemapServiceCLI::getVerbose, &AGRC::RemapServiceCLI::setVerbose,
    py::doc(R"doc(:class:`bool`: Whether the fix prints its progress as it runs

Reads and writes :attr:`logger`'s own flag rather than a copy kept alongside it, so setting
``logger.verbose`` directly and setting this cannot disagree

Independent of whether a log file is being written: a quiet run still accumulates a full log)doc"))

        .def_property("log", &AGRC::RemapServiceCLI::getLog, &AGRC::RemapServiceCLI::setLog,
    py::doc(R"doc(Optional[:class:`str`]: The full path of the log file, or ``None`` when none is being written

Assign a **folder** to it; the file's name is always ``RemapFixLog.txt``. Assigning also flips the
logger's ``logTxt``, so turning logging on part-way through does not write an empty file)doc"))

        .def("fix", [](AGRC::RemapServiceCLI &self) {
            translatingErrors([&self]() { self.fix(); });
        }, py::doc(R"doc(
Runs the remap, prints the tips, then writes the log file

The log is written whatever happened -- including when the fix throws, since a run that failed is
the one whose log is worth keeping. The exception still propagates
        )doc"))

        .def("createLog", &AGRC::RemapServiceCLI::createLog, py::doc(R"doc(
Writes everything reported so far out to the log file

Does nothing when no log folder was given. Announces itself first, so the line naming the log file
is itself in the log
        )doc"))

        .def("printModsToFix", &AGRC::RemapServiceCLI::printModsToFix, py::doc(R"doc(
Prints the banner naming which types of mods this run will fix

Reads ``service.fromModTypeIds`` and prints one bullet per type, sorted by **name** rather than by
id

The two empty cases are not the same thing, and this is where a user sees the difference: ``None``
is "no filter" and prints *All mods*, while an empty set accepts nothing and prints *No mods*

Called at the top of :meth:`fix`, not from the constructor -- a virtual call from a C++ constructor
would never reach a subclass's override
        )doc"))

        .def("addTips", &AGRC::RemapServiceCLI::addTips, py::doc(R"doc(
Prints whatever the user might find useful to know next -- nothing at all in this class

Called by :meth:`fix` after the remap and **before** :meth:`createLog`, so the tips land in the log
file too, and only when the run finished with nothing skipped -- advice about what to try next is
noise on top of a run that already went wrong

Override this in a subclass that knows the command-line option names
        )doc"));
}
