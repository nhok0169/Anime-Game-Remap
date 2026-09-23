"""ModInstaller: extract a folder of downloaded mod archives into a mods folder, one numbered folder
per archive -- ``<Name>1``, ``<Name>2``, ...

    py -3 main.py <archive folder> <mods folder> <character name>
    py -3 main.py "C:/Users/me/Downloads/Charlotte" "E:/.../GIMI/Mods" Charlotte --dryRun

Read README.md before using it. In short:

- ``.zip`` is read with Python's ``zipfile``; ``.rar`` with WinRAR's ``UnRAR.exe`` / ``Rar.exe``
  when installed; ``.7z`` / ``.tar*`` (and ``.rar`` without WinRAR) go through ``tar`` -- Windows 10+
  ships bsdtar, which reads all of them.
- Numbering never reuses an index: it continues after the highest ``<Name><i>`` already present in
  the mods folder, in its parent (where parked mods live), and under any ``--check`` folder, counting
  ``DISABLED<Name><i>`` too.
- Each installed folder gets a ``.modInstall.json`` recording the archive and its sha256, so running
  the tool again over the same downloads installs only the NEW archives.
- Wrapper folders are flattened: an archive holding ``ModName/`` (or ``a/b/ModName/``) and nothing
  else lands as ``<Name><i>/<contents of ModName>``.
"""

import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile

MarkerFile = ".modInstall.json"
ZipExts = (".zip",)
TarExts = (".7z", ".rar", ".tar", ".tar.gz", ".tgz", ".tar.xz", ".tar.bz2", ".tar.zst")
ArchiveExts = ZipExts + TarExts

# entries an archiver adds that are never part of the mod
JunkNames = {"__MACOSX", ".DS_Store", "Thumbs.db", "desktop.ini"}


class InstallError(Exception):
    pass


def longPath(path: str) -> str:
    """Windows MAX_PATH is 260; a mod folder under a deep mods folder crosses it easily."""
    path = os.path.abspath(path)
    if os.name == "nt" and not path.startswith("\\\\?\\"):
        return "\\\\?\\" + path
    return path


def archiveExt(fileName: str) -> str:
    lower = fileName.lower()
    for ext in sorted(ArchiveExts, key=len, reverse=True):
        if lower.endswith(ext):
            return ext
    return ""


def isLaterVolume(fileName: str) -> bool:
    """``x.part2.rar`` onwards: WinRAR extracts the whole set from ``x.part1.rar``."""
    match = re.search(r"\.part(\d+)\.rar$", fileName, re.IGNORECASE)
    return match is not None and int(match.group(1)) > 1


