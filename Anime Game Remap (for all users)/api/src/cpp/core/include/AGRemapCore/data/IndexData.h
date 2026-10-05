#ifndef AGRemapCore_IndexData_H
#define AGRemapCore_IndexData_H

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
#include <utility>
#include <vector>


namespace AGRemapCore {
    namespace Data {

        /**
         * @brief
         @rst
         The index data table backing :cpp:class:`Indices` -- the C++-side replacement for the
         pure-Python ``IndexData`` dict (``model/data/IndexData.py``, now removed; see git history
         for the pre-migration version) :raw-html:`<br />` :raw-html:`<br />`

         Rows are ``({version, name, component, type}, index)`` -- the exact ``(indexVals, value)``
         shape :cpp:func:`convertRows` already expects, so building a :cpp:class:`PyModDictAssets`/
         :cpp:class:`PyIndices` from this needs no extra conversion step beyond ``std::string`` ->
         ``py::str`` :raw-html:`<br />` :raw-html:`<br />`

         .. danger::
            Mechanically generated from the real, live pure-Python ``IndexData`` dict (never
            hand-transcribed -- a script imported the actual module and walked it), then verified
            row-for-row identical via a standalone round-trip check before being committed. Future
            index updates edit :cpp:func:`getIndexDataRows`'s literal directly (see
            ``IndexData.cpp``) -- there is no other copy of this data anywhere in the codebase, and
            no regeneration step to keep in sync
         @endrst
         */
        const std::vector<std::pair<std::vector<std::string>, std::string>>& getIndexDataRows();

    }
}

#endif
