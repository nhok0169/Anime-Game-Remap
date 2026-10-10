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

// -----------------------------------------------------------------------------
// Standalone regression test for a texture-only SIBLING .ini: CherryHutao6's shape (2026-10-08).
//
// The folder is an XXMI 1.6.3 export. HuTaoCherry.ini draws the mesh and binds no textures;
// textures.ini beside it watches the position hash (`$active = 1`) and recolours the skin's four
// textures with `this =` behind `if $color == 0`. Fixed CherryHuTao -> HuTao, it used to come out as
// a WHOLE second mod -- downloaded ib / buffers drawn on Hu Tao's hashes over the real fix, which
// tore the model into shards in game -- and its fix was then repeated in a texturesRemapFix1.ini.
//
// Covered, each through the real IniFile::classify / fix on files in a temp folder:
//   * the sibling's fix keeps only its own watcher, on the target's position hash: no ib, no
//     buffer, no draw and no download (GIMICharParser's getSectionTargets)
//   * no RemapFix copy is written for it: a copy whose every section repeats the mod's own file
//     draws nothing new and only declares the same hashes twice (GIMIFixer::fix)
//   * the mesh file still gets the copy its extra -> head merge needs, and binds the sibling's
//     textures under RemapRef resources
//   * no fix writes a TextureOverride without a hash: CherryHuTao has no face row in HashData, so
//     the face download's invented section had nothing to match on (GIMIParser::addDownloads)
//   * the recolour keeps its toggle on the target: bound under `if $colorRemapRef == 0` after the
//     download, ahead of the draw, with the mesh file declaring that copy of the sibling's `$color`
//     and driving it with a copy of the sibling's `left` key -- in the generated copy too, which is a
//     namespace of its own
//   * a condition on a variable nothing can drive here is not carried: the recolour is bound
//     unconditionally, as before
//   * an undo keeps the author's own `[Constants]`: the fix writes one too, and the remover used to
//     delete every section of a name the fix had written (RemapIniRemover::isMergedSection)
//
// Needs the full static lib. Build AGRemapCore first ("cd cbuild && ninja AGRemapCore"), then
// compile and link as described in RemapService_fix_test.cpp.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>

#include "AGRemapCore/constants/GlobalModTypes.h"
#include "AGRemapCore/model/files/IniFile.h"

namespace AGRC = AGRemapCore;

static int failures = 0;


static void check(bool condition, const std::string& what) {
    if (condition) {
        return;
    }

    std::printf("  FAILED: %s\n", what.c_str());
    ++failures;
}


// The mesh file, as XXMI 1.6.3 writes it: every buffer and draw, and not one texture.
static const char* MeshIni =
    "[Constants]\n"
    "global $active\n"
    "\n"
    "[Present]\n"
    "post $active = 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryPosition]\n"
    "hash = a78db232\n"
    "vb0 = ResourceHuTaoCherryPosition\n"
    "$active = 1\n"
    "\n"
    "[TextureOverrideHuTaoCherryBlend]\n"
    "hash = 6e718139\n"
    "handling = skip\n"
    "vb1 = ResourceHuTaoCherryBlend\n"
    "draw = 23420, 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryTexcoord]\n"
    "hash = 4b14b10e\n"
    "vb1 = ResourceHuTaoCherryTexcoord\n"
    "\n"
    "[TextureOverrideHuTaoCherryVertexLimitRaise]\n"
    "hash = 6715905e\n"
    "override_vertex_count = 23420\n"
    "override_byte_stride = 40\n"
    "\n"
    "[TextureOverrideHuTaoCherryIB]\n"
    "hash = 92fce51e\n"
    "handling = skip\n"
    "\n"
    "[TextureOverrideHuTaoCherryHead]\n"
    "hash = 92fce51e\n"
    "match_first_index = 0\n"
    "ib = ResourceHuTaoCherryHeadIB\n"
    "drawindexed = 39984, 1356, 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryBody]\n"
    "hash = 92fce51e\n"
    "match_first_index = 43968\n"
    "ib = ResourceHuTaoCherryBodyIB\n"
    "drawindexed = 33333, 0, 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryDress]\n"
    "hash = 92fce51e\n"
    "match_first_index = 77301\n"
    "ib = ResourceHuTaoCherryDressIB\n"
    "drawindexed = 9507, 0, 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryExtra]\n"
    "hash = 92fce51e\n"
    "match_first_index = 86808\n"
    "ib = ResourceHuTaoCherryExtraIB\n"
    "drawindexed = 432, 0, 0\n"
    "\n"
    "[ResourceHuTaoCherryPosition]\n"
    "type = Buffer\n"
    "stride = 40\n"
    "filename = HuTaoCherryPosition.buf\n"
    "\n"
    "[ResourceHuTaoCherryBlend]\n"
    "type = Buffer\n"
    "stride = 32\n"
    "filename = HuTaoCherryBlend.buf\n"
    "\n"
    "[ResourceHuTaoCherryTexcoord]\n"
    "type = Buffer\n"
    "stride = 28\n"
    "filename = HuTaoCherryTexcoord.buf\n"
    "\n"
    "[ResourceHuTaoCherryHeadIB]\n"
    "type = Buffer\n"
    "format = DXGI_FORMAT_R32_UINT\n"
    "filename = HuTaoCherryHead.ib\n"
    "\n"
    "[ResourceHuTaoCherryBodyIB]\n"
    "type = Buffer\n"
    "format = DXGI_FORMAT_R32_UINT\n"
    "filename = HuTaoCherryBody.ib\n"
    "\n"
    "[ResourceHuTaoCherryDressIB]\n"
    "type = Buffer\n"
    "format = DXGI_FORMAT_R32_UINT\n"
    "filename = HuTaoCherryDress.ib\n"
    "\n"
    "[ResourceHuTaoCherryExtraIB]\n"
    "type = Buffer\n"
    "format = DXGI_FORMAT_R32_UINT\n"
    "filename = HuTaoCherryExtra.ib\n";


