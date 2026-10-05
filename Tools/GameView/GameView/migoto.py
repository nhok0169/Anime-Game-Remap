"""The importer side: reloading, frame analysis, the 3DMigoto log, and which mods are loaded."""

import fnmatch
import json
import os
import re
import time
from pathlib import Path

from . import config as cfg
from . import win32
from .game import GameViewError

LOG_NAME = "d3d11_log.txt"
PROBLEM = re.compile(r"(?i)^\s*(warning|error)\b|\b(failed|could not|cannot|unrecognised|"
                     r"unrecognized|invalid|missing)\b")
# Printed on every reload of an install with no ShaderFixes overrides: noise, not a problem.
NOISE = re.compile(r"(?i)^\s*Reverting \w+ not found in ShaderFixes|^If this is intentional|"
                   r"^\s+allow_duplicate_hash")
# 3DMigoto logs each section's name as it parses it, so a warning belongs to the last one:
# [TextureOverride\Mods\Bennett7\gb_bennett\merged.ini\BennettBody]
SECTION = re.compile(r"^\[(\w+)\\(.+)\]\s*$")
# ...except a duplicate hash, which is one warning line FOLLOWED by the name of every section
# carrying that hash (in any mod), in the same format as a section being parsed:
#   WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=e35ce2c4
#   [TextureOverride\Mods\CharlotteIdentity\Charlotte.ini\CharlottePosition...]
#   [TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlottePosition...]
#   If this is intentional, add a match_priority=n to suppress warning and disambiguate order
# The warning belongs to every listed section, and the listed names are not sections being parsed.
CONFLICT = re.compile(r"(?i)^\s*WARNING:\s*Possible Mod Conflict\b")
CONFLICT_END = re.compile(r"(?i)^If this is intentional\b")


def importerFolder(config, importer):
    info = config.get("importers", {}).get(importer)
    if not info:
        raise GameViewError("unknown importer {!r}; configured: {}".format(
            importer, ", ".join(config.get("importers", {})) or "none (run `setup`)"))
    return info["folder"]


# ---------------------------------------------------------------- the log


def logPath(folder):
    return os.path.join(folder, LOG_NAME)


def logSize(folder):
    try:
        return os.path.getsize(logPath(folder))
    except OSError:
        return 0


def readLogFrom(folder, offset, cap=32 * 1024 * 1024):
    """The text appended to d3d11_log.txt since ``offset`` (at most the last ``cap`` bytes of it:
    with call logging on the log grows by megabytes a second, and has reached 91 GB)."""
    path = logPath(folder)
    try:
        size = os.path.getsize(path)
    except OSError:
        return ""
    if size < offset:  # the game restarted and 3DMigoto truncated it
        offset = 0
    start = max(offset, size - cap)
    with open(path, "rb") as f:
        f.seek(start)
        return f.read(size - start).decode("utf-8", "replace")


RELOAD_MARK = "Reloading d3dx.ini"
# Logged once every section has been parsed; every section header and warning of a reload comes
# before it (checked on all 15 reloads of the GIMI log, 2026-09-23). The reload goes on to patch
# shaders, but that tail may never reach the file: with d3dx.ini's [Logging] unbuffered=0 the last
# few KB sit in 3DMigoto's buffer until something else is logged -- typically the NEXT reload.
# "The log went quiet" is not the end either: 3DMigoto loads every Resource file silently between
# logging the [Resource...] and the [TextureOverride...] sections, and a 1.5 s quiet window there
# cut CharlotteIdentity's report off before any of its TextureOverride warnings.
RELOAD_DONE = re.compile(r"(?m)^> d3dx\.ini reloaded\b")
# "Frame analysis saved to ..." / "Frame Analysis: Unable to create ..." -- WITH the space. With call
# logging on (WWMI's XXMI default) every log line starts "FrameAnalysisContext(...)", which a
# space-optional pattern matched, so a dump that never started looked like one in progress.
DUMP_START = re.compile(r"(?im)^Frame analysis\b")
DUMP_SAVED = re.compile(r"(?im)^Frame analysis saved to (.+?)\s*$")


