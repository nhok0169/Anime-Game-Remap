#ifndef AGRemapPyBind_PyResEdit_H
#define AGRemapPyBind_PyResEdit_H

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <pybind11/pybind11.h>

#include "../PyBaseIniGraphGroupEdit.h"
#include "../../../../iftemplate/PyIfTemplate.h"  // reuses PyIfTemplate (what a built `section`_ is)
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/ResEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::BaseResEdit`\\<py::object, py::object\\> -- the type
 registered to `Python`_ as ``BaseResEdit``, and the real C++ base of every concrete resource edit
 below, so the `pybind11`_ inheritance here is genuine rather than merely claimed
 @endrst
 */
using PyBaseResEditCore = AGRC::BaseResEdit<std::string, std::string>;


/**
 * @brief
 @rst
 The :cpp:type:`AGRemapCore::BaseResEdit::ResEditConfig` every `pybind11`_-facing resource edit uses
 -- the ``filename`` key, plus the ``py::object`` <-> file-path conversions
 @endrst
 */
PyBaseResEditCore::ResEditConfig makeResEditConfig();


/**
 * @brief Converts a Python ``IniGraphReplaceMode`` member into the core enum
 *
 * @param mode The Python enum member (read by its ``.value``), or ``None`` for ``Ignore``
 */
AGRC::IniGraphReplaceMode parseGraphReplaceMode(const py::object &mode);


/**
 * @brief
 @rst
 The bound ``core`` module itself, captured at module-init time :raw-html:`<br />` :raw-html:`<br />`

 Resource edits build their models by calling the very classes this module registers
 (``IniResource``, ``RemapBlendResource``, ...) rather than by constructing the C++ types and
 casting: that is literally what the pure-Python originals did, so every default argument and
 conversion those constructors apply is preserved for free
 @endrst
 */
py::module_ pyCoreModule();



/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IniResEditContext` -- the still-pure-Python
 ``IniFile`` a resource edit is building resources for, plus the two pieces of per-call state that
 travel with it :raw-html:`<br />` :raw-html:`<br />`

 ``modType`` and the ``resources`` result collection ride along here rather than through the core
 signatures for the same reason the ``.ini`` file itself does: they are `Python`_ objects with no
 C++ counterpart, and every core method that would otherwise need them already takes this context
 @endrst
 */
class PyIniResEditContext: public AGRC::IniResEditContext<std::string, std::string> {
    public:
        using Base = AGRC::IniResEditContext<std::string, std::string>;
        using Section = Base::Section;

        /**
         * @brief The Python ``IniFile``, or ``None``
         */
        py::object ini;

        /**
         * @brief The Python ``ModType``, or ``None``
         */
        py::object modType;

        /**
         * @brief
         @rst
         Where built resource models are collected -- a ``Dict[str, Deque[IniResource]]``, or
         ``None`` to append to ``ini.resources`` instead
         @endrst
         */
        py::object resources;

        /**
         * @brief Constructs a context
         *
         * @param ini The Python ``IniFile``, or ``None``
         * @param modType The Python ``ModType``, or ``None``
         * @param resources Where to collect built models, or ``None`` for ``ini.resources``
         */
        explicit PyIniResEditContext(py::object ini, py::object modType = py::none(), py::object resources = py::none());

        bool hasIni() const override;
        std::string iniFolder() const override;
        std::shared_ptr<AGRC::BaseLogger> logger() const override;
        std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
        AGRC::Z3Context* z3Ctx() const override;
        void storeResource(const std::string &fileKey, std::unique_ptr<AGRC::IniResource> resource) override;
        void beginCollectingResources() override;
        std::vector<std::pair<std::string, AGRC::IniResource*>> takeCollectedResources() override;
        void endCollectingResources() override;

        /**
         * @brief
         @rst
         The `Python`_ object for a built resource model this context has seen, or ``None``
         :raw-html:`<br />` :raw-html:`<br />`

         The same pointer -> object bookkeeping ``PyIniGraphGroups`` keeps for graphs, and for the
         same reason: the core hands models around as plain :cpp:class:`AGRemapCore::IniResource`
         pointers, and only this side can turn one back into the `Python`_ object it really is
         @endrst
         *
         * @param resource The model to look up
         */
        py::object resourceToPy(const AGRC::IniResource *resource) const;

