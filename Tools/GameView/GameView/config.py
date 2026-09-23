"""Where everything is: the scratch folder, the repo's image folder, the importers, the games.

Machine-specific paths are DISCOVERED (the XXMI Launcher's own config names every importer
folder and game folder) and cached in ``~/.claude/gameview/config.json``, which is the one file
to edit when a guess is wrong. The maintainer has two machines with different layouts, so nothing
here may assume one of them.
"""

import json
import os
import re
from pathlib import Path

# Paths off __file__, not the cwd: the helper runs this from a scheduled task.
TOOL_DIR = Path(__file__).resolve().parents[1]
REPO_ROOT = TOOL_DIR.parents[1]
MAIN_PY = TOOL_DIR / "main.py"
KEEP_ROOT = REPO_ROOT / "AI Agent Help" / "CreatingRemaps" / "Images"

SCRATCH = Path(os.environ.get("AGREMAP_GAMEVIEW_HOME", Path.home() / ".claude" / "gameview"))
SHOTS = SCRATCH / "shots"
CONFIG_FILE = SCRATCH / "config.json"
STATE_FILE = SCRATCH / "state.json"
HELPER_FILE = SCRATCH / "helper.json"
HELPER_LOG = SCRATCH / "helper.log"

TASK_NAME = "AGRemapGameViewHelper"

# What a game's process and window look like, per importer. Only the exe name identifies a game
# reliably: Genshin's window title is localised ("原神" on a CN client).
GAMES = {
    "GIMI": {"processes": ["GenshinImpact.exe", "YuanShen.exe"], "exe": "GenshinImpact.exe",
             "launch": "loader"},
    "WWMI": {"processes": ["Client-Win64-Shipping.exe"],
             "exe": "Client/Binaries/Win64/Client-Win64-Shipping.exe", "launch": "xxmi"},
    "SRMI": {"processes": ["StarRail.exe"], "exe": "StarRail.exe", "launch": "xxmi"},
    "ZZMI": {"processes": ["ZenlessZoneZero.exe"], "exe": "ZenlessZoneZero.exe", "launch": "xxmi"},
    "HIMI": {"processes": ["BH3.exe"], "exe": "BH3.exe", "launch": "xxmi"},
}

# Folders under an importer's Mods/ that are libraries other mods call into, never a mod under
# test. `mods only` leaves these loaded.
DEFAULT_PROTECTED = ["BufferValues", "TexFx*", "ORFix*", "NNFix*", "RabbitFX*", "ModManager*",
                     "Offset*", "__pycache__", "Core", "Libraries"]

# Folders in an importer's root that are the importer's own, not a parked mod.
IMPORTER_OWN_DIRS = {"mods", "core", "shadercache", "shaderfixes", "backups", "screenshots",
                     "__pycache__"}

XXMI_CANDIDATES = [
    os.environ.get("AGREMAP_XXMI_ROOT", ""),
    os.path.join(os.environ.get("APPDATA", ""), "XXMI Launcher"),
    r"E:\Computer\Games\Wuthering Waves Mods\Importer",
    r"C:\XXMI Launcher",
    r"D:\XXMI Launcher",
]


def ensureScratch():
    SHOTS.mkdir(parents=True, exist_ok=True)


def _readJson(path, default):
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return default


def _writeJson(path, data):
    ensureScratch()
    tmp = Path(str(path) + ".tmp")
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
    os.replace(tmp, path)


def findXxmiRoot():
    for candidate in XXMI_CANDIDATES:
        if candidate and os.path.isfile(os.path.join(candidate, "XXMI Launcher Config.json")):
            return candidate
    return ""


def discover(xxmiRoot=None):
    """A fresh config from the XXMI Launcher's own settings file."""
    root = xxmiRoot or findXxmiRoot()
    config = {"xxmiRoot": root, "importers": {}, "restoreFocus": True, "viewMax": 1568,
              "keepMax": 1920, "protectedMods": list(DEFAULT_PROTECTED), "defaultImporter": "GIMI",
              "uidStrip": 0.04}
    launcherCfg = _readJson(os.path.join(root, "XXMI Launcher Config.json"), {}) if root else {}
    importers = launcherCfg.get("Importers", {})
    active = launcherCfg.get("Launcher", {}).get("active_importer")
    if active:
        config["defaultImporter"] = active

    for name, game in GAMES.items():
        imp = importers.get(name, {}).get("Importer", {})
        folder = os.path.join(root, imp.get("importer_folder", name + "/")) if root else ""
        folder = os.path.normpath(folder) if folder else ""
        if not folder or not os.path.isdir(folder):
            continue
        gameFolder = imp.get("game_folder", "")
        gameExe = os.path.normpath(os.path.join(gameFolder, game["exe"])) if gameFolder else ""
        config["importers"][name] = {
            "folder": folder,
            "gameExe": gameExe,
            "processes": list(game["processes"]),
            # "loader": the importer's own 3DMigoto Loader.exe, then the game exe. The maintainer
            # launches GIMI this way because XXMI's injection is blocked by Genshin.
            # "xxmi": XXMI Launcher.exe --nogui --xxmi <name>.
            "launch": game["launch"],
        }
    return config


def load():
    config = _readJson(CONFIG_FILE, None)
    if config is None:
        config = discover()
        if config["xxmiRoot"]:
            _writeJson(CONFIG_FILE, config)
    return config


def save(config):
    _writeJson(CONFIG_FILE, config)


def loadState():
    return _readJson(STATE_FILE, {})


def saveState(state):
    _writeJson(STATE_FILE, state)


def launcherExe(config):
    root = config.get("xxmiRoot", "")
    path = os.path.join(root, "Resources", "Bin", "XXMI Launcher.exe")
    return path if os.path.isfile(path) else ""


# ---------------------------------------------------------------- d3dx.ini


def readD3dx(folder):
    """{section: {key: value}} of an importer's d3dx.ini, first value wins, comments dropped.
    Not a full 3DMigoto parser -- just enough for [Hunting], [Loader] and [Logging]."""
    result = {}
    section = None
    path = os.path.join(folder, "d3dx.ini")
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith(";"):
                    continue
                match = re.match(r"\[(.+)\]$", line)
                if match:
                    section = match.group(1).strip().lower()
                    result.setdefault(section, {})
                    continue
                if section is not None and "=" in line:
                    key, value = line.split("=", 1)
                    result[section].setdefault(key.strip().lower(), value.strip())
    except OSError:
        pass
    return result


def hotkey(folder, name, fallback):
    """The VK (and required modifiers) bound to a [Hunting] action, e.g. ``analyse_frame``."""
    from . import keys
    value = readD3dx(folder).get("hunting", {}).get(name)
    if value:
        try:
            vk, mods = keys.parseMigotoHotkey(value)
            if vk is not None:
                return vk, mods
        except ValueError:
            pass
    return keys.vkOf(fallback), []


def huntingMode(folder):
    """0 off, 1 on, 2 soft-disabled (on after ``toggle_hunting``). Frame analysis and
    show_original only respond while it is ON."""
    value = readD3dx(folder).get("hunting", {}).get("hunting", "0")
    try:
        return int(value.split(";")[0].strip())
    except ValueError:
        return 0
