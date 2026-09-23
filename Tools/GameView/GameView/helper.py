"""The elevated helper.

Genshin runs as administrator, and Windows' UIPI silently DROPS keyboard and mouse input that a
lower-integrity process sends to it: ``SendInput`` still reports success. An agent's shell is not
elevated, so without this every key press "works" and nothing happens in game.

The helper is a tiny localhost server, started elevated BY THE USER once per session
(``helper start`` -> one UAC prompt, or ``helper serve`` from an admin terminal), that re-runs a
GameView command line on the client's behalf (``main.py <argv>`` in a fresh child, so an edit to
the tool never needs a helper restart) and returns its output. It installs nothing and does not
survive a logoff; ``helper stop`` ends it. Requests carry a random token kept in the user's
profile.
"""

import hmac
import json
import os
import secrets
import socket
import subprocess
import sys
import threading
import time
from datetime import datetime

from . import config as cfg
from . import win32

DEFAULT_PORT = 47863
IN_HELPER_ENV = "AGREMAP_GAMEVIEW_IN_HELPER"
CREATE_NO_WINDOW = 0x08000000


def inHelper():
    return os.environ.get(IN_HELPER_ENV) == "1"


def _log(message):
    try:
        cfg.ensureScratch()
        with open(cfg.HELPER_LOG, "a", encoding="utf-8") as f:
            f.write("{} {}\n".format(datetime.now().isoformat(timespec="seconds"), message))
    except OSError:
        pass


def _sibling(exe, want):
    """python.exe <-> pythonw.exe in the same install."""
    lower = exe.lower()
    for name in ("pythonw.exe", "python.exe"):
        if lower.endswith(name):
            candidate = exe[:-len(name)] + want
            if os.path.isfile(candidate):
                return candidate
    return exe


def _recvLine(conn, limit=64 * 1024 * 1024):
    chunks = []
    total = 0
    while True:
        data = conn.recv(65536)
        if not data:
            break
        chunks.append(data)
        total += len(data)
        if b"\n" in data or total > limit:
            break
    return b"".join(chunks).split(b"\n", 1)[0]


def serve(port=DEFAULT_PORT):
    token = secrets.token_hex(24)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("127.0.0.1", port))
    sock.listen(8)
    cfg.ensureScratch()
    info = {"port": port, "token": token, "pid": os.getpid(), "elevated": win32.isSelfElevated(),
            "python": sys.executable, "started": datetime.now().isoformat(timespec="seconds")}
    tmp = str(cfg.HELPER_FILE) + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(info, f)
    os.replace(tmp, cfg.HELPER_FILE)
    _log("serving on 127.0.0.1:{} elevated={}".format(port, info["elevated"]))

    runLock = threading.Lock()
    stopping = threading.Event()

    def handle(conn):
        with conn:
            try:
                request = json.loads(_recvLine(conn).decode("utf-8"))
            except (ValueError, OSError):
                return
            if not hmac.compare_digest(str(request.get("token", "")), token):
                _log("rejected a request with a bad token")
                conn.sendall(b'{"rc": 99, "out": "", "err": "bad token"}\n')
                return
            op = request.get("op")
            if op == "ping":
                reply = {"rc": 0, "elevated": info["elevated"], "pid": info["pid"],
                         "busy": runLock.locked(), "started": info["started"]}
            elif op == "stop":
                reply = {"rc": 0}
                stopping.set()
            elif op == "run":
                argv = [str(a) for a in request.get("argv", [])]
                if argv and argv[0] == "helper":
                    reply = {"rc": 2, "out": "", "err": "the helper does not run helper commands\n"}
                else:
                    with runLock:  # one command at a time: two input streams would interleave
                        reply = _run(argv, float(request.get("timeout", 900)))
            else:
                reply = {"rc": 2, "out": "", "err": "unknown op {!r}\n".format(op)}
            try:
                conn.sendall((json.dumps(reply) + "\n").encode("utf-8"))
            except OSError:
                pass

    sock.settimeout(1.0)
    try:
        while not stopping.is_set():
            try:
                conn, _ = sock.accept()
            except socket.timeout:
                continue
            threading.Thread(target=handle, args=(conn,), daemon=True).start()
    finally:
        sock.close()
        try:
            with open(cfg.HELPER_FILE, encoding="utf-8") as f:
                mine = json.load(f).get("pid") == os.getpid()
            if mine:
                os.remove(cfg.HELPER_FILE)
        except (OSError, ValueError):
            pass
        _log("stopped")


