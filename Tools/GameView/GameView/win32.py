"""Thin ctypes layer over the Win32 calls GameView needs: windows, processes, elevation, focus,
input injection and screen capture. No pywin32 dependency, so it runs on any CPython on Windows.

Every function that takes or returns a handle has explicit argtypes / restype: ctypes' default is
a 32-bit ``int``, which silently truncates a 64-bit ``HDC`` / ``HBITMAP`` and fails later in a
way that names nothing.
"""

import ctypes
import ctypes.wintypes as wt
import os
import time

user32 = ctypes.WinDLL("user32", use_last_error=True)
kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
gdi32 = ctypes.WinDLL("gdi32", use_last_error=True)
advapi32 = ctypes.WinDLL("advapi32", use_last_error=True)
shell32 = ctypes.WinDLL("shell32", use_last_error=True)

ULONG_PTR = ctypes.c_size_t


def _sig(fn, restype, *argtypes):
    fn.restype = restype
    fn.argtypes = list(argtypes)
    return fn


WNDENUMPROC = ctypes.WINFUNCTYPE(wt.BOOL, wt.HWND, wt.LPARAM)

_sig(user32.EnumWindows, wt.BOOL, WNDENUMPROC, wt.LPARAM)
_sig(user32.IsWindowVisible, wt.BOOL, wt.HWND)
_sig(user32.IsIconic, wt.BOOL, wt.HWND)
_sig(user32.IsWindow, wt.BOOL, wt.HWND)
_sig(user32.IsHungAppWindow, wt.BOOL, wt.HWND)
_sig(user32.GetWindow, wt.HWND, wt.HWND, wt.UINT)
_sig(user32.GetWindowTextW, ctypes.c_int, wt.HWND, wt.LPWSTR, ctypes.c_int)
_sig(user32.GetClassNameW, ctypes.c_int, wt.HWND, wt.LPWSTR, ctypes.c_int)
_sig(user32.GetWindowThreadProcessId, wt.DWORD, wt.HWND, ctypes.POINTER(wt.DWORD))
_sig(user32.GetWindowRect, wt.BOOL, wt.HWND, ctypes.POINTER(wt.RECT))
_sig(user32.GetClientRect, wt.BOOL, wt.HWND, ctypes.POINTER(wt.RECT))
_sig(user32.ClientToScreen, wt.BOOL, wt.HWND, ctypes.POINTER(wt.POINT))
_sig(user32.GetForegroundWindow, wt.HWND)
_sig(user32.SetForegroundWindow, wt.BOOL, wt.HWND)
_sig(user32.BringWindowToTop, wt.BOOL, wt.HWND)
_sig(user32.ShowWindow, wt.BOOL, wt.HWND, ctypes.c_int)
_sig(user32.AttachThreadInput, wt.BOOL, wt.DWORD, wt.DWORD, wt.BOOL)
_sig(user32.PostMessageW, wt.BOOL, wt.HWND, wt.UINT, wt.WPARAM, wt.LPARAM)
_sig(user32.GetDC, wt.HDC, wt.HWND)
_sig(user32.GetWindowDC, wt.HDC, wt.HWND)
_sig(user32.ReleaseDC, ctypes.c_int, wt.HWND, wt.HDC)
_sig(user32.PrintWindow, wt.BOOL, wt.HWND, wt.HDC, wt.UINT)
_sig(user32.GetSystemMetrics, ctypes.c_int, ctypes.c_int)
_sig(user32.MapVirtualKeyW, wt.UINT, wt.UINT, wt.UINT)
_sig(user32.GetCursorPos, wt.BOOL, ctypes.POINTER(wt.POINT))
_sig(kernel32.GetCurrentThreadId, wt.DWORD)
_sig(kernel32.OpenProcess, wt.HANDLE, wt.DWORD, wt.BOOL, wt.DWORD)
_sig(kernel32.CloseHandle, wt.BOOL, wt.HANDLE)
_sig(kernel32.CreateToolhelp32Snapshot, wt.HANDLE, wt.DWORD, wt.DWORD)
_sig(advapi32.OpenProcessToken, wt.BOOL, wt.HANDLE, wt.DWORD, ctypes.POINTER(wt.HANDLE))
_sig(advapi32.GetTokenInformation, wt.BOOL, wt.HANDLE, ctypes.c_int, ctypes.c_void_p, wt.DWORD,
     ctypes.POINTER(wt.DWORD))
