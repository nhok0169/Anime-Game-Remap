# Documentation

The Doxygen -> Sphinx (via Breathe) documentation pipeline, and the doc-authoring conventions
and gotchas this codebase has actually hit. See [Building](../Building/CLAUDE.md) for how the
`-d` build flag and the standalone `doxygen Doxyfile` step fit together.

Two source trees, two audiences:

- **`Docs/src/coreAPI.rst`** — the C++ core API (`AGRemapCore`), via `.. doxygenclass::` /
  `.. doxygenfunction::` + the custom `.. cppattributetable::` directive.
- **`Docs/src/api.rst`** — the Python API, via `.. autoclass::` / `.. autofunction::` + the
  custom `.. attributetable::` directive. Sphinx's `sys.path` points straight at
  `api/src/py`, so this documents your locally-built package, not a pip-installed one.

**Both files list each of their live sections' entries in strict alphabetical order**
(case-insensitive) — keep new entries sorted in, don't just append. Each file currently has two
live sections, `Model` and `Tools` (see the dedicated structure section below for what belongs in
which). This is the intent and mostly holds, but isn't airtight already — `api.rst`'s `Tools`
section has at least one pre-existing pair (`OrderedMultiMap`/`OrderedMultiMapSqrt`) sitting out
of order, from before this rule was established. Don't take a pre-existing neighbor's position as
proof of where a new entry alphabetically belongs — compute it from the full section's names
yourself, and don't spend time fixing unrelated pre-existing misplacements while you're in there
(same "don't fix unrelated pre-existing issues in a feature PR" spirit as the warning-baseline
guidance below).

