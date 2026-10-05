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

#include "PyTexFilterCommon.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


// The pure-Python TextureFile keeps a Pillow image in 'img'; a CppTextureFile a fixer opens inside C++ (the one a
// config's diffuseEdits / TexEdit filter is handed) has no such attribute at all. Both mean the same thing: the
// native buffer is the source of truth. Before this, every filter routed through these helpers raised
// "'CppTextureFile' object has no attribute 'img'" there, and the fixer recorded the texture as skipped.
// (Not a ?: -- its common type would be py::none, and casting the Pillow image to that raises.)
static py::object imgOf(const py::object &texFileObj) {
    if (!py::hasattr(texFileObj, "img")) {
        return py::none();
    }
    return texFileObj.attr("img");
}


AGRC::TextureFile& syncTextureFileFromImg(py::object texFileObj) {
    AGRC::TextureFile &texFile = texFileObj.cast<AGRC::TextureFile&>();
    py::object img = imgOf(texFileObj);

    // 'img' is None whenever TextureFile isn't maintaining it (TexEngine.Compressonator +
    // readPillowImg == False) -- the native Compressonator buffer is already the up-to-date
    // source of truth in that case (populated by open()/a prior filter), so there's nothing to
    // pull in. This is what makes every filter routed through these two helpers buffer-native
    // (real C++ speed, zero Pillow overhead) whenever Pillow compatibility isn't actually needed.
    if (img.is_none()) {
        return texFile;
    }

    auto size = img.attr("size").cast<std::pair<int, int>>();
    std::string raw = img.attr("tobytes")().cast<py::bytes>();
    texFile.setPixels(std::vector<std::uint8_t>(raw.begin(), raw.end()), size.first, size.second);

    return texFile;
}

void syncTextureFileToImg(py::object texFileObj) {
    py::object img = imgOf(texFileObj);

    // Symmetric with syncTextureFileFromImg's own early-out: if 'img' isn't being maintained,
    // leave it alone (still None) rather than forcing it into existence -- the transform's result
    // already lives in the native buffer, which is all TextureFile.save() needs afterward.
    if (img.is_none()) {
        return;
    }

    AGRC::TextureFile &texFile = texFileObj.cast<AGRC::TextureFile&>();
    const std::vector<std::uint8_t> &pixels = texFile.getPixels();

    py::bytes newBytes(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    img.attr("frombytes")(newBytes);
}