def sha256(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def slotPattern(name: str):
    return re.compile(r"^(?:DISABLED[\s_-]*)?" + re.escape(name) + r"(\d+)$", re.IGNORECASE)


def scanExisting(folders, name):
    """(highest index in use, {sha256: folder} of archives already installed)."""
    pattern = slotPattern(name)
    highest = 0
    installed = {}
    for folder in folders:
        if not os.path.isdir(folder):
            continue
        for entry in os.scandir(folder):
            if not entry.is_dir():
                continue
            match = pattern.match(entry.name)
            if match is None:
                continue
            highest = max(highest, int(match.group(1)))
            marker = os.path.join(entry.path, MarkerFile)
            if os.path.isfile(marker):
                try:
                    with open(marker, encoding="utf-8") as f:
                        installed[json.load(f)["sha256"]] = entry.path
                except (OSError, ValueError, KeyError):
                    pass
    return highest, installed


# ---------------------------------------------------------------------------------------------
# extraction

def zipMemberName(info: zipfile.ZipInfo, encoding: str) -> str:
    """A zip without the UTF-8 flag is decoded as cp437 by ``zipfile``, which mangles the
    Chinese / Japanese / Korean names many mods carry. Recover the raw bytes and try again."""
    if info.flag_bits & 0x800:
        return info.filename
    raw = info.filename.encode("cp437")
    # utf-8 first: it is strict, so a name that decodes as utf-8 almost certainly is utf-8
    for candidate in ["utf-8"] + ([encoding] if encoding else []):
        try:
            return raw.decode(candidate)
        except (UnicodeDecodeError, LookupError):
            continue
    return info.filename


def safeJoin(root: str, member: str) -> str:
    parts = [p for p in re.split(r"[\\/]+", member) if p not in ("", ".")]
    if any(p == ".." for p in parts) or (parts and re.match(r"^[A-Za-z]:", parts[0])):
        raise InstallError(f"refusing unsafe path in archive: {member!r}")
    return os.path.join(root, *parts)


def extractZip(archive: str, outDir: str, encoding: str):
    try:
        with zipfile.ZipFile(archive) as zf:
            for info in zf.infolist():
                target = safeJoin(outDir, zipMemberName(info, encoding))
                if info.is_dir():
                    os.makedirs(longPath(target), exist_ok=True)
                    continue
                os.makedirs(longPath(os.path.dirname(target)), exist_ok=True)
                with zf.open(info) as src, open(longPath(target), "wb") as dst:
                    shutil.copyfileobj(src, dst, 1 << 20)
    except RuntimeError as e:  # encrypted member
        raise InstallError(f"cannot extract (password protected?): {e}")
    except zipfile.BadZipFile as e:
        raise InstallError(f"not a readable zip: {e}")


def findTar():
    if os.name == "nt":
        system = os.path.join(os.environ.get("SystemRoot", r"C:\Windows"), "System32", "tar.exe")
        if os.path.isfile(system):
            return system  # bsdtar; a Git-for-Windows GNU tar on the PATH cannot read .7z / .rar
    return shutil.which("bsdtar") or shutil.which("tar")


def findWinRar():
    """WinRAR's own extractor: the reference for .rar (RAR5, solid, multi-volume, passworded)."""
    for folder in [shutil.which("UnRAR"), shutil.which("Rar")] + [
            os.path.join(os.environ.get(v, ""), "WinRAR", exe)
            for v in ("ProgramFiles", "ProgramFiles(x86)", "ProgramW6432") for exe in ("UnRAR.exe", "Rar.exe")]:
        if folder and os.path.isfile(folder):
            return folder
    return None


def extractRar(archive: str, outDir: str):
    winRar = findWinRar()
    if winRar is None:
        extractTar(archive, outDir)
        return
    # x: keep paths; -y: yes to all; -o+: overwrite; -p-: never prompt for a password; -idq: quiet
    result = subprocess.run([winRar, "x", "-y", "-o+", "-p-", "-idq", os.path.abspath(archive),
                             os.path.abspath(outDir) + os.sep],
                            capture_output=True, text=True, errors="replace")
    if result.returncode != 0:
        raise InstallError(f"WinRAR failed ({result.returncode}, password protected or damaged?): "
                           f"{(result.stderr or result.stdout).strip()}")


def extractTar(archive: str, outDir: str):
    tar = findTar()
    if tar is None:
        raise InstallError("no 'tar' found to extract this archive type")
    result = subprocess.run([tar, "-xf", os.path.abspath(archive), "-C", outDir],
                            capture_output=True, text=True, errors="replace")
    if result.returncode != 0:
        raise InstallError(f"tar failed ({result.returncode}): {result.stderr.strip()}")


def removeJunk(root: str):
    for dirPath, dirNames, fileNames in os.walk(longPath(root)):
        for d in [d for d in dirNames if d in JunkNames]:
            shutil.rmtree(os.path.join(dirPath, d), ignore_errors=True)
            dirNames.remove(d)
        for f in fileNames:
            if f in JunkNames:
                os.remove(os.path.join(dirPath, f))


def modRoot(root: str) -> str:
    """Descend through wrapper folders: a folder whose only entry is one folder."""
    while True:
        entries = os.listdir(longPath(root))
        if len(entries) == 1 and os.path.isdir(longPath(os.path.join(root, entries[0]))):
            root = os.path.join(root, entries[0])
        else:
            return root


def describe(root: str):
    """(.ini files, archives nested inside) under an extracted mod."""
    inis, nested = [], []
    for dirPath, _, fileNames in os.walk(longPath(root)):
        for f in fileNames:
            lower = f.lower()
            rel = os.path.relpath(os.path.join(dirPath, f), longPath(root))
            if lower.endswith(".ini"):
                inis.append(rel)
            elif archiveExt(lower):
                nested.append(rel)
    return inis, nested


# ---------------------------------------------------------------------------------------------

def install(args) -> int:
    source, dest, name = args.archives, args.dest, args.name
    if not os.path.isdir(source):
        print(f"archive folder not found: {source}")
        return 2
    if not re.match(r"^[A-Za-z0-9_]+$", name):
        print(f"character name must be letters, digits or '_' (got {name!r})")
        return 2

    archives = sorted(f for f in os.listdir(source)
                      if os.path.isfile(os.path.join(source, f)) and archiveExt(f) and not isLaterVolume(f))
    others = sorted(f for f in os.listdir(source)
                    if os.path.isfile(os.path.join(source, f)) and not archiveExt(f))
    if not archives:
        print(f"no archives ({', '.join(ArchiveExts)}) in {source}")
        return 1

    checkFolders = [dest] + list(args.check or [])
    if not args.noParentCheck:
        checkFolders.append(os.path.dirname(os.path.abspath(dest)))
    highest, installed = scanExisting(checkFolders, name)
    nextIndex = highest + 1

    if not args.dryRun:
        os.makedirs(dest, exist_ok=True)

    rows, failures = [], 0
    for fileName in archives:
        path = os.path.join(source, fileName)
        digest = sha256(path)
        if digest in installed and not args.reinstall:
            rows.append((fileName, "skipped", f"already installed as {os.path.basename(installed[digest])}"))
            continue

        folderName = f"{name}{nextIndex}"
        if args.dryRun:
            rows.append((fileName, folderName, "(dry run)"))
            nextIndex += 1
            continue

        # extract beside the destination so the final move is a same-drive rename
        tmp = tempfile.mkdtemp(prefix=".modInstall-", dir=dest)
        try:
            ext = archiveExt(fileName)
            if ext in ZipExts:
                extractZip(path, tmp, args.zipEncoding)
            elif ext == ".rar":
                extractRar(path, tmp)
            else:
                extractTar(path, tmp)
            removeJunk(tmp)
            if not os.listdir(tmp):
                raise InstallError("archive is empty")
            root = modRoot(tmp)
            target = os.path.join(dest, folderName)
            if os.path.exists(target):
                raise InstallError(f"{target} exists")
            os.rename(longPath(root), longPath(target))
            with open(os.path.join(target, MarkerFile), "w", encoding="utf-8") as f:
                json.dump({"archive": fileName, "sha256": digest,
                           "installed": datetime.datetime.now().isoformat(timespec="seconds")},
                          f, ensure_ascii=False, indent=2)
        except (InstallError, OSError) as e:
            failures += 1
            rows.append((fileName, "FAILED", str(e)))
            continue
        finally:
            shutil.rmtree(longPath(tmp), ignore_errors=True)

        inis, nested = describe(target)
        notes = [f"{len(inis)} .ini"]
        if not inis:
            notes.append("WARNING: no .ini -- not a mod, or the mod is in a nested archive")
        if nested:
            notes.append("nested archives: " + ", ".join(nested))
        rows.append((fileName, folderName, "; ".join(notes)))
        installed[digest] = target
        nextIndex += 1

    width = max(len(r[0]) for r in rows)
    for archive, folder, note in rows:
        print(f"{archive:<{width}}  ->  {folder:<{len(name) + 4}}  {note}")
    if others:
        print(f"\nignored (not an archive): {', '.join(others)}")
    done = sum(1 for r in rows if r[1] not in ("skipped", "FAILED"))
    print(f"\n{'would install' if args.dryRun else 'installed'} {done}, "
          f"skipped {sum(1 for r in rows if r[1] == 'skipped')}, failed {failures}  ->  {os.path.abspath(dest)}")
    return 1 if failures else 0


def main():
    parser = argparse.ArgumentParser(description="Extract downloaded mod archives into <Name>1, <Name>2, ... folders.")
    parser.add_argument("archives", help="folder holding the downloaded archives (not searched recursively)")
    parser.add_argument("dest", help="the mods folder to install into, e.g. .../GIMI/Mods")
    parser.add_argument("name", help="the character name the folders are numbered after, e.g. Charlotte")
    parser.add_argument("--dryRun", action="store_true", help="print the plan, extract nothing")
    parser.add_argument("--reinstall", action="store_true",
                        help="install archives again even if a folder already records their sha256")
    parser.add_argument("--check", action="append", metavar="DIR",
                        help="another folder whose <Name><i> folders count as taken (repeatable)")
    parser.add_argument("--noParentCheck", action="store_true",
                        help="do not count <Name><i> folders in the mods folder's parent (where parked mods live)")
    parser.add_argument("--zipEncoding", default="gbk",
                        help="encoding to try for zip names without the UTF-8 flag, after utf-8 (default gbk; "
                             "e.g. shift_jis, cp949)")
    sys.exit(install(parser.parse_args()))


if __name__ == "__main__":
    main()
