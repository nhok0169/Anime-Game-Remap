#include "PyRemapIniResource.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <pybind11/functional.h>
#include <pybind11/stl.h>

#include "AGRemapCore/model/iniresources/RemapIniResource.h"
#include "AGRemapCore/tools/files/FileDownload.h"
#include "../stats/PyRemapStats.h"
#include "../stats/PyCachedFileStats.h"
#include "../stats/PyStatsConversion.h"
#include "PyIniResource.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppRemapIniResourceMixin(pybind11::module_ &m) {
    // Bound once here -- srcEncounteredError/srcIsFixed/fixEncounteredError/fixIsFixed/fixExists
    // are genuine C++ virtuals, so a RemapIniResource/RemapIniFixResource/
    // RemapIniGroupedResource/RemapIniDownload instance calling any of these (via normal
    // Python attribute inheritance from this base) correctly dispatches to whichever override that
    // concrete class actually provides, through the real C++ vtable -- same "bind once on the base,
    // let real inheritance carry it to every derived registration" pattern as
    // BaseIniClassifier::classify()/clear() being inherited by IniClassifier without rebinding.
    //
    // Each lambda converts the caller's PyRemapStats into a throwaway AGRemapCore::RemapStats
    // snapshot first -- see PyStatsConversion.h's own note for why this conversion exists and why
    // it's safe for these specific (read-only, key-membership-only) methods.
    py::class_<AGRC::RemapIniResourceMixin, py::smart_holder>(m, "RemapIniResourceMixin", R"doc(
Interface for a resource in a .ini file that's used by the overall remap process
    )doc")

        .def(py::init<>())

        .def("srcEncounteredError", [](AGRC::RemapIniResourceMixin &self, const PyRemapStats &stats) {
            return self.srcEncounteredError(toCppRemapStats(stats));
        }, py::arg("stats"), py::doc(R"doc(
Determines whether the (unfixed) resource has previously encountered an error

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

Returns
-------
:class:`bool`
    Whether the resource has encountered an error
        )doc"))

        .def("srcIsFixed", [](AGRC::RemapIniResourceMixin &self, const PyRemapStats &stats) {
            return self.srcIsFixed(toCppRemapStats(stats));
        }, py::arg("stats"), py::doc(R"doc(
Determines whether the (unfixed) resource was already fixed

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

Returns
-------
:class:`bool`
    Whether the resource was already fixed
        )doc"))

        .def("fixEncounteredError", [](AGRC::RemapIniResourceMixin &self, const PyRemapStats &stats) {
            return self.fixEncounteredError(toCppRemapStats(stats));
        }, py::arg("stats"), py::doc(R"doc(
Determines whether the fixed resource has previously encountered an error

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

Returns
-------
:class:`bool`
    Whether the fixed resource has encountered an error
        )doc"))

        .def("fixIsFixed", [](AGRC::RemapIniResourceMixin &self, const PyRemapStats &stats) {
            return self.fixIsFixed(toCppRemapStats(stats));
        }, py::arg("stats"), py::doc(R"doc(
Determines whether the fixed resource was already fixed

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

Returns
-------
:class:`bool`
    Whether the fixed resource was already fixed
        )doc"))

        .def("fixExists", [](AGRC::RemapIniResourceMixin &self, const PyRemapStats &stats) {
            return self.fixExists(toCppRemapStats(stats));
        }, py::arg("stats"), py::doc(R"doc(
Determines whether the fixed resource already exists on disk

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

Returns
-------
:class:`bool`
    Whether the fixed resource already exists
        )doc"))

        .def("hasRequired", &AGRC::RemapIniResourceMixin::hasRequired, py::doc(R"doc(
Determines whether all the necessary data has been collected to fix this resource

Returns
-------
:class:`bool`
    Whether all the required data is gathered
        )doc"));
}

void initCppRemapIniResource(pybind11::module_ &m) {
    py::class_<AGRC::RemapIniResource, AGRC::IniResource, AGRC::RemapIniResourceMixin, py::smart_holder>(m, "RemapIniResource", R"doc(
This class inherits from :class:`IniResource` and :class:`RemapIniResourceMixin`

Base class for some resource in a .ini file that's used by the overall remap process
    )doc")

        .def(py::init<std::string, std::string, std::string>(), py::arg("type"), py::arg("iniFolderPath"), py::arg("srcPath"), py::doc(R"doc(
Constructs a new resource -- see :class:`IniResource`'s constructor for the parameters
        )doc"));
}

void initCppRemapIniFixResource(pybind11::module_ &m) {
    py::class_<AGRC::RemapIniFixResource, AGRC::IniFixResource, AGRC::RemapIniResourceMixin, py::smart_holder>(m, "RemapIniFixResource", R"doc(
This class inherits from :class:`IniFixResource` and :class:`RemapIniResourceMixin`

Base class for some resource to fix in a .ini file that's used by the overall remap process
    )doc")

        .def(py::init<std::string, std::string, std::string, std::string>(), py::arg("type"), py::arg("iniFolderPath"),
             py::arg("srcPath"), py::arg("fixedPath"), py::doc(R"doc(
Constructs a new resource to fix -- see :class:`IniFixResource`'s constructor for the parameters
        )doc"));
}

// initCppRemapIniGroupedResource moved to PyRemapIniGroupedResource.cpp -- it binds
// PyRemapIniGroupedResource (composing PyIniGroupedResource + RemapIniResourceMixin directly), not
// AGRC::RemapIniGroupedResource. See that file's own header comment for why.

void initCppRemapIniDownload(pybind11::module_ &m) {
    py::class_<AGRC::RemapIniDownload, AGRC::RemapIniResource, py::smart_holder>(m, "RemapIniDownload", R"doc(
This class inherits from :class:`RemapIniResource`

Class for some download resource in a .ini file that's used by the overall remap process --
unlike the deprecated pure-Python original, this class does not accept a ``Mod`` object anywhere --
:meth:`remapFix`'s progress-reporting callbacks ('downloadHandler'/'cacheHitHandler') are supplied
by the caller directly instead
    )doc")

        .def(py::init([](const std::string &iniFolderPath, const std::string &srcPath, py::object download,
                          std::string type, std::function<bool(AGRC::RemapIniDownload&, PyCachedFileStats&)> fixFunc) {
            std::function<bool(AGRC::RemapIniDownload&, AGRC::CachedFileStats&)> convertedFixFunc = nullptr;
            if (fixFunc) {
                // fixFunc is invoked from the real C++ _fix() call path with a plain
                // AGRemapCore::CachedFileStats -- wrap it in a throwaway PyCachedFileStats for the
                // Python callback (so it sees the same shape as every other stats parameter), then
                // copy any mutations back (see PyStatsConversion.h's own note).
                convertedFixFunc = [fixFunc](AGRC::RemapIniDownload &self, AGRC::CachedFileStats &downloadStats) {
                    PyCachedFileStats pyStats;
                    copyBackCachedFileStats(downloadStats, pyStats);
                    bool result = fixFunc(self, pyStats);
                    downloadStats = toCppCachedFileStats(pyStats);
                    return result;
                };
            }
            // py::init(factory) returning std::unique_ptr<T> by value -- same
            // neither-copyable-nor-movable reasoning as IniGroupedResource's own binding.
            return std::make_unique<AGRC::RemapIniDownload>(iniFolderPath, srcPath, download.cast<std::unique_ptr<AGRC::FileDownload>>(),
                                                              std::move(type), std::move(convertedFixFunc));
        }), py::arg("iniFolderPath"), py::arg("srcPath"), py::arg("download"), py::arg("type") = "download",
            py::arg("fixFunc") = py::none(), py::doc(R"doc(
Constructs a new download resource

Parameters
----------
iniFolderPath: :class:`str`
    The path to the folder of the .ini file

srcPath: :class:`str`
    The file path to the resource

download: :class:`FileDownload`
    The downloader associated with the file. Ownership is transferred into this resource

type: :class:`str`
    The name for the type of resource

    **Default**: ``"download"``

fixFunc: Optional[Callable[[:class:`RemapIniDownload`, :class:`CachedFileStats`], :class:`bool`]]
    Custom function for fixing the resource, overriding the default download behavior if given --
    takes this resource and the download stats to mutate, and returns whether a fresh download
    occurred (as opposed to a cache hit)

    **Default**: ``None``
        )doc"))

        .def("fix", [](AGRC::RemapIniDownload &self, PyCachedFileStats &downloadStats, std::optional<std::string> proxy) {
            AGRC::CachedFileStats cppStats = toCppCachedFileStats(downloadStats);
            bool result = self.fix(cppStats, proxy);
            copyBackCachedFileStats(cppStats, downloadStats);
            return result;
        }, py::arg("downloadStats"), py::arg("proxy") = py::none(), py::doc(R"doc(
Downloads the resource -- calls the custom 'fixFunc' if set at construction, otherwise the default
download behavior

Parameters
----------
downloadStats: :class:`CachedFileStats`
    The stats for the file download to mutate

proxy: Optional[:class:`str`]
    The link to the proxy server used for any internet network requests made, if any

    **Default**: ``None``

Returns
-------
:class:`bool`
    Whether a fresh download occurred (``False`` for a cache hit)
        )doc"))

        .def("remapFix", [](AGRC::RemapIniDownload &self, PyRemapStats &stats, std::optional<std::string> proxy,
                             std::function<void(const std::string&)> downloadHandler,
                             std::function<void(const std::string&)> cacheHitHandler) {
            AGRC::RemapStats cppStats = toCppRemapStats(stats);
            bool result = self.remapFix(cppStats, proxy, downloadHandler, cacheHitHandler);
            copyBackRemapStats(cppStats, stats);
            return result;
        }, py::arg("stats"), py::arg("proxy") = py::none(), py::arg("downloadHandler") = py::none(),
           py::arg("cacheHitHandler") = py::none(), py::doc(R"doc(
Fixes the resource for the overall remap process -- same as :meth:`fix`, additionally invoking
'downloadHandler'/'cacheHitHandler' (whichever applies) with this resource's ``srcPath`` once the
download/cache-hit completes

Parameters
----------
stats: :class:`RemapStats`
    The stats tracked by the remap process

proxy: Optional[:class:`str`]
    The link to the proxy server used for any internet network requests made, if any

    **Default**: ``None``

downloadHandler: Optional[Callable[[:class:`str`], Any]]
    Called with 'srcPath' if a fresh download occurred

    **Default**: ``None``

cacheHitHandler: Optional[Callable[[:class:`str`], Any]]
    Called with 'srcPath' if the file was retrieved from the cache instead

    **Default**: ``None``

Returns
-------
:class:`bool`
    Whether a fresh download occurred (``False`` for a cache hit)
        )doc"));
}
