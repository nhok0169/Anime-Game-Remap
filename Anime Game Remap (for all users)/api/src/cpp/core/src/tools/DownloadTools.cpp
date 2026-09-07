#include "AGRemapCore/tools/DownloadTools.h"

#include <utility>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/VertexCountData.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace AGRemapCore {

    const std::string& DownloadTools::downloadFolder() {
        static const std::string folder =
            "https://github.com/nhok0169/Anime-Game-Remap/raw/nhok0169/Data/Mod%20Downloads";
        return folder;
    }


    long long DownloadTools::vertexCountOf(const std::string& version, const std::string& modName,
                                            const std::string& component) {
        for (const auto& row : Data::getVertexCountDataRows()) {
            if (row.first.size() == 3 && row.first[0] == version && row.first[1] == modName
                    && row.first[2] == component) {
                return row.second;
            }
        }

        return 0;
    }


    DownloadTools::Download::DownloadConfig DownloadTools::makeConfig() {
        Download::DownloadConfig config{};
        config.filenameKey = IniKeywords::Filename;
        config.valOfPath = [](const std::string& path) { return path; };
        config.runConfig = IfTemplateRunConfig<std::string, std::string>{
            IniKeywords::Run,
            [](const std::string& val) { return val; },
            [](const std::string& name) { return name; }
        };

        return config;
    }


    std::unique_ptr<DownloadTools::Download> DownloadTools::make(const std::string& name, const std::string& urlPath,
                                                                  const std::string& fixedName, KVPs resourceKVPs,
                                                                  KVPs downloadRefKVPs) {
        return std::make_unique<Download>(
            name,
            std::make_unique<FileDownload>(downloadFolder() + "/" + urlPath, fixedName),
            makeConfig(),
            /*refToSection*/ false,
            std::move(downloadRefKVPs),
            std::move(resourceKVPs));
    }


    std::string DownloadTools::fixedFileName(const std::string& prefix, const std::string& kind,
                                              const std::string& ext) {
        return prefix + kind + IniKeywords::RemapDL + ext;
    }


    std::string DownloadTools::urlPath(const std::string& charFolder, const std::string& versionFolder,
                                        const std::string& prefix, const std::string& kind,
                                        const std::string& ext) {
        return "GI/" + charFolder + "/" + versionFolder + "/" + prefix + kind + ext;
    }


    DownloadTools::KVPs DownloadTools::bufResourceKVPs(int stride) {
        return {{"type", "Buffer"}, {"stride", std::to_string(stride)}};
    }


    DownloadTools::KVPs DownloadTools::ibResourceKVPs() {
        return {{"type", "Buffer"}, {"format", "DXGI_FORMAT_R32_UINT"}};
    }


    DownloadTools::KVPs DownloadTools::blendRefKVPs(long long vertexCount) {
        // A downloaded Blend.buf is drawn by hand: the .ini file it is dropped into has no
        // drawindexed of its own for it, so the reference is followed by 'handling = skip' and a
        // non-indexed 'draw = <vertex count>,0'. That is what the pure-Python BlendDownloadData
        // adds in its own addToPart override, and what downloadRefKVPs is for.
        return {{IniKeywords::Handling, "skip"},
                {IniKeywords::Draw, std::to_string(vertexCount) + ",0"}};
    }


    void DownloadStore::add(Downloads& downloads, const ModObj& modObj, const std::string& reg,
                             std::unique_ptr<Download> download) {
        downloads[modObj][reg] = download.get();
        owned_.push_back(std::move(download));
    }
}
