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
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/model/buffers/SkeletonNumberingProbe.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/BaseIniGraphGroupEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/IParseResult.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphGroupEdit`

     Reads which skeleton numbering the mod's blend gives its bone ids in, with a
     :cpp:class:`SkeletonNumberingProbe` over the mod's own buffers, editing nothing -- and files it as
     :cpp:func:`IniParseData::blendVersion` (see :cpp:class:`IParseResult`) :raw-html:`<br />` :raw-html:`<br />`

     The buffers are the files the ``.ini``'s resource `sections`_ named here give, found without regard to the
     case of their names, so the probe runs only from #editFromIni with a real ``.ini`` file. Nothing is read with
     fewer than two numberings, with no centroids, or when a buffer is missing
     @endrst
     */
    class SkeletonNumberingRead: public BaseIniGraphGroupEdit<>, public IParseResult {
        public:
            /**
             * @brief Which mod objects to look in: empty selects every one
             */
            using ObjFilter = std::function<bool(const std::pair<std::string, std::string>&)>;

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
             A key at least one root `section`_ of the graphs of #objs (in the first group) must carry for the mod
             to be probed, or empty to probe every mod. **Default**: empty
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

            /**
             * @brief The version the last run found, or ``std::nullopt``
             */
            const std::optional<std::string>& version() const;

            /**
             * @brief Forgets #version
             */
            void clear() override;

            /**
             * @brief Probes the mod's buffers -- see this class's own description
             *
             * @param graphGroups The group of graphs of each .ini file, for #requiredKey
             * @param ini The .ini file whose resources name the buffers -- ``nullptr`` probes nothing
             * @param modType Unused
             * @param modName Unused. **Default**: ``""``
             *
             * @return The same groups that were passed in
             */
            GraphGroups& editFromIni(GraphGroups& graphGroups, IniFile* ini, const ModType* modType,
                                     const std::string& modName = "") override;

            /**
             * @brief Probes nothing: there is no ``.ini`` file to read the buffers' names from
             *
             * @return The same groups that were passed in
             */
            GraphGroups& edit(GraphGroups& graphGroups, const ModType* modType, const std::string& modName = "") override;

            /**
             * @brief Files #version as :cpp:func:`IniParseData::blendVersion`, when there is one
             *
             * @param data The parser's data
             */
            void fileInto(IniParseData& data) const override;

        private:
            std::optional<std::string> version_;
    };
}

#endif
