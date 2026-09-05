#ifndef AGRemapPyBind_PyVertexCounts_H
#define AGRemapPyBind_PyVertexCounts_H

#include <pybind11/pybind11.h>

#include "AGRemapCore/model/assets/VertexCounts.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_ bound ``VertexCounts`` -- one *specific*, pre-populated
 :cpp:class:`AGRC::ModDictAssets` (see :cpp:class:`AGRC::VertexCounts`), the same "the data is baked
 in, a default construction is already useful" shape as :cpp:class:`PyHashes`/:cpp:class:`PyIndices`
 :raw-html:`<br />` :raw-html:`<br />`

 .. note::
    Unlike those two, this one is **not** registered as a subclass of a bound
    :cpp:class:`ModDictAssets`. It can't be: the pybind-registered ``ModDictAssets`` is the
    ``<std::string, std::string>`` instantiation, and this table's value type is ``int`` -- a
    different C++ type entirely, not a base of it. So its handful of methods are bound directly
    here rather than inherited, and Python sees a standalone class

 .. note::
    Replaces the pure-Python ``VertexCounts`` (``model/assets/VertexCounts.py``, deleted) --
    both the class and its data (``data/VertexCountData.py`` is still live, but only as the
    generation source for :cpp:func:`AGRC::Data::getVertexCountDataRows`; see that header's
    ``@danger`` note about the two having deliberately diverged)
 @endrst
 */
void initCppVertexCounts(pybind11::module_ &m);

#endif
