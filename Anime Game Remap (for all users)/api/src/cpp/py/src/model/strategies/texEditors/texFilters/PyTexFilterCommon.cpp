#include "PyTexFilterCommon.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


AGRC::TextureFile& syncTextureFileFromImg(py::object texFileObj) {
    AGRC::TextureFile &texFile = texFileObj.cast<AGRC::TextureFile&>();
    py::object img = texFileObj.attr("img");

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
    py::object img = texFileObj.attr("img");

    // Symmetric with syncTextureFileFromImg's own early-out: if 'img' isn't being maintained,
    // leave it alone (still None) rather than forcing it into existence -- the transform's result
    // already lives in the native buffer, which is all TextureFile.save() needs afterward.
    if (img.is_none()) {
        return;
    }

    AGRC::TextureFile &texFile = texFileObj.cast<AGRC::TextureFile&>();
    const std::vector<std::uint8_t> &pixels = texFile.getPixels();

    py::bytes newBytes(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    texFileObj.attr("img").attr("frombytes")(newBytes);
}
