// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyGraphInherit.h"

#include <cstddef>
#include <memory>
#include <utility>

#include "PyGraphGroupEdit.h"  // an adder's edit is run through a real GraphGroupEdit
#include "../graphEdits/PyBaseIniGraphEdit.h"  // the isinstance target for an adder's graph edit
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the same "a bound Ranges, or a raw
                                        // list of bounds" resolver every reg edit already uses),
                                        // and the isinstance target for an adder's register edit


namespace {

// Publishes 'ini' on the edit for the duration of one forwarded 'edit' call -- see
// PyGraphInherit::currentIni. Restored (not just cleared) on the way out so a nested edit can't
// clobber an outer one, and restored even when the forwarded call raises.
class CurrentIniScope {
    public:
        CurrentIniScope(PyGraphInherit &edit, py::object ini): edit_(edit), previous_(edit.currentIni) {
            edit_.currentIni = std::move(ini);
        }

        ~CurrentIniScope() {
            edit_.currentIni = std::move(previous_);
        }

        CurrentIniScope(const CurrentIniScope&) = delete;
        CurrentIniScope& operator=(const CurrentIniScope&) = delete;

    private:
        PyGraphInherit &edit_;
        py::object previous_;
};


// Builds the per-.ini-file list a GraphGroupEdit takes ('edits' or 'keyFilters'), holding 'value'
// for the one graph at 'id' and None for every .ini file before it.
py::list singleGraphEntry(const PyGraphInherit::GraphId &id, const py::object &value) {
    py::list result;
    for (std::size_t i = 0; i < id.iniIndex; ++i) {
        result.append(py::none());
    }

    py::dict objEntry;
    objEntry[PyIniGraphGroups::modObjToPy(id.modObj)] = value;
    result.append(objEntry);
    return result;
}

}


PyGraphInherit::PyGraphInherit(py::object srcObj, py::object dstObj, std::string reg, bool latest, py::object partFilterObj,
                               py::object adderObj):
    Core(GraphId(), GraphId(), std::move(reg), latest, {}, {}),
    srcObj(std::move(srcObj)), dstObj(std::move(dstObj)), partFilterObj(std::move(partFilterObj)), adderObj(std::move(adderObj)) {}


