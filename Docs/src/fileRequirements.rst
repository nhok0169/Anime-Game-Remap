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

- Your mod works before even using the fix

:raw-html:`<br />`

Definitions
-----------

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

Sections
~~~~~~~~
As long as your `sections`_ references the correct ``hash`` `KVPs`_, then you are good.

:raw-html:`<br />`

TextureOverride Register Value Naming
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. tip::
    This only applies for GI mods

.. tip::
    See `Registers`_ for how we define a **register**

:raw-html:`<br />`

Certain `sections`_ may reference many pixel shader registers (registers with the format of ``ps-tx`` for some non-negative integer x)

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
For Raiden Shogun, according to `GIMI Assets`_ , the ``ps-t0`` register for her ``Head`` mod object should be named like below:

.. code-block:: ini
    :linenos:

    ps-t0 = ResourceLaDameauxCaméliasDiffuse

.. _GIMI Assets: https://github.com/SilentNightSound/GI-Model-Importer-Assets
.. _section: https://en.wikipedia.org/wiki/INI_file#Sections
.. _sections: https://en.wikipedia.org/wiki/INI_file#Sections
.. _KVP: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _KVPs: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair