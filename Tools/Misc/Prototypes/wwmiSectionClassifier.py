# WWMISectionClassifier: the GIMISectionClassifier of a WWMI mod -- a convenience for hash / index classification.
#
# A WWMI mod draws its character as components of one mesh (`hash = <vb0>` + `match_first_index`) and binds most of its
# textures by HASH (`[TextureOverride...] hash = <a texture> / this = Resource...`). This sorts a mod's sections into:
#
#   ("", <component>)       a component's sections, by the vb0 hash and match_first_index (the source's index table)
#   ("", <object>)          a hash-only object (the bone data, the shape keys), by its hash
#   (Texture, <role>)       a texture override of one of a role's hashes -- every hash the role has had
#   (Texture, "")           any other texture override (not a previous fix's)
#   ("", "watch")           a section that only WATCHES the source's vb0 (no match_first_index): a help overlay, a key
#                           menu, a skill effect's recolour setting a variable while she is on screen
#
# and nothing else: what a fix READS off those graphs is the parser's own list of edits.
#
# PROTOTYPE of an API class: it is ported to C++ (with its binding) once its shape is settled.
import src.py.FixRaidenBoss2 as FRB

K = FRB.IniKeywords
TEXTURE = FRB.IniGraphModObjKeywords.Texture
WATCH = ("", "watch")


class WWMISectionClassifier(FRB.GIMISectionClassifier):
    """
    Parameters
    ----------
    modType: the source's ModType -- its hash and index tables
    version: the game version its mods are read at
    components: the source's components, eg. ["component0", ...]
    roles: the texture roles to classify overrides into, eg. ["bangsDiffuse", ...]
    hashOnly: the objects identified by a hash alone, eg. {"cb4": "boneData"}
    remapNames: the names a section of a previous fix carries (the source's, and the targets')
    """

    def __init__(self, modType, version, components, roles, hashOnly, remapNames):
        # the C++ base first: an attribute set on 'self' before it is built is undefined behaviour (it crashed 2 runs in 3)
        name, roles = modType.name, set(roles)
        hashTypes = {hashType for names in modType.hashes.repo.toNestedDict().values() for hashType in names.get(name, {})}
        textureObjs = {hashType: (TEXTURE, FRB.Hashes.generationOf(hashType)[0]) for hashType in hashTypes
                       if FRB.Hashes.generationOf(hashType)[0] in roles}
        super().__init__({**textureObjs, **{hashType: ("", obj) for hashType, obj in hashOnly.items()}}, modType.hashes,
                         {"vb0": {("", c): ("", c) for c in components}}, modType.indices, version, [name, None], [name, None, None])
        self.remapNames = list(remapNames)
        self.vb0s = {value.lower() for names in modType.hashes.repo.toNestedDict().values() for hashType, value in names.get(name, {}).items()
                     if hashType == "vb0"}
        # the objects it sorts into, for the parser's modObjs
        self.objects = list(dict.fromkeys([("", c) for c in components] + [("", obj) for obj in hashOnly.values()]
                                          + list(textureObjs.values()) + [(TEXTURE, ""), WATCH]))

    def __call__(self, parser, name, section, disjoint, part, kvps):
        """The base classification; and a texture override of a hash the source's tables do not know is (Texture, "")"""
        if kvps is None:
            return []
        hashes = [value.strip().lower() for value in part.getVals(K.Hash)]
        if (hashes and hashes[0] in self.vb0s and section.firstVal(K.MatchFirstIndex) is None
                and not FRB.IniNamingTools.looksRemapped(name, self.remapNames)):
            return [WATCH]
        found = super().__call__(parser, name, section, disjoint, part, kvps)
        if (not found and name.startswith(K.TextureOverride) and part.getVals(K.Hash) and section.firstVal(K.MatchFirstIndex) is None
                and not FRB.IniNamingTools.looksRemapped(name, self.remapNames)):
            found = [(TEXTURE, "")]
        return found
