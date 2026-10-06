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

#include "AGRemapCore/data/IniFixData/Citlali/CitlaliFixer.h"

#include <string>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMIComponentFixer.h"
#include "AGRemapCore/model/iniresources/VGSplitGroupResource.h"
#include "AGRemapCore/model/strategies/texEditors/texFilters/MaterialBandRemapFilter.h"


namespace AGRemapCore {
    namespace {
        // ---- the band legend (measured 2026-09-21, over the texels each object's UVs cover) ----
        //
        //   Citlali head          0 (pale),  255 = her navy HAIR
        //   Citlali body          0 (purple cloth),  78-80 (navy cloth),  255 = her SKIN (96% skin-coloured)
        //   CitlaliWhisperofStars Body A: 0, 78 (pale cloth), 128, 177 = SKIN (92% skin-coloured),
        //                         255 = HAIR (its Bangs, which share A's textures, are 100% alpha 255)
        //
        // ONE band moves: her body's skin from 255 onto the skin's skin ramp at 177, gated on a
        // skin-coloured diffuse and on the BODY object only -- 255 on her head is her hair, which
        // already sits on the skin's hair ramp. Her cloth bands (0, 78-80) have ramps of the same
        // number on slot A.
        const std::vector<MaterialBandRemapFilter::Band> Bands = {
            {255, 177, &MaterialBandRemapFilter::skinColoured},
        };


        GIMIComponentFixerConfig citlaliWhisperofStarsConfig() {
            // Remapped onto CitlaliWhisperofStars -- the third skin of SEVERAL components. Every
            // value was read off the prototype (Tools/Misc/Prototypes/citlaliWhisperofStarsFix.py),
            // confirmed in game on her identity mod and one real mod, and off the two 6.7 frame
            // dumps (Tools/Misc/Diagnostics/giDrawTable.py).
            GIMIComponentFixerConfig config{};
            config.targetSkin = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStars);
            config.drawnObjs = {"head", "body"};

            // What differs in KIND from Bennett: Citlali's own sections are already the normal-map
            // layout her target slots read, so a mod's normal map, diffuse and light map pass through
            // -- no register shift and no invented normal map -- and a mod still on the plain layout
            // (no ps-t2) gets Bennett's shift. Per object, read off the mod's own section.
            config.sourceLayout = GIMIComponentFixerConfig::SourceLayout::Detect;

            // Both characters read the face diffuse at ps-t1 (GI 6.x). Swap only a mod still on ps-t0.
            config.faceSwapOnlyFromDiffuseReg = true;

            // Every component draws through slot A, spelled out as "0" rather than read from
            // IndexData -- see Component::slotIndex. Slot A: the only Body slot with the skin's
            // skin ramp at any size, and its hair ramp is at Citlali's hair's 255. Head and body
            // both land on it, so the Body fixer writes a second .ini file (the merge).
            //
            // All three components read the normal-map layout (shader 2c157719180b096c in the
            // frame dump). Her hair has bones of its own (21-34), so the Bangs is a real target,
            // unlike BennettAdventure's. All three take the graph cut: on her identity mod every
            // triangle is drawn by exactly one component.
            GIMIComponentFixerConfig::Component body{};
            body.name = "Body";
            body.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBody);
            body.slot = "A";
            body.slotIndex = "0";
            body.negativeIndex = false;
            body.normalMap = true;
            body.face = true;
            body.texcoordStride = 20;
            body.slotRegisters = {"ps-t0", "ps-t1", "ps-t2"};

            // ---- the ground plane ----
            //
            // THE TWO MODELS STAND ON DIFFERENT HEIGHTS. A mod's vertices are Citlali's, and the
            // skinning is the identity at rest, so a remapped mod renders at CITLALI's coordinates
            // while the game plants the character by the SKIN's -- and Citlali's sole sits 0.045
            // lower than CitlaliWhisperofStars'. Measured off the two download folders: both models'
            // lowest 400 vertices are the soles, dominated by the matching toe and foot groups
            // (Citlali 109/113/133/137 -> the skin's 25/29/49/53), and each sole is a flat plane --
            // 50 vertices inside 3 mm at y = -0.0629 for Citlali and y = -0.0178 for the skin.
            // Unshifted, the whole remapped model sits 4.5 cm low and her feet sink into the ground
            // in the overworld (reported 2026-10-04).
            //
            // Set on `body` BEFORE the two copies below, so all three components move together --
            // shifting one of them alone shears the model apart at its seams.
            body.positionOffset = {0.0f, 0.0450f, 0.0f};

