"""
Texture edits a remap between two skins needs, as functions over the API's ``TextureFile``
:raw-html:`<br />` :raw-html:`<br />`

A GIMI lightmap's alpha is not a brightness but a **material band**: a handful of discrete values,
each selecting a shading ramp of the skin being drawn, and the bands mean different things on
different skins (Yelan's skin is 115-127 and her hair is 0; YelanTranquil's skin is 255, her hair
115-128, her fur 0). Its blue channel carries the painted hair-highlight mask, coloured by the
target's hair constant. The diffuse alpha darkens in some shaders and is ignored in others. So
moving a mod from one skin to another is a **band table** --- source band to target band, with rows
conditioned on what the diffuse shows where two materials share a band on the source --- plus a
few channel edits. :func:`applyBandTable` is that table; :func:`hairMask` is the condition that
told hair from dress on the Yelan pair, by hue rather than darkness (a blue-painted hair has
strands lighter than any darkness cutoff, which a darkness rule leaves on the cloth band)
"""

from typing import Callable, Dict, List, Optional, Sequence, Tuple

import numpy as np


def readTexture(FRB, path: str):
    """
    Opens a texture through the API and hands back its pixels as an array

    Parameters
    ----------
    FRB: module
        The imported API

    path: :class:`str`
        The ``.dds``

    Returns
    -------
    Tuple[:class:`TextureFile`, :class:`numpy.ndarray`]
        The open texture and its ``(height, width, 4)`` RGBA bytes (a copy)
    """

    tex = FRB.TextureFile(path)
    tex.open()
    if (not tex.hasImage):
        raise FileNotFoundError(f"could not read texture '{path}'")
    pixels = np.frombuffer(tex.getPixels(), dtype = np.uint8).reshape(tex.height, tex.width, 4).copy()
    return tex, pixels


def writeTexture(tex, pixels: np.ndarray, dest: str, compress: bool = True) -> None:
    """
    Writes edited pixels back out as a new file, in the source's format by default

    Parameters
    ----------
    tex: :class:`TextureFile`
        The texture :func:`readTexture` opened (its size and format are reused)

    pixels: :class:`numpy.ndarray`
        The ``(height, width, 4)`` RGBA bytes

    dest: :class:`str`
        The file to write

    compress: :class:`bool`
        Re-encode to the source's BCn format (slow) or write 32bpp (fast)
    """

    tex.setPixels(np.ascontiguousarray(pixels, dtype = np.uint8).tobytes(), tex.width, tex.height)
    tex.src = dest
    tex.save(compress = compress)


def hairMask(rgb: np.ndarray, rMax: int = 90, gMax: int = 120, bMin: int = 50, blueOver: int = 15) -> np.ndarray:
    """
    Which pixels are a blue / navy hair, by hue: blue-dominant and not light

    Parameters
    ----------
    rgb: :class:`numpy.ndarray`
        ``(..., 3)`` or ``(..., 4)`` bytes

    rMax: :class:`int`
        Red at most this

    gMax: :class:`int`
        Green at most this

    bMin: :class:`int`
        Blue at least this

    blueOver: :class:`int`
        Blue exceeds red by at least this

    Returns
    -------
    :class:`numpy.ndarray`
        The mask
    """

    r, g, b = rgb[..., 0].astype(int), rgb[..., 1].astype(int), rgb[..., 2].astype(int)
    return (b > r + blueOver) & (r < rMax) & (g < gMax) & (b > bMin)


def darkMask(rgb: np.ndarray, lMax: int = 125) -> np.ndarray:
    """
    Which pixels are dark: max channel at most ``lMax``
    """

    return rgb[..., :3].max(axis = -1) <= lMax


class BandRule():
    """
    One row of a band table: alpha in ``[lo, hi]`` becomes ``to`` --- everywhere, or only where a
    mask over the diffuse holds

    Parameters
    ----------
    lo: :class:`int`
        The band's low end (inclusive)

    hi: :class:`int`
        The band's high end (inclusive)

    to: :class:`int`
        The target band

    where: Optional[Callable[[:class:`numpy.ndarray`], :class:`numpy.ndarray`]]
        A mask over the diffuse RGB (:func:`hairMask`, :func:`darkMask`); ``None`` for every pixel

    name: :class:`str`
        A label for the report
    """

    def __init__(self, lo: int, hi: int, to: int, where: Optional[Callable[[np.ndarray], np.ndarray]] = None, name: str = ""):
        self.lo, self.hi, self.to, self.where, self.name = lo, hi, to, where, name or f"{lo}-{hi} -> {to}"


