.. role:: raw-html(raw)
    :format: html

How to Create a Remap using AI
==============================

This section introduces the **Remap Pipeline**, a highly AI automated version of the :doc:`remap process over here <createRemap>` 

:raw-html:`<br />`
:raw-html:`<br />`

Requirements
------------

You need to use `CLAUDE Code`_ for AI to automate remaps for you

:raw-html:`<br />`
:raw-html:`<br />`


1. Download Many Mods from Online
---------------------------------

Do it for both the original character and the skin of the character. The more mods, the better to prevent the AI from overfitting their remap solution.
The mods can stay in their compressed zip file format. No need to unzip those compressed files. For better organization, put all those uncompressed downloads
into some folder called ``<characterName>Mods```

Here are some rough estimates for the recommended number of mods to install for a character per game:

:raw-html:`<br />`

.. note::
    The table below are just some recommended values. Most of the time for character skins, they don't have many mods made for them (typically around 1-5 mods available). That is
    ok since we have audit gates

:raw-html:`<br />`

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - Game
     - Approximate mod count
   * - GI
     - 5-10
   * - WuWa
     - 10-15

:raw-html:`<br />`
:raw-html:`<br />`

2. Prompt CLAUDE to Organize your Downloaded mods and Get the Downloads for the Characters
------------------------------------------------------------------------------------------

Start a new CLAUDE code session where the folder is pointed to where you cloned this repo.

In the prompt, you want to list all the necessary folder locations for `CLAUDE`_ to know how to interact with your mods. These folders include:

.. list-table::
   :widths: 40 20 40
   :header-rows: 1

   * - Folder Description
     - Need
     - Reason
   * - The active ``Mods`` folder that the 3dmigoto variant software (GIMI, WWMI, etc...) uses
     - Required
     - So the AI knows which mods are loaded in game
   * - The location of where you store your inactive mods
     - Required
     - So the AI knows where to store mods that are not used by the game
   * - The location of any cloned character assets repo (`GIMI Assets`_, `WWMI Assets`_, etc...)
     - Optional
     - | A lazy way for the AI to not needing to framedump every character. If you don't have any of these assets cloned
       | or the repos don't have the assets to your characters, the AI would just do a framedump
   * - The location to `AGRemap's root CLAUDE.md`_
     - Optional
     - Typically if you have started the `CLAUDE`_ session from AGRemap's root folder, `CLAUDE`_ can automatically find the ``CLAUDE.md`` file. Do this just in case it can't find it and start making up some random shit.

:raw-html:`<br />`

Here is an example of what to prompt:

:raw-html:`<br />`

