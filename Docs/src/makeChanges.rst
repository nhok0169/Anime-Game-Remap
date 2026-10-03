.. role:: raw-html(raw)
    :format: html

How to Make Changes to the Project
==================================

.. tip::
    We provide :doc:`AI support <aiSupport>` for automating most of the steps below

:raw-html:`<br />`
:raw-html:`<br />`

1. Install the Required Tools
-----------------------------
You would need to install the following tools to compile or run the project:

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Name
     - Description
   * - `Git`_
     - To clone the project and its submodules
   * - `Python`_
     - | Version 3.9 or newer. Runs the build tools, the tests and the project itself
       |
       | Use the same Python for building and for running the project afterwards, since the compiled modules only load in the
         Python they were built against
   * - C++ compiler
     - | A compiler that supports C++23, to compile the C++ core, the Python bindings and the Cython extensions
       |
       | **Windows**: `Visual Studio Build Tools`_ with the *Desktop development with C++* workload and the
         *C++ CMake tools for Windows* component
       | **Linux**: GCC 13 or newer
   * - `CMake`_
     - | Configures and runs the build
       |
       | On Windows, the *C++ CMake tools for Windows* component of Visual Studio already includes it
   * - `Ninja`_
     - | The build system CMake generates for
       |
       | On Windows, the *C++ CMake tools for Windows* component of Visual Studio already includes it
   * - Python packages
     - | `pybind11`_ (exactly version 3.0.4), `Cython`_, `NumPy`_ and `pybind11-stubgen`_, used by the build
       |
       | Install them with ``pip install -r Tools/APIBuilder/requirements.txt``
   * - `Doxygen`_
     - | *Optional*. Version 1.17.0. Only needed when you change documentation in the C++ core or the Python bindings
         (see the ``-d`` option in step 4)

:raw-html:`<br />`
:raw-html:`<br />`

2. Clone the Project
--------------------
Fork the `Github repo`_ , then clone your fork **together with its submodules**
(the C++ libraries the API depends on are git submodules under ``api/extern``):

.. code-block:: bash

    git clone --recurse-submodules <url of your fork>

If you already have a clone without the submodules, run ``git submodule update --init --recursive``

Create a new branch for your changes off the ``master`` branch.

:raw-html:`<br />`
:raw-html:`<br />`

3. Make your Changes
--------------------
AG Remap has 3 different types of builds:

#. `The API`_ (The source code for the project: a C++ core, its Python bindings and some Cython extensions, under a Python package)
#. `The API Mirror`_ (A mirror to the API)
#. `The Script`_ (A compatible script for users who do not know how to use Pypi or any other Python package manager)

:raw-html:`<br />`

You would want to make your changes within `the API`_ . All the other builds are generated using other tools within the project.

Within `the API`_, the source code is split into:

* ``src/cpp/core``: the C++ core, where most of the logic and all the data for the remaps live (see :doc:`coreAPI`)
* ``src/cpp/py``: the pybind11 bindings that expose the C++ core to Python
* ``src/cy``: the Cython extensions
* ``src/py/FixRaidenBoss2``: the Python package (see :doc:`api`)

.. note::
    If you change the source code of `the Script`_ itself, its source is at `Tools/Script`_

:raw-html:`<br />`
:raw-html:`<br />`

4. Compile your Changes
-----------------------
Since the API contains compiled code, you need to build it before you can run or test your changes.

You can do this by running the `API Builder`_.

.. code-block:: bash

    cd Tools/APIBuilder
    python3 main.py -pb -pi

The ``-pb`` and ``-pi`` options build and install the external libraries (the first time takes a while) and skip themselves
once they are done, so afterwards a plain ``python3 main.py`` is enough. The build installs the compiled modules into
``api/src/py/FixRaidenBoss2``.

.. note::
    On Windows, run the API Builder from a shell where Visual Studio's ``vcvarsall.bat x64`` has already been called.

    Add the ``-d`` option if you changed any documentation in the C++ core or the Python bindings. It regenerates the
    committed ``core/xml`` and ``core.pyi`` files that the docs are built from.

:raw-html:`<br />`

To update the other builds (the Script and the API Mirror) with your changes, run the `CI Pipeline`_ (its first stage runs the API Builder)

Steps on how to run the CI Pipeline are here:
https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/CIPipeline

:raw-html:`<br />`
:raw-html:`<br />`

5. Test your Changes
--------------------
Once you are done making your changes, you would want to test whether your new changes work properly.
This step involves both running tests and making new test cases.

