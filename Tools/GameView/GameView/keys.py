"""Key names -> virtual-key codes, and the 3DMigoto hotkey syntax (``no_modifiers VK_F8``)."""

import re

NAMED = {
    "backspace": 0x08, "tab": 0x09, "enter": 0x0D, "return": 0x0D, "shift": 0x10, "ctrl": 0x11,
    "control": 0x11, "alt": 0x12, "menu": 0x12, "pause": 0x13, "capslock": 0x14, "esc": 0x1B,
    "escape": 0x1B, "space": 0x20, "pgup": 0x21, "pageup": 0x21, "pgdn": 0x22, "pagedown": 0x22,
    "end": 0x23, "home": 0x24, "left": 0x25, "up": 0x26, "right": 0x27, "down": 0x28,
    "printscreen": 0x2C, "snapshot": 0x2C, "insert": 0x2D, "ins": 0x2D, "delete": 0x2E, "del": 0x2E,
    "lwin": 0x5B, "rwin": 0x5C,
    "multiply": 0x6A, "add": 0x6B, "separator": 0x6C, "subtract": 0x6D, "decimal": 0x6E,
    "divide": 0x6F, "numlock": 0x90, "scrolllock": 0x91,
    "lshift": 0xA0, "rshift": 0xA1, "lctrl": 0xA2, "rctrl": 0xA3, "lalt": 0xA4, "ralt": 0xA5,
    ";": 0xBA, "semicolon": 0xBA, "=": 0xBB, "equals": 0xBB, ",": 0xBC, "comma": 0xBC,
    "-": 0xBD, "minus": 0xBD, ".": 0xBE, "period": 0xBE, "/": 0xBF, "slash": 0xBF,
    "`": 0xC0, "backtick": 0xC0, "grave": 0xC0, "[": 0xDB, "\\": 0xDC, "backslash": 0xDC,
    "]": 0xDD, "'": 0xDE, "quote": 0xDE,
}
for _i in range(10):
    NAMED[str(_i)] = 0x30 + _i
    NAMED["num{}".format(_i)] = 0x60 + _i
    NAMED["numpad{}".format(_i)] = 0x60 + _i
for _i in range(1, 25):
    NAMED["f{}".format(_i)] = 0x6F + _i
for _c in "abcdefghijklmnopqrstuvwxyz":
    NAMED[_c] = ord(_c.upper())

# Windows' own VK_ names, so a key copied out of a mod's .ini or d3dx.ini works as written.
VK_NAMES = {
    "VK_BACK": 0x08, "VK_TAB": 0x09, "VK_RETURN": 0x0D, "VK_SHIFT": 0x10, "VK_CONTROL": 0x11,
    "VK_MENU": 0x12, "VK_PAUSE": 0x13, "VK_CAPITAL": 0x14, "VK_ESCAPE": 0x1B, "VK_SPACE": 0x20,
    "VK_PRIOR": 0x21, "VK_NEXT": 0x22, "VK_END": 0x23, "VK_HOME": 0x24, "VK_LEFT": 0x25,
    "VK_UP": 0x26, "VK_RIGHT": 0x27, "VK_DOWN": 0x28, "VK_SNAPSHOT": 0x2C, "VK_INSERT": 0x2D,
    "VK_DELETE": 0x2E, "VK_MULTIPLY": 0x6A, "VK_ADD": 0x6B, "VK_SEPARATOR": 0x6C,
    "VK_SUBTRACT": 0x6D, "VK_DECIMAL": 0x6E, "VK_DIVIDE": 0x6F, "VK_NUMLOCK": 0x90,
    "VK_LSHIFT": 0xA0, "VK_RSHIFT": 0xA1, "VK_LCONTROL": 0xA2, "VK_RCONTROL": 0xA3,
    "VK_LMENU": 0xA4, "VK_RMENU": 0xA5, "VK_OEM_1": 0xBA, "VK_OEM_PLUS": 0xBB,
    "VK_OEM_COMMA": 0xBC, "VK_OEM_MINUS": 0xBD, "VK_OEM_PERIOD": 0xBE, "VK_OEM_2": 0xBF,
    "VK_OEM_3": 0xC0, "VK_OEM_4": 0xDB, "VK_OEM_5": 0xDC, "VK_OEM_6": 0xDD, "VK_OEM_7": 0xDE,
}
for _i in range(10):
    VK_NAMES["VK_NUMPAD{}".format(_i)] = 0x60 + _i
for _i in range(1, 25):
    VK_NAMES["VK_F{}".format(_i)] = 0x6F + _i

MODIFIERS = {0x10, 0x11, 0x12, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}


def vkOf(name):
    """One key: a friendly name (``f8``, ``esc``, ``num0``), a Windows name (``VK_OEM_4``), a
    3DMigoto-style bare name (``F9``), or a hex code (``0x77``)."""
    text = name.strip()
    if re.fullmatch(r"0x[0-9a-fA-F]{1,2}", text):
        return int(text, 16)
    upper = text.upper()
    if upper in VK_NAMES:
        return VK_NAMES[upper]
    if upper.startswith("VK_") and upper[3:].lower() in NAMED:
        return NAMED[upper[3:].lower()]
    lower = text.lower()
    if lower in NAMED:
        return NAMED[lower]
    raise ValueError("unknown key {!r}".format(name))


def parseChord(chord):
    """``ctrl+shift+f10`` -> [vk, vk, vk], modifiers first as written."""
    if chord in ("+", "plus"):
        return [0xBB]
    return [vkOf(part) for part in chord.split("+") if part]


def byVk(vk):
    """Keys that must be injected by virtual key rather than by scan code: the numpad (its scan
    codes collide with the navigation cluster when NumLock is off) and the modifiers' L/R forms."""
    return 0x60 <= vk <= 0x6F or vk == 0x90 or 0xA0 <= vk <= 0xA5


def parseMigotoHotkey(value):
    """``no_modifiers NO_VK_DECIMAL VK_F8`` -> (vk of the key, [vk of each REQUIRED modifier]).
    The last non-negated token is the key; earlier non-negated ones are held modifiers."""
    tokens = [t for t in re.split(r"\s+", value.split(";")[0].strip()) if t]
    wanted = [t for t in tokens if not t.lower().startswith("no_")]
    if not wanted:
        return None, []
    key = vkOf(wanted[-1])
    mods = []
    for token in wanted[:-1]:
        try:
            mods.append(vkOf(token))
        except ValueError:
            pass
    return key, mods
