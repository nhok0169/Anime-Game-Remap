#include "PyGIMISectionClassifier.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/stl.h>

#include "AGRemapCore/data/HashToModObjData.h"

#include "../../PyVersion.h"
#include "../../assets/PyModDictAssets.h"
#include "../../assets/PyModMappedAssets.h"
#include "../../iftemplate/PyIfContentPartColour.h"
#include "../../iftemplate/PyIfTemplate.h"


namespace {

using ModObj = PyGIMISectionClassifier::ModObj;
using Core = PyGIMISectionClassifierCore;


// Every real mod object key in this codebase is a plain (componentName, objectName) tuple of str.
// Anything else can't identify a mod object at all, so it maps to the empty pair rather than
// raising -- the same lenient shape PyIniGraphGroups::modObjFromPy already uses for graph keys.
ModObj modObjFromPy(const py::handle &value) {
    if (!py::isinstance<py::tuple>(value)) {
        return ModObj();
    }

    py::tuple asTuple = py::reinterpret_borrow<py::tuple>(value);
    if (asTuple.size() < 2) {
        return ModObj();
    }

    return ModObj(py::str(asTuple[0]).cast<std::string>(), py::str(asTuple[1]).cast<std::string>());
}


py::tuple modObjToPy(const ModObj &modObj) {
    return py::make_tuple(py::str(modObj.first), py::str(modObj.second));
}


// The two-element key of the inner indexKeyToModObj dicts -- an (component, object) tuple, but of
// whatever the Indices table's own last two index columns hold, so kept as py::object rather than
// narrowed to std::string like a mod object key is.
Core::IndexKey indexKeyFromPy(const py::handle &value) {
    if (!py::isinstance<py::tuple>(value)) {
        return Core::IndexKey(py::none(), py::none());
    }

    py::tuple asTuple = py::reinterpret_borrow<py::tuple>(value);
    if (asTuple.size() < 2) {
        return Core::IndexKey(py::none(), py::none());
    }

    return Core::IndexKey(py::reinterpret_borrow<py::object>(asTuple[0]), py::reinterpret_borrow<py::object>(asTuple[1]));
}


// The every-mod-type-shares-them default mappings, in the Python dict form this class keeps them
// in. Built fresh per call rather than cached in a static py::object: a py::object that outlives
// the interpreter crashes at shutdown, and these are small enough that it does not matter.
py::dict defaultHashKeyOnlyToModObj() {
    py::dict result;

    for (const std::pair<const std::string, AGRC::Data::ModObjKey> &entry : AGRC::Data::getHashKeyOnlyToModObj()) {
        result[py::str(entry.first)] = modObjToPy(ModObj(entry.second.first, entry.second.second));
    }

    return result;
}


py::dict defaultIndexKeyToModObj() {
    py::dict result;

    for (const std::pair<const std::string, AGRC::Data::IndexKeyToModObj> &entry : AGRC::Data::getIndexKeyToModObj()) {
        py::dict inner;

        for (const std::pair<const AGRC::Data::ModObjKey, AGRC::Data::ModObjKey> &innerEntry : entry.second) {
            inner[py::make_tuple(py::str(innerEntry.first.first), py::str(innerEntry.first.second))] =
                modObjToPy(ModObj(innerEntry.second.first, innerEntry.second.second));
        }

        result[py::str(entry.first)] = std::move(inner);
    }

    return result;
}


PyModMappedAssets* assetsOf(const py::object &raw) {
    if (raw.is_none()) {
        return nullptr;
    }

    return raw.cast<PyModMappedAssets*>();
}


// The pure-Python original normalizes its raw bare-value/list/dict filter through the asset table's
// own '_convertNonVersionVals'. That is exactly PyModMappedAssets::nonVersionIndexNames +
// toWildcardList -- and a generic table that never got any index names has nothing to normalize
// against, so it filters on nothing at all.
std::vector<std::optional<py::object>> convertNonVersionVals(PyModMappedAssets *assets, const py::object &raw) {
    if (assets == nullptr || !assets->nonVersionIndexNames.has_value()) {
        return {};
    }

    return toWildcardList(raw, *assets->nonVersionIndexNames);
}

}


