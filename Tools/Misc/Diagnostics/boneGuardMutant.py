r"""Make the "target bone this remap cannot name" refusal REACHABLE, so it can be seen firing.

No pair registered today maps to a target bone at or past 512, so the corrected guard is correct and
silent -- indistinguishable from one that still cannot fire.

FIRST ATTEMPT, AND WHY IT PROVED NOTHING: shrinking `WWMIBlendRemapSize` itself to 64. That constant
is read by an EARLIER gate too --

    if (targetPast256_ && blendRemapBones_ > WWMIBlendRemapSize) { ...; targetPast256_ = false; }

-- which, with 64, fires first for Chisa's row, clears `targetPast256_`, and sends the fix to the
8-bit lift. `writeBlendRemap` is then never called and the guard under test is never reached: the run
reported `fixed 1 Blend.buf files and skipped 0`, which reads exactly like "the guard is still dead".
**A mutant that moves more than the thing under test is not a test of that thing.**

So this shrinks only the classification inside `writeBlendRemap`: bones at or past 64 become
unaddressable while the remap stays 512, the earlier gate still passes, and the refusal is due. That
is also the real gap between the two guards -- the earlier one asks how MANY distinct targets there
are, this one asks whether any single one is too high, and a row with 300 targets one of which is
bone 600 passes the first and must fail the second.

  py -3 boneGuardMutant.py on     # shrink the guard's own reach
  py -3 boneGuardMutant.py off    # restore
Rebuild between.
"""
import pathlib
import sys

path = (pathlib.Path("Anime Game Remap (for all users)/api/src/cpp/core")
        / "src/data/IniFixData/WWMIFixer.cpp")
real = """                if (static_cast<std::size_t>(dstBone) < WWMIBlendRemapSize) {
                    used.insert(static_cast<std::uint16_t>(dstBone));"""
mutant = """                if (static_cast<std::size_t>(dstBone) < 64) {   // MUTANT -- boneGuardMutant.py
                    used.insert(static_cast<std::uint16_t>(dstBone));"""

raw = path.read_bytes()
crlf = b"\r\n" in raw
text = raw.replace(b"\r\n", b"\n").decode("utf-8")

want = sys.argv[1] if len(sys.argv) > 1 else "on"
old, new = (real, mutant) if want == "on" else (mutant, real)

n = text.count(old)
assert n == 1, "{!r} found {} time(s) -- already {}?".format(old[:50], n, want)
text = text.replace(old, new)

blob = text.encode("utf-8")
if crlf:
    blob = blob.replace(b"\n", b"\r\n")

path.write_bytes(blob)
print("the guard's reach is {} now -- rebuild".format("64 (mutant)" if want == "on" else "WWMIBlendRemapSize"))
