"""GameView: let an agent look at and drive the game itself -- screenshots, keyboard and mouse,
3DMigoto reloads, frame dumps, and which mods are loaded -- so an in-game check of a remap no
longer needs the maintainer at the keyboard.

    py -3 main.py status
    py -3 main.py screenshot --name CitlaliFront
    py -3 main.py do "key c; wait 2; drag 700 400 900 400; screenshot --name Turned"

Read README.md (and AI Agent Help/GameView/CLAUDE.md) before using it.
"""

import argparse
import os
import shlex
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from GameView import capture, keys, migoto, win32  # noqa: E402
from GameView import config as cfg  # noqa: E402
from GameView import helper  # noqa: E402
from GameView.game import (UID_STRIP, GameViewError, Session, findTarget, gameProcesses,  # noqa: E402
                           hideUid, isBlack, requireTarget)

# Commands that inject input into, focus, or capture the game window -- the ones that must run
# ELEVATED when the game is.
INTERACTIVE = {"screenshot", "compare", "key", "hold", "type", "click", "move", "drag", "look",
               "scroll", "reload", "dump", "close", "do", "toggle"}


class Context:
    """What one invocation shares across the steps of a `do`: the config, the target, the focus."""

    def __init__(self, args):
        self.config = cfg.load()
        self.importerName = getattr(args, "importer", None)
        self.window = getattr(args, "window", None)
        self.restore = not getattr(args, "stay", False) and self.config.get("restoreFocus", True)
        self._session = None

    @property
    def session(self):
        if self._session is None:
            target = requireTarget(self.config, self.importerName, self.window)
            self._session = Session(target, restore=self.restore,
                                    cropBottom=self.config.get("uidStrip", UID_STRIP))
        return self._session

    @property
    def importer(self):
        if self.importerName:
            return self.importerName
        if self._session is not None and self._session.target.importer:
            return self._session.target.importer
        running = gameProcesses(self.config)
        if running:
            return running[0][0]
        return self.config.get("defaultImporter", "GIMI")

    @property
    def folder(self):
        return migoto.importerFolder(self.config, self.importer)

    def close(self):
        if self._session is not None:
            self._session.__exit__(None, None, None)


def _vks(chord):
    return keys.parseChord(chord)


def _hotkey(ctx, name, fallback):
    vk, mods = cfg.hotkey(ctx.folder, name, fallback)
    return mods + [vk]


# ---------------------------------------------------------------- capture commands


def cmdScreenshot(args, ctx):
    if args.screen:
        from PIL import Image
        x, y, w, h = win32.virtualScreen()
        image = Image.frombuffer("RGB", (w, h), win32.grabScreen(x, y, w, h), "raw", "BGRX", 0, 1)
        image = hideUid(image, ctx.config.get("uidStrip", UID_STRIP))
        method = "blt"
        clientSize = None
    else:
        session = ctx.session
        if args.method != "print" or args.original:
            session.ensureFocus()
        if args.settle:
            time.sleep(args.settle)
        if args.original:
            vks = _hotkey(ctx, "show_original", "f9")
            for vk in vks:
                session.keyDown(vk)
            try:
                time.sleep(0.35)
                image, method = session.grab(args.method)
            finally:
                for vk in reversed(vks):
                    session.keyUp(vk)
        else:
            image, method = session.grab(args.method)
        clientSize = tuple(session.target.client()[2:])  # the window's, not the UID-cropped image's

    if args.crop:
        image = image.crop(_box(args.crop, args.space, image.size, ctx, clientSize))
    full, view, scale = capture.saveShot(image, args.name, args.max or ctx.config.get("viewMax"))
    print("full: {} ({}x{}, via {})".format(full, image.width, image.height, method))
    print("view: {} (1 view px = {:.3f} full px)".format(view, scale))
    if clientSize and not args.crop:
        state = cfg.loadState()
        state["lastShot"] = {"full": str(full), "view": str(view), "scale": scale,
                             "clientSize": list(clientSize), "time": time.time()}
        cfg.saveState(state)
    else:
        print("(not recorded as the last shot: 'view' click coordinates still refer to the "
              "previous full-window screenshot)")
    if isBlack(image):
        print("WARNING: the capture is black. Exclusive fullscreen, HDR, or the game still "
              "loading -- retry with --method print, or switch the game to borderless.")
    if args.keep:
        print("kept: {}".format(capture.keep(image, args.keep, ctx.config.get("keepMax", 1920))))


