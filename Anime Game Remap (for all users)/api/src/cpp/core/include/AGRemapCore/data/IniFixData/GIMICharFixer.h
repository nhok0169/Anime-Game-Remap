#ifndef AGRemapCore_GIMICharFixer_H
#define AGRemapCore_GIMICharFixer_H

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

#include "AGRemapCore/model/strategies/iniFixers/IniFixBuilder.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What one character's fix does differently, for the **standard GIMI character shape** --
     everything :cpp:func:`makeGIMICharFixer` needs that is not the same for every character
     @endrst
     */
    struct GIMICharFixerConfig {
        /**
         * @brief
         @rst
         The objects that actually draw, lowercase -- **the SOURCE's**, not the target's. **Must
         match the paired :cpp:member:`GIMICharParserConfig::drawnObjs` exactly** -- the parser is
         what decides these names exist at all :raw-html:`<br />` :raw-html:`<br />`

         Where the target draws a different set, see \ref objSplits
         @endrst
         */
        std::vector<std::string> drawnObjs;

        /**
         * @brief
         @rst
         Which of the target's drawn objects each of the source's becomes -- **empty (the default)
         means one-to-one**, every object keeping its own name :raw-html:`<br />`
         :raw-html:`<br />`

         Two characters of the same "shape" need not draw the same objects. Jean draws
         ``head``/``body``; JeanSea draws ``head``/``body``/``dress``, its cape being geometry Jean
         simply does not have. Remapping between them is not a rename, it is a **split** (one source
         graph emitted once per target object, each carrying that object's own hash and
         ``match_first_index``) or a **merge** (several source graphs landing on one target object)

         .. code-block::

            split                            merge
            =====                            =====
            Jean          JeanSea            JeanSea        Jean
            head  ------>  head               head   ----->  head
            body  --+--->  body               body   --+-->  body
                    +--->  dress              dress  --+

         Written as ``{{"head", {"head"}}, {"body", {"body", "dress"}}}`` and
         ``{{"head", {"head"}}, {"body", {"body"}}, {"dress", {"body"}}}`` respectively. A source
         object left out of the list is dropped from the remap entirely :raw-html:`<br />`
         :raw-html:`<br />`

         .. note::
            **A merge produces MORE THAN ONE ``.ini`` file.** Two sources naming one target collide,
            and :cpp:func:`GraphGroupRemap::remapGraphs` puts the loser in an additional
            :cpp:class:`IniGraphGroup` for the same ``.ini`` file, which the fixer writes out as
            ``<name>RemapFix1.ini``. That is deliberate and is how the pure-Python
            ``GIMIObjMergeFixer`` worked: the game loads both and overlaps them. Set
            :cpp:member:`GIMIFixer::copyPreamble` so the extra file says what it is

         .. warning::
            **Order matters.** These are applied in sequence, and the FIRST target to claim a
            ``(component, object)`` key keeps the main group. List the objects in the order the
            target draws them
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::string>>> objSplits;

        /**
         * @brief
         @rst
         Registers **renamed** on one target object's parts -- ``{{"head", {{"ps-t1", {"ps-t0"}},
         {"ps-t2", {"ps-t1"}}}}}`` :raw-html:`<br />` :raw-html:`<br />`

         Each entry is one old register and the register(s) it becomes. Naming **two** targets
         duplicates the value into both, which is how a fix synthesises a slot the source does not
         have :raw-html:`<br />` :raw-html:`<br />`

         What it is for: GI 3.x gave characters a **normal map**, and GI 6.x took it away again.
         Remapping across that boundary is a shift -- a character with no normal map has its diffuse
         on ``ps-t0`` and its lightmap on ``ps-t1``, one with a normal map has them on ``ps-t1`` and
         ``ps-t2`` with the normal map on ``ps-t0`` :raw-html:`<br />` :raw-html:`<br />`

         All of an object's renames are applied in **one pass**, so a shift and a swap are both
         expressible and neither re-reads its own output -- see
         :cpp:func:`IfContentPart::remapKeys`
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>> objRegRemaps;

        /**
         * @brief
         @rst
         Which external library calls one target object re-issues, in order -- ``{{"head",
         {IniKeywords::ORFixPath, IniKeywords::TexFxTransparency1}}}`` :raw-html:`<br />`
         :raw-html:`<br />`

         An object not named here re-issues :cpp:member:`IniKeywords::NNFixPath`, which is the
         common case :raw-html:`<br />` :raw-html:`<br />`

         **Every one of these follows the same placement rule** -- immediately before each
         ``drawindexed``, and once at the end of any path that draws nothing (see
         :cpp:class:`RegDelimitedAdd`). They differ only in what they do:

         =========  ====================================================================
         NNFix      the general case
         ORFix      for a character carrying a **normal map** -- see \ref objRegRemaps
         TexFx      the texture-capability addon, whose call names which register the
                    diffuse ended up on
         =========  ====================================================================

         The mod's own calls to all three are stripped first, so this is a re-issue rather than an
         addition
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::string>>> objFixCalls;

        /**
         * @brief
         @rst
         Registers stripped from one **target** object's parts entirely :raw-html:`<br />`
         :raw-html:`<br />`

         Written as ``{{"head", {"ps-t3"}}, {"body", {"ps-t3"}}}``. Every occurrence of the register
         goes, whatever its value :raw-html:`<br />` :raw-html:`<br />`

         What it is for: the target simply does not read that slot. Ningguang binds a ``ps-t3`` that
         NingguangOrchid has no use for, and leaving it bound is how a remapped character ends up
         sampling a texture the shader was not expecting :raw-html:`<br />` :raw-html:`<br />`

         Applied **before** every other register edit on that object, so a later edit never has to
         reason about a register that is on its way out
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::string>>> objRegRemovals;

        /**
         * @brief
         @rst
         Registers forced onto one **target** object's parts, after the split :raw-html:`<br />`
         :raw-html:`<br />`

         Written as ``{{"dress", {{"ib", "null"}}}}`` -- which is exactly what Jean -> JeanSea needs,
         the split's second copy having inherited an ``ib`` that is not its own :raw-html:`<br />`
         :raw-html:`<br />`

         Existing values are replaced; a part with no such register does **not** grow one
         @endrst
         */
        std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> objNewRegVals;

        /**
         * @brief One texture edit -- a texture the fix rewrites and repoints a register at
         */
        struct TexEdit {
            /**
             * @brief The **target** object whose graph holds the register
             */
            std::string obj;

            /**
             * @brief The register the texture hangs off, eg. ``"ps-t1"``
             */
            std::string reg;

            /**
             * @brief
             @rst
             What the edited texture is called in the ``.ini`` file it is written into, eg.
             ``"ShadeLightMap"``
             @endrst
             */
            std::string name;

            /**
             * @brief What the edit does to the texture
             */
            TexEditor::Filter filter;

            /**
             * @brief
             @rst
             Whether the edited texture is written back compressed -- see
             :cpp:func:`TexEditor::getCompress` for the speed/size trade. **Default**: ``true``
             @endrst
             */
            bool compress = true;
        };

        /**
         * @brief
         @rst
         Textures this fix rewrites, repointing the register at the rewritten copy -- the direct
         equivalent of the pure-Python parser's ``texEdits`` plus the fixer's ``RegTexEdit``, which
         are one thing here :raw-html:`<br />` :raw-html:`<br />`

         **Default**: empty -- most characters need none
         @endrst
         */
        std::vector<TexEdit> texEdits;

        /**
         * @brief One texture the fix CREATES, and the register it binds the new file to
         */
        struct TexAdd {
            /**
             * @brief The **target** object whose graph holds the register
             */
            std::string obj;

            /**
             * @brief The register the created texture is bound to, eg. ``"ps-t0"``
             */
            std::string reg;

            /**
             * @brief
             @rst
             The name the texture is filed under, eg. ``"NormalMap"`` --- it becomes the middle
             of both the ``[Resource...]`` `section`_ name and the ``.dds`` file name
             @endrst
             */
            std::string name;

            /**
             * @brief What produces the texture --- size and fill colour
             */
            TexCreator texCreator;
        };

        /**
         * @brief
         @rst
         The textures this fix creates from scratch --- **empty by default**, which is the
         common case :raw-html:`<br />` :raw-html:`<br />`

         What it is for: remapping ACROSS the normal-map boundary in the direction that **gains**
         one. GI 3.x gave characters a normal map on ``ps-t0``; a source that predates that has
         nothing to put there, so the fix invents a flat one. Ganyu -> GanyuTwilight is the
         worked example, and it is the exact mirror of GanyuTwilight -> Ganyu, which instead
         **drops** ``ps-t0`` through \ref objRegRemovals :raw-html:`<br />` :raw-html:`<br />`

         Pair it with \ref objRegRemaps. The shift has to duplicate the diffuse into the slot it
         is moving to *and* leave it where it was, so this has something to overwrite:
         ``{{"head", {{"ps-t0", {"ps-t0", "ps-t1"}}, {"ps-t1", {"ps-t2"}}}}}``

         .. note::
            Ordering is handled for you: the collectors run before the register edits, so a
            texture is declared against the register it hangs off **before** the shift, not
            after. See ``buildTexEdits``
         @endrst
         */
        std::vector<TexAdd> texAdds;

        /**
         * @brief
         @rst
         What every ``.ini`` file this fix GENERATES opens with -- see
         :cpp:member:`GIMIFixer::copyPreamble` :raw-html:`<br />` :raw-html:`<br />`

         Only a **merge** generates one: a file the user never asked for and did not write should say
         why it exists. :cpp:member:`IniComments::GIMIObjMergerPreamble` is the paragraph written for
         that, and is what the pure-Python merge used :raw-html:`<br />` :raw-html:`<br />`

         **Default**: empty -- right for every shape that writes only the mod's own file
         @endrst
         */
        std::string copyPreamble;

        /**
         * @brief
         @rst
         Whether the fix takes the shared ``drawindexed`` off ``("", "ib")`` and re-issues one per
         drawn object :raw-html:`<br />` :raw-html:`<br />`

         **Per character, and not guessable from the ``.ini`` file's shape** -- Amber and Mona both
         ship a ``[TextureOverride<Char>IB]`` carrying ``handling = skip`` and ``drawindexed = auto``,
         yet Amber's fix moves the draw call and Mona's leaves it exactly where it is. Check what
         the pure-Python row for that character does (the ``Ib*`` entries in its
         ``IniFixBuilderData`` row), or read a mod the old script has already fixed
         :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false`` -- the commoner of the two
         @endrst
         */
        bool moveDrawIndexed = false;

        /**
         * @brief
         @rst
         The register the face diffuse USED to hang off, and the first half of the swap the fix
         applies :raw-html:`<br />` :raw-html:`<br />`

         GI 6.x swapped which register the shader reads the face diffuse and the face lightmap out
         of, so the fix swaps \ref faceDiffuseReg and \ref faceLightMapReg back -- see
         :cpp:func:`makeGIMICharFixer` :raw-html:`<br />` :raw-html:`<br />`

         **Read it off the mod's own ``.ini`` file** -- which ``ps-tN`` a character's face sits on is
         not derivable in general, though every character so far uses ``ps-t0``.
         **Default**: ``"ps-t0"``
         @endrst
         */
        std::string faceDiffuseReg = "ps-t0";

        /**
         * @brief
         @rst
         The other half of that swap -- the register the face lightmap used to hang off, and the one
         the diffuse has to move to :raw-html:`<br />` :raw-html:`<br />`

         A mod that binds no lightmap at all is the common case and needs nothing extra: this half
         of the swap simply finds nothing to move. **Default**: ``"ps-t1"``
         @endrst
         */
        std::string faceLightMapReg = "ps-t1";
    };


    /**
     * @brief
     @rst
     Builds the fixer for a character with the **standard GIMI shape**, paired with
     :cpp:func:`makeGIMICharParser` :raw-html:`<br />` :raw-html:`<br />`

     What it does, for a remap onto a genuinely different model (a CN skin, or a skin's base
     character):

     #. remaps every ``hash`` with :cpp:class:`RegAssetRemap`, and every drawn object's
        ``match_first_index`` with a **forward** lookup -- see :cpp:class:`RegAssetRemap`'s own
        warning for why the index cannot use the same tool
     #. rebuilds the ``Blend.buf`` through :cpp:class:`VGRemapBlendReplace`
     #. swaps the face's diffuse and lightmap registers with :cpp:class:`RegRemap` -- the cure for
        the white cheek spots, GI 6.x having swapped which register the shader reads each out of --
        and **hides the original face section**, the one thing it hides, because a CN pair shares
        its ``tex_face_diffuse`` hash and the two would otherwise both bind
     #. drops the mod's own ``ORFix``/``NNFix`` calls and re-issues ``NNFix`` where it belongs
     #. renames each graph with the convention belonging to **its own kind**

     .. note::
        This is **not** the shape for remapping onto a boss that shares the source's geometry --
        that keeps the source's hashes and has to hide the originals instead. Raiden has her own
        fixer for exactly that reason; see `Creating Remaps <../CreatingRemaps/CLAUDE.md>`_'s
        "Two shapes of remap"
     @endrst
     *
     * @param config What this character does differently -- see #GIMICharFixerConfig
     */
    IniFixBuilder::Factory makeGIMICharFixer(GIMICharFixerConfig config);
}

#endif
