# Tools

The maintainer tooling under [`Tools/`](../../Tools) --- the builders, the CI pipeline, the script,
and the shared `AGRemapUtils` library they all sit on. Every other file in
[AI Agent Help](../README.md) is about the API and its C++ core; this one is about the layer that
*builds and ships* it.

Read this before touching anything under `Tools/`, and read
[Overview](../Overview/CLAUDE.md)'s "Working a feature or bug request here" first regardless.

## Nothing tests these tools, and they rot in one specific way

**Run the tool before you change it.** Not as a formality --- as the first step, expecting it to be
broken. There is no suite over `Tools/`, nothing in CI exercises most of it, and a tool can sit
broken for months because the only person who would notice is whoever next runs it.

On 2026-09-10 a session that set out to add one flag found **three separate tools that could not
run at all**, all broken by the same event: the API's package moved from `api/src/FixRaidenBoss2` to
`api/src/py/FixRaidenBoss2` when the C++ and Cython layers arrived, and nothing that pointed at the
old location was updated.

| What | How it failed | Nothing said so because |
| --- | --- | --- |
| `ScriptBuilder` | `import src.FixRaidenBoss2.main` --- `ModuleNotFoundError` on line 18 | it is only run when cutting a release |
| `APIMirrorBuilder` | `buildMirrorInit()` opened `<apiFolder>/__init__.py`, which no longer existed --- `FileNotFoundError` | same |
| `ScriptSrcFolder` | derived from the API's *module* path, so the script build's **output folder** silently moved to `script build/src/py/FixRaidenBoss2` | it is a path constant; nothing reads it until a build writes there |

The third is the shape to fear: not a crash, a **silently relocated output**. Two of these were
found only because the session happened to run the tool; the third only because the run then failed
one step later.

**So: `py -3 main.py --help` at minimum, and ideally a real run into a scratch directory, before you
form any opinion about what a tool does.** A tool whose paths point at a layout that has since moved
will often fail in a way that looks like *your* change broke it.

### The corollary: check the output, not the exit code

`APIMirrorBuilder` did not merely produce a stale `pyproject.toml` --- it produced one that **no TOML
parser would accept**: a duplicate `dependencies` key, entries re-quoted as `""numpy>=1.26.4""`, and
no comma between array entries (a single-dependency list hid that one for as long as there was only
one dependency). It exited 0 every time. `tomli` and `tomlkit` are installed in this dev environment
for exactly this kind of check, though deliberately **not** dependencies of `AGRemapUtils`.

## How a tool is put together

```
Tools/<Name>/
  main.py              the entry point, and the ONLY entry point
  README.md
  requirements.txt
  <Name>/              the package: constants/, exceptions/, CommandBuilder.py, ...
    constants/Paths.py
```

Two conventions carry real weight:

- **Every tool reaches its siblings by a path relative to the *current working directory*, not to
  `__file__`** --- `UtilitiesPath = os.path.join("..", "Utilities", "src", "AGRemapUtils")`, then
  `sys.path.insert`. This works only because every tool sits at the same depth directly under
  `Tools/`, which is also why the `CIPipeline` can run another tool's `main.py` as a subprocess and
  have that tool's relative paths still resolve. **Put a new tool directly under `Tools/`**; nesting
  it one level deeper breaks every sibling path it uses.
- **Anything that must survive a `chdir` is computed from `__file__`.** `APIBuilder` `os.chdir`s
  into the API during its build, so its `PathToProject` is an `os.path.abspath` off `__file__`.
  Mixing the two conventions in one file is how a tool ends up working from one directory and not
  another.

A tool's command uses `Utils.commands.BaseCommandBuilder`, which calls `_addArguments()` **from
inside its own `__init__`** --- so anything a subclass sets *after* `super().__init__()` arrives too
late to be read. Configuration for a command therefore lives on the **class**, not the instance;
`Utils.commands.BuildEnvCommandBuilder` is the worked example.

## `AGRemapUtils` is a published package

`Tools/Utilities` is on PyPI as `AGRemapUtils`, which constrains it in ways the other tools are not:

- **Keep its dependencies light.** They are `ordered-set` and `directory-tree`. The `.toml` handling
  in `Utils/toml/TomlFile.py` is deliberately text-based rather than pulling in a parser for this
  reason. Do not add a dependency casually.
- **`Utils/constants/toolStats.py` is the source of truth for versions**, and `ToolStatsUpdater`
  propagates them into each `pyproject.toml`. Bump the version *there* and run the updater --- hand
  editing a `.toml` is what left `toolStats.py` on 1.0.5 while `Tools/Utilities/pyproject.toml`
  said 1.0.6, so that running the updater would have silently **downgraded** it.

