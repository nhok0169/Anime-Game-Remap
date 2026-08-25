#include "PyIniClassifier.h"

#include <string>
#include <unordered_set>

#include <pybind11/stl.h>

#include "AGRemapCore/model/strategies/ModTypeIdData.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIniClassifier(pybind11::module_ &m) {
    // 'Cpp'-prefixed -- matches the same temporary "wrapper-outcome-1" naming CppBaseIniClassifier/
    // CppIniClassifyStats already use (see PyIniClassifyStats.cpp's own comment) while this new
    // C++-backed classifier is built and verified in isolation. The live pure-Python original has
    // already been renamed to 'IniClassifierOld' (model/strategies/iniClassifiers/IniClassifierOld.py)
    // in preparation for this class's eventual bare-named replacement, but that migration isn't
    // finished yet -- nothing currently occupies the bare 'IniClassifier' name.
    //
    // Real pybind11 inheritance from AGRC::BaseIniClassifier (registered as "CppBaseIniClassifier"
    // in initCppBaseIniClassifier, which must run before this function -- see bindings.cpp): this
    // means classify()/clear() are inherited from CppBaseIniClassifier's own bindings and don't
    // need to be rebound here -- IniClassifier's C++ overrides are reached automatically through
    // ordinary virtual dispatch, the same as any real C++ base/derived pair (see
    // Architecture/CLAUDE.md's "To give a Python-bound class real inheritance..." note).
    py::class_<AGRC::IniClassifier, AGRC::BaseIniClassifier>(m, "CppIniClassifier", R"doc(
This class inherits from :class:`CppBaseIniClassifier`

Class to help classify the type of mod given the mod's .ini files

Parameters
----------
checkHasTextureOverride: :class:`bool`
    Whether :meth:`addGIModType`/section-name reading should require a section name to start with
    ``TextureOverride`` before doing anything else with it

    **Default**: ``True``
    )doc")

        .def(py::init<bool>(), py::arg("checkHasTextureOverride") = true)

        .def("addGIModType", &AGRC::IniClassifier::addGIModType, py::arg("modType"), py::arg("hashes"), py::arg("sectionKeywords"), py::doc(R"doc(
Registers a GI mod type into the classifier

Fails (returns ``False``) without registering anything if ``modType.modTypeId`` is already
registered, or if ``modType.gameTypeId`` isn't :attr:`GameTypeId.GI`

Parameters
----------
modType: :class:`ModTypeIdData`
    The mod type to register

hashes: Set[:class:`str`]
    The hashes that identify 'modType'

sectionKeywords: Set[:class:`str`]
    The section keywords that identify 'modType'

Returns
-------
:class:`bool`
    Whether 'modType' was newly registered
        )doc"))

        .def("addWuWaModType", &AGRC::IniClassifier::addWuWaModType, py::arg("modType"), py::arg("hashes"), py::doc(R"doc(
Registers a WuWa mod type into the classifier

Fails (returns ``False``) without registering anything if ``modType.modTypeId`` is already
registered, or if ``modType.gameTypeId`` isn't :attr:`GameTypeId.WuWa`

Parameters
----------
modType: :class:`ModTypeIdData`
    The mod type to register

hashes: Set[:class:`str`]
    The hashes that identify 'modType'

Returns
-------
:class:`bool`
    Whether 'modType' was newly registered
        )doc"))

        .def("getModType", &AGRC::IniClassifier::getModType, py::arg("modTypeId"), py::doc(R"doc(
Retrieves the registered :class:`ModTypeIdData` for a :class:`ModTypeId`

Parameters
----------
modTypeId: :class:`int`
    The id for the :class:`ModTypeId` to retrieve the :class:`ModTypeIdData` for

Raises
------
IndexError
    Raised if 'modTypeId' is not registered

Returns
-------
:class:`ModTypeIdData`
    The corresponding :class:`ModTypeIdData`
        )doc"));
}
