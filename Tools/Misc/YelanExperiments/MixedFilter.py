"""
The split of this Yelan mod onto YelanTranquil the way issue #190's comment prescribes, one
strategy per component:

    | Component | Strategy       | Why (from the issue)                                                  |
    | Body      | graph cut      | otherwise the unmapped face/hair indices interpolate into a beam      |
    | Eye       | graph cut      | no interpolation to mess up the eyes                                  |
    | Bangs     | negative index | small hair details go missing with the cut                            |

See NegIndexFilter.py and GraphCutFilter.py for the two strategies on their own. This script
writes <out>/<Component>/ (the Body's and Eye's own filtered buffers, the Bang's remapped blend
over the whole mod) and a COMPLETE .ini: the mod's own Yelan sections copied from the mod's .ini
(up to its 4.1 fix, i.e. without that file's experimental YelanTranquil sections), then the
generated YelanTranquil sections.

The graph cut's --mode picks the output:

    py -3 MixedFilter.py                    strict   -> Mixed/    + YelanCopy2.ini
    py -3 MixedFilter.py --mode relaxed     relaxed  -> Relaxed/  + YelanCopy4.ini   (a triangle is kept when
                                                                                       2 of its 3 vertices are the component's)
    py -3 MixedFilter.py --mode majority    majority -> Majority/ + YelanCopy5.ini
    py -3 MixedFilter.py --mode fill        fill     -> Fill/     + YelanCopy6.ini   (the Bang draws every triangle all of whose
                                                                                       corners are live in it; Body / Eye take the rest)
    py -3 MixedFilter.py --base ""          only the YelanTranquil sections
    py -3 MixedFilter.py --variant slotA    fill, Head + Body both through slot A, skin-band lightmaps -> Fill7/ + YelanCopy7.ini
    py -3 MixedFilter.py --variant trim     slotA + the Bang draws only its fully-live triangles       -> Fill8/ + YelanCopy8.ini
    py -3 MixedFilter.py --variant vc       trim + Body vertex colour G/B 188 -> 128 (yelanTexcoordVC.buf)  -> Fill9/ + YelanCopy9.ini
    py -3 MixedFilter.py --variant nofix    trim, Body draws without ORFix (its default layout misplaces the lightmap) -> Fill10/ + YelanCopy10.ini
    py -3 MixedFilter.py --variant nullext  trim + Body draws null ps-t3..ps-t10 (lace-texture hunt)     -> Fill11/ + YelanCopy11.ini
    py -3 MixedFilter.py --variant metal    trim + skin band 255 + shiny band-0 trims -> metal band 77   -> Fill12/ + YelanCopy12.ini
    py -3 MixedFilter.py --variant nm203    metal + the Body bound to the head's alpha-203 normal map      -> Fill13/ + YelanCopy13.ini
    py -3 MixedFilter.py --variant cloth121 skin 255 + ALL of band 0 -> 121 (her navy-fabric band)          -> Fill14/ + YelanCopy14.ini
    py -3 MixedFilter.py --variant cloth177 skin 255 + ALL of band 0 -> 177 (her silk / lace-cape band)      -> Fill15/ + YelanCopy15.ini
    py -3 MixedFilter.py --variant uv1      cloth177 + the second UV set zeroed (yelanTexcoordUV1.buf)     -> Fill16/ + YelanCopy16.ini
    py -3 MixedFilter.py --variant slotC    cloth177, Head + Body through slot C (the no-normal-map shader)  -> Fill17/ + YelanCopy17.ini
    py -3 MixedFilter.py --variant slotCuv1    slot C + the second UV set zeroed                             -> Fill18/ + YelanCopy18.ini
    py -3 MixedFilter.py --variant slotCuv1vc  slot C + second UV zeroed + Body vertex colour G/B -> 128       -> Fill19/ + YelanCopy19.ini
    py -3 MixedFilter.py --variant hair121     Copy19 (NNFix on slot C) + hair on Tranquil's hair band 121       -> Fill20/ + YelanCopy20.ini
    py -3 MixedFilter.py --variant headA0      hair121 + the head diffuse's alpha zeroed (Yelan's is 255 everywhere) -> Fill21/ + YelanCopy21.ini
    py -3 MixedFilter.py --variant hairR42     hair121 + the hair's lightmap R (highlight strength) 151 -> 42   -> Fill22/ + YelanCopy22.ini
    py -3 MixedFilter.py --variant hair128     hair on band 128, her other hair material                     -> Fill23/ + YelanCopy23.ini
    py -3 MixedFilter.py --variant hairDither  hair on a 121/128 per-pixel checkerboard, as her own hair is    -> Fill24/ + YelanCopy24.ini
    py -3 MixedFilter.py --variant hairAlpha255  hair121 + body diffuse hair alpha -> 255 (top/bottom hair tone)  -> Fill25/ + YelanCopy25.ini
    py -3 MixedFilter.py --variant hairB0      hair121 + the hair's lightmap B (highlight MASK) zeroed         -> Fill26/ + YelanCopy26.ini
    py -3 MixedFilter.py --variant hairB50     hair121 + the highlight mask halved                            -> Fill27/ + YelanCopy27.ini
    py -3 MixedFilter.py --variant final       ALL of the above that worked, in one file                      -> Final/ + YelanTranquil.ini
                                             (its textures: py -3 LiftBodyLightMap.py --band 115 127 255 --bandDark 0 0 125 121 --band 0 0 177 --bDark 125 0 --suffix Final
                                                            py -3 LiftBodyLightMap.py --alphaDark 125 255 --src yelanBodyDiffuse.dds --suffix Final
                                              change --bDark's 0 to e.g. 0.25 for a faint highlight band in Tranquil's own light blue)
    py -3 MixedFilter.py --variant hairMatch   final + body hair colour-matched to the head's (MatchHairColour.py) -> Fill28/ + YelanCopy28.ini
    py -3 MixedFilter.py --variant hairHue     hairMatch + every hair rule by HUE (--navy), not darkness          -> Fill29/ + YelanCopy29.ini
                                             (py -3 LiftBodyLightMap.py --navy --band 115 127 255 --bandDark 0 0 125 121 --band 0 0 177 --bDark 125 0 --suffix Final2
                                              py -3 LiftBodyLightMap.py --navy --alphaDark 125 255 --src yelanBodyDiffuseHairMatched.dds --suffix Final2)

The Body's and Head's lightmaps are bound to LiftBodyLightMap.py's yelan{Body,Head}LightMapLifted128.dds when they exist
(--texture Body LightMap <file> to bind another; see that script for why).

The Bang's negative-index remap is augmented from the reverse sheet: Bang 0 is Yelan's head (64),
so hair vertices weighted head + bang keep both bones there (--noAugment turns that off).

Only one .ini drawing YelanTranquil may be enabled at a time: the others override the same hashes,
so give them the DISABLED prefix when the one under test is on.
"""

