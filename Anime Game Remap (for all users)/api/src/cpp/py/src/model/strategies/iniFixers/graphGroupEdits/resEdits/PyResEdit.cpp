#include "PyResEdit.h"

#include <utility>

#include "../../../../iftemplate/PyIfTemplate.h"
#include "AGRemapCore/model/iniresources/IniResource.h"


namespace {

// The Python 'collections.deque' type, imported lazily and never released -- a plain
// 'static py::object' would be decref'd during C++ static destruction, ie. after Py_Finalize (see
// PyGraphGroupEdit.cpp's own note on the access violation that causes).
py::object dequeType() {
    static py::object *type = nullptr;
    if (type == nullptr) {
        type = new py::object(py::module_::import("collections").attr("deque"));
    }

    return *type;
}


// The 'core' module, captured at init time. Never released, for the same interpreter-shutdown
// reason as dequeType above.
py::module_ *&coreModuleSlot() {
    static py::module_ *slot = nullptr;
    return slot;
}

}


py::module_ pyCoreModule() {
    return *coreModuleSlot();
}


PyBaseResEditCore::ResEditConfig makeResEditConfig() {
    // "filename" is the .ini register naming a resource's file (IniKeywords.Filename). The
    // conversions are the same shape IfTemplateRunConfig uses for its own 'run =' values.
    return PyBaseResEditCore::ResEditConfig{
        py::cast(std::string("filename")),
        [](const py::object &value) { return py::str(value).cast<std::string>(); },
        [](const std::string &file) { return py::cast(file); }
    };
}


AGRC::IniGraphReplaceMode parseGraphReplaceMode(const py::object &mode) {
    if (mode.is_none()) {
        return AGRC::IniGraphReplaceMode::Ignore;
    }

    // Read through '.value' rather than by identity: IniGraphReplaceMode is still a pure-Python
    // Enum, so there is no C++ member to compare against -- only the string each member carries.
    py::object value = py::hasattr(mode, "value") ? mode.attr("value") : mode;
    std::string parsed = py::str(value).cast<std::string>();

    if (parsed == "replace") {
        return AGRC::IniGraphReplaceMode::Replace;
    }

    if (parsed == "combine") {
        return AGRC::IniGraphReplaceMode::Combine;
    }

    return AGRC::IniGraphReplaceMode::Ignore;
}


PyBaseResEditCore::CollectedSections parseCollectedSections(const py::object &collectedSections) {
    PyBaseResEditCore::CollectedSections result;
    if (collectedSections.is_none()) {
        return result;
    }

    for (auto item : collectedSections.cast<py::dict>()) {
        result.insert_or_assign(py::str(item.first).cast<std::string>(), py::str(item.second).cast<std::string>());
    }

    return result;
}


PyBaseResEditCore::ResourceFilter parseResourceFilter(const py::object &resourceFilter) {
    if (resourceFilter.is_none()) {
        return {};
    }

    py::object held = resourceFilter;
    return [held](const std::string &file, const std::string &fileKey) {
        return held(py::str(file), py::str(fileKey)).cast<bool>();
    };
}


// ---------------------------------------------------------------------------------------
// PyIniResEditContext
// ---------------------------------------------------------------------------------------

PyIniResEditContext::PyIniResEditContext(py::object ini, py::object modType, py::object resources):
    ini(std::move(ini)), modType(std::move(modType)), resources(std::move(resources)) {}


bool PyIniResEditContext::hasIni() const {
    return !ini.is_none();
}


std::string PyIniResEditContext::iniFolder() const {
    if (!hasIni()) {
        return "";
    }

    return py::str(ini.attr("folder")).cast<std::string>();
}


std::unordered_map<std::string, PyIniResEditContext::Section*> PyIniResEditContext::sectionIfTemplates() const {
    std::unordered_map<std::string, Section*> result;
    if (!hasIni()) {
        return result;
    }

    for (auto item : ini.attr("sectionIfTemplates").cast<py::dict>()) {
        result[py::str(item.first).cast<std::string>()] = py::reinterpret_borrow<py::object>(item.second).cast<PyIfTemplate*>();
    }

    return result;
}


AGRC::Z3Context* PyIniResEditContext::z3Ctx() const {
    if (!hasIni()) {
        return nullptr;
    }

    py::object ctx = ini.attr("_z3Ctx");
    if (ctx.is_none()) {
        return nullptr;
    }

    return ctx.cast<AGRC::Z3Context*>();
}


