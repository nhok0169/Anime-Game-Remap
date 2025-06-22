.. role:: raw-html(raw)
    :format: html


File Requirements
==================

.. note::
    If you know your mods are auto-generated using the standard scripts provided
    by GIMI, you can probably skip this section. The content below is dedicated to mods 
    with custom made .ini files.


:raw-html:`<br />`
:raw-html:`<br />`

For those who stayed, let us continue.

:raw-html:`<br />`

Basic Assumptions
-----------------

- Assume you are fixing a mod with the name ``YourModName``
- Your mod works before even using the fix

:raw-html:`<br />`

Definitions
-----------

Mod Objects
~~~~~~~~~~~

A mod is usually split into many different objects/parts (eg. an object for the mod's head, an object for the mod's body, etc...).
We will be referring to these objects as **mod objects**. Luckly the mod objects for a type of mod are usually standardized to be the
same. See `GIMI Assets`_ for more info about the mod objects for a particular mod.

:raw-html:`<br />`

Registers
~~~~~~~~~

within a `section`_ in a .ini file, you may have noticed many key-value pairs shown below:

.. code-block:: ini
    :linenos:

    [TextureOverrideEverything]
    hash = baddbabe
    ps-t0 = ResourceSomething
    p = np

We define those keys as **registers**

:raw-html:`<br />`

Ini Files
---------

:raw-html:`<br />`

Blend Sections
~~~~~~~~~~~~~~

The **root sections** that reference some sort of `Blend.buf` is recommended to be named based off the following
Regex format:

.. code-block:: 

   ^\s*\[\s*TextureOverride.*YourModName((?!(RemapBlend)).)*Blend.*\s*\]

:raw-html:`<br />`

.. tip::
    Whether the section name is lowercase/uppercase does not matter

:raw-html:`<br />`

*eg.* :raw-html:`<br />`
For the following .ini file named ``blendNameExample.ini``, the **highlighted line** is the name where you need to pay attention to the naming.


.. dropdown:: blendNameExample.ini
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1

        [TextureOverrideYourModNameBlend]
        ...
        if $swapvar == 0
            Run = AnyNameYouWantHere
            vb1 = ResourceBoo
        else
            Run = AnotherSectionName
        endif

        [AnyNameYouWantHere]
        vb1 = ResourceToBlend
        Run = 

        [AnotherSectionName]
        vb1 = ResourceFreeToNameThisWhateverYouWant
        Run = ThisIsALeaf

        [ThisIsALeaf]
        vb1 = ResourceHello

        [ResourceToBlend]
        filename = BelloBlend.buf

        [ResourceFreeToNameThisWhateverYouWant]
        filename = BananaBlend.buf

        [ResourceBoo]
        filename = PoopayeBlend.buf

        [ResourceHello]
        filename = BeeDooBeeDooBlend.buf

:raw-html:`<br />`

Normal Blend Naming Examples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: 

    [TextureOverrideKeqingBlend]

.. code-block:: 

    [TextureOverrideOhNoRaidenShogunInTheKitchenBlender]

.. code-block:: 

        [      TextureOverride.Fun.With.SpacesShenheBlend         ]


:raw-html:`<br />`
:raw-html:`<br />`

If you ran the command line with the ``--all`` option, the name of the section
has a bit more flexibility to follow the following pattern. However, you need to specify what mod you are trying to fix by using 
the ``--defaultType`` option

(see :doc:`commandOpts` for details about more command line options and names for mods to specify in the options)


.. code-block:: 

   ^\s*\[\s*TextureOverride.*Blend.*\s*\]

:raw-html:`<br />`

.. tip::
    Whether the section name is lowercase/uppercase does not matter

:raw-html:`<br />`

All Option Blend Naming Examples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: 

    [TextureOverridePierrotLunaireBlend]

.. code-block:: 

    [     TextureOverrideDerKrankeMondBlenderDerMondfleck       ]


:raw-html:`<br />`
:raw-html:`<br />`

Mod Object Sections
~~~~~~~~~~~~~~~~~~~~

.. tip::
    See `Mod Objects`_ for how we define a **mod object**

- Assume we are referring to a mod object by the name ``YourModObject``

:raw-html:`<br />`

The **root sections** that reference some soft of `Blend.buf` is recommended to be named based off the following Regex format:

.. code-block:: 

   ^\s*\[\s*TextureOverride.*YourModObject\]

:raw-html:`<br />`

*eg.* :raw-html:`<br />`
For the following .ini file named ``modObjectNameExample.ini``, the **highlighted line** is the name where you need to pay attention to the naming.


.. dropdown:: modObjectNameExample.ini
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:
        :emphasize-lines: 1

        [TextureOverrideKeqingBody]
        ...
        if $swapvar == 0
            Run = AnyNameYouWantHere
            vb1 = ResourceBoo
        else
            Run = AnotherSectionName
        endif

        [AnyNameYouWantHere]
        vb1 = ResourceToBlend
        Run = 

        [AnotherSectionName]
        vb1 = ResourceFreeToNameThisWhateverYouWant
        Run = ThisIsALeaf

        [ThisIsALeaf]
        vb1 = ResourceHello

        [ResourceToBlend]
        filename = BelloBody.dds

        [ResourceFreeToNameThisWhateverYouWant]
        filename = BananaBody.dds

        [ResourceBoo]
        filename = PoopayeBody.dds

        [ResourceHello]
        filename = BeeDooBeeDooBody.dds

:raw-html:`<br />`

Mod Object Naming Examples
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: 

    [TextureOverrideJeanHead]

.. code-block:: 

    [TextureOverrideWhatABeautifulDress]

.. code-block:: 

        [      TextureOverride.Fun.With.SpacesCelestialBody         ]


:raw-html:`<br />`

TextureOverride Register Value Naming
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. tip::
    See `Registers`_ for how we define a **register**

For the mod object sections with the name template: ``[TextureOverride{YourModObject}]``, 
there are many pixel shader registers (registers with the format of ``ps-tx`` for some non-negative integer x)

It is recommended that the resource referenced by these pixel shader registers follow the same naming
scheme from the standard made at `GIMI Assets`_

Usually some common keywords to include in the resource name consists of:

* Diffuse
* LightMap
* Shadow
* MetalMap
* ShadowRamp

:raw-html:`<br />`

*eg.* :raw-html:`<br />`
For Raiden Shogun, according to `GIMI Assets`_ , the `ps-t0` register for her `Head` mod object should be named like below:

.. code-block:: ini
    :linenos:

    ps-t0 = ResourceLaDameauxCaméliasDiffuse

.. _GIMI Assets: https://github.com/SilentNightSound/GI-Model-Importer-Assets
.. _section: https://en.wikipedia.org/wiki/INI_file#Sections