def _box(values, space, size, ctx, clientSize):
    x, y, w, h = values
    if space == "frac":
        return (int(x * size[0]), int(y * size[1]), int((x + w) * size[0]),
                int((y + h) * size[1]))
    if space == "view":
        shot = cfg.loadState().get("lastShot") or {}
        scale = shot.get("scale", 1.0)
        x, y, w, h = x * scale, y * scale, w * scale, h * scale
    return int(x), int(y), int(x + w), int(y + h)


def cmdCrop(args, ctx):
    shot = cfg.loadState().get("lastShot")
    source = args.source or (shot or {}).get("full")
    if not source or not os.path.isfile(source):
        raise GameViewError("no screenshot to crop from; take one or pass --from")
    image = capture.openImage(source)
    box = _box(args.box, args.space, image.size, ctx, None)
    region = image.crop(box)
    full, view, scale = capture.saveShot(region, args.name or "crop",
                                         args.max or ctx.config.get("viewMax"))
    print("crop of {} box {}".format(source, box))
    print("full: {} ({}x{})".format(full, region.width, region.height))
    print("view: {} (1 view px = {:.3f} full px)".format(view, scale))
    if args.keep:
        print("kept: {}".format(capture.keep(region, args.keep, ctx.config.get("keepMax", 1920))))


def cmdCompare(args, ctx):
    """The same frame twice: with mods, and with 3DMigoto's show_original held (every mod off).
    Same pose, same camera, same lighting -- the cheapest honest A/B the game offers."""
    session = ctx.session
    session.ensureFocus()
    if args.settle:
        time.sleep(args.settle)
    modded, _ = session.grab(args.method)
    vks = _hotkey(ctx, "show_original", "f9")
    for vk in vks:
        session.keyDown(vk)
    try:
        time.sleep(0.35)
        original, _ = session.grab(args.method)
    finally:
        for vk in reversed(vks):
            session.keyUp(vk)
    if args.crop:
        box = _box(args.crop, args.space, modded.size, ctx, modded.size)
        modded, original = modded.crop(box), original.crop(box)
    combined = capture.sideBySide(original, modded, ("original (F9)", "modded"))
    full, view, scale = capture.saveShot(combined, args.name or "compare",
                                         args.max or ctx.config.get("viewMax"))
    print("full: {} ({}x{})".format(full, combined.width, combined.height))
    print("view: {}".format(view))
    if _similar(modded, original):
        print("NOTE: the two halves are (nearly) identical: either no mod draws in view, or "
              "show_original did nothing (it only works while hunting is ON -- see `status`).")
    if args.keep:
        print("kept: {}".format(capture.keep(combined, args.keep, ctx.config.get("keepMax", 1920))))


def cmdPair(args, ctx):
    """Two EXISTING images in one labelled picture -- e.g. the source mod on its own character
    beside the remap on the target, each taken in its own run with the same camera recipe."""
    first, second = capture.openImage(args.first), capture.openImage(args.second)
    if args.crop:
        first = first.crop(_box(args.crop, "frac", first.size, ctx, None))
        second = second.crop(_box(args.crop, "frac", second.size, ctx, None))
    labels = args.labels or (os.path.basename(args.first), os.path.basename(args.second))
    combined = capture.sideBySide(first, second, labels)
    full, view, _ = capture.saveShot(combined, args.name or "pair",
                                     args.max or ctx.config.get("viewMax"))
    print("full: {} ({}x{})".format(full, combined.width, combined.height))
    print("view: {}".format(view))
    if args.keep:
        print("kept: {}".format(capture.keep(combined, args.keep, ctx.config.get("keepMax", 1920))))


def _similar(a, b):
    from PIL import ImageChops
    diff = ImageChops.difference(a.convert("L"), b.convert("L")).point(lambda v: 255 if v > 24 else 0)
    histogram = diff.histogram()
    return histogram[255] < 0.001 * a.width * a.height


# ---------------------------------------------------------------- input commands


def cmdKey(args, ctx):
    for i, chord in enumerate(args.chords):
        if i:
            time.sleep(args.gap / 1000.0)
        ctx.session.chord(_vks(chord), hold=args.hold / 1000.0)
    print("pressed {}".format(" ".join(args.chords)))


