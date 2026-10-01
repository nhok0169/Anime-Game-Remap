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


#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     One `section`_ as :cpp:class:`IniScan` read it: a name, and its `KVPs`_ in file order
     @endrst
     */
    struct IniScanSection {

        /**
         * @brief The section's name, without its brackets
         */
        std::string name;

        /**
         * @brief The section's ``key = value`` pairs, both sides stripped, in the order they appear
         */
        std::vector<std::pair<std::string, std::string>> kvps;
    };


    /**
     * @brief
     @rst
     Reading an ``.ini`` file as flat lines, deliberately understanding almost none of it
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`IniFile` is the real parser: it builds a section graph, follows ``run =``, tracks
     the conditions each part sits under, and needs a :cpp:class:`ModType` to classify against.
     This does none of that. It answers "what does this file SAY" for a file that is not the one
     being fixed :raw-html:`<br />` :raw-html:`<br />`

     Which comes up more than it sounds. A mod's textures are commonly declared in a companion
     ``.ini`` that the file being fixed only namespaces to, a mod's folders may hold several ``.ini``
     files of which one is the mod and the rest are LODs or variants, and a fix that wants to know
     which file a resource name points at has to read a file no classifier will ever hand it. Every
     one of those is a ``filename =``, a ``hash =`` or a ``this =`` sitting under a section header
     :raw-html:`<br />` :raw-html:`<br />`

     .. warning::
        What this DOESN'T do is the point, so do not grow it into a parser. It ignores ``if``/
        ``endif`` nesting rather than tracking it, keeps no conditions, follows no ``run =``, and
        does not concatenate a section declared twice. A caller that needs any of those wants
        :cpp:class:`IniFile`, and a caller that quietly needs one of them while using this is
        reading a mod it does not understand
     @endrst
     */
    class IniScan {
        public:

            /**
             * @brief
             @rst
             Every `section`_ of the ``.ini`` file at 'path', in file order :raw-html:`<br />`
             :raw-html:`<br />`

             Blank lines and ``;`` comments are dropped, both sides of every ``=`` are stripped, and
             a line before the first section header is ignored. A section header is a line that both
             starts with ``[`` and ends with ``]`` :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A line with no ``=`` is skipped, which includes 3dmigoto's ``local $var``. That is a
                statement rather than a `KVP`_, and this is not the reader that has to preserve it --
                :cpp:class:`IniFile` is, and does
             @endrst
             *
             * @param path The ``.ini`` file to read
             *
             * @return Its sections, or an empty list if the file will not open
             */
            static std::vector<IniScanSection> scan(const std::string& path);

            /**
             * @brief
             @rst
             The first value in 'section' under 'key', compared exactly
             @endrst
             *
             * @param section The section to search
             * @param key The key to look for
             *
             * @return The value, or ``std::nullopt`` if the key is not in 'section'
             */
            static std::optional<std::string> firstVal(const IniScanSection& section, const std::string& key);
    };
}