// The sibling: a position watcher and four recolours of the skin's own textures.
static const char* TexturesIni =
    "[Constants]\n"
    "global persist $color = 0\n"
    "global $active\n"
    "\n"
    "[KeySwap3]\n"
    "condition = $active == 1\n"
    "key = left\n"
    "type = cycle\n"
    "$color = 0,1\n"
    "\n"
    "[Present]\n"
    "post $active = 0\n"
    "\n"
    "[TextureOverrideHuTaoCherryPosition]\n"
    "hash = a78db232\n"
    "$active = 1\n"
    "\n"
    "[TextureOverrideHuTaoCherryHeadDiffuse]\n"
    "hash = 99a26018\n"
    "if $color == 0\n"
    "this = ResourceHuTaoCherryHeadDiffuse\n"
    "endif\n"
    "\n"
    "[TextureOverrideHuTaoCherryBodyDiffuse]\n"
    "hash = b932cd65\n"
    "if $color == 0\n"
    "this = ResourceHuTaoCherryBodyDiffuse\n"
    "endif\n"
    "\n"
    "[TextureOverrideHuTaoCherryHeadLightMap]\n"
    "hash = d6089bf8\n"
    "if $color == 0\n"
    "this = ResourceHuTaoCherryHeadLightMap\n"
    "endif\n"
    "\n"
    "[TextureOverrideHuTaoCherryBodyLightMap]\n"
    "hash = dd90b43c\n"
    "if $color == 0\n"
    "this = ResourceHuTaoCherryBodyLightMap\n"
    "endif\n"
    "\n"
    "[ResourceHuTaoCherryHeadLightMap]\n"
    "filename = HuTaoCherryHeadLightMap.dds\n"
    "\n"
    "[ResourceHuTaoCherryHeadDiffuse]\n"
    "filename = HuTaoCherryHeadDiffuse.dds\n"
    "\n"
    "[ResourceHuTaoCherryBodyLightMap]\n"
    "filename = HuTaoCherryBodyLightMap.dds\n"
    "\n"
    "[ResourceHuTaoCherryBodyDiffuse]\n"
    "filename = HuTaoCherryBodyDiffuse.dds\n";


static std::filesystem::path folder() {
    return std::filesystem::temp_directory_path() / "agremap_texture_only_sibling_test";
}


static void writeFile(const std::filesystem::path& path, const std::string& txt) {
    std::ofstream out(path, std::ios::binary);
    out << txt;
}


// A fresh copy of the mod: each test fixes its own, so no fix reads another's output.
static void buildFolder(const std::string& texturesIni = TexturesIni) {
    std::error_code error;
    std::filesystem::remove_all(folder(), error);
    std::filesystem::create_directories(folder());
    writeFile(folder() / "HuTaoCherry.ini", MeshIni);
    writeFile(folder() / "textures.ini", texturesIni);
}


// The text of the section 'name' in 'content' (header included), or empty
static std::string sectionText(const std::string& content, const std::string& name) {
    const std::size_t start = content.find("[" + name + "]");
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = content.find("\n[", start + 1);
    return content.substr(start, end == std::string::npos ? std::string::npos : end - start);
}


