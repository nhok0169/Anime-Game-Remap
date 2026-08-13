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

**Both files list their "Tools" section entries in strict alphabetical order** (case-insensitive)
— keep new entries sorted in, don't just append. This is the intent and mostly holds, but isn't
airtight already — `api.rst`'s "Tools" section has at least one pre-existing pair
(`OrderedMultiMap`/`OrderedMultiMapSqrt`) sitting out of order, from before this rule was
established. Don't take a pre-existing neighbor's position as proof of where a new entry
alphabetically belongs — compute it from the full section's names yourself, and don't spend time
fixing unrelated pre-existing misplacements while you're in there (same "don't fix unrelated
pre-existing issues in a feature PR" spirit as the warning-baseline guidance below).

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

### Python side (docstrings passed as the `R"doc(...)doc"` string to `py::class_`/`.def(...)`)
- **`@copybrief`/`@copydoc`/`@copydetails` do nothing here — don't use them.** These strings are
  plain Python docstrings rendered by Sphinx's `autodoc`/`napoleon`, not by Doxygen at all (that
  pipeline only ever touches the C++ headers under `core/include`). Writing `@copydoc get(...)`
  in a `py::doc(R"doc(...)doc")` string doesn't fail loudly — it just renders as inert literal
  text in the output. This is an easy slip specifically because the C++-side conventions
  documented above use these commands constantly; if you're writing a pybind11 docstring, write
  the description out (or reuse a short hand-written pointer like "same as the overload above")
  instead.
- Same "inherits from" convention, using `:class:`Parent``.
- **A documented "inherits from" line is only meaningful if the inheritance is real at the
  pybind11 level**, i.e. `py::class_<Derived, Base>(m, "Derived", ...)` with `Base` registered
  via its own earlier `py::class_<Base>(m, "Base")` call in the same module-init function (see
  `PyDFA.cpp`'s `BaseDFA`/`DFA` pair, or `PyIfContentPart.cpp`'s `CppIfTemplatePart`/
  `CppIfContentPart` pair) — not just prose claiming a relationship pybind11 doesn't actually
  encode. Real inheritance gets you `isinstance()` and attribute inheritance for free; a
  docstring-only claim doesn't. See [Architecture](../Architecture/CLAUDE.md) for the pybind11
  mechanics behind this.
- **Naming pitfall that silently breaks doc rendering**: Sphinx's `autodoc` collapses a Python
  class to a bare `alias of X` stub — dropping the docstring, member list, and any
  "inherits from" line entirely, with **no warning** — whenever the name it's documented under
  differs from the class's real `__name__`. This happens if you register a pybind11 class under
  its bare name (e.g. `m, "IfContentPart"`) and then rename it on import
  (`from .core import IfContentPart as CppIfContentPart`). The fix used throughout this codebase:
  give the pybind11 registration itself the final `Cpp`-prefixed name
  (`m, "CppIfContentPart"`) and import it straight, no `as` — see `CppRemappedKeyData`/
  `CppKeyRemapData` for the pattern that already got this right, versus `CppIfContentPart`/
  `CppIfTemplatePart` which needed correcting to match. If you add a new Cpp-prefixed binding,
  register it under the prefixed name directly and confirm in rendered HTML that it isn't
  showing up as `alias of ...`. See [Architecture](../Architecture/CLAUDE.md) for when/why a
  binding needs the `Cpp` prefix at all.