void PyIniResEditContext::storeResource(const std::string &fileKey, std::unique_ptr<AGRC::IniResource> resource) {
    // Only reached if a core-side implementation of buildResModel ever runs against this context --
    // every Python-facing edit overrides buildResModel to build a real Python model instead. Kept
    // working rather than left unimplemented, so a mixed C++/Python edit list stays coherent.
    storeResourceObj(fileKey, py::cast(std::move(resource)));
}


void PyIniResEditContext::beginCollectingResources() {
    collecting_ = true;
    collected_.clear();
}


std::vector<std::pair<std::string, AGRC::IniResource*>> PyIniResEditContext::takeCollectedResources() {
    std::vector<std::pair<std::string, AGRC::IniResource*>> result;
    result.swap(collected_);
    return result;
}


void PyIniResEditContext::endCollectingResources() {
    collecting_ = false;
    collected_.clear();
}


py::object PyIniResEditContext::resourceToPy(const AGRC::IniResource *resource) const {
    auto it = resourceHandles_.find(resource);
    if (it == resourceHandles_.end()) {
        return py::none();
    }

    return it->second;
}


void PyIniResEditContext::storeResourceObj(const std::string &fileKey, py::object resource) {
    // Registered unconditionally, whichever way it is stored -- the core identifies models by raw
    // pointer from here on, and only this map can turn one back into its Python object.
    AGRC::IniResource *raw = resource.cast<AGRC::IniResource*>();
    resourcesKeepAlive_.append(resource);
    resourceHandles_[raw] = resource;

    if (collecting_) {
        collected_.emplace_back(fileKey, raw);
        return;
    }

    if (resources.is_none()) {
        if (hasIni()) {
            ini.attr("resources").attr("append")(std::move(resource));
        }

        return;
    }

    py::dict resourcesDict = resources.cast<py::dict>();
    py::str key(fileKey);

    if (!resourcesDict.contains(key)) {
        resourcesDict[key] = dequeType()(py::make_tuple(std::move(resource)));
        return;
    }

    resourcesDict[key].attr("append")(std::move(resource));
}


// ---------------------------------------------------------------------------------------
// PyBaseResEdit
// ---------------------------------------------------------------------------------------

PyBaseResEdit::PyBaseResEdit(std::string resType, py::object resModObj, py::object graphReplaceMode):
    PyResEditMixin<PyBaseResEditCore>(std::move(resType), PyBaseResEditCore::GraphId(), makeResEditConfig(),
                                       parseGraphReplaceMode(graphReplaceMode)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = std::move(graphReplaceMode);
    refresh();
}


py::object PyBaseResEdit::pySelf() const {
    // Cast through the *registered* type: 'BaseResEdit' is registered as PyBaseResEditCore, so
    // that is the pointer pybind11's instance registry knows this object by.
    return py::cast(static_cast<const PyBaseResEditCore*>(this), py::return_value_policy::reference);
}


void PyBaseResEdit::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                   const std::string &modName, const std::string &fileKey, Context &ctx) {
    // A plain IniResource carries only the source path -- 'fixedPath' is what ResReplace uses.
    (void)fixedPath;
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath)), modName, fileKey, ctx);
}


// ---------------------------------------------------------------------------------------
// PyResIdentity
// ---------------------------------------------------------------------------------------

PyResIdentity::PyResIdentity(py::object resModObj, bool createResModel):
    PyResEditMixin<AGRC::ResIdentity<py::object, py::object, PyObjectHash, PyObjectEqual>>(
        PyBaseResEditCore::GraphId(), makeResEditConfig(), createResModel) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = py::none();
    refresh();
}


py::object PyResIdentity::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyResIdentity::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                   const std::string &modName, const std::string &fileKey, Context &ctx) {
    // The pure-Python original never overrode buildResModel here -- an identity edit builds the
    // same plain IniResource the base does (when it builds anything at all; see createResModel).
    (void)fixedPath;
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath)), modName, fileKey, ctx);
}


// ---------------------------------------------------------------------------------------
// PyResReplace
// ---------------------------------------------------------------------------------------