static std::unordered_map<std::string, std::string> fixFile(const std::string& name) {
    AGRC::IniFile ini((folder() / name).string());
    ini.classify();
    check(!ini.getModTypes().empty(), name + " classifies as a mod type");
    return ini.fix(false, false, false);
}


static bool endsWith(const std::string& txt, const std::string& suffix) {
    return txt.size() >= suffix.size() && txt.compare(txt.size() - suffix.size(), suffix.size(), suffix) == 0;
}


// The first TextureOverride section of 'content' that has no `hash` line, or empty if none.
static std::string hashlessTextureOverride(const std::string& content) {
    std::string current;
    bool hasHash = true;
    std::size_t start = 0;
    while (start <= content.size()) {
        std::size_t end = content.find('\n', start);
        if (end == std::string::npos) {
            end = content.size();
        }

        std::string line = content.substr(start, end - start);
        const std::size_t first = line.find_first_not_of(" \t\r");
        line = (first == std::string::npos) ? std::string() : line.substr(first);

        if (!line.empty() && line[0] == '[') {
            if (!current.empty() && !hasHash) {
                return current;
            }
            current = (line.rfind("[TextureOverride", 0) == 0) ? line : std::string();
            hasHash = false;
        } else if (line.rfind("hash", 0) == 0) {
            hasHash = true;
        }
        start = end + 1;
    }
    return (!current.empty() && !hasHash) ? current : std::string();
}


static void testWatcherSiblingDrawsNothing() {
    std::printf("testWatcherSiblingDrawsNothing\n");
    buildFolder();

    const std::unordered_map<std::string, std::string> fix = fixFile("textures.ini");

    bool found = false;
    for (const auto& [path, content] : fix) {
        if (!endsWith(path, "textures.ini")) {
            continue;
        }

        found = true;
        check(content.find("dd16576c") != std::string::npos, "the watcher is carried onto Hu Tao's position hash");
        check(content.find("RemapDL") == std::string::npos, "nothing is downloaded for it");
        for (const std::string& key : {"\nib =", "\nvb0 =", "\nvb1 =", "drawindexed", "\ndraw ="}) {
            check(content.find(key) == std::string::npos, "and it binds or draws nothing of the mesh: no '" + key.substr(key[0] == '\n') + "'");
        }
    }
    check(found, "textures.ini itself is fixed");
}


static void testNoCopyRepeatingTheModsOwnFile() {
    std::printf("testNoCopyRepeatingTheModsOwnFile\n");
    buildFolder();

    const std::unordered_map<std::string, std::string> fix = fixFile("textures.ini");
    for (const auto& entry : fix) {
        check(entry.first.find("RemapFix") == std::string::npos, "no copy is written for it: " + entry.first);
    }
    check(!std::filesystem::exists(folder() / "texturesRemapFix1.ini"), "and none is on disk");
}


static void testMeshFileKeepsItsCopyAndTheSiblingsTextures() {
    std::printf("testMeshFileKeepsItsCopyAndTheSiblingsTextures\n");
    buildFolder();

    const std::unordered_map<std::string, std::string> fix = fixFile("HuTaoCherry.ini");

    bool copy = false;
    bool refs = false;
    for (const auto& [path, content] : fix) {
        copy = copy || endsWith(path, "HuTaoCherryRemapFix1.ini");
        refs = refs || (endsWith(path, "HuTaoCherry.ini") && content.find("ResourceHuTaoCherryHeadDiffuseRemapRef") != std::string::npos);
    }
    check(copy, "the extra -> head merge still gets its copy, which draws something the mod's file does not");
    check(refs, "the mesh file binds the sibling's recolour under a RemapRef resource");
}


static void testNoTextureOverrideWithoutAHash() {
    std::printf("testNoTextureOverrideWithoutAHash\n");

    for (const std::string& name : {std::string("HuTaoCherry.ini"), std::string("textures.ini")}) {
        buildFolder();
        for (const auto& [path, content] : fixFile(name)) {
            const std::string hashless = hashlessTextureOverride(content);
            check(hashless.empty(), path + " has a TextureOverride with no hash: " + hashless);
            check(content.find("FaceDiffuseRemapDL") == std::string::npos,
                  path + " fetches no face for a skin with no face hash to match it on");
        }
    }
}