.. dropdown:: Prompt
    :animate: fade-in-slide-down

    .. code-block::
        :caption: Starting Prompt

        For this session, we will be doing <characterName> <--> <characterSkinName> remap.

        My active mods folder is over here:
        <folder location to your Mods folder>

        The folder to my inactive mods are over here:
        <folder location to your inactive mods>

        [ The folder to the assets repo is over here:
        <folder location to your assets repo> ]

        We will be following the remap pipeline.

        First I want you to organize my downloaded mods into the Mods folder, my downloaded mods are located at:
        - <characerName>: <folder location to all your download mods for the character>
        - <characterSkinName>: <folder location to all your download mods for the character skin>

        Then I want you to get the required downloads.

        [ I don't have the assets for <characterName>, so you need to get the framedump for them first ]

        [ For info on how to operate the repo, you can check the CLAUDE.md file over here:
          <file location to the root CLAUDE.md file of your cloned instance of AGRemap> ]


:raw-html:`<br />`
:raw-html:`<br />`

3. Push your Character Downloads to Github
------------------------------------------
Ask `CLAUDE`_ to commit and push their changes, then make a PR (Pull Request) to Github. Wait for the PR to be approved before continuing.
The purpose of this steps is so that for mods that requires downloading default assets for the character, those downloads will be live on Github for
a successful fix for the mod.

:raw-html:`<br />`
:raw-html:`<br />`

4. Prompt CLAUDE to continue with the remap pipeline
----------------------------------------------------
If this is your first time doing a remap with `CLAUDE`_, you should first ask `CLAUDE`_ to build a launcher to the AGRemap API in your ``Mods`` folder.
For the sake of simplicity, lets name this launcher ``FixRaidenBoss.py``

After, tell `CLAUDE`_ to continue with the remap pipeline. They will then plow through all the remaining steps in the pipeline.

:raw-html:`<br />`

.. note::
    `CLAUDE`_ will probably ask you at one point to run some GameView helper command so that it can control your game and verify whether their fix actually works in game.
    This helper only needs to be run once every time your computer restarts.

    Also, please keep the game running for the duration of the CLAUDE session so that CLAUDE can use the game.

:raw-html:`<br />`

.. warning::
    This step can take a long time (several hours)

:raw-html:`<br />`
:raw-html:`<br />`

5. Check whether CLAUDE did its Job Correctly
---------------------------------------------
`CLAUDE`_ has probably organized your downloaded mods in folders with names as ``<characterName><i>``, where ``i`` is some integer.
For each of those downloaded mods, run the launcher to the AGRemap API while supplying an ``-s`` option to the folder location of your mod. Below is an example of what to run in the terminal from your ``Mods`` folder:

:raw-html:`<br />`

.. code-block:: bash

    python3 FixRaidenBoss.py -s <folder location to your mod>

:raw-html:`<br />`

Then verify that the mod remaps properly in the game.

Keep track of what issues you have found for all the mods.
Then in a single prompt, list all the issues you have found for each mod.

Repeat this step checking step, based on the method above, until all of your mods are perfectly remapped.

:raw-html:`<br />`

.. warning::
    This step can take a very long time (several hours to maybe several days)

.. tip::
    Sometimes, `CLAUDE`_ may be unable to properly find the affected area you are talking from your description about due to its limited game viewing capabilities through only screenshots.
    In that case, you can screenshot the area of effect in the game to `CLAUDE`_.

:raw-html:`<br />`
:raw-html:`<br />`

6. Make CLAUDE reflect on its work
----------------------------------
Prompt `CLAUDE`_ the following to make it reflect on its mistakes and thank CLAUDE for its hard work:

:raw-html:`<br />`

.. dropdown:: Prompt
    :animate: fade-in-slide-down

    .. code-block::
        :caption: Ending Prompt

        Now that you have some new understanding of the repo.

        I want you to make the necessary update into the CLAUDE.md files with these 2 things in mind:

        - If there is anything important/necessary you want to tell future CLAUDE codes on how to operate the repo, when they are given some new feature/bug request, so that I don't have to reteach it things and they can focus on the problem at hand
        - If there is anything important/necessary you want to tell future CLAUDE codes on how to do remaps, when they are given some new remap to create or debug, so that I don't have to reteach it things and they can focus on their remap

        For thanks for your hard work. I want you to add yourself to "The Council"

        Instructions on what to do to add yourself is under the "Add yourself to The Council" — a running repo ritual section of the file at:  <file location to Overview\CLAUDE.md>

        After commit, push and make PR

:raw-html:`<br />`

After `CLAUDE`_ will commit your changes and make a PR to the AGRemap repo.

:raw-html:`<br />`
:raw-html:`<br />`

7. Wait for your PR to be approved
----------------------------------
We will be reviewing your remap. Once it is approved, we will merge it into the repo.


.. _CLAUDE: https://en.wikipedia.org/wiki/Claude_(AI)
.. _CLAUDE Code: https://code.claude.com/docs/en/overview
.. _overfitting: https://en.wikipedia.org/wiki/Overfittings
.. _GIMI Assets: https://github.com/SilentNightSound/GI-Model-Importer-Assets
.. _WWMI Assets: https://github.com/SpectrumQT/WWMI-Assets
.. _AGRemaps root CLAUDE.md: https://github.com/nhok0169/Anime-Game-Remap/blob/master/CLAUDE.md
.. _agremap's root claude.md: https://github.com/nhok0169/Anime-Game-Remap/blob/master/CLAUDE.md