#ifndef AGRemapCore_GIMICharParser_H
#define AGRemapCore_GIMICharParser_H

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
#include <vector>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseBuilder.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What one character's ``.ini`` file looks like, for the **standard GIMI character shape** --
     everything :cpp:func:`makeGIMICharParser` needs that is not the same for every character
     :raw-html:`<br />` :raw-html:`<br />`

     Deliberately small. If you find yourself wanting to add a field for something only one
     character does, that character probably wants a parser of its own instead -- Raiden has one
     (`RaidenParser`), because a boss remap is a different shape rather than a variation on this one
     @endrst
     */
    struct GIMICharParserConfig {
        /**
         * @brief
         @rst
         Which mod type this parses, used for the :cpp:class:`VertexCountData` lookup the downloaded
         `blend`_ needs
         @endrst
         */
        ModTypeId modTypeId;

        /**
         * @brief
         @rst
         The character's folder under ``Data/Mod Downloads/GI/``, eg. ``"Amber"``
         @endrst
         */
        std::string downloadCharFolder;

        /**
         * @brief The version subfolder inside it, eg. ``"4_0"``
         */
        std::string downloadVersionFolder;

        /**
         * @brief
         @rst
         The prefix every file in that folder carries, eg. ``"Amber"`` :raw-html:`<br />`
         :raw-html:`<br />`

         .. warning::
            **Separate from** \ref downloadCharFolder **on purpose, and not derivable from it.**
            ``Raiden/`` holds ``RaidenShogun``-prefixed files, and one character's version
            subfolders can disagree with each other. Read it off the folder's own contents
         @endrst
         */
        std::string downloadPrefix;

        /**
         * @brief
         @rst
         The objects that actually draw, lowercase and **in the order the game draws them** --
         ``{"head", "body"}``, or ``{"head", "body", "dress", "extra"}`` for a character with more
         :raw-html:`<br />` :raw-html:`<br />`

         They all share one ``ib`` hash and are told apart by the ``match_first_index`` that follows
         it, which is what :cpp:class:`Indices` is keyed by
         @endrst
         */
        std::vector<std::string> drawnObjs;

        /**
         * @brief
         @rst
         The byte size of one texcoord vertex -- **per character**, and worth checking rather than
         copying: Amber and Mona are ``12`` where Rosaria is ``20`` :raw-html:`<br />`
         :raw-html:`<br />`

         .. note::
            A literal because there is no C++ ``TexcoordByteSizeData`` yet, only
            ``api/src/py/FixRaidenBoss2/data/TexcoordByteSizeData.py``, which this layer cannot
            read. Worth porting once enough characters need it
         @endrst
         */
        int texcoordStride;

        /**
         * @brief The byte size of one position vertex. **Default**: ``40``
         */
        int positionStride = 40;

        /**
         * @brief The byte size of one `blend`_ vertex. **Default**: ``32``
         */
        int blendStride = 32;
    };


    /**
     * @brief
     @rst
     Builds the parser for a character with the **standard GIMI shape**: drawn objects sharing one
     ``ib`` and separated by ``match_first_index``, plus `blend`_/position/texcoord/ib named
     outright by their own hashes, plus ``("", "other")`` and ``("", "face")``
     :raw-html:`<br />` :raw-html:`<br />`

     Every mod object it produces:

     ===================  ====================================================================
     Mod object           What it is
     ===================  ====================================================================
     each of 'drawnObjs'  a drawn part -- ``ib`` hash **plus** its own ``match_first_index``
     ``("", "ib")``       the shared draw call -- the ``ib`` hash and **no** index
     ``("", "blend")``    the ``Blend.buf`` the fix remaps
     ``("", "position")``
     ``("", "texcoord")``
     ``("", "other")``    ``draw_vb`` -- VertexLimitRaise, a hash swap and nothing else
     ``("", "face")``     ``tex_face_diffuse`` -- tracked so the fix can swap its REGISTERS
     ===================  ====================================================================

     Pair it with :cpp:func:`makeGIMICharFixer`; the two agree on these names by hand, and neither
     half makes sense alone
     @endrst
     *
     * @param config What this character does differently -- see #GIMICharParserConfig
     */
    IniParseBuilder::Factory makeGIMICharParser(GIMICharParserConfig config);
}

#endif
