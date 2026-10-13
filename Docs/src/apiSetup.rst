.. role:: raw-html(raw)
    :format: html

API Setup
=========

The library can be used from Python, or its C++ core can be used directly from your own C++ project.

:raw-html:`<br />`

Python
------

Installing
~~~~~~~~~~

The library is published on `Pypi`_ as ``AnimeGameRemap``. To install the library, run the following command in the terminal:

:raw-html:`<br />`

.. parsed-literal::

    python3 -m pip install -U "AnimeGameRemap"

On Windows, use ``py -3`` in place of ``python3``:

.. parsed-literal::

    py -3 -m pip install -U "AnimeGameRemap"

:raw-html:`<br />`

.. note::
    The library contains compiled code, so ``pip`` needs a prebuilt wheel for your Python version and operating
    system. Version |release| ships wheels for **CPython 3.9 - 3.15** on Windows (x86-64), Linux (x86-64 and ARM64) and
    macOS 14+ (Intel and Apple Silicon).

.. note::
    Sometimes, you may want to test out a pre-release version of the library. Add the ``--pre`` flag to the commands above to
    install a pre-release, if available. 

:raw-html:`<br />`
:raw-html:`<br />`

Importing
~~~~~~~~~

.. code-block:: python

    import AnimeGameRemap as AGR

    iniFile = AGR.IniFile("MyMod.ini")
    iniFile.parse()
    iniFile.fix()

:raw-html:`<br />`

.. note::
    ``AnimeGameRemap`` re-exports the public classes and functions of the ``FixRaidenBoss2`` package, which is the
    package that actually contains the library (``AnimeGameRemap`` depends on it and installs it for you). That is
    why the :doc:`Python API Reference <api>` lists every class under ``FixRaidenBoss2``: ``AGR.IniFile`` and
    ``FixRaidenBoss2.IniFile`` are the same class. You can always ``import FixRaidenBoss2`` directly instead.

:raw-html:`<br />`
:raw-html:`<br />`

C++
---

The library's C++ core can also be used on its own, as a C++ library. It is not published as a package, so you build it
from the source code with the `API Builder`_, which installs it as an SDK: the library, its headers and a CMake package.

:raw-html:`<br />`

Requirements
~~~~~~~~~~~~

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Tool
     - Notes
   * - C++ compiler
     - | A compiler that supports C++23
       |
       | **Windows**: Visual Studio's MSVC
       | **Linux**: GCC 13 or newer
   * - `CMake`_ and `Ninja`_
     - CMake 3.21 or newer. On Windows, Visual Studio's *C++ CMake tools for Windows* component includes both
   * - `Python`_
     - | Version 3.9 or newer, only to run the API Builder
       |
       | No Python packages are needed to build the C++ core
   * - `Git`_
     - To clone the project together with its submodules (the C++ libraries the core is built with)

.. important::
    On Windows, run the commands below from a shell where Visual Studio's ``vcvarsall.bat x64`` has already been called.
    The build uses the Ninja generator, which needs the MSVC environment set up in the same shell.

:raw-html:`<br />`
:raw-html:`<br />`

Building
~~~~~~~~

.. code-block:: bash

    git clone --recurse-submodules https://github.com/nhok0169/Anime-Game-Remap.git
    cd Anime-Game-Remap/Tools/APIBuilder
    python3 main.py -e core -pb -pi

The first build also builds `Z3`_, one of the core's external libraries, which takes a while. Later builds skip it.

The build installs everything into the ``csdk`` folder at the root of the repo, and Z3 into ``cext/z3``:

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Folder
     - Contents
   * - ``csdk/include``
     - The core's headers (``AGRemapCore/...``), and the third-party headers they include
   * - ``csdk/lib``
     - The core's static library (``AGRemapCore``) and the libraries it is built with
   * - ``csdk/bin``
     - The shared libraries (DLLs) needed when your program runs
   * - ``csdk/lib/cmake/AGRemapCore``
     - The CMake package, for ``find_package(AGRemapCore)``
   * - ``cext/z3``
     - Z3, which the CMake package finds rather than carries

.. tip::
    Add ``-f <folder>`` to install the SDK somewhere else. The `API Builder`_'s README lists all of its options.

:raw-html:`<br />`
:raw-html:`<br />`

Using it in a CMake project
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake
    :caption: CMakeLists.txt

    cmake_minimum_required(VERSION 3.21)
    project(MyRemapTool LANGUAGES CXX)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    find_package(AGRemapCore CONFIG REQUIRED)

    add_executable(MyRemapTool main.cpp)
    target_link_libraries(MyRemapTool PRIVATE AGRemapCore::AGRemapCore)

:raw-html:`<br />`

.. code-block:: cpp
    :caption: main.cpp

    #include "AGRemapCore/model/files/IniFile.h"

    int main() {
        AGRemapCore::IniFile iniFile("MyMod.ini");
        iniFile.parse();
        iniFile.fix();
        return 0;
    }

:raw-html:`<br />`

Point ``CMAKE_PREFIX_PATH`` at both the SDK and Z3 when configuring your project:

.. code-block:: bash

    cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="<repo>/csdk;<repo>/cext/z3"
    cmake --build build

:raw-html:`<br />`

.. note::
    When your program runs, the shared libraries in ``csdk/bin`` and ``cext/z3/bin`` need to be next to it or on your ``PATH``.

:raw-html:`<br />`
:raw-html:`<br />`

How To Use
----------
- For some simple examples of using the API, visit :doc:`API Examples <apiExamples>`
- For a full reference of the Python API, visit :doc:`Python API Reference <api>`
- For the C++ library underneath it, visit :doc:`C++ Core API Reference <coreAPI>`

.. _Pypi: https://pypi.org/project/AnimeGameRemap/
.. _API Builder: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/APIBuilder
.. _CMake: https://cmake.org/
.. _Ninja: https://ninja-build.org/
.. _Python: https://www.python.org/downloads/
.. _Git: https://git-scm.com/
.. _Z3: https://github.com/Z3Prover/z3
