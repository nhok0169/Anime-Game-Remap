#ifndef AGRemapPyBind_PyRegFillMissing_H
#define AGRemapPyBind_PyRegFillMissing_H

#include <optional>

#include <pybind11/pybind11.h>

#include "PyBaseIniGraphEdit.h"
#include "AGRemapCore/constants/DownloadMode.h"
#include "AGRemapCore/constants/RegFillMissingMode.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegFillMissing.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The still-pure-Python ``RegFillMissingMode`` enum class object
 (``constants/RegFillMissingMode.py``), imported lazily on first use :raw-html:`<br />`
 :raw-html:`<br />`

 Only needed to materialize this edit's ``fillMode`` **default**, so that an omitted argument reads
 back as the real ``RegFillMissingMode.FillMissing`` member rather than ``None`` -- every other use
 goes through `parseFillMissingMode`, which reads the member's ``.value`` string instead and so
 needs no import at all. Deliberately **not** :cpp:enum:`AGRemapCore::RegFillMissingMode`, which is
 an unrelated C++ enum the `Python`_ side never sees
 @endrst
 */
py::object pyRegFillMissingModeEnum();


/**
 * @brief Converts a Python ``RegFillMissingMode`` member into the core enum
 *
 * @param mode The Python value to convert -- anything unrecognized (including ``None``) reads as ``RegFillMissingMode::FillMissing``
 */
AGRC::RegFillMissingMode parseFillMissingMode(const py::object &mode);


/**
 * @brief
 @rst
 Reads a `Python`_-side ``IniFile``'s own ``downloadMode`` and converts it into the core enum
 :raw-html:`<br />` :raw-html:`<br />`

 An ``ini`` that is ``None``, or that carries no ``downloadMode`` attribute at all, reads as
 ``DownloadMode::Normal`` -- the mode under which this edit behaves identically to
 ``dependOnDownload`` being ``False``. The pure-Python original raised ``AttributeError`` for that
 case instead; nothing depends on the crash, and a graph edit is routinely handed a stand-in ``ini``
 by ``GraphGroupEdit``'s own callers
 @endrst
 *
 * @param ini The Python .ini file to read the download mode off
 */