def waitLog(folder, offset, until=None, quiet=1.5, minimum=1.0, maximum=30.0):
    """Wait for the log after a hotkey: until ``until`` matches the appended text, or (with no
    ``until``) until it has stopped growing for ``quiet`` seconds. Returns the appended text.
    A reload of a big Mods folder takes several seconds, and anything pressed or read before it
    ends is lost -- a frame dump pressed mid-reload never happens."""
    start = time.time()
    lastSize, lastChange = logSize(folder), time.time()
    while True:
        elapsed = time.time() - start
        size = logSize(folder)
        if size != lastSize:
            lastSize, lastChange = size, time.time()
        if elapsed >= minimum:
            if until is not None:
                if until(readLogFrom(folder, offset)):
                    break
            elif time.time() - lastChange >= quiet:
                break
        if elapsed >= maximum:
            break
        time.sleep(0.25)
    return readLogFrom(folder, offset)


def tailLog(folder, lines=60, cap=4 * 1024 * 1024):
    size = logSize(folder)
    text = readLogFrom(folder, max(0, size - cap), cap)
    return text.splitlines()[-lines:]


def _sectionName(header):
    return header.group(1) + "\\" + header.group(2)


def inMod(section, mod):
    """Whether a section path (``TextureOverride\\Mods\\<folder>\\...``) is under
    ``Mods\\<mod>`` (a case-insensitive glob over the mod's folder name)."""
    parts = section.split("\\")
    folder = parts[2] if len(parts) > 2 and parts[1].lower() == "mods" else ""
    return fnmatch.fnmatch(folder.lower(), mod.lower())


def _entries(text):
    """(warning line, section) for every problem line, and the set of sections parsed. A
    duplicate-hash warning yields one entry per section it lists (see CONFLICT)."""
    lines = text.splitlines()
    entries, parsed = [], set()
    section = ""
    i = 0
    while i < len(lines):
        line = lines[i]
        i += 1
        header = SECTION.match(line)
        if header:
            section = _sectionName(header)
            parsed.add(section)
            continue
        if not PROBLEM.search(line) or NOISE.search(line):
            continue
        if CONFLICT.search(line):
            listed = []
            while i < len(lines) and SECTION.match(lines[i]):
                listed.append(_sectionName(SECTION.match(lines[i])))
                i += 1
            if listed and not (i < len(lines) and CONFLICT_END.search(lines[i])):
                # The list did not end the way 3DMigoto ends it, so its last name may be the next
                # section being parsed: keep it as the context, as the plain loop would have.
                section = listed[-1]
                parsed.add(section)
            for name in listed or [section]:
                entries.append((line, name))
            continue
        entries.append((line, section))
    return entries, parsed


def problems(text, limit=60, mod=None):
    """[(warning line, section it was logged under or "")], de-duplicated per section, in order.
    3DMigoto reports a bad .ini line (an unknown key, a line outside any section, a duplicate
    hash) here and in orange on screen. ``mod`` keeps only warnings under ``Mods\\<mod>``
    (a glob over the mod's folder name). A duplicate hash is reported under every section that
    carries it, so a conflict between two mods shows up under both."""
    seen = set()
    result = []
    for line, section in _entries(text)[0]:
        if mod is not None and not inMod(section, mod):
            continue
        key = (re.sub(r"\d+", "#", line.strip()), section)
        if key in seen:
            continue
        seen.add(key)
        result.append((line.rstrip(), section))
        if len(result) >= limit:
            break
    return result


# Logged as each .ini is read:  Processing "E:\...\GIMI\Mods\BufferValues\ORFix.ini"
PROCESSING = re.compile(r'(?m)^\s*Processing "(.+?\.ini)"\s*$')


def iniFilesRead(text, mod):
    """The .ini files under ``Mods\\<mod>`` that 3DMigoto logged reading. Files read but no
    section parsed means the .ini sets ``namespace =``: its sections are logged under the
    namespace (``[Resource\\global\\ORFix\\...]``), which --mod cannot attribute to a folder."""
    found = []
    for path in PROCESSING.findall(text):
        parts = path.replace("/", "\\").split("\\")
        lowered = [p.lower() for p in parts]
        if "mods" in lowered:
            at = lowered.index("mods")  # the importer's Mods; a mod may have its own "Mods"
            if at + 1 < len(parts) - 1 and fnmatch.fnmatch(lowered[at + 1], mod.lower()):
                found.append(path)
    return sorted(set(found))