_sig(gdi32.CreateCompatibleDC, wt.HDC, wt.HDC)
_sig(gdi32.CreateCompatibleBitmap, wt.HBITMAP, wt.HDC, ctypes.c_int, ctypes.c_int)
_sig(gdi32.SelectObject, wt.HGDIOBJ, wt.HDC, wt.HGDIOBJ)
_sig(gdi32.DeleteObject, wt.BOOL, wt.HGDIOBJ)
_sig(gdi32.DeleteDC, wt.BOOL, wt.HDC)
_sig(gdi32.BitBlt, wt.BOOL, wt.HDC, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, wt.HDC,
     ctypes.c_int, ctypes.c_int, wt.DWORD)


def setDpiAware():
    """Physical pixels everywhere; without this a 150%-scaled desktop reports a shrunken game."""
    try:
        user32.SetProcessDpiAwarenessContext.argtypes = [ctypes.c_void_p]
        if user32.SetProcessDpiAwarenessContext(ctypes.c_void_p(-4)):
            return
    except (AttributeError, OSError):
        pass
    try:
        ctypes.WinDLL("shcore").SetProcessDpiAwareness(2)
    except (AttributeError, OSError):
        user32.SetProcessDPIAware()


# ---------------------------------------------------------------- processes


class PROCESSENTRY32W(ctypes.Structure):
    _fields_ = [("dwSize", wt.DWORD), ("cntUsage", wt.DWORD), ("th32ProcessID", wt.DWORD),
                ("th32DefaultHeapID", ULONG_PTR), ("th32ModuleID", wt.DWORD),
                ("cntThreads", wt.DWORD), ("th32ParentProcessID", wt.DWORD),
                ("pcPriClassBase", ctypes.c_long), ("dwFlags", wt.DWORD),
                ("szExeFile", wt.WCHAR * 260)]


_sig(kernel32.Process32FirstW, wt.BOOL, wt.HANDLE, ctypes.POINTER(PROCESSENTRY32W))
_sig(kernel32.Process32NextW, wt.BOOL, wt.HANDLE, ctypes.POINTER(PROCESSENTRY32W))


def processNames():
    """{pid: exe name}, from a toolhelp snapshot -- which needs no handle on the process, so it
    works on an elevated or anti-cheat-protected game where OpenProcess is refused."""
    result = {}
    snap = kernel32.CreateToolhelp32Snapshot(0x2, 0)
    if not snap or snap == wt.HANDLE(-1).value:
        return result
    try:
        entry = PROCESSENTRY32W()
        entry.dwSize = ctypes.sizeof(entry)
        ok = kernel32.Process32FirstW(snap, ctypes.byref(entry))
        while ok:
            result[entry.th32ProcessID] = entry.szExeFile
            ok = kernel32.Process32NextW(snap, ctypes.byref(entry))
    finally:
        kernel32.CloseHandle(snap)
    return result


def isSelfElevated():
    try:
        return bool(shell32.IsUserAnAdmin())
    except (AttributeError, OSError):
        return False


def isProcessElevated(pid):
    """True / False, or None when the process cannot even be queried (access denied) -- which on
    this machine means an elevated or protected game, and is treated as elevated by the caller."""
    handle = kernel32.OpenProcess(0x1000, False, pid)  # PROCESS_QUERY_LIMITED_INFORMATION
    if not handle:
        return None
    try:
        token = wt.HANDLE()
        if not advapi32.OpenProcessToken(handle, 0x0008, ctypes.byref(token)):  # TOKEN_QUERY
            return None
        try:
            elevation = wt.DWORD()
            size = wt.DWORD()
            if not advapi32.GetTokenInformation(token, 20, ctypes.byref(elevation), 4,
                                                ctypes.byref(size)):  # TokenElevation
                return None
            return bool(elevation.value)
        finally:
            kernel32.CloseHandle(token)
    finally:
        kernel32.CloseHandle(handle)


# ---------------------------------------------------------------- windows


def windowText(hwnd):
    buf = ctypes.create_unicode_buffer(512)
    user32.GetWindowTextW(hwnd, buf, 512)
    return buf.value


