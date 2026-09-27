"""The d3d11_log.txt parsing in GameView.migoto, against lines copied from the real GIMI log
(old-loader 3DMigoto 1.3.16, 2026-09-23). Run from Tools/GameView:

    py -3 -m unittest discover -s tests -v
"""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from GameView import migoto  # noqa: E402

# A duplicate hash inside one fixed mod: the warning is logged while parsing the RemapFix1 .ini's
# section, and 3DMigoto lists BOTH sections carrying the hash after it.
SAME_MOD = r"""
[TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlottePositionCharlotteHurlockBodyRemapFix]
  Hash=00000000e35ce2c4
  run = commandlistcharlottepositioncharlottehurlockbodyremapfix
  $activecharacter = 1
WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=e35ce2c4
[TextureOverride\Mods\CharlotteIdentity\Charlotte.ini\CharlottePositionCharlotteHurlockBodyRemapFix]
[TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlottePositionCharlotteHurlockBodyRemapFix]
If this is intentional, add a match_priority=n to suppress warning and disambiguate order
[TextureOverride\Mods\CharlotteIdentity\CharlotteRemapFix1.ini\CharlotteTexcoord]
  Hash=0000000054841c9b
"""

# The same shape between two mods: logged while parsing ModB, but it concerns ModA as much.
CROSS_MOD = r"""
[TextureOverride\Mods\Charlotte1\merged.ini\CharlotteBlend]
  Hash=00000000c195ab20
[TextureOverride\Mods\Charlotte2\merged.ini\CharlotteBlend]
  Hash=00000000c195ab20
WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=c195ab20
[TextureOverride\Mods\Charlotte1\merged.ini\CharlotteBlend]
[TextureOverride\Mods\Charlotte2\merged.ini\CharlotteBlend]
If this is intentional, add a match_priority=n to suppress warning and disambiguate order
[TextureOverride\Mods\Charlotte2\merged.ini\CharlotteIB]
  Hash=00000000ff554aca
"""

UNRECOGNISED = r"""
[TextureOverride\Mods\CharlotteIdentity\Charlotte.ini\CharlotteVertexLimitRaiseCharlotteHurlockBodyRemapFix]
  Hash=00000000f45bbdaf
WARNING: Unrecognised entry: override_byte_stride = 40
WARNING: Unrecognised entry: override_vertex_count = 17670
"""

# What a reload looks like when the text is cut in the silent Resource-loading gap: the mod's
# Resource sections are there, its TextureOverride sections (and their warnings) are not.
CUT_IN_THE_GAP = r"""
Reloading d3dx.ini (EXPERIMENTAL)...
[Resource\Mods\CharlotteIdentity\Charlotte.ini\CharlotteBlend]
  type = Buffer
  stride = 32
  filename = CharlotteBlend.buf
[Resource\Mods\CharlotteIdentity\Charlotte.ini\CharlotteBodyDiffuse]
  filename = CharlotteBodyDiffuse.dds
"""

# Lines that must NOT be read as warnings, headers, or the end of a reload.
NOT_PROBLEMS = r"""
Reverting 664cfecfb45239b7 not found in ShaderFixes
If this is intentional, add a match_priority=n to suppress warning and disambiguate order
ShaderRegex: ps_5_0 6edc47d407c43250 matches [ShaderRegex\TexFx\censor]
FrameAnalysisContext(000001BC765DA0A0)::DrawIndexed(IndexCount:17790)
  Hash=00000000e35ce2c4
> successfully reloaded shaders from ShaderFixes
"""

PATH = "TextureOverride\\Mods\\CharlotteIdentity\\{}.ini\\CharlottePositionCharlotteHurlockBodyRemapFix"


