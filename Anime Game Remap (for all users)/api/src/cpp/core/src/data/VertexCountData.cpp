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

#include "AGRemapCore/data/VertexCountData.h"

// See VertexCountData.h's own note: mechanically generated from the real, live pure-Python
// VertexCountData dict (a script imported the actual module and walked it), never
// hand-transcribed. Grouped/commented by version, mirroring the Python dict's own structure.
// Future vertex-count updates edit the literal below directly.
//
// The third index column is the component name. Every row below carries "" for it -- no
// vertex count is component-specific yet -- but the column exists so one can be without
// reshaping the table. An empty component is a real key value here, not a "missing" marker,
// exactly as in IndexData.

#include "AGRemapCore/constants/ModTypeId.h"


namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, int>>& getVertexCountDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, int>> rows = {
        // ===== version 4.0 =====
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Amber), ""}, 10406},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::AmberCN), ""}, 10514},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Ayaka), ""}, 15700},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::AyakaSpringbloom), ""}, 19401},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Barbara), ""}, 12498},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::BarbaraSummertime), ""}, 13580},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Diluc), ""}, 13618},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::DilucFlamme), ""}, 16897},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Fischl), ""}, 11834},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::FischlHighness), ""}, 22225},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Ganyu), ""}, 14871},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::HuTao), ""}, 14427},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Jean), ""}, 13279},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanCN), ""}, 12061},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::JeanSea), ""}, 14672},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Kaeya), ""}, 14711},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::KaeyaSailwind), ""}, 21365},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Keqing), ""}, 15009},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::KeqingOpulent), ""}, 16066},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Kirara), ""}, 20396},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Klee), ""}, 13647},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::KleeBlossomingStarlight), ""}, 24110},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Lisa), ""}, 13644},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::LisaStudent), ""}, 19683},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Mona), ""}, 13529},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::MonaCN), ""}, 13333},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Nilou), ""}, 18458},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Ningguang), ""}, 12931},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::NingguangOrchid), ""}, 16612},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Raiden), ""}, 13251},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Rosaria), ""}, 12515},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::RosariaCN), ""}, 13992},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Shenhe), ""}, 13830},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Xiangling), ""}, 12352},
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Xingqiu), ""}, 13068},

        // ===== version 4.4 =====
        {{"4.4", ModTypeIdTools::getName(ModTypeId::ShenheFrostFlower), ""}, 19357},
        {{"4.4", ModTypeIdTools::getName(ModTypeId::GanyuTwilight), ""}, 24118},
        {{"4.4", ModTypeIdTools::getName(ModTypeId::XingqiuBamboo), ""}, 20494},

        // ===== version 4.6 =====
        {{"4.6", ModTypeIdTools::getName(ModTypeId::Arlecchino), ""}, 22510},

        // ===== version 4.8 =====
        {{"4.8", ModTypeIdTools::getName(ModTypeId::NilouBreeze), ""}, 21830},
        {{"4.8", ModTypeIdTools::getName(ModTypeId::KiraraBoots), ""}, 20854},

        // ===== version 5.3 =====
        {{"5.3", ModTypeIdTools::getName(ModTypeId::CherryHuTao), ""}, 23136},
        {{"5.3", ModTypeIdTools::getName(ModTypeId::XianglingCheer), ""}, 22151},

        // ===== Yelan (2026-09-12) =====
        // The game's own model, counted off her identity mod (Tools/Misc/Prototypes/identityMod.py).
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Yelan), ""}, 16062},

        // ===== Bennett (2026-09-14) =====
        // The game's own model, counted off Data/Mod Downloads/GI/Bennett/4_0's Position.buf
        // (657320 bytes / 40). No row for BennettAdventure: as with YelanTranquil, a skin of
        // several components has no single vertex count.
        {{"4.0", ModTypeIdTools::getName(ModTypeId::Bennett), ""}, 16433},

        // ===== Citlali (2026-09-21) =====
        // The game's own model, counted off Data/Mod Downloads/GI/Citlali/5_3's Position.buf
        // (1084080 bytes / 40). No row for CitlaliWhisperofStars: a skin of several components has
        // no single vertex count.
        {{"5.3", ModTypeIdTools::getName(ModTypeId::Citlali), ""}, 27102},

        // ===== WuWa: Sanhua and SanhuaExorcist (2026-09-19) =====
        // The whole mesh, every draw slot together -- Metadata.json's 'vertex_count', and what a
        // WWMI mod declares as $mesh_vertex_count.
        {{"2.5", ModTypeIdTools::getName(ModTypeId::Sanhua), ""}, 36126},
        {{"2.5", ModTypeIdTools::getName(ModTypeId::SanhuaExorcist), ""}, 40896},
    };

    return rows;
}

}
}
