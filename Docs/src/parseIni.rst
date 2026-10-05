.. role:: raw-html(raw)
    :format: html

.. role:: strike
   :class: strike

Simplified 3dmigoto Ini (S3DMIni) Language Specification
========================================================

To be honest, this page should instead be written by the creators of `3dmigoto`_ or `GIMI`_, not me.
There already exists many documentation that explain the semantics of how to use 3dmigoto's Ini language (eg. how to do TextureOverrides/ShaderOverrides, etc...).
However, I find that there is no adequate documentation that specifies the gory details of all the language features
for someone who wants to build their own tool that fixes 3dmigoto .ini files or maybe someone who wants to build their own compiler for the language.

:raw-html:`<br />`


Simplified 3dmigoto Ini (S3DMIni) Vs. 3dmigoto Ini (3DMIni)
***********************************************************

Overall, the 3dmigoto Ini language is a pretty interesting **programming** language for richer expression of mods. But without any previous language specifications,
most of the stuff below are discovered from my own testing of the language. From these testing results, `AGRemap`_ assumes .ini files use a variant of the 3dmigoto Ini
language called the Simplified 3dmigoto Ini language that is:

- Expressive enough for off-the-shelf mods
- Simple to understand for people with a programming background

:raw-html:`<br />`

The table below lists some (not all) details on the differences between both languages:

.. list-table::
   :widths: 50 50
   :header-rows: 1

   * - 3DMIni
     - S3DMIni
   * - Some specific `KVPs`_ such as those with the key names ``hash`` or ``match_first_index`` completely ignore
       the ``if...else...`` grammar. Instead the first instance of those `KVPs`_ from some root `section`_ will be used.

       :raw-html:`<br />`

       For example, if you do something like this:

       .. code-block:: ini
           :linenos:

           [sectionRoot]
           if $x == 8 || $y > 0
               hash = deadbeef
           else
               run = subSection
           endif

           [subSection]
           hash = baddbabe
           if $x + 3 <= 9
               hash = deaddead
           endif

       :raw-html:`<br />`

       For both ``sectionRoot`` and ``subSection``, the ``hash`` value is always ``deadbeef`` regardless of what key swap you press

     - All `KVPs`_ obey the ``if...else...`` grammar introduced in 3DMIni
   * - Terms in a predicate can accept a variety of different datatypes including objects
     - The datatypes for a term in a predicate can only be a variable, integer or a float

:raw-html:`<br />`
:raw-html:`<br />`

How AGRemap Parses S3DMIni
**************************

Overall, we use a hybrid of adhoc and classical parsing to parse the .ini files. The adhoc part is for parsing the overall structure of the .ini
file into a `Control Flow Graph`_ while the classical part uses `SLR parsing`_ to makes sense of the predicates.

Below are the details for each part:

:raw-html:`<br />`

Adhoc Ini Parsing
-----------------
Below are the observed specifications for the general shape of a 3dmigoto .ini file:

:raw-html:`<br />`

**Lines**

- The file is read line by line. Leading and trailing whitespace on every line is ignored, so indentation has no meaning
- Blank lines are ignored
- A line whose first non-whitespace character is ``;`` or ``#`` is a comment and is ignored
- Comments only take up a whole line. A ``;`` after a value is part of the value (eg. ``hash = deadbeef ; body`` has the value ``deadbeef ; body``)

:raw-html:`<br />`

**Sections**

- A line starting with ``[`` and containing a ``]`` is a `section`_ header. The section's name is the text between the first ``[`` and the last ``]``, with
  the whitespace around it trimmed (eg. ``[   TextureOverrideBody  ]`` is the section ``TextureOverrideBody``)
- A section runs from its header line until the next header line or the end of the file
- Any line before the first section header is ignored
- Section names are case-sensitive
- If a section name is declared more than once, only its first declaration is used

:raw-html:`<br />`

**Key-Value Pairs (KVPs)**

- Any other line inside a section is a `KVP`_ of the form ``key = value``. The line is split at its **first** ``=`` (or ``:``), and the whitespace around
  both the key and the value is trimmed