What lives in it that you are likely to want:

| Area | What |
| --- | --- |
| `files/SourceFile.py` | the keyword-section reader (below). `python/PyFile.py` subclasses it and adds import parsing |
| `credits/CreditsUpdater.py` | the credits walk behind `APIBuilder --addCredits` |
| `toml/TomlFile.py` | sections and whole `key = value` assignments as text, formatting and comments preserved. **Not a parser** --- do not read values out of it |
| `toolStatsUpdater/TomlUpdater.py` | writes name/version/dependencies. **Scoped to `[project]`** |
| `commands/` | `BaseCommandBuilder`, `BuildEnvCommandBuilder` |
| `pipeline/` | `Pipeline` and `Stage` |
| `scriptBuilder/ScriptBuilder.py` | the topological compile |

### `TomlUpdater` is scoped to `[project]` for a reason

Its version pattern used to be `(?<=version)\s*=.*`, matched against the whole file --- so a run
over the API's own `pyproject.toml` rewrote `cmake.version = ">=3.18"` into the software's version.
`ToolStatsUpdater` is CIPipeline's last stage, so that fired on every pipeline run. **Keys are not
unique across a `.toml` file; scope any edit to its section.**

## The keyword sections, and the trap in them

Source files across this repo are divided by comment keywords --- `##### Credits`,
`##### ExtImports`, `##### LocalImports`, `##### Script`, each with an `End` partner. `SourceFile`
reads them; `ScriptBuilder` concatenates the `Script` sections in topological order;
`--addCredits` rewrites the `Credits` ones.

**The keywords are matched as a SUBSTRING of a line, not as a whole line.** That is a feature --- it
is what lets `// ##### Credits` work in C++ with the same keyword table as python. It is also a
landmine:

> A comment reading `# ScriptEnv: The environment this script was built for` contains `# Script`,
> so it **opens a section that never closes**, and the build dies with
> `Missing closing keyword for type: Script`.

This repo's own `# Name: description` comment convention makes that easy to write by accident, and
it cost a build in the session that introduced it. `MissingKeyWord` and `InvalidKeyWordType` now name
the **file and line**, so the next occurrence is a five-second fix --- but the way to avoid it is to
not start a comment with `# Script`, `# Credits`, `# ExtImports` or `# LocalImports`.

The credits convention itself --- which files carry a block, the include-guard placement rule for
headers, and why `--addCredits` maintains blocks but cannot create one --- is in
[Overview](../Overview/CLAUDE.md)'s "Every source file in `api/src` carries a credits block".

## The script does not contain the API any more

**This is the biggest structural change in this layer, and old notes describing the script as "the
API flattened into one file" are stale (changed 2026-09-10).**

The API is a C++/Cython/python project, and a single `.py` file cannot carry a compiled extension
module. So the script *reaches* the API instead of containing it, and went from **31732 lines to
490**.

```
Tools/Script/          the script's own source -- a tool like any other
  Script/
    constants/         BuildEnvs (dev/prod), BuildData (the values compiled in)
    controller/        the options the script adds ON TOP OF the API's
    apiRefs/           BaseApiRef -> PathApiRef (dev) | PackageApiRef (prod)
Tools/ScriptBuilder/   compiles the above into 'script build/src/FixRaidenBoss2/AGRemap.py'
```

**Where an option goes depends on whose option it is.** An option about *remapping* belongs to the
API's `controller/CommandBuilder.py` and the script inherits it for free. An option about *how the
script gets the API* belongs in `Tools/Script`.

`--env/-e` on the `ScriptBuilder` decides how a build reaches the API, and **defaults to `dev`**:

- **`dev`** --- a path relative to the compiled script, worked out at build time and written with
  `/` rather than `os.sep` so a script compiled on one OS still finds the API on another.
- **`prod`** --- downloaded from pypi at runtime, the same shape as the API's own `PackageManager`.
  This build also gains `--update/-up` and `--preRelease/-pre`.

Three things about that design are load-bearing and easy to undo by accident:

1. **The prod options are registered twice, deliberately.** The script must read them *before the
   API exists* --- one of them decides whether the API is downloaded at all --- so they go into a
   throwaway parser first, and then into the API's own command through `remapMain`'s `commandSetup`
   hook. That second registration is what puts them in the same `--help` as every API option, and
   what stops the API's parser rejecting them as unrecognised.
