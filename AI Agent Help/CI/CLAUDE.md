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

No workflow runs on a plain push: the testers cost enough that a pull request, the schedule and each
publish are the gates.

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

## Caches: one works, one cannot

- **`externs-...` works, and is the one that matters.** `cebuild<os>` / `cext<os>` hold the built z3
  (~45 minutes from cold). The key is the z3 SUBMODULE's commit, and `APIBuilder` skips `-pb` / `-pi`
  whenever those folders exist, so a hit removes the cost outright. The build step prints
  `externs cache hit: true|false`.
- **`apibuild-...` (`cbuild<os>`) restores and saves NOTHING, and never could.** Ninja decides what is
  stale by modification time, and `actions/checkout` writes every source file --- submodules included
  --- with the time of the checkout, so every source is newer than every restored object and all ~790
  steps rebuild. The tell is in the log: a CMake configure of about a second (the restored
  `CMakeCache.txt` skipping curl's checks) followed by `[1/790]` compiling utf8proc. It also spends the
  repository's 10 GB of cache, where the least-recently-used entry is evicted --- which can be the z3
  one. **Open as of 2026-09-18**, with a proposed fix waiting on the maintainer: compile through sccache
  (content-keyed, so a fresh checkout does not matter; `AGREMAP_SCCACHE=ON`, which also turns the PCH
  off since sccache will not cache it) with `mozilla-actions/sccache-action`, give `APIBuilder` a way to
  pass CMake options (an `AGREMAP_CMAKE_ARGS` environment variable next to `AGREMAP_BUILD_LOCATION`),
  and delete the `cbuild` cache step. Do not "fix" it by caching harder.

## cibuildwheel's Linux container gets a COPY of the project

Not a mount. So a cache restored on the runner does reach the container (and z3 is skipped), but
whatever the container builds does not come back by itself --- the externs cache for the Linux wheels
had nothing to save until `CIBW_BEFORE_ALL_LINUX` copied `cebuild<os>` / `cext<os>` out through
`/host`, where cibuildwheel mounts the runner's filesystem. Anything else a Linux wheel build must hand
back to the runner needs the same. Test a command like that with its `${{ }}` expressions filled in
and `bash -n` before shipping it: that is what caught `$folderubuntu-latest` --- a variable named
`folderubuntu`, where `${folder}ubuntu-latest` was meant.

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
- **The default branch is `master`, renamed from `nhok0169` on 2026-09-18.** Never create a branch
  named `nhok0169` again: every released package downloads its assets from
  `.../raw/nhok0169/Data/Mod%20Downloads`, which works only through GitHub's redirect for a renamed
  branch (checked: it redirects), and a new branch of that name would end it. See
  [Overview](../Overview/CLAUDE.md).
