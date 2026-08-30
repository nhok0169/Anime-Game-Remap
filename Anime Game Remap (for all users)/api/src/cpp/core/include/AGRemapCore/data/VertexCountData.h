#ifndef AGRemapCore_VertexCountData_H
#define AGRemapCore_VertexCountData_H

#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {
    namespace Data {

        /**
         * @brief
         @rst
         The vertex count data table backing :cpp:class:`VertexCounts` -- the C++-side counterpart to
         the pure-Python ``VertexCountData`` dict (``data/VertexCountData.py``) :raw-html:`<br />`
         :raw-html:`<br />`

         Rows are ``({version, name, component}, count)`` -- the ``(indexVals, value)`` shape
         :cpp:class:`ModDictAssets` expects, so building the table is a straight
         :cpp:class:`Row` conversion :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            ``component`` sits between the mod name and the count, and is currently ``""`` for
            **every** row -- no vertex count is component-specific yet. The column exists so one can
            be, without reshaping the table or touching :cpp:class:`VertexCounts` again. An empty
            component is a real key value, not a "missing" marker, exactly as in
            :cpp:func:`Data::getIndexDataRows`

         .. note::
            The value type is ``int``, not ``std::string`` -- unlike
            :cpp:func:`Data::getHashDataRows` and :cpp:func:`Data::getIndexDataRows`, whose values
            are hex/index *strings*. A vertex count is a real number and the pure-Python original
            stores it as one, so it stays one here

         .. danger::
            Originally generated mechanically from the real, live pure-Python ``VertexCountData``
            dict (never hand-transcribed -- a script imported the actual module and walked it), then
            verified row-for-row identical against it. Future vertex count updates edit
            :cpp:func:`getVertexCountDataRows`'s literal directly (see ``VertexCountData.cpp``);
            unlike the hash/index tables the pure-Python source dict still exists, so if both are
            ever live at once they must be kept in step by hand :raw-html:`<br />` :raw-html:`<br />`

            **The two have since diverged on purpose: this table has a ``component`` column and
            the pure-Python dict does not.** It is still ``({version, name}, count)`` over there,
            two levels deep. So this table can no longer be regenerated from that dict by a naive
            walk -- doing so would silently drop the component column and shrink every row back to
            two index values. Add the ``""`` component back in if you ever do regenerate, or bring
            the Python dict up to three levels first
         @endrst
         */
        const std::vector<std::pair<std::vector<std::string>, int>>& getVertexCountDataRows();

    }
}

#endif
