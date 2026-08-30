#ifndef AGRemapPyBind_PyGIMIFixer_H
#define AGRemapPyBind_PyGIMIFixer_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>

#include "PyBaseIniFixer.h"
#include "graphGroupEdits/PyIniGraphGroups.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIFixer.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixContext.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IniFixContext` -- the still-pure-Python ``IniFile``
 a fixer is fixing :raw-html:`<br />` :raw-html:`<br />`

 The fixing counterpart of ``PyIniParseContext``. Every method here forwards through genuine
 `Python`_ attribute lookup rather than reimplementing what ``IniFile`` does -- which matters
 beyond faithfulness for the two that touch the filesystem: this project's test harness patches
 ``builtins.open`` and ``os.path`` at the `Python`_ level, so a ``std::filesystem`` call here would
 silently bypass every one of those mocks
 @endrst
 */
class PyIniFixContext: public AGRC::IniFixContext<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::IniFixContext<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using GraphGroups = Base::GraphGroups;

        /**
         * @brief Constructs a context over one Python ``IniFile``
         *
         * @param ini The Python ``IniFile``, or ``None``
         */
        explicit PyIniFixContext(py::object ini = py::none());

        /**
         * @brief The Python ``IniFile``, or ``None``
         */
        py::object ini;

        bool hasIni() const override;
        std::vector<std::string> modsToFix() const override;
        std::optional<std::string> fixedFilePath(std::size_t groupInd) const override;
        bool fixedFileExists() const override;
        std::string fileTxt() const override;
        void setFileTxt(std::string txt) override;
        void hideOriginalSections() override;
        void disableIni() override;
        void log(const std::string &message) override;
        std::string addFixBoilerPlate(const std::string &fix) const override;
        void writeFixedFile(const std::string &path, const std::string &content) override;
        void setIsFixed(bool isFixed) override;
        std::unique_ptr<GraphGroups> makeGraphGroups() override;

        /**
         * @brief The Python ``ModType`` the .ini file was classified as, or ``None``
         */
        py::object modType() const;
};


/**
 * @brief The core :cpp:class:`AGRemapCore::GIMIFixer` specialization this binds
 */
using PyGIMIFixerCore = AGRC::GIMIFixer<py::object, py::object, PyObjectHash, PyObjectEqual, PyBaseIniFixer>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``GIMIFixer`` :raw-html:`<br />` :raw-html:`<br />`

 Keeps the caller's own `Python`_ objects for ``graphGroupEdits``/``modsToFix``/``prevFixer`` and
 re-derives the core members from them at the start of every operation (see #refresh) -- the same
 identity/in-place-mutation contract every other ported class here keeps, and a hard requirement:
 this fixer's own test suite constructs it and then assigns ``graphGroupEdits`` afterwards

 :raw-html:`<br />`

 .. note::
    #getFix overrides the core's own and ignores the :cpp:type:`ParseData` it is handed, sourcing
    the graphs from ``self._parser`` instead. That is not a shortcut: the `Python`_ ``fix``
    signature has no parse-data parameter (see :cpp:func:`PyBaseIniFixer::fixToPy`), so a
    `Python`_ fixer has nowhere else to get them from -- and its parser already knows how to
    collect them, in exactly the shape the core would have been handed
 @endrst
 */
class PyGIMIFixer: public PyGIMIFixerCore {
    public:
        using Core = PyGIMIFixerCore;
        using ModObj = Core::ModObj;

        /**
         * @brief Constructs a new fixer
         *
         * @param parser The Python ``GIMIParser`` to retrieve data for the fix
         * @param graphGroupEdits The edits to apply to the parsed graphs, or ``None``
         * @param modsToFix The mods to fix to, or ``None`` to ask the .ini file
         * @param prevFixer A fixer whose already-edited groups this one continues from, or ``None``
         */
        PyGIMIFixer(py::object parser, py::object graphGroupEdits, py::object modsToFix, py::object prevFixer);

        /**
         * @brief The .ini file being fixed
         */
        PyIniFixContext ctxImpl;

        /**
         * @brief The exact Python object given for ``graphGroupEdits``
         */
        py::object graphGroupEditsObj;

        /**
         * @brief The exact Python object given for ``modsToFix``
         */
        py::object modsToFixObj;

        /**
         * @brief The exact Python object given for ``prevFixer``
         */
        py::object prevFixerObj;

        /**
         * @brief Re-derives every core member from the Python objects above -- see this class's own note
         */
        void refresh();

        /**
         * @brief The ``List[IniGraphGroup]`` this fixer's groups live in -- empty until a fix has run
         */
        py::object graphGroupsToPy() const;

        /**
         * @brief Replaces this fixer's groups with 'groups'
         *
         * @param groups A ``List[IniGraphGroup]``
         */
        void setGraphGroupsFromPy(py::object groups);

        /**
         * @brief #getFix, with the result converted to what the pure-Python original returned
         *
         * @param onlyEditObjGraphs Whether to stop after editing -- returns ``None`` in that case
         *
         * @return A ``Dict[Union[str, int], IniGraphGroup]``, or ``None``
         */
        py::object getFixToPy(bool onlyEditObjGraphs);

        FixTargets getFix(ParseData &parseData, bool onlyEditObjGraphs) override;
        void applyGraphGroupEdits(const std::string &modName) override;
        py::object fixToPy(bool keepBackup, bool fixOnly, bool hideOrig) override;
};


void initCppGIMIFixer(pybind11::module_ &m);

#endif