def cmdHold(args, ctx):
    ctx.session.holdKeys(_vks(args.chord), args.seconds)
    print("held {} for {}s".format(args.chord, args.seconds))


def cmdType(args, ctx):
    ctx.session.typeText(args.text)
    print("typed {} characters".format(len(args.text)))


def cmdClick(args, ctx):
    sx, sy = ctx.session.click(args.x, args.y, args.space, args.button, 2 if args.double else 1,
                               args.hold / 1000.0)
    print("{} click at screen ({}, {})".format(args.button, sx, sy))


def cmdMove(args, ctx):
    sx, sy = ctx.session.moveTo(args.x, args.y, args.space)
    print("cursor at screen ({}, {})".format(sx, sy))


def cmdDrag(args, ctx):
    a, b = ctx.session.drag(args.x1, args.y1, args.x2, args.y2, args.space, args.button,
                            args.duration)
    print("dragged {} -> {}".format(a, b))


def cmdLook(args, ctx):
    ctx.session.look(args.dx, args.dy, args.duration)
    print("camera moved by ({}, {})".format(args.dx, args.dy))


def cmdScroll(args, ctx):
    at = args.at or (None, None)
    ctx.session.scroll(args.notches, at[0], at[1], args.space)
    print("scrolled {}".format(args.notches))


def cmdWait(args, ctx):
    time.sleep(args.seconds)


# ---------------------------------------------------------------- 3DMigoto commands


def _reload(ctx, maximum=30.0):
    """Press reload_config and wait until 3DMigoto has finished reloading: its log says
    "Reloading d3dx.ini" and, once every section is parsed, "> d3dx.ini reloaded". Presses once
    more if the first press was not seen.
    Returns (the log text the reload appended, whether a reload was seen at all)."""
    folder = ctx.folder
    keysToPress = _hotkey(ctx, "reload_config", "f10")
    seen = False
    for _ in range(2):
        offset = migoto.logSize(folder)
        ctx.session.chord(keysToPress, hold=0.15)
        migoto.waitLog(folder, offset, until=lambda text: migoto.RELOAD_MARK in text,
                       minimum=0.3, maximum=6.0)
        # Until the reload's END marker, not until the log goes quiet -- see migoto.RELOAD_DONE.
        text = migoto.waitLog(folder, offset, until=migoto.reloadFinished, minimum=1.0, maximum=maximum)
        if migoto.RELOAD_MARK in text:
            seen = True
            break
    if seen and not migoto.reloadFinished(text):
        print("WARNING: the reload had not finished after {:.0f}s -- warnings of mods parsed after "
              "that are MISSING below; pass a larger --wait".format(maximum))
    return text, seen


def cmdReload(args, ctx):
    folder = ctx.folder
    text, seen = _reload(ctx, args.wait)
    print("reloaded {} ({})".format(ctx.importer, folder))
    if not seen:
        print("WARNING: the log never said 'Reloading d3dx.ini' -- the key may not have reached "
              "the game (focus?), or this importer's log is off")
    found = migoto.problems(text, args.limit, args.mod)
    _printProblems(found, "since the reload", args.mod)


def _printProblems(found, when, mod):
    scope = " under Mods\\{}".format(mod) if mod else ""
    if not found:
        print("no warnings in d3d11_log.txt {}{}".format(when, scope))
        return
    print("{} distinct warning(s){} {}:".format(len(found), scope, when))
    lastSection = None
    for line, section in found:
        if section != lastSection:
            print("  [{}]".format(section or "before any section"))
            lastSection = section
        print("    " + line.strip())


def cmdToggleHunting(args, ctx):
    ctx.session.chord(_hotkey(ctx, "toggle_hunting", "num0"), hold=0.15)
    print("toggled hunting (mode in d3dx.ini: {})".format(cfg.huntingMode(ctx.folder)))