**`core/xml/` (Doxygen's generated XML output) is checked into git in this repo**, not
gitignored — so a plain `doxygen Doxyfile` run touches (and stages, if you `git add` broadly)
most files under it, including many for headers nobody edited this session, purely from
timestamp/hash churn. Expect a large `core/xml/*.xml` diff alongside your real source/`.rst`
changes any time you rerun Doxygen; that's expected, not a sign something went wrong.
`Docs/build/` (the actual rendered HTML/Sphinx output) *is* gitignored, so a local Sphinx build
needs no cleanup regardless.

## Building the docs
```bash
cd Docs
py -3 -m sphinx -b html -W --keep-going src build/html
```
(equivalent to `make.bat html` / `make html`, but `-W --keep-going` surfaces every warning
instead of stopping at the first). Remember: rerun Doxygen first (see
[Building](../Building/CLAUDE.md#fast-iteration-on-c-core-only-changes)) if you changed a C++
header comment — Sphinx reads the cached `core/xml/`, not the headers.

There's an established warning baseline from pre-existing, out-of-scope issues elsewhere in the
docs (stale `@copydoc` targets, a couple of `@param` name mismatches, some toctree/inventory
warnings) — compare the **count and content** of warnings before/after your change rather than
assuming zero is achievable; don't fix unrelated pre-existing warnings inside a feature PR unless
asked. Grep the rendered HTML for your own new content (class name, doc text) to confirm it
actually rendered — silence in the warning log is necessary but not sufficient; some failure
modes below produce no warning at all.

**A plain incremental build only shows warnings for files Sphinx actually reprocessed** (`0
added, 0 changed, 0 removed` means it reused the cached result and reported nothing new) — a
handful of files being edited this session can make the build look like it only has ~7 warnings
total, when a full `py -3 -m sphinx -E -b html -W --keep-going src build/html` (the `-E` discards
the cached environment and reprocesses *everything*) reveals closer to 30, spread across files
nobody touched this session (`apiExamples.rst`, `tutorial.rst`, `commandOpts.rst`,
`findVertexGroupRemap.rst` — undefined labels, duplicate labels, a couple of malformed enumerated
lists). Neither number is "wrong" — they're answering different questions. Use a plain incremental
build for your everyday compare-before/after-my-change loop (fast, and the file(s) you're editing
always get reprocessed regardless of cache state), but reach for `-E` specifically when: a warning
you're chasing doesn't make sense against the file it's reported against (see the next paragraph),
or you need the actual full-site total rather than "whatever happened to already be stale."

**Sphinx's incremental cache is keyed off the `.rst` file's own content, not the compiled module it
autodocuments — editing only a pybind11 docstring (no `.rst` text change) is invisible to a plain
incremental build.** After moving/changing methods on a pybind11-bound class (a C++ recompile,
`api.rst` untouched), an incremental Sphinx build reports `0 changed` for `api.rst` and silently
keeps serving the *old* rendered content from its cached environment — a diff/grep against that
stale output can look clean (no new warnings, content unchanged) while actually proving nothing
about your change. Always rerun with `-E` after any C++/Cython/pybind11 change that alters what
`autodoc`/`autoclass` would introspect, even if you didn't touch the `.rst` file at all — this is
the same fix as the header-comment case above, just for a different trigger (compiled binary
changed vs. Doxygen XML changed), both invisible to the same incremental cache.

**When a Doxygen/Breathe-sourced error's reported line number doesn't match anything meaningful in
the named `.rst` file** (e.g. `coreAPI.rst:4: ERROR: ...` pointing at a blank title-underline
line), the real source is very likely injected content from a C++ header's `@rst` block reached
via `.. doxygenclass::`/`.. doxygenfunction::` elsewhere in that same page — Breathe's line-number
attribution for expanded Doxygen content is not reliable. Don't stare at the reported line; `grep`
the *entire* relevant header tree for the suspicious pattern, and when the grep returns multiple
candidate files, check **every** one of them before concluding you've found the culprit — narrowing
down to "the file I already had a reason to be looking at" without verifying the others is exactly
how a fix can look like it worked-by-coincidence while the actual bug (in a file you never
reopened) survives untouched.

## Doc-writing conventions specific to this codebase

### C++ side (Doxygen, in `.h`/`.tpp` comments)
- Long-form prose goes inside `@rst ... @endrst` blocks so Sphinx-flavored reST (cross-refs like
  `:cpp:class:`Foo``, `` `Term`_ `` links, math, tables) renders correctly.
- `@copybrief`/`@copydoc`/`@copydetails` **must be at the top level of the comment, never nested
  inside an `@rst` block** — `@rst` is a raw-passthrough block for Breathe, so Doxygen commands
  inside it are never processed and render as literal text (e.g. `@copybrief Foo::bar(...)`
  showing up verbatim in the built docs). This is a real bug this codebase has hit more than
  once; if you see literal `@copybrief`/`@copydoc` text in rendered output, this is why.
- Even placed correctly, cross-class `@copybrief`/`@copydoc` targeting an overload disambiguated
  by a **complex templated parameter list** (e.g. an argument typed
  `tsl::ordered_map<K, V, Hash, Eq>`) can silently fail to resolve — no warning, just an empty
  brief/description. Simpler disambiguation (unambiguous name, or a plain-type arity difference)
  resolves fine. When you hit this, don't keep tweaking the `@copydoc` target — hand-write the
  prose instead (a little duplication beats a silently-broken doc).
- Inline `(@copybrief Name)` / `(@copydoc Name)` can swallow trailing punctuation into the target
  name if not careful (Doxygen reads the target until whitespace) — e.g. `@copybrief removeKey)`
  parses `removeKey)` as the target and fails to resolve. Prefer hand-written prose or `@ref` over
  cramming a copy-command inside parens.
- `@copybrief Name` used as a `@param`'s description text does **not** copy that parameter's own
  doc — there's no such mechanism; it copies `Name`'s function-level `@brief` instead, dumping
  the wrong (and usually much longer) text in the wrong place. If you want "this param means the
  same as that other function's same-named param," write it out by hand
  (`Same meaning/default as <other function>'s <param>`) rather than reaching for `@copybrief`.
- For a type alias that should show its **expanded** definition in the docs instead of the opaque
  alias name (e.g. a `std::function<...>` typedef), use the established
  `AGREMAPCORE_DOCS_PARSE` pattern already present throughout `tools/orderedMultiMap/`:
  ```cpp
  #ifdef AGREMAPCORE_DOCS_PARSE
  #define RemoveKeyCheck std::function<bool(const V&, long long)>
  #else
  using RemoveKeyCheck = std::function<bool(const V&, long long)>;
  #endif
  ```
  driven by `PREDEFINED = AGREMAPCORE_DOCS_PARSE` + `MACRO_EXPANSION = YES` in `Doxyfile`.
- For a child class, document the parent with a line at the top of the class's `@rst` block:
  `This class inherits from :cpp:class:`Parent``. Existing precedent:
  `IfContentPart`/`OrderedMultiMap`/`OrderedMultiMapSqrt`/`OrderedMultiMapAdapter`.
- **A `:cpp:X:` cross-reference role missing its domain prefix** (plain `` :member:`bytes_` ``
  instead of `` :cpp:member:`bytes_` ``) produces `ERROR: Unknown interpreted text role "member"`
  — a hard error, not a warning, and one that Breathe/docutils attributes to an essentially
  meaningless line number in whichever `.rst` file's `.. doxygenclass::`/`.. doxygenfunction::`
  pulled the comment in (see the `-E`/misattributed-line-number note above for how to actually
  track this down). `member`, `func`, `class`, `var`, `type`, `enum`, `enumerator`, `concept` are
  all real, registered Sphinx C++ domain roles (verified via
  `sphinx.domains.cpp.CPPDomain.roles`) — the bug is specifically a missing/typo'd `cpp:` prefix,
  not an invalid role choice, so don't "fix" one of these by downgrading it to a plain code
  literal (losing the cross-reference) when adding the missing prefix is the real, correct fix.
  **The reverse mistake also happens and is just as fatal**: `` :cpp:`Foo` `` with *no* member-type
  suffix at all (`class`/`func`/`expr`/etc.) is not a valid role either — `:cpp:` alone isn't
  registered, and this produces `ERROR: Unknown interpreted text role "cpp"` (also misattributed
  to a meaningless line number the same way). If you just want to reference a generic
  term/library name inline rather than a specific documented C++ entity, don't reach for a bare
  `:cpp:` role at all — either give it the real suffix you mean (`:cpp:class:`/`:cpp:type:`), or,
  if it's a term with an established glossary entry (e.g. `` `tsl::ordered_map`_ `` — check the
  `.. _Term: URL` block at the end of the file first), use that link form instead, matching how
  every other file in this codebase already references it.
- **`#member`-style auto-linking (`` #Id `` inside a plain, non-`@rst` Doxygen field like
  `@tparam`/`@throws`/`@param`) only resolves to a real *member* of the enclosing scope** — a
  variable, function, typedef, or nested class. **It does not resolve for a template parameter
  name**, even though `#Id` looks identical whether `Id` is a real member or the class's own
  template parameter. `@tparam IdHash The hash function for #Id` (referring back to the `Id`
  template parameter from a sibling `@tparam` line) produces
  `warning: explicit link request to 'Id' could not be resolved` — easy to miss since it's *only*
  a warning, and only fires for uses *outside* an `@rst ... @endrst` block (content inside `@rst`
  is raw passthrough for Breathe/Sphinx, so Doxygen's own auto-linking never touches it there,
  warning or not — meaning the identical-looking `#Id` mention elsewhere in the same class's `@rst`
  prose produces no warning at all, which can make the bug look inconsistent until you notice the
  block boundary). Fix: reference a template parameter by name in plain code formatting
  (```` ``Id`` ````), not `#Id` — reserve `#member` linking for actual members.
- **A bare `` :class:`Foo` `` cross-domain reference from a C++ header's `@rst` block (rendered
  into `coreAPI.rst`) into a Python-side name documented in `api.rst` does not resolve** — Sphinx's
  Python domain needs the fully-qualified dotted name to look it up from a different document with
  no established `py:module`/`py:class` context stack, and `coreAPI.rst` never sets one (it's a
  C++-domain page). Per the existing "unresolved `:class:` reference" gotcha above, this fails
  **silently** — no warning, degrades to inert `<code>`-styled plain text with no link. Verified
  empirically: `` :class:`BaseSLR1Parser` `` rendered as non-clickable text; changing it to
  `` :class:`~FixRaidenBoss2.BaseSLR1Parser` `` (leading `~` hides everything but the last
  component in the rendered text, exactly like the equivalent Python-side convention) made it a
  real `class="reference internal"` link into `api.html`. When cross-referencing a Python-side
  class by name from *inside* a C++ header's doc comment, always use the fully-qualified
  `~FixRaidenBoss2.X` form, never the bare class name — and grep the rendered HTML afterward (see
  the general "grep for `class="reference internal"`" advice above) to confirm it actually
  resolved, since a clean build alone won't tell you.
- **Blank-line spacing around a grouped h2 section differs from flat entries in `coreAPI.rst`.**
  When several related classes share a group heading with h3 sub-items underneath (`DFAs and
  Tries`, `If Templates`, `Ordered MultiMaps`, `Hashing`), **two** blank lines surround the
  group's own h2 heading on both sides (right before it, and right after its last h3 item's
  closing `:raw-html:`<br />``) — but consecutive **flat** h2 entries with no nesting (`Algo` →
  `BaseIdGenerator`, `IntTools` → `ListTools`) get only **one** blank line between them. Not
  enforced by Sphinx (no warning either way) — purely a visual convention already followed
  throughout the file — but check a couple of existing group/flat transitions before adding a new
  grouped section so the spacing actually matches.