In general, AG Remap has 3 layers of QA testing, 2 automated tests and 1 manual test. These tests are:

#. `Unit tests`_
#. Acceptance tests
#. `Integration tests`_

:raw-html:`<br />`

I. Unit Tests
~~~~~~~~~~~~~
This is the first line of defense to see whether your changes may break other modules within the software.

For steps on how to run the Unit tester, see the link below:
https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Unit%20Tester

.. important::
    If you have created a new module in the software, it is recommended that you build
    unit tests since:

    #. to see whether your module works by itself
    #. your tests are used for future `regression testing`_ against your module

    The unit tests are built using Python's `unittest`_ library. You can check out `the different unit tests here`_ for 
    how to make a unit tests within the project

.. note::
    Code in the C++ core that has no Python binding cannot be reached by the Unit tester. For such code,
    write a standalone C++ test at `core/tests`_ (one ``<Name>_test.cpp`` file per test).
    These tests are not built automatically, so also rebuild the tests that mention any C++ class you changed.

:raw-html:`<br />`

II. Acceptance Tests
~~~~~~~~~~~~~~~~~~~~
This test is where you verify whether your changes actually work in the game.

#. Go to `Tools/Script`_ , which runs the software against the API in your copy of the repository
#. Run ``python3 main.py -s <the Mods folder of 3dmigoto>``
#. Check the fixed mods in the game

.. tip::
    Here are some useful command options when running the build:
    
    * ``-t str``: Used to filter which types of mods to fix. Enter a list of character names, seperated by a comma (,)
    * ``-s str``: Sets the folder where the software first scans for mods
    * ``-ft str``: Forces the software to assume the mod type for the mod being fixed. Use this option only if you are confident about the type for the mod.

    You can check out :doc:`commandOpts` for more info about what command options to supply

:raw-html:`<br />`

III. Integration Tests
~~~~~~~~~~~~~~~~~~~~~~~
This is the final test, once you are confident your changes work in the game.

These tests verify whether the overall features of the softare are working properly by running the software against
different types of folder/file structures

For steps on how to run the Integration tester, see the link below:
https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Integration%20Tester

You can check out the `specific integration tests here`_

.. warning::
    This test may take a while since it performs a lot of file manipulation

:raw-html:`<br />`
:raw-html:`<br />`

6. Commit your Changes
----------------------
When all tests are clear, commmit your changes using ``git``, then push your changes back to your forked Github repo.

After send a `Pull Request (PR)`_ to merge your new changes from your fork back to the ``master`` branch of the `AG Remap repo`_
We will do a code review on your PR.

.. note::
    When you open a Pull Request, Github will trigger a CI pipeline that builds the API and automatically runs your changes 
    against the `unit tests`_ and the `integration tests`_ to make sure your changes have met the test requirements.

:raw-html:`<br />`
:raw-html:`<br />`

7. Merge your Changes
---------------------
Once your PR is approved, we will merge your changes back to the `AG Remap repo`_


.. _Github repo: https://github.com/nhok0169/Anime-Game-Remap
.. _AG Remap repo: https://github.com/nhok0169/Anime-Game-Remap
.. _The API: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api
.. _The API Mirror: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/apiMirror
.. _The Script: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/script%20build/src/FixRaidenBoss2
.. _regression testing: https://en.wikipedia.org/wiki/Regression_testing
.. _CI Pipeline: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/CIPipeline
.. _API Builder: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/APIBuilder
.. _Tools/Script: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/Script
.. _core/tests: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/tests
.. _CMake: https://cmake.org/
.. _Ninja: https://ninja-build.org/
.. _Doxygen: https://www.doxygen.nl/
.. _Git: https://git-scm.com/
.. _Python: https://www.python.org/downloads/
.. _Visual Studio Build Tools: https://visualstudio.microsoft.com/downloads/
.. _pybind11: https://pybind11.readthedocs.io/
.. _Cython: https://cython.org/
.. _NumPy: https://numpy.org/
.. _pybind11-stubgen: https://pypi.org/project/pybind11-stubgen/
.. _Unit tests: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Unit%20Tester
.. _Integration tests: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Integration%20Tester
.. _unittest: https://docs.python.org/3/library/unittest.html
.. _the different unit tests here: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Unit%20Tester/UnitTester/Tests
.. _specific integration tests here: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Testing/Integration%20Tester/IntegrationTester/Tests
.. _Pull Request (PR): https://github.com/nhok0169/Anime-Game-Remap/pulls