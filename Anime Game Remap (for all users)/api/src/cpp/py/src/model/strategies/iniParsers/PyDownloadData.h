#ifndef AGRemapPyBind_PyDownloadData_H
#define AGRemapPyBind_PyDownloadData_H

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

#include <memory>
#include <string>
#include <type_traits>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/strategies/iniParsers/IniParseDownloadData.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The core download interface this binds, at the binding's key and value types
 */
using PyIniParseDownloadDataCore = AGRC::IniParseDownloadData<std::string, std::string>;

/**
 * @brief The core download class this binds, at the binding's key and value types
 */
using PyDownloadDataCore = AGRC::DownloadData<std::string, std::string>;


/**
 * @brief
 @rst
 The `pybind11`_ trampoline for :cpp:class:`AGRemapCore::IniParseDownloadData` and
 :cpp:class:`AGRemapCore::DownloadData` -- lets a `Python`_ subclass override any of the interface's
 methods and still be reached through the parser's C++ virtual dispatch :raw-html:`<br />`
 :raw-html:`<br />`

 ``name`` and ``refToSection`` are methods a subclass of the interface overrides, and read-only data on
 :cpp:class:`AGRemapCore::DownloadData`. A part or `section`_ is handed over **by reference**, so what an override adds to it is what the
 parser sees. A `section`_ an override of ``createResSection`` returns is moved out of the `Python`_
 object it came in
 @endrst
 *
 * @tparam Base The class this stands in for
 */
template <typename Base>
class PyDownloadDataTrampoline: public Base, public py::trampoline_self_life_support {
    public:
        using Base::Base;
        using Section = typename Base::Section;
        using ContentPart = typename Base::ContentPart;

        // Data on DownloadData (its constructor sets them, and Python reads them as properties), so only the
        // interface asks a Python subclass for them -- a property looked up as an override would call back
        // into itself
        std::string name() const override {
            if constexpr (std::is_abstract_v<Base>) {
                PYBIND11_OVERRIDE_PURE(std::string, Base, name);
            } else {
                return Base::name();
            }
        }

        bool refToSection() const override {
            if constexpr (std::is_abstract_v<Base>) {
                PYBIND11_OVERRIDE_PURE(bool, Base, refToSection);
            } else {
                return Base::refToSection();
            }
        }

        void addToPart(ContentPart &part, const std::string &key, const std::string &val) override {
            {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(static_cast<const Base*>(this), "addToPart");
                if (override) {
                    override(py::cast(&part, py::return_value_policy::reference), key, val);
                    return;
                }
            }
            if constexpr (std::is_abstract_v<Base>) {
                throw py::type_error(notImplemented("addToPart"));
            } else {
                Base::addToPart(part, key, val);
            }
        }

        void addToSection(Section &section, const std::string &key, const std::string &val) override {
            {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(static_cast<const Base*>(this), "addToSection");
                if (override) {
                    override(py::cast(&section, py::return_value_policy::reference), key, val);
                    return;
                }
            }
            if constexpr (std::is_abstract_v<Base>) {
                throw py::type_error(notImplemented("addToSection"));
            } else {
                Base::addToSection(section, key, val);
            }
        }

        std::unique_ptr<Section> createResSection(const std::string &sectionName) const override {
            {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(static_cast<const Base*>(this), "createResSection");
                if (override) {
                    py::object section = override(sectionName);
                    if (section.is_none()) {
                        return nullptr;
                    }
                    return std::make_unique<Section>(std::move(*section.cast<Section*>()));
                }
            }
            if constexpr (std::is_abstract_v<Base>) {
                throw py::type_error(notImplemented("createResSection"));
            } else {
                return Base::createResSection(sectionName);
            }
        }

        std::unique_ptr<AGRC::IniResource> createFileDownload(const std::string &iniFolder) const override {
            {
                py::gil_scoped_acquire gil;
                py::function override = py::get_override(static_cast<const Base*>(this), "createFileDownload");
                if (override) {
                    py::object download = override(iniFolder);
                    if (download.is_none()) {
                        return nullptr;
                    }
                    return download.cast<std::unique_ptr<AGRC::IniResource>>();
                }
            }
            if constexpr (std::is_abstract_v<Base>) {
                throw py::type_error(notImplemented("createFileDownload"));
            } else {
                return Base::createFileDownload(iniFolder);
            }
        }

    private:
        static std::string notImplemented(const char* method) {
            return std::string("IniParseDownloadData.") + method + " is not implemented";
        }
};


void initCppDownloadData(pybind11::module_ &m);

#endif
