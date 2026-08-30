#include "PyGIMIParser.h"

#include <string>
#include <unordered_set>
#include <utility>

#include <pybind11/stl.h>

#include "PyGIMISectionClassifier.h"
#include "../../PyIniGraphGroup.h"
#include "../../PyIniSectionGraph.h"
#include "../../PyVersion.h"
#include "../../assets/PyModMappedAssets.h"
#include "../../iftemplate/PyIfContentPart.h"
#include "../../iftemplate/PyIfContentPartColour.h"
#include "../../iftemplate/PyIfTemplate.h"
#include "../iniFixers/graphGroupEdits/resEdits/PyResEdit.h"  // reuses pyCoreModule()


namespace {

using ModObj = PyGIMIParser::ModObj;


// Same lenient (componentName, objectName) reading PyIniGraphGroups::modObjFromPy does -- see its
// own comment; a key that can't name a mod object simply never matches one.
ModObj modObjFromPy(const py::handle &value) {
    return PyIniGraphGroups::modObjFromPy(value);
}


py::tuple modObjToPy(const ModObj &modObj) {
    return PyIniGraphGroups::modObjToPy(modObj);
}


// The Python DownloadMode is a still-pure-Python Enum whose members carry the same three string
// values as AGRC::DownloadMode -- see that enum's own note. Mapped by value rather than by
// position, so adding a member on either side can't silently misalign the two.
AGRC::DownloadMode parseDownloadMode(const py::object &raw) {
    if (raw.is_none()) {
        return AGRC::DownloadMode::Normal;
    }

    py::object value = py::hasattr(raw, "value") ? raw.attr("value") : raw;
    std::string parsed = py::str(value).cast<std::string>();

    if (parsed == "disabled") {
        return AGRC::DownloadMode::Disabled;
    }
    if (parsed == "always") {
        return AGRC::DownloadMode::Always;
    }
    return AGRC::DownloadMode::Normal;
}


py::list makeInitialGroups() {
    py::list result;
    result.append(py::cast(std::make_unique<PyIniGraphGroup>(py::dict())));
    return result;
}


// The .ini-domain customization points every Python-facing parser uses -- the py::object
// equivalents of the plain-string defaults AGRC::GIMIParser::defaultConfig builds.
PyGIMIParserCore::ParserConfig makeParserConfig() {
    PyGIMIParserCore::ParserConfig result{};
    result.classifier = PyGIMISectionClassifier::makeConfig();
    result.runConfig = AGRC::IfTemplateRunConfig<py::object, py::object>{
        py::cast(AGRC::IniKeywords::Run),
        [](const py::object &value) { return py::str(value).cast<std::string>(); },
        [](const std::string &name) { return py::cast(name); }
    };
    result.valOfSectionName = [](const std::string &name) { return py::cast(name); };
    return result;
}


// Wraps one Python objTargetFuncs entry as a core ObjTargetFunc. 'parser' is captured rather than
// taken from the callback's own first argument because the core hands that over as the base type,
// which pybind11 has no registration for -- only the derived PyGIMIParser does.
PyGIMIParserCore::ObjTargetFunc makeObjTargetFunc(PyGIMIParser *parser, py::object func) {
    return [parser, func](PyGIMIParserCore&, const std::string &sectionName, PyGIMIParserCore::Section *section,
                           bool disjoint, PyGIMIParserCore::ContentPart *part, const PyGIMIParserCore::Colouring *colouring) {
        py::object parserObj = py::cast(parser, py::return_value_policy::reference);
        // py::object(...) on both branches deliberately: a bare `cond ? py::none() : obj`
        // collapses to py::none, whose converting constructor then rejects the real object at
        // runtime with "Object of type 'X' is not an instance of 'none'".
        py::object sectionObj = (section == nullptr)
            ? py::object(py::none())
            : py::object(py::cast(section, py::return_value_policy::reference));
        py::object partObj = (part == nullptr)
            ? py::object(py::none())
            : py::object(py::cast(part, py::return_value_policy::reference));
        py::object colouringObj = (colouring == nullptr)
            ? py::object(py::none())
            : py::object(py::cast(const_cast<PyGIMIParserCore::Colouring*>(colouring), py::return_value_policy::reference));

        py::object result = func(std::move(parserObj), py::str(sectionName), std::move(sectionObj), disjoint,
                                  std::move(partObj), std::move(colouringObj));

        std::vector<ModObj> parsed;
        if (result.is_none()) {
            return parsed;
        }

        for (auto item : result) {
            parsed.push_back(modObjFromPy(item));
        }

        return parsed;
    };
}

}


