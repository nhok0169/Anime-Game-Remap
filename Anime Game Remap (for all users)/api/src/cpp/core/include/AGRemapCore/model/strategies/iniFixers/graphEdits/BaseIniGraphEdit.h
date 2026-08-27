#ifndef AGRemapCore_BaseIniGraphEdit_H
#define AGRemapCore_BaseIniGraphEdit_H

#include <functional>
#include <string>

#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniGraphPartEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     Base class for a filter that edits some caller/callee graph of :cpp:class:`IniSectionGraph`
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original's :cpp:class:`IniSectionGraph` is a class template here, so this
        pins it (and :cpp:class:`SectionIterData`) to ``<std::string, std::string>`` -- the same
        instantiation :cpp:class:`IniGraphGroup` and :cpp:class:`IniFile` already use, and the only
        one any real ``.ini`` file needs
     @endrst
     */
    class BaseIniGraphEdit: public BaseIniGraphPartEdit {
        public:

            /**
             * @brief The type of graph this edits
             */
            using Graph = IniSectionGraph<std::string, std::string>;

            /**
             * @brief The per-part iteration data a #PartFilter is handed
             */
            using IterData = SectionIterData<std::string, std::string>;

            /**
             * @brief
             @rst
             The ranges of valid `KVP`_ order indices a #PartFilter returns -- ``long long`` because
             that's the type :cpp:class:`IfContentPart` keys its order indices by
             @endrst
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief
             @rst
             The filter used to indicate the valid order indices to process for some
             :cpp:class:`IfContentPart` in the graph :raw-html:`<br />` :raw-html:`<br />`

             The ``.ini`` file argument is a **non-owning, nullable** pointer, matching the
             pure-Python original's ``Optional[IniFile]`` -- an empty ``std::function`` stands in for
             that original's ``partFilter = None``
             @endrst
             */
            using PartFilter = std::function<OrderRanges(const IterData&, const ModType&, IniFile*)>;

            /**
             * @brief
             @rst
             Edits the caller/callee graph with state info from 'ini' :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The base implementation forwards straight to #edit and **ignores 'ini' entirely**,
                exactly as the pure-Python original does (its ``BaseIniPartEdit.editFromIni`` calls
                ``self.edit(*args, **kwargs)`` without passing ``ini`` along). Subclasses that
                actually need the ``.ini`` file's state override this rather than #edit
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param ini The associated .ini file
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partFilter The filter for valid order indices -- empty for "no filter". **Default**: empty
             *
             * @return The same graph that was passed in, after editing
             */
            virtual Graph& editFromIni(Graph& graph, IniFile& ini, const ModType& modType,
                                        const std::string& modName = "", const PartFilter& partFilter = {});

            /**
             * @brief
             @rst
             Edits the caller/callee graph. No-op by default (returns 'graph' untouched), matching
             the pure-Python original's ``pass``
             @endrst
             *
             * @param graph The graph to edit, modified in place
             * @param modType The type of mod to fix
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partFilter The filter for valid order indices -- empty for "no filter". **Default**: empty
             *
             * @return The same graph that was passed in, after editing
             */
            virtual Graph& edit(Graph& graph, const ModType& modType,
                                 const std::string& modName = "", const PartFilter& partFilter = {});
    };
}

#endif
