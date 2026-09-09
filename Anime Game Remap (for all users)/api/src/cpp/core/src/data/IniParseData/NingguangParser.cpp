#include "AGRemapCore/data/IniParseData/NingguangParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::ningguang4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Ningguang does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Ningguang;
        config.downloadCharFolder = "Ningguang";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Ningguang";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory NingguangParser::v4_0() {
        return IniParseBuilderFuncs::ningguang4_0();
    }


}