### Python side (docstrings passed as the `R"doc(...)doc"` string to `py::class_`/`.def(...)`)
- **`@copybrief`/`@copydoc`/`@copydetails` do nothing here — don't use them.** These strings are
  plain Python docstrings rendered by Sphinx's `autodoc`/`napoleon`, not by Doxygen at all (that
  pipeline only ever touches the C++ headers under `core/include`). Writing `@copydoc get(...)`
  in a `py::doc(R"doc(...)doc")` string doesn't fail loudly — it just renders as inert literal
  text in the output. This is an easy slip specifically because the C++-side conventions
  documented above use these commands constantly; if you're writing a pybind11 docstring, write
  the description out (or reuse a short hand-written pointer like "same as the overload above")
  instead.
- Same "inherits from" convention, using `:class:`Parent``. **Write it out as prose — Sphinx does
  not auto-render a "Bases: X" line for these classes in this project's config**, even when the
  inheritance is real at the pybind11 level (confirmed empirically: a full rendered build has zero
  occurrences of "Bases:" anywhere in `api.html`). Skipping this line because "the inheritance is
  already real, autodoc will show it" silently produces a page with no visible parent-class
  mention at all — this is exactly what happened to `Hashes`/`Indices` (real `py::class_<PyHashes,
  PyModMappedAssets>` inheritance, but the docstring omitted the line) until caught by comparing
  the rendered page against every other live entry in the file.
