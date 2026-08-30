#include "PyGIMIFixer.h"

#include <string>
#include <utility>

#include <pybind11/stl.h>

#include "graphGroupEdits/resEdits/PyResEdit.h"  // reuses pyCoreModule()
#include "../../PyIniGraphGroup.h"
#include "../../PyIniSectionGraph.h"
#include "../../iftemplate/PyIfTemplate.h"


namespace {

using ModObj = PyGIMIFixer::ModObj;


// The FixRaidenBoss2 package itself, captured lazily off the bound core module's own __name__ (so
// it resolves whether the package was imported as 'FixRaidenBoss2' or as the Unit Tester's
// 'src.py.FixRaidenBoss2'). Never released, for the same interpreter-shutdown reason PyResEdit's
// own cached objects aren't -- see its comment.
py::module_ pyPackageModule() {
    static py::module_ *slot = nullptr;
    if (slot == nullptr) {
        std::string name = py::str(pyCoreModule().attr("__name__")).cast<std::string>();
        const std::string suffix = ".core";
        if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
            name = name.substr(0, name.size() - suffix.size());
        }

        slot = new py::module_(py::module_::import(name.c_str()));
    }

    return *slot;
}


std::string remapFixCopySuffix() {
    static std::string *slot = nullptr;
    if (slot == nullptr) {
        slot = new std::string(py::str(pyPackageModule().attr("FileSuffixes").attr("RemapFixCopy").attr("value")).cast<std::string>());
    }

    return *slot;
}


std::string iniFileEncoding() {
    static std::string *slot = nullptr;
    if (slot == nullptr) {
        slot = new std::string(py::str(pyPackageModule().attr("FileEncodings").attr("UTF8").attr("value")).cast<std::string>());
    }

    return *slot;
}


// The .ini-domain customization points every Python-facing fixer uses. Rendering a section goes
// back through genuine Python attribute lookup on IfTemplate.toStr -- the same shape
// PyIniSectionGraph's own 'toStr' binding uses, and for the same reason: neither IfTemplate nor
// IfContentPart has a core toStr to call.
PyGIMIFixerCore::FixerConfig makeFixerConfig() {
    PyGIMIFixerCore::FixerConfig result{};
    result.sectionToStr = [](PyIfTemplate &section, const std::string &linePrefix, bool autoindent) {
        py::object sectionObj = py::cast(&section, py::return_value_policy::reference);
        return sectionObj.attr("toStr")(py::arg("linePrefix") = linePrefix,
                                         py::arg("autoindent") = autoindent).cast<std::string>();
    };
    return result;
}

}


// ---------------------------------------------------------------------------------------
// PyIniFixContext
// ---------------------------------------------------------------------------------------

PyIniFixContext::PyIniFixContext(py::object ini): ini(std::move(ini)) {}


bool PyIniFixContext::hasIni() const {
    return !ini.is_none();
}


py::object PyIniFixContext::modType() const {
    if (!hasIni()) {
        return py::none();
    }

    return ini.attr("availableType");
}


std::vector<std::string> PyIniFixContext::modsToFix() const {
    std::vector<std::string> result;

    py::object type = modType();
    if (type.is_none()) {
        return result;
    }

    for (auto item : type.attr("getModsToFix")()) {
        result.push_back(py::str(item).cast<std::string>());
    }

    return result;
}


std::optional<std::string> PyIniFixContext::fixedFilePath(std::size_t groupInd) const {
    if (!hasIni()) {
        return std::nullopt;
    }

    py::object filePath = ini.attr("filePath");
    if (filePath.is_none()) {
        return std::nullopt;
    }

    if (groupInd == 0) {
        return py::str(filePath.attr("path")).cast<std::string>();
    }

    // A copy, so renaming this group's destination can't disturb the .ini file's own FilePath --
    // the pure-Python original deep-copies once up front and mutates that copy as it walks.
    py::object copied = py::module_::import("copy").attr("deepcopy")(filePath);
    std::string baseName = py::str(filePath.attr("baseName")).cast<std::string>();
    copied.attr("baseName") = py::str(baseName + remapFixCopySuffix() + std::to_string(groupInd));

    return py::str(copied.attr("path")).cast<std::string>();
}


bool PyIniFixContext::fixedFileExists() const {
    if (!hasIni()) {
        return false;
    }

    py::object filePath = ini.attr("filePath");
    if (filePath.is_none()) {
        return false;
    }

    // Python's own os.path.exists, not std::filesystem -- see this class's own note on the test
    // harness's mocks.
    return py::module_::import("os").attr("path").attr("exists")(filePath.attr("path")).cast<bool>();
}


