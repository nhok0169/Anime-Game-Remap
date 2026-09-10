# AG Remap's Script Builder

Compiles [the script's source code](../Script) into a [single script](https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/script%20build/src/FixRaidenBoss2/AGRemap.py)

<br>

> [!NOTE]
> Essentially, this tool copies all the necessary information of each module from
> [the script's source code](../Script) and puts them all into
> [the script](https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/script%20build/src/FixRaidenBoss2/AGRemap.py)
> using ***[topological ordering](https://en.wikipedia.org/wiki/Topological_sorting)***

<br>

> [!IMPORTANT]
> This used to compile the ***API's*** source code into the script, back when the API was pure
> python. The API is a C++/Cython/Python project now, and a single `.py` file cannot carry a
> compiled extension module -- so the script no longer *contains* the API, it *reaches* it.
> The topological ordering still happens, but over [the script's own source](../Script).

<br>

## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py
```

<br>

## Options

### `--env/-e`

What environment to build the script for, which decides how the compiled script reaches the API:

| Env | How the script reaches the API | Options the script gains |
| --- | --- | --- |
| `dev` *(default)* | a path on this machine, relative to the compiled script, the way the tools in this repo reach each other | none |
| `prod` | downloaded from [pypi](https://pypi.org/project/FixRaidenBoss2/) when the script runs, the same shape as the API's own `PackageManager` | `--update/-up`, `--preRelease/-pre` |

```bash
python3 main.py --env prod
```

<br>

The script's own options for a `prod` build:

* **`--update/-up`** -- explicitly update the API's package before running. Without it, the package
  is only downloaded when it is not already installed.
* **`--preRelease/-pre`** -- also consider prereleases of the package when downloading it.

<br>

> [!NOTE]
> Those two are registered into the API's *own* command, so they show up in the compiled script's
> `--help` alongside every API option, rather than being parsed separately and left undocumented.
