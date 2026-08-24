#include "AGRemapCore/model/strategies/texEditors/texFilters/HueAdjust.h"

#include <algorithm>
#include <cmath>

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/textures/Colour.h"

namespace AGRemapCore {

    namespace {
        // Converts an RGB colour (each channel 0-255) to an HSV colour, with H/S/V each also
        // encoded as a single byte (0-255) -- matching Pillow's own "HSV" image mode convention,
        // rather than the more common H in [0, 360) degrees.
        void rgbToHsvBytes(int r, int g, int b, int &h, int &s, int &v) {
            double rf = r / 255.0, gf = g / 255.0, bf = b / 255.0;
            double maxc = std::max({rf, gf, bf});
            double minc = std::min({rf, gf, bf});
            double delta = maxc - minc;

            double hDeg = 0.0;
            if (delta > 1e-9) {
                if (maxc == rf) {
                    hDeg = 60.0 * std::fmod((gf - bf) / delta, 6.0);
                } else if (maxc == gf) {
                    hDeg = 60.0 * (((bf - rf) / delta) + 2.0);
                } else {
                    hDeg = 60.0 * (((rf - gf) / delta) + 4.0);
                }
            }
            if (hDeg < 0.0) {
                hDeg += 360.0;
            }

            double sRatio = (maxc <= 1e-9) ? 0.0 : delta / maxc;

            h = Colour::boundColourChannel(static_cast<int>(std::lround(hDeg / 360.0 * 255.0)));
            s = Colour::boundColourChannel(static_cast<int>(std::lround(sRatio * 255.0)));
            v = Colour::boundColourChannel(static_cast<int>(std::lround(maxc * 255.0)));
        }

        // The inverse of rgbToHsvBytes.
        void hsvBytesToRgb(int h, int s, int v, int &r, int &g, int &b) {
            double hDeg = h / 255.0 * 360.0;
            double sRatio = s / 255.0;
            double vRatio = v / 255.0;

            double c = vRatio * sRatio;
            double x = c * (1.0 - std::fabs(std::fmod(hDeg / 60.0, 2.0) - 1.0));
            double m = vRatio - c;

            double rf, gf, bf;
            if (hDeg < 60.0) {
                rf = c; gf = x; bf = 0.0;
            } else if (hDeg < 120.0) {
                rf = x; gf = c; bf = 0.0;
            } else if (hDeg < 180.0) {
                rf = 0.0; gf = c; bf = x;
            } else if (hDeg < 240.0) {
                rf = 0.0; gf = x; bf = c;
            } else if (hDeg < 300.0) {
                rf = x; gf = 0.0; bf = c;
            } else {
                rf = c; gf = 0.0; bf = x;
            }

            r = Colour::boundColourChannel(static_cast<int>(std::lround((rf + m) * 255.0)));
            g = Colour::boundColourChannel(static_cast<int>(std::lround((gf + m) * 255.0)));
            b = Colour::boundColourChannel(static_cast<int>(std::lround((bf + m) * 255.0)));
        }
    }

    HueAdjust::HueAdjust(int hue): hue(hue) {}

    int HueAdjust::adjustedHue(int hueByte) const {
        int result = hueByte + hue;
        if (result > 360) {
            result = 360 - result;
        } else if (result < 0) {
            result += 255;
        }
        return Colour::boundColourChannel(result);
    }

    void HueAdjust::transform(TextureFile &texFile) {
        int width = texFile.getWidth();
        int height = texFile.getHeight();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Colour pixel = texFile.getPixel(x, y);

                int h, s, v;
                rgbToHsvBytes(pixel.red, pixel.green, pixel.blue, h, s, v);
                h = adjustedHue(h);

                int r, g, b;
                hsvBytesToRgb(h, s, v, r, g, b);

                pixel.red = r;
                pixel.green = g;
                pixel.blue = b;
                // alpha is left untouched
                texFile.setPixel(x, y, pixel);
            }
        }
    }
}