def sectionsParsed(text, mod=None):
    """The sections 3DMigoto logged as parsed in ``text`` (under ``Mods\\<mod>`` when given).
    Empty means the text never reached the mod's sections: "no warnings" would then be a report
    about nothing, not a pass."""
    parsed = _entries(text)[1]
    return {s for s in parsed if mod is None or inMod(s, mod)}


# ---------------------------------------------------------------- frame analysis


def dumpFolders(folder):
    try:
        return {e.name for e in os.scandir(folder)
                if e.is_dir() and e.name.startswith("FrameAnalysis")}
    except OSError:
        return set()


def folderStats(path):
    count = size = 0
    for root, _, files in os.walk(path):
        for name in files:
            count += 1
            try:
                size += os.path.getsize(os.path.join(root, name))
            except OSError:
                pass
    return count, size


def drawCalls(path):
    ids = set()
    try:
        for entry in os.scandir(path):
            match = re.match(r"(\d{6})[-.]", entry.name)
            if match:
                ids.add(match.group(1))
    except OSError:
        pass
    return len(ids)


def waitForDump(folder, before, offset, press, hwnd, appearTimeout, settle, totalTimeout,
                progress=None, retries=2, retryAfter=8.0):
    """After ``press()`` (the analyse_frame hotkey): wait for the dump to START -- a new
    FrameAnalysis folder or a "Frame analysis" log line -- pressing again up to ``retries`` times
    if nothing reacts (a press during a reload, or during a frame 3DMigoto skipped, is lost); then
    wait for it to FINISH, which 3DMigoto logs as "Frame analysis saved to <folder>". The
    folder-stopped-growing test is only the fallback for a log that says nothing.
    Returns the new folder's name, or None if no dump ever started."""
    start = time.time()
    lastPress = start
    newName = None
    while time.time() - start < appearTimeout:
        new = dumpFolders(folder) - before
        if new:
            newName = sorted(new)[-1]
            break
        if DUMP_START.search(readLogFrom(folder, offset, cap=1024 * 1024)):
            time.sleep(0.5)
            continue
        if retries > 0 and time.time() - lastPress >= retryAfter:
            press()
            retries -= 1
            lastPress = time.time()
        time.sleep(0.5)
    if not newName:
        return None

    path = os.path.join(folder, newName)
    last = None
    stableSince = time.time()
    while time.time() - start < totalTimeout:
        time.sleep(2.0)
        saved = DUMP_SAVED.search(readLogFrom(folder, offset))
        stats = folderStats(path)
        if progress and stats != last:
            progress(stats)
        if saved:
            return os.path.basename(saved.group(1).rstrip("\\/")) or newName
        hung = hwnd and win32.user32.IsWindow(hwnd) and win32.user32.IsHungAppWindow(hwnd)
        if stats != last or hung:
            last = stats
            stableSince = time.time()
        elif time.time() - stableSince >= settle:
            return newName
    raise GameViewError("{} was still being written after {}s (pass a larger --timeout)".format(
        newName, totalTimeout))


def labelDump(folder, name, label):
    """FrameAnalysis-2026-09-22-053142 -> FrameAnalysis-<label>-2026-09-22-053142, the naming the
    maintainer's own dumps use. Retries: 3DMigoto can hold a handle for a moment after writing."""
    if not label:
        return name
    stamp = name[len("FrameAnalysis-"):] if name.startswith("FrameAnalysis-") else name
    newName = "FrameAnalysis-{}-{}".format(re.sub(r"[^A-Za-z0-9_.]+", "", label), stamp)
    for _ in range(20):
        try:
            os.rename(os.path.join(folder, name), os.path.join(folder, newName))
            return newName
        except OSError:
            time.sleep(0.5)
    return name


def setAnalyseOptions(folder, options):
    """Swap the [Hunting] analyse_options line; returns the original bytes to restore. Bytes in,
    bytes out, so the file's line endings and encoding are untouched."""
    path = os.path.join(folder, "d3dx.ini")
    with open(path, "rb") as f:
        original = f.read()
    pattern = re.compile(rb"(?m)^([ \t]*analyse_options[ \t]*=)[^\r\n]*")
    if len(pattern.findall(original)) != 1:
        raise GameViewError("expected exactly one active analyse_options line in " + path)
    patched = pattern.sub(lambda m: m.group(1) + b" " + options.encode("utf-8"), original)
    with open(path, "wb") as f:
        f.write(patched)
    return original