        /**
         * @brief
         @rst
         Stores one already-built `Python`_ resource model -- appended to ``ini.resources`` when
         #resources is ``None``, and into ``resources[fileKey]`` (a ``deque``, created on demand)
         otherwise. Faithful to the pure-Python original's own three-way branch
         @endrst
         *
         * @param fileKey The assigned id for the source file
         * @param resource The built resource model
         */
        void storeResourceObj(const std::string &fileKey, py::object resource);

    private:
        // While collecting, models are buffered here instead of reaching the .ini file -- see
        // AGRemapCore::IniResEditContext::beginCollectingResources.
        bool collecting_ = false;
        std::vector<std::pair<std::string, AGRC::IniResource*>> collected_;

        // Strong references to every model this context has seen, plus the pointer -> object map
        // resourceToPy answers from. A model that no group ends up claiming is still owned here
        // for the lifetime of the edit, matching the pure-Python original (where the same model
        // stayed alive in its 'resources' deque until the whole call finished).
        py::list resourcesKeepAlive_;
        std::unordered_map<const AGRC::IniResource*, py::object> resourceHandles_;
};


/**
 * @brief
 @rst
 The non-template half of :cpp:class:`PyResEditMixin` :raw-html:`<br />` :raw-html:`<br />`

 It exists so the shared bindings can be written against **one** type. `pybind11`_ registers
 ``BaseResEdit`` as `PyBaseResEditCore`, so that is the only type a bound method may take -- but the
 members those bindings need live on the mixin, which is a different type per concrete edit. Every
 instance reaching a binding really is a mixin, so one ``dynamic_cast`` to this interface recovers
 them without the bindings having to be templated over the concrete class at all
 @endrst
 */
class PyResEditCommon {
    public:
        virtual ~PyResEditCommon() = default;

        /**
         * @brief The exact Python tuple given for ``resModObj``
         */
        py::object resModObjObj;

        /**
         * @brief The exact Python ``IniGraphReplaceMode`` member given for ``graphReplaceMode``
         */
        py::object graphReplaceModeObj;

        /**
         * @brief
         @rst
         The ``modType`` the operation currently running was called with :raw-html:`<br />`
         :raw-html:`<br />`

         The pure-Python originals take ``modType`` on ``getFixResourceName``/``getFixFile``, which
         the core signatures do not (no implementation anywhere reads it). It is kept here so those
         `Python`_-facing signatures stay unchanged for anyone overriding them
         @endrst
         */
        py::object modTypeObj = py::none();

        /**
         * @copydoc PyResEditMixin::pySelf
         */
        virtual py::object pySelf() const = 0;

        /**
         * @copydoc PyResEditMixin::refresh
         *
         * @param modType The ``modType`` the current operation was called with
         */
        virtual void refresh(py::object modType) = 0;

        /**
         * @copydoc PyResEditMixin::coreGetFixResourceName
         *
         * @param resource The name of the original resource `section`_
         * @param modName The name of the mod to fix to
         */
        virtual std::optional<std::string> coreGetFixResourceName(const std::string &resource, const std::string &modName) const = 0;

        /**
         * @copydoc PyResEditMixin::coreCollectResourceName
         *
         * @param oldResourceName The old name of the resource `section`_
         * @param newResourceName The fixed name for the resource `section`_
         */
        virtual std::pair<std::string, std::string> coreCollectResourceName(const std::string &oldResourceName,
                                                                             const std::string &newResourceName) const = 0;

        /**
         * @copydoc PyResEditMixin::coreGetFixFile
         *
         * @param file The file path to the original resource
         * @param modName The name of the mod to fix to
         * @param graphId The unique id for the graph of the resource
         */
        virtual std::string coreGetFixFile(const std::string &file, const std::string &modName,
                                            const std::string &graphId) const = 0;
};


/**
 * @brief
 @rst
 The `Python`_-facing behaviour shared by every resource edit, mixed into whichever core class the
 concrete edit derives from :raw-html:`<br />` :raw-html:`<br />`

 Every method the pure-Python original documented as overridable
 (``getFixResourceName``/``getFixFile``/``collectResourceName``/``buildResModel``/``buildSection``)
 is re-dispatched through genuine `Python`_ attribute lookup (``pySelf().attr("...")``), never
 straight to the C++ base. Without that, a pure-`Python`_ subclass overriding one of them would be
 silently ignored the moment the core called it internally -- the same trap ``PyBaseTexFilter``'s
 ``__call__`` binding documents. The `Python`_-facing bindings themselves must therefore call the
 **core** implementation explicitly (``self.CoreClass::method(...)``), or the two would call each
 other forever
 @endrst
 *
 * @tparam CoreT The core resource-edit class this wraps
 */
