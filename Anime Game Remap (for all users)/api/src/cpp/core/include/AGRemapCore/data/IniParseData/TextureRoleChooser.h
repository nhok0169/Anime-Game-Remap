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


#ifndef AGRemapCore_TextureRoleChooser_H
#define AGRemapCore_TextureRoleChooser_H

#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniParsers/IniParseData.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Chooses, from what a parser found out about a mod's textures, the one file each mod object takes
     for each role -- and records the answer in an :cpp:class:`IniParseData` :raw-html:`<br />`
     :raw-html:`<br />`

     A parser collects the evidence (:cpp:member:`candidates`, :cpp:member:`bindings`,
     :cpp:member:`declared`, :cpp:member:`declaredFiles`) and :cpp:func:`choose` does the rest, in
     three steps:

     #. **One entry per file**: a file found for a role more than one way keeps the first way it was
        found
     #. **Refusals**, for a :cpp:member:`regionRoles` role only -- a role whose texture marks REGIONS,
        such as a material mask:

        * a file one mod object binds for that role AND another role is not that role's file (a mod
          may point its mask and its normal map at one resource, and the target then reads slope
          data as material codes)
        * a flat file (:cpp:member:`isFlat`) marks no regions -- ``R = 255`` everywhere says "all of
          this is bare skin"

        A refused role takes the ordinary "the mod has no file for this role" path
     #. **Ranking, per mod object**, lower first:

        #. a file the mod DECLARES as another role, by the ``hash =`` over it, comes after one it does
           not -- a register says where a file is used, the hash what it is
        #. how specifically the mod binds it on THIS object: only here, here and elsewhere, nowhere,
           and last only elsewhere
        #. whether the ``.ini`` declares a resource for it -- a file nothing names is a spare
        #. how far it is from the ``.ini``: folders up, then path length, then the path

        Two files equal on the first four are a tie, recorded in :cpp:member:`IniParseData::RoleFile::tiedFile`.
     #. **Variants of one toggle** (:cpp:member:`variantsOf`) are not ranked against each other: when the best
        file is a variant, the toggle's first-named variant among the candidates that ranks equal to it on the
        first key is chosen, and the toggle's files are recorded as
        :cpp:member:`IniParseData::RoleFile::variants`
     @endrst
     */
    class TextureRoleChooser {
        public:
            /**
             * @brief A mod object: ``(component, object)``
             */
            using ModObj = IniParseData::ModObj;

            /**
             * @brief Every file found for each role, as ``role -> (file, how it was found)``, in the order found
             */
            std::map<std::string, std::vector<std::pair<std::string, std::string>>> candidates;

            /**
             * @brief
             @rst
             What the mod's own `sections`_ bind each file for, as ``file -> mod object -> roles`` --
             the author writing ``ps-t5 = ResourceFoo`` inside an object's `section`_ IS the author
             saying what that object's role is
             @endrst
             */
            std::map<std::string, std::map<ModObj, std::set<std::string>>> bindings;

            /**
             * @brief The roles the mod DECLARES each file as, by writing the game texture's ``hash =`` over it
             */
            std::map<std::string, std::set<std::string>> declared;

            /**
             * @brief The files the ``.ini`` declares a resource `section`_ for
             */
            std::set<std::string> declaredFiles;

            /**
             * @brief The roles whose textures mark regions -- the only ones a file is refused for
             */
            std::set<std::string> regionRoles;

            /**
             * @brief The folder of the ``.ini``, which distance is measured from
             */
            std::string folder;

            /**
             * @brief
             @rst
             Whether a file is flat, for a :cpp:member:`regionRoles` role. **Default**: its channel 0
             -- the code that says where the regions are -- is constant
             (:cpp:func:`TexThumbprint::channelIsConstant`)
             @endrst
             */
            std::function<bool(const std::string& file)> isFlat;

            /**
             * @brief
             @rst
             The files of the toggle a file is a variant of, in the order the toggle names them, or empty for a file
             that is no toggle's. Variants of one toggle are not ranked against each other: the one the toggle names
             first is chosen and every one is recorded (:cpp:member:`IniParseData::RoleFile::variants`). **Default**:
             no file is a toggle's
             @endrst
             */
            std::function<std::vector<std::string>(const std::string& file)> variantsOf;

            /**
             * @brief
             @rst
             Narrows the candidates and chooses a file for every role and mod object, into 'data' --
             :cpp:func:`IniParseData::setRoleCandidates`, :cpp:func:`IniParseData::reject` and
             :cpp:func:`IniParseData::setRoleFile`
             @endrst
             *
             * @param data Where the answers go
             * @param modObjs The mod objects to choose for
             */
            void choose(IniParseData& data, const std::vector<ModObj>& modObjs) const;

        private:
            using Rank = std::tuple<int, int, int, std::size_t, std::size_t, std::string>;

            Rank rank(const std::string& role, const std::string& file, const ModObj& modObj) const;
    };
}

#endif