PyResReplace::PyResReplace(std::string resType, py::object resModObj, py::object graphReplaceMode):
    PyResEditMixin<AGRC::ResReplace<py::object, py::object, PyObjectHash, PyObjectEqual>>(
        std::move(resType), PyBaseResEditCore::GraphId(), makeResEditConfig(), parseGraphReplaceMode(graphReplaceMode)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = std::move(graphReplaceMode);
    refresh();
}


py::object PyResReplace::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyResReplace::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                  const std::string &modName, const std::string &fileKey, Context &ctx) {
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), py::str(fixedPath), modTypeObj),
                           modName, fileKey, ctx);
}


// ---------------------------------------------------------------------------------------
// PyResCreate
// ---------------------------------------------------------------------------------------

PyResCreate::PyResCreate(std::string resType, py::object resModObj, py::object graphReplaceMode):
    PyResCreateMixin<AGRC::ResCreate<py::object, py::object, PyObjectHash, PyObjectEqual>>(
        std::move(resType), PyBaseResEditCore::GraphId(), makeResEditConfig(), parseGraphReplaceMode(graphReplaceMode)) {
    resModObjObj = std::move(resModObj);
    graphReplaceModeObj = std::move(graphReplaceMode);
    refresh();
}


py::object PyResCreate::pySelf() const {
    return py::cast(this, py::return_value_policy::reference);
}


void PyResCreate::buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                                 const std::string &modName, const std::string &fileKey, Context &ctx) {
    (void)fixedPath;
    auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
    dispatchBuildResModel(py::make_tuple(py::str(resType), pyCtx.ini, py::str(srcPath), modTypeObj), modName, fileKey, ctx);
}


// ---------------------------------------------------------------------------------------
// bindings
// ---------------------------------------------------------------------------------------