def cmdDump(args, ctx):
    folder = ctx.folder
    session = ctx.session
    session.ensureFocus()
    original = None
    if args.options:
        original = migoto.setAnalyseOptions(folder, args.options)
        _reload(ctx)
    toggled = False
    dumpKey = _hotkey(ctx, "analyse_frame", "f8")

    def press():
        session.chord(dumpKey, hold=0.2)

    progress = (lambda s: print("  ... {} files, {:.0f} MB".format(s[0], s[1] / 1e6),
                                flush=True)) if args.verbose else None
    try:
        before = migoto.dumpFolders(folder)
        offset = migoto.logSize(folder)
        press()
        name = migoto.waitForDump(folder, before, offset, press, session.target.hwnd, args.appear,
                                  args.settle, args.timeout, progress)
        if name is None and cfg.huntingMode(folder) != 1:
            # hunting = 2 is soft-disabled: analyse_frame is dead until toggle_hunting.
            print("no dump started; hunting is {} in d3dx.ini, toggling it on".format(
                cfg.huntingMode(folder)))
            session.chord(_hotkey(ctx, "toggle_hunting", "num0"), hold=0.15)
            toggled = True
            time.sleep(1.0)
            offset = migoto.logSize(folder)
            press()
            name = migoto.waitForDump(folder, before, offset, press, session.target.hwnd,
                                      args.appear, args.settle, args.timeout, progress)
        if name is None:
            raise GameViewError("no frame dump started in {}s (F8 pressed 3 times). Is hunting "
                                "on (`status`), and is the game itself focused, not a "
                                "launcher or an OS dialog?".format(args.appear))
    finally:
        if toggled:
            session.chord(_hotkey(ctx, "toggle_hunting", "num0"), hold=0.15)
        if original is not None:
            migoto.restoreFile(folder, original)
            _reload(ctx)

    name = migoto.labelDump(folder, name, args.label)
    path = os.path.join(folder, name)
    count, size = migoto.folderStats(path)
    print("dump: {}".format(path))
    print("  {} files, {:.1f} MB, {} draw calls".format(count, size / 1e6, migoto.drawCalls(path)))
    if count < 50:
        print("WARNING: very few files -- 3DMigoto may still have been writing, or the frame was a "
              "loading screen")


def cmdLog(args, ctx):
    folder = ctx.folder
    print("{} ({:.1f} MB)".format(migoto.logPath(folder), migoto.logSize(folder) / 1e6))
    if args.problems or args.mod:
        text = migoto.readLogFrom(folder, 0, cap=int(args.scan_mb * 1024 * 1024))
        _printProblems(migoto.problems(text, args.tail, args.mod),
                       "in the last {} MB".format(args.scan_mb), args.mod)
        return
    for line in migoto.tailLog(folder, args.tail):
        print(line)


# ---------------------------------------------------------------- game process


def cmdLaunch(args, ctx):
    importer = args.importer_name
    info = ctx.config.get("importers", {}).get(importer)
    if not info:
        raise GameViewError("unknown importer {}; configured: {}".format(
            importer, ", ".join(ctx.config.get("importers", {}))))
    existing = findTarget(ctx.config, importer)
    if existing:
        print("already running: " + existing.describe())
        return
    method = args.method or info.get("launch", "xxmi")
    if method == "loader":
        migoto.launchLoader(info["folder"], info["gameExe"])
    else:
        migoto.launchXxmi(ctx.config, importer)
    print("started {} via {}; waiting up to {}s for its window".format(importer, method, args.wait))
    deadline = time.time() + args.wait
    while time.time() < deadline:
        target = findTarget(ctx.config, importer)
        if target:
            _, _, w, h = target.client()
            if w >= 640 and h >= 360:
                print("up: " + target.describe())
                print("It is probably still on a loading / login screen: screenshot it and click "
                      "through (Genshin: click anywhere on the door screen).")
                return
        time.sleep(2.0)
    raise GameViewError("no {} window after {}s".format(importer, args.wait))


def cmdClose(args, ctx):
    target = ctx.session.target
    if args.force:
        ok = win32.terminate(target.pid)
        print("terminated {}: {}".format(target.exe, ok))
    else:
        win32.postClose(target.hwnd)
        print("asked {} to close (the game may show its own exit dialog)".format(target.exe))
    ctx.restore = False


# ---------------------------------------------------------------- local-only commands