def _run(argv, timeout):
    env = dict(os.environ)
    env[IN_HELPER_ENV] = "1"
    env["PYTHONIOENCODING"] = "utf-8"
    _log("run {}".format(argv))
    try:
        proc = subprocess.run([_sibling(sys.executable, "python.exe"), str(cfg.MAIN_PY)] + argv,
                              cwd=str(cfg.TOOL_DIR), env=env, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, timeout=timeout,
                              creationflags=CREATE_NO_WINDOW)
        return {"rc": proc.returncode, "out": proc.stdout.decode("utf-8", "replace"),
                "err": proc.stderr.decode("utf-8", "replace")}
    except subprocess.TimeoutExpired:
        return {"rc": 124, "out": "", "err": "timed out after {}s in the helper\n".format(timeout)}


# ---------------------------------------------------------------- client


def _info():
    try:
        with open(cfg.HELPER_FILE, encoding="utf-8") as f:
            return json.load(f)
    except (OSError, ValueError):
        return None


def request(payload, timeout=10.0):
    info = _info()
    if not info:
        return None
    payload = dict(payload, token=info["token"])
    try:
        with socket.create_connection(("127.0.0.1", info["port"]), timeout=timeout) as conn:
            conn.settimeout(timeout)
            conn.sendall((json.dumps(payload) + "\n").encode("utf-8"))
            return json.loads(_recvLine(conn).decode("utf-8"))
    except (OSError, ValueError):
        return None


def ping():
    """The helper's answer, or None. A reply without a pid is not an answer: a helper that has
    just started rewrites helper.json, so a ping holding the PREVIOUS session's token gets
    "bad token" back from the new one (seen after a reboot)."""
    reply = request({"op": "ping"}, timeout=3.0)
    return reply if reply and reply.get("rc") == 0 and "pid" in reply else None


def start(wait=60.0):
    """Start the helper elevated through ShellExecute("runas"): the USER approves the UAC prompt.
    Returns the ping reply, or None if it did not come up (prompt declined or not answered)."""
    reply = ping()
    if reply:
        return reply
    import ctypes
    pythonw = _sibling(sys.executable, "pythonw.exe")
    ctypes.windll.shell32.ShellExecuteW(None, "runas", pythonw,
                                        '"{}" helper serve'.format(cfg.MAIN_PY),
                                        str(cfg.TOOL_DIR), 0)
    deadline = time.time() + wait
    while time.time() < deadline:
        time.sleep(0.5)
        reply = ping()
        if reply:
            return reply
    return None


def stop():
    return request({"op": "stop"}, timeout=5.0)


NOT_RUNNING = (
    "This needs the ELEVATED helper: the game runs as administrator, and Windows drops input "
    "from a non-elevated process WITHOUT any error. It is not running. Ask the user to start it "
    "(one UAC prompt, once per session):\n"
    "    py -3 \"{0}\" helper start\n"
    "or from an admin terminal:  py -3 \"{0}\" helper serve\n")


def forward(argv, timeout):
    """Run ``main.py <argv>`` in the helper; print its output; return its exit code."""
    if not ping():
        sys.stderr.write(NOT_RUNNING.format(cfg.MAIN_PY))
        return 3
    result = request({"op": "run", "argv": argv, "timeout": timeout}, timeout=timeout + 30)
    if result is None:
        sys.stderr.write("the helper did not answer (see {})\n".format(cfg.HELPER_LOG))
        return 3
    if result.get("out"):
        sys.stdout.write(result["out"])
    if result.get("err"):
        sys.stderr.write(result["err"])
    return int(result.get("rc", 1))
