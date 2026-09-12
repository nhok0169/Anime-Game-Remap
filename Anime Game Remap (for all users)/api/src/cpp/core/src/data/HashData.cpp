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

#include "AGRemapCore/data/HashData.h"

// See HashData.h's class-level note: mechanically generated from the real, live pure-Python
// HashData dict, verified row-for-row identical before being committed -- not hand-transcribed.
//
// ===== THE POLICY FOR THIS TABLE: FOLLOW THE ASSETS REPO, EVEN WHERE IT IS WRONG =====
//
// Carried over by hand (2026-09-11) because the mechanical generation brought the VALUES across
// and left the reasoning behind, in a pre-migration file that now survives only inside a git
// worktree. The maintainer's own notes on three rows, verbatim:
//
//   LisaStudent's ib row:
//     "Which mf classified ps-t0 as diffuse, in actuality, this a normal map"
//
//   ShenheFrostFlower's head/dress normal maps:
//     "Seriously, which son of a gun added normal maps for ShenheFrostFlower. She has no normal
//      maps. Im just going to follow what GIMI assets has even though I know it is wrong"
//
//   ShenheFrostFlower's tex_dress_shadowramp, whose value is the STRING "000050-ps-t3":
//     "is the hash for tex_dress_shadowramp even valid?", linking
//     github.com/SilentNightSound/GI-Model-Importer-Assets/commit/79d40a4d4708500f44035ade5a6be6c5d6cb0285
//
// So: do NOT 'correct' a row that looks wrong. It is probably wrong on purpose, matching an
// upstream dump. What IS in bounds is following that repo to a NEWER state of the same file --
// see Kirara's face rows below, where the assets themselves were later re-dumped.
//
// ONE CARVE-OUT, and it is narrow: a value that could not have come from any dump at all.
// A hash is eight hex digits; `29cf09   14` is not a hash that happens to be wrong, it is a
// TYPO, and it matches nothing whatsoever. Two of those were found and repaired on
// 2026-09-11 (Nilou and GanyuTwilight, both `tex_dress_lightmap`) -- see the notes at those
// rows for why each replacement value is evidenced rather than guessed. Everything else in
// this table stays as upstream has it. The sweep that found them is worth repeating after
// any bulk edit: every value should match ^[0-9a-f]{8}$, and today the only intended
// exceptions are ShenheFrostFlower's two `000050-ps-t3` shadow ramps above.
// Grouped/commented by version, then by mod name, mirroring the pre-migration Python dict's own
// visual structure. Future hash updates edit the literal below directly.

