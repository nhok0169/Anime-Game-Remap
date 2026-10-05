// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include "PyTexCache.h"

#include <cstddef>

#include "AGRemapCore/model/files/TexCache.h"

namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppTexCache(pybind11::module_ &m) {
    py::class_<AGRC::TexCache>(m, "TexCache", R"doc(
What a run has already decoded and already written, so the same source texture is not decoded twice
and an image written twice is copied the second time

:class:`RemapService` owns one of these and hands it to every texture resource as it goes, so an
ordinary run needs nothing here. It is exposed for the other caller: a prototype (see
``Tools/Misc/Prototypes/``) that drives :class:`TextureFile` directly rather than through the
service, which otherwise pays the full decode-and-encode for every duplicate

Both halves are keyed on **content**, and neither assumes anything about the filters:

- the **decode** side is keyed on the source file's bytes, which is a pure function of them --
  no filter has run at that point
- the **write** side is keyed on the finished pixel buffer. Every filter still runs, every time;
  only the encode-and-write of an image already written is replaced, with a file copy

That distinction is the whole design: a filter chain is an arbitrary callable and cannot be
inspected for what it reads, and at least one real filter (:class:`MaterialBandRemapFilter`) reads
a *second* file whose contents change the result

.. note::
    Not synchronized -- one is meant to live per run, walked sequentially. Sharing one across
    threads needs a lock adding to the C++ class first

Examples
--------
.. code-block:: python

    cache = TexCache()

    for src, dest in manyTextures:
        texFile = TextureFile(src)
        texFile.setCache(cache)
        texFile.open()
        texFile.save(compress = False)

    print(f"{cache.getWriteHits()} of those were already written")
    )doc")

        .def(py::init<std::size_t>(), py::arg("maxBytes") = AGRC::TexCache::DefaultMaxBytes,
              py::doc(R"doc(
Constructs a new texture cache

Parameters
----------
maxBytes: :class:`int`
    How many bytes of decoded pixels to hold before evicting the least recently used one.
    Decoded pixels are RGBA8, so a 4096x4096 texture is 64MB -- this is a byte budget rather than
    a count of textures for that reason. ``0`` disables the decode half and leaves only the write
    half :raw-html:`<br />` :raw-html:`<br />`

    **Default**: 256MB
        )doc"))

        .def("getDecodeHits", &AGRC::TexCache::getDecodeHits, py::doc(R"doc(
How many decodes this run skipped

Returns
-------
:class:`int`
    The number of times a source file's decoded pixels were already held
        )doc"))

        .def("getDecodeMisses", &AGRC::TexCache::getDecodeMisses, py::doc(R"doc(
How many decodes this run actually performed

Returns
-------
:class:`int`
    The number of source files that had to be decoded
        )doc"))

        .def("getWriteHits", &AGRC::TexCache::getWriteHits, py::doc(R"doc(
How many texture writes this run served with a file copy instead of an encode

Returns
-------
:class:`int`
    The number of writes whose pixels had already been written somewhere
        )doc"))

        .def("getWriteMisses", &AGRC::TexCache::getWriteMisses, py::doc(R"doc(
How many texture writes this run actually encoded

Returns
-------
:class:`int`
    The number of writes that really wrote a file
        )doc"))

        .def("getBytes", &AGRC::TexCache::getBytes, py::doc(R"doc(
How many bytes of decoded pixels are currently held

Returns
-------
:class:`int`
    The decode half's current size, which never exceeds the ``maxBytes`` it was built with
        )doc"));
}