// ---------------------------------------------------------------------------------------
// PyIniParseContext
// ---------------------------------------------------------------------------------------

PyIniParseContext::PyIniParseContext(py::object ini): ini(std::move(ini)), groups(makeInitialGroups()) {}


bool PyIniParseContext::hasIni() const {
    return !ini.is_none();
}


std::string PyIniParseContext::iniFolder() const {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("folder")).cast<std::string>();
}


std::optional<AGRC::Version> PyIniParseContext::version() const {
    if (!hasIni()) {
        return std::nullopt;
    }

    return parseVersionArg(ini.attr("version"));
}


AGRC::DownloadMode PyIniParseContext::downloadMode() const {
    if (!hasIni()) {
        return AGRC::DownloadMode::Normal;
    }

    return parseDownloadMode(ini.attr("downloadMode"));
}


AGRC::Z3Context* PyIniParseContext::z3Ctx() const {
    if (!hasIni()) {
        return nullptr;
    }

    py::object ctx = ini.attr("_z3Ctx");
    if (ctx.is_none()) {
        return nullptr;
    }

    return ctx.cast<AGRC::Z3Context*>();
}


std::unordered_map<std::string, PyIniParseContext::Section*> PyIniParseContext::sectionIfTemplates() const {
    std::unordered_map<std::string, Section*> result;
    if (!hasIni()) {
        return result;
    }

    for (auto item : ini.attr("sectionIfTemplates").cast<py::dict>()) {
        result[py::str(item.first).cast<std::string>()] = py::reinterpret_borrow<py::object>(item.second).cast<PyIfTemplate*>();
    }

    return result;
}


std::vector<std::string> PyIniParseContext::sectionNames() const {
    std::vector<std::string> result;
    if (!hasIni()) {
        return result;
    }

    // A real Python dict, so this is genuinely declaration-ordered -- which is exactly why this
    // method exists separately from sectionIfTemplates(). See IniParseContext's own note.
    for (auto item : ini.attr("sectionIfTemplates").cast<py::dict>()) {
        result.push_back(py::str(item.first).cast<std::string>());
    }

    return result;
}


PyIniParseContext::Section* PyIniParseContext::getSection(const std::string &name) const {
    if (!hasIni()) {
        return nullptr;
    }

    py::dict sections = ini.attr("sectionIfTemplates").cast<py::dict>();
    py::str key(name);
    if (!sections.contains(key)) {
        return nullptr;
    }

    return sections[key].cast<PyIfTemplate*>();
}


PyIniParseContext::Section* PyIniParseContext::addSection(const std::string &name, std::unique_ptr<Section> section) {
    // Handing the unique_ptr to Python transfers ownership into a real IfTemplate wrapper, which
    // then lives in ini.sectionIfTemplates like every other section.
    return addSectionObj(name, py::cast(std::move(section)));
}


PyIniParseContext::Section* PyIniParseContext::addSectionObj(const std::string &name, py::object section) {
    if (!hasIni() || section.is_none()) {
        return nullptr;
    }

    ini.attr("sectionIfTemplates")[py::str(name)] = section;
    return section.cast<PyIfTemplate*>();
}


void PyIniParseContext::removeSection(const std::string &name) {
    if (!hasIni()) {
        return;
    }

    ini.attr("sectionIfTemplates").attr("pop")(py::str(name), py::none());
}


void PyIniParseContext::addFileDownload(std::unique_ptr<AGRC::IniResource> download) {
    addFileDownloadObj(py::cast(std::move(download)));
}


void PyIniParseContext::addFileDownloadObj(py::object download) {
    if (!hasIni() || download.is_none()) {
        return;
    }

    ini.attr("fileDownloads").attr("append")(std::move(download));
}


py::object PyIniParseContext::modType() const {
    if (!hasIni()) {
        return py::none();
    }

    return ini.attr("availableType");
}


bool PyIniParseContext::hasModType() const {
    return !modType().is_none();
}


std::string PyIniParseContext::modTypeName() const {
    py::object type = modType();
    if (type.is_none()) {
        return "";
    }

    return py::str(type.attr("name")).cast<std::string>();
}


PyIniParseContext::Assets* PyIniParseContext::modTypeHashes() const {
    py::object type = modType();
    if (type.is_none()) {
        return nullptr;
    }

    py::object hashes = type.attr("hashes");
    if (hashes.is_none()) {
        return nullptr;
    }

    return hashes.cast<PyModMappedAssets*>();
}


