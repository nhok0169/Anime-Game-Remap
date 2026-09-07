#include "AGRemapCore/data/IniParseData/MonaParser.h"

#include "AGRemapCore/constants/ModTypeId.h"
#include "AGRemapCore/data/IniParseBuilderData.h"
#include "AGRemapCore/data/IniParseData/GIMICharParser.h"


namespace AGRemapCore {

    IniParseBuilder::Factory IniParseBuilderFuncs::mona4_0() {
        // The standard GIMI character shape -- see makeGIMICharParser for what that means and for
        // every mod object this produces. Only what Mona does differently lives here.
        GIMICharParserConfig config{};
        config.modTypeId = ModTypeId::Mona;
        config.downloadCharFolder = "Mona";
        config.downloadVersionFolder = "4_0";
        config.downloadPrefix = "Mona";
        config.drawnObjs = {"head", "body"};
        config.texcoordStride = 12;

        return makeGIMICharParser(std::move(config));
    }


    IniParseBuilder::Factory MonaParser::v4_0() {
        return IniParseBuilderFuncs::mona4_0();
    }
}
