# AG Remap's Script

The source for [AG Remap's single-file script](https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/script%20build/src/FixRaidenBoss2/AGRemap.py).

<br>

> [!NOTE]
> The script no longer *contains* the API. The API is a C++/Cython/Python project, and a single
> `.py` file cannot carry a compiled extension module -- so the script now only *reaches* the API
> and hands it the command line.

<br>

## How the script reaches the API

Decided when the script is compiled, by the [ScriptBuilder](../ScriptBuilder)'s `--env` option:

| Env | How | Extra options |
| --- | --- | --- |
| `dev` | a path on this machine, relative to the script | none |
| `prod` | downloaded from [pypi](https://pypi.org/project/FixRaidenBoss2/) at runtime | `--update/-up`, `--preRelease/-pre` |

<br>

## How To Run

To run the script straight from this source, against the API in this repo, on
[CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py
```

Every option of the API's own CLI works here, since they are the API's:

```bash
python3 main.py --help
```
