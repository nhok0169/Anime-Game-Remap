#ifndef AGRemapCore_GIMIApiNormalizer_H
#define AGRemapCore_GIMIApiNormalizer_H

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


#include <cstddef>
#include <memory>
#include <string>

#include "tsl/ordered_map.h"

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Rewrites GIMI's newer texture API into the traditional one, in parsed `sections`_
     :raw-html:`<br />` :raw-html:`<br />`

     GIMI ships helper command lists a mod can bind its textures through instead of ``ps-t``
     registers -- ``Resource\GIMI\NormalMap`` / ``Diffuse`` / ``LightMap = ref <resource>``,
     then ``run = CommandList\GIMI\SetTextures`` -- which is ORFix's own reference-and-fix
     read from those names rather than from ``ps-t0`` / ``ps-t1`` / ``ps-t2``
     (``Core/GIMI/Libraries/ORFixAPI.ini``). Few mods use it, and nothing in this library read
     it: a CitlaliWhisperofStars mod written that way had every merged slot drawn with the
     GAME's textures (2026-09-22). Rather than teach every reader and every fixer a second
     spelling, the fixes speak the traditional API only, and a mod's sections are brought into it
     here, as they are read :raw-html:`<br />` :raw-html:`<br />`

     Per `section`_ that calls ``SetTextures`` itself, in the traditional layout its keys describe
     -- with a normal map set anywhere in the `section`_, ORFix's:

     .. code-block:: ini

        Resource\GIMI\NormalMap = ref X      ->   ps-t0 = X
        Resource\GIMI\Diffuse = ref X        ->   ps-t1 = X
        Resource\GIMI\LightMap = ref X       ->   ps-t2 = X
        run = CommandList\GIMI\SetTextures   ->   run = CommandList\global\ORFix\ORFix

     and without one, NNFix's -- ``SetTextures`` leaves the normal map null, which is NNFix's
     reading, and a plain-shader slot is written that way:

     .. code-block:: ini

        Resource\GIMI\Diffuse = ref X        ->   ps-t0 = X
        Resource\GIMI\LightMap = ref X       ->   ps-t1 = X
        run = CommandList\GIMI\SetTextures   ->   run = CommandList\global\ORFix\NNFix

     Keys and the call compare ignoring case, as 3DMigoto does; the ``ref`` is dropped because a
     texture register is bound by reference anyway. A `section`_ that sets those names WITHOUT
     calling ``SetTextures`` is left alone: nothing reads them there, and converting them would
     bind files the mod never bound. ``Resource\GIMI\NatlanGlow`` / ``NatlanTattoo`` have no
     traditional register and are left as they are :raw-html:`<br />` :raw-html:`<br />`

     Only the PARSED `sections`_ change -- the ``.ini`` file's own text is written back untouched
     by a fix, so an undo restores exactly what the author wrote. Built from
     :cpp:class:`RegRemap` and :cpp:class:`RegNewVals`
     @endrst
     */
    class GIMIApiNormalizer {
        public:
            /**
             * @brief The parsed `sections`_ of one ``.ini`` file, by name
             */
            using Sections = tsl::ordered_map<std::string, std::unique_ptr<IfTemplate<std::string, std::string>>>;

            /**
             * @brief Normalizes every `section`_ that calls ``SetTextures``
             *
             * @param sections The parsed `sections`_ to rewrite, in place
             * @return How many `sections`_ were rewritten
             */
            static std::size_t normalize(Sections& sections);

            /**
             * @brief Normalizes one `section`_, if it calls ``SetTextures``
             *
             * @param section The `section`_ to rewrite, in place
             * @param sectionName Its name
             * @return Whether it was rewritten
             */
            static bool normalize(IfTemplate<std::string, std::string>& section, const std::string& sectionName);
    };
}

#endif