void PyGraphInherit::refresh(PyIniGraphGroups &groups, const py::list &graphGroups, const py::object &modType) {
    src = parseGraphId(srcObj);
    dst = parseGraphId(dstObj);

    partFilter = {};
    if (!partFilterObj.is_none()) {
        py::object filterObj = partFilterObj;
        py::object ini = currentIni;

        // 'modType' and 'ini' are captured from the Python side rather than travelling through the
        // C++ signature: the core takes a nullable AGRC::ModType* / AGRC::IniFile*, and the
        // Python-side ModType/IniFile have nothing castable to hand over.
        partFilter = [filterObj, modType, ini](const IterData &iterData, const AGRC::ModType *, AGRC::IniFile *) -> OrderRanges {
            py::object result = filterObj(py::cast(&iterData, py::return_value_policy::reference), modType, ini);

            PyPartRanges parsed(result);
            const OrderRanges *ranges = parsed.get();
            if (ranges == nullptr) {
                throw py::type_error("A GraphInherit partFilter must return a Ranges (or a list of (start, end) bounds), not None");
            }

            return *ranges;
        };
    }

    adder = {};
    if (adderObj.is_none()) {
        return;
    }

    py::object heldAdder = adderObj;
    py::object heldPartFilter = partFilterObj;
    py::object heldModType = modType;
    py::object heldIni = currentIni;
    py::list heldGraphGroups = graphGroups;
    PyIniGraphGroups *heldGroups = &groups;
    GraphId heldSrc = src;

    adder = [heldAdder, heldPartFilter, heldModType, heldIni, heldGraphGroups, heldGroups, heldSrc](
                Graph &srcGraph, const KVPs &kvps, AGRC::IniFile *, const AGRC::ModType *, const std::string &modName) -> AddEdit {
        py::list kvpList;
        for (const auto &kvp : kvps) {
            kvpList.append(py::make_tuple(kvp.first, kvp.second));
        }

        py::object result = heldAdder(heldGroups->graphToPy(&srcGraph), kvpList, heldIni, heldModType, modName);
        if (result.is_none()) {
            return {};
        }

        if (!py::isinstance<PyBaseRegEdit>(result) && !py::isinstance<PyBaseIniGraphEdit>(result)) {
            throw py::type_error("A GraphInherit adder must return a BaseRegEdit, a BaseIniGraphEdit, or None");
        }

        // Run through a real GraphGroupEdit rather than handed back to the core as a C++ base
        // pointer: with no trampoline, a C++ call would skip a pure-Python subclass's own 'edit'.
        // The core would run it through GraphGroupEdit::editSectionGraph anyway, so this is the
        // same dispatch, reached from the Python side.
        py::list edits = singleGraphEntry(heldSrc, py::make_tuple(result));
        py::object keyFilters = py::none();
        if (!heldPartFilter.is_none()) {
            keyFilters = singleGraphEntry(heldSrc, py::make_tuple(heldPartFilter));
        }

        py::object groupEdit = py::type::of<PyGraphGroupEdit>()(edits, py::arg("keyFilters") = keyFilters);
        if (heldIni.is_none()) {
            groupEdit.attr("edit")(heldGraphGroups, heldModType, py::arg("modName") = modName);
        } else {
            groupEdit.attr("editFromIni")(heldGraphGroups, heldIni, heldModType, py::arg("modName") = modName);
        }

        return {};
    };
}