std::string PyIniFixContext::fileTxt() const {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("fileTxt")).cast<std::string>();
}


void PyIniFixContext::setFileTxt(std::string txt) {
    if (!hasIni()) {
        return;
    }

    ini.attr("fileTxt") = py::str(txt);
}


void PyIniFixContext::hideOriginalSections() {
    if (hasIni()) {
        ini.attr("hideOriginalSections")();
    }
}


void PyIniFixContext::disableIni() {
    if (hasIni()) {
        ini.attr("disIni")();
    }
}


void PyIniFixContext::log(const std::string &message) {
    if (hasIni()) {
        ini.attr("print")(py::str("log"), py::str(message));
    }
}


std::string PyIniFixContext::addFixBoilerPlate(const std::string &fix) const {
    if (!hasIni()) {
        return fix;
    }

    return py::str(ini.attr("addFixBoilerPlate")(py::arg("fix") = fix)).cast<std::string>();
}


void PyIniFixContext::writeFixedFile(const std::string &path, const std::string &content) {
    // Through builtins.open (resolved at call time) and its context-manager protocol, matching the
    // original's own 'with open(...) as f' -- the test harness patches exactly this.
    py::object file = py::module_::import("builtins").attr("open")(py::str(path), py::str("w"),
                                                                    py::arg("encoding") = iniFileEncoding());
    py::object entered = file.attr("__enter__")();
    entered.attr("write")(py::str(content));
    file.attr("__exit__")(py::none(), py::none(), py::none());
}


void PyIniFixContext::setIsFixed(bool isFixed) {
    if (hasIni()) {
        ini.attr("_isFixed") = py::bool_(isFixed);
    }
}


std::unique_ptr<PyIniFixContext::GraphGroups> PyIniFixContext::makeGraphGroups() {
    return std::make_unique<PyIniGraphGroups>(py::list());
}


// ---------------------------------------------------------------------------------------
// PyGIMIFixer
// ---------------------------------------------------------------------------------------

PyGIMIFixer::PyGIMIFixer(py::object parser, py::object graphGroupEdits, py::object modsToFix, py::object prevFixer):
    Core(nullptr, nullptr, {}, std::nullopt, nullptr, makeFixerConfig()),
    ctxImpl(py::none()),
    graphGroupEditsObj(graphGroupEdits.is_none() ? py::object(py::list()) : std::move(graphGroupEdits)),
    modsToFixObj(std::move(modsToFix)),
    prevFixerObj(std::move(prevFixer)) {

    this->parserObj = std::move(parser);
    if (!this->parserObj.is_none() && py::hasattr(this->parserObj, "_iniFile")) {
        this->iniFileObj = this->parserObj.attr("_iniFile");
    }

    // The context is a member, so it can only be handed to the core base after that base has
    // already been constructed -- hence nullptr above plus this.
    this->setCtx(&ctxImpl);
    refresh();
}


void PyGIMIFixer::refresh() {
    ctxImpl.ini = this->iniFileObj;

    if (modsToFixObj.is_none()) {
        this->modsToFix = std::nullopt;
    } else {
        std::vector<std::string> parsed;
        for (auto item : modsToFixObj) {
            parsed.push_back(py::str(item).cast<std::string>());
        }
        this->modsToFix = std::move(parsed);
    }

    // The core 'prevFixer' pointer is only ever read back through prevFixerObj (see getFix), but
    // keeping it in sync means a plain C++ reader of the member sees the truth too.
    this->prevFixer = prevFixerObj.is_none() ? nullptr : prevFixerObj.cast<PyGIMIFixer*>();

    // 'graphGroupEdits' stays empty on the core side: applyGraphGroupEdits is overridden here and
    // walks the Python list directly, so a pure-Python edit (which has no C++ base to cast to)
    // works exactly like a C++-backed one.
    this->graphGroupEdits.clear();
}


py::object PyGIMIFixer::graphGroupsToPy() const {
    auto *groups = static_cast<PyIniGraphGroups*>(this->graphGroups());
    if (groups == nullptr) {
        return py::list();
    }

    return groups->list();
}


void PyGIMIFixer::setGraphGroupsFromPy(py::object groups) {
    py::list asList = groups.is_none() ? py::list() : groups.cast<py::list>();
    this->graphGroups_ = std::make_unique<PyIniGraphGroups>(std::move(asList));
}


