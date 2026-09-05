#ifndef AGRemapCore_RemapStats_H
#define AGRemapCore_RemapStats_H

#include <string>

#include "AGRemapCore/model/stats/CachedFileStats.h"
#include "AGRemapCore/model/stats/FileStats.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The file stats for the overall remap process :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``RemapStats`` class (``model/stats/RemapStats.py``)
     @endrst
     */
    class RemapStats {
        public:

            /**
             * @brief
             @rst
             Stats about whether some ``Blend.buf`` files got fixed/skipped/removed -- a removed
             ``Blend.buf`` file refers to a ``RemapBlend.buf`` file previously made by this software
             on a prior run
             @endrst
             */
            FileStats blend;

            /**
             * @brief
             @rst
             Stats about whether some ``Position.buf`` files got fixed/skipped/removed -- a removed
             ``Position.buf`` file refers to a ``RemapPosition.buf`` file previously made by this
             software on a prior run
             @endrst
             */
            FileStats position;

            /**
             * @brief
             @rst
             Stats about whether some ``Texcoord.buf`` files got fixed/skipped/removed -- a removed
             ``Texcoord.buf`` file refers to a ``RemapTexcoord.buf`` file previously made by this
             software on a prior run
             @endrst
             */
            FileStats texcoord;

            /**
             * @brief
             @rst
             Stats about whether some ``.buf`` files that are *not* one of the three specific kinds
             above got fixed/skipped/removed :raw-html:`<br />` :raw-html:`<br />`

             The catch-all for the ``.buf`` half of :cpp:func:`RemapIniRemover::classifyResource` -- a
             ``.buf`` resource inside a fix whose `section`_ name names no particular element
             @endrst
             */
            FileStats buf;

            /**
             * @brief
             @rst
             Stats about whether some file of no recognized kind at all got fixed/skipped/removed
             :raw-html:`<br />` :raw-html:`<br />`

             The final catch-all of :cpp:func:`RemapIniRemover::classifyResource` -- neither a download,
             nor a ``.dds``, nor a ``.buf``
             @endrst
             */
            FileStats other;

            /**
             * @brief
             @rst
             Stats about whether some .ini files got fixed/skipped/undone :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                A skipped .ini file may or may not have been previously fixed -- a path appearing
                here does **not** imply the .ini file previously had a fix
             @endrst
             */
            FileStats ini;

            /**
             * @brief Stats about whether an existing texture file has been edited/removed
             */
            FileStats texEdit;

            /**
             * @brief Stats about whether a brand new texture file created by this software has been created/removed
             */
            FileStats texAdd;

            /**
             * @brief Stats about whether some downloaded mod files have been recently downloaded/removed
             */
            CachedFileStats download;

            /**
             * @brief Clears all the stats for the remap process
             */
            void clear();

            /**
             * @brief
             @rst
             The bucket some kind of resource is tracked in, by name :raw-html:`<br />`
             :raw-html:`<br />`

             The names are :cpp:class:`RemapIniRemover::ResourceType`'s members, which are spelled
             to match this class's own member names precisely so this lookup can exist -- it is the
             C++ counterpart to the pure-Python original's ``getattr(self.stats, resType)``, which
             a caller sorting removed resources into their buckets relied on :raw-html:`<br />`
             :raw-html:`<br />`

             ``"ini"`` resolves to #ini as well, even though a ``.ini`` file is never a resource
             *inside* itself -- a caller iterating every bucket by name should not have to special
             case it
             @endrst
             *
             * @param resourceType The name for the kind of resource
             *
             * @return The bucket, or ``nullptr`` when nothing is tracked under that name
             */
            FileStats* get(const std::string& resourceType);
    };
}

#endif