- **A documented "inherits from" line is only meaningful if the inheritance is real at the
  pybind11 level**, i.e. `py::class_<Derived, Base>(m, "Derived", ...)` with `Base` registered
  via its own earlier `py::class_<Base>(m, "Base")` call in the same module-init function (see
  `PyDFA.cpp`'s `BaseDFA`/`DFA` pair, or `PyIfContentPart.cpp`'s `IfTemplatePart`/
  `IfContentPart` pair) — not just prose claiming a relationship pybind11 doesn't actually
  encode. Real inheritance gets you `isinstance()` and attribute inheritance for free; a
  docstring-only claim doesn't. See [Architecture](../Architecture/CLAUDE.md) for the pybind11
  mechanics behind this.
- **A pure-Python function's type hint referencing a pybind11-bound class needs a real import
  backing it, not just a bare quoted forward-reference string** — `bufFile: "CppBufFile"` with no
  `CppBufFile` import anywhere in the file type-checks/imports/runs fine (Python never evaluates a
  string annotation unless something explicitly calls it), and a `` :class:`CppBufFile` `` role in
  the same docstring renders and cross-references fine independently (Sphinx resolves that from
  its own `.rst`/docstring text, not from the live Python annotation) — so nothing in the normal
  build-and-render verification loop catches the gap. It only shows up if something actually
  resolves the annotation, e.g. `typing.get_type_hints()` (raises `NameError: name 'CppBufFile' is
  not defined`) or a static type checker. Fix by adding a real import for the referenced class
  (under a `##### CppLocalImports` block if it comes from `.core`, per
  [Architecture](../Architecture/CLAUDE.md)) and using the unquoted annotation once it's importable
  in the module — then confirm with `typing.get_type_hints()` on the function, since a clean Sphinx
  build alone doesn't prove the annotation itself is valid.
- **Naming pitfall that silently breaks doc rendering**: Sphinx's `autodoc` collapses a Python
  class to a bare `alias of X` stub — dropping the docstring, member list, and any
  "inherits from" line entirely, with **no warning** — whenever the name it's documented under
  differs from the class's real `__name__`. This happens if you register a pybind11 class under
  its bare name (e.g. `m, "Xxx"`) and then rename it on import (`from .core import Xxx as CppXxx`).
  The fix used throughout this codebase: give the pybind11 registration itself the final
  `Cpp`-prefixed name (`m, "CppXxx"`) and import it straight, no `as` — see
  `CppRemappedKeyData`/`CppKeyRemapData` for the pattern that already got this right, versus
  `CppIfContentPart`/`CppIfTemplatePart` (both since renamed further, see below) which needed
  correcting to match. If you add a new Cpp-prefixed binding, register it under the prefixed name
  directly and confirm in rendered HTML that it isn't showing up as `alias of ...`. See
  [Architecture](../Architecture/CLAUDE.md) for when/why a binding needs the `Cpp` prefix at all.
  The prefix is a means, not an end, though: once nothing bare-named collides with a given
  pybind11 class anymore, drop the prefix — two examples of this, taken via slightly different
  routes:
  - `IfTemplatePart` (`PyIfContentPart.cpp`) used to be registered `CppIfTemplatePart` for exactly
    this collision reason, until the deprecated pure-Python `IfTemplatePart` it collided with was
    itself renamed to `IfTemplatePartOld` — at that point the binding was renamed to the bare
    `m, "IfTemplatePart"`.
  - `IfContentPart` (same file) used to be registered `CppIfContentPart`, wrapped by a pure-Python
    `IfContentPart.py` subclass. Once that wrapper's own behavior shrank to nothing but forwarding
    constructor args, it was deleted outright (not renamed to `...Old` — see
    [Architecture](../Architecture/CLAUDE.md)'s "Two different outcomes" section for when deletion
    beats an `...Old` rename) and the binding took over the bare `m, "IfContentPart"` name.

  Both end up matching how `OrderedMultiMap` was never prefixed in the first place (nothing ever
  collided with it).
