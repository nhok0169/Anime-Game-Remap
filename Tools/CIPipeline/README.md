# AG Remap's CI Pipeline

Build system pipeline used to transform the source code into deliverible code for the user

<br>

### Order of Processes to be Run on Pipeline
1. [API Builder](https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/APIBuilder) (compiles the API's binaries and its documentation)
2. [Script Builder](https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/ScriptBuilder)
3. [API Mirror Builder](https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/APIMirrorBuilder)
4. [Tool Stats Updater](https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/ToolStatsUpdater)

<br>

> [!IMPORTANT]
> The first stage compiles C++ and Cython, so it needs what any API build needs: `cmake`, `ninja`
> and `doxygen` on `PATH`, and on Windows the MSVC environment already initialized in the same
> shell (`vcvarsall.bat x64`). It is also by far the longest stage.
>
> Because it runs the API Builder with `-d`, it regenerates the **tracked** `core/xml` and
> `core.pyi`. Expect those in `git status` after a pipeline run, and see the API Builder's notes on
> when to keep them.

<br>

## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py
```

<br>

## Options

### `--env/-e`

What environment to build the deliverables for. The same environments as the
[Script Builder](https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/ScriptBuilder)'s,
since that is the stage the environment reaches:

| Env | How the compiled script reaches the API | Options the script gains |
| --- | --- | --- |
| `dev` *(default)* | a path on this machine, relative to the compiled script | none |
| `prod` | downloaded from [pypi](https://pypi.org/project/FixRaidenBoss2/) when the script runs | `--update/-up`, `--preRelease/-pre` |

```bash
python3 main.py --env prod
```

<br>

> [!NOTE]
> The environment is only handed down to the stages that take one, which today is the Script
> Builder. The API mirror and the software metadata are the same in either environment.
