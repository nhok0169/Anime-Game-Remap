"""Saving what was captured: a full-resolution PNG, a small VIEW copy for looking at, crops,
side-by-sides, and the copy kept in the repo for future agents."""

import os
import re
from pathlib import Path

from . import config as cfg
from .game import GameViewError, timestamp


def _scaled(image, maxSide):
    w, h = image.size
    longest = max(w, h)
    if not maxSide or longest <= maxSide:
        return image, 1.0
    from PIL import Image
    scale = longest / float(maxSide)
    size = (max(1, int(round(w / scale))), max(1, int(round(h / scale))))
    return image.resize(size, Image.LANCZOS), w / float(size[0])


def safeName(name):
    return re.sub(r"[^A-Za-z0-9_.-]+", "_", name).strip("_") or "shot"


def saveShot(image, name=None, viewMax=1568, out=None):
    """Write <name>.png (full) and <name>.view.png (at most ``viewMax`` on its long side).
    Returns (fullPath, viewPath, scale = full pixels per view pixel)."""
    cfg.ensureScratch()
    stem = "{}_{}".format(timestamp(), safeName(name)) if name else timestamp()
    folder = Path(out) if out else cfg.SHOTS
    folder.mkdir(parents=True, exist_ok=True)
    full = folder / (stem + ".png")
    image.save(str(full), compress_level=1)
    view, scale = _scaled(image, viewMax)
    viewPath = full
    if scale != 1.0:
        viewPath = folder / (stem + ".view.png")
        view.save(str(viewPath), compress_level=3)
    return full, viewPath, scale


def keep(image, relative, keepMax=1920):
    """Copy an image into AI Agent Help/CreatingRemaps/Images/<relative>.jpg for future agents.
    ``relative`` is e.g. ``Citlali/6_7/CitlaliHeadFix``: character, then game version, then a
    CamelCase name saying what the picture shows -- the folder's existing convention."""
    rel = relative.replace("\\", "/").strip("/")
    if rel.lower().endswith((".jpg", ".jpeg", ".png")):
        rel = os.path.splitext(rel)[0]
    if ".." in rel.split("/"):
        raise GameViewError("--keep must stay inside the Images folder")
    target = cfg.KEEP_ROOT / (rel + ".jpg")
    target.parent.mkdir(parents=True, exist_ok=True)
    scaled, _ = _scaled(image.convert("RGB"), keepMax)
    scaled.save(str(target), quality=88, optimize=True)
    return target


def _font(size):
    from PIL import ImageFont
    for name in ("arialbd.ttf", "arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def sideBySide(first, second, labels=("A", "B")):
    """Two images in one, each labelled. Stacked top / bottom when they are wider than tall
    (an ultrawide pair side by side shrinks to a strip nobody can read), else left / right."""
    from PIL import Image, ImageDraw
    if second.size != first.size:
        second = second.resize(first.size, Image.LANCZOS)
    w, h = first.size
    vertical = w > h * 1.2
    canvas = Image.new("RGB", (w, h * 2) if vertical else (w * 2, h), (0, 0, 0))
    offsets = ((0, 0), (0, h)) if vertical else ((0, 0), (w, 0))
    draw = ImageDraw.Draw(canvas)
    size = max(16, min(w, h) // 22)
    font = _font(size)
    for image, (x, y), label in zip((first, second), offsets, labels):
        canvas.paste(image, (x, y))
        box = draw.textbbox((0, 0), label, font=font)
        pad = size // 3
        draw.rectangle([x, y, x + box[2] + 2 * pad, y + box[3] + 2 * pad], fill=(0, 0, 0))
        draw.text((x + pad, y + pad), label, fill=(255, 230, 0), font=font)
    if vertical:
        draw.line([0, h, w, h], fill=(255, 230, 0), width=max(2, size // 8))
    return canvas


def openImage(path):
    from PIL import Image
    return Image.open(path).convert("RGB")
