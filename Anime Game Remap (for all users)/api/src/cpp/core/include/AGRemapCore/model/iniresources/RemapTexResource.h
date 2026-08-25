#ifndef AGRemapCore_RemapTexResource_H
#define AGRemapCore_RemapTexResource_H

#include <functional>
#include <string>

#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniResource`

     Class for adding a brand new texture file used by the overall remap process -- mirrors the
     pure-Python ``RemapTexAddResource`` class (``model/iniresources/RemapTexResource.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        #_fix's return value: same "always implicitly ``None``, contradicting the documented intent"
        gap as :cpp:class:`RemapIniDownload::_fix` -- see that class's own doc comment. This port
        returns ``true`` on success instead
     @endrst
     */
    class RemapTexAddResource: public RemapIniResource {
        public:

            /**
             * @brief Constructs a new texture-add resource
             *
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource
             * @param texCreator The texture creator used to create the ``.dds`` file if it's missing
             * @param type The name for the type of resource
             * @param fixFunc Custom function for fixing the resource, overriding #_fix if given
             */
            RemapTexAddResource(const std::string& iniFolderPath, const std::string& srcPath, TexCreator texCreator,
                                 std::string type = "resourceRemapTexAdd", std::function<bool(RemapTexAddResource&)> fixFunc = nullptr);

            /**
             * @brief The texture creator used to create the ``.dds`` file if it's missing
             */
            TexCreator texCreator;

            /**
             * @brief Custom function for fixing the resource, overriding #_fix if set
             */
            std::function<bool(RemapTexAddResource&)> fixFunc;

            bool srcEncounteredError(const RemapStats& stats) const override;
            bool srcIsFixed(const RemapStats& stats) const override;

            /**
             * @brief Same as #srcEncounteredError for this class (there's no separate fixed-file path -- #srcPath is both the source and the destination)
             */
            bool fixEncounteredError(const RemapStats& stats) const override;

            /**
             * @brief Same as #srcIsFixed for this class (there's no separate fixed-file path -- #srcPath is both the source and the destination)
             */
            bool fixIsFixed(const RemapStats& stats) const override;

            /**
             * @brief Determines whether the resource already exists on disk at #srcPath
             */
            bool fixExists(const RemapStats& stats) const override;

            /**
             * @brief Fixes the resource -- calls #fixFunc if set, otherwise #_fix
             *
             * @return Whether the resource was fixed
             */
            bool fix();

        protected:

            /**
             * @brief Creates the texture file at #srcPath via #texCreator, if it doesn't already exist
             *
             * @return Whether the resource was fixed
             */
            virtual bool _fix();
    };
}

#endif