template <typename CoreT>
class PyResEditMixin: public CoreT, public PyResEditCommon {
    public:
        using CoreT::CoreT;

        /**
         * @brief The core resource-edit class this wraps -- what a Python-facing binding must call into
         */
        using CoreClass = CoreT;

        /**
         * @copydoc AGRemapCore::BaseResEdit::Context
         */
        using Context = typename CoreT::Context;

        /**
         * @copydoc AGRemapCore::BaseResEdit::GraphId
         */
        using GraphId = typename CoreT::GraphId;

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``resModObj``/``graphReplaceMode`` members from their
         `Python`_ counterparts, and records the ``modType`` for the operation about to run
         @endrst
         *
         * @param modType The ``modType`` the current operation was called with. **Default**: ``None``
         */
        void refresh(py::object modType = py::none()) override {
            if (!resModObjObj.is_none()) {
                this->resModObj = parseGraphId(resModObjObj);
            }

            this->graphReplaceMode = parseGraphReplaceMode(graphReplaceModeObj);
            modTypeObj = std::move(modType);
        }

        /**
         * @brief
         @rst
         This instance as its most-derived `Python`_ object, so ``.attr("...")`` finds a
         pure-`Python`_ subclass's overrides
         @endrst
         */
        virtual py::object pySelf() const = 0;

        /**
         * @brief
         @rst
         The three methods below re-dispatch into `Python`_, so the `Python`_-facing bindings must
         reach the **core** implementation instead -- otherwise the two would call each other
         forever. These are that entry point :raw-html:`<br />` :raw-html:`<br />`

         They exist as named helpers rather than a qualified call written at the binding site
         (``self.T::CoreClass::method(...)``) because MSVC does not parse that form inside a
         function template; the identical qualified call *inside* this class template does
         @endrst
         *
         * @param resource The name of the original resource `section`_
         * @param modName The name of the mod to fix to
         */
        std::optional<std::string> coreGetFixResourceName(const std::string &resource, const std::string &modName) const override {
            return CoreT::getFixResourceName(resource, modName);
        }

        /**
         * @copydoc coreGetFixResourceName
         *
         * @param oldResourceName The old name of the resource `section`_
         * @param newResourceName The fixed name for the resource `section`_
         */
        std::pair<std::string, std::string> coreCollectResourceName(const std::string &oldResourceName,
                                                                     const std::string &newResourceName) const override {
            return CoreT::collectResourceName(oldResourceName, newResourceName);
        }

        /**
         * @copydoc coreGetFixResourceName
         *
         * @param file The file path to the original resource
         * @param modName The name of the mod to fix to
         * @param graphId The unique id for the graph of the resource
         */
        std::string coreGetFixFile(const std::string &file, const std::string &modName, const std::string &graphId) const override {
            return CoreT::getFixFile(file, modName, graphId);
        }

        std::optional<std::string> getFixResourceName(const std::string &resource, const std::string &modName = "") const override {
            py::object result = pySelf().attr("getFixResourceName")(py::str(resource), modTypeObj, py::arg("modName") = modName);
            if (result.is_none()) {
                return std::nullopt;
            }

            return result.cast<std::string>();
        }

        std::pair<std::string, std::string> collectResourceName(const std::string &oldResourceName,
                                                                 const std::string &newResourceName) const override {
            py::object result = pySelf().attr("collectResourceName")(py::str(oldResourceName), py::str(newResourceName));
            py::sequence pair = result.cast<py::sequence>();
            return {py::str(pair[0]).cast<std::string>(), py::str(pair[1]).cast<std::string>()};
        }

        std::string getFixFile(const std::string &file, const std::string &modName = "",
                                const std::string &graphId = "") const override {
            py::object result = pySelf().attr("getFixFile")(py::str(file), modTypeObj, py::arg("modName") = modName,
                                                             py::arg("graphId") = graphId);
            return result.cast<std::string>();
        }

