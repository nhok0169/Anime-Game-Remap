# Misc

Scripts the `AI Agent Help/` guides refer to that were written next to the mods they operate on,
on the maintainer's machine (`E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI\...`), copied
here so an agent on another machine can read and run them. **The copies under `Importer/GIMI/`
are the live ones the maintainer runs; when the two differ, the live one is newer.** Everything
here runs on Windows or Linux / WSL (a Windows-form path argument is translated to `/mnt/<drive>/...`
on Linux), and finds the API through `AG_REMAP_REPO` or the repo's own layout.

| Folder | What | Guide |
| --- | --- | --- |
| `Prototypes/overrideScript.py` | the **config route** prototype (`GIMICharParserConfig` / `GIMICharFixerConfig` handed to the shipped factories); `--ab` proves it byte-identical to the compiled fix | [Creating Remaps](../../AI%20Agent%20Help/CreatingRemaps/CLAUDE.md) |
| `Prototypes/overrideScript2.py` | the **hand-built route** prototype (a `GIMIParser` / `GIMIFixer` from the individual edits) | same |
| `Prototypes/overrideVgRemap.py` | a vertex-group remap at runtime: `--dump` prints a pair's `unmapped source groups`, `--ab` diffs the `RemapBlend.buf` bytes with and without a change | [Vertex Group Remaps](../../AI%20Agent%20Help/VGRemaps/CLAUDE.md) |
| `Prototypes/yelanTranquilFix.py` | **the Yelan -> YelanTranquil fix, whole**, through the API's parser / fixer / `RemapService`: a runtime `ModType`, one fixer per target component, the buffers as one resource group (`ResGroupCollect` + `BufReplace` + `VGSplitGroupResource`), the textures band-mapped and written with mip chains. Runs from WSL (`--wsl` from Windows). The thing the compiled fixer (`makeGIMIComponentFixer`, 2026-09-13) was transcribed from and is A/B'd against | VGRemaps recipe step 8; Creating Remaps' "Yelan is COMPILED now" |
| `Prototypes/yelanTranquilFixPerBuffer.py` | its earlier shape: one `ResRegCollect` + `fixFunc` per buffer (runs on an API without the split classes) | |
| `Prototypes/yelanTranquilFixStandalone.py` | its first shape, with no API at all | |
| `Prototypes/identityMod.py` | **the identity mod**: a character's own model as a GIMI mod out of `GI-Model-Importer-Assets/PlayerCharacterData/<Name>` -- the first mod to test any remap on, since every bone and every material band of the real skin is in it | VGRemaps recipe step 8 |
| `Diagnostics/modTally.py` | per object of a mod: geometry, vertex groups, lightmap band at its own vertices with the diffuse colour under each band; `--against` another mod, `--remap` against the shipped row, `--centroids` | Creating Remaps' "The Yelan lessons" |
| `Diagnostics/boneCentroids.py` | every vertex group's centroid / weight / dominated vertices of a character, from a frame analysis (`--hashes`), a dump folder or a mod folder -- for choosing an anchor for a part the target lacks | same |
| `YelanExperiments/` | the hand-experiment generators from the china-dress test mod (`NegIndexFilter.py`, `GraphCutFilter.py`, `MixedFilter.py` on top of `Tools/VGRemapFinder`'s `ComponentSplit.py`; `LiftBodyLightMap.py`, `EditVertexColour.py`, `MatchHairColour.py`), `YelanTranquil_hash.json` they read, and the maintainer's **hand-made reference pair** `YelanHandMade.ini` / `YelanCopyHandMade.ini` (the double-file shape done by hand, the A/B reference that found the vertex-limit and texture omissions) | VGRemaps "A character of SEVERAL components" |
| `Linux/linuxBuild.sh` | rebuild the core module under WSL from the native build tree and drop the `.so` into the package folder, printing before / after mtimes and the exit codes | [Building](../../AI%20Agent%20Help/Building/CLAUDE.md) |
| `Linux/buildTests.sh <Test> ...` | compile and run named `core/tests/*.cpp` standalone tests on Linux against the native build tree | [Testing](../../AI%20Agent%20Help/Testing/CLAUDE.md) |
| `Diagnostics/runCompiled.py <mod folder>` | fix a folder through the COMPILED tables alone and print every stats bucket -- the compiled half of an A/B against a prototype (fix a scratch copy with each, diff the folders) | Creating Remaps' "Yelan is COMPILED now" |

Nothing tests this folder; run a script before you change it (see
[Tools](../../AI%20Agent%20Help/Tools/CLAUDE.md)). The `YelanExperiments/` scripts carry absolute
paths from the maintainer's machine near their top -- they are reference, not tooling.
