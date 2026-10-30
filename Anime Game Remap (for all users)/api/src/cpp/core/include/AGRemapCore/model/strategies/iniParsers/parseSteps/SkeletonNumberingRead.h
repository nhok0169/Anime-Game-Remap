#ifndef AGRemapCore_SkeletonNumberingRead_H
#define AGRemapCore_SkeletonNumberingRead_H

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

#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/strategies/iniParsers/parseSteps/BaseIniParseStep.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Reads which skeleton numbering the mod's blend gives its bone ids in, with a
     :cpp:class:`SkeletonNumberingProbe` over the mod's own buffers, into
     :cpp:func:`IniParseData::blendVersion` :raw-html:`<br />` :raw-html:`<br />`

     The buffers are the files of the resource `sections`_ this names, as
     :cpp:func:`IniParseData::resourceFile` has them -- so run :cpp:class:`ResourceRead` first. Nothing is
     read with fewer than two numberings, with no centroids, or when a buffer is missing
     @endrst
     */
    class SkeletonNumberingRead: public BaseIniParseStep<> {
        public:
            /**
             * @param numberings The candidate numberings -- see :cpp:func:`SkeletonNumberingProbe::probe`
             * @param centroids Each bone's rest-pose centroid -- see :cpp:func:`SkeletonNumberingProbe::probe`
             * @param positionResource The resource `section`_ of the mod's positions
             * @param blendResource The resource `section`_ of the mod's blend
             * @param vertexVGResource
             @rst
             Text in the name of the resource `section`_ of the mod's 16-bit bone ids, compared lowercased and
             with underscores removed, or empty for none. The first such `section`_ in name order is read.
             **Default**: empty
             @endrst
             * @param requiredKey
             @rst
             A key at least one root `section`_ of #objs must carry for the mod to be probed, or empty to probe
             every mod. **Default**: empty
             @endrst
             * @param objs The mod objects whose root `sections`_ #requiredKey is looked for in. **Default**: every one
             */
            SkeletonNumberingRead(std::vector<SkeletonNumberingProbe::Numbering> numberings,
                                  std::unordered_map<long long, std::array<double, 3>> centroids,
                                  std::string positionResource, std::string blendResource,
                                  std::string vertexVGResource = "", std::string requiredKey = "", ObjFilter objs = {});

            /**
             * @brief The candidate numberings
             */
            std::vector<SkeletonNumberingProbe::Numbering> numberings;

            /**
             * @brief Each bone's rest-pose centroid
             */
            std::unordered_map<long long, std::array<double, 3>> centroids;

            /**
             * @brief The resource `section`_ of the mod's positions
             */
            std::string positionResource;

            /**
             * @brief The resource `section`_ of the mod's blend
             */
            std::string blendResource;

            /**
             * @brief Text in the name of the resource `section`_ of the mod's 16-bit bone ids -- see the constructor
             */
            std::string vertexVGResource;

            /**
             * @brief A key the mod must carry to be probed -- see the constructor
             */
            std::string requiredKey;

            /**
             * @brief The mod objects whose root `sections`_ #requiredKey is looked for in
             */
            ObjFilter objs;

            void read(Context& ctx, const SectionTargets& targets, IniParseData& data) override;
    };
}

#endif
