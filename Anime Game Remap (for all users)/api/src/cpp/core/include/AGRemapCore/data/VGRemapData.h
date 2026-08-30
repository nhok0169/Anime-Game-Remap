#ifndef AGRemapCore_VGRemapData_H
#define AGRemapCore_VGRemapData_H

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"


namespace AGRemapCore {
    namespace Data {

        /**
         * @brief
         @rst
         The vertex group remap table backing :cpp:class:`VGRemaps` -- the C++-side counterpart to the
         pure-Python ``VGRemapData`` (``data/VGRemapData.py``) :raw-html:`<br />` :raw-html:`<br />`

         Rows are ``({fromVersion, fromChar, fromComp, toVersion, toChar, toComp}, remap)`` -- **six**
         index columns, of which **two** (``fromVersion`` at position 0 and ``toVersion`` at position
         3) are version columns :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            That two-version-column shape is what sets this table apart from every other one in
            ``data/``. The hash, index, vertex-count and builder-args tables all have exactly one
            version column and so are :cpp:class:`ModDictAssets`; this one cannot be, and
            :cpp:class:`VGRemaps` uses :cpp:class:`ModAssets` instead -- see that class

         .. note::
            The value is a whole :cpp:class:`VGRemap` object, not a scalar -- 52 rows carrying 5229
            index pairs between them

         .. note::
            Both ``fromComp`` and ``toComp`` are ``""`` on every row the software currently ships.
            They are real key values, not "missing" markers, exactly as with the component columns in
            :cpp:func:`Data::getIndexDataRows` and :cpp:func:`Data::getVertexCountDataRows`

         .. danger::
            Mechanically generated from the real, live pure-Python data (never hand-transcribed -- a
            script imported the actual module, ran ``vgRemapDataBuilder.build()`` and walked the
            resulting 6-deep dict), then verified row-for-row and pair-for-pair against it. Future
            remap updates edit :cpp:func:`getVGRemapDataRows`'s literal directly (see
            ``VGRemapData.cpp``) :raw-html:`<br />` :raw-html:`<br />`

            The pure-Python source still exists and is still live, so the two must be kept in step by
            hand -- the same caveat :cpp:func:`Data::getVertexCountDataRows` carries
         @endrst
         */
        const std::vector<std::pair<std::vector<std::string>, VGRemap>>& getVGRemapDataRows();

    }
}

#endif