- **An unresolved `:class:`X`` cross-reference (e.g. a "This class inherits from :class:`X`" line
  where `X` has no live `.. autoclass::` entry anywhere) does *not* produce a Sphinx warning by
  default** (this repo's `conf.py` doesn't set `nitpicky = True`) — it silently degrades to inert,
  unlinked plain text instead of a hard/soft error either way. Confirmed empirically: making
  `RegAdd` live (whose docstring references `:class:`BaseRegEdit``) while `BaseRegEdit` itself was
  still commented out produced the exact same warning count as before, and the rendered HTML showed
  `BaseRegEdit` as plain `<code>` text, not a broken link. Practical effect: you do **not** strictly
  need to also make a referenced parent class live just to keep the build clean — but doing so
  (when reasonably in scope) upgrades the reference from dead text to a real link, which is why
  `BaseRegEdit`, `BaseIniGraphPartEdit`, and `BaseIniGraphEdit` were each made live alongside a
  child class that referenced them. Treat "does it still warn" and "does the cross-reference
  actually resolve to a link" as two separate questions — grep the rendered HTML for
  `class="reference internal"` around the class name to check the latter, since a clean warning
  count alone doesn't confirm it.
- **A base pybind11 class with real inheritance but no live `api.rst` entry hides its methods from
  the derived class's docs page — even though they work fine at runtime.** `DFA` (`PyDFA.cpp`) has
  genuine pybind11 inheritance from `BaseDFA` (`py::class_<PyDFA, PyBindDFA, BaseDFACls>`), so
  `someDFA.clear()` etc. resolve correctly via Python's MRO at runtime — but `BaseDFA` itself was
  never given a `.. autoclass::` entry anywhere in `api.rst`, and `DFA`'s own entry doesn't use
  `:inherited-members:` (matching this file's usual live-entry style — see the naming-pitfall
  bullet above), so every method bound only on `BaseDFA`'s `py::class_` (`clear`, `reset`,
  `isAccept`, `stateExists`, `stateLen`, `acceptLen`, `isStart`, `addState`) was completely absent
  from `DFA`'s rendered page. The fix: bind those methods directly on `DFA`'s own `py::class_`
  registration too (moving, not duplicating, the `.def(...)` calls — a method with no reference to
  the base type in its argument/return position binds identically regardless of which
  registration it's attached to, and virtual dispatch through a trampoline is unaffected either
  way, since that's decided by the C++ vtable, not by which `py::class_` block a `.def()` call
  happens to sit in). `BaseDFA`'s registration itself has to stay (pybind requires a base to
  already be registered before it can be named as one in `py::class_<Derived, ..., Base>`), just
  with no methods left on it. If another base/derived pair in this codebase has the same shape
  (a base class registered only for the inheritance relationship, with no corresponding live doc
  page), check whether the same fix applies before assuming `:inherited-members:` is the answer.
  **Update, since this was originally written**: that specific fix (moving `.def(...)` calls) is
  the right one for a **pybind11**-bound class specifically, where the trampoline/vtable makes
  moving registrations free and equivalent. For a plain **pure-Python** edit class whose base is
  itself pure-Python and not worth making its own live page (e.g. `RegAdd`/`RegRemap`/`RegRemove`
  vs. `BaseRegEdit`, or `GraphRename` vs. `BaseIniGraphEdit`), reaching for `:inherited-members:`
  on the derived class's own `.. autoclass::` *is* the answer, and by now is used throughout this
  file's live `Model` section — don't take the "isn't used anywhere" framing above as still
  accurate; it described the file's state before this session's `regEdits`/`graphGroupEdits`/
  `graphEdits` pass, which added it to every entry whose immediate parent isn't itself live.
  **Second update, later still**: `:inherited-members:` also works correctly for a **pybind11**
  base with no live page, not just a pure-Python one — confirmed empirically making `Hashes`/
  `Indices` (`py::class_<PyHashes, PyModMappedAssets>`) live while `ModMappedAssets` itself stayed
  commented out: every inherited method (`hasFrom`, `getKey`, `replace`, `replaceAll`, `get`, ...)
  rendered correctly on `Hashes`'/`Indices`' own pages via `:inherited-members:` alone, no method
  moving needed. This means the original `DFA`/`BaseDFA` fix (moving `.def(...)` registrations)
  was solving a *different* problem than "the base has no live page" — `DFA`'s own entry genuinely
  didn't use `:inherited-members:` at the time (this was written before that directive's use
  became this file's live-entry convention), and that, not the missing base page, was the actual
  cause. **Don't reach for "move the `.def()` calls" as the default fix any more — try
  `:inherited-members:` on the derived class's own entry first and verify empirically (grep the
  rendered HTML for the inherited method names), regardless of whether the base is pybind11- or
  pure-Python-backed.** Moving registrations is still the *right* move in one specific situation
  the DFA case actually had: a base class that exists **only** to be inherited from and is never
  meant to be constructed/used directly on its own (there, "the method is missing from the base's
  own page" is moot, since nothing should be looking at that page). It is the **wrong** move for a
  base like `ModMappedAssets` that's independently useful and directly constructed elsewhere (e.g.
  by its own test suite) — moving a method off such a base's own `py::class_` registration onto
  only the derived class's would be a real *runtime* regression (a plain `ModMappedAssets`
  instance loses that method entirely), not just a docs cosmetic issue. Check whether the base is
  ever constructed/used directly in its own right before choosing between the two fixes.
