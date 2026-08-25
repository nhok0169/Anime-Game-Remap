#ifndef AGRemapCore_IniDownloadModel_H
#define AGRemapCore_IniDownloadModel_H

#include <memory>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iniresources/IniSrcResourceModel.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniSrcResourceModel`

     Contains data about a particular resource to download in the original .ini file :raw-html:`<br />`
     :raw-html:`<br />`

     Mirrors the pure-Python ``IniDownloadModel`` class (``model/iniresources/IniDownloadModel.py``)
     -- #downloads owns its :cpp:class:`FileDownload`\\s via ``std::unique_ptr`` (matches
     :cpp:class:`IniTexModel`'s own reasoning for owning polymorphic entries this way)
     @endrst
     */
    class IniDownloadModel: public IniSrcResourceModel {
        public:

            /**
             * @brief Constructs new data for a resource to download
             *
             * @param iniFolderPath The folder path to where the .ini file of the resource is located
             * @param paths See :cpp:class:`IniSrcResourceModel`'s constructor
             * @param downloads
             @rst
             The downloader associated with each file -- the keys are the indices to the
             :cpp:class:`IfContentPart` that the resource file appears in the :cpp:class:`IfTemplate`
             for some resource, and the values are the downloaders for the files within that
             :cpp:class:`IfContentPart`
             @endrst
             */
            IniDownloadModel(std::string iniFolderPath, tsl::ordered_map<int, std::vector<std::string>> paths,
                              tsl::ordered_map<int, std::vector<std::unique_ptr<FileDownload>>> downloads);

            /**
             * @brief The downloader associated with each file (see the constructor)
             */
            tsl::ordered_map<int, std::vector<std::unique_ptr<FileDownload>>> downloads;
    };
}

#endif