- Keys keep their case (``Filename`` is not the same key as ``filename``)
- Values are taken literally: there is no interpolation, no escaping and no multi-line continuation
- A key may appear more than once in a section. Every occurrence is kept, in the order it appears (eg. several ``drawindexed`` or ``run`` lines in one section)
- A line with no ``=`` (eg. 3dmigoto's ``local $var`` declaration) is kept as a key with an empty value

:raw-html:`<br />`

**if...else...endif Blocks**

- Inside a section, a line whose first word is ``if``, ``else if``, ``elif``, ``else`` or ``endif`` is a conditional line rather than a `KVP`_. These keywords
  are **case-insensitive** (``ELSE IF`` and ``EndIf`` are both valid)
- ``if <predicate>`` opens a block, ``else if <predicate>`` / ``elif <predicate>`` and ``else`` add further branches to it, and ``endif`` closes it
- The predicate after ``if`` / ``else if`` / ``elif`` is the rest of the line, and is parsed separately (see `Ini Predicate Parsing`_ below)
- Blocks can be nested to any depth
- Every `KVP`_ belongs to the innermost branch it sits in, and only takes effect when the predicates of all the branches enclosing it are satisfied
- A block does not extend past the end of its section
- Since a conditional line is recognized by how it starts, a key cannot start with one of these keywords

.. code-block:: ini
    :linenos:

    [TextureOverrideBody]
    hash = deadbeef
    if $swapvar == 0
        ps-t0 = ResourceBodyDiffuse
        if $glow == 1
            ps-t1 = ResourceBodyGlowLightMap
        else
            ps-t1 = ResourceBodyLightMap
        endif
    else if $swapvar == 1
        ps-t0 = ResourceBodyDiffuseAlt
    endif
    drawindexed = auto

:raw-html:`<br />`

**Section Chains Through run**

- The value of a ``run`` `KVP`_ names another section in the same file (usually a ``CommandList...`` section). When the line is reached, the
  named section's content is executed in its place, as if it were a function call
- A ``run`` line obeys the ``if...else...endif`` blocks around it like any other `KVP`_, so a call may only happen under some predicates
- The called section can itself contain ``if...else...endif`` blocks and further ``run`` lines, so `sections`_ form a chain (a call graph) starting
  from a root section such as a ``TextureOverride...`` section
- The predicates of the branches a ``run`` line sits in also apply to everything inside the called section
- A section may be called from several places, and the calls may form a cycle (a section that, directly or through other sections, runs itself)
- A ``run`` value that does not name a section in the file (eg. ``run = CommandList\global\ORFix\ORFix``, which lives in a separate library .ini)
  is an external call: it is kept as is, but is not followed

.. code-block:: ini
    :linenos:

    [TextureOverrideBody]
    hash = deadbeef
    run = CommandListBody

    [CommandListBody]
    if $swapvar == 0
        ib = ResourceBodyIB
        run = CommandListBodyTextures
    endif

    [CommandListBodyTextures]
    ps-t0 = ResourceBodyDiffuse
    run = CommandList\global\ORFix\ORFix
    drawindexed = auto

:raw-html:`<br />`
:raw-html:`<br />`


Ini Predicate Parsing
---------------------
For the predicates that folow the ``if`` keyword, we use classical techniques of:

- Tokenization
- `Parsing (Bottom up)`_
- :strike:`Type Checking`
- Code Generation

:raw-html:`<br />`

Lexical Syntax (Tokenization)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A S3DMIni predicate contains *tokens* that are seperate by whitespace (SPACE or TAB). A valid token is in the following table below:

.. tip::
    You can use our tokenizer that implements the specification below over here:

    - Python: :class:`FixRaidenBoss2.IfPredTokenizer`
    - C++: :cpp:class:`AGRemapCore::IfPredTokenizer`

.. tip::
    You can also use our custom tokenizer for your own language over here:

    - The base tokenizer, which you set up with your own tokens, states and transitions:

      - Python: :class:`FixRaidenBoss2.BaseTokenizer`
      - C++: :cpp:class:`AGRemapCore::BaseTokenizer`

    - The same tokenizer, but able to discard some tokens from its output (eg. whitespace):

      - Python: :class:`FixRaidenBoss2.FilteredTokenizer`
      - C++: :cpp:class:`AGRemapCore::FilteredTokenizer`

:raw-html:`<br />`

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Token
     - Description
   * - ID
     - | A variable: a ``$`` followed by one or more letters, digits or any of the symbols ``% , . : ? @ [ \ ] ^ _ ` { } ~``
       |
       | eg. ``$swapvar``, ``$\mods\Master\active``, ``$state_id_0``
   * - INT
     - | An integer: one or more digits, optionally led by a ``-``
       |
       | eg. ``0``, ``42``, ``-5``
   * - FLOAT
     - | A decimal number: an integer, a ``.``, then one or more digits
       |
       | eg. ``0.5``, ``-1.25`` (``1.`` and ``.5`` are not valid)
   * - NULL
     - The keyword ``null`` (lowercase only)
   * - PLUS
     - ``+``
   * - MINUS
     - | ``-``
       |
       | A ``-`` directly followed by a digit is read as the start of an INT or FLOAT instead,
       | so ``5 - 3`` is ``INT MINUS INT`` while ``5-3`` is ``INT INT`` (``5`` and ``-3``)
   * - STAR
     - ``*``
   * - SLASH
     - ``/``
   * - LPAREN
     - ``(``
   * - RPAREN
     - ``)``
   * - EQ
     - ``==``
   * - NE
     - ``!=``
   * - LT
     - ``<``
   * - GT
     - ``>``
   * - LE
     - ``<=``
   * - GE
     - ``>=``
   * - AND
     - ``&&``
   * - OR
     - ``||``
   * - NOT
     - ``!``
   * - SPACE
     - | A space character
       |
       | Only separates other tokens, and is discarded by the tokenizer
   * - TAB
     - | A tab character
       |
       | Only separates other tokens, and is discarded by the tokenizer

:raw-html:`<br />`

Context Free Syntax (Bottom up Parsing)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A `Context-Free Grammar`_ for a S3DMIni predicate is the following:

.. tip::
    You can use our parser that implements the specification below:

    - Python: :class:`FixRaidenBoss2.IfPredParser`
    - C++: :cpp:class:`AGRemapCore::IfPredParser`

.. tip::
    You can use our custom `SLR parser`_ to do `bottom-up parsing`_ on your own language over here:

    - Python: :class:`FixRaidenBoss2.BaseSLR1Parser`
    - C++: :cpp:class:`AGRemapCore::BaseSLR1Parser`

:raw-html:`<br />`

- terminal symbols: The set of valid tokens above (except SPACE and TAB, which the tokenizer discards), plus the 2 tokens the parser wraps around
  every predicate: ``STARTTOKEN`` and ``ENDTOKEN``
- nonterminal symbols: ``pred_prime``, ``pred``, ``test``, ``keyexpr``, ``addexpr``, ``multexpr``, ``nterm``, ``term``
- start symbol: ``pred_prime``
- production rules:

  - ``pred_prime`` → ``STARTTOKEN`` ``pred`` ``ENDTOKEN``
  - ``pred_prime`` → ``STARTTOKEN`` ``ENDTOKEN``
  - ``pred`` → ``test``
  - ``pred`` → ``pred`` ``AND`` ``test``
  - ``pred`` → ``pred`` ``OR`` ``test``
  - ``test`` → ``keyexpr``
  - ``test`` → ``keyexpr`` ``EQ`` ``keyexpr``
  - ``test`` → ``keyexpr`` ``NE`` ``keyexpr``
  - ``test`` → ``keyexpr`` ``GT`` ``keyexpr``
  - ``test`` → ``keyexpr`` ``GE`` ``keyexpr``
  - ``test`` → ``keyexpr`` ``LT`` ``keyexpr``
  - ``test`` → ``keyexpr`` ``LE`` ``keyexpr``
  - ``keyexpr`` → ``addexpr``
  - ``keyexpr`` → ``NULL``
  - ``addexpr`` → ``multexpr``
  - ``addexpr`` → ``addexpr`` ``PLUS`` ``multexpr``
  - ``addexpr`` → ``addexpr`` ``MINUS`` ``multexpr``
  - ``multexpr`` → ``nterm``
  - ``multexpr`` → ``multexpr`` ``STAR`` ``nterm``
  - ``multexpr`` → ``multexpr`` ``SLASH`` ``nterm``
  - ``nterm`` → ``term``
  - ``nterm`` → ``NOT`` ``nterm``
  - ``term`` → ``ID``
  - ``term`` → ``INT``
  - ``term`` → ``FLOAT``
  - ``term`` → ``LPAREN`` ``pred`` ``RPAREN``

:raw-html:`<br />`

Some behaviours of the grammar above worth noting:

- ``&&`` and ``||`` have the **same** precedence and are left-associative. Unlike C, ``&&`` does not bind tighter than ``||``, so
  ``$a || $b && $c`` means ``($a || $b) && $c``. Use brackets to group them any other way
- ``!`` binds the tightest of all the operators, and applies to a single term or a bracketed group (eg. ``!$a == 1`` means ``(!$a) == 1``)
- ``*`` and ``/`` bind tighter than ``+`` and ``-``, and all 4 are left-associative
- Comparisons cannot be chained: ``$a == $b == $c`` is a syntax error. Write ``$a == $b && $b == $c`` instead
- ``null`` can only be a whole side of a comparison (eg. ``$a == null``). It cannot be used in arithmetic
- An empty predicate is valid


:raw-html:`<br />`


Context Sensive Syntax (Type Checking)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
No type checking needed, everything is a number (int or float)!

:raw-html:`<br />`


Code Generation
^^^^^^^^^^^^^^^
Based on the parse tree derived from the parsing above, we provide several code generators that converts the parse tree into some useful format. These formats include:

- `Sympy`_: A `sympy logic query`_

  - Python: :class:`FixRaidenBoss2.IfPredLogicGenerator`

- `Z3`_: A `Z3`_ predicate

  - C++: :cpp:class:`AGRemapCore::IfPredZ3Generator`


:raw-html:`<br />`

.. tip::
    We also have the tools for compiling a `Sympy`_ or a `Z3`_ predicate back to a S3DMIni predicate over here:

    - `Sympy`_: The string of a `sympy logic query`_ is tokenized, parsed, then generated back into a S3DMIni predicate

      - Tokenizer:

        - Python: :class:`FixRaidenBoss2.SympyTokenizer`
        - C++: :cpp:class:`AGRemapCore::SympyTokenizer`

      - Parser:

        - Python: :class:`FixRaidenBoss2.SympyParser`
        - C++: :cpp:class:`AGRemapCore::SympyParser`

      - Generator:

        - Python: :class:`FixRaidenBoss2.SympyIfPredGenerator`

    - `Z3`_: A `Z3`_ predicate is already an expression tree, so no tokenizer or parser is needed. The generator walks the predicate directly

      - Generator:

        - C++: :cpp:class:`AGRemapCore::Z3IfPredGenerator`




.. _GIMI: https://github.com/SilentNightSound/GI-Model-Importer
.. _3dmigoto: https://github.com/bo3b/3Dmigoto
.. _AGRemap: https://github.com/nhok0169/Anime-Game-Remap
.. _SLR parsing: https://en.wikipedia.org/wiki/Simple_LR_parser
.. _SLR parser: https://en.wikipedia.org/wiki/Simple_LR_parser
.. _KVP: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _KVPs: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _Parsing (Bottom up): https://en.wikipedia.org/wiki/Bottom-up_parsing
.. _bottom-up parsing: https://en.wikipedia.org/wiki/Bottom-up_parsing
.. _Context-Free Grammar: https://en.wikipedia.org/wiki/Context-free_grammar
.. _Control Flow Graph: https://en.wikipedia.org/wiki/Control-flow_graph
.. _section: https://en.wikipedia.org/wiki/INI_file#Sections
.. _sections: https://en.wikipedia.org/wiki/INI_file#Sections
.. _Sympy: https://www.sympy.org/en/index.html
.. _Z3: https://github.com/Z3Prover/z3
.. _sympy logic query: https://docs.sympy.org/latest/modules/logic.html
