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
}
