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

#include "AGRemapCore/data/IniParseData/Arlecchino/ArlecchinoParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::arlecchino5_4() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Arlecchino does differently lives here.

        // HER PURE-PYTHON PARSE ROW REGISTERS NO DOWNLOADS AT ALL -- the only 5.x row in that table
        // setting neither bufDownloads nor objFileDownloads, and FileDownloadData carries not one
        // Arlecchino entry. The assets were on disk the whole time: Data/Mod Downloads/GI/Arlecchino
        // holds a 4_6 and a 5_4, and 5_4 already shipped the buffers, the index buffers and the face
        // diffuse. Only the six object textures were missing, and those came straight out of
        // GI-Model-Importer-Assets' PlayerCharacterData/Arlecchino (2026-09-12). So wiring downloads
        // up here closes a gap in the old script rather than adding a dependency.
        //
        // 5_4 and not 4_6: the NEWEST version subfolder is the one to point at (see CreatingRemaps'
        // "The download assets"), and it is the only one of the two carrying a face diffuse.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Arlecchino;
        config.downloadCharFolder = "Arlecchino";
        config.downloadVersionFolder = "5_4";
        config.downloadPrefix = "Arlecchino";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory ArlecchinoParser::v5_4() {
        return IniParseBuilderFuncs::arlecchino5_4();
    }
}