import os
import sys

# the tool this rides on: Tools/VGRemapFinder in the Anime Game Remap repo
ToolSrc = r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Tools\VGRemapFinder\src"
if (ToolSrc not in sys.path):
    sys.path.insert(0, ToolSrc)

from VGRemapFinder.ComponentSplit import IniLayout, makeArgParser, runSplit   # noqa: E402

Here = os.path.dirname(os.path.abspath(__file__))

# the mod's own .ini, whichever name it currently has
BaseCandidates = ["yelan copy.ini", "DISABLED_yelan copy.ini", "DISABLEDyelan copy.ini"]
Base = next((name for name in BaseCandidates if os.path.isfile(os.path.join(Here, name))), BaseCandidates[0])

Defaults = {"mod": Here,
            "prefix": "yelan",
            "hashJson": r"E:\Computer\Downloads\YelanTranquil\YelanTranquil\hash.json",
            "draft": r"E:\Computer\Games\Genshin\Repos\Repos\Fix-Raiden-Boss\Data\RemapDrafts\YelanRemapDraft.xlsx",
            "sheet": "V5.7 Yelan to Tranquil (tool)",      # same rows as the library's Yelan -> YelanTranquil table
            "fromName": "Yelan",
            "toName": "YelanTranquil",
            "out": "Mixed",
            "ini": "YelanCopy2.ini",
            "base": Base,                                   # the mod's own sections come from here ...
            "baseEnd": "; ----------------------------------",   # ... up to (not including) this line, after the 4.1 fix
            "faceRegister": "ps-t1",                        # GI 6.x face diffuse register (the mod's own .ini still says ps-t0)
            "textures": [("Body", "LightMap", "yelanBodyLightMapLifted128.dds"),    # LiftBodyLightMap.py's outputs; --texture overrides
                         ("Head", "LightMap", "yelanHeadLightMapLifted128.dds")]}

