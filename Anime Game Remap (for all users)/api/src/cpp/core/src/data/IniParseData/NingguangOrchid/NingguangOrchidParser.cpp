#include "AGRemapCore/data/IniParseData/NingguangOrchid/NingguangOrchidParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::ningguangOrchid4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what NingguangOrchid does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::NingguangOrchid;
        config.downloadCharFolder = "NingguangOrchid";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "NingguangOrchid";
        config.drawnObjs = {"head", "body", "dress"};
        config.texcoordStride = 20;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory NingguangOrchidParser::v4_0() {
        return IniParseBuilderFuncs::ningguangOrchid4_0();
    }


}
