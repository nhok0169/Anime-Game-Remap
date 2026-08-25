#ifndef AGRemapCore_RemapIniResource_H
#define AGRemapCore_RemapIniResource_H

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/stats/RemapStats.h"
#include "AGRemapCore/tools/files/FileDownload.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Interface for a resource in a .ini file that's used by the overall remap process :raw-html:`<br />`
     :raw-html:`<br />`

     Mirrors the pure-Python ``RemapIniResourceMixin`` class
     (``model/iniresources/RemapIniResource.py``) -- every method here defaults to ``false`` (matches
     the Python original's own methods, which are all a bare ``pass`` -- an implicit ``None``, ie.
     falsy -- rather than raising ``NotImplementedError``), so a concrete class only needs to
     override whichever of these actually apply to it :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The Python original's own ``RemapIniFixResource.fixExists`` overrides this interface's
        ``fixExists(self, stats)`` with a **different** signature (``fixExists(self)``, no ``stats``
        parameter) -- a real mismatch only possible because Python doesn't enforce override
        signatures. #fixExists here keeps the one uniform ``stats``-taking signature the interface
        declares (also matching how ``RemapIniResource.fixExists`` actually *uses* ``stats``);
        :cpp:class:`RemapIniFixResource`'s own override just doesn't reference the parameter
     @endrst
     */
    class RemapIniResourceMixin {
        public:
            virtual ~RemapIniResourceMixin() = default;

            /**
             * @brief Determines whether the (unfixed) resource has previously encountered an error
             *
             * @param stats The stats tracked by the remap process
             *
             * @return Whether the resource has encountered an error
             */
            virtual bool srcEncounteredError(const RemapStats& stats) const;

            /**
             * @brief Determines whether the (unfixed) resource was already fixed
             *
             * @param stats The stats tracked by the remap process
             *
             * @return Whether the resource was already fixed
             */
            virtual bool srcIsFixed(const RemapStats& stats) const;

            /**
             * @brief Determines whether the fixed resource has previously encountered an error
             *
             * @param stats The stats tracked by the remap process
             *
             * @return Whether the fixed resource has encountered an error
             */
            virtual bool fixEncounteredError(const RemapStats& stats) const;

            /**
             * @brief Determines whether the fixed resource was already fixed
             *
             * @param stats The stats tracked by the remap process
             *
             * @return Whether the fixed resource was already fixed
             */
            virtual bool fixIsFixed(const RemapStats& stats) const;

            /**
             * @brief Determines whether the fixed resource already exists on disk
             *
             * @param stats The stats tracked by the remap process
             *
             * @return Whether the fixed resource already exists
             */
            virtual bool fixExists(const RemapStats& stats) const;

            /**
             * @brief Determines whether all the necessary data has been collected to fix this resource
             *
             * @return Whether all the required data is gathered
             */
            virtual bool hasRequired() const;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniResource` and :cpp:class:`RemapIniResourceMixin`

     Base class for some resource in a .ini file that's used by the overall remap process --
     mirrors the pure-Python ``RemapIniResource`` class (``model/iniresources/RemapIniResource.py``)
     @endrst
     */
    class RemapIniResource: public IniResource, public RemapIniResourceMixin {
        public:
            using IniResource::IniResource;

            bool hasRequired() const override;

            /**
             * @brief Determines whether the fixed resource already exists -- same as #srcIsFixed for this class
             */
            bool fixExists(const RemapStats& stats) const override;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniFixResource` and :cpp:class:`RemapIniResourceMixin`

     Base class for some resource to fix in a .ini file that's used by the overall remap process --
     mirrors the pure-Python ``RemapIniFixResource`` class (``model/iniresources/RemapIniResource.py``)
     @endrst
     */
    class RemapIniFixResource: public IniFixResource, public RemapIniResourceMixin {
        public:
            using IniFixResource::IniFixResource;

            bool hasRequired() const override;

            /**
             * @brief Determines whether the fixed resource already exists on disk at #fixedPath
             *
             * @param stats Unused -- see this file's own note on why #fixExists still takes it
             */
            bool fixExists(const RemapStats& stats) const override;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IniGroupedResource` and :cpp:class:`RemapIniResourceMixin`

     Base class for a group of resources to fix in a .ini file that's used by the overall remap
     process -- mirrors the pure-Python ``RemapIniGroupedResource`` class
     (``model/iniresources/RemapIniResource.py``)
     @endrst
     */
    class RemapIniGroupedResource: public IniGroupedResource, public RemapIniResourceMixin {
        public:
            using IniGroupedResource::IniGroupedResource;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniResource`

     Class for some download resource in a .ini file that's used by the overall remap process --
     mirrors the pure-Python ``RemapIniDownload`` class (``model/iniresources/RemapIniResource.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The Python original's own ``remapFix`` took a ``Mod`` purely to build
        ``downloadHandler``/``cacheHitHandler`` callbacks that called ``mod.print(...)`` -- per the
        maintainer's own direction (Mod is being removed from this API entirely), #remapFix instead
        takes those same handler callbacks directly from its caller. It also actually returns
        whether a fresh download occurred, unlike the Python original's own ``_fix``/``remapFix``,
        which have no ``return`` statement at all and so always implicitly return ``None`` --
        contradicting their own docstrings ("Whether the resource has been downloaded"/"Whether the
        resource was fixed"). There's no concrete example of the "correct" `None` return being relied
        upon anywhere, so this port implements the documented contract rather than the (almost
        certainly unintentional) always-`None` gap
     @endrst
     */
    class RemapIniDownload: public RemapIniResource {
        public:

            /**
             * @brief Constructs a new download resource
             *
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource
             * @param download The downloader associated with the file -- ownership is transferred into this resource
             * @param type The name for the type of resource
             * @param fixFunc
             @rst
             Custom function for fixing the resource, overriding #_fix if given -- takes this
             resource and the download stats to mutate, and returns whether a fresh download
             occurred (as opposed to a cache hit)
             @endrst
             */
            RemapIniDownload(const std::string& iniFolderPath, const std::string& srcPath, std::unique_ptr<FileDownload> download,
                              std::string type = "download", std::function<bool(RemapIniDownload&, CachedFileStats&)> fixFunc = nullptr);

            /**
             * @brief The downloader associated with the file
             */
            std::unique_ptr<FileDownload> download;

            /**
             * @brief Custom function for fixing the resource, overriding #_fix if set
             */
            std::function<bool(RemapIniDownload&, CachedFileStats&)> fixFunc;

            bool srcEncounteredError(const RemapStats& stats) const override;
            bool srcIsFixed(const RemapStats& stats) const override;
            bool fixEncounteredError(const RemapStats& stats) const override;
            bool fixIsFixed(const RemapStats& stats) const override;

            /**
             * @brief Determines whether the resource already exists -- same as #srcIsFixed for this class
             */
            bool fixExists(const RemapStats& stats) const override;

            /**
             * @brief Downloads the resource -- calls #fixFunc if set, otherwise #_fix
             *
             * @param downloadStats The stats for the file download to mutate
             * @param proxy The link to the proxy server used for any internet network requests made, if any
             *
             * @return Whether a fresh download occurred (``false`` for a cache hit)
             */
            bool fix(CachedFileStats& downloadStats, std::optional<std::string> proxy = std::nullopt);

            /**
             * @brief
             @rst
             Fixes the resource for the overall remap process -- same as #fix, additionally invoking
             'downloadHandler'/'cacheHitHandler' (whichever applies) with the resource's #srcPath
             once the download/cache-hit completes
             @endrst
             *
             * @param stats The stats tracked by the remap process
             * @param proxy The link to the proxy server used for any internet network requests made, if any
             * @param downloadHandler Called with #srcPath if a fresh download occurred
             * @param cacheHitHandler Called with #srcPath if the file was retrieved from the cache instead
             *
             * @return Whether a fresh download occurred (``false`` for a cache hit)
             */
            bool remapFix(RemapStats& stats, std::optional<std::string> proxy = std::nullopt,
                          const std::function<void(const std::string&)>& downloadHandler = nullptr,
                          const std::function<void(const std::string&)>& cacheHitHandler = nullptr);

        protected:

            /**
             * @brief
             @rst
             Downloads the resource via #download, moving it to #srcPath if it didn't already land
             there directly, and records the outcome in 'downloadStats'
             @endrst
             *
             * @param downloadStats The stats for the file download to mutate
             * @param proxy The link to the proxy server used for any internet network requests made, if any
             *
             * @return Whether a fresh download occurred (``false`` for a cache hit)
             */
            virtual bool _fix(CachedFileStats& downloadStats, std::optional<std::string> proxy);
    };
}

#endif
