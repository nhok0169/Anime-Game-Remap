# CI (GitHub Actions)

How the workflows under `.github/workflows` fit together, and the things about them that fail
**silently** --- a stuck pull request, a badge with nothing to show, a cache that restores and saves
nothing. Everything here was learned from real runs between 2026-09-17 and 2026-09-18. Pair it with
[Testing](../Testing/CLAUDE.md)'s "The first CI run", which covers the test *failures* those runs
found, and with [Overview](../Overview/CLAUDE.md) for the branches.

**Before changing any workflow, run the wiring checker**, and run it again after:

```bash
python Tools/Misc/Diagnostics/checkWorkflowWiring.py
```

It checks what GitHub only discovers at run time --- inputs a caller passes that the callee does not
declare, `needs.X.outputs.Y` naming nothing, **a tester called without the build artifact** --- and
prints the status-check NAMES every workflow reports. The artifact check is the one that caught a
publish flow calling the testers with no build, which reading the files had missed. It was proved to
fail (exit 1, naming the job) on a copy with that one line removed.

## The map

| File | Runs on | What it does |
| --- | --- | --- |
| `tests.yml` "Testers" | pull request, manual, **schedule every 3 days** (`0 0 */3 * *`, midnight UTC, on `master`) | calls `test-workflow.yml` once |
| `test-workflow.yml` | reusable | `build` (`build-workflow.yml`) then `unit-tests` and `integration-tests` **in parallel**, both handed the build's artifact |
| `build-workflow.yml` | reusable, manual | checkout **with submodules**, restore the z3 cache, `Tools/APIBuilder/main.py -pb -pi -i`, prove the import, upload `api/src/py/FixRaidenBoss2/` as the artifact |
| `unit-test-workflow.yml`, `integration-test-workflow.yml` | reusable | download the artifact over the package folder, install the API's runtime dependencies, then the tester's, prove the import, run |
| `python-publish.yml` | **release published**, manual | build and test, then wheels (`cibuildwheel`, five runners), publish **FixRaidenBoss2**, and only then the mirror |
| `mirror-publish-workflow.yml` / `mirror-publish.yml` | reusable / manual | build and publish **AnimeGameRemap**; last in a release because it pins `FixRaidenBoss2==<version>` |
| `utility-publish.yml` | manual only | **AGRemapUtils**, which has its own version --- deliberately not on a release, which would re-publish an unchanged version and fail as a duplicate |
| `build.yml` | manual | a matrix build, for trying an OS |
| `warm-caches.yml` | **push to `master`** (not prose-only merges), manual | the testers' build once (ubuntu-latest, Python 3.12 -- test-workflow.yml's defaults, which are part of the cache key), so its caches are saved ON `master`, where every branch and PR can read them; and, **only when run by hand**, the wheels' z3 per wheel runner through `.github/actions/wheel-externs` --- so a release shortly after restores it |

No workflow runs the testers on a push: they cost enough that a pull request, the schedule and each
publish are the gates. The one push trigger is `warm-caches.yml`, a build on `master` alone.

## Renaming a job or a workflow strands branch protection

A required status check is matched on its **name**, and a job reached through reusable workflows is
named by the whole chain of caller job names: `tests.yml`'s unit tester reports as
**`Build and Test / Unit Tests / Run Unit Tester`**. When the two tester workflows were merged into
`tests.yml`, the old names (`Unit Tests / Run Unit Tester`, `Integration Tests / Run Integration
Tester`) stopped being reported, and the pull request sat on *"Expected --- Waiting for status to be
reported"* with every real check green. Nothing in the repo shows this; only the PR does.

**So: after renaming anything in a job-name chain, tell the maintainer** the old and new names ---
`checkWorkflowWiring.py` prints them --- because branch protection is their setting, not ours
(Settings -> Branches, or Rules -> Rulesets). And recommend requiring the **build** check as well as
the testers: a tester skipped because the build it `needs` failed counts as passing for branch
protection, so requiring only the testers lets a broken build through.

Two more rules of the same kind:
- **A called workflow cannot hold more permissions than its caller grants.** The publish files set
  `contents: read` at the top, so the job that CALLS the mirror publish declares `id-token: write`
  itself; without it trusted publishing is refused its token at the very last step of a release.
