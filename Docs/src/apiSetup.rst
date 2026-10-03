.. role:: raw-html(raw)
    :format: html

API Setup
=========

Installing
----------

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
---------

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

How To Use
----------
- For some simple examples of using the API, visit :doc:`API Examples <apiExamples>`
- For a full reference of the Python API, visit :doc:`Python API Reference <api>`
- For the C++ library underneath it, visit :doc:`C++ Core API Reference <coreAPI>`

.. _Pypi: https://pypi.org/project/AnimeGameRemap/