void initCppResEdit(pybind11::module_ &m) {
    coreModuleSlot() = new py::module_(m);

    auto baseCls = py::class_<PyBaseResEditCore, py::smart_holder>(m, "BaseResEdit", R"doc(
Base class to construct the necessary parts for a particular resource in a .ini file

Parameters
----------
resType: :class:`str`
    The name of the type of resource

resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource :raw-html:`<br />` :raw-html:`<br />`

    The tuple contains:

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

graphReplaceMode: :class:`IniGraphReplaceMode`
    What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``IniGraphReplaceMode.Ignore``
    )doc");

    baseCls.def(py::init([](std::string resType, py::object resModObj, py::object graphReplaceMode) -> std::unique_ptr<PyBaseResEditCore> {
        return std::make_unique<PyBaseResEdit>(std::move(resType), std::move(resModObj), std::move(graphReplaceMode));
    }), py::arg("resType"), py::arg("resModObj"), py::arg("graphReplaceMode") = py::none());

    baseCls.def("buildResModel", [](PyBaseResEditCore &self, const std::string &resType, const py::object &ini,
                                    const std::string &srcPath, const py::args &, const py::kwargs &) -> py::object {
        (void)self;
        if (ini.is_none()) {
            return py::none();
        }

        std::string folder = py::str(ini.attr("folder")).cast<std::string>();
        return py::cast(std::make_unique<AGRC::IniResource>(resType, folder, srcPath));
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::doc(R"doc(
Builds the model for the resource

Parameters
----------
resType: :class:`str`
    The name for the type of resource

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

Returns
-------
:class:`IniResource`
    The built resource
    )doc"));

    bindResEditCommonMethods(baseCls);


    auto identityCls = py::class_<PyResIdentity, PyBaseResEditCore, py::smart_holder>(m, "ResIdentity", R"doc(
This class inherits from :class:`BaseResEdit`

Class to only build the :class:`IniSectionGraph` for the original collected resource

Parameters
----------
resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource

createResModel: :class:`bool`
    Whether to build the models for the resources :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc");

    identityCls.def(py::init([](py::object resModObj, bool createResModel) {
        return std::make_unique<PyResIdentity>(std::move(resModObj), createResModel);
    }), py::arg("resModObj"), py::arg("createResModel") = true);

    identityCls.def_readwrite("createResModel", &PyResIdentity::createResModel, py::doc(R"doc(
:class:`bool`: Whether to build the models for the resources
    )doc"));

    bindResEditCommonMethods(identityCls);


    auto replaceCls = py::class_<PyResReplace, PyBaseResEditCore, py::smart_holder>(m, "ResReplace", R"doc(
This class inherits from :class:`BaseResEdit`

Class that creates the necessary parts for a fixed resource by building upon the existing
:class:`IniSectionGraph` of the original resource

Parameters
----------
resType: :class:`str`
    The name of the type of resource

resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource

graphReplaceMode: :class:`IniGraphReplaceMode`
    What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``IniGraphReplaceMode.Ignore``
    )doc");

    replaceCls.def(py::init([](std::string resType, py::object resModObj, py::object graphReplaceMode) {
        return std::make_unique<PyResReplace>(std::move(resType), std::move(resModObj), std::move(graphReplaceMode));
    }), py::arg("resType"), py::arg("resModObj"), py::arg("graphReplaceMode") = py::none());

    replaceCls.def("buildResModel", [](PyResReplace &self, const std::string &resType, const py::object &ini,
                                       const std::string &srcPath, const std::string &fixedPath, const py::object &modType,
                                       const py::args &, const std::string &modName, const py::kwargs &) -> py::object {
        (void)self;
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        std::string folder = py::str(ini.attr("folder")).cast<std::string>();
        return py::cast(std::make_unique<AGRC::IniFixResource>(resType, folder, srcPath, fixedPath));
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("fixedPath"), py::arg("modType") = py::none(),
       py::kw_only(), py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource

Parameters
----------
resType: :class:`str`
    The name for the type of resource

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

fixedPath: :class:`str`
    The file path to the fixed resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`IniFixResource`
    The built resource
    )doc"));

    bindResEditCommonMethods(replaceCls);


    auto createCls = py::class_<PyResCreate, PyBaseResEditCore, py::smart_holder>(m, "ResCreate", R"doc(
This class inherits from :class:`BaseResEdit`

Class that creates the necessary parts for a brand-new fixed resource, building its `sections`_ from
scratch rather than from the .ini file's existing ones

Parameters
----------
resType: :class:`str`
    The name of the type of resource

resModObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object to hold the newly created :class:`IniSectionGraph` for the resource

graphReplaceMode: :class:`IniGraphReplaceMode`
    What to do when the corresponding :class:`IniSectionGraph` to construct already exists :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``IniGraphReplaceMode.Ignore``
    )doc");

    createCls.def(py::init([](std::string resType, py::object resModObj, py::object graphReplaceMode) {
        return std::make_unique<PyResCreate>(std::move(resType), std::move(resModObj), std::move(graphReplaceMode));
    }), py::arg("resType"), py::arg("resModObj"), py::arg("graphReplaceMode") = py::none());

    createCls.def("buildResModel", [](PyResCreate &self, const std::string &resType, const py::object &ini,
                                      const std::string &srcPath, const py::object &modType, const py::args &,
                                      const std::string &modName, const py::kwargs &) -> py::object {
        (void)self;
        (void)modType;
        (void)modName;
        if (ini.is_none()) {
            return py::none();
        }

        std::string folder = py::str(ini.attr("folder")).cast<std::string>();
        return py::cast(std::make_unique<AGRC::IniResource>(resType, folder, srcPath));
    }, py::arg("resType"), py::arg("ini"), py::arg("srcPath"), py::arg("modType") = py::none(), py::kw_only(),
       py::arg("modName") = "", py::doc(R"doc(
Builds the model for the resource

Parameters
----------
resType: :class:`str`
    The name for the type of resource

ini: :class:`IniFile`
    The .ini file to build the resource for

srcPath: :class:`str`
    The file path to the original resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to. Unused :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`IniResource`
    The built resource
    )doc"));

    createCls.def("buildSection", [](PyResCreate &self, const std::string &sectionName, const py::object &modType,
                                     const std::string &modName) -> py::object {
        (void)self;
        (void)sectionName;
        (void)modType;
        (void)modName;
        // No-op by default, matching the pure-Python original's own 'pass' -- a subclass
        // (eg. TexCreate) is what actually builds something here.
        return py::none();
    }, py::arg("sectionName"), py::arg("modType") = py::none(), py::arg("modName") = "", py::doc(R"doc(
Builds a `section`_ for the resource

Parameters
----------
sectionName: :class:`str`
    The name for the `section`_

modType: Optional[:class:`ModType`]
    The type of mod to fix from

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
Optional[:class:`IfTemplate`]
    The generated `section`_
    )doc"));

    bindResEditCommonMethods(createCls);
}