PyIniParseContext::Assets* PyIniParseContext::modTypeIndices() const {
    py::object type = modType();
    if (type.is_none()) {
        return nullptr;
    }

    py::object indices = type.attr("indices");
    if (indices.is_none()) {
        return nullptr;
    }

    return indices.cast<PyModMappedAssets*>();
}


PyIniParseContext::GraphGroups& PyIniParseContext::graphGroups() {
    return groups;
}


py::list PyIniParseContext::groupsList() const {
    return groups.list();
}


py::object PyIniParseContext::commandGraphs() const {
    py::list list = groups.list();
    if (py::len(list) == 0) {
        return py::dict();
    }

    return list[0].attr("graphs");
}


void PyIniParseContext::setCommandGraphs(py::object graphs) {
    py::list list = groups.list();
    py::object group = py::cast(std::make_unique<PyIniGraphGroup>(graphs.is_none() ? py::dict() : graphs.cast<py::dict>()));

    if (py::len(list) == 0) {
        list.append(std::move(group));
        return;
    }

    list[0] = std::move(group);
}


// ---------------------------------------------------------------------------------------
// PyIniParseDownloadData
// ---------------------------------------------------------------------------------------

PyIniParseDownloadData::PyIniParseDownloadData(py::object downloadData): downloadData(std::move(downloadData)) {}


std::string PyIniParseDownloadData::name() const {
    return py::str(downloadData.attr("name")).cast<std::string>();
}


bool PyIniParseDownloadData::refToSection() const {
    return downloadData.attr("refToSection").cast<bool>();
}


void PyIniParseDownloadData::addToPart(ContentPart &part, const py::object &key, const py::object &val) {
    downloadData.attr("addToPart")(py::cast(&part, py::return_value_policy::reference), key, val);
}


void PyIniParseDownloadData::addToSection(Section &section, const py::object &key, const py::object &val) {
    downloadData.attr("addToSection")(py::cast(&section, py::return_value_policy::reference), key, val);
}


PyIniParseDownloadData::Section* PyIniParseDownloadData::createResSection(const std::string &sectionName, Context &ctx) {
    // 'createResSection' is overridable on the Python side, so the section has to come from there
    // rather than being rebuilt here -- and it comes back as a Python object that must be stored
    // as-is, hence addSectionObj rather than the interface's unique_ptr-taking addSection.
    py::object section = downloadData.attr("createResSection")(py::str(sectionName));
    return static_cast<PyIniParseContext&>(ctx).addSectionObj(sectionName, std::move(section));
}


void PyIniParseDownloadData::addFileDownload(Context &ctx, const std::string &iniFolder) {
    auto &pyCtx = static_cast<PyIniParseContext&>(ctx);
    py::object download = downloadData.attr("download");
    if (download.is_none()) {
        return;
    }

    // Built through the bound core module rather than by constructing AGRC::RemapIniDownload
    // directly, exactly as the pure-Python original did -- the FileDownload it takes ownership of
    // is the caller's own Python object, not a C++ copy of it.
    py::object remapIniDownload = pyCoreModule().attr("RemapIniDownload");
    pyCtx.addFileDownloadObj(remapIniDownload(py::str(iniFolder), download.attr("filename"), download));
}


// ---------------------------------------------------------------------------------------
// PyGIMIParser
// ---------------------------------------------------------------------------------------

PyGIMIParser::PyGIMIParser(py::object iniFile, py::object modObjs, py::object objTargetFuncs, py::object downloads,
                             py::object commandEdits, bool makeGlobalGraph, bool disjointModObjs, bool trackKeys,
                             py::object keysToTrack):
    Core(nullptr, {}, {}, {}, nullptr, makeGlobalGraph, disjointModObjs, trackKeys, std::nullopt, makeParserConfig()),
    ctxImpl(iniFile),
    modObjsObj(modObjs.is_none() ? py::object(py::set()) : std::move(modObjs)),
    objTargetFuncsObj(objTargetFuncs.is_none() ? py::object(py::list()) : std::move(objTargetFuncs)),
    downloadsObj(downloads.is_none() ? py::object(py::dict()) : std::move(downloads)),
    commandEditsObj(std::move(commandEdits)),
    keysToTrackObj(std::move(keysToTrack)) {

    // The context is a member, so it can only be handed to the core base after that base has
    // already been constructed -- hence nullptr above plus this.
    this->setCtx(&ctxImpl);
    this->iniFileObj = std::move(iniFile);

    refresh();
}


