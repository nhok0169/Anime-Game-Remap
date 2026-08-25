#ifndef AGRemapCore_IniSrcResourceModel_H
#define AGRemapCore_IniSrcResourceModel_H

#include <string>
#include <utility>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iniresources/IniResourceModel.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniResourceModel`

     Contains data for a particular resource in the original .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniSrcResourceModel`` class
     (``model/iniresources/IniSrcResourceModel.py``) -- #paths/#fullPaths use ``tsl::ordered_map``
     rather than ``std::unordered_map`` specifically to preserve the same insertion-order iteration
     the Python original's plain ``dict`` gives for free, since these keys are the (order-meaningful)
     :cpp:class:`IfContentPart` indices for some :cpp:class:`IfTemplate`
     @endrst
     */
    class IniSrcResourceModel: public IniResourceModel {
        public:

            /**
             * @brief Constructs new data for a resource in the original .ini file
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             * @param paths
             @rst
             The file paths to the resource -- the keys are the indices to the
             :cpp:class:`IfContentPart` that the resource file appears in the :cpp:class:`IfTemplate`
             for some resource, and the values are the file paths within that
             :cpp:class:`IfContentPart`
             @endrst
             */
            IniSrcResourceModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths);

            virtual ~IniSrcResourceModel() = default;

            /**
             * @brief The file paths to the resource, keyed by :cpp:class:`IfContentPart` index (see the constructor)
             */
            tsl::ordered_map<int, std::vector<std::string>> paths;

            /**
             * @brief The absolute paths to the resource, keyed the same way as #paths
             */
            tsl::ordered_map<int, std::vector<std::string>> fullPaths;

            /**
             * @brief
             @rst
             Every ``(path, fullPath)`` pair across every :cpp:class:`IfContentPart` in #paths, in
             the same order #paths itself iterates -- the C++ counterpart to the pure-Python
             original's ``__iter__`` (a plain flattened list here rather than a lazy generator,
             since the whole flattened sequence is small and already fully materialized in #paths/
             #fullPaths anyway)
             @endrst
             *
             * @return The flattened ``(path, fullPath)`` pairs
             */
            std::vector<std::pair<std::string, std::string>> items() const;
    };
}

#endif
