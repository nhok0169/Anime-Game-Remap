# AG Remap's API Builder

Compiles [the API](https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api)'s
C++ core, its pybind11 bindings and its Cython extensions, and installs the compiled modules
(`core`, `CyDictTools`, `CyListTools`, `CyHashTools`, `CyAlgo`) into the API's Python package at
`Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2`, so that `import FixRaidenBoss2` works from the repo.

It is also the first stage of the [CI Pipeline](https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/CIPipeline).

<br>

## Requirements

| Tool | Notes |
| --- | --- |
| A C++23 compiler | **Windows**: Visual Studio's MSVC. **Linux**: GCC 13 or newer |
| [CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/) | On Windows, Visual Studio's *C++ CMake tools for Windows* component includes both |
| The Python packages in [requirements.txt](requirements.txt) | `pybind11==3.0.4`, `cython`, `numpy` and `pybind11-stubgen` |
| [Doxygen](https://www.doxygen.nl/) 1.17.0 | Only for the `-d` option |
| The API's git submodules | `git submodule update --init --recursive` |

```bash
pip install -r requirements.txt
```

> [!IMPORTANT]
> On Windows, run the API Builder from a shell where Visual Studio's `vcvarsall.bat x64` has already been called.
> The build always uses the Ninja generator, which needs the MSVC environment set up in the same shell.

> [!IMPORTANT]
> The Python that runs the API Builder should be the same Python you will later use to run the API.
> CMake builds the modules against the Python it finds, and they only load in that version of Python.

<br>

## How To Run
On [CMD](https://www.google.com/search?q=how+to+open+cmd+in+a+folder&oq=how+to+open+cmd), enter

```bash
python3 main.py -pb -pi
```

The first run builds and installs [Z3](https://github.com/Z3Prover/z3), one of the API's external libraries,
which takes a while. `-pb` and `-pi` skip themselves once Z3 is already built and installed, so afterwards a plain
`python3 main.py` is enough to rebuild the API.

<br>

### What a run does, in order

1. Updates the credits of the API's source files (only with `-c`)
2. Removes the prebuild / preinstall / build folders you asked to remove (`-p`, `-pir`, `-b`)
3. Deletes the previously installed `.pyd` / `.so` modules from the API folder (skipped with `-i`)
4. Builds Z3 into `cebuild/z3` (only with `-pb`, and only if that folder does not exist yet)
5. Installs Z3 into `cext/z3` (only with `-pi`, and only if that folder does not exist yet)
6. Configures, builds and installs the API in `cbuild` (skipped with `-s`)
7. Regenerates the API's documentation files (only with `-d`)

The `cebuild`, `cext` and `cbuild` folders are created at the root of the repo, and are not tracked by git.

<br>

## Options

| Option | Description |
| --- | --- |
| `-h`, `--help` | Shows the help message and exits |
| `-e str`, `--env str` | The environment to build for, passed to CMake as the API's `BUILD_MODE`. See [Environments](#environments) |
| `-pb`, `--makePreBuild` | Builds the external libraries (Z3) into the prebuild folder. Skipped when that folder already exists |
| `-pi`, `--makePreInstall` | Installs the external libraries (Z3) into the preinstall folder. Skipped when that folder already exists |
| `-s`, `--skipBuild` | Skips compiling and installing the API |
| `-i`, `--installKeep` | Keeps the previously installed `.pyd` / `.so` modules instead of deleting them before the build |
| `-f str`, `--installFolder str` | Where to install the compiled modules. By default, the API's Python package folder |
| `-d`, `--addDocs` | Regenerates the documentation files: `core.pyi` (the API's Python stubs, made with `pybind11-stubgen`) and `core/xml` (the C++ core's [Doxygen](https://www.doxygen.nl/) output). Needs Doxygen 1.17.0 on `PATH` |
| `-c`, `--addCredits` | Updates the credits boilerplate of the API's source files (Python, Cython, pybind11 and the C++ core). Only the text between a file's `##### Credits` and `##### EndCredits` comments changes, and files without those comments are left alone |
| `-bl str`, `--buildLocation str` | The folder the build folder is created in. By default, the `AGREMAP_BUILD_LOCATION` environment variable if it is set, otherwise the root of the repo. Useful when the repo is on a slow drive: keeping the build folder on a fast local drive can make a rebuild several times faster |
| `-bs str`, `--buildSuffix str` | A suffix added to the build folder's name (eg. `-bs lin` gives `cbuildlin`) |
| `-ps str`, `--prebuildSuffix str` | A suffix added to the prebuild folder's name (`cebuild`) |
| `-pis str`, `--preinstallSuffix str` | A suffix added to the preinstall folder's name (`cext`) |
| `-b str`, `--buildRemove str` | Removes a build folder before building. See [Removing folders](#removing-folders) |
| `-p str`, `--prebuildRemove str` | Removes a prebuild folder before building. See [Removing folders](#removing-folders) |
| `-pir str`, `--preinstallRemove str` | Removes a preinstall folder before building. See [Removing folders](#removing-folders) |

> [!NOTE]
> A suffix name cannot contain whitespace or slashes.

<br>

### Environments

| `--env` | CMake `BUILD_MODE` | What it builds |
| --- | --- | --- |
| `dev` *(default)* | `python_dev` | The whole API for local development. Link-time optimization is off, so a small change rebuilds quickly |
| `cibuildwheel` | `cibuildwheel` | The whole API for a release wheel. Link-time optimization is on, and the modules link against `Python::Module` |
| `core` | `core_sdk` | Only the C++ core, as an SDK for other C++ projects: the library and the libraries it is built with, its headers, and a CMake package (`find_package(AGRemapCore)`). The pybind11 bindings and the Cython extensions are not built. It installs to `csdk` at the root of the repo by default, leaves the Python package's compiled modules alone, and ignores `-d` |

> [!NOTE]
> CMake remembers some settings from the first time a build folder is configured (link-time optimization among them),
> so give each environment its own build folder: eg. `python3 main.py -e cibuildwheel -bs Wheel`.

To use the SDK from your own CMake project, point `CMAKE_PREFIX_PATH` at both the SDK and the Z3 install the
API Builder made (the SDK finds Z3 rather than carrying it):

```cmake
# cmake -DCMAKE_PREFIX_PATH="<repo>/csdk;<repo>/cext/z3" ...
find_package(AGRemapCore CONFIG REQUIRED)
target_link_libraries(yourTarget PRIVATE AGRemapCore::AGRemapCore)
```

At run time, the DLLs in `csdk/bin` and `cext/z3/bin` need to be next to your program or on `PATH`.

<br>

### Removing folders

`-b`, `-p` and `-pir` take one of three forms:

| Value | Removes |
| --- | --- |
| `/` | The folder with no suffix (eg. `cbuild`) |
| `someName/` | The folder with that suffix (eg. `cbuildsomeName`) |
| `*` | Every folder of that kind |

```bash
python3 main.py -b /
```

> [!NOTE]
> A build folder that is a symbolic link or a directory junction has its contents deleted and the link itself kept,
> so the build tree stays wherever the link points.

<br>

### Extra CMake options

Any extra options for CMake's configure step can be passed through the `AGREMAP_CMAKE_ARGS` environment variable.
The value is split like a shell command line, so quote a value that contains spaces, and write paths with forward slashes.

```bash
AGREMAP_CMAKE_ARGS="-DAGREMAP_SCCACHE=ON" python3 main.py
```

> [!NOTE]
> CMake remembers the path of its build folder, so moving an existing build folder by hand breaks it.
> To move the build, point `--buildLocation` at a new folder and let the next build configure a fresh one there.