def windowClass(hwnd):
    buf = ctypes.create_unicode_buffer(256)
    user32.GetClassNameW(hwnd, buf, 256)
    return buf.value


def windowPid(hwnd):
    pid = wt.DWORD()
    user32.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
    return pid.value


def topLevelWindows():
    """Visible, unowned top-level windows."""
    found = []

    def callback(hwnd, _):
        if user32.IsWindowVisible(hwnd) and not user32.GetWindow(hwnd, 4):  # GW_OWNER
            found.append(hwnd)
        return True

    user32.EnumWindows(WNDENUMPROC(callback), 0)
    return found


def windowRect(hwnd):
    rect = wt.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    return rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top


def clientRect(hwnd):
    """(screen x, screen y, width, height) of the client area."""
    rect = wt.RECT()
    user32.GetClientRect(hwnd, ctypes.byref(rect))
    origin = wt.POINT(0, 0)
    user32.ClientToScreen(hwnd, ctypes.byref(origin))
    return origin.x, origin.y, rect.right - rect.left, rect.bottom - rect.top


def virtualScreen():
    return (user32.GetSystemMetrics(76), user32.GetSystemMetrics(77),
            user32.GetSystemMetrics(78), user32.GetSystemMetrics(79))


def foreground():
    return user32.GetForegroundWindow()


def focus(hwnd, timeout=3.0):
    """Bring a window to the foreground and wait until it is.

    A game only reads input while it is foreground, and 3DMigoto's own hotkeys
    (``check_foreground_window=1``) likewise. Windows' foreground lock refuses a plain
    SetForegroundWindow from a background process, so this attaches to the foreground thread's
    input queue first and falls back to the ALT-tap trick."""
    if not hwnd or not user32.IsWindow(hwnd):
        return False
    if user32.IsIconic(hwnd):
        user32.ShowWindow(hwnd, 9)  # SW_RESTORE
    if user32.GetForegroundWindow() == hwnd:
        return True

    current = kernel32.GetCurrentThreadId()
    fgThread = user32.GetWindowThreadProcessId(user32.GetForegroundWindow(), None)
    attached = fgThread and fgThread != current and user32.AttachThreadInput(current, fgThread, True)
    try:
        user32.BringWindowToTop(hwnd)
        user32.SetForegroundWindow(hwnd)
    finally:
        if attached:
            user32.AttachThreadInput(current, fgThread, False)

    deadline = time.time() + timeout
    triedAlt = False
    while time.time() < deadline:
        if user32.GetForegroundWindow() == hwnd:
            time.sleep(0.15)  # let the game see WM_ACTIVATE before input arrives
            return True
        if not triedAlt and time.time() > deadline - timeout / 2:
            triedAlt = True
            sendKeyVk(0x12, True)
            sendKeyVk(0x12, False)
            user32.SetForegroundWindow(hwnd)
        time.sleep(0.05)
    return user32.GetForegroundWindow() == hwnd


# ---------------------------------------------------------------- input


class MOUSEINPUT(ctypes.Structure):
    _fields_ = [("dx", wt.LONG), ("dy", wt.LONG), ("mouseData", wt.DWORD), ("dwFlags", wt.DWORD),
                ("time", wt.DWORD), ("dwExtraInfo", ULONG_PTR)]


class KEYBDINPUT(ctypes.Structure):
    _fields_ = [("wVk", wt.WORD), ("wScan", wt.WORD), ("dwFlags", wt.DWORD), ("time", wt.DWORD),
                ("dwExtraInfo", ULONG_PTR)]


class HARDWAREINPUT(ctypes.Structure):
    _fields_ = [("uMsg", wt.DWORD), ("wParamL", wt.WORD), ("wParamH", wt.WORD)]


class _INPUTUNION(ctypes.Union):
    _fields_ = [("mi", MOUSEINPUT), ("ki", KEYBDINPUT), ("hi", HARDWAREINPUT)]


class INPUT(ctypes.Structure):
    _fields_ = [("type", wt.DWORD), ("u", _INPUTUNION)]


_sig(user32.SendInput, wt.UINT, wt.UINT, ctypes.POINTER(INPUT), ctypes.c_int)

