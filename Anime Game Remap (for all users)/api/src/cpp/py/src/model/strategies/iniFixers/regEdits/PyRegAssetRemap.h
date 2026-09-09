#ifndef AGRemapPyBind_PyRegAssetRemap_H
#define AGRemapPyBind_PyRegAssetRemap_H

#include <optional>
#include <string>

#include <pybind11/pybind11.h>

#include "PyBaseRegEdit.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegAssetRemap.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing subclass of `AGRC::RegAssetRemap`\\<py::object, py::object\\> -- holds the
 exact `Python`_ ``dict`` given for ``assets``, for the same reason `PyRegNewVals` holds its own
 (see that class's note) :raw-html:`<br />` :raw-html:`<br />`

 That `Python`_ ``dict`` is also what keeps the asset tables **alive**. The inherited C++ member
 holds a borrowed :cpp:class:`AGRemapCore::ModMappedAssets` pointer, and the table it points at is
 normally owned by a ``ModType``'s ``shared_ptr``; holding the `Python`_ object that owns it is what
 stops this edit outliving it
 @endrst
 */
class PyRegAssetRemap: public AGRC::RegAssetRemap<std::string, std::string> {
    public:

        /**
         * @brief The C++ core class this wraps
         */
        using Core = AGRC::RegAssetRemap<std::string, std::string>;

        /**
         * @brief The asset table type an entry names -- what a ``ModType``'s ``hashes`` is
         */
        using Assets = AGRC::ModMappedAssets<std::string, std::string>;

        /**
         * @brief
         @rst
         The exact `Python`_ object given for ``assets`` -- a ``dict`` of register name -> either
         the asset table on its own, or a ``(table, notFoundVal)`` pair
         @endrst
         */
        py::object assetsObj;

        /**
         * @brief Constructs a new asset-remapping register edit
         *
         * @param assetsObj The Python dict of register name -> asset table (optionally paired with a not-found value)
         * @param toModName The name of the mod being fixed to
         * @param fromModName The name of the mod being fixed from -- see the core class's warning on why leaving this empty is a real hazard
         * @param fromVersion The version being fixed from, or ``None``
         * @param toVersion The version being fixed to, or ``None``
         */
        PyRegAssetRemap(py::object assetsObj, std::string toModName, std::string fromModName,
                         const py::object &fromVersion, const py::object &toVersion);

        /**
         * @brief
         @rst
         Re-derives the inherited C++ ``assets`` member from #assetsObj -- called at the start of
         every ``edit``, so an in-place mutation of the `Python`_ ``dict`` is honoured, the same
         contract `PyRegNewVals` has
         @endrst
         *
         * @param modType The modType edit was called with. Unused here -- every asset table comes from #assetsObj
         */
        void refresh(const py::object &modType);
};


void initCppRegAssetRemap(pybind11::module_ &m);

#endif
