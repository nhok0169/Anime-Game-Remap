import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class IniClassifierTest(BaseUnitTest):
    """
    Tests for :class:`IniClassifier`. Bare-named -- the older, pure-Python
    ``IniClassifier``/``IniClassifierBuilder`` this class name used to belong to has been deleted
    outright (it lived under the now-removed ``model/strategies/iniClassifiers/`` package as
    ``IniClassifierOld``/``IniClassifierBuilderOld``), so this C++-backed replacement graduated out
    of its temporary ``Cpp``-prefixed name into the bare one.
    """

    def setUp(self):
        super().setUp()
        self.gi = int(FRB.GameTypeId.GI)
        self.wuwa = int(FRB.GameTypeId.WuWa)
        self.amberId = int(FRB.ModTypeId.Amber)
        self.raidenId = int(FRB.ModTypeId.Raiden)
        self.jeanId = int(FRB.ModTypeId.Jean)
        self.jeanCNId = int(FRB.ModTypeId.JeanCN)
        self.jeanSeaId = int(FRB.ModTypeId.JeanSea)

    # =================== __init__ =====================

    def test_isSubclassOfBaseIniClassifier(self):
        self.assertTrue(issubclass(FRB.IniClassifier, FRB.BaseIniClassifier))

    def test_defaultConstruct_checkHasTextureOverrideDefaultsTrue(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        # no "TextureOverride" prefix -> blocked by the default checkHasTextureOverride=True
        stats = c.classify("[AmberKeyword]\n")
        self.compareDict(stats.modType, {})

    # ================================================
    # ================= addGIModType ====================

    def test_newModTypeId_addGIModTypeReturnsTrue(self):
        c = FRB.IniClassifier()
        self.assertTrue(c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set()))

    def test_alreadyRegisteredModTypeId_addGIModTypeReturnsFalse(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        self.assertFalse(c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"anotherhash"}, set()))

    def test_wrongGameTypeId_addGIModTypeReturnsFalse(self):
        c = FRB.IniClassifier()
        self.assertFalse(c.addGIModType(FRB.ModTypeIdData(self.wuwa, self.amberId), {"deadbeef"}, set()))

    # ================================================
    # ================ addWuWaModType ===================

    def test_newModTypeId_addWuWaModTypeReturnsTrue(self):
        c = FRB.IniClassifier()
        self.assertTrue(c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"cafebabe"}))

    def test_alreadyRegisteredModTypeId_addWuWaModTypeReturnsFalse(self):
        c = FRB.IniClassifier()
        c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"cafebabe"})

        self.assertFalse(c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"anotherhash"}))

    def test_wrongGameTypeId_addWuWaModTypeReturnsFalse(self):
        c = FRB.IniClassifier()
        self.assertFalse(c.addWuWaModType(FRB.ModTypeIdData(self.gi, self.raidenId), {"cafebabe"}))

    # ================================================
    # =================== getModType =====================

    def test_registeredModTypeId_getModTypeReturnsIt(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        result = c.getModType(self.amberId)

        self.assertEqual(result.gameTypeId, self.gi)
        self.assertEqual(result.modTypeId, self.amberId)

    def test_unregisteredModTypeId_getModTypeRaisesIndexError(self):
        c = FRB.IniClassifier()

        with self.assertRaises(IndexError):
            c.getModType(999)

    # ================================================
    # ==================== classify ======================

    def test_hashMatch_GI_classifiedCorrectly(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        stats = c.classify("hash = deadbeef\n")

        self.assertTrue(stats.isMod)
        self.assertIn(self.amberId, stats.modType)

    def test_sectionNameMatch_GI_classifiedCorrectly(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        stats = c.classify("[TextureOverrideAmberKeyword]\n")

        self.assertIn(self.amberId, stats.modType)

    def test_noMatch_notClassifiedAsMod(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        stats = c.classify("hash = unrelated\n")

        self.assertFalse(stats.isMod)
        self.compareDict(stats.modType, {})

    def test_remapSubstringInSectionName_isFixedSetRegardlessOfMatch(self):
        c = FRB.IniClassifier()

        stats = c.classify("[TextureOverrideAmberRemap]\n")

        self.assertTrue(stats.isFixed)

    # ---- TextureOverride/ShaderOverride prefix -> isMod ----

    def test_textureOverridePrefix_isModSetTrueEvenWithNoModTypeMatch(self):
        c = FRB.IniClassifier()

        stats = c.classify("[TextureOverrideRandomThing]\n")

        self.assertTrue(stats.isMod)
        self.compareDict(stats.modType, {})

    def test_shaderOverridePrefix_isModSetTrueEvenWhenCheckHasTextureOverrideBlocksFurtherProcessing(self):
        # checkHasTextureOverride defaults to True, which blocks keyword-based classification for a
        # non-"TextureOverride" section -- but the isMod check itself runs unconditionally, before
        # that gate, so a "ShaderOverride" section is still recognized as belonging to a mod
        c = FRB.IniClassifier()

        stats = c.classify("[ShaderOverrideRandomThing]\n")

        self.assertTrue(stats.isMod)
        self.compareDict(stats.modType, {})

    def test_neitherOverridePrefix_isModStaysFalse(self):
        c = FRB.IniClassifier()

        stats = c.classify("[SomeRandomSection]\n")

        self.assertFalse(stats.isMod)

    def test_shaderOverridePrefix_checkHasTextureOverrideFalse_stillMatchesRegisteredKeyword(self):
        c = FRB.IniClassifier(checkHasTextureOverride = False)
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        stats = c.classify("[ShaderOverrideAmberKeyword]\n")

        self.assertTrue(stats.isMod)
        self.assertIn(self.amberId, stats.modType)

    def test_listOfLinesOverload_worksTheSameAsFullText(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        stats = c.classify(["hash = deadbeef\n"])

        self.assertIn(self.amberId, stats.modType)

    def test_gameTypeIdArg_restrictsSectionKeywordMatchingToThatGame(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        stats = c.classify("[TextureOverrideAmberKeyword]\n", FRB.GameTypeId.GI)

        self.assertIn(self.amberId, stats.modType)

    # ---- checkHasTextureOverride ----

    def test_checkHasTextureOverrideTrue_blocksSectionWithoutPrefix(self):
        c = FRB.IniClassifier(checkHasTextureOverride = True)
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        stats = c.classify("[AmberKeyword]\n")

        self.compareDict(stats.modType, {})

    def test_checkHasTextureOverrideFalse_allowsSectionWithoutPrefix(self):
        c = FRB.IniClassifier(checkHasTextureOverride = False)
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"AmberKeyword"})

        stats = c.classify("[AmberKeyword]\n")

        self.assertIn(self.amberId, stats.modType)

    # ---- WuWa marker/hash ordering ----

    def test_wuwaMarkerBeforeHash_directHitClassified(self):
        c = FRB.IniClassifier()
        c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"cafebabe"})

        stats = c.classify("$\\WWMIv1\nhash = cafebabe\n")

        self.assertTrue(stats.isMod)
        self.assertIn(self.raidenId, stats.modType)

    def test_wuwaHashBeforeMarker_savedThenClassifiedOnMarker(self):
        c = FRB.IniClassifier()
        c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"cafebabe"})

        stats = c.classify("hash = cafebabe\n$\\WWMIv1\n")

        self.assertTrue(stats.isMod)
        self.assertIn(self.raidenId, stats.modType)

    def test_wuwaHashWithNoMarkerAtAll_notClassified(self):
        c = FRB.IniClassifier()
        c.addWuWaModType(FRB.ModTypeIdData(self.wuwa, self.raidenId), {"cafebabe"})

        stats = c.classify("hash = cafebabe\n")

        self.assertFalse(stats.isMod)
        self.compareDict(stats.modType, {})

    # ---- tie-breaking / distribution ----

    def test_tiedModTypeIds_allIncludedSortedAscending(self):
        c = FRB.IniClassifier()
        # deliberately registered out of ascending order to prove sorting, not registration order
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.raidenId), {"cafebabe"}, set())
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        stats = c.classify(["hash = deadbeef\n", "hash = cafebabe\n"])

        self.compareList(sorted(stats.modType.keys()), sorted([self.amberId, self.raidenId]))

    def test_strictlyHigherCount_onlyThatModTypeIdIncluded(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.raidenId), {"cafebabe"}, set())

        # matched twice (hash worth 2 points) vs. once -> Amber strictly wins
        stats = c.classify(["hash = deadbeef\n", "hash = deadbeef\n", "hash = cafebabe\n"])

        # ModTypeIdData has no __eq__ (bare pybind11 class, compares by identity) -- compare keys only
        self.compareList(list(stats.modType.keys()), [self.amberId])

    # ---- 2 ModTypes sharing the same hash/keyword (collision fix) ----

    def test_twoModTypesShareSameHash_bothClassified(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.raidenId), {"deadbeef"}, set())

        stats = c.classify("hash = deadbeef\n")

        self.compareList(sorted(stats.modType.keys()), sorted([self.amberId, self.raidenId]))

    def test_twoModTypesShareSameSectionKeyword_bothClassified(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), set(), {"SharedKeyword"})
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.raidenId), set(), {"SharedKeyword"})

        stats = c.classify("[TextureOverrideSharedKeyword]\n")

        self.compareList(sorted(stats.modType.keys()), sorted([self.amberId, self.raidenId]))

    # ---- no state leak between calls ----

    def test_repeatedClassifyCalls_noStateLeakBetweenCalls(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        first = c.classify("hash = deadbeef\n")
        second = c.classify("hash = unrelated\n")

        self.assertIn(self.amberId, first.modType)
        self.compareDict(second.modType, {})

    # ---- HideOriginalComment ----

    def test_hideOriginalCommentPrefix_hashLineStillReadNormally(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanId), {"jeanhash"}, set())

        stats = c.classify(";RemapFixHideOrig -->hash = jeanhash\n")

        self.assertIn(self.jeanId, stats.modType)

    def test_hideOriginalCommentPrefix_sectionHeaderStillReadNormally(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanId), set(), {"JeanKeyword"})

        stats = c.classify(";RemapFixHideOrig -->[TextureOverrideJeanKeyword]\n")

        self.assertIn(self.jeanId, stats.modType)

    # ---- Remap-section hash skipping ----

    def test_hashInsideRemapNamedSection_notCounted(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanCNId), {"jeancnhash"}, set())

        stats = c.classify("[TextureOverrideJeanCNRemapFix]\nhash = jeancnhash\n")

        self.assertNotIn(self.jeanCNId, stats.modType)

    def test_hashInsideNonRemapSection_stillCounted(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanId), {"jeanhash"}, set())

        stats = c.classify("[TextureOverrideJean]\nhash = jeanhash\n")

        self.assertIn(self.jeanId, stats.modType)

    def test_sectionKeywordInsideRemapNamedSection_notCounted(self):
        # A "Remap"-named section that ALSO matches a registered section-keyword (not just a
        # hash) should not increment #modTypeIdDistribution either -- readSectionName returns
        # right after setting stats.isFixed, before searching for a keyword match at all.
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanCNId), set(), {"JeanCNKeyword"})

        stats = c.classify("[TextureOverrideJeanCNRemapFixJeanCNKeyword]\n")

        self.assertNotIn(self.jeanCNId, stats.modType)
        self.assertTrue(stats.isFixed)

    def test_sectionKeywordInsideNonRemapSection_stillCounted(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanCNId), set(), {"JeanCNKeyword"})

        stats = c.classify("[TextureOverrideJeanCNKeyword]\n")

        self.assertIn(self.jeanCNId, stats.modType)

    def test_alreadyFixedJean_classifiesAsJeanOnly(self):
        # Exactly the scenario this feature exists for: an already-fixed Jean .ini (fixed to also
        # cover JeanCN and JeanSea) should classify as Jean only, not Jean+JeanCN+JeanSea.
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanId), {"jeanhash"}, set())
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanCNId), {"jeancnhash"}, set())
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.jeanSeaId), {"jeanseahash"}, set())

        iniTxt = (
            ";RemapFixHideOrig -->[TextureOverrideJean]\n"
            ";RemapFixHideOrig -->hash = jeanhash\n"
            "\n"
            "[TextureOverrideJeanCNRemapFix]\n"
            "hash = jeancnhash\n"
            "\n"
            "[TextureOverrideJeanSeaRemapFix]\n"
            "hash = jeanseahash\n"
        )
        stats = c.classify(iniTxt)

        self.compareList(list(stats.modType.keys()), [self.jeanId])

    # ================================================
    # ==================== checkIsMod ======================

    def test_baseIniClassifier_checkIsModAlwaysFalse(self):
        base = FRB.BaseIniClassifier()

        self.assertFalse(base.checkIsMod("[TextureOverrideAnything]\n"))
        self.assertFalse(base.checkIsMod(["[TextureOverrideAnything]\n"]))

    def test_textureOverridePrefix_checkIsModReturnsTrue(self):
        c = FRB.IniClassifier()

        self.assertTrue(c.checkIsMod("[TextureOverrideRandomThing]\n"))

    def test_noModLikeContent_checkIsModReturnsFalse(self):
        c = FRB.IniClassifier()

        self.assertFalse(c.checkIsMod("[SomeRandomSection]\n"))

    def test_hashMatch_checkIsModReturnsTrue(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        self.assertTrue(c.checkIsMod("hash = deadbeef\n"))

    def test_checkIsMod_matchesClassifysIsModForIdenticalInput(self):
        c1 = FRB.IniClassifier()
        c1.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        stats = c1.classify("hash = deadbeef\nhash = unrelated\n")

        c2 = FRB.IniClassifier()
        c2.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        isMod = c2.checkIsMod("hash = deadbeef\nhash = unrelated\n")

        self.assertEqual(isMod, stats.isMod)

    def test_checkIsMod_noStateLeakBetweenRepeatedCalls(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        c.checkIsMod("hash = deadbeef\n")
        second = c.checkIsMod("hash = unrelated\n")

        self.assertFalse(second)

    def test_checkIsMod_listOfLinesOverloadWorksTheSameAsFullText(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())

        self.assertTrue(c.checkIsMod(["hash = deadbeef\n"]))

    def test_shaderOverridePrefix_checkIsModReturnsTrue(self):
        # same isMod-independent-of-checkHasTextureOverride rule classify() itself follows
        c = FRB.IniClassifier()

        self.assertTrue(c.checkIsMod("[ShaderOverrideRandomThing]\n"))

    # ================================================
    # ================ checkIsFixedMod ==================

    def test_baseIniClassifier_checkIsFixedModAlwaysFalse(self):
        base = FRB.BaseIniClassifier()

        self.assertEqual(base.checkIsFixedMod("[TextureOverrideAmberRemap]\n"), (False, False))
        self.assertEqual(base.checkIsFixedMod(["[TextureOverrideAmberRemap]\n"]), (False, False))

    def test_isModTrueIsFixedFalse_checkIsFixedModReflectsBoth(self):
        c = FRB.IniClassifier()

        # TextureOverride prefix alone -> isMod=True, no "Remap" substring -> isFixed stays False
        isFixed, isMod = c.checkIsFixedMod("[TextureOverrideRandomThing]\n")
        self.assertFalse(isFixed)
        self.assertTrue(isMod)

    def test_isModFalseIsFixedTrue_checkIsFixedModReflectsBoth(self):
        # checkHasTextureOverride=False is required here: with it on (the default), a non-
        # TextureOverride/ShaderOverride-prefixed section early-returns before the "Remap"
        # substring check even runs, so isFixed would stay False too, not just isMod.
        c = FRB.IniClassifier(checkHasTextureOverride = False)

        isFixed, isMod = c.checkIsFixedMod("[SomeRandomRemapSection]\n")
        self.assertTrue(isFixed)
        self.assertFalse(isMod)

    def test_bothIsModAndIsFixedTrue_checkIsFixedModReflectsBoth(self):
        c = FRB.IniClassifier()

        isFixed, isMod = c.checkIsFixedMod("[TextureOverrideAmberRemap]\n")
        self.assertTrue(isFixed)
        self.assertTrue(isMod)

    def test_neitherIsModNorIsFixed_checkIsFixedModReflectsBoth(self):
        c = FRB.IniClassifier()

        isFixed, isMod = c.checkIsFixedMod("[SomeRandomSection]\n")
        self.assertFalse(isFixed)
        self.assertFalse(isMod)

    def test_checkIsFixedMod_matchesClassifysFlagsForIdenticalInput(self):
        c1 = FRB.IniClassifier()
        c1.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        stats = c1.classify("[TextureOverrideAmberRemap]\nhash = deadbeef\n")

        c2 = FRB.IniClassifier()
        c2.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        isFixed, isMod = c2.checkIsFixedMod("[TextureOverrideAmberRemap]\nhash = deadbeef\n")

        self.assertEqual(isFixed, stats.isFixed)
        self.assertEqual(isMod, stats.isMod)

    def test_checkIsFixedMod_noStateLeakBetweenRepeatedCalls(self):
        c = FRB.IniClassifier()

        c.checkIsFixedMod("[TextureOverrideAmberRemap]\n")
        isFixed, isMod = c.checkIsFixedMod("[SomeRandomSection]\n")

        self.assertFalse(isFixed)
        self.assertFalse(isMod)

    def test_checkIsFixedMod_listOfLinesOverloadWorksTheSameAsFullText(self):
        c = FRB.IniClassifier()

        self.assertEqual(c.checkIsFixedMod(["[TextureOverrideAmberRemap]\n"]), (True, True))

    # ================================================
    # ==================== clear =========================

    def test_clear_registeredModTypesForgotten(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        self.assertEqual(c.getModType(self.amberId).modTypeId, self.amberId)

        c.clear()

        with self.assertRaises(IndexError):
            c.getModType(self.amberId)

    def test_clear_previouslyMatchingHashNoLongerClassified(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        c.clear()

        stats = c.classify("hash = deadbeef\n")

        self.assertFalse(stats.isMod)
        self.compareDict(stats.modType, {})

    def test_clear_afterClearCanReRegisterSameModTypeId(self):
        c = FRB.IniClassifier()
        c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"deadbeef"}, set())
        c.clear()

        self.assertTrue(c.addGIModType(FRB.ModTypeIdData(self.gi, self.amberId), {"newhash"}, set()))

    # ================================================