PyGIMISectionClassifier::Core::ClassifierConfig PyGIMISectionClassifier::makeConfig() {
    Core::ClassifierConfig result{};
    result.hashKey = py::cast(AGRC::IniKeywords::Hash);
    result.matchFirstIndexKey = py::cast(AGRC::IniKeywords::MatchFirstIndex);
    return result;
}


PyGIMISectionClassifier::PyGIMISectionClassifier(py::object hashKeyOnlyToModObj, py::object hashes, py::object indexKeyToModObj,
                                                  py::object indices, py::object version, py::object hashNonVersionVals,
                                                  py::object indexNonVersionVals):
    Core({}, nullptr, {}, nullptr, std::nullopt, makeConfig()),
    hashKeyOnlyToModObjObj(std::move(hashKeyOnlyToModObj)), indexKeyToModObjObj(std::move(indexKeyToModObj)),
    hashesObj(std::move(hashes)), indicesObj(std::move(indices)), versionObj(std::move(version)),
    hashNonVersionValsObj(std::move(hashNonVersionVals)), indexNonVersionValsObj(std::move(indexNonVersionVals)) {

    if (hashKeyOnlyToModObjObj.is_none()) {
        hashKeyOnlyToModObjObj = py::dict();
    }
    if (indexKeyToModObjObj.is_none()) {
        indexKeyToModObjObj = py::dict();
    }

    refresh();
}


void PyGIMISectionClassifier::refresh() {
    PyModMappedAssets *hashAssets = assetsOf(hashesObj);
    PyModMappedAssets *indexAssets = assetsOf(indicesObj);

    setHashes(hashAssets);
    setIndices(indexAssets);
    setHashNonVersionVals(convertNonVersionVals(hashAssets, hashNonVersionValsObj));
    setIndexNonVersionVals(convertNonVersionVals(indexAssets, indexNonVersionValsObj));
    version = parseVersionArg(versionObj);

    hashKeyOnlyToModObj.clear();
    if (!hashKeyOnlyToModObjObj.is_none()) {
        for (auto item : hashKeyOnlyToModObjObj.cast<py::dict>()) {
            hashKeyOnlyToModObj[py::reinterpret_borrow<py::object>(item.first)] = modObjFromPy(item.second);
        }
    }

    indexKeyToModObj.clear();
    if (!indexKeyToModObjObj.is_none()) {
        for (auto item : indexKeyToModObjObj.cast<py::dict>()) {
            IndexModObjs inner;

            for (auto innerItem : py::reinterpret_borrow<py::object>(item.second).cast<py::dict>()) {
                inner[indexKeyFromPy(innerItem.first)] = modObjFromPy(innerItem.second);
            }

            indexKeyToModObj[py::reinterpret_borrow<py::object>(item.first)] = std::move(inner);
        }
    }
}


py::list PyGIMISectionClassifier::classifyFromPy(const std::string &sectionName, const py::object &section, const py::object &partKeys) {
    refresh();

    py::list result;
    if (partKeys.is_none()) {
        return result;
    }

    PyIfTemplate *sectionPtr = section.is_none() ? nullptr : section.cast<PyIfTemplate*>();
    auto *colouring = partKeys.cast<PyIfContentPartColouring*>();

    for (const ModObj &modObj : classify(sectionName, sectionPtr, *colouring)) {
        result.append(modObjToPy(modObj));
    }

    return result;
}