def applyBandTable(lightmap: np.ndarray, rules: Sequence[BandRule], diffuse: Optional[np.ndarray] = None) -> Dict[str, int]:
    """
    Applies a band table to a lightmap's alpha in place. Rules are tried in order; the first
    whose band and mask match a pixel wins, so put the conditioned rows before the plain ones

    Parameters
    ----------
    lightmap: :class:`numpy.ndarray`
        ``(height, width, 4)`` bytes, edited in place

    rules: Sequence[:class:`BandRule`]
        The table

    diffuse: Optional[:class:`numpy.ndarray`]
        The diffuse at the same size, for the conditioned rows

    Returns
    -------
    Dict[:class:`str`, :class:`int`]
        Pixels each rule changed
    """

    alpha = lightmap[..., 3].astype(int)
    done = np.zeros(alpha.shape, dtype = bool)
    counts: Dict[str, int] = {}
    for rule in rules:
        m = (alpha >= rule.lo) & (alpha <= rule.hi) & ~done
        if (rule.where is not None):
            if (diffuse is None or diffuse.shape[:2] != lightmap.shape[:2]):
                raise ValueError(f"rule {rule.name!r} is conditioned on the diffuse, which is missing or not the lightmap's size")
            m &= rule.where(diffuse)
        lightmap[..., 3][m] = rule.to
        done |= m
        counts[rule.name] = int(m.sum())
    return counts


def scaleChannel(image: np.ndarray, channel: int, factor: float, where: Optional[np.ndarray] = None) -> int:
    """
    Scales one channel in place (``2`` = the lightmap's blue, the hair-highlight mask), optionally
    only under a mask

    Returns
    -------
    :class:`int`
        Pixels touched
    """

    m = np.ones(image.shape[:2], dtype = bool) if (where is None) else where
    image[..., channel][m] = np.clip(np.round(image[..., channel][m].astype(float) * factor), 0, 255).astype(np.uint8)
    return int(m.sum())


def setChannel(image: np.ndarray, channel: int, value: int, where: Optional[np.ndarray] = None) -> int:
    """
    Sets one channel in place (``3`` = alpha), optionally only under a mask

    Returns
    -------
    :class:`int`
        Pixels touched
    """

    m = np.ones(image.shape[:2], dtype = bool) if (where is None) else where
    image[..., channel][m] = value
    return int(m.sum())


def matchColour(image: np.ndarray, where: np.ndarray, reference: np.ndarray, referenceWhere: np.ndarray, strength: float = 1.0) -> Tuple[np.ndarray, np.ndarray]:
    """
    Moves the RGB of ``image`` under ``where`` onto the distribution (mean and spread per channel)
    of ``reference`` under ``referenceWhere``, in place --- the mod's body-painted hair onto its
    head-painted hair

    Returns
    -------
    Tuple[:class:`numpy.ndarray`, :class:`numpy.ndarray`]
        The source mean before and after
    """

    src = image[..., :3][where].astype(np.float64)
    ref = reference[..., :3][referenceWhere].astype(np.float64)
    if (not len(src) or not len(ref)):
        return np.zeros(3), np.zeros(3)
    matched = (src - src.mean(0)) * (ref.std(0) / np.maximum(src.std(0), 1e-6)) + ref.mean(0)
    out = src + (matched - src) * strength
    image[..., :3][where] = np.clip(np.round(out), 0, 255).astype(np.uint8)
    return src.mean(0), out.mean(0)


def uvCoverageMask(uv: np.ndarray, triangles: np.ndarray, width: int, height: int) -> np.ndarray:
    """
    Which texels a set of triangles actually samples --- their UV triangles rasterised --- so a
    statistic over "the hair" is taken over the hair this object draws, not over every navy
    pixel the texture holds (a mod may keep the source skin's whole texture on one half)

    Parameters
    ----------
    uv: :class:`numpy.ndarray`
        ``(vertices, 2)`` texture coordinates

    triangles: :class:`numpy.ndarray`
        ``(n, 3)`` vertex indices

    width: :class:`int`
        The texture's width

    height: :class:`int`
        The texture's height

    Returns
    -------
    :class:`numpy.ndarray`
        A ``(height, width)`` mask
    """

    from PIL import Image, ImageDraw

    image = Image.new("1", (width, height), 0)
    draw = ImageDraw.Draw(image)
    pts = np.stack([uv[:, 0] * width, uv[:, 1] * height], axis = 1)
    for t in triangles:
        draw.polygon([tuple(pts[i]) for i in t], fill = 1, outline = 1)
    return np.array(image, dtype = bool)
