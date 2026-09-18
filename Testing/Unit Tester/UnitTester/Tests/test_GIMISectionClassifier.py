import sys

from .baseUnitTest import BaseUnitTest
from ..src.Config import Configs
from ..src.constants.ConfigKeys import ConfigKeys

sys.path.insert(1, Configs[ConfigKeys.SysPath])
import src.py.FixRaidenBoss2 as FRB


class GIMISectionClassifierTest(BaseUnitTest):
    """
    Tests for :class:`FRB.GIMISectionClassifier`, the "classify a `section`_ by what it *does*"
    half of :class:`FRB.GIMIParser`'s mod-object attribution.

    .. note::
        The asset rows are added to fresh ``Hashes``/``Indices`` instances under a deliberately
        absurd version (``99.0``) with made-up hash/index values, so nothing here depends on the
        real, frequently-updated ``HashData``/``IndexData`` content -- only on the lookup
        machinery underneath it.
    """

    _VERSION = "99.0"
    _OLD_VERSION = "50.0"
    _NAME = "testrika"

    _BLEND_HASH = "test-hash-blend-vb"
    _OLD_BLEND_HASH = "test-hash-blend-vb-old"
    _IB_HASH = "test-hash-ib"
    _BODY_INDEX = "test-index-body"
    _HEAD_INDEX = "test-index-head"

    def _makeHashes(self):
        hashes = FRB.Hashes()

        # Hashes' index columns, in order: version, name, type
        hashes.addRepoRows({self._VERSION: {self._NAME: {"blend_vb": self._BLEND_HASH,
                                                        "ib": self._IB_HASH}},
                            self._OLD_VERSION: {self._NAME: {"blend_vb": self._OLD_BLEND_HASH}}})
        return hashes

    def _makeIndices(self):
        indices = FRB.Indices()

        # Indices' index columns, in order: version, name, component, object
        indices.addRepoRows({self._VERSION: {self._NAME: {"": {"body": self._BODY_INDEX,
                                                              "head": self._HEAD_INDEX}}}})
        return indices

    def _makeClassifier(self, hashKeyOnlyToModObj = None, indexKeyToModObj = None):
        if (hashKeyOnlyToModObj is None):
            hashKeyOnlyToModObj = {"blend_vb": ("", "blend")}

        if (indexKeyToModObj is None):
            indexKeyToModObj = {"ib": {("", "body"): ("", "body"),
                                       ("", "head"): ("", "head")}}

        return FRB.GIMISectionClassifier(hashKeyOnlyToModObj, self._makeHashes(), indexKeyToModObj, self._makeIndices())

    def _colouring(self, src):
        return FRB.IfContentPartColouring(src)

    # =========================== construction / properties ===========================

    def test_construct_keepsTheCallersOwnObjects(self):
        hashKeyOnly = {"blend_vb": ("", "blend")}
        indexKey = {"ib": {("", "body"): ("", "body")}}
        hashes = self._makeHashes()
        indices = self._makeIndices()

        classifier = FRB.GIMISectionClassifier(hashKeyOnly, hashes, indexKey, indices, 3.0)

        # Identity, not equality: GIMIParser's own callers build a default classifier and then
        # assign into these dicts afterwards.
        self.assertIs(classifier.hashKeyOnlyToModObj, hashKeyOnly)
        self.assertIs(classifier.indexKeyToModObj, indexKey)
        self.assertIs(classifier.hashes, hashes)
        self.assertIs(classifier.indices, indices)
        self.assertEqual(classifier.version, 3.0)

    def test_construct_onlyRequiredArgs_optionalsDefaultEmptyOrNone(self):
        hashes = self._makeHashes()
        classifier = FRB.GIMISectionClassifier({}, hashes)

        self.compareDict(classifier.hashKeyOnlyToModObj, {})
        self.compareDict(classifier.indexKeyToModObj, {})
        self.assertIsNone(classifier.indices)
        self.assertIsNone(classifier.version)

    # =========================== classify: hash only ===========================

    def test_classify_hashOnly_matchedToItsModObj(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]}))
        self.compareList(result, [("", "blend")])

    def test_classify_unknownHash_noModObjs(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None, self._colouring({"hash": [(0, "not-a-real-hash-anywhere")]}))
        self.compareList(result, [])

    def test_classify_noHashKVP_noModObjs(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None, self._colouring({"match_first_index": [(0, self._BODY_INDEX)]}))
        self.compareList(result, [])

    def test_classify_hashKnownButUnmapped_noModObjs(self):
        # The hash resolves to a real key ("ib"), but nothing maps that key on the hash-only side
        # and there's no match_first_index to disambiguate with.
        classifier = self._makeClassifier(hashKeyOnlyToModObj = {}, indexKeyToModObj = {})
        result = classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._IB_HASH)]}))
        self.compareList(result, [])

    # =========================== classify: hash + match_first_index ===========================

    def test_classify_hashAndIndex_matchedToTheIndexsModObj(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(0, self._IB_HASH)],
                                                      "match_first_index": [(1, self._BODY_INDEX)]}))
        self.compareList(result, [("", "body")])

    def test_classify_hashAndIndex_picksTheRightOneOfSeveral(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(0, self._IB_HASH)],
                                                      "match_first_index": [(1, self._HEAD_INDEX)]}))
        self.compareList(result, [("", "head")])

    def test_classify_indexOutOfTheHashsWindow_isIgnored(self):
        # The match_first_index sits *before* the hash it would otherwise qualify, so it belongs to
        # whatever came earlier in the part -- not to this hash.
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(5, self._IB_HASH)],
                                                      "match_first_index": [(1, self._BODY_INDEX)]}))
        self.compareList(result, [])

    def test_classify_indexHashFallsBackToHashOnlyWhenIndexUnknown(self):
        classifier = self._makeClassifier(hashKeyOnlyToModObj = {"ib": ("", "ibFallback")})
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(0, self._IB_HASH)],
                                                      "match_first_index": [(1, "not-a-real-index")]}))
        self.compareList(result, [("", "ibFallback")])

    def test_classify_severalHashes_bothClassified(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(0, self._BLEND_HASH), (2, self._IB_HASH)],
                                                      "match_first_index": [(3, self._BODY_INDEX)]}))
        self.compareSet(set(result), {("", "blend"), ("", "body")})

    def test_classify_duplicateMatches_deduplicated(self):
        classifier = self._makeClassifier()
        result = classifier.classify("SomeSection", None,
                                     self._colouring({"hash": [(0, self._BLEND_HASH), (1, self._BLEND_HASH)]}))
        self.compareList(result, [("", "blend")])

    # =========================== live re-derivation ===========================

    def test_classify_afterAssigningHashKeyOnlyToModObj_usesTheNewMapping(self):
        classifier = self._makeClassifier()
        classifier.hashKeyOnlyToModObj = {"blend_vb": ("bang", "B")}

        result = classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]}))
        self.compareList(result, [("bang", "B")])

    def test_classify_afterInPlaceMutation_usesTheNewMapping(self):
        classifier = self._makeClassifier(hashKeyOnlyToModObj = {})
        classifier.hashKeyOnlyToModObj["blend_vb"] = ("", "lateBlend")

        result = classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]}))
        self.compareList(result, [("", "lateBlend")])

    def test_classify_pinnedVersion_resolvesThatVersionsOwnRow(self):
        classifier = self._makeClassifier()

        # ``Hashes`` resolves a version by inclusive floor-match, so a 60.0 .ini file sees the 50.0
        # row's hash rather than the 99.0 one's. Both still resolve to the same "blend_vb" key
        # though: getKey is a reverse-index lookup, which a pinned version narrows rather than
        # hides.
        classifier.version = "60.0"
        self.compareList(classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._OLD_BLEND_HASH)]})),
                         [("", "blend")])

        classifier.version = None
        self.compareList(classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]})),
                         [("", "blend")])

    def test_classify_hashNonVersionValsFilter_narrowsTheLookup(self):
        classifier = self._makeClassifier()

        classifier.hashNonVersionVals = {"name": "someoneElse"}
        self.compareList(classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]})), [])

        classifier.hashNonVersionVals = {"name": self._NAME}
        self.compareList(classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]})),
                         [("", "blend")])

    # =========================== __call__ ===========================

    def test_call_matchesClassify(self):
        classifier = self._makeClassifier()
        colouring = self._colouring({"hash": [(0, self._BLEND_HASH)]})

        # The objTargetFuncs calling convention: (parser, sectionName, section, disjoint, part, kvps)
        self.compareList(classifier(None, "SomeSection", None, True, None, colouring),
                         classifier.classify("SomeSection", None, colouring))

    # =========================== buildDefaultClassifier ===========================

    class _FakeModType():
        def __init__(self, hashes, indices):
            self.name = "Bernkastel"
            self.hashes = hashes
            self.indices = indices

    def test_buildDefaultClassifier_defaultMappingsRealAssets(self):
        hashes = self._makeHashes()
        indices = self._makeIndices()
        classifier = FRB.GIMISectionClassifier.buildDefaultClassifier(self._FakeModType(hashes, indices), 3.0)

        # The default mappings are derived from the whole hash/index data tables, so this asserts
        # the convention rather than an exact listing -- adding a character to HashData must not
        # break this test.
        self.assertGreater(len(classifier.hashKeyOnlyToModObj), 0)
        self.assertEqual(classifier.hashKeyOnlyToModObj["blend_vb"], ("", "blend_vb"))
        self.assertEqual(classifier.hashKeyOnlyToModObj["tex_head_diffuse"], ("", "tex_head_diffuse"))
        self.assertNotIn("ib", classifier.hashKeyOnlyToModObj)

        self.compareList(list(classifier.indexKeyToModObj.keys()), ["ib"])
        self.assertEqual(classifier.indexKeyToModObj["ib"][("", "head")], ("", "head"))
        self.assertEqual(classifier.indexKeyToModObj["ib"][("", "body")], ("", "body"))

        self.assertIs(classifier.hashes, hashes)
        self.assertIs(classifier.indices, indices)
        self.assertEqual(classifier.version, 3.0)

    def test_buildDefaultClassifier_mappingsAreThisClassifiersOwn(self):
        first = FRB.GIMISectionClassifier.buildDefaultClassifier(None)
        second = FRB.GIMISectionClassifier.buildDefaultClassifier(None)

        # Each build gets its own dicts -- callers are expected to add their mod type's own
        # entries, and must not write through to every other classifier's.
        first.hashKeyOnlyToModObj["blend_vb"] = ("", "somethingElse")
        self.assertEqual(second.hashKeyOnlyToModObj["blend_vb"], ("", "blend_vb"))

    def test_buildDefaultClassifier_defaultMappingsClassifyARealHash(self):
        # The point of the defaults: a classifier built for a mod type resolves that mod type's own
        # hashes with no further setup.
        classifier = FRB.GIMISectionClassifier.buildDefaultClassifier(self._FakeModType(self._makeHashes(), self._makeIndices()))

        self.compareList(classifier.classify("SomeSection", None, self._colouring({"hash": [(0, self._BLEND_HASH)]})),
                         [("", "blend_vb")])

    def test_buildDefaultClassifier_noModType_noAssets(self):
        classifier = FRB.GIMISectionClassifier.buildDefaultClassifier(None)
        self.assertIsNone(classifier.hashes)
        self.assertIsNone(classifier.indices)

    class _FakeIni():
        def __init__(self, availableType, version):
            self.availableType = availableType
            self.version = version

    def test_buildDefaultClassifierFromIni_takesTypeAndVersionFromTheIni(self):
        hashes = self._makeHashes()
        indices = self._makeIndices()
        ini = self._FakeIni(self._FakeModType(hashes, indices), 4.5)

        classifier = FRB.GIMISectionClassifier.buildDefaultClassifierFromIni(ini)

        self.assertIs(classifier.hashes, hashes)
        self.assertIs(classifier.indices, indices)
        self.assertEqual(classifier.version, 4.5)

    def test_buildDefaultClassifierFromIni_unclassifiedIni_noAssets(self):
        classifier = FRB.GIMISectionClassifier.buildDefaultClassifierFromIni(self._FakeIni(None, None))
        self.assertIsNone(classifier.hashes)
        self.assertIsNone(classifier.indices)
