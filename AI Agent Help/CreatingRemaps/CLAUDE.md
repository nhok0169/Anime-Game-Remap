# Creating Remaps

How to add or fix the remap for one character: the `IniParser` that finds the mod's parts, the
`IniFixer` that rewrites them, and the loop that proves the result actually works in game.

Read [Architecture](../Architecture/CLAUDE.md) first if you have never touched
`model/strategies/` — this file assumes the strategy-context seam and the
`GIMIParser`/`GIMIFixer` split, and does not re-explain them.

<br>

## The one thing to internalise: **almost every failure here is silent**

A remap can be wrong in five different ways and *every observable signal still says it worked* —
the section names look right, the `.buf` file is on disk, the summary prints a success line, no
exception is raised. That is not bad luck; it is the shape of the domain. The `.ini` file is data,
the game is the only interpreter, and this software has no way to ask it anything.

Concretely, all five of these happened while building Raiden's 6.1 remap, and none of them raised:

- the fix generated **no remapped sections at all** and hid the originals on top of it, so the mod
  rendered nothing
- the `Blend.buf` was written as an **unremapped copy**, which explodes the mesh in game
  (`Images/Raiden/6_1/RaidenBlendBroken.jpg`) while looking perfect on disk
- the resource was built correctly and **counted nowhere**, so the summary said `0 Blend.buf files`
- `--undo` computed a perfect removal and **threw it away**, while printing
  `Removed fix from up to 3 .ini files`
- the fix reflowed **every line of every `.ini` file** from CRLF to LF

So: **never conclude a remap works because a run was clean.** The two things that actually decide it
are the A/B diff against the old script (below) and a screenshot from the game.

<br>

## Where the data lives — **not** in the `.py.txt` files

`data/IniParseBuilderData.py.txt` and `data/IniFixBuilderData.py.txt` are **reference only**. They
are the pre-migration pure-Python tables, kept so you can read what a character's fix used to do.
They are not loaded, and porting them by editing Python will not work: `CppIniParseBuilderArgs` is
bound opaque (*"there is no way to build one from Python yet"* — only the fixed-factory flavour is
constructible), so the version-keyed tables cannot be fed from `ModData.IniParseBuilderArgs` at all.

The live tables are C++:

| | |
| --- | --- |
| Parse table | `api/src/cpp/core/src/data/IniParseBuilderData.cpp` |
| Fix table | `api/src/cpp/core/src/data/IniFixBuilderData.cpp` |
| One character's parser | `core/{include/AGRemapCore,src}/data/IniParseData/<Name>Parser.*` |
| One character's fixer | `core/{include/AGRemapCore,src}/data/IniFixData/<Name>Fixer.*` |

Every generator in those two tables is still a stub returning `defaultFactory()` **except Raiden's
6.1 pair**, which is the worked example to copy. A real generator is roughly an order of magnitude
larger than the stub it replaces, which is why each character gets its own file rather than living
in the table's own translation unit. Keep the `Ini*BuilderFuncs::<name><version>` declaration in the
table header (the table refers to it by that name) and put only the *definition* in the new file;
expose it as `<Name>Parser::vX_Y()` / `<Name>Fixer::vX_Y()` for discoverability. **A new `.cpp` needs
a `core/CMakeLists.txt` entry** — the source list is explicit, not a glob.

Adding a row breaks `core/tests/BuilderData_test.cpp`'s hardcoded row/version counts. Nothing builds
`core/tests/*.cpp`, so it fails silently — see [Testing](../Testing/CLAUDE.md).

<br>

## Before writing anything: get the hashes and indices

A remap cannot work without them, and they are the one input this repo does not derive.

They come from the **GI-Model-Importer-Assets** checkout (ask the user for its path;
`PlayerCharacterData/<Character>/hash.json` is the file). Populate
`core/src/data/HashData.cpp` and `core/src/data/IndexData.cpp`.

**`hash.json` is a moving snapshot, not a per-version record.** Its `draw_vb`/`ib` are whatever the
*latest* game version uses, while `HashData.cpp` is keyed by version. Do not file everything under
the character's base version and hope. **Use that repo's own git history** to see what each bump
actually changed:

```bash
git log --oneline -- "PlayerCharacterData/<Character>/hash.json"
git diff <older> <newer> -- "PlayerCharacterData/<Character>/hash.json"
```

For Raiden this showed the 4.1 commit changed *only* `draw_vb` and the 4.3 commit *only* `ib`, with
`texture_hashes` and `object_indexes` untouched across the entire history — so those belong at the
4.0 baseline, and only two rows belong at the later versions. Guessing would have mis-filed 20 rows.

