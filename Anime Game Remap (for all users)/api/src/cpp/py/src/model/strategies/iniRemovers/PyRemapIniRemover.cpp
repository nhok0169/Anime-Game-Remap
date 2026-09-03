#include "PyRemapIniRemover.h"

#include <utility>

#include <pybind11/stl.h>

#include "../../PyVersion.h"
#include "../../assets/PyModMappedAssets.h"
#include "../../iftemplate/PyIfContentPart.h"
#include "../../iftemplate/PyIfTemplate.h"
#include "AGRemapCore/constants/IniKeywords.h"


namespace {

// The .ini-domain customization points every Python-facing remover uses -- the py::object
// equivalents of the plain-string defaults AGRC::RemapIniRemover::defaultConfig builds. Same shape as
// PyGIMIParser's own makeParserConfig.
PyRemapIniRemoverCore::RemoverConfig buildConfig() {
    PyRemapIniRemoverCore::RemoverConfig result{};
    result.hashKey = AGRC::IniKeywords::Hash;
    result.filenameKey = AGRC::IniKeywords::Filename;
    result.runConfig = AGRC::IfTemplateRunConfig<std::string, std::string>{
        AGRC::IniKeywords::Run,
        [](const std::string &value) { return value; },
        [](const std::string &name) { return name; }
    };
    return result;
}

}


// ---------------------------------------------------------------------------------------
// PyIniRemoveContext
// ---------------------------------------------------------------------------------------

PyIniRemoveContext::PyIniRemoveContext(py::object ini): ini(std::move(ini)) {}


bool PyIniRemoveContext::hasIni() const {
    return !ini.is_none();
}


std::string PyIniRemoveContext::iniFolder() const {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("folder")).cast<std::string>();
}


std::optional<AGRC::Version> PyIniRemoveContext::version() const {
    if (!hasIni()) {
        return std::nullopt;
    }

    return parseVersionArg(ini.attr("version"));
}


py::object PyIniRemoveContext::modType() const {
    if (!hasIni()) {
        return py::none();
    }

    return ini.attr("availableType");
}


std::vector<PyIniRemoveContext::Assets*> PyIniRemoveContext::modTypeHashes() const {
    // At most one entry, unlike IniFileRemoveContext's -- the Python IniFile's 'availableType' is
    // singular. See IniRemoveContext::modTypeHashes' own note on why the interface is a vector.
    std::vector<Assets*> result;

    py::object type = modType();
    if (type.is_none()) {
        return result;
    }

    py::object hashes = type.attr("hashes");
    if (hashes.is_none()) {
        return result;
    }

    result.push_back(hashes.cast<PyModMappedAssets*>());
    return result;
}


std::vector<std::string> PyIniRemoveContext::readFileLines() {
    std::vector<std::string> result;
    if (!hasIni()) {
        return result;
    }

    // The equivalent of the pure-Python original's own "_readLines" decorator, done through Python
    // so the test harness's builtins.open/os.path mocks still apply.
    if (!ini.attr("fileLinesRead").cast<bool>()) {
        ini.attr("readFileLines")();
    }

    for (auto line : ini.attr("fileLines")) {
        result.push_back(py::str(line).cast<std::string>());
    }

    return result;
}


std::unordered_map<std::string, PyIniRemoveContext::Section*> PyIniRemoveContext::sectionIfTemplates() const {
    std::unordered_map<std::string, Section*> result;
    if (!hasIni()) {
        return result;
    }

    for (auto item : ini.attr("sectionIfTemplates").cast<py::dict>()) {
        result[py::str(item.first).cast<std::string>()] =
            py::reinterpret_borrow<py::object>(item.second).cast<PyIfTemplate*>();
    }

    return result;
}


std::string PyIniRemoveContext::fileTxt() const {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("fileTxt")).cast<std::string>();
}


void PyIniRemoveContext::setFileTxt(std::string txt) {
    if (!hasIni()) {
        return;
    }

    ini.attr("fileTxt") = py::str(txt);
}


std::string PyIniRemoveContext::write() {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("write")()).cast<std::string>();
}


void PyIniRemoveContext::clearRead() {
    if (!hasIni()) {
        return;
    }

    ini.attr("clearRead")();
}


void PyIniRemoveContext::setIsFixed(bool isFixed) {
    if (!hasIni()) {
        return;
    }

    // The private attribute, not the read-only 'isFixed' property -- exactly what the pure-Python
    // original assigns.
    ini.attr("_isFixed") = py::bool_(isFixed);
}