# per --mode, where the output goes when --out / --ini are left at their defaults
ModeOutputs = {"strict": ("Mixed", "YelanCopy2.ini"), "relaxed": ("Relaxed", "YelanCopy4.ini"), "majority": ("Majority", "YelanCopy5.ini"),
               "fill": ("Fill", "YelanCopy6.ini")}

# issue #190's table
Strategies = {"Body": "graphcut", "Eye": "graphcut", "Bang": "negative"}

# What each YelanTranquil component's draws take, from the experiments in 'yelan copy.ini'.
#   Body : ps-t0 normal map, ps-t1 diffuse, ps-t2 light map, ORFix   (slot C of the Body has no normal map)
#   Bang : ps-t1 diffuse, ps-t2 light map, ORFix
#   Eye  : ps-t0 diffuse, ps-t1 light map, NNFix
# The mod objects (Head, Body, ...) are drawn in the component's slots in rank order: Head in the
#   first slot (match_first_index 0), Body in the second, ...; an empty mod object is never drawn.
ORFix = "CommandList\\global\\ORFix\\ORFix"
NNFix = "CommandList\\global\\ORFix\\NNFix"
Layouts = {"Body": IniLayout({"A": [("ps-t0", "NormalMap"), ("ps-t1", "Diffuse"), ("ps-t2", "LightMap")],
                              "B": [("ps-t0", "NormalMap"), ("ps-t1", "Diffuse"), ("ps-t2", "LightMap")],
                              "C": [("ps-t0", "Diffuse"), ("ps-t1", "LightMap")]}, ORFix),
           "Bang": IniLayout([("ps-t1", "Diffuse"), ("ps-t2", "LightMap")], ORFix),
           "Eye": IniLayout([("ps-t0", "Diffuse"), ("ps-t1", "LightMap")], NNFix)}


