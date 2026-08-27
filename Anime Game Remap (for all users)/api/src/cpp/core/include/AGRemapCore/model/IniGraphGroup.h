#ifndef AGRemapCore_IniGraphGroup_H
#define AGRemapCore_IniGraphGroup_H

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/IniSectionGraph.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A group of caller/callee graphs within a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     * The keys contain the name of the component and the name of the mod object
     * The values are the associated graph

     :raw-html:`<br />`

     .. note::
        :cpp:class:`IniSectionGraph` is a class template, but this deliberately is **not** -- it
        pins its graphs to ``IniSectionGraph<std::string, std::string>``, the same instantiation
        :cpp:class:`IniFile` already pins :cpp:class:`IfTemplate` to. Every ``.ini`` file this
        codebase actually parses is string-keyed and string-valued, so a template parameter here
        would only ever take one argument

     .. note::
        There is a separate, unrelated ``PyIniGraphGroup`` in the `pybind11`_ binding layer that is
        **not** built on this class. That one deliberately wraps a genuine `Python`_ ``dict``,
        because real call sites (``GIMIParser.py``) depend on `Python`_ dict *reference* semantics
        -- reading ``graphGroups[0].graphs`` back out and expecting the same object identity.
        Converting to/from a C++ map would silently break that aliasing, so the two coexist for now
     @endrst
     */
    class IniGraphGroup {
        public:

            /**
             * @brief The type of graph stored in this group
             */
            using Graph = IniSectionGraph<std::string, std::string>;

            /**
             * @brief
             @rst
             The key identifying a graph -- ``(component name, mod object name)``, the direct
             equivalent of the pure-Python original's ``Tuple[str, str]``
             @endrst
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief
             @rst
             Hashes a #ModObj, so it can key a ``tsl::ordered_map`` :raw-html:`<br />`
             :raw-html:`<br />`

             Uses the same multiply-and-xor combine the standard library's own implementations
             conventionally use -- nothing in this codebase needed a ``std::pair`` hash before this
             class, so there was no existing helper to reuse
             @endrst
             */
            struct ModObjHash {
                std::size_t operator()(const ModObj& modObj) const {
                    std::size_t firstHash = std::hash<std::string>{}(modObj.first);
                    std::size_t secondHash = std::hash<std::string>{}(modObj.second);
                    return firstHash ^ (secondHash + 0x9e3779b9 + (firstHash << 6) + (firstHash >> 2));
                }
            };

            /**
             * @brief
             @rst
             The container the graphs live in -- a ``tsl::ordered_map`` rather than a
             ``std::unordered_map`` so iteration follows *insertion* order, matching the `Python`_
             ``dict`` the pure-Python original used (which #toStr's output order depends on)
             @endrst
             */
            using GraphMap = tsl::ordered_map<ModObj, Graph, ModObjHash>;

            IniGraphGroup() = default;

            /**
             * @brief Constructs a new graph group
             *
             * @param graphs The group of graphs, keyed by their ``(component name, mod object name)``
             */
            explicit IniGraphGroup(GraphMap graphs);

            /**
             * @brief
             @rst
             :cpp:class:`IniSectionGraph` is move-only (its own copy constructor is deleted), so
             this necessarily is too
             @endrst
             */
            IniGraphGroup(const IniGraphGroup&) = delete;
            IniGraphGroup& operator=(const IniGraphGroup&) = delete;
            IniGraphGroup(IniGraphGroup&&) = default;
            IniGraphGroup& operator=(IniGraphGroup&&) = default;

            /**
             * @brief The group of graphs, keyed by their ``(component name, mod object name)``
             */
            const GraphMap& graphs() const;

            /**
             * @brief The group of graphs, keyed by their ``(component name, mod object name)``
             */
            GraphMap& graphs();

            /**
             * @brief Adds a new graph, replacing any graph already stored under 'modObj'
             *
             * @param modObj The associated component and mod object for the graph
             * @param graph The new graph to add
             */
            void addGraph(ModObj modObj, Graph graph);

            /**
             * @brief Removes the graph stored under 'modObj', if there is one
             *
             * @param modObj The associated component and mod object for the graph
             *
             * @return Whether a graph was actually removed
             */
            bool removeGraph(const ModObj& modObj);

            /**
             * @brief The graph stored under 'modObj', or ``nullptr`` if there isn't one
             *
             * @param modObj The associated component and mod object for the graph
             */
            Graph* getGraph(const ModObj& modObj);

            /**
             * @brief The number of graphs in this group
             */
            std::size_t size() const;

            /**
             * @brief Whether this group holds no graphs at all
             */
            bool empty() const;

        private:
            GraphMap graphs_;
    };
}

#endif
