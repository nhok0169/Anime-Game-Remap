.. role:: raw-html(raw)
    :format: html

How to Create a Remap
======================

.. attention::
    Due to the project constantly changing, there is a possibility that this page may become outdated.

.. important::
    This guide will only show a **high-level overview** on how to create a remap.

    We will not go over very specific details since going over every detail will take too long and each character
    remap have their own unique quirks.

    Generally, the parts to add for a character's remap will follow the same repetitive patterns seen in the referenced files of this guide.
    The best starting point is always an existing character whose remap has the same *shape* as yours
    (eg. the target draws the same objects, more objects, or is a skin made of several components).

    It is expected that those who are willing to make their own remaps to **actually read the source code**.

.. tip::
    If you do not know the general idea on how to make a change to the project, it is recommended that you
    read :doc:`makeChanges`  first

:raw-html:`<br />`

Where the Data for a Remap Lives
--------------------------------
The data and the logic for every remap live in the library's C++ core (see :doc:`coreAPI`):

* `core/src/data`_ (and the matching headers in `core/include/AGRemapCore/data`_): the tables of hashes, indices, vertex counts,
  vertex group remaps, and the builder tables that pick the .ini parser and .ini fixer for each character
* `core/src/data/IniParseData`_ and `core/src/data/IniFixData`_: **one folder per character** holding that character's
  .ini parser and .ini fixers. The files at the top level of these folders (eg. ``GIMICharFixer.cpp``) are the shared
  templates that the characters are built from
* `core/src/constants`_: the registration of the characters (:cpp:enum:`AGRemapCore::ModTypeId`, :cpp:class:`AGRemapCore::GIBuilder`, ...)

A remap is a pair of directions (``character -> skin`` and ``skin -> character``) and each direction is built and tested on its own.

:raw-html:`<br />`
:raw-html:`<br />`

1. Add the Textures Files and Model Binary Files to the Mod Downloads
---------------------------------------------------------------------
When a mod is missing some files that the remapped mod needs (eg. a texture or a .ib file of an object the mod does not draw),
the software downloads the character's original files from the `Mod Downloads`_ folder of this repository.

a. Get the assets for the characters. For Genshin Impact, go to `GIMI Assets`_ and find the characters you are trying to create a remap for.
   For Wuthering Waves, use `WWMI Assets`_. If the assets do not have the character, take a frame dump of the character in game.

b. Create the folder to hold the character files at `Mod Downloads`_, in the form ``<Game>/<Character Name>/<Game Version>``
   (eg. ``GI/Jean/4_0``)

c. Generate the .buf and .ib binary files and copy the .dds texture files into the newly created folder.

   * For a Genshin Impact character, `giDownloadFolder.py`_ builds the whole folder from the character's asset folder.
   * For a Wuthering Waves character, use `wwmiDownloadFolder.py`_, or `wwmiExtractDump.py`_ for a frame dump.
   * You can also do the conversion step by step with the Jupyter Notebooks at `Dump To Mod Converter`_, and get some basic
     analytics on the result (number of vertices, size of the Texcoord per vertex, ...) with the Notebooks at `Mod Analyzer`_

.. important::
    The software fetches the downloads from the ``master`` branch of the `AG Remap repo`_ when it runs. So, until the new
    download folders are merged into ``master``, every download of your new remap will fail.
    It is best to send the download folders in their own `Pull Request (PR)`_ before working on the rest of the remap.

:raw-html:`<br />`
:raw-html:`<br />`

2. Register the Characters
--------------------------
Each character is identified by a :cpp:enum:`AGRemapCore::ModTypeId` and constructed as a :cpp:class:`AGRemapCore::ModType`.
A new character needs to be added at:

a. `ModTypeId.h`_ and `ModTypeId.cpp`_: the enumeration entry, its name, which characters it can be remapped to, and the keywords
   in the names of the .ini sections that identify the character (used by the .ini classifier to decide whether some .ini file belongs to the character)
b. `GIBuilder.cpp`_ (or `WWMIBuilder.cpp`_ for Wuthering Waves): the construction of the mod type, including its aliases
c. The Python bindings at `PyModTypeId.cpp`_ and `PyGIBuilder.cpp`_ (or `PyWWMIBuilder.cpp`_)
d. `ModTypes.py`_: the Python enumeration of the supported mod types, used by the software and as a convenience for API users

.. note::
    Some characters are only ever remapped *onto* (eg. a boss, or a single component of a skin made of several components).
    These characters only need an entry in `ModTypeId.h`_ / `ModTypeId.cpp`_ and their data from the next step.

:raw-html:`<br />`
:raw-html:`<br />`

3. Add the Hashes/Indices for the Characters
--------------------------------------------
The hashes and indices of a Genshin Impact character are located in a file called ``hash.json`` in `GIMI Assets`_.
For a Wuthering Waves character, they are in the character's ``Metadata.json`` in `WWMI Assets`_.