    protected:
        /**
         * @brief
         @rst
         Calls the `Python`_ ``buildResModel`` with 'args' and stores whatever it returns into 'ctx'
         -- the shared half of every concrete edit's own ``buildResModel`` override :raw-html:`<br />`
         :raw-html:`<br />`

         Building and storing are one step in the core (see
         :cpp:func:`AGRemapCore::BaseResEdit::buildResModel`) but two in the pure-Python original,
         whose ``buildResModel`` *returns* the model and leaves storing to its caller. This bridges
         the two, so a `Python`_ override keeps the signature it always had
         @endrst
         *
         * @param args The already-assembled Python argument list for this edit's own signature
         * @param modName The name of the mod to fix to, passed as a keyword argument
         * @param fileKey The assigned id for the source file
         * @param ctx The .ini file the resource is being built for
         */
        void dispatchBuildResModel(const py::tuple &args, const std::string &modName, const std::string &fileKey, Context &ctx) {
            auto &pyCtx = static_cast<PyIniResEditContext&>(ctx);
            py::object resource = pySelf().attr("buildResModel")(*args, py::arg("modName") = modName);
            if (resource.is_none()) {
                return;
            }

            pyCtx.storeResourceObj(fileKey, std::move(resource));
        }
};


/**
 * @brief
 @rst
 The extra half of :cpp:class:`PyResEditMixin` that only the `section`_-building resource edits
 need :raw-html:`<br />` :raw-html:`<br />`

 :cpp:func:`AGRemapCore::ResCreate::buildSection` is abstract in the core, so every
 ``ResCreate``-derived binding has to implement it -- and they all implement it the same way: hand
 off to `Python`_, then keep a strong reference to whatever comes back. The keep-alive is not
 optional: the graph built from these `sections`_ only *borrows* them unless it was asked to copy
 them (see :cpp:class:`IniSectionGraph`'s own note), so nothing else would hold the `Python`_
 wrapper alive
 @endrst
 *
 * @tparam CoreT The core resource-edit class this wraps -- must derive from :cpp:class:`AGRemapCore::ResCreate`
 */
template <typename CoreT>
class PyResCreateMixin: public PyResEditMixin<CoreT> {
    public:
        using PyResEditMixin<CoreT>::PyResEditMixin;
        using Section = typename CoreT::Section;

        Section* buildSection(const std::string &sectionName, const std::string &modName) override {
            py::object result = this->pySelf().attr("buildSection")(py::str(sectionName), this->modTypeObj,
                                                                     py::arg("modName") = modName);
            if (result.is_none()) {
                return nullptr;
            }

            sectionsKeepAlive_.append(result);
            return result.cast<PyIfTemplate*>();
        }

    private:
        py::list sectionsKeepAlive_;
};


/**
 * @brief The `pybind11`_-facing ``BaseResEdit``. Builds a plain :class:`IniResource` per referenced file
 */
class PyBaseResEdit: public PyResEditMixin<PyBaseResEditCore> {
    public:
        /**
         * @brief Constructs a new resource edit
         *
         * @param resType The name of the type of resource
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param graphReplaceMode The Python ``IniGraphReplaceMode`` member
         */
        PyBaseResEdit(std::string resType, py::object resModObj, py::object graphReplaceMode);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


/**
 * @brief The `pybind11`_-facing ``ResIdentity``. Only builds the graph, optionally skipping the models entirely
 */
class PyResIdentity: public PyResEditMixin<AGRC::ResIdentity<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new identity resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param createResModel Whether to build the models for the resources
         */
        PyResIdentity(py::object resModObj, bool createResModel);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


/**
 * @brief The `pybind11`_-facing ``ResReplace``. Builds an :class:`IniFixResource` per referenced file
 */
class PyResReplace: public PyResEditMixin<AGRC::ResReplace<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new replacing resource edit
         *
         * @param resType The name of the type of resource
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param graphReplaceMode The Python ``IniGraphReplaceMode`` member
         */
        PyResReplace(std::string resType, py::object resModObj, py::object graphReplaceMode);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


/**
 * @brief
 @rst
 The `pybind11`_-facing ``ResCreate`` :raw-html:`<br />` :raw-html:`<br />`

 ``buildSection`` is abstract in the core class, so this supplies an implementation that dispatches
 to `Python`_ -- and keeps a strong reference to every `section`_ it builds in #sectionsKeepAlive_,
 since the graph built from them only *borrows* them unless it was asked to copy them (see
 :cpp:class:`IniSectionGraph`'s own note)
 @endrst
 */
class PyResCreate: public PyResCreateMixin<AGRC::ResCreate<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new creating resource edit
         *
         * @param resType The name of the type of resource
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param graphReplaceMode The Python ``IniGraphReplaceMode`` member
         */
        PyResCreate(std::string resType, py::object resModObj, py::object graphReplaceMode);

