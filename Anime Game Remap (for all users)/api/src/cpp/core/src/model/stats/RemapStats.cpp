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

#include "AGRemapCore/model/stats/RemapStats.h"


namespace AGRemapCore {
    void RemapStats::clear() {
        blend.clear();
        position.clear();
        texcoord.clear();
        buf.clear();
        other.clear();
        ini.clear();
        texEdit.clear();
        texAdd.clear();
        download.clear();
    }

    FileStats* RemapStats::get(const std::string& resourceType) {
        // Spelled out rather than driven off RemapIniRemover::ResourceType's members: this file
        // cannot see that class (RemapIniRemover is a template, and including it here would be a
        // cycle), and a member-by-member map is what makes the two sets of names provably line up
        // anyway -- add a kind there and the compiler says nothing, but a caller looking it up here
        // gets nullptr, which is the failure that is easy to notice.
        if (resourceType == "blend") { return &blend; }
        if (resourceType == "position") { return &position; }
        if (resourceType == "texcoord") { return &texcoord; }
        if (resourceType == "buf") { return &buf; }
        if (resourceType == "other") { return &other; }
        if (resourceType == "ini") { return &ini; }
        if (resourceType == "texEdit") { return &texEdit; }
        if (resourceType == "texAdd") { return &texAdd; }

        // The same two buckets under the names the RESOURCE classes carry.
        //
        // The remover classifies with RemapIniRemover::ResourceType ("texEdit"/"texAdd") while the
        // fixer builds RemapTexEditResource/RemapTexAddResource, whose 'type' defaults to
        // "resourceRemapTexEdit"/"resourceRemapTexAdd" -- so a texture the fixer EDITED looked up a
        // name this function did not know, got nullptr, and was silently never counted. The summary
        // line for edited textures existed the whole time and simply never had anything to print.
        //
        // Both spellings are answered here rather than renaming either side: the resource-type
        // strings are the maintainer's explicit call (see RemapIniRemover's own note on
        // "RemapTexAdd"), and the remover's names are what the removal half already records under.
        if (resourceType == "resourceRemapTexEdit") { return &texEdit; }
        if (resourceType == "resourceRemapTexAdd") { return &texAdd; }

        // And the blend's, for the same reason: the compiled VGRemapBlendReplace names its models
        // "blend", but RemapBlendReplace -- the one a Python-side fix builds its own models through --
        // defaults to "resourceRemapBlend", so every Blend.buf a prototype wrote was counted nowhere
        // and the summary read "fixed 0 Blend.buf files" over a folder full of them (2026-09-12).
        if (resourceType == "resourceRemapBlend") { return &blend; }

        // CachedFileStats is a FileStats, so a caller sorting removed paths into buckets treats
        // downloads like anything else -- only the cache-hit half needs the derived type.
        if (resourceType == "download") { return &download; }

        return nullptr;
    }
}