# --variant slotA: fill mode, the mod's Head AND Body drawn through YelanTranquil's slot A (its ramp set has
#   the skin band; slot B, the dress, has none -- the arms/legs vs neck shading seam), with the lightmaps'
#   skin band moved 115..127 -> 255 and nothing else touched (the 128 lift also put alpha 0 onto Tranquil's
#   sheer-lace band 128, which rendered as static on the neck and earrings)
# --variant trim: slotA plus the Bang drawing ONLY its fully-live triangles (a sentinel corner lands near the
#   origin, so a half-live triangle is a sliver from the bangs down through the neck and shoulders -- static in
#   a side view). This is the generator's default now; slotA keeps the old untrimmed draw for comparison.
# --variant vc: trim plus the vertex-colour experiment -- EditVertexColour.py's yelanTexcoordVC.buf, the Body
#   object's COLOR G/B 188 -> 128 as on both game models.
SkinTextures = [("Body", "LightMap", "yelanBodyLightMapSkin255.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255.dds")]
Variants = {"slotA": {"mode": "fill", "out": "Fill7", "ini": "YelanCopy7.ini", "objectSlots": ["Head=A", "Body=A"], "textures": SkinTextures, "keepAllTriangles": True},
            "trim": {"mode": "fill", "out": "Fill8", "ini": "YelanCopy8.ini", "objectSlots": ["Head=A", "Body=A"], "textures": SkinTextures},
            "vc": {"mode": "fill", "out": "Fill9", "ini": "YelanCopy9.ini", "objectSlots": ["Head=A", "Body=A"], "textures": SkinTextures, "texcoordFile": "yelanTexcoordVC.buf"},
            # trim, but the Body component's draws do NOT run ORFix: YelanTranquil's vertex shader is not in ORFix's
            #   list, so ORFix falls through to its LDX layout and moves the lightmap into ps-t0 -- the slot her dump
            #   says holds the normal map. Her dump's order (NormalMap, Diffuse, LightMap) is bound as-is instead.
            "nofix": {"mode": "fill", "out": "Fill10", "ini": "YelanCopy10.ini", "objectSlots": ["Head=A", "Body=A"], "textures": SkinTextures,
                      "layouts": {"Body": ("keepRegisters", None, [])}},
            # trim, plus the game's remaining texture registers of the Body draws nulled, to find which one carries
            #   the lace pattern that appears on the neck, earrings and shoulder (a game texture sampled with the mod's UVs)
            # the frame dumps' verdict: ORFix knows every one of Tranquil's draw shaders (main pass = lightmap, normal map,
            #   diffuse in ps-t0..2, which is what NDL + ORFix produces), so the leftovers are MATERIAL data. The 'static'
            #   on the neck / earrings / shoulder is the mod's grey-white trims: band 0 with lightmap R = 255, a combination
            #   Tranquil's slot A never uses (her band 0 is matte fur / fabric with R ~ 0; R = 255 is her metal band 64-89).
            # --variant metal: trim + skin band -> 255 AND band 0 with R >= 200 -> 77 (metal)
            "metal": {"mode": "fill", "out": "Fill12", "ini": "YelanCopy12.ini", "objectSlots": ["Head=A", "Body=A"],
                      "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Metal.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Metal.dds")]},
            # --variant nm203: metal + the Body drawn with the HEAD's normal map (flat, alpha 203, exactly what Tranquil's
            #   slot A normal map is everywhere) instead of the body's flat alpha-255 one -- the one remaining per-part
            #   difference between the face and the rest of the skin
            "nm203": {"mode": "fill", "out": "Fill13", "ini": "YelanCopy13.ini", "objectSlots": ["Head=A", "Body=A"],
                      "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Metal.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Metal.dds"),
                                   ("Body", "NormalMap", "YelanHeadNormalMap.dds")]},
            # The band legend, read off both skins' lightmaps: the mod leaves EVERYTHING that is not skin (115-127) or metal
            #   (64-89) in band 0 -- dress, hair, leaf print, sheer collar, earrings. On Yelan band 0 is default cloth/hair; on
            #   Tranquil's slot A it is her white FUR (177k of her 243k band-0 pixels), whose speckled sheen is the 'static'
            #   and whose brightness is the dress looking unlike the skin. So band 0 goes to one of her plain-cloth bands:
            # --variant cloth121: band 0 -> 121 (her navy fabric band), skin -> 255
            # --variant cloth177: band 0 -> 177 (her silk / lace-cape band), skin -> 255
            "cloth121": {"mode": "fill", "out": "Fill14", "ini": "YelanCopy14.ini", "objectSlots": ["Head=A", "Body=A"],
                         "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth121.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth121.dds")]},
            "cloth177": {"mode": "fill", "out": "Fill15", "ini": "YelanCopy15.ini", "objectSlots": ["Head=A", "Body=A"],
                         "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth177.dds")]},
            # --variant uv1: cloth177 + the mod's second UV set (TEXCOORD1) zeroed. Yelan's model has a second UV on every vertex,
            #   the mod copies that, and YelanTranquil's shader only carries one on 3010 of 25954 body vertices (her sheer
            #   panels) -- so her shader may be sampling something by a UV the mod never meant for it
            "uv1": {"mode": "fill", "out": "Fill16", "ini": "YelanCopy16.ini", "objectSlots": ["Head=A", "Body=A"], "texcoordFile": "yelanTexcoordUV1.buf",
                    "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth177.dds")]},
            # --variant slotC: every data input has now been changed without effect; the one constant was the PIXEL SHADER.
            #   Slot A's shader is the normal-map variant (vs 2c157719 / ps 6546504e); Yelan's own body uses the no-normal-map
            #   variant, and so does Tranquil's slot C (vs d4c01363 / ps 93dcb43f, layout lightmap + diffuse via ORFix's LDX).
            #   Head + Body drawn through slot C, no normal map at all
            "slotC": {"mode": "fill", "out": "Fill17", "ini": "YelanCopy17.ini", "objectSlots": ["Head=C", "Body=C"],
                      "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth177.dds")]},
            # slot C cleared the static (Copy17). Slot C is Tranquil's SHEER-PANEL slot: 3010 of its 4638 vertices carry a second
            #   UV (slots A and B carry none), so its shader reads TEXCOORD1 -- and the mod feeds it Yelan's second UV everywhere.
            #   The remaining per-region tone difference (upper back paler than arms) has no texture-side cause: arm and back skin
            #   sample identical diffuse / lightmap / band values, so it must be per-vertex.
            # --variant slotCuv1: slot C + second UV zeroed
            # --variant slotCuv1vc: slot C + second UV zeroed + Body vertex colour G/B 188 -> 128
            "slotCuv1": {"mode": "fill", "out": "Fill18", "ini": "YelanCopy18.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1.buf",
                         "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth177.dds")]},
            "slotCuv1vc": {"mode": "fill", "out": "Fill19", "ini": "YelanCopy19.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                           "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Cloth177.dds")]},
            # Copy18/19 solved both problems in game (with NNFix on the slot C draws, the maintainer's edit, folded in here).
            # --variant hair121: Copy19 + the hair on Tranquil's HAIR band. Her hair is bands 115-127/128 (270k navy pixels of
            #   slot A), Yelan's is band 0; sending all of band 0 to 177 (silk) gave the hair silk's light-blue highlight.
            #   Band 0 where the diffuse is dark -> 121, the rest of band 0 (dress, print, collar) stays 177.
            "hair121": {"mode": "fill", "out": "Fill20", "ini": "YelanCopy20.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                        "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177.dds")],
                        "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # --variant headA0: hair121 + the HEAD diffuse with its alpha zeroed. The mod's head texture is Yelan's own head
            #   texture, alpha 255 on every pixel; Tranquil's diffuse alpha is 0 almost everywhere. The crown of the hair, where
            #   the highlight band sits, is the Head object and samples that texture; if slot C's shader reads diffuse alpha as
            #   emission / highlight strength, the painted light streak glows light blue -- the band still there in Copy20
            "headA0": {"mode": "fill", "out": "Fill21", "ini": "YelanCopy21.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                       "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177.dds"),
                                    ("Head", "Diffuse", "yelanHeadDiffuseAlpha0.dds")],
                       "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # --variant hairR42: hair121 + the hair's lightmap R (specular strength) lowered from Yelan's 151 to 42. The mod's head
            #   textures are byte-identical to Yelan's own, and her hair R (151) equals Tranquil's (152): the highlight's COLOUR
            #   is the target skin's material constant (Tranquil's hair highlight is light blue by design), so only its strength
            #   is ours to change
            "hairR42": {"mode": "fill", "out": "Fill22", "ini": "YelanCopy22.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                        "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177R42.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177R42.dds")],
                        "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # Copy21 (head diffuse alpha 0) lightened ALL the hair (alpha darkens in this shader) and kept the band; Copy22 (R 42)
            #   kept it too: the band is the shader's own highlight, not a painted streak (the mod's hair texture IS Yelan's, and
            #   its streaks are no lighter than Tranquil's own). Her hair is a per-pixel CHECKERBOARD of bands 121 and 128 --
            #   two hair materials, maybe highlight on / off at 50% density. The mod's hair is all 121.
            # --variant hair128: hair -> 128 only;  --variant hairDither: hair -> 121/128 checkerboard, as hers is
            "hair128": {"mode": "fill", "out": "Fill23", "ini": "YelanCopy23.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                        "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair128Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair128Cloth177.dds")],
                        "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            "hairDither": {"mode": "fill", "out": "Fill24", "ini": "YelanCopy24.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                           "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255HairDitherCloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255HairDitherCloth177.dds")],
                           "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # The crown of the hair is the Head object, drawn from Yelan's head texture (diffuse alpha 255 everywhere); the back
            #   hair is the Body object, drawn from the body texture (alpha ~0 on the hair). Copy21 showed this shader darkens with
            #   alpha, so the two halves of the hair shade differently -- the top/bottom colour difference. Yelan's shader ignores it.
            # --variant hairAlpha255: hair121 + the BODY diffuse's hair pixels (dark, max <= 125) set to alpha 255, matching the head's
            "hairAlpha255": {"mode": "fill", "out": "Fill25", "ini": "YelanCopy25.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                             "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177.dds"),
                                          ("Body", "Diffuse", "yelanBodyDiffuseAlphaDark255.dds")],
                             "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # Bands 121 / 128 / checkerboard, R, alpha: none touched the band. The head lightmap's BLUE channel holds the painted
            #   zigzag highlight MASK (visible as white marks on black); the shader colours it with the skin's hair constant --
            #   light blue on Tranquil, grey-blue on Yelan. The colour is not ours; the mask is.
            # --variant hairB0: hair121 + the hair's lightmap B zeroed (mask off);  --variant hairB50: mask halved
            "hairB0": {"mode": "fill", "out": "Fill26", "ini": "YelanCopy26.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                       "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177B0.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177B0.dds")],
                       "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            "hairB50": {"mode": "fill", "out": "Fill27", "ini": "YelanCopy27.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                        "textures": [("Body", "LightMap", "yelanBodyLightMapSkin255Hair121Cloth177B50.dds"), ("Head", "LightMap", "yelanHeadLightMapSkin255Hair121Cloth177B50.dds")],
                        "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # ---- THE RESULT (2026-09-12, confirmed in game step by step): everything below together ----
            #   fill split; Head + Body through slot C (the no-normal-map shader, like Yelan's own) with NNFix; the Bang
            #   negative-index with ORFix; the Eye cut with NNFix; second UV zeroed and vertex colour G/B 128 (yelanTexcoordUV1VC.buf);
            #   lightmaps: skin 115-127 -> 255, hair (dark diffuse) band 0 -> 121, the rest of band 0 -> 177, the hair's painted
            #   highlight mask (lightmap B) scaled by --hairMask (0 = none, Copy26; 0.5 still reads light blue, Copy27);
            #   body diffuse alpha 255 on the hair so the crown and the back hair shade alike (Copy25).
            "final": {"mode": "fill", "out": "Final", "ini": "YelanTranquil.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                      "textures": [("Body", "LightMap", "yelanBodyLightMapFinal.dds"), ("Head", "LightMap", "yelanHeadLightMapFinal.dds"),
                                   ("Body", "Diffuse", "yelanBodyDiffuseFinal.dds")],
                      "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # --variant hairMatch: final + the body diffuse's hair colour-matched to the head diffuse's (MatchHairColour.py): the
            #   crown is Yelan's own head texture, the lower hair is the mod's body texture painted lighter and bluer, and
            #   Tranquil's hair material shows the seam Yelan's shader flattened
            "hairMatch": {"mode": "fill", "out": "Fill28", "ini": "YelanCopy28.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                          "textures": [("Body", "LightMap", "yelanBodyLightMapFinal.dds"), ("Head", "LightMap", "yelanHeadLightMapFinal.dds"),
                                       ("Body", "Diffuse", "yelanBodyDiffuseHairMatched.dds")],
                          "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            # --variant hairHue: the seam in the hair was MY band rule -- hair picked by darkness (max <= 125) sent only the dark
            #   half of the mod's lighter-painted lower hair to 121 and left 66433 lighter strand pixels on 177 (silk), a
            #   different material from the crown. Hair is now picked by HUE (--navy) for every hair rule: band 121, mask off,
            #   alpha 255, on the colour-matched body diffuse
            "hairHue": {"mode": "fill", "out": "Fill29", "ini": "YelanCopy29.ini", "objectSlots": ["Head=C", "Body=C"], "texcoordFile": "yelanTexcoordUV1VC.buf",
                        "textures": [("Body", "LightMap", "yelanBodyLightMapFinal2.dds"), ("Head", "LightMap", "yelanHeadLightMapFinal2.dds"),
                                     ("Body", "Diffuse", "yelanBodyDiffuseHairMatchedFinal2.dds")],
                        "layouts": {"Body": ("keepRegisters", NNFix, [])}},
            "nullext": {"mode": "fill", "out": "Fill11", "ini": "YelanCopy11.ini", "objectSlots": ["Head=A", "Body=A"], "textures": SkinTextures,
                        "layouts": {"Body": ("keepRegisters", ORFix, [f"ps-t{i} = null" for i in range(3, 11)])}}}


if (__name__ == "__main__"):
    parser = makeArgParser(__doc__, "graphcut", Defaults)
    parser.add_argument("--variant", choices = sorted(Variants), default = None, help = "a preset combination (slotA: see the comment above Variants)")
    args = parser.parse_args()
    if (args.variant):
        preset = Variants[args.variant]
        if (args.out == Defaults["out"] and args.ini == Defaults["ini"]):
            args.out, args.ini = preset["out"], preset["ini"]
        args.mode = preset["mode"]
        args.objectSlots = args.objectSlots or preset["objectSlots"]
        Defaults["textures"] = preset["textures"]
        args.keepAllTriangles = args.keepAllTriangles or preset.get("keepAllTriangles", False)
        args.texcoordFile = args.texcoordFile or preset.get("texcoordFile")
        for component, (registers, fix, extra) in preset.get("layouts", {}).items():
            Layouts[component] = IniLayout(Layouts[component].registers if (registers == "keepRegisters") else registers, fix, extra)
    elif (args.out == Defaults["out"] and args.ini == Defaults["ini"]):
        args.out, args.ini = ModeOutputs[args.mode]
    args.defaultTextures = [tuple(item) for item in Defaults["textures"] if os.path.isfile(os.path.join(Here, item[2]))]
    missing = [item for item in Defaults["textures"] if not os.path.isfile(os.path.join(Here, item[2]))]
    for item in missing:
        print(f"(edited texture {item[2]} not found -- run LiftBodyLightMap.py; binding the mod's own {item[0]} {item[1]} instead)")
    runSplit(Strategies, args, Layouts, Defaults["fromName"], mode = args.mode)