void PyGIMIParser::refresh() {
    std::vector<ModObj> parsedModObjs;
    if (!modObjsObj.is_none()) {
        for (auto item : modObjsObj) {
            parsedModObjs.push_back(modObjFromPy(item));
        }
    }
    this->setModObjs(std::move(parsedModObjs));

    if (keysToTrackObj.is_none()) {
        this->keysToTrack = std::nullopt;
    } else {
        std::unordered_set<py::object, PyObjectHash, PyObjectEqual> parsedKeys;
        for (auto item : keysToTrackObj) {
            parsedKeys.insert(py::reinterpret_borrow<py::object>(item));
        }
        this->keysToTrack = std::move(parsedKeys);
    }

    py::object currentModType = ctxImpl.modType();

    this->objTargetFuncs.clear();
    if (!objTargetFuncsObj.is_none()) {
        for (auto item : objTargetFuncsObj) {
            py::object func = py::reinterpret_borrow<py::object>(item);

            // The pure-Python original re-points every GIMISectionClassifier at the mod type's
            // current assets before running it (and blows up with an AttributeError when the .ini
            // file was never classified -- guarded against here rather than reproduced).
            if (py::isinstance<PyGIMISectionClassifier>(func) && !currentModType.is_none()) {
                func.attr("hashes") = currentModType.attr("hashes");
                func.attr("indices") = currentModType.attr("indices");
            }

            this->objTargetFuncs.push_back(makeObjTargetFunc(this, std::move(func)));
        }
    }

    downloadAdapters_.clear();
    this->downloads.clear();
    if (!downloadsObj.is_none()) {
        for (auto item : downloadsObj.cast<py::dict>()) {
            ModObj modObj = modObjFromPy(item.first);
            auto &inner = this->downloads[modObj];

            for (auto regItem : py::reinterpret_borrow<py::object>(item.second).cast<py::dict>()) {
                auto adapter = std::make_unique<PyIniParseDownloadData>(py::reinterpret_borrow<py::object>(regItem.second));
                inner[py::reinterpret_borrow<py::object>(regItem.first)] = adapter.get();
                downloadAdapters_.push_back(std::move(adapter));
            }
        }
    }
}


void PyGIMIParser::editCommands() {
    if (commandEditsObj.is_none()) {
        return;
    }

    py::object modType = ctxImpl.modType();
    std::string modTypeName = ctxImpl.modTypeName();

    // The exact shape of the pure-Python original: a one-element list holding a fresh
    // IniGraphGroup that *aliases* the same commandGraphs dict, edited in place, then read back
    // out. Routed through Python attribute lookup rather than the core's own editFromIni so that a
    // GraphGroupEdit holding pure-Python sub-edits gets the real IniFile/ModType objects.
    py::list graphGroups;
    graphGroups.append(py::cast(std::make_unique<PyIniGraphGroup>(ctxImpl.commandGraphs().cast<py::dict>())));

    py::object result = commandEditsObj.attr("editFromIni")(graphGroups, ctxImpl.ini, modType,
                                                             py::arg("modName") = modTypeName);

    if (result.is_none() || py::len(result) == 0) {
        return;
    }

    ctxImpl.setCommandGraphs(result.cast<py::sequence>()[0].attr("graphs"));
}


std::vector<PyGIMIParser::Core::GraphGroup> PyGIMIParser::parse() {
    refresh();
    return Core::parse();
}


std::vector<PyGIMIParser::Core::GraphGroup> PyGIMIParser::collectParseResult() const {
    // See this method's own declaration for why -- parseToPy() is what actually collects here.
    return {};
}


py::object PyGIMIParser::parseToPy() {
    parse();
    return collectToPy();
}


