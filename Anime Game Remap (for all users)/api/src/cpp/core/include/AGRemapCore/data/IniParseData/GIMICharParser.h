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
         * @brief
         @rst
         Drawn objects that have **no lightmap** to download :raw-html:`<br />`
         :raw-html:`<br />`

         Every drawn object otherwise gets a diffuse, a lightmap and an ib declared for it, and
         a declaration whose file is not in ``Data/Mod Downloads/GI/<char>/<version>/`` writes
         a resource section naming a file nothing will ever fetch -- a dangling reference the
         ``.ini`` text cannot show you :raw-html:`<br />` :raw-html:`<br />`

         **Default**: empty. Exactly one object in the whole asset tree needs this
         (CherryHuTao's ``extra`` at 5.3), so check the folder before reaching for it rather
         than assuming the character is like her
         @endrst
         */
        std::vector<std::string> objsWithoutLightMap;

        /**
         * @brief
         @rst
         Which registers ONE object's downloaded textures hang off, when they are not the
         usual ``ps-t0``/``ps-t1`` :raw-html:`<br />` :raw-html:`<br />`

         **Why this is not a constant.** A download has to land on the register the mod's own
         shader would have bound that texture to, and GI moved those: a 4.0-era character
         reads its diffuse from ``ps-t1`` and its lightmap from ``ps-t2``, leaving ``ps-t0``
         for a normal map, while a modern one uses ``ps-t0``/``ps-t1``. Put a download on the
         wrong slot and the shader samples a lightmap as a diffuse :raw-html:`<br />`
         :raw-html:`<br />`

         It varies **per object as well as per version** -- Kirara's head is the 4.0 layout
         while her body and dress are already the modern one -- so this is keyed by object
         and every object not named here keeps the defaults
         @endrst
         */
        struct ObjDownloadRegs {

            /**
             * @brief The mod object these registers are for, eg. ``"head"``
             */
            std::string obj;

            /**
             * @brief The register this object's diffuse hangs off
             */
            std::string diffuseReg = "ps-t0";

            /**
             * @brief The register this object's lightmap hangs off
             */
            std::string lightMapReg = "ps-t1";

            /**
             * @brief
             @rst
             The register this object's NORMAL MAP hangs off, or empty for no normal map
             download at all -- which is the usual case :raw-html:`<br />` :raw-html:`<br />`

             Only the 4.0-era characters that shipped one want this; a modern character's
             normal map is invented by the fix (``GIMICharFixerConfig::texAdds``) rather than
             fetched
             @endrst
             */
            std::string normalMapReg;
        };

        /**
         * @brief Per-object register overrides for downloaded textures -- see \ref ObjDownloadRegs
         */
        std::vector<ObjDownloadRegs> objDownloadRegs;

        /**
         * @brief
         @rst
         The version folder the FACE diffuse download lives in, when it is not the one the rest of
         this character's assets are in -- empty (the usual case) means \ref downloadVersionFolder
         :raw-html:`<br />` :raw-html:`<br />`

         Not a rule, a per-character fact, and a narrow one: of the 44 characters in
         ``Data/Mod Downloads/GI`` exactly THREE file their face diffuse somewhere other than
         alongside the rest -- AyakaSpringbloom, Nilou and LisaStudent, all of whom keep it under
         ``5_4`` while everything else sits in ``4_0``. Get it wrong and the download 404s, which
         is invisible until a mod turns up with no face section of its own for the fix to use
         @endrst
         */
        std::string faceDownloadVersionFolder;

        /**
         * @brief
         @rst
         The file-name prefix of the FACE diffuse download, when it is not \ref downloadPrefix --
         empty (the usual case) means that one :raw-html:`<br />` :raw-html:`<br />`

         Exists because one character's assets disagree with themselves:
         ``AyakaSpringBloomBodyDiffuse.dds`` but ``AyakaSpringbloomFaceDiffuse.dds`` -- capital B
         for everything except the face, which took its spelling from the folder instead. A raw
         GitHub URL is case-sensitive, so this is a 404 and not a near miss
         @endrst
         */
        std::string faceDownloadPrefix;

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