- **A docstring section heading only gets napoleon's special numpydoc-style treatment (turned into
  a proper parameter field list) if it's spelled exactly right** — `Parameters`, not `Paramters`.
  A misspelled heading falls back to being parsed as a literal RST section title instead, which
  participates in `api.rst`'s document-wide implicit-target-label namespace (the whole file is one
  Sphinx document; every `.. autoclass::`/`.. autofunction::` on the page shares that one
  namespace) — two docstrings on the same page both headed `Paramters` collide with
  `WARNING: duplicate label api:paramters, other instance in ...`. This exact typo already existed
  in `PyDFA.cpp` (three methods) and separately in `Trie.py`, dormant and harmless as long as the
  `PyDFA.cpp` copies stayed on `BaseDFA`'s never-rendered page (see the bullet above) — moving them
  onto `DFA`'s live page made three of them collide with each other and with `Trie.py`'s copy, all
  at once, the moment they actually got rendered together for the first time. Fixing the spelling
  in `PyDFA.cpp` alone resolved all three warnings, including the one against `Trie.py` — a lone,
  correctly-unique `Paramters` heading doesn't collide with anything, so `Trie.py` didn't need
  touching. Grep an existing docstring for `Paramters` (or any other hand-typed section heading)
  before reusing it as a template; the typo won't warn until two copies of it end up rendered on
  the same page.

### `Docs/src/api.rst` / `Docs/src/coreAPI.rst` structure — read before touching either
- **Most of each file is deliberately commented out** (`.. ClassName`, `.. .. autoclass::`, every
  line of the block prefixed with `..`) — this is not stale/broken documentation to clean up, and
  not something to silently uncomment while working on something else. Per the maintainer, it
  reflects an in-progress migration (more code moving to C++, `.ini` parsing moving to a more
  graph-based approach) — a commented block means "not ready to publish yet," not "forgotten."
  Leave it alone unless the user explicitly asks to make a specific class live.
- **Each file has two live, h1-headed sections: `Model` and `Tools`**, both kept in strict
  alphabetical order (case-insensitive) internally — a maintainer-driven split, not something
  either file always had. **`Tools` is for generic, reusable-outside-this-project building blocks**
  (data structures, algorithms, string/hash/graph utilities — things with no idea what a "mod" or
  a `.ini` file even is). **`Model` is for classes that represent the actual mod-fixing domain**
  (`.ini` structure, mod content, the remap graph). When adding a new class to either file's live
  docs, classify it by what it *actually is*, not by what its name suggests — check its real
  implementation location/purpose first, since the name alone is a poor and sometimes actively
  misleading signal here:
  - `GraphTools` sounds mod-adjacent (it exists to support `.ini` graph analysis) but is itself
    fully generic — its own docstring says "no notion of what a node *is*" — so it's `Tools`, not
    `Model`. `IniSectionGraph`/`CallGraph` (which actually *build* the `.ini`-specific graph) are
    `Model`.
  - `KeyRemapData`/`RemappedKeyData`/`ReplaceIf`/`ReplaceList` sound like they're about the
    project's own "remap a mod" domain, but they're actually generic support types for
    `OrderedMultiMap`'s own internal key-remap/replace operations (confirmed by their C++
    definitions living under `tools/orderedMultiMap/`, not anywhere mod-specific) — `Tools`.
  - `BufTools` runs the other direction: it *sounds* like a generic, `DictTools`/`HashTools`-style
    utility class (a `Xxx` + `XxxTools` naming pattern this codebase uses a lot for genuinely
    generic helpers), but it exists specifically to turn a `.buf` file's decoded frame data into a
    `pandas.DataFrame` — fully `.buf`-file-domain-coupled — so it's `Model`, not `Tools`. Caught
    only after initially filing it under `Tools` by name-pattern alone; knowing this rule doesn't
    make the mistake harder to make when a real sibling name (`DictTools`) suggests the wrong
    section, so still check the implementation, every time, even when the classification "feels"
    obvious from the name.
  When genuinely unsure, grep for the class's actual definition and judge from there, rather than
  guessing from the name or copying a neighboring entry's placement.
