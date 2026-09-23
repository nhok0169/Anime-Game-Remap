"""Finding the game, focusing it, and driving it: input and capture in the game's coordinates."""

import fnmatch
import re
import time
from datetime import datetime

from . import config as cfg
from . import keys
from . import win32


class GameViewError(Exception):
    pass


class Target:
    """A window to drive: the game's main window, or (``--window``) any window by title / exe."""

    def __init__(self, hwnd, pid, exe, importer=None):
        self.hwnd = hwnd
        self.pid = pid
        self.exe = exe
        self.importer = importer

    @property
    def title(self):
        return win32.windowText(self.hwnd)

    def client(self):
        return win32.clientRect(self.hwnd)

    def describe(self):
        x, y, w, h = self.client()
        return "{} pid {} ({}) '{}' client {}x{} at ({}, {})".format(
            self.importer or "window", self.pid, self.exe, self.title, w, h, x, y)


def _largest(windows):
    best, bestArea = None, 0
    for hwnd in windows:
        _, _, w, h = win32.windowRect(hwnd)
        if w * h > bestArea:
            best, bestArea = hwnd, w * h
    return best


def gameProcesses(config):
    """[(importer, pid, exe)] of every running game the config knows."""
    names = win32.processNames()
    found = []
    for importer, info in config.get("importers", {}).items():
        wanted = {p.lower() for p in info.get("processes", [])}
        for pid, exe in names.items():
            if exe.lower() in wanted:
                found.append((importer, pid, exe))
    return found


def findTarget(config, importer=None, window=None):
    """The window to drive, or None. ``window`` is a regex over title or exe name."""
    names = win32.processNames()
    windows = win32.topLevelWindows()
    if window:
        pattern = re.compile(window, re.IGNORECASE)
        matches = [h for h in windows
                   if pattern.search(win32.windowText(h))
                   or pattern.search(names.get(win32.windowPid(h), ""))]
        hwnd = _largest(matches)
        if not hwnd:
            return None
        pid = win32.windowPid(hwnd)
        return Target(hwnd, pid, names.get(pid, "?"))

    for imp, pid, exe in gameProcesses(config):
        if importer and imp != importer:
            continue
        hwnd = _largest([h for h in windows if win32.windowPid(h) == pid])
        if hwnd:
            return Target(hwnd, pid, exe, imp)
    return None


def requireTarget(config, importer=None, window=None):
    target = findTarget(config, importer, window)
    if target is None:
        what = "window /{}/".format(window) if window else (importer or "any configured game")
        raise GameViewError("no running game found for {} -- `main.py launch {}` starts it".format(
            what, importer or "<GIMI|WWMI>"))
    return target


