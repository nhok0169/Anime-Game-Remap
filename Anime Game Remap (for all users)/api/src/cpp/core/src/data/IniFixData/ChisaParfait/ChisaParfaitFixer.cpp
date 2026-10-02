#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitFixer.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitTextures.h"

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

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitThumbprints.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"


namespace AGRemapCore {
    namespace {
        // ---- the hair's ps-t5, repacked into CHISA's layout --------------------------------------
        //
        // The inverse of ChisaFixer's hairNormalFilter, and not a copy of it: the two characters
        // pack this map DIFFERENTLY, so the constants are different and had to be measured on
        // Chisa's own art rather than carried across.
        //
        // Measured INSIDE the UV islands (`islandPacking.py`; over the whole atlas the modal is just
        // the colour of the empty background, which is the "a statistic is only as good as its mask"
        // trap that sent the material-mask repack the wrong way round for four in-game rounds):
        //
        //                     Chisa e921181d (target)            ChisaParfait d547f3c6 (mod side)
        //   background        0 in every channel                 R 0, G 75, B ~0, A 255
        //   R                 sparse, 83.8% zero, mean 11.1      EMPTY, 98.2% zero, mean 0.0
        //   G                 signal, 211 distinct, p50 74       signal, 169 distinct, p50 105
        //   B                 signal, p50 24, p95 202            sparse, 79.1% zero, p95 251
        //   A                 0 at 99.4%                         255 at 93.7%
        //
        // So:
        //   * G is the same quantity on both -- the strand term, and both are centred on ~75, which
        //     is also the value the skin's atlas uses as its neutral background. The mod's own G is
        //     kept, at the mod's own UVs, exactly as the forward direction keeps it.
        //   * A is a hard constant on both sides and they are OPPOSITE. This is the one correction
        //     that is not a judgement call: feeding Chisa's shader 255 where its own art holds 0
        //     over 99.4% of the islands is the largest possible wrong value.
        //   * R the mod does not have (98.2% zero), so writing Chisa's 0 there loses nothing.
        //   * B is the one channel with NO measured counterpart: Chisa's is broad (p50 24, p95 202)
        //     and the mod's is sparse highlights (79.1% zero). Neither keeping it nor moving it is
        //     supported by anything measured, so it takes Chisa's own modal 0 -- "no contribution",
        //     which her art also holds over 17.4% of its islands.
        //
        // If the hair reads flat or over-dark in game, the next thing to try is the UNTESTED
        // hypothesis this deliberately did not act on: that the skin's B is Chisa's R, both being
        // sparse strand highlights in different channels. It is named here rather than silently
        // chosen, because two atlases of different characters cannot be correlated per texel and
        // appearance alone is how the mask repack went wrong.
        //
        // Leaving the register to the game is NOT an option, in either direction: it is a 2048x2048
        // UV-MAPPED map, the geometry drawn through the slot is the mod's, and the other character's
        // art lands at UVs it was never authored for -- reported twice on the forward direction as
        // magenta blotches on the strands.
        TexEditor::Filter hairNormalFilter() {
            return [](TextureFile& tex) {
                tex.setGamma(std::nullopt);          // these bytes are data, not colour
                std::vector<std::uint8_t> px = tex.getPixels();
                for (std::size_t i = 0; i + 3 < px.size(); i += 4) {
                    px[i] = 0;                       // R: Chisa's is 0 over 83.8% of her islands
                    px[i + 2] = 0;                   // B: no counterpart -- her modal, see above
                    px[i + 3] = 0;                   // A: hers is 0 over 99.4%, the skin's is 255
                }

                tex.setPixels(std::move(px), tex.getWidth(), tex.getHeight());
            };
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::chisa2_8() {
        WWMIFixerConfig config{};
        config.targetId = ModTypeId::Chisa;
        config.version = "2.8";                // the TARGET's, Chisa's
        config.sourceVersion = "3.5";          // the SKIN's own


        // ---- the passes CHISA draws each slot on, off her max-LOD frame dump ----------------------
        // Only the draw that SETS the whole register set is named here; the ones that inherit go in
        // extraPassRegs, because a carried-forward binding table reads as though every draw set
        // every register and mirroring one of those mirrors another component's leftover state.
        //
        // Slot 0 has two: 86560277 sets her bangs' set, and b455d737 -- the HAIR shader -- draws the
        // bangs as well as the hair, each with its own textures.
        config.slotPasses = {
            {"13a86b87d383a40f", "5eb19d847b81ed33"},   // 0: front hair / bangs
            // 3.7 SPLIT the shader that used to draw both: old b455d737821784cd was slot 0's second
            // pass AND slot 1's only one, and the two are different shaders now. Slot 1's is the one
            // slot 0 draws FIRST with -- see the dumps' pass tables, which agree on the role layout
            // (ps-t0 mask, ps-t1 diffuse, ps-t2 ramp, ps-t5 normal) before and after.
            {"13a86b87d383a40f"},                       // 1: hair
            {"ed1c0f8b2ba08ac4"},                       // 2: face
            {"c9cdf1b99fb01750"},                       // 3: upper body
            {"3bbc20374cc3d229"},                       // 4: lower body
            {"0fbe7ebba08cd1b0"},                       // 5: her ribbon and props (nothing maps here)
            // TWO passes, and the one on screen is the SECOND. `275e4ce82ebf0976` sets the eye
            // slot's whole register set and `e04f4df80ee6b0ab` sets only ps-t1 and inherits the
            // rest -- and it is the second that renders. Naming only the first gated the eye's
            // texture list on a `vs` the visible draw never has, so the fix bound the eyes for a
            // draw nobody sees and a mod's eye toggle did nothing (2026-10-01). Proved with a flat
            // magenta: 0 magenta pixels gated, 686 ungated, 631 gated with this pass added.
            {"275e4ce82ebf0976", "e04f4df80ee6b0ab"},   // 6: eyes
        };

        // ---- every pass gated through its VERTEX shaders -----------------------------------------
        // NEVER the pixel shader: RabbitFX marks every pixel shader it patches filter_index 1718.1,
        // a shader holds ONE filter index, and a [ShaderOverride] is keyed by hash across every
        // loaded .ini -- so tagging a pixel shader switches RabbitFX off for every mod drawn with it,
        // globally. Read off her own dump's draw table, which pairs each ps with its vs.
        config.passVertexShaders = {
            {"13a86b87d383a40f", {"3e7bb648e306c671"}},
            {"5eb19d847b81ed33", {"3e7bb648e306c671"}},
            {"ed1c0f8b2ba08ac4", {"7c0b4db32cee62d3"}},
            {"c9cdf1b99fb01750", {"b3c7ad652f7a1c40"}},
            {"3bbc20374cc3d229", {"9cf0b7666af23697"}},
            {"0fbe7ebba08cd1b0", {"641c11c9ee112caf"}},
            {"275e4ce82ebf0976", {"729d10a88623b937"}},
            {"e04f4df80ee6b0ab", {"5fd6e5bb6ff81c53"}},
            {"94d9d5e981938d52", {"5102d7edd774359e"}},
            {"f8c96a270bf847dd", {"6a6650a9db8983ce", "e4a3da6d1d1068b9"}},
            {"32414b557630d98d", {"255061ec51f15e29"}},
            {"320a753b019eff67", {"aef4fc536fbff1e7"}},
            {"259b766b59f72419", {"fd12d3374ac7a7dd"}},
            {"50f2ed8061f3d351", {"49d13f65f2b7838a", "84e073e202127beb", "91256be56071db94"}},
            {"92ca4bd985fe6887", {"676fdbd61b302294"}},
        };

        // ---- the filter_index of every shader this direction tags --------------------------------
        // ALL of them explicit, so nothing is derived from filterBase and nothing drifts when a
        // config gains a pass. Two reasons a value is not free to choose:
        //
        //   * a [ShaderOverride] is keyed by shader hash across every loaded .ini, and a shader holds
        //     ONE filter index. SEVEN of Chisa's vertex shaders are also ChisaParfait's, so
        //     `Chisa -> ChisaParfait` already tags them -- and if the two disagreed, whichever file
        //     3dmigoto loaded last would win and the other mod's `if vs == ...` would never match,
        //     its textures silently unbound. Those seven keep the forward direction's values, read
        //     off a forward-fixed mod's own .ini rather than re-derived from its loop.
        //   * the rest take values from 3381.76 up, which no other pair reaches: the forward
        //     direction occupies 3381.710..3381.732 and the Sanhua pair 3381.81-.84 and .91-.96.
        //     `Tools/Misc/Diagnostics/wwmiShaderTags.py` is the check -- it reads the fixed .ini
        //     files of every installed mod and reports any shader carrying two values.
        config.filterIndices = {
            // shared with Chisa -> ChisaParfait; these values are ITS
            {"3e7bb648e306c671", "3381.71"},    // her bangs and hair
            {"641c11c9ee112caf", "3381.715"},   // her ribbon / prop slot
            {"729d10a88623b937", "3381.717"},   // her eyes
            {"6a6650a9db8983ce", "3381.722"},   // the shared outline-ish pass, slots 0/1/4
            {"e4a3da6d1d1068b9", "3381.723"},   //   ...and slot 3
            {"255061ec51f15e29", "3381.724"},
            {"fd12d3374ac7a7dd", "3381.73"},
            // this direction's own
            {"7c0b4db32cee62d3", "3381.76"},    // her face
            {"b3c7ad652f7a1c40", "3381.761"},   // her upper body
            {"9cf0b7666af23697", "3381.762"},   // her lower body
            {"5102d7edd774359e", "3381.763"},
            {"aef4fc536fbff1e7", "3381.764"},
            {"49d13f65f2b7838a", "3381.765"},
            {"84e073e202127beb", "3381.766"},
            {"91256be56071db94", "3381.767"},
            {"676fdbd61b302294", "3381.768"},
        };

        // ---- source component -> target slot, and the registers CHISA's pass reads ---------------
        // The slots were paired by GEOMETRY, which is the honest measurement when the two are skins
        // of ONE character standing in the same rest pose (`wwmiComponentGeometry.py`): her 0, 2 and
        // 6 are literally the same mesh as Chisa's (box IoU 1.00, 100% vertex overlap), and 1 / 3 / 4
        // are the same region with her own geometry. The shader families then only had to confirm it.
        //
        // TWO MERGES, because she has eight slots and Chisa has seven:
        //   * her 5 (the frilled panel, z 72-123, centroid 5.35 from Chisa's upper body against 18.3
        //     from Chisa's ribbon) goes through Chisa's UPPER BODY. Not through Chisa's slot 5: that
        //     slot's draw sets only ps-t0 and inherits the rest from the HAIR pass, and routing cloth
        //     through a hair shader is the colour-grade defect that cost the forward direction four
        //     rounds. Her panel is cloth -- 87825a9a sets a mask, a normal and a diffuse of its own.
        //   * her 7 (a right-hip prop, x 6.6..31.7, 140 vertices, 71.4% of them near Chisa's lower
        //     body) goes through Chisa's LOWER BODY.
        // Two sources on one target slot is the merge: the extra sections land in their own .ini
        // file, which is also the one-remapped-section-per-draw shape the Sanhua rounds settled on.
        //
        // CHISA'S BODY LAYOUT IS normal / mask / diffuse AT t0 / t1 / t2, where ChisaParfait's is
        // normal / mask / DETAIL / diffuse at t0 / t1 / t2 / t3 -- so the diffuse moves DOWN a
        // register in this direction, and the R8_UNORM detail map she carries has no input on
        // Chisa's shader and is simply not bound.
        //
        // THE SHEEN IS DELIBERATELY NOT BOUND, AND THAT IS NOW A MEASURED RESULT RATHER THAN A
        // GUESS (2026-09-28). Chisa's `bb73967a` (her upper ps-t3 / lower ps-t8) is 512x512 and all
        // four of its channels are the SAME matcap disc at four sharpnesses -- R sharpest through A
        // blurriest, which is what "four grayscale sheen profiles blended by the normal map's alpha"
        // looks like when you render the channels. The skin's two are holographic FOIL in RGB with a
        // matcap in ALPHA. Cross-correlating every channel against every channel (`crossChan.py` --
        // both are matcaps, so they are indexed the same way whatever mesh they sit on, which is the
        // one case where a per-texel correlation between two characters' textures means anything):
        //
        //   * `74a761f5` (her lower sheen): its ALPHA is Chisa's own matcap, r = +0.984 against
        //     Chisa's R and falling monotonically to +0.757 against her A -- the skin's alpha IS the
        //     sharp profile. Binding it would hand Chisa her own sharpest profile and throw away the
        //     three blurrier ones, which is strictly worse than what the game already gives her.
        //   * `4bee4070` (her upper / body sheen): its alpha is a DIFFERENT matcap, r = +0.109
        //     against Chisa's sharpest -- a hard foil ring. Binding that is the "pearly white shirt"
        //     bug the forward direction exists to fix, pointing the other way.
        //   * every foil RGB channel correlates |r| <= 0.21 with every Chisa channel. It is colour,
        //     Chisa's shader has no colour input for it, and it is unrepresentable either way.
        //
        // And a matcap is indexed by view-space normal rather than by UV, so leaving the register to
        // the game is SAFE -- the one condition under which leaving a register alone is safe at all.
        // So there is no inverse of `sheenFilter` to write: the translation would lose Chisa's
        // roughness range to buy a mod-side quantity her shader cannot read.
        //
        // THE HAIR ps-t5 IS BOUND AND REPACKED -- see hairNormalFilter above for the measurement.
        // It is UV-mapped, so unlike the sheen it may not be left to the game, and unlike the
        // shared roles it may not be bound raw either.
        //
        // `frontHairNormal` (`9ccd7ea7`) needs neither: the two download folders hold it
        // byte-identically (md5 f4cc2bac...), as they do `frontHairMask` (`d3b9ba76`). The same
        // asset on both characters is the one case that is free.
        config.plan = {
            {0, {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}}},
            // ps-t5 must NOT be left to the game -- it is UV-mapped; see hairNormalFilter.
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"},
                     {"ps-t5", "hairNormal"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}}}},
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}}}},
            {5, {3, {{"ps-t0", "panelNormal"}, {"ps-t1", "panelMask"}, {"ps-t2", "panelDiffuse"}}}},
            // ps-t4 as well: the eye draw sets the 512 greyscale structure map at ps-t1 and the
            // 2048 COLOURED iris at ps-t4, and only the first had a role until 2026-10-01
            {6, {6, {{"ps-t1", "irisDiffuse"}, {"ps-t4", "eyeDiffuse"}}}},
            {7, {4, {{"ps-t0", "propNormal"}, {"ps-t1", "propMask"}, {"ps-t2", "propDiffuse"}}}},
        };

        // ---- ChisaParfait's textures by the hash the game binds them under -----------------------
        // Every one read off her own frame dump's draw table, from the draw that SETS the register,
        // and cross-checked against the file present in Data/Mod Downloads/WuWa/ChisaParfait/3_5.
        // HashData carries no texture rows for her, so this table is the only place they live.
        //
        // Her components 0, 2 and 6 bind the SAME hashes Chisa does (`d3b9ba76`, `f2646d21`,
        // `9ccd7ea7`, `226b31fc`): the same mesh and the same art, the skin keeping her bangs, face
        // and eyes. Her FACE is the exception -- the same mesh, different textures.

        // ---- a file named by component and type and by nothing else ------------------------------
        config.typeRoles = {
            {0, {{"diffuse", "frontHairDiffuse"}, {"mask", "frontHairMask"}, {"normal", "frontHairNormal"}}},
            {1, {{"diffuse", "hairDiffuse"}, {"mask", "hairMask"}, {"normal", "hairNormal"}}},
            {2, {{"diffuse", "faceDiffuse"}, {"mask", "faceMask"}}},
            {3, {{"diffuse", "upperDiffuse"}, {"mask", "upperMask"}, {"normal", "upperNormal"}}},
            {4, {{"diffuse", "lowerDiffuse"}, {"mask", "lowerMask"}, {"normal", "lowerNormal"}}},
            {5, {{"diffuse", "panelDiffuse"}, {"mask", "panelMask"}, {"normal", "panelNormal"}}},
            {6, {{"diffuse", "irisDiffuse"}}},
            {7, {{"diffuse", "propDiffuse"}, {"mask", "propMask"}, {"normal", "propNormal"}}},
        };

        // ---- and the register her OWN sections bind each role at ----------------------------------
        // How a role finds the mod's file when no hash names it. Both kinds of key: the game's `ps-t`
        // registers off her own max-LOD draws, and RabbitFX's three resource lines -- a RabbitFX mod
        // sets those and binds no `ps-t` at all. RabbitFX's "Lightmap" is what this fix calls the
        // MASK.
        //
        // Her slot 5 is the clothing layout with ps-t0 and ps-t2 EXCHANGED (detail at t0, normal at
        // t2), measured from the pixels rather than assumed from its neighbours.
        config.sourceTextures = chisaParfaitTextureFacts();


        // A mask marks REGIONS, so a mod shipping a constant one has given the fix nothing to place:
        // the body's take her own, whose regions land right because the mod's UVs ARE hers. The two
        // hair masks go in `flatLeftToGame` instead, above -- an earlier version of this comment
        // left them out of BOTH lists on the grounds that ChisaParfait's own hair mask is a
        // structured map, which is the wrong artifact: both the flat test and the alias check run on
        // the MOD's candidate file, not on the download.
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "faceMask", "panelMask", "propMask"};


        // ---- RabbitFX's own texture binding, which would override the fix's ----------------------
        // Her mods do NOT carry the three `Resource*Override = ref ...Component<N>` lines the
        // forward direction strips: those belong to a source past 256 bones, and her merged skeleton
        // is 264 slots with the bones her model weights stopping at 250, so WWMI writes her no blend
        // remap. In THIS direction the fix writes those lines itself -- see mergedSkeletonSlots.
        config.removedRegs = {
            {"Resource\\RabbitFX\\Diffuse", ""},
            {"Resource\\RabbitFX\\Lightmap", ""},
            {"Resource\\RabbitFX\\Normalmap", ""},
            {"Resource\\RabbitFX\\Materialmap", ""},
            {"Resource\\RabbitFX\\Cutoutmap", ""},
            {"Resource\\RabbitFX\\Specialmap", ""},
            {"run", "commandlist\\rabbitfx\\settextures"},
        };

        // ---- CHISA'S MERGED SKELETON IS 420 SLOTS, WHICH AN 8-BIT BLEND INDEX CANNOT NAME --------
        // 768 floats is 256 bones x 3 float4, which is what ChisaParfait's own mods declare and what
        // her 264-slot skeleton needs. Chisa needs 512 bones' worth. The fix supplies its OWN
        // skeleton buffers at this size rather than editing the mod's declaration, so the mod on its
        // own character is untouched.
        //
        // Measured per component after the vertex group row is applied (`boneSpace.py`): her
        // components 3, 4, 5 and 7 reach Chisa bones 418 / 340 / 368 / 335, with 44 / 43 / 29 / 3
        // bones past 255 -- and 80 / 46 / 40 / 5 distinct bones each, far under the 256 one blend
        // remap can hold.
        config.mergedSkeletonSlots = 1536;

        // ---- a remap-only texcoord copy ----------------------------------------------------------
        // The same two reasons as the forward direction, both of which are properties of a MOD rather
        // than of a direction: a second UV that is NaN where the other skin's shader reads it, and a
        // part UV'd into the next tile relying on the sampler wrapping.
        config.cleanTexcoords = true;

        // ---- the passes that take a slot's art at a DIFFERENT register ---------------------------
        // `slotPasses` above names the pass that SETS each slot's whole register set. Chisa draws
        // every slot more than once, and a pass named in NEITHER table still draws -- with the
        // GAME's textures. The 2026-09-28 audit found this table missing entirely while the comment
        // on `slotPasses` promised it.
        //
        // Measured off her max-LOD dump with `wwmiPassLayout.py --against <the skin's dump>`, which
        // separates a register the draw SETS from one it INHERITED -- the distinction that matters,
        // and the one a draw table cannot make. Her extra passes fall into three kinds:
        //
        //   * `21176cf68a65ab7a` SETS `ps-t0` alone, to that slot's own DIFFUSE, on slots 0, 1, 3
        //     and 4. That is the whole gap: the main pass takes the diffuse at ps-t1 (hair) or
        //     ps-t2 (clothing), so a fix that binds only the main layout leaves this pass drawing
        //     the mod's mesh with CHISA's diffuse. These are the four rows below.
        //   * `320a753b019eff67` (slot 2) sets ps-t1 and `92ca4bd985fe6887` (slot 6) sets ps-t0 and
        //     ps-t1 -- but to a flat grey and a green ramp that are in NO role of either character.
        //     They are outline / shadow passes reading a lookup, so binding the mod's art there
        //     would be wrong, not missing. No rows.
        //   * `94d9d5e981938d52`, `32414b557630d98d`, `259b766b59f72419`, `50f2ed8061f3d351` and
        //     `f1ba4fec4b21dd8b` SET NOTHING -- every register inherited. Whatever the preceding
        //     draw left stands, which for a fixed mod is the fix's own binding. No rows.
        //
        // Chisa's slot 5 takes no source in this direction, so it needs none either.
        //
        // The two MERGED slots carry `srcComponent`: slot 3 is her upper body plus the skin's
        // frilled panel, slot 4 her lower body plus the hip prop, and "the diffuse" is a different
        // file for each. Without it both bindings land in one list and the second wins for the
        // source it does not belong to.
        config.extraPassRegs = {
            {0, {{"f8c96a270bf847dd", {{"ps-t0", "frontHairDiffuse"}}}}},
            {1, {{"f8c96a270bf847dd", {{"ps-t0", "hairDiffuse"}}}}},
            {3, {{"f8c96a270bf847dd", {{"ps-t0", "upperDiffuse", 3},
                                      {"ps-t0", "panelDiffuse", 5}}}}},
            {4, {{"f8c96a270bf847dd", {{"ps-t0", "lowerDiffuse", 4},
                                      {"ps-t0", "propDiffuse", 7}}}}},
        };

        // ---- the shape keys are RETARGETED, not hidden -------------------------------------------
        // The same two lines the forward direction sets, for the same reason, and the 2026-09-28
        // audit found them missing here: the template's DEFAULTS hide the two shape-key overrides
        // and bind every remapped draw a zero `vb6` stream, and hiding comments sections out of the
        // MOD'S OWN TEXT -- so a ChisaParfait mod that really uses its keys is broken on
        // ChisaParfait as well as on Chisa, which is the worst class of bug in this repo.
        //
        // Retargeting instead: the overrides keep firing, and the shared asset remap rewrites their
        // hashes and the shape-key checksum onto the target's, so WWMI's own pipeline fills `vb6`
        // and no zero stream is wanted. That is also what `sourceVersion` above is for -- a
        // versionless reverse lookup resolves the checksum the two characters SHARE (2610) to the
        // wrong one and writes `ChecksumNotFound`, and the field is pointless while the keys are
        // hidden.
        config.hiddenObjs = {};

        // ---- but the ZERO STREAM is still wanted, and that is a different thing (2026-10-02) ----
        // The two were switched off together and only the hiding deserved it. `hiddenObjs` edits the
        // MOD'S OWN text, which is why it stays empty; `zeroShapeKeyStream` is an addition to the
        // REMAPPED sections only, so a ChisaParfait mod on ChisaParfait never sees it.
        //
        // And the stream has to be bound, because Chisa's draws READ it whatever the fix does: `vb6`
        // is the game's live shape-key offsets, stride 24, addressed by vertex id and sized for HER
        // draw. Measured against a frame dump of Chisa herself, of the vertices the body draw's
        // range reaches, **4213 index past the end of her 38964-entry buffer** (470 more on another
        // slot) -- undefined reads, which is what the planes across the scene were -- and ~960 more
        // take a displacement computed for one of her vertices. Everything else measured correct the
        // whole time: all 69411 vertices skin to 113-153 units from the origin offline, where no vb6
        // exists, which is why four earlier rounds found nothing.
        //
        // Retargeting does not cover it. It makes WWMI's own shape-key pipeline run against the
        // target's checksum; it does not rebind the game's `vb6`, which is the buffer the draw reads.
        // The cost is that a mod's own shape keys do not play on the target, which is the same
        // trade Sanhua makes and is not a plane across the scene.
        config.zeroShapeKeyStream = true;

        // ---- a flat mask is LEFT TO THE GAME on the hair, as in the forward direction ------------
        // `flatFallsBackToSource` below is about a mask whose REGIONS are missing; this is about the
        // hair, where the right answer is neither the mod's nor the source's. Chisa's own hair mask
        // is a flat (255, 0, 126, 0) -- "all of this is skin" -- so a flat one is not a neutral one
        // and the register is better left alone. The earlier comment justified omitting this by the
        // DOWNLOAD's structure, which is the wrong artifact: both this rule and the alias check run
        // on the MOD's own candidate file, and a ChisaParfait mod is free to ship a flat hair mask
        // or to alias RabbitFX's Lightmap and Normalmap onto one resource (Chisa13 does exactly
        // that, on both hair slots).
        config.flatLeftToGame = {"hairMask", "frontHairMask"};

        // ---- the one texture edit ----------------------------------------------------------------
        // One, where the forward direction needs five, and each of the missing four is absent for a
        // measured reason rather than for want of looking:
        //   * the material mask -- the skin's own art is what a mod of HER ships, and Chisa's shader
        //     reads the same band legend (both mark bare skin with R = 255; the "inverse packing"
        //     reading was wrong and cost four in-game rounds). Nothing to repack.
        //   * the sheen -- not bound at all, see the plan's note and its correlation table.
        //   * the accessory colour grade -- that is Chisa's ribbon drawn through the skin's CLOTH
        //     shader, a problem this direction does not have: the skin's ribbon is her component 5,
        //     which lands on Chisa's slot 3 (cloth to cloth).
        //   * the detail map -- her R8_UNORM ps-t2 has no input on Chisa's shader and is dropped,
        //     which needs no edit, only the absence of a plan row.
        config.texEdits = {
            {"hairNormal", "Repack", [](const WWMIFixerConfig::TexEditContext&) {
                 return hairNormalFilter();
             }},
        };

        config.sourceLabels = {
            {0, "front hair"},
            {1, "hair"},
            {2, "face"},
            {3, "upper body"},
            {4, "lower body"},
            {5, "frilled panel"},
            {6, "eyes"},
            {7, "right-hip prop"},
        };
        config.targetLabels = {
            {0, "front hair"},
            {1, "hair"},
            {2, "face"},
            {3, "upper body"},
            {4, "lower body"},
            {5, "her ribbon and props (nothing maps onto it)"},
            {6, "eyes"},
        };

        config.copyPreamble = IniComments::GIMIObjMergerPreamble;
        return makeWWMIFixer(std::move(config));
    }


    IniFixBuilder::Factory ChisaParfaitFixer::chisa2_8() {
        return IniFixBuilderFuncs::chisa2_8();
    }
}