void initCppGraphInherit(pybind11::module_ &m) {
    py::class_<PyGraphInherit, PyBaseIniGraphGroupEdit, py::smart_holder> cls(m, "GraphInherit", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Merges the graph at 'dst' into the graph at 'src', by inserting consecutive `KVPs`_ into 'src' that
reference every root `section`_ of the graph at 'dst'

Each `KVP`_ is ``<reg> = <a root of dst>``, so besides the ``run =`` call the name suggests, this
can also compose a graph of resources into a graph that uses them. For example, with 'dst' as the
graph of ``[ResourceHeadDiffuse]`` and 'reg' as ``ps-t0``, a ``[TextureOverrideComponent0]`` in
'src' that binds no textures of its own gains ``ps-t0 = ResourceHeadDiffuse``

Where the `KVPs`_ go is decided by 'adder' when one is given, and otherwise by 'latest' and
'partFilter' (see :meth:`edit`)

.. note::
    This only inserts the reference `KVPs`_ into 'src' -- the `sections`_ of 'dst' themselves are
    left untouched (and still need to be reachable/present elsewhere for the reference to resolve,
    the same way a plain ``run =`` reference to another `section`_ works)

.. note::
    If either the graph at 'src' or the graph at 'dst' cannot be found, nothing is inserted and the
    original 'graphGroups' is returned as-is -- no exception is raised

Examples
--------
Binding a diffuse and a light map into a section that binds neither, after its ``hash`` and
``match_*`` `KVPs`_ (and after any ``ps-t`` register an earlier edit already bound there):

.. code-block:: python
    :linenos:

    def afterHeader(iterData, modType, ini):
        part = iterData.part
        if ("hash" not in part):
            return FRB.Ranges.createEmpty()

        header = [i for i in range(len(part)) if (part[i][0] in {"hash", "match_first_index", "match_index_count"} or part[i][0].startswith("ps-t"))]
        return FRB.Ranges([(max(header) + 1, None)])

    addAtFront = lambda srcGraph, kvps, ini, modType, modName: FRB.RegAdd(kvps, latest = False)

    edits = [FRB.GraphInherit((0, "", "Component0"), (0, "", "HeadDiffuse"), "ps-t0", partFilter = afterHeader, adder = addAtFront),
             FRB.GraphInherit((0, "", "Component0"), (0, "", "HeadLightMap"), "ps-t1", partFilter = afterHeader, adder = addAtFront)]

Parameters
----------
src: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the source :class:`IniSectionGraph` to insert the reference `KVPs`_ into. The tuple contains: :raw-html:`<br />` :raw-html:`<br />`

    #. The index for the .ini file
    #. The name of the component
    #. The name of the object

dst: Tuple[:class:`int`, :class:`str`, :class:`str`]
    The id of the :class:`IniSectionGraph` to merge into 'src'. Same tuple format as 'src'

reg: :class:`str`
    The name of the register used to reference the root `sections`_ of the graph at 'dst'

latest: :class:`bool`
    Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the front. Unused when 'adder' is given :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``

partFilter: Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]
    The filter used to indicate which areas of some :class:`IfContentPart` within the graph at 'src'
    are valid to insert the `KVPs`_ :raw-html:`<br />` :raw-html:`<br />`

    If this value is ``None``, then the `KVPs`_ are instead inserted directly at the very
    front/back (based on 'latest') of every root `section`_ of the graph at 'src', instead of being
    filtered through every :class:`IfContentPart` of the graph :raw-html:`<br />` :raw-html:`<br />`

    When 'adder' is given, this is instead the key filter the edit 'adder' returns is run with, the
    same way a :class:`GraphGroupEdit` hands its 'keyFilters' to its edits :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

adder: Optional[Callable[[:class:`IniSectionGraph`, List[Tuple[:class:`str`, :class:`str`]], Optional[:class:`IniFile`], :class:`ModType`, :class:`str`], Optional[Union[:class:`BaseRegEdit`, :class:`BaseIniGraphEdit`]]]]
    Decides how the reference `KVPs`_ are added to the graph at 'src'. It is called once per edit
    with: :raw-html:`<br />` :raw-html:`<br />`

    #. The graph at 'src'
    #. The `KVPs`_ to add, as ``(reg, rootName)`` pairs in the order of the roots of the graph at 'dst'
    #. The .ini file being fixed, or ``None`` when the edit was not given one
    #. The type of mod to fix
    #. The name of the mod to fix to

    and returns either:

    * a :class:`BaseRegEdit` or :class:`BaseIniGraphEdit` built from the `KVPs`_ (eg. a
      :class:`RegAdd` or a :class:`RegSurroundedAdd`), which is then run over the graph at 'src'
      the same way a :class:`GraphGroupEdit` would run it, with 'partFilter' as its key filter
    * ``None``, when the adder already inserted the `KVPs`_ itself

    If this value is ``None``, 'latest' and 'partFilter' decide instead :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    // py::init(factory) rather than py::init<...>(): the core class owns std::function members, and
    // a factory returning a unique_ptr avoids ever needing to move-construct the class itself --
    // see PyRegAdd.cpp's identical note.
    cls.def(py::init([](py::object src, py::object dst, std::string reg, bool latest, py::object partFilter, py::object adder) {
        return std::make_unique<PyGraphInherit>(std::move(src), std::move(dst), std::move(reg), latest, std::move(partFilter),
                                                std::move(adder));
    }), py::arg("src"), py::arg("dst"), py::arg("reg"), py::arg("latest") = true, py::arg("partFilter") = py::none(),
        py::arg("adder") = py::none());

    cls.def_property("src", [](const PyGraphInherit &self) {
        return self.srcObj;
    }, [](PyGraphInherit &self, py::object src) {
        self.srcObj = std::move(src);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the source :class:`IniSectionGraph` to
insert the reference `KVPs`_ into
    )doc"));

    cls.def_property("dst", [](const PyGraphInherit &self) {
        return self.dstObj;
    }, [](PyGraphInherit &self, py::object dst) {
        self.dstObj = std::move(dst);
    }, py::doc(R"doc(
Tuple[:class:`int`, :class:`str`, :class:`str`]: The id of the :class:`IniSectionGraph` to merge
into :attr:`src`
    )doc"));

    cls.def_readwrite("reg", &PyGraphInherit::reg, py::doc(R"doc(
:class:`str`: The name of the register used to reference the root `sections`_ of the graph at
:attr:`dst`
    )doc"));

    cls.def_readwrite("latest", &PyGraphInherit::latest, py::doc(R"doc(
:class:`bool`: Whether to insert the `KVPs`_ at the back of the areas to insert, instead of at the
front. Unused when :attr:`adder` is set
    )doc"));

    cls.def_property("partFilter", [](const PyGraphInherit &self) {
        return self.partFilterObj;
    }, [](PyGraphInherit &self, py::object partFilter) {
        self.partFilterObj = std::move(partFilter);
    }, py::doc(R"doc(
Optional[Callable[[:class:`SectionIterData`, :class:`ModType`, Optional[:class:`IniFile`]], :class:`Ranges`]]:
The filter used to indicate which areas of some :class:`IfContentPart` within the graph at
:attr:`src` are valid to insert the `KVPs`_. When :attr:`adder` is set, this is instead the key
filter the adder's edit is run with
    )doc"));

    cls.def_property("adder", [](const PyGraphInherit &self) {
        return self.adderObj;
    }, [](PyGraphInherit &self, py::object adder) {
        self.adderObj = std::move(adder);
    }, py::doc(R"doc(
Optional[Callable[[:class:`IniSectionGraph`, List[Tuple[:class:`str`, :class:`str`]], Optional[:class:`IniFile`], :class:`ModType`, :class:`str`], Optional[Union[:class:`BaseRegEdit`, :class:`BaseIniGraphEdit`]]]]:
Decides how the reference `KVPs`_ are added to the graph at :attr:`src` -- see the class's own
'adder' parameter
    )doc"));

    cls.def("edit", [](PyGraphInherit &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        PyIniGraphGroups groups(graphGroups);
        self.refresh(groups, graphGroups, modType);

        // nullptr for modType: the real Python object is captured by the rebuilt partFilter and
        // adder instead -- see PyGraphInherit::refresh.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Inserts the reference `KVPs`_ from the graph at :attr:`dst` into the graph at :attr:`src`

With an :attr:`adder`, the adder decides: the edit it returns is run over the graph at :attr:`src`
with :attr:`partFilter` as its key filter, or, if it returns ``None``, it is taken to have inserted
the `KVPs`_ itself

Without one, and with no :attr:`partFilter`, the `KVPs`_ go straight to the very front/back (based
on :attr:`latest`) of every root `section`_ of the graph at :attr:`src`. With a :attr:`partFilter`,
they instead go at the earliest/latest valid index of every :class:`IfContentPart` the filter
accepts

Nothing is added, and :attr:`adder` is not called, when the graph at :attr:`dst` has no roots

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix. Only ever handed to :attr:`partFilter` and :attr:`adder`

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));

    // Routed through self.attr("edit") rather than straight to the core editFromIni, so a
    // pure-Python subclass overriding only 'edit' still has it run -- see PyRegFillMissing.cpp's
    // identical note. 'ini' reaches the rebuilt partFilter and adder through PyGraphInherit::currentIni.
    cls.def("editFromIni", [](py::object self, py::list graphGroups, const py::object &ini, const py::object &modType,
                              const std::string &modName) -> py::object {
        CurrentIniScope iniScope(py::cast<PyGraphInherit &>(self), ini);
        return self.attr("edit")(graphGroups, modType, py::arg("modName") = modName);
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
The same as :meth:`edit`, except that 'ini' is also handed to :attr:`partFilter`, :attr:`adder`,
and the edit the adder returns

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: Optional[:class:`IniFile`]
    The .ini file being fixed

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));
}