- Beyond the live `Model`/`Tools` split, everything else is a much larger, **non-alphabetical**,
  thematically-grouped region (`Ini Parts`, `Utilities`, etc. in `api.rst`; similar groupings in
  `coreAPI.rst`), each its own h2 group with h3 sub-items, that's currently *entirely* commented
  out. When you do make a new class live (only on explicit request), it goes into the correct live
  section (`Model` or `Tools`, per the classification above) by insertion sort against the existing
  entries there — not appended to a thematic WIP group further down. `coreAPI.rst`'s `Model`
  section additionally nests classes under h2 group headings (e.g. `If Templates`) rather than
  listing them flat the way `api.rst`'s sections do — match whichever style the specific file
  you're editing already uses, don't impose one file's style on the other.
- **The thematic WIP region contains stale, already-superseded duplicate entries** for at least
  `IfContentPart`, `IfContentPartColourChange`, `IfContentPartColouring`, `IfTemplatePart`,
  `IniSectionGraph`, `KeyRemapData`, and `RemappedKeyData` — each also has its own live entry up in
  `Model` or `Tools` (per the classification above), and the old commented-out copy further down
  was simply never deleted. Don't take a duplicate's continued presence as evidence a class still
  needs migrating, and don't spend time deduping these while working on something unrelated (same
  "don't fix unrelated pre-existing issues" rule as elsewhere in this file) — only clean up a
  specific duplicate if you're already touching that exact class for another reason.
- **Once a class has gone through the full-replacement migration outcome (see
  [Architecture](../Architecture/CLAUDE.md)'s "Two different outcomes for porting a class to
  C++/pybind11") and its old pure-Python file is actually deleted (not just renamed to
  `...Old`)**, clean up any "the C++ counterpart to the pure-Python ``Xxx``
  (``some/path/Xxx.py``)" framing left over in the new class's own doc comment/docstring from
  when it was first ported — that specific file path no longer exists, so the claim is now false,
  not just stale-sounding. Confirmed via the already-migrated Tokenizer family
  (`BaseTokenizer.h`/`FilteredTokenizer.h`/`IfPredTokenizer.h`/`SympyTokenizer.h`): none of their
  class-level docs mention a pure-Python original at all — the established convention post-full-
  replacement is to describe the class plainly, present tense, with no reference to (or renaming
  of) what it used to be. A design-rationale note that explains *why* a particular default/behavior
  was chosen (e.g. "matches the default id-generation behavior real callers rely on") is fine to
  keep even after the file is gone — that's still true and useful; it's specifically the "this is
  the counterpart to (`path/to/File.py`)" *file-existence* claim that becomes actively wrong and
  should go.