py::object PyGIMIParser::collectToPy() const {
    // A fresh dict rather than commandGraphs() itself: the returned group is the caller's to edit,
    // and adding the download graphs to it must not also add them to the parser's own
    // commandGraphs. The *graphs* inside are still the live objects.
    py::dict graphs;
    for (auto item : ctxImpl.commandGraphs().cast<py::dict>()) {
        graphs[item.first] = item.second;
    }

    for (const auto &modObjEntry : downloadResourceGraphs()) {
        auto objDownloads = downloads.find(modObjEntry.first);
        if (objDownloads == downloads.end()) {
            continue;
        }

        for (const auto &regEntry : modObjEntry.second) {
            auto foundDownload = objDownloads->second.find(regEntry.first);
            if (foundDownload == objDownloads->second.end() || foundDownload->second == nullptr) {
                continue;
            }

            // Keyed by the download's own name, not the register's -- see
            // AGRemapCore::GIMIParser::collectParseResult's own comment.
            py::tuple modObj = py::make_tuple(py::str(AGRC::IniGraphModObjKeywords::Download),
                                               py::str(foundDownload->second->name()));
            if (graphs.contains(modObj)) {
                continue;
            }

            py::object graph = ctxImpl.groups.graphToPy(regEntry.second);
            if (graph.is_none()) {
                continue;
            }

            graphs[modObj] = std::move(graph);
        }
    }

    py::list result;
    result.append(py::cast(std::make_unique<PyIniGraphGroup>(std::move(graphs))));
    return result;
}


void PyGIMIParser::clear() {
    Core::clear();
    tempKwargs.clear();
}