class Session:
    """Focus the target for the duration of a command, then (by default) hand the foreground back
    to whatever had it, so the maintainer's own window is not left buried under the game."""

    def __init__(self, target, restore=True):
        self.target = target
        self.restore = restore
        self.previous = None
        self.focused = False

    def __enter__(self):
        return self

    def ensureFocus(self):
        if self.focused and win32.foreground() == self.target.hwnd:
            return
        if self.previous is None:
            self.previous = win32.foreground()
        if not win32.focus(self.target.hwnd):
            raise GameViewError(
                "could not bring {} to the foreground. If the game runs elevated, this command "
                "has to go through the helper (`main.py helper status`).".format(self.target.exe))
        self.focused = True

    def __exit__(self, *exc):
        if self.restore and self.previous and self.previous != self.target.hwnd:
            win32.focus(self.previous, timeout=1.0)
        return False

    # ---------------------------------------------------------- coordinates

    def toScreen(self, x, y, space="view"):
        """A point in one of four spaces -> screen pixels.

        view:   pixels of the last VIEW image taken of this window (what an agent looks at)
        client: pixels of the game's client area, full resolution
        frac:   fractions 0..1 of the client area
        screen: absolute desktop pixels"""
        cx, cy, cw, ch = self.target.client()
        if space == "screen":
            return int(round(x)), int(round(y))
        if space == "frac":
            return cx + int(round(x * cw)), cy + int(round(y * ch))
        if space == "view":
            shot = cfg.loadState().get("lastShot")
            if not shot:
                raise GameViewError("'view' coordinates are pixels of the last screenshot, and "
                                    "there is none yet: take one, or pass --space client/frac")
            if tuple(shot.get("clientSize", ())) != (cw, ch):
                raise GameViewError("the window is {}x{} but the last screenshot was of {}x{}: "
                                    "take a new one before clicking by its pixels".format(
                                        cw, ch, *shot.get("clientSize", (0, 0))))
            x, y = x * shot["scale"], y * shot["scale"]
        return cx + int(round(x)), cy + int(round(y))

    # ---------------------------------------------------------- input

    def keyDown(self, vk):
        (win32.sendKeyVk if keys.byVk(vk) else win32.sendKeyScan)(vk, True)

    def keyUp(self, vk):
        (win32.sendKeyVk if keys.byVk(vk) else win32.sendKeyScan)(vk, False)

    def chord(self, vks, hold=0.08):
        """Press every key of a chord in order, hold, release in reverse. A hotkey 3DMigoto polls
        once per frame needs the hold, or a fast frame-less tap is missed."""
        self.ensureFocus()
        pressed = []
        try:
            for vk in vks:
                self.keyDown(vk)
                pressed.append(vk)
                time.sleep(0.02)
            time.sleep(hold)
        finally:
            for vk in reversed(pressed):
                self.keyUp(vk)
                time.sleep(0.02)

    def holdKeys(self, vks, seconds):
        self.chord(vks, hold=seconds)

    def typeText(self, text):
        self.ensureFocus()
        win32.sendUnicode(text)

    def moveTo(self, x, y, space="view"):
        self.ensureFocus()
        sx, sy = self.toScreen(x, y, space)
        win32.mouseMoveAbs(sx, sy)
        return sx, sy

    def click(self, x, y, space="view", button="left", count=1, hold=0.05):
        sx, sy = self.moveTo(x, y, space)
        time.sleep(0.05)
        for _ in range(count):
            win32.mouseButton(button, True)
            time.sleep(hold)
            win32.mouseButton(button, False)
            time.sleep(0.08)
        return sx, sy

    def drag(self, x1, y1, x2, y2, space="view", button="left", duration=0.6, steps=30):
        """Press, glide, release -- e.g. to turn a character on the character screen."""
        sx1, sy1 = self.moveTo(x1, y1, space)
        sx2, sy2 = self.toScreen(x2, y2, space)
        time.sleep(0.05)
        win32.mouseButton(button, True)
        try:
            for i in range(1, steps + 1):
                t = i / steps
                win32.mouseMoveAbs(sx1 + (sx2 - sx1) * t, sy1 + (sy2 - sy1) * t)
                time.sleep(duration / steps)
        finally:
            win32.mouseButton(button, False)
        return (sx1, sy1), (sx2, sy2)

    def look(self, dx, dy, duration=0.4):
        """Relative mouse motion, which is what turns a game camera when the cursor is captured.
        Split into small steps: one huge delta is clamped or read as a flick by most games."""
        self.ensureFocus()
        steps = max(1, int(max(abs(dx), abs(dy)) / 15))
        doneX = doneY = 0
        for i in range(1, steps + 1):
            tx, ty = int(round(dx * i / steps)), int(round(dy * i / steps))
            win32.mouseMoveRel(tx - doneX, ty - doneY)
            doneX, doneY = tx, ty
            time.sleep(duration / steps)

    def scroll(self, notches, x=None, y=None, space="view"):
        if x is not None and y is not None:
            self.moveTo(x, y, space)
        else:
            self.ensureFocus()
        step = 1 if notches > 0 else -1
        for _ in range(abs(int(notches))):
            win32.mouseWheel(step)
            time.sleep(0.06)

    # ---------------------------------------------------------- capture

    def grab(self, method="auto"):
        """(PIL image, method used) of the client area. For ``blt`` / ``auto`` the game is
        focused first, since a covered window has nothing on screen to copy; ``print`` asks the
        window to render itself and leaves the foreground alone (works only from the helper when
        the game is elevated, and may come back black for some games)."""
        from PIL import Image
        if method != "print":
            self.ensureFocus()
        x, y, w, h = self.target.client()
        if w <= 0 or h <= 0:
            raise GameViewError("the window has an empty client area (minimised?)")
        used = method
        if method in ("auto", "blt"):
            raw = win32.grabScreen(x, y, w, h)
            image = Image.frombuffer("RGB", (w, h), raw, "raw", "BGRX", 0, 1)
            used = "blt"
            if method == "auto" and isBlack(image):
                method = "print"
        if method == "print":
            raw, w, h = win32.grabWindow(self.target.hwnd)
            image = Image.frombuffer("RGB", (w, h), raw, "raw", "BGRX", 0, 1)
            used = "print"
        return image.copy(), used


def isBlack(image, threshold=6):
    extrema = image.convert("L").getextrema()
    return extrema[1] <= threshold


def timestamp():
    return datetime.now().strftime("%Y%m%d-%H%M%S-%f")[:-3]


def matchesAny(name, patterns):
    return any(fnmatch.fnmatch(name.lower(), p.lower()) for p in patterns)
