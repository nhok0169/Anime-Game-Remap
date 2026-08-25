#ifndef AGRemapCore_IniResourceModel_H
#define AGRemapCore_IniResourceModel_H

#include <string>
#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Contains data for some particular resource in a .ini file :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniResourceModel`` class (``model/iniresources/IniResourceModel.py``)
     @endrst
     */
    class IniResourceModel {
        public:

            /**
             * @brief Constructs new data for a resource in a .ini file
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             */
            explicit IniResourceModel(std::string iniFolderPath): iniFolderPath(std::move(iniFolderPath)) {}

            virtual ~IniResourceModel() = default;

            /**
             * @brief The folder path to where the .ini file of the resource is located
             */
            std::string iniFolderPath;
    };
}

#endif