            // AND IT FADES OUT BY THE FACE, which is the half this did not have on its first run
            // (2026-10-04). This mod carries NO face mesh -- only a face DIFFUSE bound by hash -- so
            // the GAME draws the skin's face, at the skin's fixed height. Lifting every vertex put
            // the feet right and left the face 4.5 cm below the head it belongs to, which the user
            // reported straight back. Full lift at Citlali's sole, none at or above y 1.400, which
            // is just over her eye groups at 1.384: the head stays where the game's face is and the
            // 4.5 cm is taken up through the body, which is where the two models differ anyway (the
            // skin's foot and knee groups sit 2-6 cm higher than hers, her head group 1 cm lower).
            body.positionOffsetFade = {-0.0629f, 1.400f};

            GIMIComponentFixerConfig::Component bangs = body;
            bangs.name = "Bangs";
            bangs.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBangs);
            bangs.face = false;
            bangs.texcoordStride = 12;

            GIMIComponentFixerConfig::Component eyes = bangs;
            eyes.name = "Eyes";
            eyes.modTypeName = ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsEyes);

            // ---- the two long front braids ----
            //
            // THE CHAIN USED TO BE CUT IN HALF ACROSS TWO COMPONENTS, AND THAT WAS THE FIRST BUG.
            // Each braid is a four-link chain of Citlali's (23 -> 24 -> 25 -> 26, mirror
            // 29 -> 30 -> 31 -> 32) hanging from the temple to the chest. Its top two links went to
            // the Bangs' own front-hair chain and its bottom two to a bone of the Body, so one braid
            // was skinned in two index spaces by two unrelated bones. A user reported both halves of
            // that in one sentence: "the braid is stationary sticking to her body instead of flowing
            // naturally like hair", and "dislocated from her hair" -- the bottom half rode a bone
            // that does not move, and a `pushAway` put on the Body displaced only that half, so the
            // braid visibly broke at the seam. It is the same fault as Neuvillette's capes ripping
            // along a component boundary: ONE part belongs on ONE component's bones. VGRemapData.cpp
            // sends all four links to the skin's own front lock, Bangs 3 -> 5 -> 7 (mirror 4 -> 6 -> 8).
            //
            // AND THE BRAID IS NOT DAMPED, WHICH WAS THE SECOND BUG. A `splitGroups` shared each
            // lower link's weight with the Bangs' head bone, on the argument that bone 7's lever is
            // 3.8 cm while Citlali's braid hangs 24 cm below it, so the skin's hair sim would move
            // her tip "six times too far". That arithmetic is right and the conclusion is wrong: a
            // longer lock swinging further at the tip is what hair does. Damped, the braid read as
            // "stationary, solid with her head" -- the user's words -- because 65-80% of its weight
            // was on the head. It carries the hair bones' full weight now and swings with them.
            //
            // WHAT REMAINS IS A GENUINE CLIP, and a push is the right tool for it now that the braid
            // is whole: whatever this moves, it moves all of. The braid hangs down the front of the
            // shoulder and the body is drawn through it, so it goes OUT and FORWARD -- about 4.7 cm
            // sideways and 10.1 forward on her left, 4.1 and 6.3 on her right, measured at the
            // braid's mid-height. `from` sits 12 cm behind it, which is what sets the direction.
            //
            // GOING SIDEWAYS IS WORTH FAR MORE THAN GOING FORWARD HERE, and the reason generalises:
            // a body is much SHALLOWER just outside the bust than in front of it. Measured on this
            // mod's own body at y 1.10-1.14, the front surface is at z 0.124-0.149 while the braid
            // sits inside |x| 0.105, and z 0.015-0.026 outside it. So the first 3 cm of lateral
            // travel carries the braid off the breast entirely and buys ~10 cm of clearance, where
            // forward travel only ever buys its own length. An almost purely forward push (1.5 cm
            // sideways against 9.4) cleared the body by 0.4 cm at best -- a margin the idle could
            // still close, which is exactly what the user kept seeing. Angled out, the same push
            // clears by 3.5-11 cm in every band. WHEN A PART CLIPS A TORSO, ASK WHERE THE SILHOUETTE
            // ENDS BEFORE REACHING FOR MORE DISTANCE: the clearance a displacement buys is a
            // property of the surface it is moving across, not of its size.
            //
            // THAT EDGE IS ALSO WHY THESE NUMBERS ARE A FLOOR, not a preference. The user's last
            // note was that the braids now sat too far out, so both were swept against the body
            // per VERTEX rather than per band. Pulling a braid back inside |x| 0.105 puts it in
            // front of the bust again, where it needs ~12 cm of forward travel to clear -- which
            // reads as detached. Her left had room and came in 1.4 cm; her RIGHT is already at its
            // minimum, and loses its margin at x +0.097, y 1.15 (the bust edge) the moment it comes
            // in further. The two sides end up nearly even laterally, 4.7 against 4.1, which is
            // also what makes them read as a pair. SWEEP THE PARAMETER AGAINST THE MOD'S OWN BODY
            // BEFORE THE NEXT IN-GAME ROUND: it costs seconds, and it is what turns "a bit closer"
            // into a number with a known margin instead of another guess.
            //
            // AND IT IS NOT SYMMETRIC, WHICH TOOK THREE REPORTS TO ACCEPT. Everything that can be
            // measured off the files says the two braids are the same part: the MOD's two braids are
            // mirror-exact (mean z agreeing to 0.0000 in every height band below y 1.30, and the
            // share profiles on 25/26 and 31/32 agree band for band), and the skin's own bones 7 and
            // 8 are a mirror PAIR (centroids -0.059 / +0.063 at the same y 1.279, the same
            // 1.241-1.379 span). So the row is right and the geometry is right -- and her LEFT braid
            // still sank into the bodice about 2 cm higher than her right at the same instant.
            //
            // That difference lives in the skin's standing IDLE, which no file here can see: the
            // animation puts her left shoulder and its hair chain somewhere her right ones are not.
            // A static displacement is the only tool that reaches it, so her left takes the larger
            // distance. Measured by stepping the left side alone and comparing the two braids WITHIN
            // each frame, never by arguing from the buffers.
            //
            // The symmetric value shipped twice before this on the argument that a per-side number
            // would be "tuned to the one animation it was measured in". That is still true and it is
            // still the cost of this: it is the only animation this skin is ever seen in, because
            // the outfit cannot be worn without owning the character, so the shop preview IS the
            // test. Re-measure both numbers if that ever stops being so. The general lesson is the
            // one that cost the rounds: MIRROR-EXACT INPUTS DO NOT GIVE A MIRROR-EXACT PICTURE, and
            // a symmetry argument made entirely from the files cannot outvote what the target's rig
            // does to them. Check the pair in the frame, not on disk.
            //
            // WHICH GROUPS THE PUSH NAMES IS WHAT SETS HOW FAR UP IT REACHES, and naming all four
            // links reached the HAIRLINE. VGPushAway weighs each vertex by its share of the groups
            // it lists, so listing 23 and 29 -- the links at the temple, where the braid's weight
            // blends into the scalp -- gave the fringe a share of 0.39 on average at y 1.36-1.40 and
            // 0.66 at y 1.32-1.36. At 4 cm that was 1.5-2.6 cm and went unnoticed; at 6.5 cm it is
            // 2.5-4.6 cm, and a user reported her BANGS standing forward off her face.
            //
            // So the push names only the LOWER two links of each braid. Their share is 0.00 above
            // y 1.355 (the fringe cannot move at all), 0.05 at y 1.32-1.36, 0.40 at 1.24-1.28 and
            // 1.00 by y 1.10 -- the braid now BENDS forward from where it leaves the head over about
            // 20 cm, instead of the whole lock including its root translating. The clip is at the
            // shoulder, y 1.15-1.25, where the share is 0.80-1.00, so the braid takes nearly the
            // whole displacement there. At the top band the push can reach (y 1.355-1.381, share
            // 0.02-0.05) it is 0.2-0.6 cm, so the fringe stays put at either side's distance.
            //
            // This is the same group list the FIRST push used, on 2026-10-04, when it drew the
            // complaint "the braid is dislocated from her hair" -- and the difference is not the
            // list. Back then the chain was cut across two components, so moving the lower links
            // broke the braid at a hard seam. In ONE component on ONE chain the same list is a
            // smooth bend, because the share tapers across the 24 -> 25 blend instead of stopping
            // dead at a component boundary. A field that was wrong under a broken row can be right
            // once the row is fixed; re-test it rather than ruling it out from memory.
            //
            // WHAT NO PUSH CAN FIX, recorded so the next report is not chased: when she BENDS, both
            // braids pass through her chest. The skin has no hair bone below y 1.24, so the whole
            // braid hangs off a chain parented to the HEAD; bending rotates the torso forward about
            // the hips while the braid stays with the head, and the chest sweeps into it. A static
            // displacement cannot follow a pose -- the offset a bend needs is not the offset
            // standing needs -- and the Bangs component has no body bone to share weight with even
            // if sharing were wanted. On Citlali the same motion is absorbed by her own hair sim.
            // AND THE LOCK'S ROOT IS COVERED BY AN OVERLAP BAND, because the chain and the scalp
            // it grows out of end up in DIFFERENT COMPONENTS. On the mod's mesh the hairline blends
            // smoothly from the scalp (source 91) into the lock (source 21/22/23), but a split has
            // to give each vertex to ONE component, so the blend becomes a hard switch: 104 vertices
            // at y 1.355-1.453 sit at the same point in space in both buffers, bound to Bangs 3/4/5/6/9
            // on one side and to Body 7 -- the skin's HEAD -- on the other. Standing still the two
            // copies coincide exactly and nothing shows; the moment the hair sim moves the chain
            // bones and the head does not, the surface opens. A user saw it as a "slight
            // dislocation/rip" at the temple that is "only noticeable when she moves".
            //
            // `overlapRings` is the field written for this: the band is drawn by BOTH components, so
            // a gap narrower than it is covered by the other side's copy, and ownership does not
            // change. It is NOT a weighting fix -- pinning the lock's root to the head bone would
            // close the same gap by making the root rigid, which is the damping mistake of
            // 2026-10-04 in another costume.
            //
            // The push is NOT what opens this, measured rather than argued: against a zero-push
            // control build, nothing above y 1.36 moves at all, the highest vertex it touches is
            // y 1.3564 and it moves 1 mm, and the Body and Eyes components do not move a vertex.
            bangs.overlapRings = 1;

            bangs.pushAway = {VGPushAway{{25, 26}, {-0.0100f, 1.130f, -0.1183f}, 0.1120f, -1},
                              VGPushAway{{31, 32}, {-0.0179f, 1.130f, -0.1183f}, 0.0760f, 1}};

            config.components = {body, bangs, eyes};

            // Every component receives geometry, so nothing of the skin's is hidden. Its Body slot D
            // (a small plain-shader piece) stops drawing with the rest of its Body, whose ib section
            // the fix leaves skipping.
            config.hiddenComponents = {};

            // Body slots B, C and D -- every Body draw but slot A's. Slot B's outline draws before
            // slot A's, so without these a TexFx request from the mod's slot A draw was served
            // there: the skin's whole ib over the mod's buffers, as a shiny web between the arms
            // and the hair (2026-09-22). Bangs and Eyes are one slot each.
            config.unremappedSlots = {{ModTypeIdTools::getName(ModTypeId::CitlaliWhisperofStarsBody), {"60888", "111096", "122916"}}};
            config.diffuseEdits = {};

            config.lightMapEdit = MaterialBandRemapFilter::lightMapEdit(Bands);
            config.lightMapObjs = {"body"};

            // NOT BC7-compressed: the alpha being edited is a band SELECTOR -- see Bennett's config.
            config.compressTextures = false;

            config.copyPreamble = IniComments::GIMIObjMergerPreamble;

            return config;
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsBody6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Body");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsBangs6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Bangs");
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::citlaliWhisperofStarsEyes6_7() {
        return makeGIMIComponentFixer(citlaliWhisperofStarsConfig(), "Eyes");
    }


    IniFixBuilder::Factory CitlaliFixer::body6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsBody6_7();
    }


    IniFixBuilder::Factory CitlaliFixer::bangs6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsBangs6_7();
    }


    IniFixBuilder::Factory CitlaliFixer::eyes6_7() {
        return IniFixBuilderFuncs::citlaliWhisperofStarsEyes6_7();
    }
}