static void testRecolourKeepsItsToggle() {
    std::printf("testRecolourKeepsItsToggle\n");
    buildFolder();

    const std::unordered_map<std::string, std::string> fix = fixFile("HuTaoCherry.ini");
    std::size_t files = 0;
    for (const auto& [path, content] : fix) {
        if (!endsWith(path, "HuTaoCherry.ini") && !endsWith(path, "HuTaoCherryRemapFix1.ini")) {
            continue;
        }
        ++files;

        const std::string constants = content.substr(content.rfind("[Constants]") == std::string::npos ? 0 : content.rfind("[Constants]"));
        check(constants.find("global persist $colorRemapRef = 0") != std::string::npos,
              path + " declares its copy of the sibling's $color, with the sibling's default");

        const std::string key = sectionText(content, "KeySwap3RemapRef");
        check(key.find("$colorRemapRef = 0,1") != std::string::npos, path + " cycles the copy with the sibling's key section");
        check(key.find("key = left") != std::string::npos, "...on the sibling's key");
        check(key.find("condition = $active == 1") != std::string::npos,
              "...under the sibling's condition, which reads a variable this file declares too");
    }
    check(files == 2, "both the mesh file and its generated copy carry the toggle");

    for (const auto& [path, content] : fix) {
        if (!endsWith(path, "HuTaoCherry.ini")) {
            continue;
        }
        const std::string head = sectionText(content, "TextureOverrideHuTaoCherryHeadHuTaoRemapFix");
        const std::size_t download = head.find("HeadDiffuseRemapDL");
        const std::size_t cond = head.find("if $colorRemapRef == 0");
        const std::size_t ref = head.find("ResourceHuTaoCherryHeadDiffuseRemapRef");
        const std::size_t draw = head.find("drawindexed");
        check(download != std::string::npos && cond != std::string::npos && ref != std::string::npos,
              "the head binds the download and, under the toggle, the recolour:\n" + head);
        check(download < cond && cond < ref && ref < draw, "...download first, then the toggled recolour, then the draw");
    }
}


static void testUndrivenConditionIsNotCarried() {
    std::printf("testUndrivenConditionIsNotCarried\n");

    std::string texturesIni = TexturesIni;
    std::size_t at;
    while ((at = texturesIni.find("if $color == 0")) != std::string::npos) {
        texturesIni.replace(at, std::string("if $color == 0").size(), "if $nobodySetsThis == 0");
    }
    buildFolder(texturesIni);

    for (const auto& [path, content] : fixFile("HuTaoCherry.ini")) {
        if (!endsWith(path, "HuTaoCherry.ini")) {
            continue;
        }
        check(content.find("RemapRef ==") == std::string::npos && content.find("nobodySetsThis") == std::string::npos,
              "no condition is carried for a variable nothing drives");
        const std::string head = sectionText(content, "TextureOverrideHuTaoCherryHeadHuTaoRemapFix");
        check(head.find("ResourceHuTaoCherryHeadDiffuseRemapRef") != std::string::npos && head.find("RemapDL") == std::string::npos,
              "...and the recolour is bound unconditionally in place of the download, as before");
    }
}


static std::string readFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}


static void testUndoKeepsTheAuthorsConstants() {
    std::printf("testUndoKeepsTheAuthorsConstants\n");
    buildFolder();
    fixFile("HuTaoCherry.ini");
    check(readFile(folder() / "HuTaoCherry.ini").find("$colorRemapRef") != std::string::npos, "the fix carried the toggle");

    AGRC::IniFile ini((folder() / "HuTaoCherry.ini").string());
    ini.removeFix();

    const std::string undone = readFile(folder() / "HuTaoCherry.ini");
    check(undone.find("[Constants]\nglobal $active") != std::string::npos || undone.find("[Constants]\r\nglobal $active") != std::string::npos,
          "an undo keeps the author's [Constants]:\n" + undone.substr(0, 200));
    check(undone.find("RemapRef") == std::string::npos, "...and removes everything the fix wrote, its own [Constants] included");
}


int main() {
    AGRC::GlobalModTypes::registerAll();

    testWatcherSiblingDrawsNothing();
    testNoCopyRepeatingTheModsOwnFile();
    testMeshFileKeepsItsCopyAndTheSiblingsTextures();
    testNoTextureOverrideWithoutAHash();
    testRecolourKeepsItsToggle();
    testUndrivenConditionIsNotCarried();
    testUndoKeepsTheAuthorsConstants();

    std::error_code error;
    std::filesystem::remove_all(folder(), error);

    if (failures == 0) {
        std::printf("\nAll tests passed.\n");
    } else {
        std::printf("\n%d test(s) FAILED.\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