`object_indexes` maps positionally onto `object_classifications`. Cross-check against the
`<Character><Obj>-ib=*.txt` dumps' `first index:` headers, which are independent of `hash.json`.

<br>

## The parser

One `GIMIParser` per (character, version). Its whole job is to say **which sections belong to which
mod object**, using one `GIMISectionClassifier` that works two ways at once:

- **`hashKeyOnlyToModObj`** — for objects a hash names outright. `blend` is the usual case: its
  `blend_vb` hash identifies it and nothing else.
- **`indexKeyToModObj`** — for objects that *share* a hash and need a `match_first_index` to tell
  them apart. `head`/`body`/`dress` all draw from one `ib`, so they go here, keyed by the last two
  index columns of their own `Indices` row — `(component, object)`, which *is* the mod object.

**Set `disjointModObjs = false`.** 3dmigoto reads only the first `hash`/`match_first_index` pair in
a section and applies it to every branch, ignoring its own `if`/`else` grammar — so a section that
spells out a different pair per branch does not actually cycle mod objects the way its author
intended. Attributing such a section to every object it names is what a reader expects, and costs
nothing in practice because nobody writes that structure (anyone who tries hits the same 3dmigoto
bug and stops).

A parser subclass has to **own its `IniFileParseContext` and its classifier** — `GIMIParser` holds a
bare `Context*` and stores obj-target funcs as `std::function`s, and a `Factory` hands back only the
parser. Declare the context before the constructor body so `setCtx` can take its address.

**On `("", "other")`** — a catch-all mod object for sections carrying one of the character's hashes
that no other mod object claims (Position/Texcoord/IB/VertexLimitRaise and friends). Add it *only
when those sections genuinely need their hash or index remapped.* Raiden does not: the 6.1 change
was the NNFix handling, the fix historically only touched the blend, and an `("", "other")` there
emitted sections byte-identical to the originals — pure `.ini` bloat. Other characters do need it.
Ask rather than assume.

<br>

## The fixer

One `GIMIFixer` subclass per (character, version), owning **everything it hands over by pointer**:
its `IniFileFixContext`, every edit, and every group edit. Declaration order is load-bearing twice
over — the context before the constructor body, and every edit before whatever points at it.

A fix is built out of these pieces:

| Piece | What it does |
| --- | --- |
| `GraphRename` | **Turns the copy into the remap.** Without it nothing is generated at all |
| `RegAssetRemap` | Rewrites `hash`/`match_first_index` onto the target mod's equivalents |
| `ResRegCollect` + `VGRemapBlendReplace` | Collects `vb1` and builds the remapped `Blend.buf` |
| `RegRemove` / `RegDelimitedAdd` | The per-character register surgery (eg. NNFix placement) |
| `hiddenModObjs` | Comments out originals the fix *replaces* rather than adds beside |

### Two shapes of remap, and almost everything follows from which one you have

Raiden and Amber are the two worked examples, and they differ in one fact that then decides four
separate design questions. **Ask it first:**

> Does the remap keep the source's `hash` and `match_first_index`, or replace them?

- **Keeps them** (Raiden -> RaidenBoss). The target shares the source's geometry and only the blend
  weights differ, so the original and the remapped section trigger on *the same draw*.
- **Replaces them** (Amber -> AmberCN, and every CN-skin character). The target is a genuinely
  different model with its own hashes.

| | Keeps hash/index | Replaces hash/index |
| --- | --- | --- |
| `hiddenModObjs` | **Required.** Both sections fire on the same draw; hiding the original is what stops the double draw | **Must not.** They can never both fire, and hiding the originals breaks the mod on the character it was built for |
| `RegAssetRemap` | Usually just the blend's `hash` | The `hash` of every object -- but see below, **never** the index |
| `match_first_index` | Left alone | Rewritten per mod object, with a **forward** lookup |
| `("", "other")` | Skip it -- an empty remap is wasted `.ini` space | Usually needed; those sections have hashes too |

### `RegAssetRemap` is right for a hash and wrong for an index

`ModMappedAssets::replace` is *reverse-then-forward*: look the old value up to find which row owns
it, then forward that row's key onto the target.

- For a **hash** the reverse step is the point. The value is what tells you which *kind* of hash it
  is (`ib` / `blend_vb` / `position_vb`), which a mod object cannot tell you, and hashes are unique
  so the lookup is unambiguous.
- For an **index** it is both ambiguous and unnecessary. `0` is *every* character's head index, so
  the reverse lookup fails and writes the literal `IndexNotFound` into a numeric field. And an
  index's meaning **is** its mod object, which the fixer already knows.

