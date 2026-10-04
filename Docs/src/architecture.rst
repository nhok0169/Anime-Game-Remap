.. role:: raw-html(raw)
    :format: html

AGRemap's Architecture
======================
After migrating the `old API`_ from Python to C++, AGRemap's overall architecture became a lot more complex.
The diagram below shows the new AGRemap architecture:

.. image:: ./_static/images/AGRemap\ Architecture.png
    :alt: AGRemap's architecture
    :width: 100%
    :align: center

:raw-html:`<br />`
:raw-html:`<br />`

Notes on Architecture
---------------------
- The green boxes are API endpoints that can be interacted with using your own code. AGRemap's can be used in both Python and C++.
- Currently the Core's unit tester has not been fully developed and is simply many scratch unit tests. Later we will implement an actual unit tester framework like the Python side.

:raw-html:`<br />`
:raw-html:`<br />`

Why go for a C++ Extended Python Layered Architecture
-----------------------------------------------------
**Short Story**: Speed

:raw-html:`<br />`

**Long Story**:

The stories below are what motivated for this change:

:raw-html:`<br />`

.. list-table::
   :widths: 33 33 34
   :header-rows: 1

   * - Story
     - Before (Old API)
     - After (New API)
   * - | When mega-merged mods started appearing, they were not optimized to use multiple ``draw-indexed`` lines in a section path.
       | Many simply use a merge script or a namespace merge script that naively combines a bunch of mod variants into a giant .ini.
       |
       | Let :math:`n` be the number of toggles
       | Let :math:`c_i` be the number of variants for toggle :math:`i`
       |
       | The number of mods a modder needs to create would be 
       |
       | :math:`\prod_{i=1}^{n} c_i \le \left( \max_{1 \le i \le n} c_i \right)^{n}`
       |
       | So a modder has to create every possible toggle combination of mods. This quickly bloats out the size of a mod to be several GBs in size.
       |
       | An example of such mods that we have tested for this story would be `this Ayaka mod`_.
       |
       | We did some basic benchmarking of the old API and new API for fixing mods at this `mod fix benchmark PR`_
     - That ayaka mod would take about 5 minutes (244.2 s) to fix. It would hang a lot from parsing and fixing some 20K line merged.ini.
     - | Now, that same mod would take around 2 minutes (96.8 s) to fix, about 2.5x faster than the old API. Funny part is that the new API actually does a lot more heavier graph computation for the .ini for better accuracy (eg. precise insertion of ORFix)
       | The new API is also a more scalable, gaining more speedup compared to the old API the larger the .ini file is.
   * - | Sometimes to analyze mods, we would need to run a mod through the `Mod to Dump Converter Notebook`_ which utilizes the API, so that we can debug and view the mod in `Blender`_. The issue is that we would need to iterate through each
       | part of the frame of a vertex and convert it to a string representation.
       |
       | we did some basic benchmark comparisons of the conversion at this `conversion benchmark`_
     - | Since the old API was manually looping through a frame, it would take about 1 minute (37.93 s) to convert some Hutao mod with about 100000 vertices.
     - | The new API was lightning fast in the conversion, all within a few seconds. That same Hutao mod took only 1.24 s to convert, which is about 30x speedup. For an extreme case, some large Nilou mesh that took the around 5 minutes to convert (240.09 s)
       | in the old API now takes only 4.87s to convert in the new API, which is a 128.6x speedup. From this speedup increase, we see the new API maintains its scalability, the larger the mesh graph. 
   * - | To support remapping `multi component mods`_, we encounter of challenge of `how to collect resources that depend on each other`_. At the end, we found that this challenge essentially reduces to the `SAT (or 3SAT) NP complete problem`_. So we would need
       | use some SAT solver in the remap. For the case of :doc:`3dmigto's .ini language <parseIni>`, this is some `Satisfiability Modulo Theory (SMT)`_ in the context of `Non-linear Integer Programming (NILP)`_
       |
       | The problem is that we need to answer the satisfiability question for a huge number of logic queries.
       | As much as want to come up with some very smart solution with some clever trick that can answer the solve the question in :math:`O(n)` queries, where :math:`n` is the max number of resource instances for a particular type of resource, that might be too hard or maybe impossible.
       | We can't even use a comparison model to have :math:`O(n \log n)` queries since there is not really any notion of comparison of ``greater than/lesser than`` between logic queries.
       | So we are stuck with :math:`O(n^2)` queries, where we always need to compare between the set of currently running satisfiable resource groups and the set of resources for the current resource type.
       |
       | *eg.*
       | If there was a mod with 100 toggle variants that is created using some standard merge script and we need some resource group that requires a *Blend.buf*, *Position.buf* and a *Texcoord.buf* file. We would need to check :math:`100^2 < 100^3 = 1000000` different logic queries.
       |
       | **In general:** 
       | For the worse case, if a resource group has :math:`m` different types of resources and :math:`n` is the max number of resource instances for all resource types, then the number of logic queries needed to be computed would be :math:`O(n^m)`
       | But for typical merged mods created using merge scripts, many combinations of resources are not satisfiable, which reduces the number of queries needed to be computed to around :math:`O(n^2)`
     - | Before migration, we decided to use `Sympy`_ since:
       |
       | - It satisifies the flavour of `SMT`_ we want to solve
       | - User's do not need to install anything extra to their computer (eg. a C compiler)
       |
       | However, we later found that `Sympy`_ was purely implemented in Python and for our context of `SMT`_, `Sympy`_ does not provide any faster optimizations (eg. running some other engine in C/C++).
       | I can't imagine how slow it would be when doing a remap for a mod with 100+ toggle variants
     - | We decided to use `Z3`_ which exactly solves our issue and its very fast. For a large mega-merged mod such as `this Ayaka mod`_, it can simplify 200 queries to simplest terms within 1-2s.


.. _old API: https://anime-game-remap.readthedocs.io/en/v4.6.0/api.html
.. _this Ayaka mod: https://gamebanana.com/mods/506454
.. _mod fix benchmark PR: https://github.com/nhok0169/Anime-Game-Remap/pull/235
.. _Blender: https://www.blender.org/
.. _Mod to Dump Converter Notebook: https://github.com/nhok0169/Anime-Game-Remap/blob/master/Tools/ModToDumpConverter/GI/GIModToDumpConverter.ipynb
.. _conversion benchmark: https://github.com/nhok0169/Anime-Game-Remap/issues/211#issuecomment-5537903217
.. _multi component mods: https://github.com/nhok0169/Anime-Game-Remap/issues/192
.. _how to collect resources that depend on each other: https://github.com/nhok0169/Anime-Game-Remap/issues/190#issuecomment-3209337247
.. _SAT (or 3SAT) NP complete problem: https://en.wikipedia.org/wiki/Boolean_satisfiability_problem
.. _Satisfiability Modulo Theory (SMT): https://en.wikipedia.org/wiki/Satisfiability_modulo_theories
.. _SMT: https://en.wikipedia.org/wiki/Satisfiability_modulo_theories
.. _Non-linear Integer Programming (NILP): https://en.wikipedia.org/wiki/Nonlinear_programming
.. _Z3: https://github.com/Z3Prover/z3
.. _Sympy: https://www.sympy.org/en/index.html