Add the hashes, the indices and the number of vertices into `HashData.cpp`_, `IndexData.cpp`_ and `VertexCountData.cpp`_ respectively.

Wuthering Waves characters carry 4 more tables: `IndexCountData.cpp`_, `VGOffsetData.cpp`_, `VGCountData.cpp`_ and `ShapeKeyChecksumData.cpp`_

.. tip::
    Don't just get the latest hashes/indices from `GIMI Assets`_ . Try to get the historical changes
    for the characters' hashes/indices by checking the commit history of some characters within `GIMI Assets`_ ,
    since a mod carries the hashes of whichever game version it was made for.

:raw-html:`<br />`
:raw-html:`<br />`

4. Add the Vertex Group Remap for the Characters
------------------------------------------------
Find the vertex group remap by comparing the vertex group indices of the character to map from vs. the character to map to (see :doc:`findVertexGroupRemap` for details on how to find the vertex group)

* `VGRemapFinder`_ proposes a draft of the remap from the geometry of the two characters
* The drafts are kept as Excel workbooks at `Remap Drafts`_, with one sheet per direction

Once you have the remaps, update them at `VGRemapData.cpp`_

.. warning::
    **Every vertex group of the character to map from must be mapped to some vertex group**. An unmapped vertex group
    deforms the model in game.

    Also, a proposed remap only finds the *closest* vertex group. Review it by hand for parts like capes, coat tails or skirts,
    where the closest vertex group may belong to a part that moves differently.

:raw-html:`<br />`
:raw-html:`<br />`

5. Make Identity Mods for the Characters
----------------------------------------
An identity mod is a character's own model written out as a mod: every object, vertex group and texture of the original character in one folder.
It is the first mod to test a remap on, and it tells you how the target character binds its own textures.

* Genshin Impact: `identityMod.py`_
* Wuthering Waves: `wwmiIdentityMod.py`_

After the identity mods, collect a **variety** of real mods for the characters (different structures: merged mods, mods with
toggles, mods missing some objects, ...) so that the remap does not only work for one mod.

:raw-html:`<br />`
:raw-html:`<br />`

6. Prototype the Remap from Python
----------------------------------
Rather than rebuilding the C++ core for each attempt, prototype the .ini parser and .ini fixer from Python first.
:class:`FixRaidenBoss2.CppStrategyOverrides` registers a parser or a fixer at runtime, which takes the place of the
compiled one for that character.

For a character of the standard shape, the prototype is just a configuration handed to the same functions the
compiled characters use (eg. :class:`FixRaidenBoss2.GIMICharFixerConfig` with :func:`FixRaidenBoss2.makeGIMICharFixer`).
See `overrideScript.py`_ for a small example, and the other scripts in `Prototypes`_ for full remaps.

Run the prototype on the mods from step 5 until the remapped mods work in the game. Build the prototype from the classes of the
library wherever possible, and note anything the library cannot do yet: those gaps are what the next steps add to the library.

:raw-html:`<br />`
:raw-html:`<br />`

7. Add the .ini Parser for the Characters
-----------------------------------------
This part indicates how the .ini files of a character will be parsed:
which sections belong to which object of the character, and which files to download from step 1
(see :cpp:struct:`AGRemapCore::GIMICharParserConfig` for the options of the standard shape).

a. Add the parser to the character's folder at `core/src/data/IniParseData`_ (and its header in `core/include/AGRemapCore/data/IniParseData`_)
b. Add a row for the character and the game version at `IniParseBuilderData.cpp`_

:raw-html:`<br />`
:raw-html:`<br />`

8. Add the .ini Fixer for the Characters
----------------------------------------
This part indicates how the .ini files of a character will be fixed.

This section also includes:

* some basic logic on how the registers within the .ini file should be editted
* the creation or editting of .dds texture files
* edits to the .buf files (eg. moving the model in the Position.buf file)

Most fixers are a configuration for one of the shared templates:

.. list-table::
    :header-rows: 1

    * - Shape of the remap
      - Configuration
      - Template
    * - A one-mesh character onto another one-mesh character
      - :cpp:struct:`AGRemapCore::GIMICharFixerConfig`
      - :cpp:func:`AGRemapCore::makeGIMICharFixer`
    * - A one-mesh character onto a skin made of several components
      - :cpp:struct:`AGRemapCore::GIMIComponentFixerConfig`
      - :cpp:func:`AGRemapCore::makeGIMIComponentFixer`
    * - A skin made of several components onto a one-mesh character
      - :cpp:struct:`AGRemapCore::GIMIMergeFixerConfig`
      - :cpp:func:`AGRemapCore::makeGIMIMergeFixer`
    * - A Wuthering Waves character
      - :cpp:struct:`AGRemapCore::WWMIFixerConfig`
      - :cpp:func:`AGRemapCore::makeWWMIFixer`