// ---------------------------------------------------------------------------------------
// PyRemapIniRemover
// ---------------------------------------------------------------------------------------

PyRemapIniRemoverCore::RemoverConfig PyRemapIniRemover::makeConfig() {
    return buildConfig();
}


PyRemapIniRemover::PyRemapIniRemover(py::object iniFile): PyRemapIniRemoverCore(nullptr, buildConfig()), ctxImpl(iniFile) {
    // Not a constructor argument on the base: PyBaseIniRemover's own Python state is what the
    // caller reads back, and the context is derived from it (see refresh).
    this->iniFileObj = std::move(iniFile);
    this->setContext(&ctxImpl);
}


void PyRemapIniRemover::refresh() {
    if (!ctxImpl.ini.is(this->iniFileObj)) {
        ctxImpl.ini = this->iniFileObj;
    }

    // setContext is idempotent, but a caller could have pointed this remover somewhere else.
    if (this->getContext() != &ctxImpl) {
        this->setContext(&ctxImpl);
    }
}


std::string PyRemapIniRemover::remove(bool parse, bool writeBack, AGRC::IniRemovalContext context) {
    refresh();
    return PyRemapIniRemoverCore::remove(parse, writeBack, context);
}


py::object PyRemapIniRemover::removedResourcesToPy() const {
    py::dict result;

    for (const auto &entry : this->getRemovedResources()) {
        py::list resources;

        for (const std::unique_ptr<AGRC::IniResource> &resource : entry.second) {
            // reference_internal: the resources are owned by this remover and stay valid until its
            // next remove() -- see RemapIniRemover::getRemovedResources' own note.
            resources.append(py::cast(resource.get(), py::return_value_policy::reference_internal, py::cast(this)));
        }

        result[py::str(entry.first)] = resources;
    }

    return result;
}


void initCppRemapIniRemover(py::module_ &m) {
    py::class_<PyRemapIniRemover, PyBaseIniRemover, py::smart_holder> cls(m, "RemapIniRemover", R"doc(
This class inherits from :class:`BaseIniRemover`

Class for removing the fixes from .ini files, by reachability rather than by name

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to remove the fix from
    )doc");

    cls.def(py::init<py::object>(), py::arg("iniFile") = py::none());

    bindBaseIniRemoverCommonMethods<PyRemapIniRemover>(cls, R"doc(
Removes the fix from the .ini file

The fix is whatever the fix boilerplate surrounds, plus any ``Remap``-named leftovers outside it
that carry one of the mod type's hashes -- together with everything those reference and everything
that references them.

Pass a :class:`IniRemovalContext` with ``ignoreModType`` set to drop the hash question and take
every ``Remap``-named leftover regardless of whose it is.

Parameters
----------
parse: :class:`bool`
    Ignored -- the resources that went with the removed `sections`_ are always collected, and are
    available from :meth:`RemapIniRemover.getRemovedResources` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

writeBack: :class:`bool`
    Whether to write back the new text content of the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

context: :class:`IniRemovalContext`
    The per-call options for this removal :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``, which means a default-constructed one -- ie. the strict rule above

Returns
-------
:class:`str`
    The new content of the .ini file
    )doc");

    cls.def("getTargetSectionNames", [](const PyRemapIniRemover &self) {
        return self.getTargetSectionNames();
    }, py::doc(R"doc(
The `sections`_ the last :meth:`RemapIniRemover.remove` treated as this software's own output

Returns
-------
List[:class:`str`]
    The names of the target `sections`_, in the order the .ini file declared them
    )doc"))

       .def("getRemovedSectionNames", [](const PyRemapIniRemover &self) {
           return self.getRemovedSectionNames();
       }, py::doc(R"doc(
Every `section`_ name the last :meth:`RemapIniRemover.remove` deleted

Returns
-------
List[:class:`str`]
    The names of the removed `sections`_, in the order the .ini file declared them
    )doc"))

       .def("getRemovedResources", [](const PyRemapIniRemover &self) {
           return self.removedResourcesToPy();
       }, py::doc(R"doc(
Every resource the last :meth:`RemapIniRemover.remove` took out with the `sections`_ that declared it

Returns
-------
Dict[:class:`str`, List[:class:`IniResource`]]
    The removed resources, keyed by the type of resource -- one of ``"blend"``, ``"position"``,
    ``"texcoord"``, ``"buf"``, ``"texEdit"``, ``"texAdd"``, ``"download"`` or ``"other"``, each of
    which names a :class:`RemapStats` attribute
    )doc"));
}
