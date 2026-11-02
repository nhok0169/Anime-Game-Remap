#ifndef AGRemapCore_GraphEditVariables_H
#define AGRemapCore_GraphEditVariables_H
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

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     Named variables that edits save data about one graph into, and read or write back out later -- the data a
     list of edits passes from one edit to another :raw-html:`<br />` :raw-html:`<br />`

     A variable holds one graph's worth of `sections`_ (:cpp:class:`GraphEditVariables::Sections`): an edit copies
     sections of a graph into it (:cpp:class:`VarSectionsTake`), code adds sections it builds itself
     (:cpp:func:`Sections::add`), and an edit writes it out as a graph (:cpp:member:`GraphCreate::sectionsOf`). A fixer's own
     sections are one such variable

     .. warning::
        A variable is a snapshot. An edit that changes a graph afterwards does not change what a variable copied from
        it holds, and an edit that changes the graph a variable was written to does not change the variable
     @endrst
     */
    class GraphEditVariables {
        public:
            /**
             * @brief A `section`_ held in a variable
             */
            using Section = IfTemplate<std::string, std::string>;

            /**
             * @brief
             @rst
             A variable holding the `sections`_ of one graph, in order :raw-html:`<br />` :raw-html:`<br />`

             Each :cpp:func:`add` is one place in the order. :cpp:func:`reserve` holds a place open so sections known
             only later (a :cpp:class:`VarSectionsTake` that runs at fix time) land where the reservation was made
             rather than after everything added since
             @endrst
             */
            class Sections {
                public:
                    /**
                     * @brief A place :cpp:func:`reserve` held open: its index, and how many times the variable was
                     * cleared when it was held
                     */
                    using Place = std::pair<std::size_t, std::size_t>;

                    Sections() = default;
                    Sections(const Sections&) = delete;
                    Sections& operator=(const Sections&) = delete;

                    /**
                     * @brief Adds a section at the end
                     *
                     * @param section The section
                     */
                    void add(std::unique_ptr<Section> section);

                    /**
                     * @brief Holds a place open at the end, for sections added later with :cpp:func:`addAt`
                     *
                     * @return The place
                     */
                    Place reserve();

                    /**
                     * @brief
                     @rst
                     Adds a section at a place :cpp:func:`reserve` held, after the sections already added there --
                     or at the end, when the variable was cleared since the place was held
                     @endrst
                     *
                     * @param section The section
                     * @param place The place
                     */
                    void addAt(std::unique_ptr<Section> section, const Place& place);

                    /**
                     * @brief The sections, in order
                     */
                    std::vector<Section*> list() const;

                    /**
                     * @brief Whether the variable holds no section
                     */
                    bool empty() const;

                    /**
                     * @brief Drops every section; a place held before is held no more
                     */
                    void clear();

                private:
                    std::vector<std::vector<std::unique_ptr<Section>>> places_;
                    std::size_t cleared_ = 0;
            };

            GraphEditVariables() = default;
            GraphEditVariables(const GraphEditVariables&) = delete;
            GraphEditVariables& operator=(const GraphEditVariables&) = delete;

            /**
             * @brief The sections variable named 'name', made empty if there is none
             *
             * @param name The variable's name
             *
             * @return The variable
             */
            Sections& sections(const std::string& name);

            /**
             * @brief The sections variable named 'name', or ``nullptr`` if there is none
             *
             * @param name The variable's name
             *
             * @return The variable
             */
            const Sections* findSections(const std::string& name) const;

            /**
             * @brief The names of the sections variables, in the order they were made
             */
            std::vector<std::string> sectionsNames() const;

            /**
             * @brief Drops every section of every variable; a place held before is held no more
             */
            void clear();

        private:
            // the variables in the order they were made; a pointer each, so a variable keeps its address
            tsl::ordered_map<std::string, std::unique_ptr<Sections>> sections_;
    };
}

#endif
