#ifndef AGRemapCore_BaseIniParseStep_H
#define AGRemapCore_BaseIniParseStep_H

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

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniPartEdit.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for one step of reading a mod: it reads part of the ``.ini`` file into the parser's
     :cpp:class:`IniParseData` :raw-html:`<br />` :raw-html:`<br />`

     A parser is a list of steps, the way a fixer is a list of edits: :cpp:member:`GIMIParser::parseSteps`
     holds them in the order they run, once the parser has found each mod object's `sections`_ and built
     their command graphs. That list may also hold :cpp:class:`BaseIniGraphGroupEdit`\ s, which edit
     the command graphs where they sit in it
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseIniParseStep: public BaseIniPartEdit {
        public:
            /**
             * @brief The ``.ini`` file a step reads
             */
            using Context = IniParseContext<K, V, KeyHash, KeyEqual>;

            /**
             * @brief A `section`_ of the ``.ini`` file
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief A part of a `section`_ that holds `KVPs`_
             */
            using ContentPart = typename Section::ContentPart;

            /**
             * @brief A mod object: ``(component, object)``
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief Each mod object's root `sections`_, as :cpp:func:`GIMIParser::sectionTargets` found them
             */
            using SectionTargets = tsl::ordered_map<ModObj, std::vector<std::string>, typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash>;

            /**
             * @brief Which mod objects a step reads: empty reads every one
             */
            using ObjFilter = std::function<bool(const ModObj&)>;

            /**
             * @brief
             @rst
             Whether the parser runs this step on the first ask of its :cpp:class:`IniParseData` rather than
             during the parse, for a step that may open many files (see :cpp:func:`IniParseData::defer`).
             The deferred steps of a parser run together, in their order in the list, after every step that
             is not deferred :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            bool deferred = false;

            /**
             * @brief Reads the ``.ini`` file into 'data'
             *
             * @param ctx The ``.ini`` file
             * @param targets Each mod object's root `sections`_
             * @param data What the parser has read so far, which this adds to
             */
            virtual void read(Context& ctx, const SectionTargets& targets, IniParseData& data) = 0;

            /**
             * @brief Whether 'filter' selects 'modObj': an empty filter selects every mod object
             */
            static bool selects(const ObjFilter& filter, const ModObj& modObj) {
                return !filter || filter(modObj);
            }

            /**
             * @brief The parts of 'section' that hold `KVPs`_, in order
             */
            static std::vector<const ContentPart*> contentParts(const Section& section) {
                std::vector<const ContentPart*> parts;
                for (const auto& part : section.parts()) {
                    if (const auto* content = dynamic_cast<const ContentPart*>(part.get())) {
                        parts.push_back(content);
                    }
                }
                return parts;
            }

            /**
             * @brief The first value of 'key' in the parts of 'section', stripped, without following ``run =``
             */
            static std::optional<std::string> firstVal(const Section& section, const K& key) {
                for (const ContentPart* content : contentParts(section)) {
                    std::vector<V> vals = content->getVals(key);
                    if (!vals.empty()) {
                        return std::string(StringTools::strip(vals.front()));
                    }
                }
                return std::nullopt;
            }
    };
}

#endif