- **A dangling "see the pure-Python original's own docstring for a worked example" pointer is not
  a substitute for the worked example** — it was a shortcut taken during the initial port (skip
  reproducing a diagrammed example, just point at the file that still had it), and it silently
  breaks the moment that file is deleted (see the bullet above), leaving real documentation value
  lost, not just a stale reference. Confirmed missing and restored for `IfTemplate`/`IfTemplateTree`/
  `IfTemplateNonEmptyNodeTree`/`IfTemplateNormTree`: the old pure-Python docstrings had genuinely
  useful `.ini`-code-block examples and hand-drawn ASCII tree diagrams (parse-tree shape,
  before/after transformations) that never got ported at all, not even summarized. If a class's
  own doc comment still says "see the old file for an example" (or said so before being cleaned up
  per the bullet above), don't just delete the sentence — **go get the actual example**:
  - If the pure-Python original still exists (renamed to `...Old` or otherwise), copy the example
    straight from its docstring, translating `:class:`/`` `Term`_ `` references as needed.
  - **If it's already been deleted outright, it's still recoverable from git history** as long as
    it was tracked at some point — find a commit where `git show --stat <commit>` (or
    `git log --diff-filter=D -- <path>`) shows the file being deleted, then
    `git show <that-commit>^:<path>` prints the file's content from immediately before that
    commit. This works even when the whole port (rename to `...Old`, then later delete) happened
    across many uncommitted working-tree edits inside one long agent session and only ever landed
    as a single squashed commit — the *original*, pre-rename file path still shows up as a clean
    deletion in that commit's diff, since git's own rename-detection doesn't survive content
    changing as much as a Python-to-C++ port does.
  - **When one old Python file documented several classes that only one new C++ binding now
    covers** (eg. `IfTemplateTree`/`IfTemplateNonEmptyNodeTree`/`IfTemplateNormTree` were three
    separate Python-bound classes, each with its own "how I differ from my parent" example, but
    only the base `IfTemplateTree` is Python-bound after the port — see
    [Architecture](../Architecture/CLAUDE.md)'s note on why), there's nowhere left to hang the
    subclasses' own examples in the *Python-facing* docs specifically. Fold them into the one
    remaining class's docstring instead, reframed around whatever real, present-tense distinction
    replaced the old class hierarchy (here: "by default" vs "after calling `.normalize()`", since
    that's what actually determines the behavior a Python caller sees now) — don't just drop the
    subclasses' examples because their own class no longer exists. The **C++ core** side is
    different: the core classes (`AGRemapCore::IfTemplateNonEmptyNodeTree`/`IfTemplateNormTree`)
    still exist even though they're not Python-bound, so `coreAPI.rst`/their own header doc
    comments can (and should) keep one dedicated entry each, same as before the port — restore the
    example there per-class, unfolded, and add a `.. doxygenclass::`/`.. cppattributetable::` entry
    for each in `coreAPI.rst` if one doesn't already exist for a class that's newly gained real
    documentation worth surfacing.
- **Every `` `Term`_ `` link-style reference in a docstring needs a matching `.. _Term: URL`
  definition**, or the build reports `ERROR: Unknown target name: "term"` (a hard error, not just
  a warning — `--keep-going` lets the build finish anyway, but don't mistake that for success).
  These definitions live in one big block at the very end of `api.rst` (search for `.. _section:`
  to find it), not scattered near their usage. Before introducing a new glossary-style term in a
  docstring, check whether it's already defined there; if not, add it to that end-of-file block
  rather than assuming a plain-English phrase like `` `dataflow analysis`_ `` or `` `call graph`_ ``
  will just work. Sphinx never validates that the target URL is actually reachable — only that
  some `.. _Term:` definition exists — so a build passing is not proof the link goes anywhere real.
- **A multi-phase feature effort can go its *entire* duration without anyone actually running a
  Sphinx build, even while writing `@rst`-block doc comments with `` `Term`_ `` links throughout.**
  A full Z3 predicate-conversion effort (`IfPredZ3Generator`/`Z3IfPredGenerator`/`Z3Context`/
  `Z3Predicate`/`IfPredPart`, several classes and headers across multiple work sessions) introduced
  `` `Z3`_ ``/`` `sympy`_ `` links and a stray `` `if`_ ``/`` `elif`_ ``/`` `else`_ `` set in a
  docstring, all with no matching glossary entry — and none of it surfaced until a real build was
  finally run at the very end, at which point it was 26 new broken references at once, not one at a
  time when each was easy to spot and fix. The doc comments *look* correct by inspection — the
  `` `Term`_ `` syntax renders as plausible prose in a code review, and nothing short of an actual
  Sphinx build catches the missing target. **Two habits prevent this pile-up**: (1) add the
  `.. _Term: URL` glossary entry in the *same edit* as introducing a new `` `Term`_ `` link, not as
  a "polish later" step — treat the two as one atomic change, same file, same edit; (2) periodically
  run a real `py -3 -m sphinx -b html -W --keep-going src build/html` build during a long doc-touching
  effort, not only once at the very end — don't let "the doc comments look right" substitute for
  "the doc comments were actually built" for the whole duration of a multi-session feature.
- **A Sphinx role (`:class:`, `:meth:`, etc.) immediately followed by a word character is
  invalid reST** — `` :class:`IfContentPart`s `` produces `WARNING: Inline interpreted text or
  phrase reference start-string without end-string`, because inline markup must be followed by
  whitespace or specific punctuation, not directly by more letters. This is easy to write by
  accident when pluralizing a class reference and easy to miss, since nothing catches it short of
  an actual Sphinx build — a docstring can sit wrong for a long time if the containing class is
  commented out of the docs (as most currently are, see above) and only starts warning once it
  goes live. Write `` :class:`IfContentPart`\\s `` instead (backslash-escaped space before the
  trailing letters) — this exact bug was found in four `IniSectionGraph` docstrings the moment
  that class was made live, so don't assume existing docstrings in a still-commented class are
  already correct; check before/while making anything newly live.