void initCppGIMISectionClassifier(pybind11::module_ &m) {
    py::class_<PyGIMISectionClassifier>(m, "GIMISectionClassifier", R"doc(
A callable class used to classify `sections`_ based on their ``hash`` value and their
``match_first_index`` value

:raw-html:`<br />`

.. container:: operations

    **Supported Operations:**

    .. describe:: x(parser, sectionName, section, disjoint, part, kvps)

        Classifies the mod objects based on the current :class:`IfContentPart`. For more details on
        the arguments to pass, see :attr:`GIMIParser.objTargetFuncs`

Parameters
----------
hashKeyOnlyToModObj: Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]
    Mapping for mod objects that are only identified by ``hash`` value :raw-html:`<br />` :raw-html:`<br />`

    The keys are the names for the type of hashes (most inner keys at :attr:`ModData.Hashes`) and
    the values are tuples that contain the corresponding component and mod object to classify the
    `section`_

hashes: :class:`Hashes`
    The assets for the ``hashes``

indexKeyToModObj: Optional[Dict[:class:`str`, Dict[Tuple[:class:`str`, :class:`str`], Tuple[:class:`str`, :class:`str`]]]]
    Mapping for mod objects that are identified by both ``hash`` value and their
    ``match_first_index`` value :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

indices: Optional[:class:`Indices`]
    The assets for the ``match_first_index`` values :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
    The version of the .ini file. If ``None``, assumes the latest version :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

hashNonVersionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
    The filter values used when searching :attr:`hashes` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``UnHashableNone``

indexNonVersionVals: Optional[Union[`Hashable`_, List[`Hashable`_], Dict[:class:`str`, `Hashable`_]]]
    The filter values used when searching :attr:`indices` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``UnHashableNone``
    )doc")

        .def(py::init([](py::object hashKeyOnlyToModObj, py::object hashes, py::object indexKeyToModObj, py::object indices,
                          py::object version, py::object hashNonVersionVals, py::object indexNonVersionVals) {
            return std::make_unique<PyGIMISectionClassifier>(std::move(hashKeyOnlyToModObj), std::move(hashes),
                                                              std::move(indexKeyToModObj), std::move(indices), std::move(version),
                                                              std::move(hashNonVersionVals), std::move(indexNonVersionVals));
        }), py::arg("hashKeyOnlyToModObj"), py::arg("hashes"), py::arg("indexKeyToModObj") = py::none(),
            py::arg("indices") = py::none(), py::arg("version") = py::none(),
            py::arg("hashNonVersionVals") = py::none(), py::arg("indexNonVersionVals") = py::none())

        .def_readwrite("hashKeyOnlyToModObj", &PyGIMISectionClassifier::hashKeyOnlyToModObjObj,
            py::doc(R"doc(Dict[:class:`str`, Tuple[:class:`str`, :class:`str`]]: Mapping for mod objects that are only identified by ``hash`` value)doc"))

        .def_readwrite("indexKeyToModObj", &PyGIMISectionClassifier::indexKeyToModObjObj,
            py::doc(R"doc(Dict[:class:`str`, Dict[Tuple[:class:`str`, :class:`str`], Tuple[:class:`str`, :class:`str`]]]: Mapping for mod objects that are identified by both ``hash`` value and their ``match_first_index`` value)doc"))

        .def_readwrite("hashes", &PyGIMISectionClassifier::hashesObj,
            py::doc(R"doc(:class:`Hashes`: The assets for the ``hash`` values)doc"))

        .def_readwrite("indices", &PyGIMISectionClassifier::indicesObj,
            py::doc(R"doc(Optional[:class:`Indices`]: The assets for the ``match_first_index`` values)doc"))

        .def_readwrite("version", &PyGIMISectionClassifier::versionObj,
            py::doc(R"doc(Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]: The version of the .ini file)doc"))

        .def_readwrite("hashNonVersionVals", &PyGIMISectionClassifier::hashNonVersionValsObj,
            py::doc(R"doc(The filter values used when searching :attr:`hashes`)doc"))

        .def_readwrite("indexNonVersionVals", &PyGIMISectionClassifier::indexNonVersionValsObj,
            py::doc(R"doc(The filter values used when searching :attr:`indices`)doc"))

        .def("classify", [](PyGIMISectionClassifier &self, const std::string &sectionName, const py::object &section, const py::object &partKeys) {
            return self.classifyFromPy(sectionName, section, partKeys);
        }, py::arg("sectionName"), py::arg("section"), py::arg("partKeys"), py::doc(R"doc(
Classifies which mod objects a particular :class:`IfContentPart` belongs to

Parameters
----------
sectionName: :class:`str`
    The name of the `section`_ where the part belongs in

section: :class:`IfTemplate`
    The `section`_ where the part belongs in

partKeys: :class:`IfContentPartColouring`
    The current state of the `KVPs`_ for the part

Returns
-------
List[Tuple[:class:`str`, :class:`str`]]
    The classified mod objects
        )doc"))

        .def("__call__", [](PyGIMISectionClassifier &self, const py::object &parser, const std::string &sectionName,
                             const py::object &section, bool disjoint, const py::object &part, const py::object &kvps) {
            (void)parser;
            (void)disjoint;
            (void)part;
            return self.classifyFromPy(sectionName, section, kvps);
        }, py::arg("parser"), py::arg("sectionName"), py::arg("section"), py::arg("disjoint"), py::arg("part"), py::arg("kvps"))

        .def_static("buildDefaultClassifier", [](const py::object &modType, const py::object &version) {
            // py::object(...) on both branches deliberately: a bare `cond ? py::none() : obj`
            // collapses to py::none, whose converting constructor then rejects the real object at
            // runtime with "Object of type 'X' is not an instance of 'none'".
            py::object hashes = modType.is_none() ? py::object(py::none()) : py::object(modType.attr("hashes"));
            py::object indices = modType.is_none() ? py::object(py::none()) : py::object(modType.attr("indices"));

            return std::make_unique<PyGIMISectionClassifier>(defaultHashKeyOnlyToModObj(), std::move(hashes),
                                                              defaultIndexKeyToModObj(), std::move(indices),
                                                              version, py::none(), py::none());
        }, py::arg("modType"), py::arg("version") = py::none(), py::doc(R"doc(
Builds the default classifier for the `sections`_

:raw-html:`<br />`

The classifier comes pre-filled with the mod object mappings every mod type shares -- one entry per
hash type the hash data table ships, named by that hash type's own key (``blend_vb`` becomes
``("", "blend_vb")``, and a ``compName;objName`` key becomes ``(compName, objName)``). The
``ib``-suffixed hash types land in :attr:`GIMISectionClassifier.indexKeyToModObj` instead, since a
``hash`` value alone can't tell those apart; every other one lands in
:attr:`GIMISectionClassifier.hashKeyOnlyToModObj` :raw-html:`<br />` :raw-html:`<br />`

The mappings are this classifier's own :class:`dict`\s -- editing them to add whatever else a
particular mod type needs won't write through to any other classifier

Parameters
----------
modType: :class:`ModType`
    The type of mod

version: Optional[Union[:class:`str`, :class:`float`, :class:`CppVersion`]]
    The version of the .ini file. If ``None``, assumes the latest version :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

Returns
-------
:class:`GIMISectionClassifier`
    The built classifier
        )doc"))

        .def_static("buildDefaultClassifierFromIni", [](const py::object &ini) {
            py::object modType = ini.attr("availableType");
            // py::object(...) on both branches deliberately: a bare `cond ? py::none() : obj`
            // collapses to py::none, whose converting constructor then rejects the real object at
            // runtime with "Object of type 'X' is not an instance of 'none'".
            py::object hashes = modType.is_none() ? py::object(py::none()) : py::object(modType.attr("hashes"));
            py::object indices = modType.is_none() ? py::object(py::none()) : py::object(modType.attr("indices"));

            return std::make_unique<PyGIMISectionClassifier>(defaultHashKeyOnlyToModObj(), std::move(hashes),
                                                              defaultIndexKeyToModObj(), std::move(indices),
                                                              ini.attr("version"), py::none(), py::none());
        }, py::arg("ini"), py::doc(R"doc(
Builds the default classifier for the `sections`_ from a .ini file

:raw-html:`<br />`

The classifier comes pre-filled with the mod object mappings every mod type shares -- one entry per
hash type the hash data table ships, named by that hash type's own key (``blend_vb`` becomes
``("", "blend_vb")``, and a ``compName;objName`` key becomes ``(compName, objName)``). The
``ib``-suffixed hash types land in :attr:`GIMISectionClassifier.indexKeyToModObj` instead, since a
``hash`` value alone can't tell those apart; every other one lands in
:attr:`GIMISectionClassifier.hashKeyOnlyToModObj` :raw-html:`<br />` :raw-html:`<br />`

The mappings are this classifier's own :class:`dict`\s -- editing them to add whatever else a
particular mod type needs won't write through to any other classifier

Parameters
----------
ini: :class:`IniFile`
    The .ini file

Returns
-------
:class:`GIMISectionClassifier`
    The built classifier
        )doc"));
}
