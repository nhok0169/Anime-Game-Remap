#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitFixer.h"

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

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/ChisaParfait/ChisaParfaitThumbprints.h"
#include "AGRemapCore/data/IniFixData/WWMIFixer.h"


namespace AGRemapCore {
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
            {"86560277b8531781", "b455d737821784cd"},   // 0: front hair / bangs
            {"b455d737821784cd"},                       // 1: hair
            {"374a4f8fc9a5ea6a"},                       // 2: face
            {"42721e1d0c282918"},                       // 3: upper body
            {"6ea8898edc27d7aa"},                       // 4: lower body
            {"3df800c350681ec9"},                       // 5: her ribbon and props (nothing maps here)
            {"da00ec8f7c73d5e3"},                       // 6: eyes
        };

        // ---- every pass gated through its VERTEX shaders -----------------------------------------
        // NEVER the pixel shader: RabbitFX marks every pixel shader it patches filter_index 1718.1,
        // a shader holds ONE filter index, and a [ShaderOverride] is keyed by hash across every
        // loaded .ini -- so tagging a pixel shader switches RabbitFX off for every mod drawn with it,
        // globally. Read off her own dump's draw table, which pairs each ps with its vs.
        config.passVertexShaders = {
            {"86560277b8531781", {"d83a54772fc666f9"}},
            {"b455d737821784cd", {"d83a54772fc666f9"}},
            {"374a4f8fc9a5ea6a", {"e35973101a6ba4fe"}},
            {"42721e1d0c282918", {"5a674a73c6741bd7"}},
            {"6ea8898edc27d7aa", {"6e4d16a7da96fde0"}},
            {"3df800c350681ec9", {"bbabe18b97a63509"}},
            {"da00ec8f7c73d5e3", {"a6e9eb6303b1b631"}},
            {"94d9d5e981938d52", {"5102d7edd774359e"}},
            {"21176cf68a65ab7a", {"0ccd030bff8b515c", "5d60ebdc89fe3833"}},
            {"32414b557630d98d", {"ba4eee7b53cf726e"}},
            {"320a753b019eff67", {"aef4fc536fbff1e7"}},
            {"259b766b59f72419", {"fd12d3374ac7a7dd"}},
            {"50f2ed8061f3d351", {"503033bc07782274", "84e073e202127beb", "a208489c9b30ad3d"}},
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
            {"d83a54772fc666f9", "3381.71"},    // her bangs and hair
            {"bbabe18b97a63509", "3381.715"},   // her ribbon / prop slot
            {"a6e9eb6303b1b631", "3381.717"},   // her eyes
            {"0ccd030bff8b515c", "3381.722"},   // the shared outline-ish pass, slots 0/1/4
            {"5d60ebdc89fe3833", "3381.723"},   //   ...and slot 3
            {"ba4eee7b53cf726e", "3381.724"},
            {"fd12d3374ac7a7dd", "3381.73"},
            // this direction's own
            {"e35973101a6ba4fe", "3381.76"},    // her face
            {"5a674a73c6741bd7", "3381.761"},   // her upper body
            {"6e4d16a7da96fde0", "3381.762"},   // her lower body
            {"5102d7edd774359e", "3381.763"},
            {"aef4fc536fbff1e7", "3381.764"},
            {"503033bc07782274", "3381.765"},
            {"84e073e202127beb", "3381.766"},
            {"a208489c9b30ad3d", "3381.767"},
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
        // Deliberately NOT bound, each for a measured reason:
        //   * Chisa's sheen matcap (`bb73967a` at her upper ps-t3 / lower ps-t8). ChisaParfait's is a
        //     HOLOGRAPHIC FOIL (`4bee4070`) and Chisa's is four packed grayscale sheen profiles, so
        //     binding the foil raw is the "pearly white shirt" bug in reverse. A matcap is indexed by
        //     view-space normal rather than by UV, so leaving it to the game is safe -- which is the
        //     one condition under which leaving a register alone is safe at all. The inverse of
        //     `sheenFilter` is the texture pass's job.
        //   * Chisa's hair ps-t5 (`e921181d`). It IS UV-mapped, so leaving it to the game is NOT
        //     safe and neither is binding hers raw: the forward direction had to repack that map
        //     channel by channel. The reverse repack is a measurement this config has not made yet,
        //     so the row is absent rather than wrong -- if the hair comes back blotchy, this is the
        //     first line to add.
        config.plan = {
            {0, {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"}}}},
            {1, {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}}}},
            {2, {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"}}}},
            {3, {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t2", "upperDiffuse"}}}},
            {4, {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t2", "lowerDiffuse"}}}},
            {5, {3, {{"ps-t0", "panelNormal"}, {"ps-t1", "panelMask"}, {"ps-t2", "panelDiffuse"}}}},
            {6, {6, {{"ps-t1", "irisDiffuse"}}}},
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
        config.roles = {
            // shared with Chisa outright
            {"d3b9ba76", "frontHairMask"}, {"f2646d21", "frontHairDiffuse"}, {"9ccd7ea7", "frontHairNormal"},
            {"226b31fc", "irisDiffuse"},
            // hers
            {"3f433212", "hairMask"}, {"a94ee44f", "hairDiffuse"}, {"81f48e54", "hairRamp"},
            {"57aa5a71", "hairTipRamp"}, {"d547f3c6", "hairNormal"},
            {"226d9bc4", "faceMask"}, {"53e96488", "faceDiffuse"},
            {"3c4279a9", "upperNormal"}, {"6b7ae743", "upperMask"}, {"4c420ea9", "upperDiffuse"},
            {"b9a888ec", "lowerNormal"}, {"4668fce8", "lowerMask"}, {"1d79fc96", "lowerDiffuse"},
            {"74a761f5", "lowerSheen"},
            {"4bee4070", "bodySheen"},              // the holographic foil -- see the plan's note
            {"2f911db8", "panelMask"}, {"56e725c4", "panelNormal"}, {"1e1b7bbc", "panelDiffuse"},
            {"00e3f13b", "panelMatcap"},
            {"2c990f51", "propMask"}, {"71a6e63f", "propDiffuse"}, {"e4463fca", "propNormal"},
        };

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
        config.sourceRegisterRoles = {
            {0, {{"ps-t0", "frontHairMask"}, {"ps-t1", "frontHairDiffuse"}, {"ps-t5", "frontHairNormal"},
                 {"Resource\\RabbitFX\\Diffuse", "frontHairDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "frontHairNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "frontHairMask"}}},
            {1, {{"ps-t0", "hairMask"}, {"ps-t1", "hairDiffuse"}, {"ps-t2", "hairRamp"}, {"ps-t4", "hairTipRamp"},
                 {"ps-t5", "hairNormal"}, {"Resource\\RabbitFX\\Diffuse", "hairDiffuse"},
                 {"Resource\\RabbitFX\\Normalmap", "hairNormal"}, {"Resource\\RabbitFX\\Lightmap", "hairMask"}}},
            {2, {{"ps-t0", "faceMask"}, {"ps-t1", "faceDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "faceDiffuse"}, {"Resource\\RabbitFX\\Lightmap", "faceMask"}}},
            {3, {{"ps-t0", "upperNormal"}, {"ps-t1", "upperMask"}, {"ps-t3", "upperDiffuse"}, {"ps-t8", "bodySheen"},
                 {"Resource\\RabbitFX\\Diffuse", "upperDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "upperNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "upperMask"}}},
            {4, {{"ps-t0", "lowerNormal"}, {"ps-t1", "lowerMask"}, {"ps-t3", "lowerDiffuse"}, {"ps-t5", "lowerSheen"},
                 {"Resource\\RabbitFX\\Diffuse", "lowerDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "lowerNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "lowerMask"}}},
            {5, {{"ps-t1", "panelMask"}, {"ps-t2", "panelNormal"}, {"ps-t3", "panelDiffuse"},
                 {"Resource\\RabbitFX\\Diffuse", "panelDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "panelNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "panelMask"}}},
            {6, {{"ps-t1", "irisDiffuse"}, {"Resource\\RabbitFX\\Diffuse", "irisDiffuse"}}},
            {7, {{"ps-t0", "propMask"}, {"ps-t2", "propDiffuse"}, {"ps-t4", "propNormal"},
                 {"Resource\\RabbitFX\\Diffuse", "propDiffuse"}, {"Resource\\RabbitFX\\Normalmap", "propNormal"},
                 {"Resource\\RabbitFX\\Lightmap", "propMask"}}},
        };

        // ---- a role the mod ships no file for falls back to HER game texture ----------------------
        // The mod's UVs are ChisaParfait's, so her own texture is the right default and Chisa's is
        // wrong by construction. The roles both skins share (`frontHair*`, `irisDiffuse`) need no
        // entry: the hash is the same on both sides, so the game already binds the right thing.
        // `hairRamp` and `hairTipRamp` are not in her download folder, so they cannot fall back --
        // they are lookups, and the game's own serve them.
        config.downloadCharFolder = "ChisaParfait";
        config.downloadVersionFolder = "3_5";
        config.downloadPrefix = "ChisaParfait";
        config.fallbackTextures = {
            {"hairMask", "3f433212"}, {"hairDiffuse", "a94ee44f"}, {"hairNormal", "d547f3c6"},
            {"faceMask", "226d9bc4"}, {"faceDiffuse", "53e96488"},
            {"upperNormal", "3c4279a9"}, {"upperMask", "6b7ae743"}, {"upperDiffuse", "4c420ea9"},
            {"lowerNormal", "b9a888ec"}, {"lowerMask", "4668fce8"}, {"lowerDiffuse", "1d79fc96"},
            {"panelMask", "2f911db8"}, {"panelNormal", "56e725c4"}, {"panelDiffuse", "1e1b7bbc"},
            {"propMask", "2c990f51"}, {"propNormal", "e4463fca"}, {"propDiffuse", "71a6e63f"},
        };

        // A mask marks REGIONS, so a mod shipping a constant one has given the fix nothing to place:
        // the body's take her own, whose regions land right because the mod's UVs ARE hers. Her hair
        // mask is in neither list -- unlike Chisa's flat (255, 0, 126, 0) it is a structured map, so
        // the flat test cannot fire on it either way.
        config.flatFallsBackToSource = {"upperMask", "lowerMask", "faceMask", "panelMask", "propMask"};

        config.textureThumbprints = chisaParfaitTextureThumbprints();

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