def cmdStatus(args, ctx):
    config = ctx.config
    print("config:  {}".format(cfg.CONFIG_FILE))
    print("scratch: {}".format(cfg.SCRATCH))
    print("elevated: {}".format(win32.isSelfElevated()))
    reply = helper.ping()
    print("helper:  {}".format("running (pid {}, elevated={}, since {})".format(
        reply["pid"], reply["elevated"], reply.get("started")) if reply else "not running"))
    for name, info in config.get("importers", {}).items():
        folder = info["folder"]
        mode = cfg.huntingMode(folder)
        loaded, parked = migoto.listMods(folder)
        dumps = sorted(migoto.dumpFolders(folder))
        print("\n[{}] {}".format(name, folder))
        print("  launch: {}   game exe: {}".format(info.get("launch"), info.get("gameExe")))
        print("  hunting = {} ({})".format(mode, {0: "OFF: no dumps, no show_original", 1: "on",
                                                  2: "soft-off: `toggle` turns it on"}.get(mode, "?")))
        for action, default in (("reload_config", "f10"), ("analyse_frame", "f8"),
                                ("show_original", "f9"), ("toggle_hunting", "num0")):
            vk, mods = cfg.hotkey(folder, action, default)
            print("  {:15s} vk 0x{:02X}{}".format(action, vk, " + mods" if mods else ""))
        print("  mods loaded: {}   parked: {}".format(len(loaded), len(parked)))
        print("  log: {:.1f} MB".format(migoto.logSize(folder) / 1e6))
        if dumps:
            print("  newest dump: {}".format(dumps[-1]))
        target = findTarget(config, name)
        if target:
            elevated = win32.isProcessElevated(target.pid)
            print("  RUNNING: {}".format(target.describe()))
            print("  game elevated: {}".format("yes" if elevated else
                                               "no" if elevated is False else "cannot query (treated as yes)"))
        else:
            print("  not running")


def cmdSetup(args, ctx):
    if args.reset or args.xxmi_root or not os.path.isfile(cfg.CONFIG_FILE):
        config = cfg.discover(args.xxmi_root)
        if not config["xxmiRoot"]:
            raise GameViewError("could not find 'XXMI Launcher Config.json'; pass --xxmi-root")
        cfg.save(config)
        print("wrote {}".format(cfg.CONFIG_FILE))
    import json
    print(json.dumps(cfg.load(), indent=2, ensure_ascii=False))


def cmdWindows(args, ctx):
    names = win32.processNames()
    for hwnd in win32.topLevelWindows():
        title = win32.windowText(hwnd)
        if not title:
            continue
        _, _, w, h = win32.windowRect(hwnd)
        pid = win32.windowPid(hwnd)
        print("{:>8} {:28.28s} {:5d}x{:<5d} {}".format(pid, names.get(pid, "?"), w, h, title))


def cmdMods(args, ctx):
    importer = args.importer_name
    folder = migoto.importerFolder(ctx.config, importer)
    source = os.path.abspath(args.source) if args.source else None
    if source and not os.path.isdir(source):
        raise GameViewError("--from folder not found: " + source)
    if args.action == "scan":
        rows = migoto.scanMods(source or folder)
        print("{} mod folder(s) in {}".format(len(rows), source or folder))
        print("  {:40s} {:>4s} {:>6s}  {:5s} {}".format("folder", "inis", "hashes", "fixed", "hint"))
        for row in rows:
            print("  {:40.40s} {:>4d} {:>6d}  {:5s} {}".format(
                row["mod"], row["inis"], row["hashes"], "yes" if row["fixed"] else "", row["hint"]))
        print("(hint = the commonest first word of its TextureOverride sections: a guess, not a "
              "classification)")
        return
    if args.action == "list":
        loaded, available = migoto.listMods(folder, source)
        print("loaded ({}): {}".format(len(loaded), ", ".join(loaded)))
        where = source or "parked in " + folder
        if args.all or source:
            print("available ({}, {}): {}".format(len(available), where, ", ".join(available)))
        else:
            print("available: {} folders parked in {} (--all lists them)".format(len(available),
                                                                                folder))
        return
    if args.action == "restore":
        undone, failed = migoto.restoreMods(importer)
        print("moved back: {}".format(", ".join(undone) or "nothing"))
        for line in failed:
            print("  could not undo " + line)
    elif not args.names and args.action != "only":
        raise GameViewError("`mods {}` needs folder names (globs allowed)".format(args.action))
    elif args.action == "load":
        print("loaded: {}".format(", ".join(migoto.loadMods(importer, folder, args.names, source))))
    elif args.action == "park":
        for line in migoto.parkMods(importer, folder, args.names):
            print("parked: " + line)
    elif args.action == "only":
        protected = ctx.config.get("protectedMods", cfg.DEFAULT_PROTECTED) + (args.keep or [])
        parked, loaded, kept = migoto.onlyMods(importer, folder, args.names, protected, source)
        print("parked: {}".format(", ".join(parked) or "nothing"))
        print("loaded: {}".format(", ".join(loaded) or "nothing"))
        print("kept (libraries): {}".format(", ".join(kept) or "none"))
    print("Now `reload` (F10) for the game to see it. `mods {} restore` undoes every move.".format(
        importer))


