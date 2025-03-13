.. role:: raw-html(raw)
    :format: html

How to Make Changes to the Project
==================================


1. Clone the Project
--------------------
Fork the `Github repo`_ , then do a ``git clone`` on your fork

:raw-html:`<br />`
:raw-html:`<br />`

2. Make your Changes
--------------------
AG Remap has 3 different types of builds:

#. `The API`_ (The source code for the project)
#. `The API Mirror`_ (A mirror to the API)
#. `The Script`_ (A compatible script for users who do not know how to use Pypi or any other Python package manager)

:raw-html:`<br />`

You would want to make your changes within `the API`_ . All the other builds are generated using other tools within the project.

:raw-html:`<br />`
:raw-html:`<br />`

3. Compile your Changes
-----------------------
Once you are done making changes, you want compile your changes to be updated within the other builds.

You can do this by running the `CI Pipeline`_

Steps on how to run the CI Pipeline are here:
https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/CIPipeline

:raw-html:`<br />`
:raw-html:`<br />`

4. Test your Changes
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
https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Unit%20Tester

.. important::
    If you have created a new module in the software, it is recommended that you build
    unit tests since:

    #. to see whether your module works by itself
    #. your tests are used for future `regression testing`_ against your module

    The unit tests are built using Python's `unittest` library. You can check out `the different unit tests here`_ for 
    how to make a unit tests within the project

:raw-html:`<br />`

II. Acceptance Tests
~~~~~~~~~~~~~~~~~~~~
This test is where you verify whether your changes actually work in the game.

#. Within `the Script`_ build, copy ``AGRemap.py`` into the ``Mod`` folder of 3dmigoto
#. Run ``AGRemap.py``

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
https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Integration%20Tester

You can check out the specific integration tests here

.. warning::
    This test may take a while since it performs a lot of file manipulation

:raw-html:`<br />`
:raw-html:`<br />`

5. Commit your Changes
----------------------
When all tests are clear, commmit your changes using ``git``, then push your changes back to your forked Github repo.

After send a `Pull Request (PR)`_ to merge your new changes from your fork back to the `AG Remap repo`_
We will do a code review on your PR.

.. note::
    When you push or merge your changes to Github, Github will trigger a CD pipeline that will automatically run your changes 
    against the `unit tests`_ and the `integration tests`_ to make sure your changes have met the test requirements.

:raw-html:`<br />`
:raw-html:`<br />`

6. Merge your Changes
---------------------
Once your PR is approved, we will merge your changes back to the `AG Remap repo`_


.. _Github repo: https://github.com/nhok0169/Anime-Game-Remap
.. _AG Remap repo: https://github.com/nhok0169/Anime-Game-Remap
.. _The API: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/api
.. _The API Mirror: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/apiMirror
.. _The Script: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Anime%20Game%20Remap%20(for%20all%20users)/script%20build/src/FixRaidenBoss2
.. _regression testing: https://en.wikipedia.org/wiki/Regression_testing
.. _CI Pipeline: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Tools/CIPipeline
.. _Unit tests: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Unit%20Tester
.. _Integration tests: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Integration%20Tester
.. _unittest: https://docs.python.org/3/library/unittest.html
.. _the different unit tests here: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Unit%20Tester/UnitTester/Tests
.. _specific integration tests here: https://github.com/nhok0169/Anime-Game-Remap/tree/nhok0169/Testing/Integration%20Tester/IntegrationTester/Tests
.. _Pull Request (PR): https://github.com/nhok0169/Anime-Game-Remap/pulls