void PyGIMIFixer::applyGraphGroupEdits(const std::string &modName) {
    if (graphGroupEditsObj.is_none()) {
        return;
    }

    py::object modType = ctxImpl.modType();

    for (auto item : graphGroupEditsObj) {
        py::object edit = py::reinterpret_borrow<py::object>(item);
        if (edit.is_none()) {
            continue;
        }

        py::object groups = graphGroupsToPy();
        py::object result = edit.attr("editFromIni")(groups, ctxImpl.ini, modType, py::arg("modName") = modName);

        // Every edit in this codebase returns the same list it was handed, but a pure-Python one is
        // free to hand back a different one -- the original reassigns unconditionally, so do that.
        if (!result.is_none() && !result.is(groups)) {
            setGraphGroupsFromPy(std::move(result));
        }
    }
}


PyGIMIFixer::FixTargets PyGIMIFixer::getFix(ParseData &parseData, bool onlyEditObjGraphs) {
    (void)parseData;  // sourced from the parser instead -- see this class's own note
    refresh();

    if (!prevFixerObj.is_none()) {
        auto *prev = prevFixerObj.cast<PyGIMIFixer*>();
        ParseData empty;
        prev->getFix(empty, true);
        this->graphGroups_ = std::move(prev->graphGroups_);
        prev->clear();
    } else {
        this->graphGroups_ = ctxImpl.makeGraphGroups();
        this->graphGroups_->insertGroup(0);

        if (!this->parserObj.is_none() && py::hasattr(this->parserObj, "collectParseResult")) {
            for (auto groupItem : this->parserObj.attr("collectParseResult")()) {
                py::object group = py::reinterpret_borrow<py::object>(groupItem);

                for (auto graphItem : group.attr("graphs").cast<py::dict>()) {
                    py::object graphObj = py::reinterpret_borrow<py::object>(graphItem.second);
                    if (graphObj.is_none()) {
                        continue;
                    }

                    // Deep-copied, so editing them leaves the parser's own graphs alone -- see
                    // AGRemapCore::GIMIFixer's own note.
                    auto *srcGraph = graphObj.cast<PyIniSectionGraph*>();
                    this->graphGroups_->addGraph(0, PyIniGraphGroups::modObjFromPy(graphItem.first),
                                                  this->graphGroups_->deepcopyGraph(*srcGraph));
                }
            }
        }
    }

    for (const std::string &modName : this->getModsToFix()) {
        applyGraphGroupEdits(modName);
    }

    if (onlyEditObjGraphs) {
        return {};
    }

    FixTargets result;
    std::size_t groupCount = (this->graphGroups_ == nullptr) ? 0 : this->graphGroups_->size();
    for (std::size_t i = 0; i < groupCount; ++i) {
        result.push_back(ctxImpl.fixedFilePath(i));
    }

    return result;
}


py::object PyGIMIFixer::getFixToPy(bool onlyEditObjGraphs) {
    ParseData empty;
    FixTargets targets = getFix(empty, onlyEditObjGraphs);

    if (onlyEditObjGraphs) {
        return py::none();
    }

    py::list groups = graphGroupsToPy().cast<py::list>();
    py::dict result;

    for (std::size_t i = 0; i < targets.size(); ++i) {
        // Keyed by the file path when there is one, and by the group's own index otherwise --
        // exactly the pure-Python original's Dict[Union[str, int], IniGraphGroup].
        py::object key = targets[i].has_value() ? py::object(py::str(*targets[i]))
                                                 : py::object(py::int_(static_cast<int>(i)));
        result[key] = groups[i];
    }

    return result;
}


py::object PyGIMIFixer::fixToPy(bool keepBackup, bool fixOnly, bool hideOrig) {
    refresh();

    ParseData empty;
    this->fix(empty, keepBackup, fixOnly, hideOrig);

    py::dict result;
    const FixTargets &targets = this->fixTargets();
    const std::vector<std::string> &contents = this->fixedContents();

    for (std::size_t i = 0; i < contents.size(); ++i) {
        py::object key = (i < targets.size() && targets[i].has_value())
            ? py::object(py::str(*targets[i]))
            : py::object(py::int_(static_cast<int>(i)));
        result[key] = py::str(contents[i]);
    }

    return result;
}


