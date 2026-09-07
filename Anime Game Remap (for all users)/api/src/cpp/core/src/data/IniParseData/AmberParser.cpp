#include "AGRemapCore/data/IniParseData/AmberParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::amber4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Amber does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Amber;
        config.downloadCharFolder = "Amber";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Amber";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory AmberParser::v4_0() {
        return IniParseBuilderFuncs::amber4_0();
    }
}