        py::object pySelf() const override;
        void buildResModel(const std::string &resType, const std::string &srcPath, const std::string &fixedPath,
                            const std::string &modName, const std::string &fileKey, Context &ctx) override;
};


/**
 * @brief
 @rst
 Parses a `Python`_ ``Dict[str, str]`` of collected `sections`_ into the insertion-ordered map every
 resource edit takes
 @endrst
 *
 * @param collectedSections The Python dict to parse
 */
PyBaseResEditCore::CollectedSections parseCollectedSections(const py::object &collectedSections);


/**
 * @brief
 @rst
 Wraps a `Python`_ ``resourceFilter`` callable into the predicate every resource edit takes --
 an empty ``std::function`` for ``None``
 @endrst
 *
 * @param resourceFilter The Python callable, or ``None``
 */
PyBaseResEditCore::ResourceFilter parseResourceFilter(const py::object &resourceFilter);


/**
 * @brief
 @rst
 Chains every shared resource-edit binding onto one concrete resource edit's already-constructed
 ``py::class_`` :raw-html:`<br />` :raw-html:`<br />`

 Real `pybind11`_ inheritance already hands the concrete classes the base's methods, so in practice
 only ``BaseResEdit`` itself calls this -- it stays a helper (docstrings included) so a future
 resource edit that ends up outside this hierarchy can reuse the same surface
 @endrst
 *
 * @tparam PyClass The concrete ``py::class_`` type being extended
 *
 * @param cls The class to chain the bindings onto
 */
