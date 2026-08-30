#ifndef AGRemapCore_ModDataAssets_H
#define AGRemapCore_ModDataAssets_H

#include <memory>

#include "AGRemapCore/model/assets/VGRemaps.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Global, shared asset tables used by the software :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``ModDataAssets`` class (``data/ModDataAssets.py``) -- a
     ``DeferredEnum`` there, lazily building each table the first time it is accessed.
     #vgRemaps below gets the same lazy, build-once-then-reuse behavior from a C++11 function-local
     ``static`` (guaranteed thread-safe, exactly-once initialization), exactly as
     :cpp:class:`GlobalIniClassifiers` and :cpp:class:`GlobalIniRemoveBuilders` do
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Only #vgRemaps is exposed so far, because it is the only one :cpp:class:`ModType` actually
        falls back to. The pure-Python original also holds ``VertexCounts``, ``PositionEditors``,
        ``IniParseBuilderArgs`` and ``IniFixBuilderArgs``; the C++ equivalents of the last two live
        on :cpp:class:`IniParseBuilderData`/:cpp:class:`IniFixBuilderData` instead, and
        :cpp:class:`ModType` builds its own :cpp:class:`VertexCounts` per instance rather than
        sharing one. Add entries here as more of them need to be shared
     @endrst
     */
    class ModDataAssets {
        public:

            ModDataAssets() = delete;

            /**
             * @brief
             @rst
             The shared :cpp:class:`VGRemaps`, lazily constructed on first access and reused for
             every later call -- what :cpp:member:`ModType::vgRemaps` falls back to
             :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                This is genuinely **shared**: :cpp:class:`ModAssets` is mutable, so anything adding
                rows to the table returned here changes it for every :cpp:class:`ModType` that fell
                back to it. That is the pure-Python behaviour too (its ``ModType`` default is
                ``ModDataAssets.VGRemaps.value``, not a fresh ``VGRemaps()``). Construct a
                :cpp:class:`VGRemaps` directly if an independent table is wanted
             @endrst
             *
             * @return The shared vertex group remap table
             */
            static const std::shared_ptr<VGRemaps>& vgRemaps();
    };
}

#endif
