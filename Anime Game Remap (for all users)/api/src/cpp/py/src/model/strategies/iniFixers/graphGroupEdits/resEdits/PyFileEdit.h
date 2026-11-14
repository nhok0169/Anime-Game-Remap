#ifndef AGRemapPyBind_PyFileEdit_H
#define AGRemapPyBind_PyFileEdit_H

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

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>

#include "PyResEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileAddEdit.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/FileDownloadEdit.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing :cpp:class:`AGRemapCore::FileAddCreate` -- the core class does all of the work
 (its `section`_ and its :cpp:class:`AGRemapCore::RemapFileAddResource`), and the mixin only gives
 `Python`_ the shared resource-edit surface
 @endrst
 */
class PyFileAddCreate: public PyResEditMixin<AGRC::FileAddCreate<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new file-adding resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param files The files to make, as ``section name -> file``
         * @param resType The name of the type of resource
         */
        PyFileAddCreate(py::object resModObj, std::unordered_map<std::string, File> files, std::string resType);

        // Not copyable: the core class owns the sections it builds, and pybind11 would otherwise
        // instantiate a copy for it
        PyFileAddCreate(const PyFileAddCreate&) = delete;
        PyFileAddCreate& operator=(const PyFileAddCreate&) = delete;

        py::object pySelf() const override;
};


/**
 * @brief
 @rst
 The `pybind11`_-facing :cpp:class:`AGRemapCore::FileRefAdd` -- the core class does all of the work
 @endrst
 */
class PyFileRefAdd: public PyResEditMixin<AGRC::FileRefAdd<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new file-adding resource edit for files a register names directly
         *
         * @param files The files to make, as ``file -> contents``
         * @param resType The name of the type of resource
         */
        PyFileRefAdd(std::unordered_map<std::string, std::vector<std::uint8_t>> files, std::string resType);

        // Not copyable, like every other resource edit binding
        PyFileRefAdd(const PyFileRefAdd&) = delete;
        PyFileRefAdd& operator=(const PyFileRefAdd&) = delete;

        py::object pySelf() const override;
};


/**
 * @brief
 @rst
 The `pybind11`_-facing :cpp:class:`AGRemapCore::FileDownloadCreate` -- the core class does all of the
 work (its `section`_, its :cpp:class:`AGRemapCore::RemapIniDownload` and the optional edit of the
 download)
 @endrst
 */
class PyFileDownloadCreate: public PyResEditMixin<AGRC::FileDownloadCreate<std::string, std::string>> {
    public:
        /**
         * @brief Constructs a new downloading resource edit
         *
         * @param resModObj The Python tuple id of the mod object holding the resource's graph
         * @param files The files to download, as ``section name -> file``
         * @param resType The name of the type of resource
         */
        PyFileDownloadCreate(py::object resModObj, std::unordered_map<std::string, File> files, std::string resType);

        // Not copyable: the core class owns the sections it builds, and pybind11 would otherwise
        // instantiate a copy for it
        PyFileDownloadCreate(const PyFileDownloadCreate&) = delete;
        PyFileDownloadCreate& operator=(const PyFileDownloadCreate&) = delete;

        py::object pySelf() const override;
};


void initCppFileEdits(pybind11::module_ &m);

#endif
