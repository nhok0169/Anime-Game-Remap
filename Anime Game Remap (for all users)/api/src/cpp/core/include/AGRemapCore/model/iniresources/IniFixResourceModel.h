#ifndef AGRemapCore_IniFixResourceModel_H
#define AGRemapCore_IniFixResourceModel_H

#include <optional>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iniresources/IniResourceModel.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniResourceModel`

     Contains data for fixing a particular resource in a .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniFixResourceModel`` class
     (``model/iniresources/IniFixResourceModel.py``) -- see :cpp:class:`IniSrcResourceModel`'s own
     doc comment for why ``tsl::ordered_map`` is used throughout instead of ``std::unordered_map``
     @endrst
     */
    class IniFixResourceModel: public IniResourceModel {
        public:

            /**
             * @brief One entry of #items -- the C++ counterpart to one tuple the pure-Python
             *      original's ``__iter__`` would yield
             */
            struct Entry {
                /**
                 * @brief The path name of the fixed file
                 */
                std::string fixedPath;

                /**
                 * @brief The full path name to the fixed file
                 */
                std::string fullPath;

                /**
                 * @brief The path to the original file, if available
                 */
                std::optional<std::string> origPath;

                /**
                 * @brief The full path name to the original file, if available
                 */
                std::optional<std::string> origFullPath;
            };

            /**
             * @brief Constructs new data for fixing a resource in a .ini file
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             * @param fixedPaths
             @rst
             The file paths to the fixed files for the resource -- the outer keys are the indices to
             the :cpp:class:`IfContentPart` that the resource file appears in the :cpp:class:`IfTemplate`
             for some resource, the inner keys are the names for the type of mod to fix to, and the
             inner values are the file paths within that :cpp:class:`IfContentPart`
             @endrst
             * @param origPaths
             @rst
             The file paths for the (unfixed) resource -- the keys are the indices to the
             :cpp:class:`IfContentPart` that the resource file appears in the :cpp:class:`IfTemplate`
             for some resource, and the values are the file paths within that
             :cpp:class:`IfContentPart`. ``std::nullopt`` if there's no original-file data at all
             @endrst
             */
            IniFixResourceModel(std::string iniFolderPath,
                                 tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths,
                                 std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths = std::nullopt);

            virtual ~IniFixResourceModel() = default;

            /**
             * @brief The file paths to the fixed files for the resource (see the constructor)
             */
            tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths;

            /**
             * @brief The file paths for the (unfixed) resource, if any (see the constructor)
             */
            std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths;

            /**
             * @brief The absolute paths to the fixed resource files, keyed the same way as #fixedPaths
             */
            tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fullPaths;

            /**
             * @brief The absolute paths to the (unfixed) resource files, keyed the same way as #origPaths
             */
            tsl::ordered_map<int, std::vector<std::string>> origFullPaths;

            /**
             * @brief
             @rst
             Every fixed/orig path combination across every :cpp:class:`IfContentPart` and mod type
             in #fixedPaths, in the same order #fixedPaths itself iterates -- the C++ counterpart to
             the pure-Python original's ``__iter__`` (see :cpp:class:`IniSrcResourceModel::items` for
             why this is a plain flattened list rather than a lazy generator)
             @endrst
             *
             * @return The flattened entries
             */
            std::vector<Entry> items() const;

            /**
             * @brief Clears out all the path data stored
             */
            virtual void clear();
    };
}

#endif