void initCppGIMIFixer(pybind11::module_ &m) {
    auto cls = py::class_<PyGIMIFixer, PyBaseIniFixer>(m, "GIMIFixer", R"doc(
This class inherits from :class:`BaseIniFixer`

Fixes a .ini file used by a ``GIMI``-style importer

Parameters
----------
parser: :class:`GIMIParser`
    The associated parser to retrieve data for the fix

graphGroupEdits: Optional[List[:class:`BaseIniGraphGroupEdit`]]
    The edits to apply to the parsed caller/callee graphs, run in order, once for each mod being
    fixed to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

modsToFix: Optional[List[:class:`str`]]
    The names of the mods to fix to :raw-html:`<br />` :raw-html:`<br />`

    If this argument is ``None``, will ask the .ini file's own :class:`ModType` instead -- see
    :meth:`getModsToFix` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

prevFixer: Optional[:class:`GIMIFixer`]
    A fixer whose already-edited graph groups this one continues from instead of starting fresh
    from the parser :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object parser, py::object graphGroupEdits, py::object modsToFix, py::object prevFixer) {
        return std::make_unique<PyGIMIFixer>(std::move(parser), std::move(graphGroupEdits),
                                              std::move(modsToFix), std::move(prevFixer));
    }), py::arg("parser"), py::arg("graphGroupEdits") = py::none(), py::arg("modsToFix") = py::none(),
        py::arg("prevFixer") = py::none());

    bindBaseIniFixerCommonMethods<PyGIMIFixer>(cls, R"doc(
Fixes the .ini file

Parameters
----------
keepBackup: :class:`bool`
    Whether to keep backups for the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

fixOnly: :class:`bool`
    Whether to only fix the .ini file without undoing any fixes :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

hideOrig: :class:`bool`
    Whether to hide the mod for the original character :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Dict[Union[:class:`str`, :class:`int`], :class:`str`]
    The new content of the fixed .ini file(s) :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the file paths each fixed .ini file was written to. A .ini file with no path at
      all is keyed by its group's index instead, and nothing is written for it
    * The values are that file's new content, including the original content and the boilerplate
    )doc");

    cls.def_readwrite("graphGroupEdits", &PyGIMIFixer::graphGroupEditsObj,
            py::doc(R"doc(List[:class:`BaseIniGraphGroupEdit`]: The edits to apply to the parsed caller/callee graphs)doc"))

       .def_readwrite("modsToFix", &PyGIMIFixer::modsToFixObj,
            py::doc(R"doc(Optional[List[:class:`str`]]: The names of the mods to fix to, or ``None`` to ask the .ini file)doc"))

       .def_readwrite("prevFixer", &PyGIMIFixer::prevFixerObj,
            py::doc(R"doc(Optional[:class:`GIMIFixer`]: A fixer whose already-edited graph groups this one continues from)doc"))

       .def_property("graphGroups", [](const PyGIMIFixer &self) {
            return self.graphGroupsToPy();
        }, [](PyGIMIFixer &self, py::object groups) {
            self.setGraphGroupsFromPy(std::move(groups));
        }, py::doc(R"doc(
List[:class:`IniGraphGroup`]: The graph groups this fixer edited, one per .ini file the fix
produces -- empty until :meth:`getFix` or :meth:`fix` has run
        )doc"))

       .def("getModsToFix", [](PyGIMIFixer &self) {
            self.refresh();
            return self.getModsToFix();
        }, py::doc(R"doc(
Retrieves the mods to fix to

Returns
-------
List[:class:`str`]
    :attr:`modsToFix` when it was set explicitly, otherwise whatever the .ini file's own
    :class:`ModType` says -- empty when the .ini file was never classified
        )doc"))

       .def("getFix", [](PyGIMIFixer &self, bool onlyEditObjGraphs) {
            return self.getFixToPy(onlyEditObjGraphs);
        }, py::arg("onlyEditObjGraphs") = false, py::doc(R"doc(
Retrieves only the content of the fix, without writing anything

Parameters
----------
onlyEditObjGraphs: :class:`bool`
    Whether to only run :attr:`graphGroupEdits` :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``True``, returns nothing and the results are left on :attr:`graphGroups`
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Optional[Dict[Union[:class:`str`, :class:`int`], :class:`IniGraphGroup`]]
    The content of the fix :raw-html:`<br />` :raw-html:`<br />`

    * The keys are the file paths each group should be written to. A .ini file with no path at all
      is keyed by its group's index instead
    * The values are the edited graph groups themselves
        )doc"))

       .def("groupToStr", [](PyGIMIFixer &self, std::size_t groupInd) {
            return self.groupToStr(groupInd);
        }, py::arg("groupInd"), py::doc(R"doc(
Renders one of :attr:`graphGroups` to .ini text -- every graph in it, joined by blank lines

Parameters
----------
groupInd: :class:`int`
    Which group to render

Returns
-------
:class:`str`
    The rendered .ini text
        )doc"));
}