def restoreFile(folder, original):
    with open(os.path.join(folder, "d3dx.ini"), "wb") as f:
        f.write(original)


# ---------------------------------------------------------------- mods


def _journalPath(importer):
    return cfg.SCRATCH / "mods-journal-{}.json".format(importer)


def _journal(importer):
    try:
        with open(_journalPath(importer), encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return []


def _saveJournal(importer, entries):
    cfg.ensureScratch()
    with open(_journalPath(importer), "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=1, ensure_ascii=False)


def _dirs(path, skipOwn=False):
    if not path or not os.path.isdir(path):
        return []
    return sorted(e.name for e in os.scandir(path)
                  if e.is_dir() and not e.name.startswith("FrameAnalysis")
                  and not (skipOwn and e.name.lower() in cfg.IMPORTER_OWN_DIRS))


def listMods(folder, source=None):
    """(loaded, available): folders under Mods/, and the mod folders that can be loaded -- by
    default the ones parked one level up (the maintainer's convention: keep only what is under
    test in Mods and park the rest in the importer's own folder), or those in ``source``, e.g. a
    folder of every downloaded mod of one character."""
    loaded = _dirs(os.path.join(folder, "Mods"))
    available = _dirs(source or folder, skipOwn=not source)
    return loaded, available


def _sameVolume(a, b):
    return os.path.splitdrive(os.path.abspath(a))[0].lower() == \
        os.path.splitdrive(os.path.abspath(b))[0].lower()


def _move(importer, src, dst, entries):
    if os.path.exists(dst):
        raise GameViewError("{} already exists; not overwriting it".format(dst))
    if not _sameVolume(src, dst):
        # A cross-drive "move" is a copy plus a DELETE of the maintainer's original. Refuse it.
        raise GameViewError("{} is on another drive than {}: GameView only renames, never "
                            "copies and deletes. Ask for the mods folder to be on the "
                            "importer's drive.".format(src, dst))
    for _ in range(10):
        try:
            os.rename(src, dst)
            break
        except PermissionError:
            time.sleep(0.5)
    else:
        raise GameViewError("could not move {} (in use?)".format(src))
    entries.append({"from": src, "to": dst, "time": time.strftime("%Y-%m-%d %H:%M:%S")})
    _saveJournal(importer, entries)


def _origin(entries, loadedPath, default):
    """Where a folder now in Mods/ came from, by the newest journal entry that put it there."""
    for entry in reversed(entries):
        if os.path.normcase(entry["to"]) == os.path.normcase(loadedPath):
            return entry["from"]
    return default


def _resolve(names, available):
    """Exact names or globs against the available folder names (case-insensitive)."""
    chosen = []
    for name in names:
        hits = [a for a in available if fnmatch.fnmatch(a.lower(), name.lower())]
        if not hits:
            raise GameViewError("no mod folder matches {!r}".format(name))
        chosen.extend(h for h in hits if h not in chosen)
    return chosen


def _park(importer, folder, name, entries):
    here = os.path.join(folder, "Mods", name)
    there = _origin(entries, here, os.path.join(folder, name))
    _move(importer, here, there, entries)
    return there


def loadMods(importer, folder, names, source=None):
    _, available = listMods(folder, source)
    entries = _journal(importer)
    moved = []
    for name in _resolve(names, available):
        _move(importer, os.path.join(source or folder, name), os.path.join(folder, "Mods", name),
              entries)
        moved.append(name)
    return moved


def parkMods(importer, folder, names):
    """Out of Mods/, back to wherever GameView loaded each one from (the importer folder when it
    did not load it)."""
    loaded, _ = listMods(folder)
    entries = _journal(importer)
    return ["{} -> {}".format(name, _park(importer, folder, name, entries))
            for name in _resolve(names, loaded)]


def onlyMods(importer, folder, names, protected, source=None):
    """Load exactly ``names`` (plus the protected libraries); park every other loaded mod back
    where it came from. The step of a "test every mod of this character" loop."""
    loaded, available = listMods(folder, source)
    wanted = _resolve(names, loaded + available)
    entries = _journal(importer)
    parkedNow, loadedNow, kept = [], [], []
    for name in loaded:
        if name in wanted:
            continue
        if any(fnmatch.fnmatch(name.lower(), p.lower()) for p in protected):
            kept.append(name)
            continue
        _park(importer, folder, name, entries)
        parkedNow.append(name)
    for name in wanted:
        if name in available and name not in loaded:
            _move(importer, os.path.join(source or folder, name),
                  os.path.join(folder, "Mods", name), entries)
            loadedNow.append(name)
    return parkedNow, loadedNow, kept


def restoreMods(importer):
    """Undo every move GameView made, newest first, and empty the journal."""
    entries = _journal(importer)
    undone, failed = [], []
    while entries:
        entry = entries.pop()
        if os.path.exists(entry["to"]) and not os.path.exists(entry["from"]):
            try:
                os.rename(entry["to"], entry["from"])
                undone.append(os.path.basename(entry["from"]))
            except OSError as e:
                failed.append("{}: {}".format(entry["to"], e))
        else:
            failed.append("{} (already moved by hand?)".format(entry["to"]))
        _saveJournal(importer, entries)
    return undone, failed


_OVERRIDE = re.compile(r"^\s*\[\s*TextureOverride([A-Za-z]+?)(?=[A-Z0-9_]|\])", re.MULTILINE)
_HASH = re.compile(r"^\s*hash\s*=\s*([0-9a-fA-F]{8})\b", re.MULTILINE)
_FIXED = re.compile(r"(?i)remap|RemapFix")


def scanMods(source):
    """One row per mod folder in ``source``: its .ini count, whether it already carries a remap
    fix, and a CHARACTER HINT -- the commonest first word of its TextureOverride section names.
    Only a hint (habit: a label is not the thing it names -- modders name sections after the base
    character while building on a skin); the fix run's own classification is the authority."""
    rows = []
    for name in _dirs(source):
        path = os.path.join(source, name)
        inis, words, hashes, fixed = 0, {}, set(), False
        for root, dirs, files in os.walk(path):
            dirs[:] = [d for d in dirs if not d.upper().startswith("DISABLED")]
            for file in files:
                if not file.lower().endswith(".ini") or file.upper().startswith("DISABLED"):
                    continue
                inis += 1
                if _FIXED.search(file):
                    fixed = True
                try:
                    with open(os.path.join(root, file), encoding="utf-8", errors="replace") as f:
                        text = f.read(2 * 1024 * 1024)
                except OSError:
                    continue
                for word in _OVERRIDE.findall(text):
                    words[word] = words.get(word, 0) + 1
                hashes.update(h.lower() for h in _HASH.findall(text))
        hint = max(words, key=words.get) if words else "?"
        rows.append({"mod": name, "inis": inis, "hint": hint, "hashes": len(hashes),
                     "fixed": fixed})
    return rows


# ---------------------------------------------------------------- scratch


def cleanShots(days):
    cutoff = time.time() - days * 86400
    removed = 0
    for path in Path(cfg.SHOTS).glob("*"):
        if path.is_file() and path.stat().st_mtime < cutoff:
            path.unlink()
            removed += 1
    return removed


def launchLoader(folder, gameExe):
    """GIMI the maintainer's way: the importer's own 3DMigoto Loader.exe first (it waits for the
    game's process and injects d3d11.dll), then the game exe. Both need elevation, so this runs
    in the helper."""
    import subprocess
    loader = os.path.join(folder, "3DMigoto Loader.exe")
    if not os.path.isfile(loader):
        raise GameViewError("no 3DMigoto Loader.exe in " + folder)
    if not os.path.isfile(gameExe):
        raise GameViewError("game exe not found: {} (fix gameExe in {})".format(gameExe,
                                                                              cfg.CONFIG_FILE))
    subprocess.Popen([loader], cwd=folder, creationflags=0x00000010)  # CREATE_NEW_CONSOLE
    time.sleep(3.0)
    subprocess.Popen([gameExe], cwd=os.path.dirname(gameExe))


def launchXxmi(config, importer):
    import subprocess
    exe = cfg.launcherExe(config)
    if not exe:
        raise GameViewError("XXMI Launcher.exe not found under " + config.get("xxmiRoot", "?"))
    subprocess.Popen([exe, "--nogui", "--xxmi", importer], cwd=os.path.dirname(exe))

