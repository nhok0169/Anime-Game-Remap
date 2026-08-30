#ifndef AGRemapCore_HashData_H
#define AGRemapCore_HashData_H

#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {
    namespace Data {

        /**
         * @brief
         @rst
         The hash data table backing :cpp:class:`Hashes` -- the C++-side replacement for the pure-Python
         ``HashData`` dict (``model/data/HashData.py``, now removed; see git history for the
         pre-migration version) :raw-html:`<br />` :raw-html:`<br />`

         Rows are ``({version, name, type}, hash)`` -- the exact ``(indexVals, value)`` shape
         :cpp:func:`convertRows` already expects, so building a :cpp:class:`PyModDictAssets`/
         :cpp:class:`PyHashes` from this needs no extra conversion step beyond
         ``std::string`` -> ``py::str`` :raw-html:`<br />` :raw-html:`<br />`

         .. danger::
            Mechanically generated from the real, live pure-Python ``HashData`` dict (never
            hand-transcribed -- a script imported the actual module and walked it), then verified
            row-for-row identical via a standalone round-trip check before being committed. Future
            hash updates edit :cpp:func:`getHashDataRows`'s literal directly (see
            ``HashData.cpp``) -- there is no other copy of this data anywhere in the codebase, and
            no regeneration step to keep in sync
         @endrst
         */
        const std::vector<std::pair<std::vector<std::string>, std::string>>& getHashDataRows();

    }
}

#endif
