#ifndef AGRemapCore_IniTexModel_H
#define AGRemapCore_IniTexModel_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iniresources/IniFixResourceModel.h"
#include "AGRemapCore/model/strategies/texEditors/BaseTexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniFixResourceModel`

     Contains data for editing some texture files in a .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniTexModel`` class (``model/iniresources/IniTexModel.py``) --
     #texEdits owns its :cpp:class:`BaseTexEditor`\\s via ``std::unique_ptr`` since it's a
     polymorphic type (matches how it's used elsewhere in this codebase, eg.
     :cpp:class:`BlendFile`'s owned ``BufElementType``\\s)
     @endrst
     */
    class IniTexModel: public IniFixResourceModel {
        public:

            /**
             * @brief Constructs new data for editing a texture file in a .ini file
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             * @param fixedPaths See :cpp:class:`IniFixResourceModel`'s constructor
             * @param texEdits
             @rst
             The texture editors used to edit the texture -- the outer keys are the indices to the
             :cpp:class:`IfContentPart` that the ``.dds`` file appears in the :cpp:class:`IfTemplate`
             for some texture, the inner keys are the names for the type of mod to fix to, and the
             inner values are the different texture editors used on the ``.dds`` files
             @endrst
             * @param origPaths See :cpp:class:`IniFixResourceModel`'s constructor
             */
            IniTexModel(std::string iniFolderPath,
                        tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::string>>> fixedPaths,
                        tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>>> texEdits,
                        std::optional<tsl::ordered_map<int, std::vector<std::string>>> origPaths = std::nullopt);

            /**
             * @brief The texture editors used to edit the texture (see the constructor)
             */
            tsl::ordered_map<int, tsl::ordered_map<std::string, std::vector<std::unique_ptr<BaseTexEditor>>>> texEdits;

            void clear() override;
    };
}

#endif
