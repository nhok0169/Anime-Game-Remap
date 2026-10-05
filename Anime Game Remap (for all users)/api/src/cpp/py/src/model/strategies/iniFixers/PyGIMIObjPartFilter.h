#ifndef AGRemapPyBind_PyGIMIObjPartFilter_H
#define AGRemapPyBind_PyGIMIObjPartFilter_H

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

#include <string>

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/strategies/iniFixers/GIMIObjPartFilter.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::GIMIObjPartFilter`\\<std::string, std::string\\> -- holds
 the `Python`_ objects for the two asset tables it borrows :raw-html:`<br />` :raw-html:`<br />`

 Same ownership story as `PyRegAssetRemap`: the inherited C++ members are borrowed pointers into
 tables a ``ModType`` owns through a ``shared_ptr``, so the `Python`_ objects are held here to stop
 this filter outliving them
 @endrst
 */
class PyGIMIObjPartFilter: public AGRC::GIMIObjPartFilter<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::GIMIObjPartFilter<std::string, std::string>;

        /**
         * @brief The asset table type -- what a ``ModType``'s ``hashes``/``indices`` is
         */
        using Assets = AGRC::ModMappedAssets<std::string, std::string>;

        /**
         * @brief The Python ``hashes`` table, held so this filter cannot outlive it
         */
        py::object hashesObj;

        /**
         * @brief The Python ``indices`` table, held for the same reason
         */
        py::object indicesObj;

        /**
         * @brief Constructs a new part filter
         *
         * @param hashes The Python ``hash`` asset table, or ``None``
         * @param indices The Python ``match_first_index`` asset table, or ``None``
         * @param indexHashKeys The types of hash whose mod objects need a ``match_first_index``
         * @param version The version of the .ini file, or ``None`` for "the latest"
         */
        PyGIMIObjPartFilter(py::object hashes, py::object indices, const py::object &indexHashKeys,
                             const py::object &version);
};


void initCppGIMIObjPartFilter(pybind11::module_ &m);

#endif
