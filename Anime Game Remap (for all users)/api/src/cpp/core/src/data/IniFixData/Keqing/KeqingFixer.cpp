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

#include "AGRemapCore/data/IniFixData/Keqing/KeqingFixer.h"

#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {
    namespace {
        // Fully opaque. The pure-Python row spells this out twice -- once for the dress diffuse and
        // once for the head diffuse -- as two classmethods with identical bodies.
        const int Opaque = 255;


        /**
         * Why the diffuse is forced opaque on the way over: Keqing's own head and dress diffuses
         * carry an alpha channel her shader reads as a mask. KeqingOpulent's does not, so a
         * partially transparent texel that was a masked-out region on Keqing becomes a see-through
         * hole on the skin.
         */
        void makeOpaque(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, Opaque);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::keqing6_1() {
        // Remapped onto KeqingOpulent -- and this is the direction that MERGES, the exact mirror of
        // keqingOpulent6_1's split.
        //
        // Keqing draws head, body and dress. KeqingOpulent draws head and body only: her Lantern
        // Rite outfit is one mesh, and there is no third object for Keqing's dress to land on. So
        // the dress and the head both become the target's head, which is a merge -- and a merge is
        // the one thing here that produces MORE THAN ONE .ini file, since two sources cannot share
        // one section name.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // ORDER IS LOAD-BEARING, and this is the one line to get right. The first claimant of a
        // target keeps the mod's OWN .ini file and later ones go to generated copies, so listing
        // 'dress' first is what puts the dress in the file a reader opens and the head in
        // <name>RemapFix1.ini -- which is the way round the pure-Python row has it
        // ({"head": ["dress", "head"]}, its list read left to right).
        //
        // 'body' maps to itself and is listed anyway: objSplits is all-or-nothing, and an object
        // left out of it is dropped from the remap entirely. It claims only the first group, so the
        // generated copy carries no body at all -- exactly as the old script's own output does.
        config.objSplits = {{"dress", {"head"}}, {"head", {"head"}}, {"body", {"body"}}};

        // The generated second file explains itself -- see IniComments::GIMIObjMergerPreamble.
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ONE edit, not the pure-Python row's two. It names the TARGET's object, and after the
        // merge both of the source's graphs are sitting under the target's 'head' -- so an entry
        // per source object is not expressible, and would not buy anything if it were: the two
        // classmethods it would name have byte-identical bodies.
        //
        // The collectors are built per group, so this fires once in the mod's own file (over the
        // graph that arrived as the dress) and once in the generated copy (over the head).
        config.texEdits = {{"head", "ps-t0", "OpaqueDiffuse", &makeOpaque}};

        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries that
        // switch it on.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KeqingFixer::v6_1() {
        return IniFixBuilderFuncs::keqing6_1();
    }
}
