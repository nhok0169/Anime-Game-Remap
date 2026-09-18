"""
Lifts the darkest part of a lightmap's alpha channel and saves it as a NEW file: every pixel whose
alpha is at or below the threshold gains the lift amount, everything brighter is left alone -- the
same edit the library's Jean -> JeanSea fix applies (AGRemapCore::JeanShading::liftLowAlpha, with
77 for both); Yelan's body AND head (ears) needed 128 for both on YelanTranquil.

The alpha is really a MATERIAL BAND (0 / 64-89 / 115-127 / 128 / 165-189 / 255 on these skins), each
selecting a shading ramp: Yelan's skin sits in 115..127, YelanTranquil's in 255, and Tranquil's 128 is
her sheer lace (dithered). A lift moves every band at once -- the 128 lift sent alpha 0 onto the lace
band, which is the "static" -- so --band moves one band only. In a GIMI lightmap
that channel drives how strongly the shader shades the surface; YelanTranquil's body shader reads
Yelan's lightmap as far paler skin, and a fully opaque lightmap restored the skin colour in a quick
test, so the near-black alpha is what is being lifted.

    py -3 LiftBodyLightMap.py                                  body + head lightmaps -> yelan{Body,Head}LightMapLifted128.dds
    py -3 LiftBodyLightMap.py --threshold 128 --lift 128       (the defaults; 77/77 is Jean's, 128/128 is what Yelan needed in game)
    py -3 LiftBodyLightMap.py --src X.dds --dest Y.dds
    py -3 LiftBodyLightMap.py --band 115 127 255 --suffix Skin255   only Yelan's skin band (115..127) -> Tranquil's (255)
    py -3 LiftBodyLightMap.py --band 115 127 255 --bandR 0 0 200 77 --suffix Skin255Metal
    py -3 LiftBodyLightMap.py --band 115 127 255 --bandDark 0 0 125 121 --band 0 0 177 --suffix Skin255Hair121Cloth177
                                                               skin -> 255; band 0 where the diffuse is dark (the hair) -> 121,
                                                               Tranquil's hair band; the rest of band 0 (dress, print, collar) -> 177
                                                               ... plus band 0 with R >= 200 (the mod's shiny collar / straps /
                                                               shoulder trims, a combination Tranquil never uses) -> her metal band
    py -3 LiftBodyLightMap.py --noCompress                     write uncompressed 32bpp instead of re-encoding to BC7 (fast)

Prints the alpha histogram before and after. MixedFilter.py binds the outputs (its Defaults["textures"]),
so regenerate the .ini after running this.
"""

import argparse
import os
import sys

APISrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Anime Game Remap (for all users)\api\src\py"
if (APISrc not in sys.path):
    sys.path.insert(0, APISrc)

Here = os.path.dirname(os.path.abspath(__file__))


def histogram(pixels: bytes, threshold: int):
    counts = [0] * 256
    for i in range(3, len(pixels), 4):
        counts[pixels[i]] += 1
    total = sum(counts)
    low = sum(counts[:threshold + 1])
    return total, low, min(i for i, c in enumerate(counts) if c), max(i for i, c in enumerate(counts) if c)


def lift(pixels: bytes, threshold: int, amount: int) -> bytes:
    out = bytearray(pixels)
    for i in range(3, len(out), 4):
        if (out[i] <= threshold):
            out[i] = min(255, out[i] + amount)
    return bytes(out)


def isHair(diffuse: bytes, i: int, lMax: int, navy: bool) -> bool:
    """whether the diffuse pixel starting at byte i is hair: dark (max channel <= lMax), or -- with navy -- blue-dominant
    and not light (b > r + 15, r < 90, g < 120, b > 50), which also catches the lighter strands of a blue-painted hair"""
    r, g, b = diffuse[i], diffuse[i + 1], diffuse[i + 2]
    if (navy):
        return (b > r + 15) and (r < 90) and (g < 120) and (b > 50)
    return max(r, g, b) <= lMax


