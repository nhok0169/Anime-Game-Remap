.. role:: raw-html(raw)
    :format: html

API Setup
=========

Installing
----------

The library is published on `Pypi`_ as ``AnimeGameRemap``. These docs describe version |release|; to install
exactly that version:

.. parsed-literal::

    python3 -m pip install -U "AnimeGameRemap==\ |release|\ "

On Windows, use ``py -3`` in place of ``python3``:

.. parsed-literal::

    py -3 -m pip install -U "AnimeGameRemap==\ |release|\ "

.. note::
    The library contains compiled code, so ``pip`` needs a prebuilt wheel for your Python version and operating
    system. Version |release| ships wheels for **CPython 3.12** on Windows (x86-64), Linux (x86-64 and ARM64) and
    macOS 14+ (Intel and Apple Silicon).

.. warning::
    While |release| is a pre-release, a plain ``pip install -U AnimeGameRemap`` (without a version) installs the
    latest *stable* release instead, which is an older version of the library whose API differs from the one
    documented here. Pin the version as shown above (or pass ``--pre``).

:raw-html:`<br />`

Importing
---------

.. code-block:: python

    import AnimeGameRemap as AGR

    iniFile = AGR.IniFile("MyMod.ini")
    iniFile.parse()
    iniFile.fix()

``AnimeGameRemap`` re-exports the public classes and functions of the ``FixRaidenBoss2`` package, which is the
package that actually contains the library (``AnimeGameRemap`` depends on it and installs it for you). That is
why the :doc:`Python API Reference <api>` lists every class under ``FixRaidenBoss2``: ``AGR.IniFile`` and
``FixRaidenBoss2.IniFile`` are the same class. You can always ``import FixRaidenBoss2`` directly instead.

:raw-html:`<br />`

How To Use
----------
- For some simple examples of using the API, visit :doc:`API Examples <apiExamples>`
- For a full reference of the Python API, visit :doc:`Python API Reference <api>`
- For the C++ library underneath it, visit :doc:`C++ Core API Reference <coreAPI>`

.. _Pypi: https://pypi.org/project/AnimeGameRemap/
