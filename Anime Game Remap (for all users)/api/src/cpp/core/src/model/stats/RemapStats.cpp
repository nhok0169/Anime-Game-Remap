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

        // CachedFileStats is a FileStats, so a caller sorting removed paths into buckets treats
        // downloads like anything else -- only the cache-hit half needs the derived type.
        if (resourceType == "download") { return &download; }

        return nullptr;
    }
}