So do the index with a forward lookup and `RegNewVals`, per mod object:

```cpp
std::optional<std::string> target = indices->get({toModName_, modObj.first, modObj.second}, toVersion, false);
if (!target.has_value()) { continue; }   // no row on the target is a real answer -- leave it alone
```

`RegNewVals`' `addNewKVPs = false` default is what stops a section with no `match_first_index` of
its own from sprouting one.

### One `GraphRename` per resource kind

`IniNamingTools` has `getRemapFixName`, `getRemapBlendName`, `getRemapPositionName`,
`getRemapTexcoordName` and `getRemapIbName`. Using the generic `getRemapFixName` everywhere compiles,
runs, reports success and produces `…AmberIBAmberCNRemapFix` where every other tool expects
`…AmberAmberCNRemapIB`. `("", "other")` is the one object that genuinely wants the generic name.

### `("", "other")` is many hash types to ONE mod object

It is not a catch-all in the classifier -- there is no fallback. It is several entries in
`hashKeyOnlyToModObj` pointing at the same `ModObj`:

```cpp
{DrawHashKey,        otherObj},   // draw_vb          -- VertexLimitRaise
{FaceDiffuseHashKey, otherObj},   // tex_face_diffuse -- the face's diffuse override
```

**Deduplicate when building `modObjs` from that list**, or the parser is handed the same graph twice.
These sections are a hash swap with no geometry, no index and no resource behind them, which is why
one shared graph is enough.

### A trailing `NNFix` after the last `drawindexed` is CORRECT -- do not "fix" it

`RegDelimitedAdd` adds its `KVP` before every delimiter **and once at the end of every path-terminal
part**, including when the delimiter is that part's last `KVP` and the stretch after it is empty. So
a section that ends in `drawindexed` renders as:

```ini
run = CommandList\global\ORFix\NNFix
drawindexed = auto
run = CommandList\global\ORFix\NNFix
```

The old pure-Python script emits only the first, so an A/B **will** flag this as a divergence. It is
not one. The behaviour is specified, it is pinned by nine tests in
`Testing/Unit Tester/UnitTester/Tests/test_RegDelimitedAdd.py` -- read
`test_edit_severalDelimitersInOnePart_beforeEachAndOnceAfterTheLast` and
`test_edit_branchEndingRightAfterItsDraw_trailingInsertionInsideTheBranch` before touching it -- and
**the maintainer confirmed in game (2026-09-06) that the extra call is harmless.** An `NNFix` after
the final draw of a path is a no-op for that draw.

Raiden never shows it because her sections carry no `drawindexed` at all: the draw lives inside the
`CommandList` her `TextureOverride` runs into, so the root part is one draw-free segment and gets
exactly one. Amber shows it because `RegFillMissing` appends the `drawindexed` to the very end.

The header's "never twice in a row" line means *two insertions with nothing between them*, not an
insertion following a delimiter. Reading it the other way costs a build cycle and nine red tests.

### `GraphRename` is not optional

`GIMIFixer` edits a deep **copy** of what the parser found; renaming that copy is what makes it the
remapped mod, and `IniSectionGraph::rename` rewrites every `run =` pointing at a renamed section so
the copy's internal wiring follows. Forget it and the fix emits **zero** remapped sections while
still hiding the originals — strictly worse than doing nothing.

Use `IniNamingTools::getRemapFixName` for drawn objects and `getRemapBlendName` for blend; they are
different conventions and mixing them produces names the remover will not recognise later. Rename
goes **first** in the edit list and **unfiltered** (an empty `PartFilter` = the whole part), since
it renames whole sections.

### `keyFilters` are load-bearing, not polish

Because the parser runs with `disjointModObjs = false`, one section is routinely classified into
several mod objects, and only the stretch governed by a given object's own `hash`/`match_first_index`
pair belongs to it. Without a filter every edit runs over the *whole* section for *each* of its
objects — so "fixing" `body` rewrites `head`'s registers. Wrong output, not merely broad.

`AGRemapCore::GIMIObjPartFilter` is that logic, generalized — the fix-side counterpart of
`GIMISectionClassifier`:

```cpp
GIMIObjPartFilter<> objFilter(ctx.modTypeHashes(), ctx.modTypeIndices(), {"ib"}, ctx.version());

iniEdits.edits[modObj]       = {renameAdapter, removeAdapter, addAdapter};
iniEdits.keyFilters[modObj]  = {PartFilter{}, objFilter.filter(modObj), objFilter.filter(modObj)};
iniEdits.keysToTrack[modObj] = objFilter.keysToTrack();
iniEdits.trackKeys[modObj]   = true;
```

