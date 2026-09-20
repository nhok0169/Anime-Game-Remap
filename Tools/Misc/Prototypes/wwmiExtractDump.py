#
# ===== wwmiExtractDump =====
#
# Extracts a Wuthering Waves character's ASSET FOLDER from a frame dump -- the same Metadata.json,
# 'Component N.fmt' / '.vb' / '.ib', 'Components-... t=<hash>.dds' and TextureUsage.json that
# WWMI-Assets' PlayerCharacterData/<Name> holds -- for a character (or skin) WWMI-Assets does not have
# yet. wwmiIdentityMod.py and Tools/VGRemapFinder then read the result exactly as they read an
# asset folder.
#
#   py -3.11 wwmiExtractDump.py <FrameAnalysis folder> <output folder>
#   py -3.11 wwmiExtractDump.py <FrameAnalysis folder> <output folder> --addon "<path to WWMI-Tools>"
#
# It does not reimplement anything: it runs WWMI Tools' own "Extract Objects From Dump" (the Blender
# addon's extract_frame_data package, which is what WWMI-Assets is made with) outside Blender. That
# code touches Blender only to resolve a path and to report a configuration error, so 'bpy' and
# 'mathutils' are stubbed and the addon is imported as a package WITHOUT running its __init__.py
# (which registers Blender operators). The options are the addon panel's defaults: skip textures
# under 256 KB, skip .jpg, keep same-slot-hash textures.
#
# The addon writes one folder per vb0 hash the dump drew, named by that hash. A character with no
# vb0 of its own in the dump (a weapon, an effect) comes out as well; pick the character's folder by
# the component count and the shaders, or by the vb0 hash the frame-dump log shows for its draws.
#
# What it reproduces (2026-09-19, FrameAnalysis-Sanhua / -SanhuaExorcist of that day): the GEOMETRY
# exactly -- every 'Component N' .vb / .ib / .fmt byte-identical to WWMI-Assets' Sanhua, Metadata.json
# equal, and wwmiDownloadFolder.py --check over the result matches the shipped download folder on all
# nine buffers and the Metadata for both characters. NOT the textures: a dump holds each texture in
# whatever streaming state it was drawn in (most of Sanhua's at 512 x 512 where the asset has 2048),
# and a streamed texture's hash changes as its mips load, so not one of the 16 extracted hashes is
# WWMI-Assets' -- though 15 of them are pixel-identical (colour correlation >= 0.97) to an asset
# texture. Take textures from a dump made with every texture fully streamed in, and check their sizes.
#

import argparse
import importlib
import os
import re
import sys
import types
from pathlib import Path

DefaultAddon = os.path.join(os.environ.get("APPDATA", ""), "Blender Foundation", "Blender", "3.6", "scripts", "addons", "WWMI-Tools")
PackageName = "wwmitools"


def stubBlender():
    """'bpy' / 'mathutils' stand-ins that cover everything extract_frame_data reaches"""
    bpy = types.ModuleType("bpy")
    bpy.path = types.SimpleNamespace(abspath = lambda path: path)
    bpy.data = types.SimpleNamespace(filepath = "")
    bpy.context = types.SimpleNamespace(scene = types.SimpleNamespace(wwmi_tools_settings = types.SimpleNamespace()))
    sys.modules.setdefault("bpy", bpy)
    sys.modules.setdefault("mathutils", types.ModuleType("mathutils"))


def importAddon(addonPath: str):
    """the addon as a package, without executing its top-level __init__.py"""
    package = types.ModuleType(PackageName)
    package.__path__ = [addonPath]
    sys.modules[PackageName] = package
    extractor = importlib.import_module(f"{PackageName}.extract_frame_data.extract_frame_data")
    skipSubCalls(importlib.import_module(f"{PackageName}.migoto_io.dump_parser.dump_parser"))
    return extractor


# a dump's own files start with the six-digit call id, then '-'. A mod whose command list runs under
# a [ShaderRegex] (RabbitFX's, 2026-09-19) adds '<call>.<n>-[ShaderRegex_...]-ps-t0=<hash>.dsc'
# files for its sub-calls, which the addon's name parser rejects ("no call id detected") and which
# are never the character's draws
CallFilePattern = re.compile(r"^\d{6}-")


def skipSubCalls(dumpParser):
    class FilteredOs:
        def __getattr__(self, name):
            return getattr(os, name)

        @staticmethod
        def listdir(path):
            names = os.listdir(path)
            skipped = [name for name in names if (not CallFilePattern.match(name)) and os.path.isfile(os.path.join(path, name)) and not name.endswith("txt")]
            if (skipped):
                print(f"skipping {len(skipped)} file(s) that are not a call's own dump (e.g. '{skipped[0]}')")
            return [name for name in names if name not in skipped]

    dumpParser.os = FilteredOs()


def main():
    parser = argparse.ArgumentParser(description = "a WWMI-Assets-style asset folder per character, extracted from a WuWa frame dump by WWMI Tools' own extractor")
    parser.add_argument("dump", help = "the FrameAnalysis-* folder (must hold log.txt)")
    parser.add_argument("output", help = "the folder to write; gets one subfolder per vb0 hash")
    parser.add_argument("--addon", default = DefaultAddon, help = f"the WWMI-Tools addon folder (default: {DefaultAddon})")
    parser.add_argument("--minTextureKB", type = int, default = 256, help = "skip textures smaller than this (the addon's default, 256); 0 keeps all")
    parser.add_argument("--keepJpg", action = "store_true", help = "keep .jpg textures (the addon skips them by default)")
    args = parser.parse_args()

    if (not os.path.isfile(os.path.join(args.addon, "extract_frame_data", "extract_frame_data.py"))):
        raise SystemExit(f"no WWMI-Tools addon at '{args.addon}' (pass --addon)")

    stubBlender()
    extractor = importAddon(args.addon)

    cfg = types.SimpleNamespace(
        frame_dump_folder = str(Path(args.dump).resolve()),
        extract_output_folder = str(Path(args.output).resolve()),
        skip_small_textures = args.minTextureKB > 0,
        skip_small_textures_size = args.minTextureKB,
        skip_jpg_textures = not args.keepJpg,
        skip_same_slot_hash_textures = False,
    )
    extractor.extract_frame_data(cfg)

    for folder in sorted(Path(cfg.extract_output_folder).iterdir()):
        if (folder.is_dir()):
            components = len(list(folder.glob("Component *.vb")))
            textures = len(list(folder.glob("*.dds")))
            print(f"  {folder.name}: {components} components, {textures} textures")


if (__name__ == "__main__"):
    main()
