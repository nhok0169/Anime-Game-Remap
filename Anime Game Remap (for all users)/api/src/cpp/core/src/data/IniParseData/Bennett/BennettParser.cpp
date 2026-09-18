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

#include "AGRemapCore/data/IniParseData/Bennett/BennettParser.h"

#include <utility>

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::bennett4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Bennett does differently lives here.
        //
        // TWO drawn objects, in draw order, where Yelan has four. His eyes are not one of them:
        // they belong to whichever object the mod's author put them in, and on the real mods
        // measured they sit in the BODY (205 vertices on his head bones, referenced only by the
        // body index buffer), not the head.
        //
        // His Texcoord is stride 12 -- COLOR 4 + TEXCOORD 8, no second UV set -- where Yelan's is
        // 20. That is the GAME model's stride, counted off Data/Mod Downloads/GI/Bennett/4_0's
        // Texcoord.buf (197196 bytes / 16433 vertices). A MOD is free to carry a second UV set and
        // be 20; the fixer measures what it is handed rather than trusting this, which is the whole
        // of the lesson in CreatingRemaps' "A REMAPPED SECTION MAY BIND ONLY WHAT THE TARGET'S SLOT
        // BINDS".
        //
        // The downloads are his identity: Data/Mod Downloads/GI/Bennett/4_0 holds the game's own
        // buffers and textures, so a mod that names no lightmap for an object still draws with one.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Bennett;
        config.downloadCharFolder = "Bennett";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Bennett";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory BennettParser::v4_0() {
        return IniParseBuilderFuncs::bennett4_0();
    }
}

// ==========================================================================================
// KNOWN, NOT BENNETT'S: the face diffuse is only ever looked for at ps-t0 (2026-09-15)
// ==========================================================================================
//
// makeGIMICharParser registers the face download against ONE register:
//
//     add(faceConfig, {"", "face"}, "ps-t0", "FaceDiffuse", "FaceDiffuse", ".dds");
//
// A mod authored before GI 6.x binds its face diffuse at ps-t0 and a mod built from a 6.x dump --
// an identity mod, say -- binds it at ps-t1, which GI 6.x swapped. So on a 6.x-shaped mod the
// register looks MISSING and a download is registered for a texture the mod already has.
//
// Measured on Bennett's own identity mod: with the face at ps-t1 the run reports
// `download: fixed 1`; rebinding that one line to ps-t0 and changing nothing else gives
// `download: fixed 0, skipped 0`. Yelan's identity mod binds ps-t0, which is why forty-odd
// characters have never shown it.
//
// The cost is a redundant fetch, not a broken binding -- the download resolves to the real asset
// now that Data/Mod Downloads is published on the nhok0169 branch the downloader reads
// (DownloadTools' base URL). Before that merge it 404'd and the output carried
// `ps-t1 = ResourceBennettFaceDiffuseRemapDL` naming a file that was never written, which is the
// worse failure and the one to watch for if a character's assets are ever unpublished again.
//
// NOT fixed here because it is not Bennett's to fix: the register is hardcoded in the SHARED parser
// and GIMICharParserConfig has no field for it, so the change lands on every classic character at
// once and wants an A/B across them. Tools/Misc/Prototypes/bennettAdventureFix.py reads BOTH
// registers and says why.
