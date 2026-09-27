"""`reload`'s wait, against a fake importer whose log is written the way 3DMigoto writes it
(no game needed). Run from Tools/GameView:

    py -3 -m unittest discover -s tests -v

The failure it pins: 3DMigoto logs the [Resource...] sections, then loads every Resource file
SILENTLY, then logs the [TextureOverride...] sections. A wait for "the log went quiet for 1.5 s"
ended in that gap, and `reload --mod` printed "no warnings" for CharlotteIdentity, Charlotte1/3/4/6
while their Duplicate-hash and Unrecognised-entry warnings were in the log (2026-09-23).
"""

import os
import shutil
import sys
import tempfile
import threading
import time
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import main  # noqa: E402
from GameView import migoto  # noqa: E402

BEFORE_GAP = r"""Reloading d3dx.ini (EXPERIMENTAL)...
[Resource\Mods\CharlotteIdentity\Charlotte.ini\CharlotteBlend]
  filename = CharlotteBlend.buf
"""
AFTER_GAP = r"""[TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlotteBlend]
  Hash=00000000c195ab20
WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=c195ab20
[TextureOverride\Mods\CharlotteIdentity\Charlotte.ini\CharlotteBlend]
[TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlotteBlend]
If this is intentional, add a match_priority=n to suppress warning and disambiguate order
> d3dx.ini reloaded
"""
# Deliberately no "> successfully reloaded shaders from ShaderFixes": with [Logging] unbuffered=0
# the reload's last lines stay in 3DMigoto's buffer until the next reload, so the wait must not
# depend on them.


class FakeSession:
    def __init__(self, press):
        self.press = press

    def chord(self, vks, hold=0.0):
        self.press()


class FakeCtx:
    def __init__(self, folder, press):
        self.folder = folder
        self.session = FakeSession(press)


class TestReloadWait(unittest.TestCase):
    def setUp(self):
        self.threads = []
        self.folder = tempfile.mkdtemp(prefix="gameview-test-")
        self.log = os.path.join(self.folder, migoto.LOG_NAME)
        with open(self.log, "w", encoding="utf-8") as f:
            f.write("older lines\n")

    def tearDown(self):
        for thread in self.threads:
            thread.join()
        shutil.rmtree(self.folder, ignore_errors=True)

    def _append(self, text):
        with open(self.log, "a", encoding="utf-8") as f:
            f.write(text)

    def _writer(self, gap, finish=True):
        def run():
            time.sleep(0.2)
            self._append(BEFORE_GAP)
            time.sleep(gap)  # the silent Resource-file loading
            self._append(AFTER_GAP if finish else AFTER_GAP.replace("> d3dx.ini reloaded\n", ""))
        def start():
            thread = threading.Thread(target=run, daemon=True)
            self.threads.append(thread)
            thread.start()
        return start

    def test_waits_through_the_silent_gap(self):
        ctx = FakeCtx(self.folder, self._writer(gap=2.5))
        text, seen, done = main._reload(ctx, maximum=15.0)
        self.assertTrue(seen)
        self.assertTrue(done)
        self.assertNotIn("older lines", text)
        found = migoto.problems(text, mod="CharlotteIdentity")
        self.assertEqual(len(found), 2, found)  # both listed sections

    def test_the_old_quiet_wait_would_have_cut_it(self):
        # Proves the fake reproduces the bug: the quiet-based wait stops in the gap.
        offset = migoto.logSize(self.folder)
        self._writer(gap=2.5)()
        time.sleep(0.4)
        text = migoto.waitLog(self.folder, offset, quiet=1.5, minimum=1.0, maximum=30.0)
        self.assertIn("Reloading d3dx.ini", text)
        self.assertEqual(migoto.problems(text, mod="CharlotteIdentity"), [])

    def test_unfinished_reload_is_reported_not_passed(self):
        ctx = FakeCtx(self.folder, self._writer(gap=0.3, finish=False))
        text, seen, done = main._reload(ctx, maximum=2.0)
        self.assertTrue(seen)
        self.assertFalse(done)


if __name__ == "__main__":
    unittest.main()