- **Nesting is limited to 4 levels** of reusable workflows. The deepest chain today is 3.

## The testers import the API from the source tree

They never `pip install` the API --- the build's artifact is laid over
`api/src/py/FixRaidenBoss2/` and the tester imports it from there. So **nothing installs the API's own
runtime dependencies unless the tester workflow does**: both read them out of `api/pyproject.toml`
(`numpy>=1.26.4` today) before the tester's `requirements.txt`, and prove the import only after both.
Add a runtime dependency to `pyproject.toml`, never to a tester's requirements.

The failure this prevents does not say what it is: with numpy missing, `CyDictTools` dies with
*"numpy.core.multiarray failed to import (auto-generated because you didn't call
'numpy.import_array()'...)"*. That text is Cython **3**'s auto-generated `import_array()` replacing
the real `ModuleNotFoundError` --- CI installs an unpinned `cython`, and a local Cython 0.29 prints the
honest message instead.

## Caches: z3 as a folder, the API through sccache

- **`externs-...` works, and is the one that matters.** `cebuild<os>` / `cext<os>` hold the built z3
  (~45 minutes from cold). The key is the z3 SUBMODULE's commit, and `APIBuilder` skips `-pb` / `-pi`
  whenever those folders exist, so a hit removes the cost outright. The build step prints
  `externs cache hit: true|false`.