`keyFilters` is a list **parallel to `edits`** — one entry per edit, same order.

**`keysToTrack` is about what the *filters* read, never what the edits act on.** The natural mistake
is `{run, drawindexed}` (the keys the edits touch); the answer is `{hash, match_first_index}`. Get it
wrong and the filter sees an empty colouring, reads it as "this object owns nothing", and skips every
edit while everything still reports success. **Call `objFilter.keysToTrack()` and never restate it.**

### The blend

Use **`AGRemapCore::VGRemapBlendReplace`**, never the plain `RemapBlendReplace`. The base inherits
`ResReplace::buildResModel`, which builds a plain `IniFixResource` — a straight *copy* of the
`Blend.buf` with the vertex-group weights untouched. Only the pybind11 layer overrides it. That
inherited behaviour is the exploded-mesh bug, and a remapped `.ini` pointing at an unremapped `.buf`
is worse than emitting nothing, because everything observable says it worked.

Three more things that each silently produce nothing:

- **`resModObj` must be a distinct mod object** from the source graph — it is where the resource's
  *newly created* graph goes. Pointing it at the source makes the resource overwrite what it was
  collected from. Use `("", "<obj>RemapBlend")`.
- **A blend `partPredicate` must be a whole-part accept/reject**, not the sub-part windowing
  `GIMISectionClassifier` does. The qualifying `hash` is on the `TextureOverride` root while the
  `vb1` is down in the `CommandList` it runs — different sections, so there is no span within one
  part, and windowing by the hash's own order index selects nothing.
- **`resType` must be `"blend"`**, not the base's `"resourceRemapBlend"`. It becomes
  `IniResource::type`, which `RemapStats::get` looks up, and that only knows the short kind names.

### Registers: `RegAssetRemap` vs `RegNewVals`

Both write register values; they answer different questions.

- **`RegAssetRemap`** maps whatever value is *actually there* onto the target mod's equivalent
  (reverse-lookup then forward), the way the pure-Python `_getHashReplacement`/`_getIndexReplacement`
  pair did. It takes the asset table **per register**, so in principle hashes and indices are two
  entries rather than two classes. An unmapped value gets `HashNotFound`/`IndexNotFound` rather than
  being left in place — a silently-unmapped hash is indistinguishable from one correctly mapped to
  itself.
- **`RegNewVals`** writes a value that does not depend on the old one. Its values may be a callable
  taking the `ModType`, which is the right tool when the new value is derived from the target mod
  but not from what was there.

**In practice `RegAssetRemap` should only ever be given the `hash`.** Handing it the
`match_first_index` as a second entry is the obvious thing to do and it is wrong — see
"`RegAssetRemap` is right for a hash and wrong for an index" above for why the reverse lookup cannot
work on an index, and what to use instead. Which registers need remapping at all is per-character:
Raiden needs only the blend's `hash`, a CN-skin character needs every object's `hash` **and** a
forward-looked-up index.

<br>

## Seams that work from Python and do nothing from C++

Whole halves of the fixer layer were only ever reached through pybind11. Each fails silently. If a
fix builds nothing, check these before anything else:

- **`ResRegCollect` never builds from C++ unless it is given a context.** It collects *and* builds,
  and the build half is gated on `ctx != nullptr && ctx->hasIni()`. `edit()` passes `nullptr`, and
  it inherited `BaseIniGraphGroupEdit::editFromIni`, which **deliberately discards its `ini`**.
  Fixed in core now (`ResRegCollect::editFromIni` builds an `IniFileResEditContext`), but the
  pattern recurs.
- **`GIMIFixer::applyGraphGroupEdits` hands every group edit a `nullptr` ini.** A fixer that owns
  its own `IniFileFixContext` must override it and pass `ctx_.getIniFile()`.
- **`GraphGroupEdit` had no core-side `PartEdit` adapters** — only `PyPartEdit`. Use
  `GraphPartEdit`/`RegPartEdit` from `graphGroupEdits/GraphGroupPartEdits.h`.

General rule: when a `graphGroupEdits/` or `resEdits/` class has a `Py*` counterpart that overrides
something, **check whether core overrides it too**. If only the binding does, the plain-C++ path is
almost certainly inert.

<br>

## Verifying — the only thing that actually decides it

The Python suite cannot see `AGRemapCore` work with no binding, and the standalone C++ tests only
assert on a strategy's *shape*. Neither would have caught any of the five silent failures above.

