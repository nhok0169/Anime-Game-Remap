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

#include "AGRemapCore/data/IniParseData/KleeBlossomingStarlight/KleeBlossomingStarlightParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::kleeBlossomingStarlight4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what KleeBlossomingStarlight does differently lives here.

        // SHE HAS A DRESS AND KLEE DOES NOT, which is the whole shape of this pair: remapping
        // off her MERGES her dress into Klee's body, and remapping onto her SPLITS Klee's body
        // in two. See KleeBlossomingStarlightFixer and KleeFixer respectively.
        //
        // NOTE: she has no Face component in GI-Model-Importer-Assets at all, so unlike Klee
        // there is no tex_face_diffuse row for her in HashData -- see the note there.

        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::KleeBlossomingStarlight;
        config.downloadCharFolder = "KleeBlossomingStarlight";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "KleeBlossomingStarlight";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory KleeBlossomingStarlightParser::v4_0() {
        return IniParseBuilderFuncs::kleeBlossomingStarlight4_0();
    }
}