def remapBands(pixels: bytes, bands, bandsR = (), bandsDark = (), diffuse: bytes = None, width: int = 0, dither = None, navy: bool = False) -> bytes:
    """alpha in [lo, hi] becomes `to`, per (lo, hi, to) band; a (lo, hi, rMin, to) rule in `bandsR` applies only
    where the pixel's R is at least rMin (R is the specular/metal mask); a (lo, hi, lMax, to) rule in `bandsDark`
    applies only where the DIFFUSE at that pixel is dark (max channel <= lMax: the hair, against a white dress);
    everything else is left alone. Dark rules win over R rules, which win over plain bands"""
    table = list(range(256))
    for lo, hi, to in bands:
        for a in range(lo, hi + 1):
            table[a] = to
    tableR = [None] * 256
    for lo, hi, rMin, to in bandsR:
        for a in range(lo, hi + 1):
            tableR[a] = (rMin, to)
    tableD = [None] * 256
    for lo, hi, lMax, to in bandsDark:
        for a in range(lo, hi + 1):
            tableD[a] = (lMax, to)
    if (bandsDark and (diffuse is None or len(diffuse) != len(pixels))):
        raise SystemExit("--bandDark needs the diffuse texture at the same size as the lightmap (--diffuse)")
    out = bytearray(pixels)
    for i in range(3, len(out), 4):
        a = out[i]
        dark = tableD[a]
        if (dark is not None and isHair(diffuse, i - 3, dark[0], navy)):
            if (dither is not None and width):
                p = i // 4
                out[i] = dither[(p % width + p // width) % 2]      # a checkerboard of the two targets, as the target skin's own hair is
            else:
                out[i] = dark[1]
            continue
        rule = tableR[a]
        if (rule is not None and out[i - 3] >= rule[0]):
            out[i] = rule[1]
        else:
            out[i] = table[a]
    return bytes(out)


if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description = __doc__, formatter_class = argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", nargs = "+", default = [os.path.join(Here, "yelanBodyLightMap.dds"), os.path.join(Here, "yelanHeadLightMap.dds")],
                        help = "the lightmap(s) to lift (default: the body and head lightmaps)")
    parser.add_argument("--dest", default = None, help = "where to write (single --src only); default: <src stem>Lifted<lift>.dds beside the source")
    parser.add_argument("--threshold", type = int, default = 128, help = "alpha at or below this is lifted (default: %(default)s)")
    parser.add_argument("--lift", type = int, default = 128, help = "how much it is lifted by (default: %(default)s)")
    parser.add_argument("--band", nargs = 3, type = int, action = "append", metavar = ("LO", "HI", "TO"), default = None,
                        help = "instead of the lift: alpha in LO..HI becomes TO (repeatable). The lightmap alpha is a material band, "
                               "e.g. --band 115 127 255 moves Yelan's skin band onto YelanTranquil's without touching the other bands")
    parser.add_argument("--bandR", nargs = 4, type = int, action = "append", metavar = ("LO", "HI", "RMIN", "TO"), default = None,
                        help = "like --band, but only where the pixel's R (the specular mask) is at least RMIN: --bandR 0 0 200 77 sends the "
                               "shiny trims Yelan keeps in band 0 to YelanTranquil's metal band")
    parser.add_argument("--bandDark", nargs = 4, type = int, action = "append", metavar = ("LO", "HI", "LMAX", "TO"), default = None,
                        help = "like --band, but only where the DIFFUSE is dark (max channel <= LMAX) -- the hair against a white dress: "
                               "--bandDark 0 0 125 121 sends the hair to YelanTranquil's hair band while --band 0 0 177 sends the rest of band 0 to silk")
    parser.add_argument("--diffuse", nargs = "+", default = None, help = "the diffuse texture(s) matching --src, for --bandDark (default: the source name with LightMap -> Diffuse)")
    parser.add_argument("--alpha", type = int, default = None, metavar = "VALUE",
                        help = "instead of any band rule: set the alpha of every pixel to VALUE (for a DIFFUSE: Tranquil's diffuse alpha is 0 "
                               "almost everywhere, Yelan's head texture is 255 everywhere and the mod's body diffuse carries faint strands)")
    parser.add_argument("--darkDither", nargs = 2, type = int, default = None, metavar = ("TO1", "TO2"),
                        help = "with --bandDark: instead of one target, a per-pixel checkerboard of TO1 and TO2 -- YelanTranquil's own hair "
                               "alternates bands 121 and 128 pixel by pixel")
    parser.add_argument("--rDark", nargs = 2, type = int, default = None, metavar = ("LMAX", "VALUE"),
                        help = "also set the lightmap R (the specular / highlight strength) to VALUE where the diffuse is dark (max channel <= LMAX): "
                               "dims the hair highlight, whose COLOUR is the target skin's own material constant")
    parser.add_argument("--alphaDark", nargs = 2, type = int, default = None, metavar = ("LMAX", "VALUE"),
                        help = "for a DIFFUSE: set the alpha to VALUE only where the pixel is dark (max channel <= LMAX) -- the hair. The mod's "
                               "crown hair comes from Yelan's head texture (alpha 255) and its back hair from the body texture (alpha 0), and "
                               "YelanTranquil's shader shades the two differently")
    parser.add_argument("--bDark", nargs = 2, type = float, default = None, metavar = ("LMAX", "FACTOR"),
                        help = "also scale the lightmap B (the painted hair-HIGHLIGHT mask: the zigzag marks) by FACTOR where the diffuse is dark "
                               "(max channel <= LMAX). The highlight's colour is the target skin's constant; its mask is ours")
    parser.add_argument("--navy", action = "store_true",
                        help = "the dark rules (--bandDark, --rDark, --bDark, --alphaDark) pick hair by HUE (blue-dominant, not light) instead of by "
                               "darkness -- a blue-painted hair has strands far lighter than any LMAX, which a darkness rule leaves on the cloth band")
    parser.add_argument("--suffix", default = None, help = "the output name is <src stem><suffix>.dds (default: Lifted<lift>, or Band<TO>... for --band)")
    parser.add_argument("--noCompress", action = "store_true", help = "write a plain 32bpp .dds instead of re-encoding to the source's BCn format")
    args = parser.parse_args()

    import FixRaidenBoss2 as FRB

    if (args.dest is not None and len(args.src) != 1):
        raise SystemExit("--dest goes with a single --src")

    suffix = args.suffix if (args.suffix is not None) else (f"AlphaDark{args.alphaDark[1]}" if (args.alphaDark is not None) else f"Alpha{args.alpha}" if (args.alpha is not None) else (f"Lifted{args.lift}" if (not args.band and not args.bandR and not args.bandDark) else "Band" + "_".join(f"{lo}-{hi}to{to}" for lo, hi, to in (args.band or [])) + "_".join(f"R{lo}-{hi}r{r}to{to}" for lo, hi, r, to in (args.bandR or []))))
    for src in args.src:
        dest = args.dest if (args.dest is not None) else os.path.join(os.path.dirname(os.path.abspath(src)), f"{os.path.splitext(os.path.basename(src))[0]}{suffix}.dds")
        tex = FRB.TextureFile(src)
        tex.open()
        if (not tex.hasImage):
            raise SystemExit(f"could not read '{src}'")
        print(f"source: {src}  {tex.width}x{tex.height}")

        pixels = tex.getPixels()
        total, low, lo, hi = histogram(pixels, args.threshold)
        print(f"  alpha before: {low} of {total} pixels at or below {args.threshold}; range {lo}..{hi}")

        diffusePixels = None
        if (args.bandDark):
            diffusePath = args.diffuse[args.src.index(src)] if (args.diffuse) else src.replace("LightMap", "Diffuse").replace("lightmap", "diffuse")
            dtex = FRB.TextureFile(diffusePath); dtex.open()
            if (not dtex.hasImage or dtex.width != tex.width or dtex.height != tex.height):
                raise SystemExit(f"diffuse '{diffusePath}' missing or not {tex.width}x{tex.height}")
            diffusePixels = dtex.getPixels()
        if ((args.rDark or args.bDark) and diffusePixels is None):
            diffusePath = args.diffuse[args.src.index(src)] if (args.diffuse) else src.replace("LightMap", "Diffuse").replace("lightmap", "diffuse")
            dtex = FRB.TextureFile(diffusePath); dtex.open(); diffusePixels = dtex.getPixels()
        if (args.alphaDark is not None):
            lMax, value = args.alphaDark; out = bytearray(pixels); n = 0
            for i in range(0, len(out), 4):
                if (isHair(out, i, lMax, args.navy)):
                    out[i + 3] = value; n += 1
            lifted = bytes(out); print(f"  alpha set to {value} on {n} dark pixels")
        elif (args.alpha is not None):
            out = bytearray(pixels); out[3::4] = bytes([args.alpha]) * (len(out) // 4); lifted = bytes(out)
        else:
            lifted = remapBands(pixels, args.band or [], args.bandR or [], args.bandDark or [], diffusePixels, tex.width, args.darkDither, args.navy) if (args.band or args.bandR or args.bandDark) else lift(pixels, args.threshold, args.lift)
        if (args.rDark):
            lMax, value = args.rDark; out = bytearray(lifted); n = 0
            for i in range(0, len(out), 4):
                if (isHair(diffusePixels, i, lMax, args.navy)):
                    out[i] = value; n += 1
            lifted = bytes(out); print(f"  lightmap R set to {value} on {n} dark-diffuse pixels")
        if (args.bDark):
            lMax, factor = args.bDark; out = bytearray(lifted); n = 0
            for i in range(0, len(out), 4):
                if (isHair(diffusePixels, i, int(lMax), args.navy)):
                    out[i + 2] = min(255, int(round(out[i + 2] * factor))); n += 1
            lifted = bytes(out); print(f"  lightmap B scaled by {factor} on {n} dark-diffuse pixels")
        total, low, lo, hi = histogram(lifted, args.threshold)
        changed = sum(1 for i in range(3, len(pixels), 4) if (pixels[i] != lifted[i]))
        print(f"  alpha after:  {low} of {total} pixels at or below {args.threshold}; range {lo}..{hi}; {changed} pixels changed")

        tex.setPixels(lifted, tex.width, tex.height)
        tex.src = dest
        tex.save(compress = not args.noCompress)
        print(f"wrote: {dest}  ({os.path.getsize(dest)} bytes)" if (os.path.isfile(dest)) else f"FAILED: '{dest}' was not written")