KEYEVENTF_EXTENDEDKEY = 0x1
KEYEVENTF_KEYUP = 0x2
KEYEVENTF_UNICODE = 0x4
KEYEVENTF_SCANCODE = 0x8

MOUSE_MOVE = 0x1
MOUSE_LEFTDOWN, MOUSE_LEFTUP = 0x2, 0x4
MOUSE_RIGHTDOWN, MOUSE_RIGHTUP = 0x8, 0x10
MOUSE_MIDDLEDOWN, MOUSE_MIDDLEUP = 0x20, 0x40
MOUSE_WHEEL = 0x800
MOUSE_VIRTUALDESK = 0x4000
MOUSE_ABSOLUTE = 0x8000

BUTTONS = {"left": (MOUSE_LEFTDOWN, MOUSE_LEFTUP), "right": (MOUSE_RIGHTDOWN, MOUSE_RIGHTUP),
           "middle": (MOUSE_MIDDLEDOWN, MOUSE_MIDDLEUP)}


def _send(*inputs):
    arr = (INPUT * len(inputs))(*inputs)
    sent = user32.SendInput(len(inputs), arr, ctypes.sizeof(INPUT))
    if sent != len(inputs):
        raise OSError("SendInput injected {} of {} events (error {})".format(
            sent, len(inputs), ctypes.get_last_error()))


def _keyInput(vk, scan, flags):
    item = INPUT(type=1)
    item.u.ki = KEYBDINPUT(wVk=vk, wScan=scan, dwFlags=flags, time=0, dwExtraInfo=0)
    return item


def sendKeyScan(vk, down):
    """By SCAN CODE, which is what a game's raw input reads (Unity / Unreal ignore a bare VK)."""
    scan = user32.MapVirtualKeyW(vk, 4)  # MAPVK_VK_TO_VSC_EX: 0xE0xx for an extended key
    flags = KEYEVENTF_SCANCODE
    if (scan >> 8) in (0xE0, 0xE1):
        flags |= KEYEVENTF_EXTENDEDKEY
    if not down:
        flags |= KEYEVENTF_KEYUP
    _send(_keyInput(0, scan & 0xFF, flags))


def sendKeyVk(vk, down):
    """By VIRTUAL KEY (scan code filled in too). Numpad keys go this way: their scan codes are
    shared with the navigation cluster and mean VK_INSERT etc. whenever NumLock is off, and
    3DMigoto's ``VK_NUMPAD0`` hotkey is read by VK."""
    scan = user32.MapVirtualKeyW(vk, 0)
    _send(_keyInput(vk, scan, 0 if down else KEYEVENTF_KEYUP))


def sendUnicode(text):
    for ch in text:
        code = ord(ch)
        _send(_keyInput(0, code, KEYEVENTF_UNICODE),
              _keyInput(0, code, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP))
        time.sleep(0.01)


def _mouseInput(dx=0, dy=0, data=0, flags=0):
    item = INPUT(type=0)
    item.u.mi = MOUSEINPUT(dx=dx, dy=dy, mouseData=data & 0xFFFFFFFF, dwFlags=flags, time=0,
                           dwExtraInfo=0)
    return item


def mouseMoveAbs(x, y):
    vx, vy, vw, vh = virtualScreen()
    nx = int(round((x - vx) * 65535 / max(vw - 1, 1)))
    ny = int(round((y - vy) * 65535 / max(vh - 1, 1)))
    _send(_mouseInput(nx, ny, 0, MOUSE_MOVE | MOUSE_ABSOLUTE | MOUSE_VIRTUALDESK))


def mouseMoveRel(dx, dy):
    _send(_mouseInput(int(dx), int(dy), 0, MOUSE_MOVE))


def mouseButton(button, down):
    downFlag, upFlag = BUTTONS[button]
    _send(_mouseInput(0, 0, 0, downFlag if down else upFlag))


def mouseWheel(notches):
    _send(_mouseInput(0, 0, int(notches * 120), MOUSE_WHEEL))


def cursorPos():
    point = wt.POINT()
    user32.GetCursorPos(ctypes.byref(point))
    return point.x, point.y


