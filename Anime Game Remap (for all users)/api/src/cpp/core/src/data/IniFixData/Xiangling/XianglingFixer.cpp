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

#include "AGRemapCore/data/IniFixData/Xiangling/XianglingFixer.h"

#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/constants/IniComments.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BufFile.h"
#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"
#include "AGRemapCore/model/textures/Colour.h"


namespace AGRemapCore {
    namespace {
        // The flat normal map a character with none of her own is given -- the BLUE one here, not
        // Ganyu's yellow. Which of the two a character wants is not derivable; it is whatever her
        // shader was built to read, and the pure-Python row is the only record of it.
        const int NormalMapSize = 1024;
        const Colour NormalMapBlue(128, 128, 255);

        // 1, not 0. Alpha 0 is fully transparent and 1 is very nearly so -- the difference matters
        // to a shader that treats 0 as "no mask at all".
        const int NearlyTransparent = 1;


        void darkenDiffuse(TextureFile& texFile) {
            TexEditor::setTransparency(texFile, NearlyTransparent);
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::xiangling4_0() {
        // Remapped onto XianglingCheer -- a MERGE, and the direction that GAINS a normal map.
        //
        // Xiangling draws head, body and dress; XianglingCheer draws head and body. All three of
        // Xiangling's objects come through the skin's HEAD, which is three .ini files, and her body
        // is left bound to nothing (see objNewRegVals below).
        //
        // REGISTERED AT 4.0, NOT 6.1, AND THAT IS DELIBERATE -- it is the only fix here that is.
        // Every other character got a 6.1 row because GI 6.1 swapped which register the shader reads
        // the diffuse and the lightmap out of, and NNFix was written to put that back. ORFix's
        // maintainers then baked the same swap into ORFix itself, so a part that was ALREADY calling
        // ORFix needs no new row -- and this fix's head calls ORFix. The pure-Python table has no
        // xiangling6_1 for exactly this reason; the absence is the answer, not an oversight.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // ORDER IS THE CLAIM ORDER, so this reads out as: the mod's own file gets the head, the
        // first generated copy the body, and the second the dress -- which is what the old script's
        // three files hold. 'body' also maps to itself, in the first file only.
        config.objSplits = {{"head", {"head"}}, {"body", {"head", "body"}}, {"dress", {"head"}}};
        config.copyPreamble = IniComments::GIMIObjMergerPreamble;

        // ---- the registers each source object arrives with ----
        //
        // SOURCE-keyed, because the three sources do not agree: only the body carries a ps-t3 worth
        // removing. Keyed by target instead, that ps-3 removal would also strip the head's and the
        // dress's -- which they may well have, since a 4.0-era mod binds a shadow ramp there.
        config.srcObjRegRemovals = {{"head", {"ps-t2"}},
                                    {"body", {"ps-t2", "ps-t3"}},
                                    {"dress", {"ps-t2"}}};

        // ...and the shift itself IS the same for all three, so it stays target-keyed. Both renames
        // in one entry so they are applied in a single pass: ps-t0 -> ps-t1 must not then be re-read
        // as the input to ps-t1 -> ps-t2.
        //
        // ps-t0 keeps a copy of itself so the slot is still bound when the texAdd below overwrites
        // it -- the same trick Ganyu uses, and for the same reason.
        const std::vector<std::pair<std::string, std::vector<std::string>>> shift = {
            {"ps-t1", {"ps-t2"}}, {"ps-t0", {"ps-t0", "ps-t1"}}};
        config.objRegRemaps = {{"head", shift}, {"body", shift}};

        // The head's diffuse is darkened on the way over -- and ONLY the head's. All three sources
        // land on the target's head, so without the srcObj this would darken the body and dress
        // copies too; the old script's RemapFix1 and RemapFix2 point at the untouched
        // ...BodyDiffuse.0 and ...DressDiffuse.0.
        config.texEdits = {{"head", "ps-t0", "DarkDiffuse", &darkenDiffuse, true, "head"}};

        // The normal map that fills the slot the shift vacated. No srcObj: the pure-Python row
        // declares one for every source object, and the collectors are built per group, so one
        // entry produces one texture per source -- named after that source's own resource.
        config.texAdds = {{"head", "ps-t0", "NormMap",
                            TexCreator(NormalMapSize, NormalMapSize, NormalMapBlue)}};

        // The body is bound to nothing: everything it draws is coming through the head above.
        config.objNewRegVals = {{"body", {{IniKeywords::Ib, "null"}}}};

        // ORFix on the head and NOTHING on the body -- an empty list is how "no call at all" is
        // asked for, since the default is NNFix. See this function's own note on why ORFix alone is
        // enough here.
        config.objFixCalls = {{"head", {IniKeywords::ORFixPath}}, {"body", {}}};

        // ---- the position translation ----
        //
        // Xiangling and XianglingCheer were authored around DIFFERENT ORIGINS -- about 0.78
        // units apart in Y -- so a mod remapped between them is correct in every other way and
        // still lands in mid-air. Every vertex moves to match.
        //
        // This is the only pair in the repo that needs it: 46 of the 47 entries in the
        // pure-Python PositionEditorData table are None. See
        // GIMICharFixerConfig::positionEdit before assuming a new character wants one.
        const double OffsetY = 0.7755;
        const double OffsetZ = -0.0405;

        config.positionEdit = [OffsetY, OffsetZ](const BufLineData& line, long long, double, long long) {
            BufLineData result = line;

            // POSITION is the element name PositionFile gives the first 12 bytes of a vertex;
            // a line that somehow has none is left exactly as it came in rather than guessed at.
            auto position = result.find("POSITION");
            if (position == result.end() || position->second.size() < 3) {
                return result;
            }

            // A Float32 decodes to the variant's double alternative. Anything else is not a
            // coordinate this fix knows how to move, so it is left alone.
            if (std::holds_alternative<double>(position->second[1])) {
                position->second[1] = std::get<double>(position->second[1]) + OffsetY;
            }

            if (std::holds_alternative<double>(position->second[2])) {
                position->second[2] = std::get<double>(position->second[2]) + OffsetZ;
            }

            return result;
        };
        // moveDrawIndexed stays false: the pure-Python row carries none of the Ib* entries.
        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory XianglingFixer::v4_0() {
        return IniFixBuilderFuncs::xiangling4_0();
    }
}