- **The API's own build compiles through sccache (2026-09-18), because caching its build FOLDER
  cannot work.** There used to be an `apibuild-...` cache of `cbuild<os>`, and it restored and saved
  nothing: ninja decides what is stale by modification time, and `actions/checkout` writes every source
  file --- submodules included --- with the time of the checkout, so every source was newer than every
  restored object and all ~790 steps rebuilt. The tell was in the log: a CMake configure of about a
  second (the restored `CMakeCache.txt` skipping curl's checks) followed by `[1/790]` compiling
  utf8proc. It also spent the repository's 10 GB of cache, where the least-recently-used entry is
  evicted --- possibly the z3 one. **Do not bring back a build-folder cache.**

  Now `build-workflow.yml` runs `mozilla-actions/sccache-action`, and both build steps set
  `SCCACHE_GHA_ENABLED=true` and `AGREMAP_CMAKE_ARGS=-DAGREMAP_SCCACHE=ON` --- an `APIBuilder` hook
  (see [Tools](../Tools/CLAUDE.md)) that reaches only the API's configure, never z3's.
  `AGREMAP_SCCACHE` sets the compiler launcher at the top of the API's CMake, so curl, Compressonator
  and utf8proc compile through it too, and it turns the precompiled headers off, which sccache refuses
  to cache. So a COLD CI build is somewhat slower than before, and a warm one compiles only what
  changed. **"Report what sccache did"** prints `--show-stats` after every build, failed or not: on a
  warm run nearly every compile should be a cache hit, and if they are all misses the cache is not
  being reached. That hook was proved on Linux end to end (with the variable, the configure recorded
  `AGREMAP_SCCACHE:BOOL=ON` and CMake's own "sccache was not found" guard fired, since that machine has
  none; without it, `OFF`); the first real CI run is the test of sccache itself.

- **A cache is visible only to its own branch and to the default branch.** A pull request's caches
  belong to that PR, so the z3 cache saved during PR #219 could not be used by a manual run on `master`
  --- which then built z3 from cold, ~45 minutes, and looked like a hang at 25. sccache stores its cache
  in the same place under the same rule. So: **the first run on `master` after its caches are lost is
  cold, and a cancelled run saves nothing** (`actions/cache` saves only when the job succeeds). Let it
  finish once and every branch and PR restores from `master` afterwards; the every-3-days schedule then
  keeps the entries from being evicted, which GitHub does after 7 days unused. A PR's own rebuild never
  helps the NEXT PR (it is scoped to that PR), so after an eviction every PR would pay again until
  something ran on `master` --- which is why **`warm-caches.yml` builds on every push to `master`**
  (every merge), re-saving the caches there. Its `os` / `python-version` must stay equal to
  `test-workflow.yml`'s defaults; change one and change both. Two more ways the schedule can stop
  keeping things warm: GitHub **disables scheduled workflows after 60 days with no repository
  activity**, and a merge-free stretch longer than 7 days with the schedule disabled evicts `master`'s
  entries --- then run Testers on `master` by hand once before opening PRs.

## cibuildwheel's Linux container gets a COPY of the project

Not a mount. So a cache restored on the runner does reach the container (and z3 is skipped), but
whatever the container builds does not come back by itself --- the externs cache for the Linux wheels
had nothing to save until `CIBW_BEFORE_ALL_LINUX` copied `cebuild<os>` / `cext<os>` out through
`/host`, where cibuildwheel mounts the runner's filesystem. Anything else a Linux wheel build must hand
back to the runner needs the same. Test a command like that with its `${{ }}` expressions filled in
and `bash -n` before shipping it: that is what caught `$folderubuntu-latest` --- a variable named
`folderubuntu`, where `${folder}ubuntu-latest` was meant.

## A release, and what it costs

Measured and read off `python-publish.yml` on 2026-09-18; the wheel half has never run yet.

- **Runners:** `ubuntu-latest`, `ubuntu-24.04-arm`, `windows-latest`, **`macos-15-intel`**, `macos-latest`.
  It said `macos-13` until GitHub removed that image outright --- a job asking for a removed label never
  gets a runner, and nothing in the repo says so. Check
  [actions/runner-images](https://github.com/actions/runner-images) before trusting any runner label here.
- **No musllinux wheels (`CIBW_SKIP: "*-musllinux_*"`).** cibuildwheel builds each Linux platform in its
  own container from a fresh copy of the project, one after another, and the step that copies z3 back out
  to the runner (so the cache can save it) hands the musllinux container the MANYLINUX z3 --- it skips
  building its own and links glibc into a musl wheel. A restored cache does the same. Shipping musllinux
  needs per-libc `cebuild`/`cext` folders first (suffix them with `$AUDITWHEEL_PLAT` inside the container).
- **A release runs on its TAG**: it can restore `master`'s caches but saves only for that tag, and no tag
  can read another's. So the wheels' z3 has to be saved ON `master`, and since 2026-09-18 it is:
  **`.github/actions/wheel-externs`** is the ONE definition of that build, used by both
  `python-publish.yml`'s wheel jobs and `warm-caches.yml`'s `wheel-externs` job --- which runs **only by
  hand** (the maintainer's call: the wheels are needed only to publish, so merges do not pay for five
  runners). With a warm `master` a release skips z3 on every runner --- roughly 30-45 minutes instead of
  1.5-2.5 hours; cold, it is still the latter. **Run Warm Caches by hand before a release**; an entry
  unused for 7 days is evicted, so a release more than a week after the last warm-up is cold again. A
  publish started by hand ON `master` (python-publish's workflow_dispatch) also saves on `master`; one
  started by a GitHub release saves only for its tag.
  - **The pinned image list has a `# <date>` comment after every entry**, which `configparser` keeps
    unless given `inline_comment_prefixes` --- the first Warm Caches run died on
    `docker: invalid reference format` for exactly that. The action now refuses anything but a clean
    `image@sha256:<64 hex>` reference, with an error that says so.
  - **The macOS wheels are the first libc++ build this project has had**, and libc++ collides with
    vendored code libstdc++ and MSVC never did: Compressonator's `common_def.h` does
    `#define __local const`, and libc++ (Xcode 26 SDK) has a function of that name in `<algorithm>`,
    `<iterator>`, `<deque>` and `<ranges>`, so every one of those included after it fails with
    `expected unqualified-id` at `_Traits::__local(...)` (2026-09-18). Fixed by ORDER, not by editing
    the submodule: `core/cmake/CompressonatorPrelude.h` is force-included ahead of every vendored
    source and reads those headers before the macro exists. It also carries the older `stdint.h`
    repair, and it has to stay the ONE `-include`: CMake de-duplicates compile options, so a second
    `add_compile_options(-include x.h)` loses its `-include` and hands `x.h` to the compiler as
    another input file (`cannot specify '-o' with '-c' ... with multiple files` -- proved on a
    throwaway project). Nothing on Windows or Linux can reproduce the original error, so the next
    macOS wheel run is its only test; a new libc++ header naming `__local` goes in the prelude.
  - **The Linux wheels need OpenSSL, and a CA bundle found at RUN time.** The vendored curl will
    not configure without OpenSSL's headers, which the manylinux image lacks
    (`CIBW_BEFORE_ALL_LINUX: dnf install -y openssl-devel`; auditwheel then bundles libssl and
    libcrypto). The quieter half: curl's CMake records the BUILD machine's CA bundle
    (`CURL_CA_BUNDLE` in its cache), which in that image is AlmaLinux's
    `/etc/pki/tls/certs/ca-bundle.crt` -- absent on Debian and Ubuntu, where every download would
    fail with error 77 even with `/etc/ssl/certs` as a CAPATH (reproduced against the vendored curl,
    2026-09-18). `FileDownload` therefore sets `CURLOPT_CAINFO` from `SSL_CERT_FILE` /
    `CURL_CA_BUNDLE` or the first well-known bundle that exists, on every OS but Windows (Schannel).
    **A wheel that builds and imports can still be unable to download anything** --- test a
    download on a DIFFERENT distro from the one it was built on.
  - **cibuildwheel is given the PACKAGE directory and run from the repo ROOT**:
    `python -m cibuildwheel "Anime Game Remap (for all users)/api" --output-dir wheelhouse`. With no
    argument it looks for `pyproject.toml` in the current directory and fails at once with `Could not
    find any of {setup.py, setup.cfg, pyproject.toml} at root of package` (the first pre-release,
    2026-09-18 -- the argument had never been there). Do not `cd` into the API instead: on Linux
    what cibuildwheel copies into the container as `/project` is the CURRENT directory, and the
    `/project/cext<os>` z3 and `/project/.sccache-bin` paths are the repo root's.
  - **Linux z3 is built on the runner, inside the manylinux image cibuildwheel uses**, with the workspace
    mounted at `/project` --- where cibuildwheel puts its copy of the project, so the paths z3 recorded
    at install time hold. The image is read from the pinned cibuildwheel's own
    `pinned_docker_images.cfg`, and cibuildwheel's version lives in ONE file,
    `.github/cibuildwheel-requirements.txt` (4.2.1). `CIBW_BEFORE_ALL_LINUX` builds nothing any more.
  - **The cache key** is runner + a "flavour" + the z3 submodule commit. The flavour is the manylinux
    image digest on Linux and `mac<target>` on macOS, so bumping cibuildwheel or the target misses
    rather than restoring a z3 built for something else.
  - **macOS targets 14.0, in two places that must agree**: the z3 build and the wheels'
    `MACOSX_DEPLOYMENT_TARGET`. Built on the runner's own macOS with no target, z3 would need macOS 15,
    and delocate refuses a library needing a newer macOS than the wheel claims. **The floor is set by
    z3, not by us**: z3 4.17 uses `std::format`, whose float formatting calls libc++'s
    `std::to_chars(double)`, which Apple ships only from macOS 13.3 --- below that the z3 build fails
    with `'to_chars' is unavailable: introduced in macOS 13.3` (the first Warm Caches run, 2026-09-18,
    at 11.0). 14.0 rather than 13.3 because a wheel's tag keeps only the major version on macOS 11+,
    so a 13.3 wheel is `macosx_13_0` and pip would install it on 13.0-13.2, which cannot load it. Do
    not "fix" a future availability error with `_LIBCPP_DISABLE_AVAILABILITY`: it compiles and then
    fails to LOAD on the older macOS. A z3 bump can raise this floor again; the error names the version.
  - `checkWorkflowWiring.py` fails if the two workflows' runner lists or macOS targets differ, or if the
    wheels are built for a different target than z3. **None of this has run yet**: the first Warm Caches
    run is the test, and the Linux build inside the manylinux image is the step with no local precedent.
  - **The Linux and macOS wheels compile through sccache** (2026-09-18) --- its LOCAL disk cache, set up
    by `mozilla-actions/sccache-action` and switched on with `CMAKE_ARGS=-DAGREMAP_SCCACHE=ON` in
    `CIBW_ENVIRONMENT_*` (scikit-build-core adds `CMAKE_ARGS` to `pyproject.toml`'s own `cmake.args`).
    The C++ core is identical for every python version, so it compiles once per runner and only the
    bindings compile per version. **With `CIBW_BUILD: "cp312-*"` that is one wheel per runner and saves
    nothing yet** --- it is in place for widening. Linux builds in cibuildwheel's container, so the
    action's static musl binary is copied into the project (`.sccache-bin/`) and put on `PATH`;
    `CIBW_BEFORE_BUILD_LINUX` / `_MACOS` print `--show-stats` before each wheel. **Not Windows**:
    scikit-build-core uses the Visual Studio generator there, which ignores a compiler launcher, and
    `AGREMAP_SCCACHE` would still turn the precompiled headers off --- to add it, force
    `CMAKE_GENERATOR=Ninja` with the MSVC environment set up first. Nothing is shared across releases
    (tag-scoped caches).
- **Trimming z3's build is not the fix:** by ninja's own totals the default target is 888 steps and
  `libz3` alone 872 --- `test-z3` (1017) is not in the default build at all.
- **Widening `CIBW_BUILD` to every python** multiplies the per-wheel extension build (~15 min each, LTO on,
  no sccache) by the version count, twice over on Linux if musllinux returns --- close to GitHub's
  **6-hour per-job limit**. Split the wheel jobs by python version, or compile through a local sccache
  inside the container, before widening it.

## Badges

A shields.io (or GitHub) workflow badge reports **a whole workflow run**, never one job. Both testers
are jobs of `tests.yml`, so the READMEs and `Docs/src/index.rst` carry ONE badge, labelled
"Unit/Integration Tests", for `tests.yml` with `branch=master` --- the scheduled runs on released code,
not whichever pull request ran last. Separate per-tester badges would need each tester job to publish
a shields *endpoint* JSON (a Gist and a token secret, which only the maintainer can create);
splitting the testers back into two workflows would build the API twice per pull request.

**"No status" on that badge means no run of `tests.yml` on `master` yet**, not a broken badge: pull
request runs are recorded under the PR's branch. A manual run on `master` (Actions -> Testers -> Run
workflow) fills it; the schedule keeps it current.

## Reproducing CI locally, and seeing GitHub from here

- **Run the suites from a CI-shaped copy on ext4**, not from the checkout on `/mnt/e`: NTFS returns a
  folder listing sorted and ext4 does not, and that difference alone failed 8 Integration Tester tests
  on the first run. Testing's "The first CI run" has the `git archive` recipe --- tracked files, LF
  endings, exactly what a runner checks out.
- **This machine cannot reach GitHub through git**: `git fetch` and `git push` fail with *"SSL
  certificate problem: unable to get local issuer certificate"* from a TLS-inspecting proxy. Do not
  disable `http.sslVerify` to get round it; the maintainer pushes. The **WebFetch** tool does reach
  public GitHub pages, which answers most questions about the remote --- whether a workflow has runs on
  a branch (`.../actions/workflows/tests.yml?query=branch%3Amaster`), whether a branch or path exists
  (`.../tree/master/<path>`), whether an old URL still redirects. The local `origin/*` refs are stale
  for the same reason, so trust the page over `git branch -r`.
- **The default branch is `master`, renamed from `nhok0169` on 2026-09-18.** GitHub's redirect for a renamed branch covers the WEB pages
  (`/tree/nhok0169/...`, `/blob/...`) and **not raw file downloads**: `github.com/.../raw/nhok0169/...`
  returns **404** (checked 2026-09-18), while `.../raw/master/...` redirects to
  `raw.githubusercontent.com` and serves the file. Every package released before the rename downloads its
  assets from `.../raw/nhok0169/Data/Mod%20Downloads`, so **their downloads broke with the rename**, and
  so did `master`'s own until the URL change in `DownloadTools.cpp` / `FileDownloadData.py` reached it ---
  the first CI run on `master` after the rename failed 5 Integration Tester tests on nothing but 404s.
  What restores the released versions is a branch named **`nhok0169`** holding `Data/Mod Downloads`;
  it can stay frozen at the rename point, since an old release only asks for files it already knew
  about. **Check a URL in the exact form the product fetches** --- a `tree/` page redirecting proved
  nothing about `raw/`, and that one untested step is what this paragraph once got backwards. See
  [Overview](../Overview/CLAUDE.md). **A CI failure that is only missing `*RemapDL*` files, with
  `FileDownload::download: ... 404` in the log, is this --- a download URL --- not the fix.**