# ---------------------------------------------------------------- capture


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
                ("biPlanes", wt.WORD), ("biBitCount", wt.WORD), ("biCompression", wt.DWORD),
                ("biSizeImage", wt.DWORD), ("biXPelsPerMeter", wt.LONG),
                ("biYPelsPerMeter", wt.LONG), ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD)]


class BITMAPINFO(ctypes.Structure):
    _fields_ = [("bmiHeader", BITMAPINFOHEADER), ("bmiColors", wt.DWORD * 3)]


_sig(gdi32.GetDIBits, ctypes.c_int, wt.HDC, wt.HBITMAP, wt.UINT, wt.UINT, ctypes.c_void_p,
     ctypes.POINTER(BITMAPINFO), wt.UINT)


def _readBitmap(memDc, bitmap, width, height):
    info = BITMAPINFO()
    info.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    info.bmiHeader.biWidth = width
    info.bmiHeader.biHeight = -height  # top-down
    info.bmiHeader.biPlanes = 1
    info.bmiHeader.biBitCount = 32
    buf = ctypes.create_string_buffer(width * height * 4)
    if not gdi32.GetDIBits(memDc, bitmap, 0, height, buf, ctypes.byref(info), 0):
        raise OSError("GetDIBits failed ({})".format(ctypes.get_last_error()))
    return buf.raw


def grabScreen(x, y, width, height):
    """BGRX bytes of a screen rectangle, via the desktop DC (what DWM composited). Works on a
    borderless or windowed DX11 game; an EXCLUSIVE-fullscreen one comes back black."""
    screenDc = user32.GetDC(None)
    memDc = gdi32.CreateCompatibleDC(screenDc)
    bitmap = gdi32.CreateCompatibleBitmap(screenDc, width, height)
    old = gdi32.SelectObject(memDc, bitmap)
    try:
        if not gdi32.BitBlt(memDc, 0, 0, width, height, screenDc, x, y, 0x00CC0020):  # SRCCOPY
            raise OSError("BitBlt failed ({})".format(ctypes.get_last_error()))
        return _readBitmap(memDc, bitmap, width, height)
    finally:
        gdi32.SelectObject(memDc, old)
        gdi32.DeleteObject(bitmap)
        gdi32.DeleteDC(memDc)
        user32.ReleaseDC(None, screenDc)


def grabWindow(hwnd):
    """BGRX bytes of a window's CLIENT area via PrintWindow(PW_RENDERFULLCONTENT) -- the fallback
    for when the desktop grab comes back black. Returns (bytes, width, height)."""
    wx, wy, ww, wh = windowRect(hwnd)
    cx, cy, cw, ch = clientRect(hwnd)
    windowDc = user32.GetWindowDC(hwnd)
    memDc = gdi32.CreateCompatibleDC(windowDc)
    bitmap = gdi32.CreateCompatibleBitmap(windowDc, ww, wh)
    old = gdi32.SelectObject(memDc, bitmap)
    try:
        if not user32.PrintWindow(hwnd, memDc, 2):
            raise OSError("PrintWindow failed ({})".format(ctypes.get_last_error()))
        raw = _readBitmap(memDc, bitmap, ww, wh)
    finally:
        gdi32.SelectObject(memDc, old)
        gdi32.DeleteObject(bitmap)
        gdi32.DeleteDC(memDc)
        user32.ReleaseDC(hwnd, windowDc)

    offX, offY = cx - wx, cy - wy
    if (offX, offY, cw, ch) == (0, 0, ww, wh):
        return raw, ww, wh
    rows = []
    stride = ww * 4
    for row in range(offY, offY + ch):
        start = row * stride + offX * 4
        rows.append(raw[start:start + cw * 4])
    return b"".join(rows), cw, ch


def postClose(hwnd):
    return bool(user32.PostMessageW(hwnd, 0x0010, 0, 0))  # WM_CLOSE


def terminate(pid):
    handle = kernel32.OpenProcess(0x0001, False, pid)  # PROCESS_TERMINATE
    if not handle:
        return False
    try:
        _sig(kernel32.TerminateProcess, wt.BOOL, wt.HANDLE, wt.UINT)
        return bool(kernel32.TerminateProcess(handle, 1))
    finally:
        kernel32.CloseHandle(handle)


def isWindows():
    return os.name == "nt"