a. Add the fixer to the character's folder at `core/src/data/IniFixData`_ (and its header in `core/include/AGRemapCore/data/IniFixData`_)
b. Add a row at `IniFixBuilderData.cpp`_ for each character to remap to
c. Add a row for the character at `IniRemoveBuilderData.cpp`_
d. Add the new source files to `CMakeLists.txt`_

:raw-html:`<br />`
:raw-html:`<br />`

9. Build and Test the Remap
---------------------------
Build the library (see :doc:`makeChanges`) and test the compiled remap:

* compare its output against the prototype from step 6 on every test mod. They should be the same
* check the remapped mods in the game
* update the counts of the tables that are checked by the C++ tests at `core/tests`_ (eg. ``BuilderData_test.cpp``, ``VertexCounts_test.cpp``,
  ``VGRemaps_test.cpp``, ``ModTypeRemaps_test.cpp``)

Keep the prototype working afterwards, since it is what any later change to the remap is compared against.

:raw-html:`<br />`
:raw-html:`<br />`

10. Repeat for the Other Direction
----------------------------------
Repeat steps 6 - 9 for the remap in the other direction (``skin -> character``). This direction usually needs a different
template from the first direction, so it gets its own prototype.

:raw-html:`<br />`
:raw-html:`<br />`

11. Document the Remap
----------------------
Add the new characters to:

* the table of mod types at :doc:`commandOpts`
* the same table in the READMEs of `the API`_ and `the API Mirror`_
* :doc:`remapGrading`, with one entry for each direction

Then run `checkModTypeTables.py`_ to make sure the tables agree with the library.



.. _GIMI Assets: https://github.com/SilentNightSound/GI-Model-Importer-Assets
.. _WWMI Assets: https://github.com/SpectrumQT/WWMI-Assets
.. _AG Remap repo: https://github.com/nhok0169/Anime-Game-Remap
.. _Pull Request (PR): https://github.com/nhok0169/Anime-Game-Remap/pulls
.. _the API: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/README.md
.. _the API Mirror: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/apiMirror/README.md
.. _Mod Downloads: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Data/Mod%20Downloads
.. _Remap Drafts: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Data/RemapDrafts
.. _Dump To Mod Converter: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/DumpToModConverter
.. _Mod Analyzer: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/ModAnalyzer
.. _VGRemapFinder: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/VGRemapFinder
.. _Prototypes: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Tools/Misc/Prototypes
.. _giDownloadFolder.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/giDownloadFolder.py
.. _wwmiDownloadFolder.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/wwmiDownloadFolder.py
.. _wwmiExtractDump.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/wwmiExtractDump.py
.. _identityMod.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/identityMod.py
.. _wwmiIdentityMod.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/wwmiIdentityMod.py
.. _overrideScript.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Prototypes/overrideScript.py
.. _checkModTypeTables.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/Misc/Diagnostics/checkModTypeTables.py
.. _core/src/data: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data
.. _core/include/AGRemapCore/data: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/include/AGRemapCore/data
.. _core/src/data/IniParseData: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IniParseData
.. _core/include/AGRemapCore/data/IniParseData: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/include/AGRemapCore/data/IniParseData
.. _core/src/data/IniFixData: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IniFixData
.. _core/include/AGRemapCore/data/IniFixData: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/include/AGRemapCore/data/IniFixData
.. _core/src/constants: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/constants
.. _core/tests: https://github.com/nhok0169/Anime-Game-Remap/tree/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/tests
.. _CMakeLists.txt: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/CMakeLists.txt
.. _ModTypeId.h: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/include/AGRemapCore/constants/ModTypeId.h
.. _ModTypeId.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/constants/ModTypeId.cpp
.. _GIBuilder.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/constants/GIBuilder.cpp
.. _WWMIBuilder.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/constants/WWMIBuilder.cpp
.. _PyModTypeId.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/py/src/constants/PyModTypeId.cpp
.. _PyGIBuilder.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/py/src/constants/PyGIBuilder.cpp
.. _PyWWMIBuilder.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/py/src/constants/PyWWMIBuilder.cpp
.. _ModTypes.py: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/py/FixRaidenBoss2/constants/ModTypes.py
.. _HashData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/HashData.cpp
.. _IndexData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IndexData.cpp
.. _VertexCountData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/VertexCountData.cpp
.. _IndexCountData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IndexCountData.cpp
.. _VGOffsetData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/VGOffsetData.cpp
.. _VGCountData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/VGCountData.cpp
.. _ShapeKeyChecksumData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/ShapeKeyChecksumData.cpp
.. _VGRemapData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/VGRemapData.cpp
.. _IniParseBuilderData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IniParseBuilderData.cpp
.. _IniFixBuilderData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IniFixBuilderData.cpp
.. _IniRemoveBuilderData.cpp: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Anime%20Game%20Remap%20(for%20all%20users)/api/src/cpp/core/src/data/IniRemoveBuilderData.cpp