namespace AGRemapCore {
namespace Data {

const std::vector<std::pair<std::vector<std::string>, std::string>>& getHashDataRows() {
    static const std::vector<std::pair<std::vector<std::string>, std::string>> rows = {
        // ===== version 1.0 =====
        // Barbara
        {{"1.0", "Barbara", "draw_vb"}, "f41c47cf"},
        {{"1.0", "Barbara", "position_vb"}, "85282151"},
        {{"1.0", "Barbara", "blend_vb"}, "02089582"},
        {{"1.0", "Barbara", "texcoord_vb"}, "0f18519e"},
        {{"1.0", "Barbara", "ib"}, "231723d2"},
        {{"1.0", "Barbara", "tex_head_diffuse"}, "d9d24fbf"},
        {{"1.0", "Barbara", "tex_head_lightmap"}, "f89f1ed6"},
        {{"1.0", "Barbara", "tex_head_metalmap"}, "b0e08915"},
        {{"1.0", "Barbara", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"1.0", "Barbara", "tex_body_diffuse"}, "d5fd9da6"},
        {{"1.0", "Barbara", "tex_body_lightmap"}, "0c0ce0ef"},
        {{"1.0", "Barbara", "tex_body_metalmap"}, "b0e08915"},
        {{"1.0", "Barbara", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"1.0", "Barbara", "tex_dress_diffuse"}, "d5fd9da6"},
        {{"1.0", "Barbara", "tex_dress_lightmap"}, "0c0ce0ef"},
        {{"1.0", "Barbara", "tex_dress_metalmap"}, "b0e08915"},
        {{"1.0", "Barbara", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"1.0", "Barbara", "tex_face_diffuse"}, "d9f80241"},
        {{"1.0", "Barbara", "tex_face_lightmap"}, "4e3376db"},
        {{"1.0", "Barbara", "tex_face_shadow"}, "3f396398"},
        {{"1.0", "Barbara", "tex_face_shadowramp"}, "7eb5b84e"},

        // ===== version 3.7 =====
        // Lisa
        {{"3.7", "Lisa", "draw_vb"}, "6f4a034a"},
        {{"3.7", "Lisa", "position_vb"}, "7d8a4e0f"},
        {{"3.7", "Lisa", "blend_vb"}, "de1311ae"},
        {{"3.7", "Lisa", "texcoord_vb"}, "50ae5602"},
        {{"3.7", "Lisa", "ib"}, "695e029f"},
        // HER FACE DIFFUSE, which this table did not have (added 2026-09-12). Confirmed against
        // GI-Model-Importer-Assets/PlayerCharacterData/Lisa, whose Face component reads
        // Diffuse = 66bea1c9. Without it a Lisa mod's own face section does not classify as the
        // face at all, so the fix leaves it alone and emits a download instead -- the same
        // failure AyakaSpringbloom had before her row was added.
        {{"3.7", "Lisa", "tex_face_diffuse"}, "66bea1c9"},
        {{"3.7", "Lisa", "tex_head_diffuse"}, "b542085f"},
        {{"3.7", "Lisa", "tex_head_lightmap"}, "f69e017e"},
        {{"3.7", "Lisa", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"3.7", "Lisa", "tex_body_diffuse"}, "2014031e"},
        {{"3.7", "Lisa", "tex_body_lightmap"}, "aa3b4074"},
        {{"3.7", "Lisa", "tex_body_metalmap"}, "b0e08915"},
        {{"3.7", "Lisa", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"3.7", "Lisa", "tex_dress_diffuse"}, "2014031e"},
        {{"3.7", "Lisa", "tex_dress_lightmap"}, "aa3b4074"},
        {{"3.7", "Lisa", "tex_dress_shadowramp"}, "7eb5b84e"},

        // ===== version 4.0 =====
        // Amber
        {{"4.0", "Amber", "draw_vb"}, "870a7499"},
        {{"4.0", "Amber", "position_vb"}, "caddc4c6"},
        {{"4.0", "Amber", "blend_vb"}, "ca5bd26e"},
        {{"4.0", "Amber", "texcoord_vb"}, "e3047676"},
        {{"4.0", "Amber", "ib"}, "9976d124"},
        {{"4.0", "Amber", "tex_head_diffuse"}, "ae27902d"},
        {{"4.0", "Amber", "tex_head_lightmap"}, "29b001ba"},
        {{"4.0", "Amber", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Amber", "tex_body_diffuse"}, "bc86882f"},
        {{"4.0", "Amber", "tex_body_lightmap"}, "9e1294dd"},
        {{"4.0", "Amber", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Amber", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Amber", "tex_face_diffuse"}, "1d064079"},
        {{"4.0", "Amber", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Amber", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Amber", "tex_face_shadowramp"}, "7eb5b84e"},
        // AmberCN
        {{"4.0", "AmberCN", "draw_vb"}, "da0adf2f"},
        {{"4.0", "AmberCN", "position_vb"}, "7f94e8da"},
        {{"4.0", "AmberCN", "blend_vb"}, "f35340d5"},
        {{"4.0", "AmberCN", "texcoord_vb"}, "dbc594b6"},
        {{"4.0", "AmberCN", "ib"}, "8cc9274b"},
        {{"4.0", "AmberCN", "tex_head_diffuse"}, "ae27902d"},
        {{"4.0", "AmberCN", "tex_head_lightmap"}, "29b001ba"},
        {{"4.0", "AmberCN", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "AmberCN", "tex_body_diffuse"}, "f683bcac"},
        {{"4.0", "AmberCN", "tex_body_lightmap"}, "69b6e698"},
        {{"4.0", "AmberCN", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "AmberCN", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "AmberCN", "tex_face_diffuse"}, "1d064079"},
        {{"4.0", "AmberCN", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "AmberCN", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "AmberCN", "tex_face_shadowramp"}, "7eb5b84e"},
        // Ayaka
        {{"4.0", "Ayaka", "draw_vb"}, "14c9337a"},
        {{"4.0", "Ayaka", "position_vb"}, "0107925f"},
        {{"4.0", "Ayaka", "blend_vb"}, "3d534190"},
        {{"4.0", "Ayaka", "texcoord_vb"}, "0c7e5f66"},
        {{"4.0", "Ayaka", "ib"}, "347bb8f8"},
        {{"4.0", "Ayaka", "tex_head_diffuse"}, "b017ea8d"},
        {{"4.0", "Ayaka", "tex_head_lightmap"}, "6922f755"},
        {{"4.0", "Ayaka", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ayaka", "tex_body_diffuse"}, "2991152b"},
        {{"4.0", "Ayaka", "tex_body_lightmap"}, "9e7eb1bf"},
        {{"4.0", "Ayaka", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Ayaka", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ayaka", "tex_dress_diffuse"}, "2991152b"},
        {{"4.0", "Ayaka", "tex_dress_lightmap"}, "9e7eb1bf"},
        {{"4.0", "Ayaka", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ayaka", "tex_face_diffuse"}, "146097c4"},
        {{"4.0", "Ayaka", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Ayaka", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Ayaka", "tex_face_shadowramp"}, "7eb5b84e"},
        // AyakaSpringBloom
        {{"4.0", "AyakaSpringBloom", "draw_vb"}, "8d173084"},
        {{"4.0", "AyakaSpringBloom", "position_vb"}, "cf78a1d0"},
        {{"4.0", "AyakaSpringBloom", "blend_vb"}, "f47a5c08"},
        {{"4.0", "AyakaSpringBloom", "texcoord_vb"}, "3990db1d"},
        {{"4.0", "AyakaSpringBloom", "ib"}, "bb6ced0e"},
        {{"4.0", "AyakaSpringBloom", "tex_head_normalmap"}, "379f92ff"},
        {{"4.0", "AyakaSpringBloom", "tex_head_diffuse"}, "1df6a5a7"},
        {{"4.0", "AyakaSpringBloom", "tex_head_lightmap"}, "e4ce0e6b"},
        {{"4.0", "AyakaSpringBloom", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "AyakaSpringBloom", "tex_body_normalmap"}, "2aca60d3"},
        {{"4.0", "AyakaSpringBloom", "tex_body_diffuse"}, "b3fc0184"},
        {{"4.0", "AyakaSpringBloom", "tex_body_lightmap"}, "f2f67036"},
        {{"4.0", "AyakaSpringBloom", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "AyakaSpringBloom", "tex_dress_diffuse"}, "b3fc0184"},
        {{"4.0", "AyakaSpringBloom", "tex_dress_lightmap"}, "f2f67036"},
        {{"4.0", "AyakaSpringBloom", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "AyakaSpringBloom", "tex_dress_metalmap"}, "b0e08915"},

        // HER FACE DIFFUSE, which this table did not have (added 2026-09-11). Confirmed against
        // GI-Model-Importer-Assets/PlayerCharacterData/AyakaSpringbloom, whose Face component reads
        // Diffuse = 146097c4 -- the SAME value Ayaka carries, which is the ordinary thing for a
        // base/skin pair (Amber/AmberCN share 1d064079, Mona/MonaCN share 8e116301).
        //
        // What its absence did: a mod's own face section did not classify as the face at all, so
        // the fix left it alone and instead emitted a [Resource...FaceDiffuseRemapDL] download --
        // which 404s, because no such file was ever uploaded. One dangling reference per mod.
        //
        // NilouBreeze and KiraraBoots had the same gap. Both are now filled in, on evidence of
        // two different strengths, and the difference is recorded at each row rather than here:
        // KiraraBoots has a real mod DECLARING her hash, while NilouBreeze's value rests on the
        // base/skin pattern alone. What made the weaker one acceptable is that its value turns
        // out not to reach the output in the direction that can be tested -- see her row for the
        // probe that showed it.
        {{"4.0", "AyakaSpringBloom", "tex_face_diffuse"}, "146097c4"},
        // Barbara
        {{"4.0", "Barbara", "blend_vb"}, "22a31278"},
        // BarbaraSummertime
        {{"4.0", "BarbaraSummertime", "draw_vb"}, "60fcbabe"},
        {{"4.0", "BarbaraSummertime", "position_vb"}, "8b9e7c22"},
        {{"4.0", "BarbaraSummertime", "blend_vb"}, "639d62b6"},
        {{"4.0", "BarbaraSummertime", "texcoord_vb"}, "27057f58"},
        {{"4.0", "BarbaraSummertime", "ib"}, "a411cfbc"},
        {{"4.0", "BarbaraSummertime", "tex_head_diffuse"}, "fa94dcc6"},
        {{"4.0", "BarbaraSummertime", "tex_head_lightmap"}, "07b96e90"},
        {{"4.0", "BarbaraSummertime", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "BarbaraSummertime", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "BarbaraSummertime", "tex_body_diffuse"}, "fa78e66c"},
        {{"4.0", "BarbaraSummertime", "tex_body_lightmap"}, "a8eec489"},
        {{"4.0", "BarbaraSummertime", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "BarbaraSummertime", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "BarbaraSummertime", "tex_dress_diffuse"}, "fa78e66c"},
        {{"4.0", "BarbaraSummertime", "tex_dress_lightmap"}, "a8eec489"},
        {{"4.0", "BarbaraSummertime", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "BarbaraSummertime", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "BarbaraSummertime", "tex_face_diffuse"}, "72a0dee8"},
        {{"4.0", "BarbaraSummertime", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "BarbaraSummertime", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "BarbaraSummertime", "tex_face_shadowramp"}, "7eb5b84e"},
        // Diluc
        {{"4.0", "Diluc", "draw_vb"}, "56159d74"},
        {{"4.0", "Diluc", "position_vb"}, "6fdb0963"},
        {{"4.0", "Diluc", "blend_vb"}, "6fd20cc4"},
        {{"4.0", "Diluc", "texcoord_vb"}, "aee0755a"},
        {{"4.0", "Diluc", "ib"}, "d1ac0687"},
        {{"4.0", "Diluc", "tex_head_diffuse"}, "575af152"},
        {{"4.0", "Diluc", "tex_head_lightmap"}, "7112b952"},
        {{"4.0", "Diluc", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Diluc", "tex_body_diffuse"}, "6e2d28e9"},
        {{"4.0", "Diluc", "tex_body_lightmap"}, "deff7a87"},
        {{"4.0", "Diluc", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Diluc", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Diluc", "tex_face_diffuse"}, "8b50c50f"},
        {{"4.0", "Diluc", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Diluc", "tex_face_shadow"}, "f596208e"},
        {{"4.0", "Diluc", "tex_face_shadowramp"}, "7eb5b84e"},
        // DilucFlamme
        {{"4.0", "DilucFlamme", "draw_vb"}, "aeab733d"},
        {{"4.0", "DilucFlamme", "position_vb"}, "a2d909c8"},
        {{"4.0", "DilucFlamme", "blend_vb"}, "105887c0"},
        {{"4.0", "DilucFlamme", "texcoord_vb"}, "16350d1b"},
        {{"4.0", "DilucFlamme", "ib"}, "9de6528c"},
        {{"4.0", "DilucFlamme", "tex_head_diffuse"}, "a8af7297"},
        {{"4.0", "DilucFlamme", "tex_head_lightmap"}, "2e936e4a"},
        {{"4.0", "DilucFlamme", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "DilucFlamme", "tex_body_diffuse"}, "d8f0b883"},
        {{"4.0", "DilucFlamme", "tex_body_lightmap"}, "05a1b11e"},
        {{"4.0", "DilucFlamme", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "DilucFlamme", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "DilucFlamme", "tex_dress_diffuse"}, "d8f0b883"},
        {{"4.0", "DilucFlamme", "tex_dress_lightmap"}, "05a1b11e"},
        {{"4.0", "DilucFlamme", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "DilucFlamme", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "DilucFlamme", "tex_face_diffuse"}, "8b50c50f"},
        {{"4.0", "DilucFlamme", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "DilucFlamme", "tex_face_shadow"}, "f596208e"},
        {{"4.0", "DilucFlamme", "tex_face_shadowramp"}, "7eb5b84e"},
        // Fischl
        {{"4.0", "Fischl", "draw_vb"}, "6c491d3b"},
        {{"4.0", "Fischl", "position_vb"}, "9838aedf"},
        {{"4.0", "Fischl", "blend_vb"}, "0d1c1932"},
        {{"4.0", "Fischl", "texcoord_vb"}, "d451d8d8"},
        {{"4.0", "Fischl", "ib"}, "5cfc7a92"},
        {{"4.0", "Fischl", "tex_head_diffuse"}, "8b7f4637"},
        {{"4.0", "Fischl", "tex_head_lightmap"}, "3b8e30d7"},
        {{"4.0", "Fischl", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Fischl", "tex_body_diffuse"}, "9f758879"},
        {{"4.0", "Fischl", "tex_body_lightmap"}, "3c5e7327"},
        {{"4.0", "Fischl", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Fischl", "tex_body_shadowramp"}, "59cd2559"},
        {{"4.0", "Fischl", "tex_dress_diffuse"}, "9f758879"},
        {{"4.0", "Fischl", "tex_dress_lightmap"}, "3c5e7327"},
        {{"4.0", "Fischl", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Fischl", "tex_face_diffuse"}, "0cd456af"},
        {{"4.0", "Fischl", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Fischl", "tex_face_shadow"}, "3f396398"},
        // FischlHighness
        {{"4.0", "FischlHighness", "draw_vb"}, "3cc8f82b"},
        {{"4.0", "FischlHighness", "position_vb"}, "8f473224"},
        {{"4.0", "FischlHighness", "blend_vb"}, "dbd6a5c3"},
        {{"4.0", "FischlHighness", "texcoord_vb"}, "a800a294"},
        {{"4.0", "FischlHighness", "ib"}, "95bf8d7e"},
        {{"4.0", "FischlHighness", "tex_head_diffuse"}, "de37696a"},
        {{"4.0", "FischlHighness", "tex_head_lightmap"}, "2f2f6932"},
        {{"4.0", "FischlHighness", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "FischlHighness", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "FischlHighness", "tex_body_diffuse"}, "a132243b"},
        {{"4.0", "FischlHighness", "tex_body_lightmap"}, "61c02f66"},
        {{"4.0", "FischlHighness", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "FischlHighness", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "FischlHighness", "tex_face_diffuse"}, "0cd456af"},
        {{"4.0", "FischlHighness", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "FischlHighness", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "FischlHighness", "tex_face_shadowramp"}, "7eb5b84e"},
        // Ganyu
        {{"4.0", "Ganyu", "draw_vb"}, "721ca964"},
        {{"4.0", "Ganyu", "position_vb"}, "a5169f1d"},
        {{"4.0", "Ganyu", "blend_vb"}, "6f47a39d"},
        {{"4.0", "Ganyu", "texcoord_vb"}, "cf27251f"},
        {{"4.0", "Ganyu", "ib"}, "2da186bc"},
        {{"4.0", "Ganyu", "tex_head_diffuse"}, "6d78ac96"},
        {{"4.0", "Ganyu", "tex_head_lightmap"}, "9b0d2126"},
        {{"4.0", "Ganyu", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ganyu", "tex_body_diffuse"}, "8a151913"},
        {{"4.0", "Ganyu", "tex_body_lightmap"}, "dbcf1d72"},
        {{"4.0", "Ganyu", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Ganyu", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ganyu", "tex_dress_diffuse"}, "8a151913"},
        {{"4.0", "Ganyu", "tex_dress_lightmap"}, "dbcf1d72"},
        {{"4.0", "Ganyu", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Ganyu", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ganyu", "tex_face_diffuse"}, "b2657593"},
        {{"4.0", "Ganyu", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Ganyu", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Ganyu", "tex_face_shadowramp"}, "7eb5b84e"},
        // HuTao
        {{"4.0", "HuTao", "draw_vb"}, "60345291"},
        {{"4.0", "HuTao", "position_vb"}, "dd16576c"},
        {{"4.0", "HuTao", "blend_vb"}, "153dba3f"},
        {{"4.0", "HuTao", "texcoord_vb"}, "51afdfcf"},
        {{"4.0", "HuTao", "ib"}, "0535853d"},
        {{"4.0", "HuTao", "tex_head_diffuse"}, "565beee6"},
        {{"4.0", "HuTao", "tex_head_lightmap"}, "245dac60"},
        {{"4.0", "HuTao", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "HuTao", "tex_body_diffuse"}, "e72dc049"},
        {{"4.0", "HuTao", "tex_body_lightmap"}, "ddfeb6b9"},
        {{"4.0", "HuTao", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "HuTao", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "HuTao", "tex_face_diffuse"}, "d00bb0ef"},
        {{"4.0", "HuTao", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "HuTao", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "HuTao", "tex_face_shadowramp"}, "7eb5b84e"},
        // Jean
        {{"4.0", "Jean", "draw_vb"}, "e6055135"},
        {{"4.0", "Jean", "position_vb"}, "191af650"},
        {{"4.0", "Jean", "blend_vb"}, "3cb8153c"},
        {{"4.0", "Jean", "texcoord_vb"}, "1722136c"},
        {{"4.0", "Jean", "ib"}, "29835d20"},
        {{"4.0", "Jean", "tex_head_diffuse"}, "dba2791d"},
        {{"4.0", "Jean", "tex_head_lightmap"}, "0bd77e81"},
        {{"4.0", "Jean", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Jean", "tex_body_diffuse"}, "d1ae8efe"},
        {{"4.0", "Jean", "tex_body_lightmap"}, "cee17ba5"},
        {{"4.0", "Jean", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Jean", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Jean", "tex_face_diffuse"}, "c2d1a57e"},
        {{"4.0", "Jean", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Jean", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Jean", "tex_face_shadowramp"}, "7eb5b84e"},
        // JeanCN
        {{"4.0", "JeanCN", "draw_vb"}, "2a29e333"},
        {{"4.0", "JeanCN", "position_vb"}, "93bb2522"},
        {{"4.0", "JeanCN", "blend_vb"}, "d159bf31"},
        {{"4.0", "JeanCN", "texcoord_vb"}, "0ffefb98"},
        {{"4.0", "JeanCN", "ib"}, "920c0b3f"},
        {{"4.0", "JeanCN", "tex_head_diffuse"}, "6eca0f93"},
        {{"4.0", "JeanCN", "tex_head_lightmap"}, "92ed604c"},
        {{"4.0", "JeanCN", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "JeanCN", "tex_body_diffuse"}, "0f9c7705"},
        {{"4.0", "JeanCN", "tex_body_lightmap"}, "617c45a0"},
        {{"4.0", "JeanCN", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "JeanCN", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "JeanCN", "tex_face_diffuse"}, "c2d1a57e"},
        {{"4.0", "JeanCN", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "JeanCN", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "JeanCN", "tex_face_shadowramp"}, "7eb5b84e"},
        // JeanSea
        {{"4.0", "JeanSea", "draw_vb"}, "972d56ee"},
        {{"4.0", "JeanSea", "position_vb"}, "16fef1eb"},
        {{"4.0", "JeanSea", "blend_vb"}, "ac801371"},
        {{"4.0", "JeanSea", "texcoord_vb"}, "3ffb0363"},
        {{"4.0", "JeanSea", "ib"}, "5114a891"},
        {{"4.0", "JeanSea", "tex_head_diffuse"}, "3b4efe72"},
        {{"4.0", "JeanSea", "tex_head_lightmap"}, "4b8a3da9"},
        {{"4.0", "JeanSea", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "JeanSea", "tex_body_diffuse"}, "e555db10"},
        {{"4.0", "JeanSea", "tex_body_lightmap"}, "15671abb"},
        {{"4.0", "JeanSea", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "JeanSea", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "JeanSea", "tex_dress_diffuse"}, "e555db10"},
        {{"4.0", "JeanSea", "tex_dress_lightmap"}, "15671abb"},
        {{"4.0", "JeanSea", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "JeanSea", "tex_face_diffuse"}, "c2d1a57e"},
        {{"4.0", "JeanSea", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "JeanSea", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "JeanSea", "tex_face_shadowramp"}, "7eb5b84e"},
        // Kaeya
        {{"4.0", "Kaeya", "draw_vb"}, "4b0aa762"},
        {{"4.0", "Kaeya", "position_vb"}, "8a081f34"},
        {{"4.0", "Kaeya", "blend_vb"}, "763b60b9"},
        {{"4.0", "Kaeya", "texcoord_vb"}, "fb2eff2a"},
        {{"4.0", "Kaeya", "ib"}, "13eb3d85"},
        {{"4.0", "Kaeya", "tex_head_diffuse"}, "0fbefdbe"},
        {{"4.0", "Kaeya", "tex_head_lightmap"}, "34e0c421"},
        {{"4.0", "Kaeya", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Kaeya", "tex_body_diffuse"}, "80a44a1a"},
        {{"4.0", "Kaeya", "tex_body_lightmap"}, "5d9ff9ba"},
        {{"4.0", "Kaeya", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Kaeya", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Kaeya", "tex_dress_diffuse"}, "80a44a1a"},
        {{"4.0", "Kaeya", "tex_dress_lightmap"}, "5d9ff9ba"},
        {{"4.0", "Kaeya", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Kaeya", "tex_face_diffuse"}, "6d5856da"},
        {{"4.0", "Kaeya", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Kaeya", "tex_face_shadow"}, "f596208e"},
        {{"4.0", "Kaeya", "tex_face_shadowramp"}, "7eb5b84e"},
        // KaeyaSailwind
        {{"4.0", "KaeyaSailwind", "draw_vb"}, "bdb6e3b7"},
        {{"4.0", "KaeyaSailwind", "position_vb"}, "b9b77eff"},
        {{"4.0", "KaeyaSailwind", "blend_vb"}, "e026c9ae"},
        {{"4.0", "KaeyaSailwind", "texcoord_vb"}, "74dce34a"},
        {{"4.0", "KaeyaSailwind", "ib"}, "59f2a0f2"},
        {{"4.0", "KaeyaSailwind", "tex_head_diffuse"}, "8bdd311f"},
        {{"4.0", "KaeyaSailwind", "tex_head_lightmap"}, "d0f3065c"},
        {{"4.0", "KaeyaSailwind", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "KaeyaSailwind", "tex_head_shadowramp"}, "58d2635b"},
        {{"4.0", "KaeyaSailwind", "tex_body_normalmap"}, "1077694d"},
        {{"4.0", "KaeyaSailwind", "tex_body_diffuse"}, "a48a72a3"},
        {{"4.0", "KaeyaSailwind", "tex_body_lightmap"}, "1f8619fc"},
        {{"4.0", "KaeyaSailwind", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "KaeyaSailwind", "tex_dress_diffuse"}, "8bdd311f"},
        {{"4.0", "KaeyaSailwind", "tex_dress_lightmap"}, "d0f3065c"},
        {{"4.0", "KaeyaSailwind", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "KaeyaSailwind", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "KaeyaSailwind", "tex_face_diffuse"}, "4e6a8e9d"},
        {{"4.0", "KaeyaSailwind", "tex_face_lightmap"}, "830046fd"},
        {{"4.0", "KaeyaSailwind", "tex_face_shadow"}, "5a1dc9f0"},
        {{"4.0", "KaeyaSailwind", "tex_face_shadowramp"}, "58d2635b"},
        // Keqing
        {{"4.0", "Keqing", "draw_vb"}, "4526145e"},
        {{"4.0", "Keqing", "position_vb"}, "3aaf3e94"},
        {{"4.0", "Keqing", "blend_vb"}, "0bf8e621"},
        {{"4.0", "Keqing", "texcoord_vb"}, "723848fe"},
        {{"4.0", "Keqing", "ib"}, "f325e394"},
        {{"4.0", "Keqing", "tex_head_diffuse"}, "58de714b"},
        {{"4.0", "Keqing", "tex_head_lightmap"}, "da3e4a28"},
        {{"4.0", "Keqing", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Keqing", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Keqing", "tex_body_diffuse"}, "874b8c0b"},
        {{"4.0", "Keqing", "tex_body_lightmap"}, "0695efb7"},
        {{"4.0", "Keqing", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Keqing", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Keqing", "tex_dress_diffuse"}, "874b8c0b"},
        {{"4.0", "Keqing", "tex_dress_lightmap"}, "0695efb7"},
        {{"4.0", "Keqing", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Keqing", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Keqing", "tex_face_diffuse"}, "d8c9c399"},
        {{"4.0", "Keqing", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Keqing", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Keqing", "tex_face_shadowramp"}, "7eb5b84e"},
        // KeqingOpulent
        {{"4.0", "KeqingOpulent", "draw_vb"}, "efcc8769"},
        {{"4.0", "KeqingOpulent", "position_vb"}, "0d7e3cc5"},
        {{"4.0", "KeqingOpulent", "blend_vb"}, "6f010b58"},
        {{"4.0", "KeqingOpulent", "texcoord_vb"}, "52f78cb7"},
        {{"4.0", "KeqingOpulent", "ib"}, "44bba21c"},
        {{"4.0", "KeqingOpulent", "tex_head_diffuse"}, "e2d7ae66"},
        {{"4.0", "KeqingOpulent", "tex_head_lightmap"}, "13e2b0ab"},
        {{"4.0", "KeqingOpulent", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "KeqingOpulent", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "KeqingOpulent", "tex_body_diffuse"}, "2af5bf71"},
        {{"4.0", "KeqingOpulent", "tex_body_lightmap"}, "195af53a"},
        {{"4.0", "KeqingOpulent", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "KeqingOpulent", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "KeqingOpulent", "tex_face_diffuse"}, "c2b17f84"},
        {{"4.0", "KeqingOpulent", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "KeqingOpulent", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "KeqingOpulent", "tex_face_shadowramp"}, "7eb5b84e"},
        // Kirara
        {{"4.0", "Kirara", "draw_vb"}, "e656b9fd"},
        {{"4.0", "Kirara", "position_vb"}, "cc833025"},
        {{"4.0", "Kirara", "blend_vb"}, "01d54938"},
        {{"4.0", "Kirara", "texcoord_vb"}, "33b3d6e5"},
        {{"4.0", "Kirara", "ib"}, "ce3dc5a2"},
        {{"4.0", "Kirara", "tex_head_normalmap"}, "6006d89d"},
        {{"4.0", "Kirara", "tex_head_diffuse"}, "0998fcda"},
        {{"4.0", "Kirara", "tex_head_lightmap"}, "c90298cc"},
        {{"4.0", "Kirara", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Kirara", "tex_body_normalmap"}, "acf97111"},
        {{"4.0", "Kirara", "tex_body_diffuse"}, "9feba8b9"},
        {{"4.0", "Kirara", "tex_body_lightmap"}, "2fadf527"},
        {{"4.0", "Kirara", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Kirara", "tex_dress_normalmap"}, "acf97111"},
        {{"4.0", "Kirara", "tex_dress_diffuse"}, "9feba8b9"},
        {{"4.0", "Kirara", "tex_dress_lightmap"}, "2fadf527"},
        {{"4.0", "Kirara", "tex_dress_metalmap"}, "b0e08915"},
        // KIRARA'S FACE: ONE TEXTURE, AND IT IS THE DIFFUSE (corrected 2026-09-11).
        //
        // This used to read
        //
        //     tex_face_normalmap  6eb20522
        //     tex_face_diffuse    4e3376db
        //     tex_face_lightmap   30180763
        //
        // which was a FAITHFUL copy of what GI-Model-Importer-Assets said at the time -- its 4.3
        // and 4.4 dumps really do list all three under Kirara's Face component. The assets repo
        // then corrected itself in its "Characters re-dump" commit, and has said this ever since:
        //
        //     Face -> [["Diffuse", ".dds", "6eb20522"]]
        //
        // One texture. The NormalMap and LightMap entries were phantoms, and 6eb20522 -- the hash
        // that never changed -- was simply MISLABELLED. So this is a label correction rather than a
        // hash update, which is why the 4.0 row moves too: the game's texture did not change, only
        // the dump's opinion of what it was.
        //
        // Two reasons the two dropped rows are dropped rather than left alone. tex_face_normalmap
        // was the ONLY such row in this entire table -- no other character has one, because faces
        // do not carry a normal map -- and keeping it alongside the corrected diffuse would give
        // Kirara two rows with the same hash, which is the shared-hash reverse-lookup trap where
        // RegAssetRemap is free to land on either.
        //
        // The symptom while it was wrong: a Kirara mod's real face section (hash 6eb20522) did not
        // classify as the face at all, so the fix left it alone and separately downloaded a face
        // diffuse the mod already had.
        //
        // NOTE ON POLICY, because this looks like it contradicts one and does not. The maintainer's
        // rule for this table is to FOLLOW the assets repo even where it is known to be wrong and
        // record the doubt -- see the pure-Python HashData.py's own notes on LisaStudent's
        // ps-t0 ("in actuality, this a normal map"), ShenheFrostFlower's invented normal maps ("Im
        // just going to follow what GIMI assets has even though I know it is wrong") and a
        // tex_dress_shadowramp whose value is the string "000050-ps-t3". This change does not
        // depart from that rule; it follows the assets repo to a NEWER state of the same file.
        {{"4.0", "Kirara", "tex_face_diffuse"}, "6eb20522"},
        // Klee
        {{"4.0", "Klee", "draw_vb"}, "52469e36"},
        {{"4.0", "Klee", "position_vb"}, "dcd74904"},
        {{"4.0", "Klee", "blend_vb"}, "aec1d55e"},
        {{"4.0", "Klee", "texcoord_vb"}, "c3448489"},
        {{"4.0", "Klee", "ib"}, "3fe81b2a"},
        {{"4.0", "Klee", "tex_head_diffuse"}, "76672fcd"},
        {{"4.0", "Klee", "tex_head_lightmap"}, "5f5c4d6e"},
        {{"4.0", "Klee", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Klee", "tex_body_diffuse"}, "20530946"},
        {{"4.0", "Klee", "tex_body_lightmap"}, "a0d91469"},
        {{"4.0", "Klee", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Klee", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Klee", "tex_face_diffuse"}, "7fccd783"},
        {{"4.0", "Klee", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Klee", "tex_face_shadow"}, "380a1467"},
        {{"4.0", "Klee", "tex_face_shadowramp"}, "7eb5b84e"},
        // KleeBlossomingStarlight
        {{"4.0", "KleeBlossomingStarlight", "draw_vb"}, "6234ae22"},
        {{"4.0", "KleeBlossomingStarlight", "position_vb"}, "0f5fedb4"},
        {{"4.0", "KleeBlossomingStarlight", "blend_vb"}, "652497c2"},
        {{"4.0", "KleeBlossomingStarlight", "texcoord_vb"}, "4d6c496b"},
        {{"4.0", "KleeBlossomingStarlight", "ib"}, "742c4ed5"},
        {{"4.0", "KleeBlossomingStarlight", "tex_head_diffuse"}, "b0929218"},
        {{"4.0", "KleeBlossomingStarlight", "tex_head_lightmap"}, "db315fa4"},
        {{"4.0", "KleeBlossomingStarlight", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "KleeBlossomingStarlight", "tex_head_shadowramp"}, "58d2635b"},
        {{"4.0", "KleeBlossomingStarlight", "tex_body_diffuse"}, "27a8989a"},
        {{"4.0", "KleeBlossomingStarlight", "tex_body_lightmap"}, "c085a587"},
        {{"4.0", "KleeBlossomingStarlight", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "KleeBlossomingStarlight", "tex_body_shadowramp"}, "58d2635b"},
        {{"4.0", "KleeBlossomingStarlight", "tex_dress_diffuse"}, "b0929218"},
        {{"4.0", "KleeBlossomingStarlight", "tex_dress_lightmap"}, "db315fa4"},
        {{"4.0", "KleeBlossomingStarlight", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "KleeBlossomingStarlight", "tex_dress_shadowramp"}, "58d2635b"},
        // Lisa
        {{"4.0", "Lisa", "position_vb"}, "2a557add"},
        {{"4.0", "Lisa", "blend_vb"}, "8bfa989d"},
        {{"4.0", "Lisa", "texcoord_vb"}, "92b87c71"},
        // LisaStudent
        // HER FACE DIFFUSE, likewise missing and likewise confirmed upstream:
        // GI-Model-Importer-Assets/PlayerCharacterData/LisaStudent gives Face Diffuse = 95cb454c.
        //
        // KleeBlossomingStarlight, in this same batch, is the one that stays empty: she has NO
        // Face component in the assets repo at all, so there is nothing to confirm against and
        // inferring one from Klee is the guess this table's policy forbids. Her generated face
        // section therefore still carries a register and no hash -- see NilouBreeze's row for the
        // probe showing what that does and does not cost.
        {{"4.0", "LisaStudent", "tex_face_diffuse"}, "95cb454c"},
        {{"4.0", "LisaStudent", "draw_vb"}, "362fb2b3"},
        {{"4.0", "LisaStudent", "position_vb"}, "37c70461"},
        {{"4.0", "LisaStudent", "blend_vb"}, "5db2f8f4"},
        {{"4.0", "LisaStudent", "texcoord_vb"}, "d77ffc4f"},
        {{"4.0", "LisaStudent", "ib"}, "cbda8639"},
        {{"4.0", "LisaStudent", "tex_head_normalmap"}, "438c9349"},
        {{"4.0", "LisaStudent", "tex_head_diffuse"}, "f7a42411"},
        {{"4.0", "LisaStudent", "tex_head_lightmap"}, "040d3ada"},
        {{"4.0", "LisaStudent", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "LisaStudent", "tex_body_normalmap"}, "35136f7b"},
        {{"4.0", "LisaStudent", "tex_body_diffuse"}, "02cb9df7"},
        {{"4.0", "LisaStudent", "tex_body_lightmap"}, "cbf77c41"},
        {{"4.0", "LisaStudent", "tex_body_metalmap"}, "b0e08915"},
        // Mona
        {{"4.0", "Mona", "draw_vb"}, "00741928"},
        {{"4.0", "Mona", "position_vb"}, "20d0bfab"},
        {{"4.0", "Mona", "blend_vb"}, "52f0e9a0"},
        {{"4.0", "Mona", "texcoord_vb"}, "a8191396"},
        {{"4.0", "Mona", "ib"}, "ef876207"},
        {{"4.0", "Mona", "tex_head_diffuse"}, "b518c5a5"},
        {{"4.0", "Mona", "tex_head_lightmap"}, "0c679d22"},
        {{"4.0", "Mona", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Mona", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Mona", "tex_body_diffuse"}, "5f873d89"},
        {{"4.0", "Mona", "tex_body_lightmap"}, "29d50a21"},
        {{"4.0", "Mona", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Mona", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Mona", "tex_face_diffuse"}, "8e116301"},
        {{"4.0", "Mona", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Mona", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Mona", "tex_face_shadowramp"}, "7eb5b84e"},
        // MonaCN
        {{"4.0", "MonaCN", "draw_vb"}, "41f18240"},
        {{"4.0", "MonaCN", "position_vb"}, "ee5ed1dc"},
        {{"4.0", "MonaCN", "blend_vb"}, "bad2731b"},
        {{"4.0", "MonaCN", "texcoord_vb"}, "e543af5d"},
        {{"4.0", "MonaCN", "ib"}, "ed79ea5b"},
        {{"4.0", "MonaCN", "tex_head_diffuse"}, "0320a4d2"},
        {{"4.0", "MonaCN", "tex_head_lightmap"}, "df0f8b90"},
        {{"4.0", "MonaCN", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "MonaCN", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "MonaCN", "tex_body_diffuse"}, "c043f913"},
        {{"4.0", "MonaCN", "tex_body_lightmap"}, "a3369d08"},
        {{"4.0", "MonaCN", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "MonaCN", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "MonaCN", "tex_face_diffuse"}, "8e116301"},
        {{"4.0", "MonaCN", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "MonaCN", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "MonaCN", "tex_face_shadowramp"}, "7eb5b84e"},
        // Nilou
        {{"4.0", "Nilou", "draw_vb"}, "2f95abf6"},
        {{"4.0", "Nilou", "position_vb"}, "b2acc1df"},
        {{"4.0", "Nilou", "blend_vb"}, "fda8e783"},
        {{"4.0", "Nilou", "texcoord_vb"}, "583fba29"},
        {{"4.0", "Nilou", "ib"}, "265e34e3"},
        {{"4.0", "Nilou", "tex_head_normalmap"}, "6f0680d3"},
        {{"4.0", "Nilou", "tex_head_diffuse"}, "9caa70ad"},
        {{"4.0", "Nilou", "tex_head_lightmap"}, "b2501b97"},
        {{"4.0", "Nilou", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Nilou", "tex_body_normalmap"}, "a87ce1c0"},
        {{"4.0", "Nilou", "tex_body_diffuse"}, "91cb97a8"},
        {{"4.0", "Nilou", "tex_body_lightmap"}, "29cf0914"},
        {{"4.0", "Nilou", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Nilou", "tex_dress_normalmap"}, "a87ce1c0"},
        {{"4.0", "Nilou", "tex_dress_diffuse"}, "91cb97a8"},

        // WAS "29cf09   14" -- three spaces inside the hex, so it matched no resource and Nilou's
        // dress lightmap was silently never remapped (repaired 2026-09-11). The same typo is in the
        // pure-Python source this table was generated from (FixRaidenBoss6.py line 4992), so we
        // carried it faithfully, which is exactly why an A/B could never report it: both scripts
        // agreed, and both did nothing.
        //
        // The replacement is not a guess. GI-Model-Importer-Assets/PlayerCharacterData/Nilou's
        // hash.json gives the Dress component LightMap = 29cf0914, and Nilou's own tex_body_lightmap
        // two lines up is already 29cf0914 -- her body and dress share every other texture too.
        {{"4.0", "Nilou", "tex_dress_lightmap"}, "29cf0914"},
        {{"4.0", "Nilou", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Nilou", "tex_face_diffuse"}, "0957b10f"},
        {{"4.0", "Nilou", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Nilou", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Nilou", "tex_face_metalmap"}, "b0e08915"},
        // Ningguang
        {{"4.0", "Ningguang", "draw_vb"}, "e4fc5902"},
        {{"4.0", "Ningguang", "position_vb"}, "55b43e99"},
        {{"4.0", "Ningguang", "blend_vb"}, "9f7dc19c"},
        {{"4.0", "Ningguang", "texcoord_vb"}, "906ad233"},
        {{"4.0", "Ningguang", "ib"}, "93085db7"},
        {{"4.0", "Ningguang", "tex_head_diffuse"}, "e0789f0d"},
        {{"4.0", "Ningguang", "tex_head_lightmap"}, "5d182ae7"},
        {{"4.0", "Ningguang", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ningguang", "tex_body_diffuse"}, "5ffe95c2"},
        {{"4.0", "Ningguang", "tex_body_lightmap"}, "64e6b893"},
        {{"4.0", "Ningguang", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Ningguang", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ningguang", "tex_dress_diffuse"}, "5ffe95c2"},
        {{"4.0", "Ningguang", "tex_dress_lightmap"}, "64e6b893"},
        {{"4.0", "Ningguang", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Ningguang", "tex_face_diffuse"}, "4cc85338"},
        {{"4.0", "Ningguang", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Ningguang", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Ningguang", "tex_face_shadowramp"}, "7eb5b84e"},
        // NingguangOrchid
        {{"4.0", "NingguangOrchid", "draw_vb"}, "10de9c78"},
        {{"4.0", "NingguangOrchid", "position_vb"}, "db37b198"},
        {{"4.0", "NingguangOrchid", "blend_vb"}, "a8246d4a"},
        {{"4.0", "NingguangOrchid", "texcoord_vb"}, "396aa3ec"},
        {{"4.0", "NingguangOrchid", "ib"}, "f1d09b47"},
        {{"4.0", "NingguangOrchid", "tex_head_diffuse"}, "b68d7488"},
        {{"4.0", "NingguangOrchid", "tex_head_lightmap"}, "bc1034dd"},
        {{"4.0", "NingguangOrchid", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "NingguangOrchid", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "NingguangOrchid", "tex_body_diffuse"}, "a4597b85"},
        {{"4.0", "NingguangOrchid", "tex_body_lightmap"}, "0e26784e"},
        {{"4.0", "NingguangOrchid", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "NingguangOrchid", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "NingguangOrchid", "tex_dress_diffuse"}, "a4597b85"},
        {{"4.0", "NingguangOrchid", "tex_dress_lightmap"}, "0e26784e"},
        {{"4.0", "NingguangOrchid", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "NingguangOrchid", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "NingguangOrchid", "tex_face_diffuse"}, "4cc85338"},
        {{"4.0", "NingguangOrchid", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "NingguangOrchid", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "NingguangOrchid", "tex_face_shadowramp"}, "7eb5b84e"},
        // Raiden
        {{"4.0", "Raiden", "draw_vb"}, "a05e7bec"},
        {{"4.0", "Raiden", "position_vb"}, "e48c61f3"},
        {{"4.0", "Raiden", "blend_vb"}, "1a495487"},
        {{"4.0", "Raiden", "texcoord_vb"}, "0c37fc86"},
        {{"4.0", "Raiden", "ib"}, "428c56cd"},
        {{"4.0", "Raiden", "tex_head_diffuse"}, "877ea73c"},
        {{"4.0", "Raiden", "tex_head_lightmap"}, "90ffd990"},
        {{"4.0", "Raiden", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Raiden", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Raiden", "tex_body_diffuse"}, "9b5d87e0"},
        {{"4.0", "Raiden", "tex_body_lightmap"}, "452e0279"},
        {{"4.0", "Raiden", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Raiden", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Raiden", "tex_dress_diffuse"}, "9b5d87e0"},
        {{"4.0", "Raiden", "tex_dress_lightmap"}, "452e0279"},
        {{"4.0", "Raiden", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Raiden", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Raiden", "tex_face_diffuse"}, "20174ee8"},
        {{"4.0", "Raiden", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Raiden", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Raiden", "tex_face_shadowramp"}, "7eb5b84e"},
        // RaidenBoss
        {{"4.0", "RaidenBoss", "blend_vb"}, "fe5c0180"},
        // Rosaria
        {{"4.0", "Rosaria", "draw_vb"}, "9e1868d9"},
        {{"4.0", "Rosaria", "position_vb"}, "748f40a5"},
        {{"4.0", "Rosaria", "blend_vb"}, "4de959bd"},
        {{"4.0", "Rosaria", "texcoord_vb"}, "06b8fbf5"},
        {{"4.0", "Rosaria", "ib"}, "5d18b9d6"},
        {{"4.0", "Rosaria", "tex_head_diffuse"}, "81b2d0ca"},
        {{"4.0", "Rosaria", "tex_head_lightmap"}, "2f19c547"},
        {{"4.0", "Rosaria", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Rosaria", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Rosaria", "tex_body_diffuse"}, "9abde85f"},
        {{"4.0", "Rosaria", "tex_body_lightmap"}, "743ffc09"},
        {{"4.0", "Rosaria", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Rosaria", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Rosaria", "tex_dress_diffuse"}, "81b2d0ca"},
        {{"4.0", "Rosaria", "tex_dress_lightmap"}, "2f19c547"},
        {{"4.0", "Rosaria", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Rosaria", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Rosaria", "tex_extra_diffuse"}, "9abde85f"},
        {{"4.0", "Rosaria", "tex_extra_lightmap"}, "743ffc09"},
        {{"4.0", "Rosaria", "tex_extra_metalmap"}, "b0e08915"},
        {{"4.0", "Rosaria", "tex_extra_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Rosaria", "tex_face_diffuse"}, "2abd61ee"},
        {{"4.0", "Rosaria", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Rosaria", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Rosaria", "tex_face_shadowramp"}, "7eb5b84e"},
        // RosariaCN
        {{"4.0", "RosariaCN", "draw_vb"}, "f3d4a01a"},
        {{"4.0", "RosariaCN", "position_vb"}, "59a1f8b1"},
        {{"4.0", "RosariaCN", "blend_vb"}, "a7bee046"},
        {{"4.0", "RosariaCN", "texcoord_vb"}, "86e0d16b"},
        {{"4.0", "RosariaCN", "ib"}, "851e4de1"},
        {{"4.0", "RosariaCN", "tex_head_diffuse"}, "55280cb0"},
        {{"4.0", "RosariaCN", "tex_head_lightmap"}, "825c32a0"},
        {{"4.0", "RosariaCN", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "RosariaCN", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "RosariaCN", "tex_body_diffuse"}, "bd6fcf34"},
        {{"4.0", "RosariaCN", "tex_body_lightmap"}, "cf7b6deb"},
        {{"4.0", "RosariaCN", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "RosariaCN", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "RosariaCN", "tex_dress_diffuse"}, "55280cb0"},
        {{"4.0", "RosariaCN", "tex_dress_lightmap"}, "825c32a0"},
        {{"4.0", "RosariaCN", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "RosariaCN", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "RosariaCN", "tex_extra_diffuse"}, "bd6fcf34"},
        {{"4.0", "RosariaCN", "tex_extra_lightmap"}, "cf7b6deb"},
        {{"4.0", "RosariaCN", "tex_extra_metalmap"}, "b0e08915"},
        {{"4.0", "RosariaCN", "tex_extra_shadowramp"}, "7eb5b84e"},
        {{"4.0", "RosariaCN", "tex_face_diffuse"}, "2abd61ee"},
        {{"4.0", "RosariaCN", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "RosariaCN", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "RosariaCN", "tex_face_shadowramp"}, "7eb5b84e"},
        // Shenhe
        {{"4.0", "Shenhe", "draw_vb"}, "fde191d7"},
        {{"4.0", "Shenhe", "position_vb"}, "e44b58b5"},
        {{"4.0", "Shenhe", "blend_vb"}, "541cf273"},
        {{"4.0", "Shenhe", "texcoord_vb"}, "86c4f5ec"},
        {{"4.0", "Shenhe", "ib"}, "0b7d4e4d"},
        {{"4.0", "Shenhe", "tex_head_diffuse"}, "7da9c07b"},
        {{"4.0", "Shenhe", "tex_head_lightmap"}, "e134c758"},
        {{"4.0", "Shenhe", "tex_head_metalmap"}, "b0e08915"},
        {{"4.0", "Shenhe", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Shenhe", "tex_body_diffuse"}, "cba1d6ec"},
        {{"4.0", "Shenhe", "tex_body_lightmap"}, "ce5176af"},
        {{"4.0", "Shenhe", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Shenhe", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Shenhe", "tex_dress_diffuse"}, "cba1d6ec"},
        {{"4.0", "Shenhe", "tex_dress_lightmap"}, "ce5176af"},
        {{"4.0", "Shenhe", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.0", "Shenhe", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Shenhe", "tex_face_diffuse"}, "f5f393cb"},
        {{"4.0", "Shenhe", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Shenhe", "tex_face_shadow"}, "bf9fccca"},
        {{"4.0", "Shenhe", "tex_face_shadowramp"}, "7eb5b84e"},
        // Xiangling
        {{"4.0", "Xiangling", "draw_vb"}, "94523bab"},
        {{"4.0", "Xiangling", "position_vb"}, "9427917d"},
        {{"4.0", "Xiangling", "blend_vb"}, "2b663556"},
        {{"4.0", "Xiangling", "texcoord_vb"}, "29744879"},
        {{"4.0", "Xiangling", "ib"}, "5363ff5d"},
        {{"4.0", "Xiangling", "tex_head_diffuse"}, "499e89bf"},
        {{"4.0", "Xiangling", "tex_head_lightmap"}, "ba6a810c"},
        {{"4.0", "Xiangling", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Xiangling", "tex_body_diffuse"}, "abd3a28e"},
        {{"4.0", "Xiangling", "tex_body_lightmap"}, "7e12708e"},
        {{"4.0", "Xiangling", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Xiangling", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Xiangling", "tex_dress_diffuse"}, "abd3a28e"},
        {{"4.0", "Xiangling", "tex_dress_lightmap"}, "7e12708e"},
        {{"4.0", "Xiangling", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Xiangling", "tex_face_diffuse"}, "1d353f0b"},
        {{"4.0", "Xiangling", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Xiangling", "tex_face_shadow"}, "3f396398"},
        {{"4.0", "Xiangling", "tex_face_shadowramp"}, "7eb5b84e"},
        // Xingqiu
        {{"4.0", "Xingqiu", "draw_vb"}, "f9caefa2"},
        {{"4.0", "Xingqiu", "position_vb"}, "25aed172"},
        {{"4.0", "Xingqiu", "blend_vb"}, "8f0e9948"},
        {{"4.0", "Xingqiu", "texcoord_vb"}, "4c25bd5f"},
        {{"4.0", "Xingqiu", "ib"}, "ba1d11c3"},
        {{"4.0", "Xingqiu", "tex_head_diffuse"}, "d8e82984"},
        {{"4.0", "Xingqiu", "tex_head_lightmap"}, "69866236"},
        {{"4.0", "Xingqiu", "tex_head_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Xingqiu", "tex_body_diffuse"}, "5423a093"},
        {{"4.0", "Xingqiu", "tex_body_lightmap"}, "0f394cb6"},
        {{"4.0", "Xingqiu", "tex_body_metalmap"}, "b0e08915"},
        {{"4.0", "Xingqiu", "tex_body_shadowramp"}, "7eb5b84e"},
        {{"4.0", "Xingqiu", "tex_face_diffuse"}, "e35f8bf8"},
        {{"4.0", "Xingqiu", "tex_face_lightmap"}, "4e3376db"},
        {{"4.0", "Xingqiu", "tex_face_shadow"}, "59df7508"},
        {{"4.0", "Xingqiu", "tex_face_shadowramp"}, "7eb5b84e"},

        // ===== version 4.1 =====
        // Amber
        {{"4.1", "Amber", "draw_vb"}, "0eef5bbe"},
        // AmberCN
        {{"4.1", "AmberCN", "draw_vb"}, "53eff008"},
        // Ayaka
        {{"4.1", "Ayaka", "draw_vb"}, "9d2c1c5d"},
        // AyakaSpringBloom
        {{"4.1", "AyakaSpringBloom", "draw_vb"}, "04f21fa3"},
        // Barbara
        {{"4.1", "Barbara", "draw_vb"}, "7df968e8"},
        // BarbaraSummertime
        {{"4.1", "BarbaraSummertime", "draw_vb"}, "e9199599"},
        // Diluc
        {{"4.1", "Diluc", "draw_vb"}, "dff0b253"},
        // DilucFlamme
        {{"4.1", "DilucFlamme", "draw_vb"}, "274e5c1a"},
        // Fischl
        {{"4.1", "Fischl", "draw_vb"}, "e5ac321c"},
        // FischlHighness
        {{"4.1", "FischlHighness", "draw_vb"}, "b52dd70c"},
        // Ganyu
        {{"4.1", "Ganyu", "draw_vb"}, "fbf98643"},
        // HuTao
        {{"4.1", "HuTao", "draw_vb"}, "e9d17db6"},
        // Jean
        {{"4.1", "Jean", "draw_vb"}, "6fe07e12"},
        // JeanCN
        {{"4.1", "JeanCN", "draw_vb"}, "a3cccc14"},
        // JeanSea
        {{"4.1", "JeanSea", "draw_vb"}, "1ec879c9"},
        // Kaeya
        {{"4.1", "Kaeya", "draw_vb"}, "c2ef8845"},
        // Keqing
        {{"4.1", "Keqing", "draw_vb"}, "ccc33b79"},
        // KeqingOpulent
        {{"4.1", "KeqingOpulent", "draw_vb"}, "6629a84e"},
        // Kirara
        {{"4.1", "Kirara", "draw_vb"}, "6fb396da"},
        // Klee
        {{"4.1", "Klee", "draw_vb"}, "dba3b111"},
        // KleeBlossomingStarlight
        {{"4.1", "KleeBlossomingStarlight", "draw_vb"}, "ebd18105"},
        // Lisa
        {{"4.1", "Lisa", "draw_vb"}, "e6af2c6d"},
        // LisaStudent
        {{"4.1", "LisaStudent", "draw_vb"}, "bfca9d94"},
        // Mona
        {{"4.1", "Mona", "draw_vb"}, "8991360f"},
        // MonaCN
        {{"4.1", "MonaCN", "draw_vb"}, "c814ad67"},
        // Nilou
        {{"4.1", "Nilou", "draw_vb"}, "a67084d1"},
        // Ningguang
        {{"4.1", "Ningguang", "draw_vb"}, "6d197625"},
        // NingguangOrchid
        {{"4.1", "NingguangOrchid", "draw_vb"}, "993bb35f"},
        // Raiden
        {{"4.1", "Raiden", "draw_vb"}, "29bb54cb"},
        // Rosaria
        {{"4.1", "Rosaria", "draw_vb"}, "17fd47fe"},
        // RosariaCN
        {{"4.1", "RosariaCN", "draw_vb"}, "7a318f3d"},
        // Shenhe
        {{"4.1", "Shenhe", "draw_vb"}, "7404bef0"},
        // Xingqiu
        {{"4.1", "Xingqiu", "draw_vb"}, "702fc085"},
        // Xiangling
        {{"4.1", "Xiangling", "draw_vb"}, "1db7148c"},

        // ===== version 4.3 =====
        // Amber
        {{"4.3", "Amber", "ib"}, "a1a2bbfb"},
        // AmberCN
        {{"4.3", "AmberCN", "ib"}, "b41d4d94"},
        // Ayaka
        {{"4.3", "Ayaka", "ib"}, "0cafd227"},
        // Barbara
        {{"4.3", "Barbara", "ib"}, "1bc3490d"},
        // BarbaraSummertime
        {{"4.3", "BarbaraSummertime", "ib"}, "9cc5a563"},
        // Diluc
        {{"4.3", "Diluc", "ib"}, "e9786c58"},
        // DilucFlamme
        {{"4.3", "DilucFlamme", "ib"}, "a5323853"},
        // Fischl
        {{"4.3", "Fischl", "ib"}, "6428104d"},
        // FischlHighness
        {{"4.3", "FischlHighness", "ib"}, "ad6be7a1"},
        // Ganyu
        {{"4.3", "Ganyu", "ib"}, "1575ec63"},
        // HuTao
        {{"4.3", "HuTao", "ib"}, "3de1efe2"},
        // Jean
        {{"4.3", "Jean", "ib"}, "115737ff"},
        // JeanCN
        {{"4.3", "JeanCN", "ib"}, "aad861e0"},
        // JeanSea
        {{"4.3", "JeanSea", "ib"}, "69c0c24e"},
        // Kaeya
        {{"4.3", "Kaeya", "ib"}, "2b3f575a"},
        // Keqing
        {{"4.3", "Keqing", "ib"}, "cbf1894b"},
        // KeqingOpulent
        {{"4.3", "KeqingOpulent", "ib"}, "7c6fc8c3"},
        // Kirara
        {{"4.3", "Kirara", "ib"}, "f6e9af7d"},
        // Klee
        {{"4.3", "Klee", "ib"}, "073c71f5"},
        // KleeBlossomingStarlight
        {{"4.3", "KleeBlossomingStarlight", "ib"}, "4cf8240a"},
        // Lisa
        {{"4.3", "Lisa", "ib"}, "518a6840"},
        // LisaStudent
        {{"4.3", "LisaStudent", "ib"}, "f30eece6"},
        // Mona
        {{"4.3", "Mona", "ib"}, "d75308d8"},
        // MonaCN
        {{"4.3", "MonaCN", "ib"}, "d5ad8084"},
        // Nilou
        {{"4.3", "Nilou", "ib"}, "1e8a5e3c"},
        // Ningguang
        {{"4.3", "Ningguang", "ib"}, "abdc3768"},
        // NingguangOrchid
        {{"4.3", "NingguangOrchid", "ib"}, "c904f198"},
        // Raiden
        {{"4.3", "Raiden", "ib"}, "7a583c12"},
        // Rosaria
        {{"4.3", "Rosaria", "ib"}, "65ccd309"},
        // RosariaCN
        {{"4.3", "RosariaCN", "ib"}, "bdca273e"},
        // Shenhe
        {{"4.3", "Shenhe", "ib"}, "33a92492"},
        // Xingqiu
        {{"4.3", "Xingqiu", "ib"}, "82c97b1c"},
        // Xiangling
        {{"4.3", "Xiangling", "ib"}, "6bb79582"},

        // ===== version 4.4 =====
        // Amber
        {{"4.4", "Amber", "position_vb"}, "a2ea4b2d"},
        {{"4.4", "Amber", "blend_vb"}, "36d20a67"},
        {{"4.4", "Amber", "texcoord_vb"}, "81b777ca"},
        {{"4.4", "Amber", "ib"}, "b03c7e30"},
        // AmberCN
        {{"4.4", "AmberCN", "position_vb"}, "557b2eff"},
        // Diluc
        {{"4.4", "Diluc", "draw_vb"}, "5b0cb984"},
        {{"4.4", "Diluc", "position_vb"}, "71625c4d"},
        {{"4.4", "Diluc", "blend_vb"}, "afb527f6"},
        {{"4.4", "Diluc", "texcoord_vb"}, "6d0e22f0"},
        {{"4.4", "Diluc", "ib"}, "e16fa548"},
        // Fischl
        {{"4.4", "Fischl", "position_vb"}, "bf6aef4d"},
        // Mona
        {{"4.4", "Mona", "position_vb"}, "7a1dc890"},
        {{"4.4", "Mona", "blend_vb"}, "b043715a"},
        // MonaCN
        {{"4.4", "MonaCN", "position_vb"}, "515f3ce6"},
        // Ningguang
        {{"4.4", "Ningguang", "draw_vb"}, "4c2f9a0a"},
        {{"4.4", "Ningguang", "position_vb"}, "f9e1b52b"},
        {{"4.4", "Ningguang", "blend_vb"}, "735eaea4"},
        {{"4.4", "Ningguang", "texcoord_vb"}, "1f0ab400"},
        {{"4.4", "Ningguang", "ib"}, "ad75352c"},
        // ShenheFrostFlower
        {{"4.4", "ShenheFrostFlower", "draw_vb"}, "6102c3ef"},
        {{"4.4", "ShenheFrostFlower", "position_vb"}, "ee0980eb"},
        {{"4.4", "ShenheFrostFlower", "blend_vb"}, "263019b8"},
        {{"4.4", "ShenheFrostFlower", "texcoord_vb"}, "d36f368d"},
        {{"4.4", "ShenheFrostFlower", "ib"}, "83a9116d"},
        {{"4.4", "ShenheFrostFlower", "tex_head_normalmap"}, "4e5d638d"},
        {{"4.4", "ShenheFrostFlower", "tex_head_diffuse"}, "1ab2f510"},
        {{"4.4", "ShenheFrostFlower", "tex_head_lightmap"}, "b0e08915"},
        {{"4.4", "ShenheFrostFlower", "tex_head_shadowramp"}, "58d2635b"},
        {{"4.4", "ShenheFrostFlower", "tex_body_normalmap"}, "625d0bb4"},
        {{"4.4", "ShenheFrostFlower", "tex_body_diffuse"}, "51529edd"},
        {{"4.4", "ShenheFrostFlower", "tex_body_lightmap"}, "b0e08915"},
        {{"4.4", "ShenheFrostFlower", "tex_body_shadowramp"}, "58d2635b"},
        {{"4.4", "ShenheFrostFlower", "tex_dress_normalmap"}, "4e5d638d"},
        {{"4.4", "ShenheFrostFlower", "tex_dress_diffuse"}, "1ab2f510"},
        {{"4.4", "ShenheFrostFlower", "tex_dress_lightmap"}, "7eb5b84e"},
        {{"4.4", "ShenheFrostFlower", "tex_dress_shadowramp"}, "000050-ps-t3"},
        {{"4.4", "ShenheFrostFlower", "tex_extra_normalmap"}, "625d0bb4"},
        {{"4.4", "ShenheFrostFlower", "tex_extra_diffuse"}, "51529edd"},
        {{"4.4", "ShenheFrostFlower", "tex_extra_lightmap"}, "7eb5b84e"},
        {{"4.4", "ShenheFrostFlower", "tex_extra_shadowramp"}, "000049-ps-t3"},
        // GanyuTwilight
        {{"4.4", "GanyuTwilight", "draw_vb"}, "1ad9c181"},
        {{"4.4", "GanyuTwilight", "position_vb"}, "9b3f356e"},
        {{"4.4", "GanyuTwilight", "blend_vb"}, "9a5c01d2"},
        {{"4.4", "GanyuTwilight", "texcoord_vb"}, "5ff2f1d1"},
        {{"4.4", "GanyuTwilight", "ib"}, "cb283c86"},
        {{"4.4", "GanyuTwilight", "tex_head_normalmap"}, "f8aa8a9d"},
        {{"4.4", "GanyuTwilight", "tex_head_diffuse"}, "ad1ed796"},
        {{"4.4", "GanyuTwilight", "tex_head_lightmap"}, "191ebe05"},
        {{"4.4", "GanyuTwilight", "tex_head_metalmap"}, "b0e08915"},
        {{"4.4", "GanyuTwilight", "tex_body_normalmap"}, "e304bdcf"},
        {{"4.4", "GanyuTwilight", "tex_body_diffuse"}, "13fa0b53"},
        {{"4.4", "GanyuTwilight", "tex_body_lightmap"}, "b0e08915"},
        {{"4.4", "GanyuTwilight", "tex_body_shadowramp"}, "58d2635b"},
        {{"4.4", "GanyuTwilight", "tex_dress_normalmap"}, "e304bdcf"},
        {{"4.4", "GanyuTwilight", "tex_dress_diffuse"}, "13fa0b53"},

        // WAS "b0e089    15" -- the same class of typo as Nilou's above, four spaces this time,
        // and likewise present in the pure-Python original (FixRaidenBoss6.py line 5122).
        //
        // The value looks wrong even repaired, because b0e08915 is the metalmap hash half this table
        // shares. It is not ours to argue with: GI-Model-Importer-Assets/PlayerCharacterData/
        // GanyuTwilight's hash.json really does list LightMap = b0e08915 for BOTH her body and her
        // dress, and her tex_body_lightmap four lines up already carries it. This is precisely the
        // "follow the assets repo even where it is known to be wrong" case from the header -- the
        // typo is ours to fix, the oddity is not.
        {{"4.4", "GanyuTwilight", "tex_dress_lightmap"}, "b0e08915"},
        {{"4.4", "GanyuTwilight", "tex_dress_shadowramp"}, "58d2635b"},
        // Kirara
        {{"4.4", "Kirara", "position_vb"}, "b57d7fe2"},
        // XingqiuBamboo
        {{"4.4", "XingqiuBamboo", "draw_vb"}, "3944dfbd"},
        {{"4.4", "XingqiuBamboo", "position_vb"}, "cc158a1e"},
        {{"4.4", "XingqiuBamboo", "blend_vb"}, "07c7c5b6"},
        {{"4.4", "XingqiuBamboo", "texcoord_vb"}, "41426386"},
        {{"4.4", "XingqiuBamboo", "ib"}, "76df4025"},
        {{"4.4", "XingqiuBamboo", "tex_head_diffuse"}, "520d5504"},
        {{"4.4", "XingqiuBamboo", "tex_head_lightmap"}, "045244a0"},
        {{"4.4", "XingqiuBamboo", "tex_head_metalmap"}, "b0e08915"},
        {{"4.4", "XingqiuBamboo", "tex_head_shadowramp"}, "58d2635b"},
        {{"4.4", "XingqiuBamboo", "tex_body_diffuse"}, "fe820a09"},
        {{"4.4", "XingqiuBamboo", "tex_body_lightmap"}, "65bb84d8"},
        {{"4.4", "XingqiuBamboo", "tex_body_metalmap"}, "b0e08915"},
        {{"4.4", "XingqiuBamboo", "tex_body_shadowramp"}, "58d2635b"},
        {{"4.4", "XingqiuBamboo", "tex_dress_diffuse"}, "fe820a09"},
        {{"4.4", "XingqiuBamboo", "tex_dress_lightmap"}, "65bb84d8"},
        {{"4.4", "XingqiuBamboo", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.4", "XingqiuBamboo", "tex_dress_shadowramp"}, "58d2635b"},

        // ===== version 4.6 =====
        // Arlecchino
        {{"4.6", "Arlecchino", "draw_vb"}, "44e3487a"},
        {{"4.6", "Arlecchino", "position_vb"}, "6895f405"},
        {{"4.6", "Arlecchino", "blend_vb"}, "e211de60"},
        {{"4.6", "Arlecchino", "texcoord_vb"}, "8b17a419"},
        {{"4.6", "Arlecchino", "ib"}, "e811d2a1"},
        // ArlecchinoBoss
        {{"4.6", "ArlecchinoBoss", "draw_vb"}, "970e7336"},
        {{"4.6", "ArlecchinoBoss", "position_vb"}, "cf66bef6"},
        {{"4.6", "ArlecchinoBoss", "blend_vb"}, "5227c79e"},
        {{"4.6", "ArlecchinoBoss", "texcoord_vb"}, "a75e7052"},
        {{"4.6", "ArlecchinoBoss", "ib"}, "480f1267"},

        // ===== version 4.8 =====
        // NilouBreeze
        {{"4.8", "NilouBreeze", "draw_vb"}, "3f79fabb"},
        {{"4.8", "NilouBreeze", "position_vb"}, "7d53d78f"},
        {{"4.8", "NilouBreeze", "blend_vb"}, "49bede49"},
        {{"4.8", "NilouBreeze", "texcoord_vb"}, "b976b848"},
        {{"4.8", "NilouBreeze", "ib"}, "00439fbb"},
        {{"4.8", "NilouBreeze", "tex_head_diffuse"}, "2593dea6"},
        {{"4.8", "NilouBreeze", "tex_head_lightmap"}, "3f78afbf"},
        {{"4.8", "NilouBreeze", "tex_head_metalmap"}, "b0e08915"},
        {{"4.8", "NilouBreeze", "tex_head_shadowramp"}, "58d2635b"},
        {{"4.8", "NilouBreeze", "tex_body_diffuse"}, "9f7e392b"},
        {{"4.8", "NilouBreeze", "tex_body_lightmap"}, "e3e73b29"},
        {{"4.8", "NilouBreeze", "tex_body_metalmap"}, "b0e08915"},
        {{"4.8", "NilouBreeze", "tex_body_shadowramp"}, "58d2635b"},
        {{"4.8", "NilouBreeze", "tex_dress_diffuse"}, "9f7e392b"},
        {{"4.8", "NilouBreeze", "tex_dress_lightmap"}, "e3e73b29"},
        {{"4.8", "NilouBreeze", "tex_dress_metalmap"}, "b0e08915"},
        {{"4.8", "NilouBreeze", "tex_dress_shadowramp"}, "58d2635b"},

        // HER FACE DIFFUSE, and the one row in this table whose value is NOT evidenced (added
        // 2026-09-11). There is no GI-Model-Importer-Assets folder for NilouBreeze, the
        // pure-Python table has no row, and the NilouBreeze mod on hand has no face section to
        // read one off. 0957b10f is Nilou's own, on the pattern that seven of the eight base/skin
        // pairs carrying both rows share a face diffuse -- a hint, not a source, and
        // Keqing/KeqingOpulent (d8c9c399 vs c2b17f84) are the eighth.
        //
        // HOW MUCH THAT UNCERTAINTY COSTS IS ASYMMETRIC, and it was measured rather than
        // reasoned. Setting this row to "deadbeef" and running NilouBreeze -> Nilou emits
        //
        //     [TextureOverrideNilouBreezeFaceNilouRemapFix]
        //     hash = 0957b10f
        //
        // -- Nilou's real hash, not the nonsense. The source row is only a SEED: the invented
        // section is given the source's value so RegAssetRemap has something to replace, and what
        // reaches the .ini is always the target's. So in this direction the row only has to
        // EXIST, and without one there was nothing to seed, nothing to replace, and a face
        // section with a register and no hash that matched no draw call at all.
        //
        // The value itself only reaches output in the OTHER direction, Nilou -> NilouBreeze, where
        // it is the target. That direction is unconfirmed -- no NilouBreeze mod here to test it,
        // and no dump to check it against. If her face ever renders wrong that way round, this
        // row is the first thing to doubt.
        {{"4.8", "NilouBreeze", "tex_face_diffuse"}, "0957b10f"},
        // KiraraBoots
        {{"4.8", "KiraraBoots", "draw_vb"}, "4955fc99"},
        {{"4.8", "KiraraBoots", "position_vb"}, "f8013ba9"},
        {{"4.8", "KiraraBoots", "blend_vb"}, "53a2502b"},
        {{"4.8", "KiraraBoots", "texcoord_vb"}, "596e8fe0"},
        {{"4.8", "KiraraBoots", "ib"}, "846979e2"},
        {{"4.8", "KiraraBoots", "tex_head_normalmap"}, "c715bcf7"},
        {{"4.8", "KiraraBoots", "tex_head_diffuse"}, "16fbe9b0"},
        {{"4.8", "KiraraBoots", "tex_head_lightmap"}, "f74f093d"},
        {{"4.8", "KiraraBoots", "tex_head_metalmap"}, "b0e08915"},
        {{"4.8", "KiraraBoots", "tex_body_normalmap"}, "89a118ba"},
        {{"4.8", "KiraraBoots", "tex_body_diffuse"}, "e3a21e6f"},
        {{"4.8", "KiraraBoots", "tex_body_lightmap"}, "8ca27fd3"},
        {{"4.8", "KiraraBoots", "tex_body_metalmap"}, "b0e08915"},
        {{"4.8", "KiraraBoots", "tex_dress_diffuse"}, "e3a21e6f"},
        {{"4.8", "KiraraBoots", "tex_dress_lightmap"}, "8ca27fd3"},
        {{"4.8", "KiraraBoots", "tex_dress_shadowramp"}, "7eb5b84e"},
        {{"4.8", "KiraraBoots", "tex_dress_metalmap"}, "b0e08915"},

        // HER FACE DIFFUSE, which neither this table nor the pure-Python one it came from had
        // (added 2026-09-11). GI-Model-Importer-Assets has no KiraraBoots folder to confirm
        // against, so the evidence is a real mod instead: `KPM CAT KiraraBoots Bikini V1` binds
        //
        //     [TextureOverrideKiraraBootsFaceHeadNormalMap]
        //     hash = 6eb20522
        //
        // which is Kirara's own face diffuse. That is a mod author DECLARING the hash, not us
        // inferring it from the base character, which is the distinction that makes this row
        // allowed where NilouBreeze's is not (see the note by AyakaSpringBloom above).
        //
        // Source and target hold the same value, so this changes nothing for a mod that already
        // has a face section -- re-A/B'd both directions to confirm exactly that. What it fixes is
        // the OTHER path: a KiraraBoots mod with no face section at all gets one INVENTED to hang
        // the download off, and an invented section seeds the source's hash. With no row there was
        // nothing to seed, so the section matched no draw call and the downloaded face was never
        // sampled -- silently, which is how the same bug survived on Kirara until it was seen in
        // game. NilouBreeze still has that failure today, visibly: her generated
        // [TextureOverrideNilouBreezeFaceNilouRemapFix] carries a ps-t1 and no hash.
        {{"4.8", "KiraraBoots", "tex_face_diffuse"}, "6eb20522"},

        // ===== version 5.2 =====
        // Diluc
        {{"5.2", "Diluc", "tex_face_diffuse"}, "e698735e"},
        // Lisa
        {{"5.2", "Lisa", "tex_head_diffuse"}, "b8ed7d4b"},

        // ===== version 5.3 =====
        // CherryHuTao
        {{"5.3", "CherryHuTao", "draw_vb"}, "6715905e"},
        {{"5.3", "CherryHuTao", "position_vb"}, "a78db232"},
        {{"5.3", "CherryHuTao", "blend_vb"}, "6e718139"},
        {{"5.3", "CherryHuTao", "texcoord_vb"}, "4b14b10e"},
        {{"5.3", "CherryHuTao", "ib"}, "92fce51e"},
        {{"5.3", "CherryHuTao", "tex_head_normalmap"}, "efe5e3ed"},
        {{"5.3", "CherryHuTao", "tex_head_diffuse"}, "99a26018"},
        {{"5.3", "CherryHuTao", "tex_head_lightmap"}, "d6089bf8"},
        {{"5.3", "CherryHuTao", "tex_body_diffuse"}, "b932cd65"},
        {{"5.3", "CherryHuTao", "tex_body_lightmap"}, "dd90b43c"},
        {{"5.3", "CherryHuTao", "tex_dress_normalmap"}, "e66b5b37"},
        {{"5.3", "CherryHuTao", "tex_dress_diffuse"}, "b932cd65"},
        {{"5.3", "CherryHuTao", "tex_dress_lightmap"}, "dd90b43c"},
        {{"5.3", "CherryHuTao", "tex_extra_diffuse"}, "99a26018"},
        // XianglingCheer
        {{"5.3", "XianglingCheer", "draw_vb"}, "e71f5012"},
        {{"5.3", "XianglingCheer", "position_vb"}, "05a65c3f"},
        {{"5.3", "XianglingCheer", "blend_vb"}, "bd659168"},
        {{"5.3", "XianglingCheer", "texcoord_vb"}, "c679abfe"},
        {{"5.3", "XianglingCheer", "ib"}, "cc7a4851"},
        {{"5.3", "XianglingCheer", "tex_head_normalmap"}, "2725cfa6"},
        {{"5.3", "XianglingCheer", "tex_head_diffuse"}, "7866ddd9"},
        {{"5.3", "XianglingCheer", "tex_head_lightmap"}, "16a7176a"},
        {{"5.3", "XianglingCheer", "tex_body_normalmap"}, "25260201"},
        {{"5.3", "XianglingCheer", "tex_body_diffuse"}, "a1ef63e6"},
        {{"5.3", "XianglingCheer", "tex_body_lightmap"}, "17c172d2"},

        // ===== version 5.4 =====
        // LisaStudent
        {{"5.4", "LisaStudent", "tex_head_normalmap"}, "a64a57de"},
        {{"5.4", "LisaStudent", "tex_head_diffuse"}, "438c9349"},
        {{"5.4", "LisaStudent", "tex_head_lightmap"}, "f7a42411"},
        {{"5.4", "LisaStudent", "tex_body_normalmap"}, "a64a57de"},
        {{"5.4", "LisaStudent", "tex_body_diffuse"}, "02cb9df7"},
        {{"5.4", "LisaStudent", "tex_body_lightmap"}, "cbf77c41"},

    };
    return rows;
}

}
}