class TestDuplicateHash(unittest.TestCase):
    def test_attributed_to_every_listed_section(self):
        found = migoto.problems(SAME_MOD, mod="CharlotteIdentity")
        self.assertEqual([s for _, s in found],
                         [PATH.format("Charlotte"), PATH.format("CharlotteRemapFix1")])
        for line, _ in found:
            self.assertIn("Duplicate TextureOverride hash=e35ce2c4", line)

    def test_listed_names_are_not_parse_context(self):
        # The listed lines must not become "the section being parsed": the next header is.
        text = SAME_MOD + "WARNING: Unrecognised entry: foo = 1\n"
        found = migoto.problems(text, mod="CharlotteIdentity")
        self.assertEqual(found[-1][1],
                         "TextureOverride\\Mods\\CharlotteIdentity\\CharlotteRemapFix1.ini\\CharlotteTexcoord")

    def test_cross_mod_conflict_reaches_both_mods(self):
        # Logged while parsing Charlotte2, but --mod Charlotte1 must see it too.
        for mod in ("Charlotte1", "Charlotte2"):
            found = migoto.problems(CROSS_MOD, mod=mod)
            self.assertEqual(len(found), 1, mod)
            self.assertEqual(found[0][1], "TextureOverride\\Mods\\{}\\merged.ini\\CharlotteBlend".format(mod))

    def test_other_mods_filtered_out(self):
        self.assertEqual(migoto.problems(SAME_MOD, mod="Charlotte1"), [])

    def test_unterminated_list_keeps_last_name_as_context(self):
        # Without the "If this is intentional" line, the last name may be a real header.
        text = ("WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=e35ce2c4\n"
                "[TextureOverride\\Mods\\A\\a.ini\\X]\n"
                "[TextureOverride\\Mods\\B\\b.ini\\Y]\n"
                "  Hash=00000000e35ce2c4\n"
                "WARNING: Unrecognised entry: foo = 1\n")
        found = migoto.problems(text)
        self.assertEqual(found[-1], ("WARNING: Unrecognised entry: foo = 1",
                                     "TextureOverride\\Mods\\B\\b.ini\\Y"))
        self.assertEqual([s for l, s in found if "Duplicate" in l],
                         ["TextureOverride\\Mods\\A\\a.ini\\X", "TextureOverride\\Mods\\B\\b.ini\\Y"])

    def test_conflict_pattern_is_only_the_conflict(self):
        self.assertTrue(migoto.CONFLICT.search(
            "WARNING: Possible Mod Conflict: Duplicate TextureOverride hash=e35ce2c4"))
        self.assertFalse(migoto.CONFLICT.search("WARNING: Unrecognised entry: override_vertex_count = 120"))
        self.assertFalse(migoto.CONFLICT.search(
            "If this is intentional, add a match_priority=n to suppress warning and disambiguate order"))


class TestOtherWarnings(unittest.TestCase):
    def test_unrecognised_entry_under_its_section(self):
        found = migoto.problems(UNRECOGNISED, mod="CharlotteIdentity")
        self.assertEqual(len(found), 2)
        self.assertTrue(all(s.endswith("CharlotteVertexLimitRaiseCharlotteHurlockBodyRemapFix")
                            for _, s in found))

    def test_lines_that_must_not_match(self):
        self.assertEqual(migoto.problems(NOT_PROBLEMS), [])
        # "matches [ShaderRegex\...]" mid-line is not a section header.
        self.assertEqual(migoto.sectionsParsed(NOT_PROBLEMS), set())


class TestNothingChecked(unittest.TestCase):
    def test_empty_text_parses_nothing(self):
        self.assertEqual(migoto.sectionsParsed("", "CharlotteIdentity"), set())

    def test_wrong_mod_parses_nothing(self):
        self.assertEqual(migoto.sectionsParsed(SAME_MOD, "Charlote*"), set())

    def test_clean_mod_is_distinguishable_from_unchecked(self):
        parsed = migoto.sectionsParsed(SAME_MOD, "CharlotteIdentity")
        # The two headers parsed here; Charlotte.ini's section is only NAMED by the duplicate
        # warning, so it does not count as parsed.
        self.assertEqual(len(parsed), 2)
        self.assertNotIn(PATH.format("Charlotte"), parsed)

    def test_cut_in_the_gap_has_no_texture_overrides(self):
        parsed = migoto.sectionsParsed(CUT_IN_THE_GAP, "CharlotteIdentity")
        self.assertEqual({s.split("\\")[0] for s in parsed}, {"Resource"})
        self.assertEqual(migoto.problems(CUT_IN_THE_GAP, mod="CharlotteIdentity"), [])


class TestReloadMarkers(unittest.TestCase):
    def test_done_matches_only_the_ini_reloaded_line(self):
        self.assertTrue(migoto.RELOAD_DONE.search("> d3dx.ini reloaded"))
        for line in ("Reloading d3dx.ini (EXPERIMENTAL)...",
                     "> reloading *_replace.txt fixes from ShaderFixes",
                     "> successfully reloaded shaders from ShaderFixes",
                     "Saving user settings to E:\\x\\GIMI\\d3dx_user.ini",
                     "----------- d3dx.ini settings -----------"):
            self.assertFalse(migoto.RELOAD_DONE.search(line), line)

    def test_done_is_line_anchored(self):
        self.assertTrue(migoto.RELOAD_DONE.search("x\n> d3dx.ini reloaded\ny"))
        self.assertFalse(migoto.RELOAD_DONE.search("echo > d3dx.ini reloaded"))


if __name__ == "__main__":
    unittest.main()