def cmdClean(args, ctx):
    print("removed {} scratch screenshot(s) older than {} day(s) from {}".format(
        migoto.cleanShots(args.days), args.days, cfg.SHOTS))


def cmdHelper(args, ctx):
    if args.action == "serve":
        helper.serve(args.port)
    elif args.action == "start":
        reply = helper.start()
        if reply:
            print("helper running: pid {}, elevated={}".format(reply["pid"], reply["elevated"]))
        else:
            print("the helper did not start (was the UAC prompt declined?)")
            return 1
    elif args.action == "stop":
        print("stopped" if helper.stop() else "was not running")
    else:
        reply = helper.ping()
        print("running: {}".format(reply) if reply else "not running")


def cmdDo(args, ctx):
    """Several steps in ONE invocation: one focus, one round trip through the helper, and the
    game is not handed back to the desktop between a key press and the screenshot after it."""
    text = args.steps
    if args.file:
        with open(args.file, encoding="utf-8") as f:
            text = f.read()
    steps = [s.strip() for chunk in (text or "").splitlines() for s in chunk.split(";")]
    steps = [s for s in steps if s and not s.startswith("#")]
    parser = buildParser()
    for i, step in enumerate(steps, 1):
        argv = shlex.split(step)
        if argv and argv[0] in ("do", "helper", "launch"):
            raise GameViewError("step {} ({}) is not allowed inside `do`".format(i, argv[0]))
        stepArgs = parser.parse_args(argv)
        print("[{}/{}] {}".format(i, len(steps), step), flush=True)
        result = stepArgs.func(stepArgs, ctx)
        if result:
            return result


# ---------------------------------------------------------------- parser