2. **The throwaway parser runs with `allow_abbrev=False`.** With abbreviations on, argparse matches
   a shortened form of an *API* option onto one of the script's and swallows it.
3. **Every placeholder in `BuildData.py` is a string.** That module has to be valid python *before*
   anything is filled in, because `ScriptBuilder` imports the package to work out the order to write
   it out in. A placeholder that is not itself parseable stops the script being built at all.

**`script build/` is the end-user deliverable, so what is committed there must be a `prod` build.**
A `dev` build looks for `../../../api/src/py` on the user's machine, which exists on nobody else's.
This is why **the `CIPipeline` defaults to `prod` while the `ScriptBuilder` defaults to `dev`** --- the
pipeline writes what gets committed, and the ScriptBuilder on its own is what you reach for while
working *on* the script. Running the ScriptBuilder directly therefore leaves a non-shippable script
in a tracked folder; check `EnvName` in the compiled script before committing it.

## The CIPipeline

Four stages, in order:

1. **Building API and Docs** --- `APIBuilder -d`
2. **Compiling Script** --- `ScriptBuilder`, handed the pipeline's `--env`
3. **Compiling API Mirror** --- `APIMirrorBuilder`
4. **Updating Software Metadata** --- `ToolStatsUpdater`

`--env/-e` is the same option as the ScriptBuilder's, **but defaults to `prod`** rather than `dev`
(see above), and is handed down **only to the stages that take one** --- today just the script. The `APIBuilder` deliberately does
*not* get it: its environments are a different set (`dev`/`core`/`cibuildwheel`), and the production
wheels come from cibuildwheel in the publish workflow rather than from the `APIBuilder`.

**Stage 1 changes what a pipeline run costs.** It compiles C++ and Cython, so it needs `cmake`,
`ninja` and `doxygen` on `PATH` and, on Windows, the MSVC environment already initialized in the
same shell --- the pipeline now fails at stage 1 where it used to run anywhere. And because it runs
with `-d`, **it regenerates the tracked `core/xml` and `core.pyi` on every run** (~966 files). See
[Building](../Building/CLAUDE.md)'s `-d` section for when to keep those; the short answer for a
tooling change is that you do not.

## `Tools/Misc`: the scripts the guides mention that were born outside the repo

The remap prototypes, the identity-mod generator, the Yelan hand-experiment scripts and the
maintainer's hand-made reference `.ini` pair, the Linux build script, and two diagnostics
(`modTally.py`, `boneCentroids.py`) -- each written next to the mod it operated on, on the
maintainer's machine, and copied here so another machine can read and run them. Its
[README](../../Tools/Misc/README.md) maps each file to the guide that uses it and says which copy
is the live one. Same rule as the rest of this folder: nothing tests it, run before you change.

## Traps that each cost real time

- **PowerShell's `Select-Object -First N` kills the native process it is reading.** The command
  reports **exit 255 / -1** and truncated output, which reads exactly like the program crashing.
  `py -3 script.py --help | Select-Object -First 16` "failed"; the same command piped to `Out-String`
  exited 0. Capture to a variable and inspect it, rather than trimming a native command's pipeline.
  This is [Overview](../Overview/CLAUDE.md) habit 10 --- validate a failing check --- in its most
  routine form.
- **`python` and `py -3` are different interpreters on this machine.** Measured 2026-09-10: bare
  `python` is **3.9.13**, while `py -3` is **3.9.3** at
  `AppData\Local\Programs\Python\Python39\python.exe`. `Stage` used to spawn the bare name, so every
  pipeline stage ran under a different interpreter than the developer's; it uses `sys.executable`
  now. Anywhere you spawn python, spawn `sys.executable`.
- **A pipeline's own output can arrive after the subprocess output it labels.** The parent's `print`
  sits in a buffer while a stage's subprocess writes straight to the same handle. `Pipeline` flushes
  before handing off; anything else that interleaves its own logging with a subprocess needs to do
  the same, or the log misattributes every line.
- **Git operations here are slow enough to look hung.** Staging the 854-file credits change took over
  two minutes, and `APIBuilder`'s `cleanInstalls()` --- an `rglob` over `api/`, which includes the
  populated `extern/` submodules --- took **twelve**. Run them in the background rather than
  concluding something is stuck.
- **Do not `git add -A`, and quote every path.** Every path in this repo contains both spaces and
  parentheses. Stage explicit path lists, ideally from a python script with a real argument list ---
  see [Overview](../Overview/CLAUDE.md)'s operating norms for the `nhok0169` submodule hazard that
  makes `git add -A` genuinely dangerous here.
