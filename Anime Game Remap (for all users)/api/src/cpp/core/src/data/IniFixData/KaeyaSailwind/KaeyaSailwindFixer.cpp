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

#include "AGRemapCore/data/IniFixData/KaeyaSailwind/KaeyaSailwindFixer.h"

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/IniFixBuilderData.h"
#include "AGRemapCore/data/IniFixData/GIMICharFixer.h"


namespace AGRemapCore {
    namespace {
        // The 3dmigoto reflection-support keys. They name the SOURCE's texture slots, so
        // carrying them into a remapped section aims the reflection pass at the wrong
        // textures -- the pure-Python row strips them as ReflectionXxxRemove.
        std::vector<GIMICharFixerConfig::RegRef> reflectionKeys(const std::string& obj) {
            return {"ResourceRef" + obj + "Diffuse", "ResourceRef" + obj + "LightMap", "$CharacterIB"};
        }
    }


    IniFixBuilder::Factory IniFixBuilderFuncs::kaeyaSailwind4_0() {
        // THE 4.0 FIX for KaeyaSailwind -> Kaeya, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE SPLIT into four: his dress becomes Kaeya's dress AND his extra.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"dress", "extra"}}};

        std::vector<GIMICharFixerConfig::RegRef> bodyRem = reflectionKeys("Body");
        bodyRem.push_back({"ps-t0"});

        config.objRegRemovals = {{"head", reflectionKeys("Head")},
                                 {"body", bodyRem},
                                 {"dress", reflectionKeys("Dress")}};

        config.objRegRemaps = {{"body", {{"ps-t1", {{"ps-t0"}}, true}, {"ps-t2", {{"ps-t1"}}, true},
                                         {"ps-t3", {{"ps-t2"}}, true}}}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = true;
        //   ^ its removal set carries ORFixCompleteRemoval, inside Reflection*Remove.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", {IniKeywords::TexFxTransparency0Pre5_0}},
                              {"dress", std::vector<std::string>{}},
                              {"extra", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }
    IniFixBuilder::Factory IniFixBuilderFuncs::kaeyaSailwind5_0() {
        // THE 5.0 FIX, and it is kaeyaSailwind4_0 with ONE change: TexFx renamed its
        // transparency sub-commands at 5.0, T.0/T.1 becoming the Natlan TN.0/TN.1. That
        // rename is the entire difference between the pure-Python rows' ...4_0 and ...5_0
        // value-rename constants, and issuing the wrong one names a sub-command that
        // version of the library does not have -- which shows up as an effect that
        // quietly does nothing.
        //
        // kaeyaSailwind5_0 and kaeyaSailwind4_0 are otherwise character-for-character identical.
        //
        // Originally for KaeyaSailwind -> Kaeya, verified only against the old script at
        // --version 4.0 --fromVersion 4.0 -- the game cannot be rolled back to play it.
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE SPLIT into four: his dress becomes Kaeya's dress AND his extra.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"dress", "extra"}}};

        std::vector<GIMICharFixerConfig::RegRef> bodyRem = reflectionKeys("Body");
        bodyRem.push_back({"ps-t0"});

        config.objRegRemovals = {{"head", reflectionKeys("Head")},
                                 {"body", bodyRem},
                                 {"dress", reflectionKeys("Dress")}};

        config.objRegRemaps = {{"body", {{"ps-t1", {{"ps-t0"}}, true}, {"ps-t2", {{"ps-t1"}}, true},
                                         {"ps-t3", {{"ps-t2"}}, true}}}};

        // ---- what this row does with the 6.1-era defaults ----
        config.swapFaceRegs = false;
        config.removeSrcFixCalls = true;
        //   ^ its removal set carries ORFixCompleteRemoval, inside Reflection*Remove.
        config.removeSrcTexFxCalls = true;   // its TexFxRemove is a FOLDER match

        // The external libraries this row re-issues, keyed by TARGET. An empty list means
        // none, and REPLACES the template's default NNFix.
        config.objFixCalls = {{"head", std::vector<std::string>{}},
                              {"body", {IniKeywords::TexFxTransparency0}},
                              {"dress", std::vector<std::string>{}},
                              {"extra", std::vector<std::string>{}}};

        return makeGIMICharFixer(std::move(config));
    }

    IniFixBuilder::Factory IniFixBuilderFuncs::kaeyaSailwind6_1ToKaeya() {
        GIMICharFixerConfig config{};
        config.drawnObjs = {"head", "body", "dress"};

        // THE SPLIT, into FOUR target objects. Kaeya's model has an 'extra' -- IndexData gives it
        // 47727 -- that no Kaeya .ini file declares and that KaeyaSailwind has no geometry for, so
        // her DRESS graph is emitted twice, once under each index. The first split in the repo whose
        // target draws more objects than either character's parser lists.
        config.objSplits = {{"head", {"head"}}, {"body", {"body"}}, {"dress", {"dress", "extra"}}};

        // ---- THE REFLECTION KEYS ----
        //
        // kaeyaSailwind6_1's Reflection<Obj>Remove sets. These are not registers: they are the
        // 3dmigoto reflection-support keys a mod writes next to its bindings -- 'ResourceRef<Obj>
        // Diffuse = reference ps-t1', 'ResourceRef<Obj>LightMap = reference ps-t2' and
        // '$CharacterIB = N'. They name KaeyaSailwind's slots, so carried across unchanged they
        // point the reflection pass at the wrong textures. The ORFix/NNFix half of those same sets
        // is stripped by the template already.
        //
        // 'extra' is deliberately absent, matching the row: the pure-Python table defines a
        // ReflectionExtraRemove and then does not use it here, so the extra copy keeps the dress's
        // keys. Transcribed, not corrected.
        config.objRegRemovals = {{"head", {"ResourceRefHeadDiffuse", "ResourceRefHeadLightMap",
                                           "$CharacterIB"}},
                                 {"body", {"ResourceRefBodyDiffuse", "ResourceRefBodyLightMap",
                                           "$CharacterIB"}},
                                 {"dress", {"ResourceRefDressDiffuse", "ResourceRefDressLightMap",
                                            "$CharacterIB"}}};

        // The body shifts DOWN a slot, the exact mirror of kaeya6_1 shifting it up: her ps-t0 normal
        // map is dropped by being overwritten rather than removed (ps-t1 lands on it), so there is
        // no ps-t0 entry in the removals above. kaeyaSailwind4_0 DID remove it explicitly and 5_0
        // and 6_1 do not -- the later rows rely on the overwrite.
        config.objRegRemaps = {{"body", {{"ps-t1", {"ps-t0"}}, {"ps-t2", {"ps-t1"}},
                                         {"ps-t3", {"ps-t2"}}}}};

        // TexFx alongside NNFix on the body, naming ps-t0 as the diffuse's home now that the shift
        // has put it back there (TN.0). The mirror of kaeya6_1's TN.1.
        config.objFixCalls = {{"body", {IniKeywords::NNFixPath, IniKeywords::TexFxTransparency0}}};

        return makeGIMICharFixer(std::move(config));
    }


    IniFixBuilder::Factory KaeyaSailwindFixer::v6_1ToKaeya() {
        return IniFixBuilderFuncs::kaeyaSailwind6_1ToKaeya();
    }
}