void initCppGIMIParser(pybind11::module_ &m) {
    auto cls = py::class_<PyGIMIParser, PyBaseIniParser>(m, "GIMIParser", R"doc(
This class inherits from :class:`BaseIniParser`

Parses a .ini file used by a ``GIMI``-style importer

Parameters
----------
iniFile: :class:`IniFile`
    The .ini file to parse

modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
    The mod objects to parse :raw-html:`<br />` :raw-html:`<br />`

    Each tuple contains:

    #. The name of the component
    #. The name of the object within the component

    .. tip::
        You can also interpret mod objects as the suffix part ending of some ``TextureOverride``
        `section`_ :raw-html:`<br />` :raw-html:`<br />`

        eg.

        ``[TextureOverrideHuTaoBody]`` --> ``("", "Body")``
        ``[TextureOverrideYelanBangB]`` --> ``("Bang", "B")``
        ``[TextureOverrideTexture16]`` --> ``("", "Texture16")``

    .. note::
        Iteration order matters -- it decides the order the command graphs are built in -- so an
        ``OrderedSet`` (or a plain ``list``) is preferred over a bare ``set`` here

    **Default**: ``None``

objTargetFuncs: Optional[List[Callable[[:class:`GIMIParser`, :class:`str`, :class:`IfTemplate`, :class:`bool`, Optional[:class:`IfContentPart`], Optional[:class:`IfContentPartColouring`]], List[Tuple[:class:`str`, :class:`str`]]]]]
    A list of custom functions to define how to retrieve the root `sections`_ of the mod objects
    :raw-html:`<br />` :raw-html:`<br />`

    Each function takes in:

    #. This parser
    #. The name of the `section`_ to parse
    #. The content of the `section`_ to parse
    #. Whether to only return 1 result
    #. The :class:`IfContentPart` that is being parsed. Only available if :attr:`trackKeys` is ``True``
    #. The `KVPs`_ to track. Only available if :attr:`trackKeys` is ``True``

    and returns the corresponding mod objects the `section`_ belongs to, or ``None`` if it belongs
    to none. :raw-html:`<br />` :raw-html:`<br />`

    If this argument is ``None``, will use :meth:`classifyByTextureOverrideName` (or a default
    :class:`GIMISectionClassifier`, when the .ini file was classified and :attr:`trackKeys` is on)
    :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

downloads: Optional[Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]]
    The files to download if the mod is missing some required files :raw-html:`<br />` :raw-html:`<br />`

    * The outer keys are tuples that contain the name of the component and the mod object
    * The inner keys are the names of the registers

    .. note::
        The :attr:`DownloadData.name` for each :class:`DownloadData` should be unique

    **Default**: ``None``

commandEdits: Optional[:class:`GraphGroupEdit`]
    Any further edits to the parsed caller/callee graphs for ``TextureOverride`` related command
    `sections`_ :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

makeGlobalGraph: :class:`bool`
    Whether to make the graph for the entire .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

disjointModObjs: :class:`bool`
    Whether the sets of `sections`_ for each mod object should be disjoint or not :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

trackKeys: :class:`bool`
    Whether to track the `KVPs`_ in the .ini file :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

keysToTrack: Optional[Set[:class:`str`]]
    Specific keys to track in the .ini file. If ``None``, keeps track of every key encountered
    :raw-html:`<br />` :raw-html:`<br />`

    .. note::
        Only takes effect if 'trackKeys' and 'makeGlobalGraph' are both ``True``

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object iniFile, py::object modObjs, py::object objTargetFuncs, py::object downloads,
                         py::object commandEdits, bool makeGlobalGraph, bool disjointModObjs, bool trackKeys,
                         py::object keysToTrack) {
        return std::make_unique<PyGIMIParser>(std::move(iniFile), std::move(modObjs), std::move(objTargetFuncs),
                                                std::move(downloads), std::move(commandEdits), makeGlobalGraph,
                                                disjointModObjs, trackKeys, std::move(keysToTrack));
    }), py::arg("iniFile"), py::arg("modObjs") = py::none(), py::arg("objTargetFuncs") = py::none(),
        py::arg("downloads") = py::none(), py::arg("commandEdits") = py::none(), py::arg("makeGlobalGraph") = true,
        py::arg("disjointModObjs") = true, py::arg("trackKeys") = true, py::arg("keysToTrack") = py::none());

    bindBaseIniParserCommonMethods<PyGIMIParser>(cls, R"doc(
Parses the .ini file

Returns
-------
List[:class:`IniGraphGroup`]
    A one-element list holding every graph this parse produced :raw-html:`<br />` :raw-html:`<br />`

    The group's ``graphs`` dict holds, in this order:

    #. every graph in :attr:`commandGraphs`, under its own ``(component, mod object)`` key
    #. every graph in :attr:`downloadResourceGraphs`, under
       ``("download", <the download's name>)``. A download whose name is already in the group is
       skipped, so one resource shared by several registers appears once

    :raw-html:`<br />`

    .. note::
        The graphs are the parser's own live objects, not copies, and the ``dict`` is a fresh one
        -- so adding to the returned group does not touch :attr:`commandGraphs`, but editing a
        graph in it does edit the parser's. Call :meth:`IniSectionGraph.deepcopy` on whichever
        graphs you need to keep independent, exactly as :class:`GIMIFixer` does
    )doc");

    cls.def_property("modObjs", [](PyGIMIParser &self) {
            return self.modObjsObj;
        }, [](PyGIMIParser &self, py::object newModObjs) {
            self.modObjsObj = newModObjs.is_none() ? py::object(py::set()) : std::move(newModObjs);
            self.refresh();
        }, py::doc(R"doc(Set[Tuple[:class:`str`, :class:`str`]]: The different mod objects to parse)doc"))

       .def_property_readonly("components", [](PyGIMIParser &self) {
            self.refresh();
            py::set result;
            for (const std::string &component : self.components()) {
                result.add(py::str(component));
            }
            return result;
        }, py::doc(R"doc(Set[:class:`str`]: The different components to parse)doc"))

       .def_readwrite("objTargetFuncs", &PyGIMIParser::objTargetFuncsObj,
            py::doc(R"doc(List[Callable]: The custom functions defining how to retrieve the root `sections`_ of the mod objects)doc"))

       .def_readwrite("downloads", &PyGIMIParser::downloadsObj,
            py::doc(R"doc(Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`DownloadData`]]: The files to download if the mod is missing some required files)doc"))

       .def_readwrite("commandEdits", &PyGIMIParser::commandEditsObj,
            py::doc(R"doc(Optional[:class:`GraphGroupEdit`]: Any further edits to the parsed caller/callee graphs for ``TextureOverride`` related command `sections`_)doc"))

       .def_readwrite("tempKwargs", &PyGIMIParser::tempKwargs,
            py::doc(R"doc(Dict[:class:`str`, Any]: Temporary user-defined keyword variables for the user to use. Only cleared by :meth:`clear`)doc"))

       .def_readwrite("makeGlobalGraph", &PyGIMIParser::makeGlobalGraph,
            py::doc(R"doc(:class:`bool`: Whether to make the graph for the entire .ini file)doc"))

       .def_readwrite("disjointModObjs", &PyGIMIParser::disjointModObjs,
            py::doc(R"doc(:class:`bool`: Whether the sets of `sections`_ for each mod object should be disjoint or not)doc"))

       .def_readwrite("trackKeys", &PyGIMIParser::trackKeys,
            py::doc(R"doc(:class:`bool`: Whether to track the `KVPs`_ in the .ini file)doc"))

       .def_property("keysToTrack", [](PyGIMIParser &self) {
            return self.keysToTrackObj;
        }, [](PyGIMIParser &self, py::object newKeys) {
            self.keysToTrackObj = std::move(newKeys);
            self.refresh();
        }, py::doc(R"doc(Optional[Set[:class:`str`]]: Specific keys to track in the .ini file)doc"))

       .def_property("commandGraphs", [](PyGIMIParser &self) {
            return self.ctxImpl.commandGraphs();
        }, [](PyGIMIParser &self, py::object graphs) {
            self.ctxImpl.setCommandGraphs(std::move(graphs));
        }, py::doc(R"doc(
Dict[Tuple[:class:`str`, :class:`str`], :class:`IniSectionGraph`]: The caller/callee graphs for
``TextureOverride`` related command `sections`_ :raw-html:`<br />` :raw-html:`<br />`

.. note::
    This is the *same* dict object every time -- it is group ``0`` of the parser's own
    ``List[IniGraphGroup]``, which is what makes :meth:`editCommands`' in-place editing work
        )doc"))

       .def_property_readonly("downloadResourceGraphs", [](PyGIMIParser &self) {
            py::dict result;
            for (const auto &modObjEntry : self.downloadResourceGraphs()) {
                py::dict inner;
                for (const auto &regEntry : modObjEntry.second) {
                    inner[regEntry.first] = self.ctxImpl.groups.graphToPy(regEntry.second);
                }
                result[modObjToPy(modObjEntry.first)] = inner;
            }
            return result;
        }, py::doc(R"doc(
Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, :class:`IniSectionGraph`]]: The
caller/callee graphs for `sections`_ related to download resources :raw-html:`<br />` :raw-html:`<br />`

.. note::
    A fresh dict is built on each access (the graphs inside are the same objects every time)
        )doc"))

       .def_property_readonly("globalGraph", [](PyGIMIParser &self) {
            return self.ctxImpl.groups.graphToPy(self.globalGraph());
        }, py::doc(R"doc(Optional[:class:`IniSectionGraph`]: The graph for the entire .ini file)doc"))

       .def_property_readonly("_sectionTargets", [](PyGIMIParser &self) {
            py::dict result;
            for (const auto &entry : self.sectionTargets()) {
                py::list names;
                for (const std::string &name : entry.second) {
                    names.append(py::str(name));
                }
                result[modObjToPy(entry.first)] = names;
            }
            return result;
        }, py::doc(R"doc(
Dict[Tuple[:class:`str`, :class:`str`], List[:class:`str`]]: The names of the `sections`_ used as
the "entry point" to a particular group of `sections`_ in the ``TextureOverride`` `section`_
caller/callee `graph`_

.. warning::
    These `sections`_ are not necessarily the roots of the graph (they may instead be a child to
    some other `section`_)
        )doc"))

       .def("removeAddedIfTemplates", [](PyGIMIParser &self) {
            self.removeAddedIfTemplates();
        }, py::doc(R"doc(
Removes the newly added :class:`IfTemplate`\s generated by this parser or its associated
:class:`BaseIniFixer`\s from :attr:`IniFile.sectionIfTemplates`
        )doc"))

       .def("buildGlobalGraph", [](PyGIMIParser &self) {
            self.refresh();
            return self.ctxImpl.groups.graphToPy(self.buildGlobalGraph());
        }, py::doc(R"doc(
Builds the graph for the entire .ini file

Returns
-------
:class:`IniSectionGraph`
    The built graph
        )doc"))

       .def("collectParseResult", [](PyGIMIParser &self) {
            self.refresh();
            return self.collectToPy();
        }, py::doc(R"doc(
Collects whatever the *last* :meth:`parse` produced into the same ``[IniGraphGroup]`` that method
returns, without parsing again

.. note::
    :class:`GIMIFixer` needs exactly this: by the time a fixer runs, :meth:`IniFile.parse` has
    already driven the parser, so re-parsing would synthesize every download resource a second time

Returns
-------
List[:class:`IniGraphGroup`]
    A one-element list -- see :meth:`parse` for the group's exact shape
        )doc"))

       .def("parseCommands", [](PyGIMIParser &self) {
            self.refresh();
            self.parseCommands();
        }, py::doc(R"doc(Parses particular command `sections`_ within the mod, specified from :attr:`modObjs`)doc"))

       .def("getDownloads", [](PyGIMIParser &self) {
            self.refresh();

            py::dict result;
            for (const auto &modObjEntry : self.getDownloads()) {
                py::dict inner;

                for (const auto &regEntry : modObjEntry.second) {
                    py::set targets;

                    for (auto *section : regEntry.second.sections) {
                        targets.add(py::cast(section, py::return_value_policy::reference));
                    }
                    for (auto *part : regEntry.second.parts) {
                        targets.add(py::cast(part, py::return_value_policy::reference));
                    }

                    inner[regEntry.first] = targets;
                }

                result[modObjToPy(modObjEntry.first)] = inner;
            }

            return result;
        }, py::doc(R"doc(
Retrieves the particular parts of `sections`_ that require a file download

Returns
-------
Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, Union[Set[:class:`IfContentPart`], Set[:class:`IfTemplate`]]]]
    The parts or `sections`_ needing each register's download
        )doc"))

       .def("addDownloads", [](PyGIMIParser &self, const py::dict &partsNeedDownload) {
            self.refresh();

            PyGIMIParser::Core::DownloadNeeds needs;
            for (auto modObjItem : partsNeedDownload) {
                ModObj modObj = modObjFromPy(modObjItem.first);

                for (auto regItem : py::reinterpret_borrow<py::object>(modObjItem.second).cast<py::dict>()) {
                    PyGIMIParser::Core::DownloadTargets targets;

                    for (auto target : py::reinterpret_borrow<py::object>(regItem.second)) {
                        py::object targetObj = py::reinterpret_borrow<py::object>(target);

                        if (py::isinstance<PyIfTemplate>(targetObj)) {
                            targets.sections.insert(targetObj.cast<PyIfTemplate*>());
                            targets.refToSection = true;
                        } else {
                            targets.parts.insert(targetObj.cast<PyIfContentPart*>());
                        }
                    }

                    needs[modObj][py::reinterpret_borrow<py::object>(regItem.first)] = std::move(targets);
                }
            }

            self.addDownloads(needs);
        }, py::arg("partsNeedDownload"), py::doc(R"doc(
Adds the required download resources to the corresponding `sections`_ and their parts

Parameters
----------
partsNeedDownload: Dict[Tuple[:class:`str`, :class:`str`], Dict[:class:`str`, Union[Set[:class:`IfContentPart`], Set[:class:`IfTemplate`]]]]
    What :meth:`getDownloads` returned
        )doc"))

       .def("setupDownloads", [](PyGIMIParser &self) {
            self.refresh();
            self.setupDownloads();
        }, py::doc(R"doc(Setup the required downloads resources, if not already setup)doc"))

       .def("editCommands", [](PyGIMIParser &self) {
            self.refresh();
            self.editCommands();
        }, py::doc(R"doc(Edits the caller/callee graphs for ``TextureOverride`` related command `sections`_)doc"))

       .def_static("classifyByTextureOverrideName", [](PyGIMIParser &parser, const std::string &sectionName, bool disjoint,
                                                        const py::object &modObjs, bool fromRoots) {
            parser.refresh();

            std::vector<ModObj> parsedModObjs;
            const std::vector<ModObj> *modObjsArg = nullptr;

            if (!modObjs.is_none()) {
                for (auto item : modObjs) {
                    parsedModObjs.push_back(modObjFromPy(item));
                }
                modObjsArg = &parsedModObjs;
            }

            py::list result;
            for (const ModObj &modObj : PyGIMIParser::Core::classifyByTextureOverrideName(parser, sectionName, disjoint, modObjsArg, fromRoots)) {
                result.append(modObjToPy(modObj));
            }

            return result;
        }, py::arg("parser"), py::arg("sectionName"), py::arg("disjoint") = true, py::arg("modObjs") = py::none(),
            py::arg("fromRoots") = true, py::doc(R"doc(
Classify the ``TextureOverride`` `sections`_ to the specified mod objects

Parameters
----------
parser: :class:`GIMIParser`
    The parser used for the classification

sectionName: :class:`str`
    The name of the `section`_ to classify

disjoint: :class:`bool`
    Whether to classify the `section`_ to only 1 mod object or allow classification to multiple mod
    objects :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

modObjs: Optional[Set[Tuple[:class:`str`, :class:`str`]]]
    The mod objects for classification. If ``None``, uses the mod objects at :attr:`modObjs` for
    the argument, 'parser' :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

fromRoots: :class:`bool`
    Whether to make sure 'parser''s :attr:`globalGraph` has been built first :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    The mod objects the `section`_ has been classified to
        )doc"))

       .def("_getSectionTargets", [](PyGIMIParser &self) {
            self.refresh();
            self.getSectionTargets();
        }, py::doc(R"doc(
Retrieves the "entry points" names of the ``TextureOverride`` `sections`_ for each mod object
specified at :attr:`modObjs`
        )doc"));
}