AGRC::DownloadMode parseIniDownloadMode(const py::object &ini);


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegFillMissing`\\<py::object, py::object\\>
 :raw-html:`<br />` :raw-html:`<br />`

 A subclass rather than a plain alias (the same reason `PyGraphRename` is one): it holds the
 **exact** `Python`_ objects the caller passed for ``fillMissing`` and ``fillMode``. The pure-Python
 original simply stored them, so ``someEdit.fillMissing is theThingYouPassed`` held -- and for the
 callable shape of ``fillMissing``, `pybind11`_'s ``std::function`` caster could not hand it back as
 the same callable anyway. The C++ members are re-derived from them at the start of every
 ``edit``/``editFromIni`` (see #refresh) :raw-html:`<br />` :raw-html:`<br />`

 ``reg`` needs no such mirror -- the core member is already a ``py::object``, since this
 instantiation's ``K`` is one
 @endrst
 */
class PyRegFillMissing: public AGRC::RegFillMissing<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegFillMissing<py::object, py::object, PyObjectHash, PyObjectEqual>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``fillMissing`` -- a bare value, a list of `KVP`_
         tuples, or a callable taking the :class:`IfContentPart` to fill
         @endrst
         */
        py::object fillMissingObj;

        /**
         * @brief
         @rst
         The exact `Python`_ ``RegFillMissingMode`` member given for ``fillMode`` -- ``None`` at
         construction is replaced by the real ``RegFillMissingMode.FillMissing`` member, so this
         never reads back as ``None``
         @endrst
         */
        py::object fillModeObj;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``keysToTrack`` -- a set/iterable of keys, or ``None``
         for "every key" :raw-html:`<br />` :raw-html:`<br />`

         Mirrored the same way ``fillMissing``/``fillMode`` are, so ``someEdit.keysToTrack is
         theSetYouPassed`` holds and an in-place mutation of that set is honoured by the next
         ``edit``
         @endrst
         */
        py::object keysToTrackObj;

        /**
         * @brief
         @rst
         The ``.ini`` file the *current* ``editFromIni`` is running for, or ``None`` when this edit
         was reached through a bare ``edit`` :raw-html:`<br />` :raw-html:`<br />`

         Needed because ``editFromIni`` deliberately forwards through
         ``self.attr("edit")`` (so a pure-`Python`_ subclass overriding only ``edit`` still wins --
         see that binding's own comment), and ``edit``'s signature has no ``ini`` parameter to carry
         it in. Without this, a ``partFilter`` reached through ``editFromIni`` is handed ``None``
         for its third argument even though the caller had a real ``IniFile`` -- which
         ``GraphGroupEdit``'s own ``keyFilters`` do get, making the two paths silently disagree
         @endrst
         */
        py::object currentIni = py::none();

        /**
         * @brief Constructs a new missing-register-filling edit
         *
         * @param regObj The register to search for
         * @param fillMissingObj How to fill the parts missing that register
         * @param fillModeObj The Python ``RegFillMissingMode`` member, or ``None`` for ``RegFillMissingMode.FillMissing``
         * @param dependOnDownload Whether the editing is dependent on the .ini file's download mode
         * @param trackKeys Whether to track `KVPs`_ for colouring while walking the graph
         * @param keysToTrackObj Which keys to track, or ``None`` for all of them
         */
        PyRegFillMissing(py::object regObj, py::object fillMissingObj, py::object fillModeObj, bool dependOnDownload,
                          bool trackKeys, py::object keysToTrackObj);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``fillMode``/``fillMissing`` members from #fillModeObj and
         #fillMissingObj -- called at the start of every ``edit``/``editFromIni`` so an in-place
         reassignment of either is honoured, exactly as it was for the pure-Python original
         :raw-html:`<br />` :raw-html:`<br />`

         The two are re-derived **together**, not independently: which end of a part a bare
         value/`KVP`_ list gets added to is decided by the *mode*
         (``RegFillMissingMode.TopdownCover`` adds to the front), matching the pure-Python
         original's own ``_getFillMissingFunc(self.fillMissing, toFront = isCoverMode)``
         @endrst
         */
        void refresh();
};


/**
 * @brief
 @rst
 Builds the :cpp:type:`AGRemapCore::RegFillMissing::FillMissingFunc` one of ``fillMissing``'s three
 `Python`_ shapes means :raw-html:`<br />` :raw-html:`<br />`

 A ``str`` adds ``reg = fillMissing``; a ``list`` adds every `KVP`_ tuple it holds; anything else
 callable is used as the modification itself. ``None`` (or a non-callable of any other type) yields
 an empty function, which every mode treats as a no-op
 @endrst
 *
 * @param fillMissing The Python value to build from
 * @param reg The register to add, for the ``str`` shape
 * @param toFront Whether the `KVPs`_ are added to the front of the part instead of the back
 */
PyRegFillMissing::Core::FillMissingFunc parseFillMissing(const py::object &fillMissing, const py::object &reg, bool toFront);


/**
 * @brief
 @rst
 Converts a `Python`_ ``keysToTrack`` argument into the core's own optional key set
 :raw-html:`<br />` :raw-html:`<br />`

 ``None`` becomes ``std::nullopt``, meaning **every** key is tracked -- the same convention
 :cpp:func:`AGRemapCore::IfContentPartColouring::updateColouring` and :cpp:class:`GraphGroupEdit`
 already use. Any other iterable is materialized into a real key set
 @endrst
 *
 * @param keysToTrack The Python value to convert
 */
std::optional<PyRegFillMissing::Core::KeySet> parseKeysToTrack(const py::object &keysToTrack);


/**
 * @brief
 @rst
 Wraps a `Python`_ ``partFilter`` callable as the core
 :cpp:type:`AGRemapCore::BaseIniGraphEdit::PartFilter` :cpp:func:`AGRemapCore::RegFillMissing::edit`
 takes :raw-html:`<br />` :raw-html:`<br />`

 The `Python`_ callable is invoked as ``partFilter(iterData, modType, ini)`` with the *captured*
 `Python`_ ``modType``/``ini`` objects rather than the core's own (always-null) pointers -- the same
 shape, and the same reason, as :cpp:class:`GraphGroupEdit`'s own ``keyFilters``. ``None`` (or any
 non-callable) yields an empty function, which accepts every part
 @endrst
 *
 * @param partFilter The Python callable to wrap
 * @param modType The Python mod type to hand it
 * @param ini The Python .ini file to hand it
 */
PyRegFillMissing::Core::PartFilter parsePartFilter(const py::object &partFilter, py::object modType, py::object ini);


/**
 * @brief Registers the Python-facing ``RegFillMissing``
 *
 * @param m The module to register into
 */
void initCppRegFillMissing(pybind11::module_ &m);

#endif