Two launchers live in the user's mods folder (ask for the path — Raiden's was
`E:\Computer\Games\Wuthering Waves Mods\Importer\GIMI\Mods`):

- **`FixRaidenBoss6.py`** — the old pure-Python script. Still works. **This is the reference.**
- **`FixRaidenBoss7.py`** — a thin launcher onto the development checkout's live `main.py`.

Both take `-s <abs path>`. **Always run them against scratch copies**, never the user's real mods.

### Getting a genuinely unfixed baseline — two traps

1. **A `RemapBKUP*.txt` backup is not necessarily pristine.** The ones in the test mod were
   themselves taken from an already-fixed `.ini`. Restore from them and the old script correctly
   no-ops, which looks like the old script doing nothing.
2. So instead: copy the folder, run `FixRaidenBoss6.py -s <copy> -u`, delete leftover `RemapBKUP*`,
   and **verify** `grep -c "RemapBlend\|RemapFix"` returns **0** before branching into `old/`/`new/`.

The `EOFError: EOF when reading a line` ending every run is `logger.waitExit()` with no stdin.
Harmless. A run left sitting at `== Press ENTER to exit ==` **holds the `core.*.pyd` open**, which
makes the next build's install step fail — see [Building](../Building/CLAUDE.md).

### What to check

```bash
# The load-bearing one. An unremapped .buf is byte-identical to its source,
# produces a correct-looking .ini, and reports success.
cmp -s "$d/<Folder>/<Char>Blend.buf" "$d/<Folder>/<Char><Target>RemapBlend.buf" \
  && echo "NOT remapped" || echo "remapped"
```

Then: the set of generated sections vs the old script's, each section's content diffed, and
**repeat-run stability** — run the fix 3-4 times and confirm `merged.ini` comes out byte-identical
every time, then `-u` and confirm it lands back on the pristine file. Repeat runs are where
re-fixing your own output, unbounded trailing newlines, and line-ending reflow all show up, and none
of them appear on a single run.

Finally, **ask the user to check in game and send a screenshot.** That is the only test for whether
the vertex groups are right.

### Reading a screenshot: shape vs colour

Two independent things can be wrong, and the screenshot says which:

- **Shape wrong** -- spikes, stretched geometry, the model torn across the screen. That is the
  **vertex** side: the blend weights (`Blend.buf` / the VG remap), or a `match_first_index` putting
  one object's vertex range onto another. `Images/Raiden/6_1/RaidenBlendBroken.jpg` is this failure
  and `RaidenWorking.jpg` is the same mod fixed.
- **Shape right, colours wrong** -- the silhouette is perfect and the character is flat green,
  yellow or pink. That is the **texture** side: a register that should hold a diffuse is being
  handed a `LightMap.dds` or similar. Either the texture-to-register mapping is wrong, or the
  texture itself needs modifying.

`Images/Amber/6_1/AmberLightMapInDiffuse.jpg` is the second kind -- **and it is worth knowing why,
because it is a fixer bug wearing a texture bug's clothes.** It is what Amber looks like on game
version 6.x when **`NNFix` was never applied**. ORFix/NNFix do some genuinely useful texture
copying, but much of that external library is an overglorified `regEdits/`, and from 6.x on a mod
that does not re-issue `NNFix` loses the copy that puts the right texture into the right register.

So on 6.x, "all green" is the expected symptom of the `RegDelimitedAdd` step silently not firing.
**Check `grep -c NNFix` on the output before concluding anything about textures.** The Amber mod
used for testing is genuinely broken in the texture department, and the old pure-Python library
still remapped it back to its correct colours -- so anything less than that is this library's
regression, not the mod's fault.

<br>

## Things this repo's writers must not do to a user's mod

Learned by doing them:

- **Do not change line endings.** `IniFile::readFromDisk` normalizes CRLF to LF; both writers
  (`IniFile::write` and `IniFileFixContext::writeFixedFile`) restore the file's own ending via
  `IniFile::lineEnding()`. There are exactly two writers — fixing one and not the other looks fixed
  and is not.
- **Do not let output grow across runs.** The fix appends a `\n\n` separator every run and the
  removal does not take the previous one back out, so `srcTxt` is `rstrip`'d first.
- **Do not re-classify this software's own output.** `GIMISectionClassifier::classify` refuses
  sections whose name carries `Remap` (as `classifyByTextureOverrideName` always has). A leftover
  remapped section keeps a perfectly valid hash and classifies just as convincingly as the original,
  which is how a second run comes to remap an already-remapped name into
  `…RaidenBossRemapRaidenBossRemapBlend`.