def buildParser():
    parser = argparse.ArgumentParser(prog="main.py", description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)

    target = argparse.ArgumentParser(add_help=False)
    target.add_argument("-i", "--importer", help="GIMI / WWMI ... (default: whichever game is running)")
    target.add_argument("-w", "--window", help="drive any window whose title or exe matches this "
                        "regex instead of a game (for testing the tool itself)")
    target.add_argument("--stay", action="store_true",
                        help="leave the game in the foreground afterwards")
    space = argparse.ArgumentParser(add_help=False)
    space.add_argument("--space", choices=["view", "client", "frac", "screen"], default="view",
                       help="coordinate space (default view: pixels of the last .view.png)")
    grab = argparse.ArgumentParser(add_help=False)
    grab.add_argument("--name", help="a word or two saying what the shot is, in its file name")
    grab.add_argument("--max", type=int, help="long side of the view image (default 1568)")
    grab.add_argument("--keep", metavar="Char/Version/Name",
                      help="also save into AI Agent Help/CreatingRemaps/Images/ for future agents")
    grab.add_argument("--method", choices=["auto", "blt", "print"], default="auto")
    grab.add_argument("--settle", type=float, default=0.3,
                      help="seconds to wait after focusing before capturing")
    grab.add_argument("--crop", type=float, nargs=4, metavar=("X", "Y", "W", "H"))

    p = sub.add_parser("screenshot", parents=[target, space, grab], help="capture the game")
    p.add_argument("--screen", action="store_true", help="the whole desktop instead (launcher "
                   "dialogs, login windows)")
    p.add_argument("--original", action="store_true",
                   help="hold show_original (F9) while capturing: the game with every mod off")
    p.set_defaults(func=cmdScreenshot)

    p = sub.add_parser("compare", parents=[target, space, grab],
                       help="side by side: show_original (mods off) vs modded, same frame")
    p.set_defaults(func=cmdCompare)

    p = sub.add_parser("crop", parents=[space], help="zoom into the last screenshot at full resolution")
    p.add_argument("box", type=float, nargs=4, metavar=("X", "Y", "W", "H"))
    p.add_argument("--from", dest="source", help="crop this image instead of the last shot")
    p.add_argument("--name")
    p.add_argument("--max", type=int)
    p.add_argument("--keep", metavar="Char/Version/Name")
    p.set_defaults(func=cmdCrop)

    p = sub.add_parser("pair", help="two existing images in one labelled picture")
    p.add_argument("first")
    p.add_argument("second")
    p.add_argument("--labels", nargs=2)
    p.add_argument("--crop", type=float, nargs=4, metavar=("X", "Y", "W", "H"),
                   help="the same region of both, as fractions 0..1")
    p.add_argument("--name")
    p.add_argument("--max", type=int)
    p.add_argument("--keep", metavar="Char/Version/Name")
    p.set_defaults(func=cmdPair)

    p = sub.add_parser("key", parents=[target], help="press keys / chords: key c | key esc esc | key ctrl+f10")
    p.add_argument("chords", nargs="+")
    p.add_argument("--hold", type=float, default=80, help="ms each chord is held (default 80)")
    p.add_argument("--gap", type=float, default=150, help="ms between chords (default 150)")
    p.set_defaults(func=cmdKey)

    p = sub.add_parser("hold", parents=[target], help="hold a key: hold w 1.5 (walk forward)")
    p.add_argument("chord")
    p.add_argument("seconds", type=float)
    p.set_defaults(func=cmdHold)

    p = sub.add_parser("type", parents=[target], help="type text into a focused text box")
    p.add_argument("text")
    p.set_defaults(func=cmdType)

    p = sub.add_parser("click", parents=[target, space], help="click at X Y")
    p.add_argument("x", type=float)
    p.add_argument("y", type=float)
    p.add_argument("--button", choices=["left", "right", "middle"], default="left")
    p.add_argument("--double", action="store_true")
    p.add_argument("--hold", type=float, default=50, help="ms the button is held")
    p.set_defaults(func=cmdClick)

    p = sub.add_parser("move", parents=[target, space], help="move the cursor to X Y (hover)")
    p.add_argument("x", type=float)
    p.add_argument("y", type=float)
    p.set_defaults(func=cmdMove)

    p = sub.add_parser("drag", parents=[target, space],
                       help="press at X1 Y1, glide to X2 Y2, release (turn a model)")
    for name in ("x1", "y1", "x2", "y2"):
        p.add_argument(name, type=float)
    p.add_argument("--button", choices=["left", "right", "middle"], default="left")
    p.add_argument("--duration", type=float, default=0.6)
    p.set_defaults(func=cmdDrag)

    p = sub.add_parser("look", parents=[target],
                       help="relative mouse motion: turns the camera when the cursor is captured")
    p.add_argument("dx", type=float)
    p.add_argument("dy", type=float)
    p.add_argument("--duration", type=float, default=0.4)
    p.set_defaults(func=cmdLook)

    p = sub.add_parser("scroll", parents=[target, space],
                       help="mouse wheel notches: positive = up / zoom in")
    p.add_argument("notches", type=int)
    p.add_argument("--at", type=float, nargs=2, metavar=("X", "Y"))
    p.set_defaults(func=cmdScroll)

    p = sub.add_parser("wait", help="sleep (mainly for `do`)")
    p.add_argument("seconds", type=float)
    p.set_defaults(func=cmdWait)

    p = sub.add_parser("reload", parents=[target],
                       help="3DMigoto reload (F10), then the log's warnings since it")
    p.add_argument("--wait", type=float, default=30.0,
                   help="most seconds to wait for the reload to finish (default 30)")
    p.add_argument("--mod", help="only warnings from sections under Mods\\<this folder glob>")
    p.add_argument("--limit", type=int, default=80)
    p.set_defaults(func=cmdReload)

    p = sub.add_parser("toggle", parents=[target], help="toggle hunting mode (numpad 0)")
    p.set_defaults(func=cmdToggleHunting)

    p = sub.add_parser("dump", parents=[target], help="frame analysis (F8), wait, label the folder")
    p.add_argument("--label", help="FrameAnalysis-<label>-<timestamp>, e.g. CitlaliGothRemap")
    p.add_argument("--options", help="analyse_options for this dump only (restored after)")
    p.add_argument("--appear", type=float, default=30.0, help="seconds for the folder to appear")
    p.add_argument("--settle", type=float, default=15.0,
                   help="fallback only: seconds of no growth = finished (the log's "
                   "'Frame analysis saved' line is what normally ends the wait)")
    p.add_argument("--timeout", type=float, default=900.0)
    p.add_argument("--verbose", action="store_true")
    p.set_defaults(func=cmdDump)

    p = sub.add_parser("log", parents=[target], help="tail the importer's d3d11_log.txt")
    p.add_argument("--tail", type=int, default=60)
    p.add_argument("--problems", action="store_true", help="only warnings, grouped by section")
    p.add_argument("--mod", help="only warnings from sections under Mods\\<this folder glob>")
    p.add_argument("--scan-mb", type=float, default=8.0, help="how much of the log's end to scan")
    p.set_defaults(func=cmdLog)

    p = sub.add_parser("launch", help="start a game with its importer and wait for the window")
    p.add_argument("importer_name", metavar="IMPORTER")
    p.add_argument("--method", choices=["loader", "xxmi"],
                   help="default from config: GIMI = loader, the rest = xxmi")
    p.add_argument("--wait", type=float, default=240.0)
    p.set_defaults(func=cmdLaunch)

    p = sub.add_parser("close", parents=[target], help="close the game (--force terminates it)")
    p.add_argument("--force", action="store_true")
    p.set_defaults(func=cmdClose)

    p = sub.add_parser("status", help="importers, hotkeys, hunting mode, what is running")
    p.set_defaults(func=cmdStatus)

    p = sub.add_parser("setup", help="show / rediscover the config")
    p.add_argument("--xxmi-root")
    p.add_argument("--reset", action="store_true")
    p.set_defaults(func=cmdSetup)

    p = sub.add_parser("windows", help="list top-level windows (for --window)")
    p.set_defaults(func=cmdWindows)

    p = sub.add_parser("mods", help="which mod folders are loaded: "
                       "scan / list / load / park / only / restore")
    p.add_argument("importer_name", metavar="IMPORTER")
    p.add_argument("action", choices=["scan", "list", "load", "park", "only", "restore"])
    p.add_argument("--from", dest="source",
                   help="a folder of mods to load from (e.g. every mod of one character); parking "
                   "returns each mod to where it was loaded from. Default: the importer folder")
    p.add_argument("names", nargs="*")
    p.add_argument("--all", action="store_true")
    p.add_argument("--keep", nargs="*", help="`only`: extra folder globs to leave loaded")
    p.set_defaults(func=cmdMods)

    p = sub.add_parser("clean", help="delete old scratch screenshots")
    p.add_argument("--days", type=float, default=3.0)
    p.set_defaults(func=cmdClean)

    p = sub.add_parser("helper", help="the elevated helper: start / stop / status / serve")
    p.add_argument("action", choices=["start", "stop", "status", "serve"])
    p.add_argument("--port", type=int, default=helper.DEFAULT_PORT)
    p.set_defaults(func=cmdHelper)

    p = sub.add_parser("do", parents=[target], help='steps in one go: do "key c; wait 2; screenshot"')
    p.add_argument("steps", nargs="?")
    p.add_argument("--file", help="one step per line")
    p.set_defaults(func=cmdDo)
    return parser


def _needsHelper(args, config):
    if helper.inHelper() or win32.isSelfElevated():
        return False
    if args.command == "launch":
        return True  # the loader, XXMI and the games all ask for administrator
    if args.command not in INTERACTIVE or getattr(args, "screen", False):
        return False
    target = findTarget(config, getattr(args, "importer", None), getattr(args, "window", None))
    if target is None:
        return False
    return win32.isProcessElevated(target.pid) is not False


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    win32.setDpiAware()
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except (AttributeError, ValueError):
        pass
    args = buildParser().parse_args(argv)
    try:
        config = cfg.load()
        if _needsHelper(args, config):
            timeout = getattr(args, "timeout", None) or 900.0
            if args.command == "launch":
                timeout = args.wait + 60
            return helper.forward(argv, timeout)
        ctx = Context(args)
        try:
            return args.func(args, ctx) or 0
        finally:
            ctx.close()
    except GameViewError as e:
        sys.stderr.write("error: {}\n".format(e))
        return 2


if __name__ == "__main__":
    sys.exit(main())
