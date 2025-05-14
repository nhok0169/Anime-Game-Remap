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

    Generally, the parts to add for a character's remap will follow the same repetitive patterns seen in the referenced module files of this guide. 

    It is expected that those who are willing to make their own remaps to **actually read the source code**.

.. tip::
    If you do not know the general idea on how to make a change to the project, it is recommended that you
    read :doc:`makeChanges`  first

:raw-html:`<br />`
:raw-html:`<br />`

1. Register the name of the characters to remap
-----------------------------------------------
Add the names of the characters to remap at `ModTypeNames.py`_

:raw-html:`<br />`
:raw-html:`<br />`

2. Add the Hashes/Indices for the Characters
--------------------------------------------
You can go to `GIMI Assets`_ and find the characters you are trying to create a remap for.
The hashes and indices are located in a file called ``hash.json``

Add the hashes and indices into `HashData.py`_ and `IndexData.py`_ respectively.

.. tip::
    Don't just get the latest hashes/indices from `GIMi Assets`_ . Try to get the historical changes
    for the characters' hashes/indices by checking the commit history of some characters within `GIMI Assets`_

:raw-html:`<br />`
:raw-html:`<br />`

3. Add the necessary Textures Files and Model Binary Files to the Mod Downloads
-------------------------------------------------------------------------------
This step requires to have the downloaded collected dump assets for the character

a. Create the folder to hold the character files at: `Mod Downloads`_
b. Copy the texture .dds files into the newly created folder
c. Generate the proper .buf binary files for the character by running the corresponding Jupyter Notebook at `Dump To Mod Converter`_

   In the Notebook, you would most likely need to specify the folder location of the dumped assets and the folder
   location for the output (the folder you just created)

:raw-html:`<br />`
:raw-html:`<br />`

4. Add the Vertex Group Remap for the Characters
------------------------------------------------
Find the vertex group remap by comparing the vertex group indices of the character to map from vs. the character to map to *(guide comming soon...)*

Once you have the remaps, update them at `VGRemapData.py`_

:raw-html:`<br />`
:raw-html:`<br />`

5. Add the Position.buf Editor for the Characters
--------------------------------------------------
Rarely any characters actually edit the Position.buf file, so most of their values are ``None``

You can fill in the info for the editor at `PositionEditorData.py`_

:raw-html:`<br />`
:raw-html:`<br />`

6. Add the .ini Parser for the Characeters
------------------------------------------
This part indicates how the .ini files of a character will be parsed.
How certain .dds texture files are editted are also indicated here.

Add the necessary parser and its arguments/keyword arguments at `IniParseBuilderData.py`_

:raw-html:`<br />`
:raw-html:`<br />`

7. Add the .ini Fixer for the Characters
----------------------------------------
This part indicates how the .ini files of a character will be fixed.

This section also includes:

* some basic logic on how the registers within the .ini file should be editted
* creation of some new .dds texture files

Add the necessary fixer and its arguments/keyword arguments at `IniFixBuilderData.py`_

:raw-html:`<br />`
:raw-html:`<br />`

8. Specify the Construction of the Overall Mod Type for the Characters
----------------------------------------------------------------------
The mod type of a character will include all the data from the previous steps.

API users will use this construction method if they want to get a brand new copy of a certain mod type that
is not referenced by the software.

Add the construction of the mod types at `GIBuilder.py`_

:raw-html:`<br />`
:raw-html:`<br />`

9. Register the Mod Type to the Software
----------------------------------------
Go to `ModTypes.py`_ and add in new enumeration entries for the new characters that uses the construction method
from Step 6.

This enumeration is used by:

* The software
* Convenience for API users if they know they will not be changing up the mod type

:raw-html:`<br />`
:raw-html:`<br />`

10. Register the Character Names into the .ini Classifier
----------------------------------------------------------  
Go to `IniClassifierBuilder.py`_ and specify how software will identify whether some .ini file belong to the characters



.. _GIMI Assets: https://github.com/SilentNightSound/GI-Model-Importer-Assets
.. _ModTypeNames.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/constants/ModTypeNames.py
.. _HashData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/HashData.py
.. _IndexData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/IndexData.py
.. _PositionEditorData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/PositionEditorData.py
.. _IniParseBuilderData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/IniParseBuilderData.py
.. _VGRemapData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/VGRemapData.py
.. _IniFixBuilderData.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/data/IniFixBuilderData.py
.. _GIBuilder.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/constants/GIBuilder.py
.. _ModTypes.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/constants/ModTypes.py
.. _IniClassifierBuilder.py: https://github.com/nhok0169/Anime-Game-Remap/blob/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api/src/FixRaidenBoss2/model/strategies/iniClassifiers/IniClassifierBuilder.py
.. _Mod Downloads: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Data/Mod%20Downloads
.. _Dump To Mod Converter: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/DumpToModConverter