template <typename PyClass>
void bindResEditCommonMethods(PyClass &cls) {
    using T = PyBaseResEditCore;

    // Every instance reaching these bindings is really a PyResEditMixin -- see PyResEditCommon.
    auto common = [](T &self) -> PyResEditCommon& {
        return dynamic_cast<PyResEditCommon&>(self);
    };

    cls.def_readwrite("resType", &T::resType, py::doc(R"doc(:class:`str`: The name of the type of resource)doc"));

    cls.def_property("resModObj", [common](const T &self) {
        return common(const_cast<T&>(self)).resModObjObj;
    }, [common](T &self, py::object resModObj) {
        common(self).resModObjObj = std::move(resModObj);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The mod object to hold the newly created
:class:`IniSectionGraph` for the resource
    )doc"));

    cls.def_property("graphReplaceMode", [common](const T &self) {
        return common(const_cast<T&>(self)).graphReplaceModeObj;
    }, [common](T &self, py::object graphReplaceMode) {
        common(self).graphReplaceModeObj = std::move(graphReplaceMode);
    }, py::doc(R"doc(
:class:`IniGraphReplaceMode`: What to do when the corresponding :class:`IniSectionGraph` to
construct already exists
    )doc"));

    cls.def("clear", [](T &self) {
        self.clear();
    }, py::doc(R"doc(Clears any saved state information)doc"));

    cls.def_static("getFileId", [](const py::object &modObj, const std::string &sectionName, const py::object &part,
                                   long long orderInd, const std::string &file) {
        // Deliberately built through a throwaway instance rather than a free function: getFileId is
        // a real virtual in the core (so a subclass can change how ids are formed), while the
        // pure-Python original exposed it as a classmethod. Constructing one here keeps both true.
        PyBaseResEdit edit("", py::none(), py::none());
        std::size_t partId = part.is_none() ? 0 : part.attr("id").cast<std::size_t>();
        return edit.PyBaseResEditCore::getFileId(parseGraphId(modObj), sectionName, partId, orderInd, file);
    }, py::arg("modObj"), py::arg("sectionName"), py::arg("part"), py::arg("orderInd"), py::arg("file"),
       py::doc(R"doc(
Retrieves a unique id for a file within a single .ini file

.. note::
    The returned value is not byte-identical to the one the pure-Python original produced -- it is
    an opaque, within-one-run dictionary key that is never persisted or written to a file

Parameters
----------
modObj: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The mod object holding the newly created :class:`IniSectionGraph` for the resource

sectionName: :class:`str`
    The name of the `section`_

part: :class:`IfContentPart`
    The part where the file belongs to

orderInd: :class:`int`
    The specific order index where the file occurs in the part

file: :class:`str`
    The path for the file

Returns
-------
:class:`str`
    The unique id for the file
    )doc"));

    cls.def_static("fileAddGraphId", &T::fileAddGraphId, py::arg("file"), py::arg("graphId") = "", py::doc(R"doc(
Adds the unique id for the :class:`IniSectionGraph` of the resource to the name of the file

Parameters
----------
file: :class:`str`
    The path to the file to add the id to

graphId: :class:`str`
    The id to add :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`str`
    The file with the id added
    )doc"));

    cls.def("collectResourceName", [common](T &self, const std::string &oldResourceName, const std::string &newResourceName) {
        auto result = common(self).coreCollectResourceName(oldResourceName, newResourceName);
        return py::make_tuple(py::str(result.first), py::str(result.second));
    }, py::arg("oldResourceName"), py::arg("newResourceName"), py::doc(R"doc(
Collects the name of the fixed resource `section`_ (used for the 'collectedSections' parameter in
:meth:`buildResources`)

Parameters
----------
oldResourceName: :class:`str`
    The old name of the resource `section`_

newResourceName: :class:`str`
    The fixed name for the resource `section`_ (created by :meth:`getFixResourceName`)

Returns
-------
Tuple[:class:`str`, :class:`str`]
    A tuple where the first value is the old resource name and the second is the new resource name
    )doc"));

    cls.def("getFixResourceName", [common](T &self, const std::string &resource, const py::object &modType,
                                     const std::string &modName) -> py::object {
        (void)modType;
        std::optional<std::string> result = common(self).coreGetFixResourceName(resource, modName);
        if (!result.has_value()) {
            return py::none();
        }

        return py::str(*result);
    }, py::arg("resource"), py::arg("modType") = py::none(), py::arg("modName") = "", py::doc(R"doc(
Retrieves the name of the fixed resource `section`_

Parameters
----------
resource: :class:`str`
    The name of the original resource `section`_

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
Optional[:class:`str`]
    The `section`_ name of the fixed resource. ``None`` indicates there was no name change between
    the original resource and the fixed resource
    )doc"));

    cls.def("getFixFile", [common](T &self, const std::string &file, const py::object &modType, const std::string &modName,
                             const std::string &graphId) {
        (void)modType;
        return common(self).coreGetFixFile(file, modName, graphId);
    }, py::arg("file"), py::arg("modType") = py::none(), py::arg("modName") = "", py::arg("graphId") = "",
       py::doc(R"doc(
Retrieves the file path to the fixed resource

Parameters
----------
file: :class:`str`
    The file path to the original resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed. Unused

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

graphId: :class:`str`
    The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`str`
    The file path to the fixed resource
    )doc"));

    cls.def("renameUncollectedSection", [common](T &self, const std::string &sectionName, const py::object &modType,
                                           const std::string &modName) {
        common(self).refresh(modType);
        return self.renameUncollectedSection(sectionName, modName);
    }, py::arg("sectionName"), py::arg("modType") = py::none(), py::arg("modName") = "", py::doc(R"doc(
The name an uncollected `section`_ gets renamed to -- :meth:`getFixResourceName`, or the
`section`_'s own name when that reports no change

Parameters
----------
sectionName: :class:`str`
    The name of the `section`_

modType: Optional[:class:`ModType`]
    The type of mod being fixed

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
:class:`str`
    The new name for the `section`_
    )doc"));

    cls.def("buildResModels", [common](T &self, py::object graph, const py::object &ini, const py::object &modType,
                                 const py::object &resources, const py::object &resourceFilter, const std::string &modName,
                                 const std::string &graphId, const py::object &resModObj) {
        common(self).refresh(modType);

        PyIniResEditContext ctx(ini, modType, resources);
        PyIniGraphGroups groups{py::list()};
        auto *parsedGraph = groups.adopt(std::move(graph));
        if (parsedGraph == nullptr) {
            return;
        }

        std::optional<T::GraphId> parsedResModObj;
        if (!resModObj.is_none()) {
            parsedResModObj = parseGraphId(resModObj);
        }

        self.buildResModels(*parsedGraph, ctx, modName, parseResourceFilter(resourceFilter), graphId,
                                           parsedResModObj.has_value() ? &(*parsedResModObj) : nullptr);
    }, py::arg("graph"), py::arg("ini") = py::none(), py::arg("modType") = py::none(), py::arg("resources") = py::none(),
       py::arg("resourceFilter") = py::none(), py::arg("modName") = "", py::arg("graphId") = "",
       py::arg("resModObj") = py::none(), py::doc(R"doc(
Builds and saves the resources, given the :class:`IniSectionGraph` for a resource

Parameters
----------
graph: :class:`IniSectionGraph`
    The graph for the particular resource

ini: Optional[:class:`IniFile`]
    The .ini file to build the resource for

modType: Optional[:class:`ModType`]
    The type of mod being fixed

resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
    Where the built resource models are stored, keyed by the unique id for the source file (created
    from :meth:`getFileId`). If ``None``, the models are appended to :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
    A predicate deciding which files to build the resource for -- takes the source file and its
    assigned id :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

graphId: :class:`str`
    The unique id for the :class:`IniSectionGraph` of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

resModObj: Optional[Tuple[:class:`int`, :class:`str`, :class:`str`]]
    The mod object used to create the unique id for the resources :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc"));

    cls.def("getResGraph", [common](T &self, const py::object &collectedSections, const py::object &modType, const py::object &ini,
                              py::list graphGroups, const std::string &modName, bool rename, bool copySections) -> py::object {
        common(self).refresh(modType);

        PyIniResEditContext ctx(ini, modType);
        PyIniGraphGroups groups(graphGroups);

        auto *result = self.getResGraph(parseCollectedSections(collectedSections), ctx, groups, modName,
                                                       rename, copySections);
        if (result == nullptr) {
            return py::none();
        }

        return groups.graphToPy(result);
    }, py::arg("collectedSections"), py::arg("modType"), py::arg("ini"), py::arg("graphGroups"), py::arg("modName") = "",
       py::arg("rename") = true, py::arg("copySections") = false, py::doc(R"doc(
Retrieves the particular :class:`IniSectionGraph` for the resource

Parameters
----------
collectedSections: Dict[:class:`str`, :class:`str`]
    The target `sections`_ that reference the resource -- the keys are the old names of the
    `sections`_ and the values are the fixed names

modType: Optional[:class:`ModType`]
    The type of mod being fixed

ini: Optional[:class:`IniFile`]
    The associated original .ini file being fixed

graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

rename: :class:`bool`
    Whether to rename the `sections`_ for the graph :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

copySections: :class:`bool`
    Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
Optional[:class:`IniSectionGraph`]
    The retrieved graph
    )doc"));

    cls.def("buildResources", [common](T &self, const py::object &collectedSections, const py::object &modType, const py::object &ini,
                                 py::list graphGroups, const std::string &modName, const py::object &resourceFilter,
                                 const py::object &resources, bool copySections) {
        common(self).refresh(modType);

        PyIniResEditContext ctx(ini, modType, resources);
        PyIniGraphGroups groups(graphGroups);

        self.buildResources(parseCollectedSections(collectedSections), ctx, groups, modName,
                                           parseResourceFilter(resourceFilter), copySections);
        return graphGroups;
    }, py::arg("collectedSections"), py::arg("modType"), py::arg("ini"), py::arg("graphGroups"), py::arg("modName") = "",
       py::arg("resourceFilter") = py::none(), py::arg("resources") = py::none(), py::arg("copySections") = false,
       py::doc(R"doc(
Builds the :class:`IniSectionGraph` and the corresponding models for the resources

Parameters
----------
collectedSections: Dict[:class:`str`, :class:`str`]
    The target `sections`_ that reference the resource

modType: Optional[:class:`ModType`]
    The type of mod being fixed

ini: Optional[:class:`IniFile`]
    The associated original .ini file being fixed

graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

resourceFilter: Optional[Callable[[:class:`str`, :class:`str`], :class:`bool`]]
    A predicate deciding which files to build the resource for :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resources: Optional[Dict[:class:`str`, Deque[:class:`IniResource`]]]
    Where the built resource models are stored. If ``None``, they are appended to
    :attr:`IniFile.resources` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

copySections: :class:`bool`
    Whether to make a deep copy of the `sections`_ referenced by the graph of the resource :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

Returns
-------
List[:class:`IniGraphGroup`]
    The group of graphs that now includes the newly created graph for the resource

    .. tip::
        You can access the newly generated graph using :attr:`resModObj` on the group of graphs
    )doc"));
}


void initCppResEdit(pybind11::module_ &m);

#endif
