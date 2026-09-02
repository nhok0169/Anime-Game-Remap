#ifndef AGRemapPyBind_PyRemapIniRemover_H
#define AGRemapPyBind_PyRemapIniRemover_H

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

#include "PyBaseIniRemover.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/RemapIniRemover.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `Python`_-backed :cpp:class:`AGRemapCore::IniRemoveContext` -- calls straight back into a real
 `Python`_ ``IniFile`` object :raw-html:`<br />` :raw-html:`<br />`

 Every method here forwards through genuine `Python`_ attribute lookup rather than reimplementing
 what ``IniFile`` does. That is load-bearing, not stylistic: the unit-test harness patches
 ``builtins.open``/``os.path`` at the `Python`_ level, so a ``std::filesystem`` call here would
 bypass every mock -- the same trap the ``iniresources`` port hit
 @endrst
 */
class PyIniRemoveContext: public AGRC::IniRemoveContext<py::object, py::object, PyObjectHash, PyObjectEqual> {
    public:
        using Base = AGRC::IniRemoveContext<py::object, py::object, PyObjectHash, PyObjectEqual>;
        using Section = Base::Section;
        using Assets = Base::Assets;

        /**
         * @brief Constructs a context over one Python ``IniFile``
         *
         * @param ini The Python ``IniFile``, or ``None``
         */
        explicit PyIniRemoveContext(py::object ini = py::none());

        /**
         * @brief The Python ``IniFile``, or ``None``
         */
        py::object ini;

        bool hasIni() const override;
        std::string iniFolder() const override;
        std::optional<AGRC::Version> version() const override;
        std::vector<Assets*> modTypeHashes() const override;
        std::vector<std::string> readFileLines() override;
        std::unordered_map<std::string, Section*> sectionIfTemplates() const override;
        std::string fileTxt() const override;
        void setFileTxt(std::string txt) override;
        std::string write() override;
        void clearRead() override;
        void setIsFixed(bool isFixed) override;

        /**
         * @brief The Python ``ModType`` the .ini file was classified as, or ``None``
         */
        py::object modType() const;
};


/**
 * @brief The core :cpp:class:`AGRemapCore::RemapIniRemover` specialization this binds
 */
using PyRemapIniRemoverCore = AGRC::RemapIniRemover<py::object, py::object, PyObjectHash, PyObjectEqual, PyBaseIniRemover>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``RemapIniRemover`` :raw-html:`<br />` :raw-html:`<br />`

 Keeps the caller's own `Python`_ ``IniFile`` on
 :cpp:member:`PyBaseIniRemover::iniFileObj` and re-points #ctxImpl at it at the start of every
 :cpp:func:`remove` (see #refresh) -- the same "the `Python`_ attribute is the truth, the core
 members are derived from it" contract ``PyGIMIParser`` keeps, and a real requirement here: the
 pure-Python original is constructed once per ``.ini`` file but its ``iniFile`` attribute is a
 plain, assignable one
 @endrst
 */
class PyRemapIniRemover: public PyRemapIniRemoverCore {
    public:
        using Core = PyRemapIniRemoverCore;

        /**
         * @brief Constructs a new remover
         *
         * @param iniFile The Python ``IniFile`` to remove the fix from
         */
        explicit PyRemapIniRemover(py::object iniFile = py::none());

        /**
         * @brief The .ini file the fix is being removed from, behind its interface
         */
        PyIniRemoveContext ctxImpl;

        /**
         * @brief
         @rst
         Re-points #ctxImpl at whatever :cpp:member:`PyBaseIniRemover::iniFileObj` currently holds
         @endrst
         */
        void refresh();

        std::string remove(bool parse = false, bool writeBack = true,
                           AGRC::IniRemovalContext context = AGRC::IniRemovalContext()) override;

        /**
         * @brief
         @rst
         :cpp:func:`AGRemapCore::RemapIniRemover::getRemovedResources`, as a
         ``Dict[str, List[IniResource]]``
         @endrst
         */
        py::object removedResourcesToPy() const;

        /**
         * @brief The .ini-domain customization points every Python-facing remover uses
         */
        static Core::RemoverConfig makeConfig();
};


void initCppRemapIniRemover(pybind11::module_ &m);

#endif
