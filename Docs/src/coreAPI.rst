.. role:: raw-html(raw)
    :format: html

======================
C++ Core API Reference
======================

:raw-html:`<br />`
:raw-html:`<br />`

Model
*****

Classes that represent the actual mod-fixing domain -- the ``.ini`` "if template" structure and the
``.buf`` binary vertex file format, below. Contrast with `Tools`_ further down, which has no notion
of what a "mod" or a ``.ini``/``.buf`` file even is.

:raw-html:`<br />`

Buf Files
=========

:raw-html:`<br />`

BinaryFile
----------

.. cppattributetable:: AGRemapCore::BinaryFile

.. doxygenclass:: AGRemapCore::BinaryFile
    :members:
    :protected-members:

:raw-html:`<br />`

BlendFile
---------

.. cppattributetable:: AGRemapCore::BlendFile

.. doxygenclass:: AGRemapCore::BlendFile
    :members:
    :protected-members:

:raw-html:`<br />`

BufBaseFloat
------------

.. cppattributetable:: AGRemapCore::BufBaseFloat

.. doxygenclass:: AGRemapCore::BufBaseFloat
    :members:
    :protected-members:

:raw-html:`<br />`

BufBaseInt
----------

.. cppattributetable:: AGRemapCore::BufBaseInt

.. doxygenclass:: AGRemapCore::BufBaseInt
    :members:
    :protected-members:

:raw-html:`<br />`

BufDataType
-----------

.. cppattributetable:: AGRemapCore::BufDataType

.. doxygenclass:: AGRemapCore::BufDataType
    :members:
    :protected-members:

:raw-html:`<br />`

BufElementType
--------------

.. cppattributetable:: AGRemapCore::BufElementType

.. doxygenclass:: AGRemapCore::BufElementType
    :members:
    :protected-members:

:raw-html:`<br />`

BufFile
-------

.. cppattributetable:: AGRemapCore::BufFile

.. doxygenclass:: AGRemapCore::BufFile
    :members:
    :protected-members:

:raw-html:`<br />`

BufFloat
--------

.. cppattributetable:: AGRemapCore::BufFloat

.. doxygenclass:: AGRemapCore::BufFloat
    :members:
    :protected-members:

:raw-html:`<br />`

BufFloat16
----------

.. cppattributetable:: AGRemapCore::BufFloat16

.. doxygenclass:: AGRemapCore::BufFloat16
    :members:
    :protected-members:

:raw-html:`<br />`

BufSignedInt
------------

.. cppattributetable:: AGRemapCore::BufSignedInt

.. doxygenclass:: AGRemapCore::BufSignedInt
    :members:
    :protected-members:

:raw-html:`<br />`

BufType
-------

.. cppattributetable:: AGRemapCore::BufType

.. doxygenclass:: AGRemapCore::BufType
    :members:
    :protected-members:

:raw-html:`<br />`

BufUnorm
--------

.. cppattributetable:: AGRemapCore::BufUnorm

.. doxygenclass:: AGRemapCore::BufUnorm
    :members:
    :protected-members:

:raw-html:`<br />`

BufUnSignedInt
--------------

.. cppattributetable:: AGRemapCore::BufUnSignedInt

.. doxygenclass:: AGRemapCore::BufUnSignedInt
    :members:
    :protected-members:

:raw-html:`<br />`

PositionFile
------------

.. cppattributetable:: AGRemapCore::PositionFile

.. doxygenclass:: AGRemapCore::PositionFile
    :members:
    :protected-members:

:raw-html:`<br />`

VGRemap
-------

.. cppattributetable:: AGRemapCore::VGRemap

.. doxygenclass:: AGRemapCore::VGRemap
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Constants
=========

:raw-html:`<br />`

GameTypeId
----------

.. doxygenenum:: AGRemapCore::GameTypeId

:raw-html:`<br />`

GameTypeIdTools
---------------

.. cppattributetable:: AGRemapCore::GameTypeIdTools

.. doxygenclass:: AGRemapCore::GameTypeIdTools
    :members:
    :protected-members:

:raw-html:`<br />`

GIBuilder
---------

.. cppattributetable:: AGRemapCore::GIBuilder

.. doxygenclass:: AGRemapCore::GIBuilder
    :members:
    :protected-members:

:raw-html:`<br />`

ModTypeId
---------

.. doxygenenum:: AGRemapCore::ModTypeId

:raw-html:`<br />`

ModTypeIdTools
--------------

.. cppattributetable:: AGRemapCore::ModTypeIdTools

.. doxygenclass:: AGRemapCore::ModTypeIdTools
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

If Templates
============

:raw-html:`<br />`

CallGraph
---------

.. cppattributetable:: AGRemapCore::CallGraph

.. doxygenclass:: AGRemapCore::CallGraph
    :members:
    :protected-members:

:raw-html:`<br />`

IfContentPart
-------------

.. cppattributetable:: AGRemapCore::IfContentPart

.. doxygenclass:: AGRemapCore::IfContentPart
    :members:
    :protected-members:

:raw-html:`<br />`

IfContentPartColourChange
--------------------------

.. cppattributetable:: AGRemapCore::IfContentPartColourChange

.. doxygenclass:: AGRemapCore::IfContentPartColourChange
    :members:
    :protected-members:

:raw-html:`<br />`

IfContentPartColouring
-----------------------

.. cppattributetable:: AGRemapCore::IfContentPartColouring

.. doxygenclass:: AGRemapCore::IfContentPartColouring
    :members:
    :protected-members:

:raw-html:`<br />`

IfPredPart
----------

.. cppattributetable:: AGRemapCore::IfPredPart

.. doxygenclass:: AGRemapCore::IfPredPart
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplate
----------

.. cppattributetable:: AGRemapCore::IfTemplate

.. doxygenclass:: AGRemapCore::IfTemplate
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplateNode
--------------

.. cppattributetable:: AGRemapCore::IfTemplateNode

.. doxygenclass:: AGRemapCore::IfTemplateNode
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplateNonEmptyNodeTree
--------------------------

.. cppattributetable:: AGRemapCore::IfTemplateNonEmptyNodeTree

.. doxygenclass:: AGRemapCore::IfTemplateNonEmptyNodeTree
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplateNormTree
------------------

.. cppattributetable:: AGRemapCore::IfTemplateNormTree

.. doxygenclass:: AGRemapCore::IfTemplateNormTree
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplatePart
--------------

.. cppattributetable:: AGRemapCore::IfTemplatePart

.. doxygenclass:: AGRemapCore::IfTemplatePart
    :members:
    :protected-members:

:raw-html:`<br />`

IfTemplateTree
--------------

.. cppattributetable:: AGRemapCore::IfTemplateTree

.. doxygenclass:: AGRemapCore::IfTemplateTree
    :members:
    :protected-members:

:raw-html:`<br />`

IniSectionGraph
---------------

.. cppattributetable:: AGRemapCore::IniSectionGraph

.. doxygenclass:: AGRemapCore::IniSectionGraph
    :members:
    :protected-members:

:raw-html:`<br />`

SectionIterData
----------------

.. cppattributetable:: AGRemapCore::SectionIterData

.. doxygenclass:: AGRemapCore::SectionIterData
    :members:
    :protected-members:

:raw-html:`<br />`

SectionIterQueryData
----------------------

.. cppattributetable:: AGRemapCore::SectionIterQueryData

.. doxygenclass:: AGRemapCore::SectionIterQueryData
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Ini Classifiers
===============

:raw-html:`<br />`

BaseIniClassifier
-----------------

.. cppattributetable:: AGRemapCore::BaseIniClassifier

.. doxygenclass:: AGRemapCore::BaseIniClassifier
    :members:
    :protected-members:

:raw-html:`<br />`

IniClassifier
-------------

.. cppattributetable:: AGRemapCore::IniClassifier

.. doxygenclass:: AGRemapCore::IniClassifier
    :members:
    :protected-members:

:raw-html:`<br />`

IniClassifyStats
----------------

.. cppattributetable:: AGRemapCore::IniClassifyStats

.. doxygenclass:: AGRemapCore::IniClassifyStats
    :members:
    :protected-members:

:raw-html:`<br />`

ModType
-------

.. cppattributetable:: AGRemapCore::ModType

.. doxygenclass:: AGRemapCore::ModType
    :members:
    :protected-members:

:raw-html:`<br />`

ModTypeIdData
-------------

.. cppattributetable:: AGRemapCore::ModTypeIdData

.. doxygenclass:: AGRemapCore::ModTypeIdData
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Predicate Parsers
==================

:raw-html:`<br />`

IfPredParser
------------

.. cppattributetable:: AGRemapCore::IfPredParser

.. doxygenclass:: AGRemapCore::IfPredParser
    :members:
    :protected-members:

:raw-html:`<br />`

SympyParser
-----------

.. cppattributetable:: AGRemapCore::SympyParser

.. doxygenclass:: AGRemapCore::SympyParser
    :members:
    :protected-members:

:raw-html:`<br />`


Predicate Tokenizers
=====================

:raw-html:`<br />`

IfPredTokenizer
----------------

.. cppattributetable:: AGRemapCore::IfPredTokenizer

.. doxygenclass:: AGRemapCore::IfPredTokenizer
    :members:
    :protected-members:

:raw-html:`<br />`

SympyTokenizer
---------------

.. cppattributetable:: AGRemapCore::SympyTokenizer

.. doxygenclass:: AGRemapCore::SympyTokenizer
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Reg Edits
=========

:raw-html:`<br />`

BaseIniGraphPartEdit
--------------------

.. cppattributetable:: AGRemapCore::BaseIniGraphPartEdit

.. doxygenclass:: AGRemapCore::BaseIniGraphPartEdit
    :members:
    :protected-members:

:raw-html:`<br />`

BaseIniPartEdit
---------------

.. cppattributetable:: AGRemapCore::BaseIniPartEdit

.. doxygenclass:: AGRemapCore::BaseIniPartEdit
    :members:
    :protected-members:

:raw-html:`<br />`

BaseRegEdit
-----------

.. cppattributetable:: AGRemapCore::BaseRegEdit

.. doxygenclass:: AGRemapCore::BaseRegEdit
    :members:
    :protected-members:

:raw-html:`<br />`

RegAdd
------

.. cppattributetable:: AGRemapCore::RegAdd

.. doxygenclass:: AGRemapCore::RegAdd
    :members:
    :protected-members:

:raw-html:`<br />`

RegNewVals
----------

.. cppattributetable:: AGRemapCore::RegNewVals

.. doxygenclass:: AGRemapCore::RegNewVals
    :members:
    :protected-members:

:raw-html:`<br />`

RegRemap
--------

.. cppattributetable:: AGRemapCore::RegRemap

.. doxygenclass:: AGRemapCore::RegRemap
    :members:
    :protected-members:

:raw-html:`<br />`

RegRemove
---------

.. cppattributetable:: AGRemapCore::RegRemove

.. doxygenclass:: AGRemapCore::RegRemove
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Textures
========

:raw-html:`<br />`

BasePixelTransform
------------------

.. cppattributetable:: AGRemapCore::BasePixelTransform

.. doxygenclass:: AGRemapCore::BasePixelTransform
    :members:
    :protected-members:

:raw-html:`<br />`

BaseTexEditor
-------------

.. cppattributetable:: AGRemapCore::BaseTexEditor

.. doxygenclass:: AGRemapCore::BaseTexEditor
    :members:
    :protected-members:

:raw-html:`<br />`

BaseTexFilter
-------------

.. cppattributetable:: AGRemapCore::BaseTexFilter

.. doxygenclass:: AGRemapCore::BaseTexFilter
    :members:
    :protected-members:

:raw-html:`<br />`

Colour
------

.. cppattributetable:: AGRemapCore::Colour

.. doxygenclass:: AGRemapCore::Colour
    :members:
    :protected-members:

:raw-html:`<br />`

ColourRange
-----------

.. cppattributetable:: AGRemapCore::ColourRange

.. doxygenclass:: AGRemapCore::ColourRange
    :members:
    :protected-members:

:raw-html:`<br />`

ColourReplace
-------------

.. cppattributetable:: AGRemapCore::ColourReplace

.. doxygenclass:: AGRemapCore::ColourReplace
    :members:
    :protected-members:

:raw-html:`<br />`

ColourReplaceFilter
-------------------

.. cppattributetable:: AGRemapCore::ColourReplaceFilter

.. doxygenclass:: AGRemapCore::ColourReplaceFilter
    :members:
    :protected-members:

:raw-html:`<br />`

CorrectGamma
------------

.. cppattributetable:: AGRemapCore::CorrectGamma

.. doxygenclass:: AGRemapCore::CorrectGamma
    :members:
    :protected-members:

:raw-html:`<br />`

GammaFilter
-----------

.. cppattributetable:: AGRemapCore::GammaFilter

.. doxygenclass:: AGRemapCore::GammaFilter
    :members:
    :protected-members:

:raw-html:`<br />`

HighlightShadow
---------------

.. cppattributetable:: AGRemapCore::HighlightShadow

.. doxygenclass:: AGRemapCore::HighlightShadow
    :members:
    :protected-members:

:raw-html:`<br />`

HueAdjust
---------

.. cppattributetable:: AGRemapCore::HueAdjust

.. doxygenclass:: AGRemapCore::HueAdjust
    :members:
    :protected-members:

:raw-html:`<br />`

InvertAlpha
-----------

.. cppattributetable:: AGRemapCore::InvertAlpha

.. doxygenclass:: AGRemapCore::InvertAlpha
    :members:
    :protected-members:

:raw-html:`<br />`

InvertAlphaFilter
-----------------

.. cppattributetable:: AGRemapCore::InvertAlphaFilter

.. doxygenclass:: AGRemapCore::InvertAlphaFilter
    :members:
    :protected-members:

:raw-html:`<br />`

PixelFilter
-----------

.. cppattributetable:: AGRemapCore::PixelFilter

.. doxygenclass:: AGRemapCore::PixelFilter
    :members:
    :protected-members:

:raw-html:`<br />`

TempControl
-----------

.. cppattributetable:: AGRemapCore::TempControl

.. doxygenclass:: AGRemapCore::TempControl
    :members:
    :protected-members:

:raw-html:`<br />`

TexCreator
----------

.. cppattributetable:: AGRemapCore::TexCreator

.. doxygenclass:: AGRemapCore::TexCreator
    :members:
    :protected-members:

:raw-html:`<br />`

TexEditor
---------

.. cppattributetable:: AGRemapCore::TexEditor

.. doxygenclass:: AGRemapCore::TexEditor
    :members:
    :protected-members:

:raw-html:`<br />`

TextureFile
-----------

.. cppattributetable:: AGRemapCore::TextureFile

.. doxygenclass:: AGRemapCore::TextureFile
    :members:
    :protected-members:

:raw-html:`<br />`

TintTransform
-------------

.. cppattributetable:: AGRemapCore::TintTransform

.. doxygenclass:: AGRemapCore::TintTransform
    :members:
    :protected-members:

:raw-html:`<br />`

Transparency
------------

.. cppattributetable:: AGRemapCore::Transparency

.. doxygenclass:: AGRemapCore::Transparency
    :members:
    :protected-members:

:raw-html:`<br />`

TransparencyAdjustFilter
-------------------------

.. cppattributetable:: AGRemapCore::TransparencyAdjustFilter

.. doxygenclass:: AGRemapCore::TransparencyAdjustFilter
    :members:
    :protected-members:

:raw-html:`<br />`
:raw-html:`<br />`

Tools
*****

Generic, reusable-outside-this-project building blocks -- data structures, algorithms, and
string/hash/graph utilities with no notion of what a "mod" or a ``.ini`` file even is. Contrast
with `Model`_ above, which is specifically about the mod-fixing domain.

:raw-html:`<br />`

Algo
=====

.. cppattributetable:: AGRemapCore::Algo

.. doxygenclass:: AGRemapCore::Algo
    :members:
    :protected-members:

:raw-html:`<br />`

BaseIdGenerator
===============

.. cppattributetable:: AGRemapCore::BaseIdGenerator

.. doxygenclass:: AGRemapCore::BaseIdGenerator
    :members:
    :protected-members:

:raw-html:`<br />`

BiMap
=====

.. cppattributetable:: AGRemapCore::BiMap

.. doxygenclass:: AGRemapCore::BiMap
    :members:
    :protected-members:

:raw-html:`<br />`


DFAs and Tries
==============

:raw-html:`<br />`

BaseAhoCorasickDFA
------------------

.. cppattributetable:: AGRemapCore::BaseAhoCorasickDFA

.. doxygenclass:: AGRemapCore::BaseAhoCorasickDFA
    :members:
    :protected-members:

:raw-html:`<br />`

BaseDFA
-------

.. cppattributetable:: AGRemapCore::BaseDFA

.. doxygenclass:: AGRemapCore::BaseDFA
    :members:
    :protected-members:

:raw-html:`<br />`

BaseTrie
--------

.. cppattributetable:: AGRemapCore::BaseTrie

.. doxygenclass:: AGRemapCore::BaseTrie
    :members:
    :protected-members:

:raw-html:`<br />`


GraphemeIterator
================

.. cppattributetable:: AGRemapCore::GraphemeIterator

.. doxygenclass:: AGRemapCore::GraphemeIterator
    :members:
    :protected-members:

:raw-html:`<br />`

GraphemeRange
==============

.. cppattributetable:: AGRemapCore::GraphemeRange

.. doxygenclass:: AGRemapCore::GraphemeRange
    :members:
    :protected-members:

:raw-html:`<br />`


Hashing
=======

:raw-html:`<br />`

Hash128
-------

.. cppattributetable:: AGRemapCore::Hash128

.. doxygenclass:: AGRemapCore::Hash128
    :members:
    :protected-members:

:raw-html:`<br />`

Hash64
------

.. cppattributetable:: AGRemapCore::Hash64

.. doxygenclass:: AGRemapCore::Hash64
    :members:
    :protected-members:

:raw-html:`<br />`

HashInt
-------

.. cppattributetable:: AGRemapCore::HashInt

.. doxygenclass:: AGRemapCore::HashInt
    :members:
    :protected-members:

:raw-html:`<br />`

HashTools
---------

.. cppattributetable:: AGRemapCore::HashTools

.. doxygenclass:: AGRemapCore::HashTools
    :members:
    :protected-members:

:raw-html:`<br />`

IncIdGenerator
==============

.. cppattributetable:: AGRemapCore::IncIdGenerator

.. doxygenclass:: AGRemapCore::IncIdGenerator
    :members:
    :protected-members:

:raw-html:`<br />`

IntTools
========

.. cppattributetable:: AGRemapCore::IntTools

.. doxygenclass:: AGRemapCore::IntTools
    :members:
    :protected-members:

:raw-html:`<br />`

ListTools
=========

.. cppattributetable:: AGRemapCore::ListTools

.. doxygenclass:: AGRemapCore::ListTools
    :members:
    :protected-members:

:raw-html:`<br />`


Ordered MultiMaps
==================

:raw-html:`<br />`

appendAll
---------

.. doxygenfunction:: AGRemapCore::appendAll

:raw-html:`<br />`

BaseOrderedMultiMap
--------------------

.. cppattributetable:: AGRemapCore::BaseOrderedMultiMap

.. doxygenclass:: AGRemapCore::BaseOrderedMultiMap
    :members:
    :protected-members:

:raw-html:`<br />`

IOrderedMultiMap
-----------------

.. cppattributetable:: AGRemapCore::IOrderedMultiMap

.. doxygenclass:: AGRemapCore::IOrderedMultiMap
    :members:
    :protected-members:

:raw-html:`<br />`

KeyRemapData
------------

.. cppattributetable:: AGRemapCore::KeyRemapData

.. doxygenclass:: AGRemapCore::KeyRemapData
    :members:
    :protected-members:

:raw-html:`<br />`

OrderedMultiMap
---------------

.. cppattributetable:: AGRemapCore::OrderedMultiMap

.. doxygenclass:: AGRemapCore::OrderedMultiMap
    :members:
    :protected-members:

:raw-html:`<br />`

OrderedMultiMapAdapter
------------------------

.. cppattributetable:: AGRemapCore::OrderedMultiMapAdapter

.. doxygenclass:: AGRemapCore::OrderedMultiMapAdapter
    :members:
    :protected-members:

:raw-html:`<br />`

OrderedMultiMapSqrt
---------------------

.. cppattributetable:: AGRemapCore::OrderedMultiMapSqrt

.. doxygenclass:: AGRemapCore::OrderedMultiMapSqrt
    :members:
    :protected-members:

:raw-html:`<br />`

RemappedKeyData
----------------

.. cppattributetable:: AGRemapCore::RemappedKeyData

.. doxygenclass:: AGRemapCore::RemappedKeyData
    :members:
    :protected-members:

:raw-html:`<br />`


Parsing
=======

:raw-html:`<br />`

BaseSLR1Parser
--------------

.. cppattributetable:: AGRemapCore::BaseSLR1Parser

.. doxygenclass:: AGRemapCore::BaseSLR1Parser
    :members:
    :protected-members:

:raw-html:`<br />`

BaseTokenizer
-------------

.. cppattributetable:: AGRemapCore::BaseTokenizer

.. doxygenclass:: AGRemapCore::BaseTokenizer
    :members:
    :protected-members:

:raw-html:`<br />`

FilteredTokenizer
------------------

.. cppattributetable:: AGRemapCore::FilteredTokenizer

.. doxygenclass:: AGRemapCore::FilteredTokenizer
    :members:
    :protected-members:

:raw-html:`<br />`

Node
----

.. cppattributetable:: AGRemapCore::Node

.. doxygenclass:: AGRemapCore::Node
    :members:
    :protected-members:

:raw-html:`<br />`

ParseContext
------------

.. cppattributetable:: AGRemapCore::ParseContext

.. doxygenclass:: AGRemapCore::ParseContext
    :members:
    :protected-members:

:raw-html:`<br />`

ParseNode
---------

.. cppattributetable:: AGRemapCore::ParseNode

.. doxygenclass:: AGRemapCore::ParseNode
    :members:
    :protected-members:

:raw-html:`<br />`

ParseTree
---------

.. cppattributetable:: AGRemapCore::ParseTree

.. doxygenclass:: AGRemapCore::ParseTree
    :members:
    :protected-members:

:raw-html:`<br />`

SyntaxErr
---------

.. cppattributetable:: AGRemapCore::SyntaxErr

.. doxygenclass:: AGRemapCore::SyntaxErr
    :members:
    :protected-members:

:raw-html:`<br />`

Token
-----

.. cppattributetable:: AGRemapCore::Token

.. doxygenclass:: AGRemapCore::Token
    :members:
    :protected-members:

:raw-html:`<br />`


Ranges
======

.. cppattributetable:: AGRemapCore::Ranges

.. doxygenclass:: AGRemapCore::Ranges
    :members:
    :protected-members:

:raw-html:`<br />`

StringTools
===========

.. cppattributetable:: AGRemapCore::StringTools

.. doxygenclass:: AGRemapCore::StringTools
    :members:
    :protected-members:

:raw-html:`<br />`

UuidIdGenerator
===============

.. cppattributetable:: AGRemapCore::UuidIdGenerator

.. doxygenclass:: AGRemapCore::UuidIdGenerator
    :members:
    :protected-members:

:raw-html:`<br />`


Z3
==

:raw-html:`<br />`

Z3Context
---------

.. cppattributetable:: AGRemapCore::Z3Context

.. doxygenclass:: AGRemapCore::Z3Context
    :members:
    :protected-members:

:raw-html:`<br />`

Z3Predicate
-----------

.. cppattributetable:: AGRemapCore::Z3Predicate

.. doxygenclass:: AGRemapCore::Z3Predicate
    :members:
    :protected-members:

:raw-html:`<br />`


.. _DFA: https://en.wikipedia.org/wiki/Deterministic_finite_automaton
.. _DFA (Deterministic Finite Automaton): https://en.wikipedia.org/wiki/Deterministic_finite_automaton
.. _section: https://en.wikipedia.org/wiki/INI_file#Sections
.. _sections: https://en.wikipedia.org/wiki/INI_file#Sections
.. _DFS: https://en.wikipedia.org/wiki/Depth-first_search
.. _call graph: https://en.wikipedia.org/wiki/Call_graph
.. _call-with-return: https://en.wikipedia.org/wiki/Subroutine
.. _dataflow analysis: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _Aho-Corasick: https://www.geeksforgeeks.org/aho-corasick-algorithm-pattern-searching/
.. _one-to-one: https://en.wikipedia.org/wiki/Bijection
.. _graphemes: https://en.wikipedia.org/wiki/Grapheme
.. _grapheme: https://en.wikipedia.org/wiki/Grapheme
.. _trie: https://en.wikipedia.org/wiki/Trie
.. _compare function: https://www.geeksforgeeks.org/how-compare-method-works-in-java/
.. _injective: https://en.wikipedia.org/wiki/Injective_function
.. _surjective: https://en.wikipedia.org/wiki/Surjective_function
.. _KVP: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _KVPs: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _adjacency list: https://www.geeksforgeeks.org/adjacency-list-meaning-definition-in-dsa
.. _Maximal Munch: https://en.wikipedia.org/wiki/Maximal_munch
.. _Simplified Maximal Munch: https://en.wikipedia.org/wiki/Maximal_munch
.. _k-way merge problem: https://en.wikipedia.org/wiki/K-way_merge_algorithm
.. _heap: https://en.wikipedia.org/wiki/Heap_(data_structure)
.. _standard heap solution: https://medium.com/@vidyasagarr7/mastering-the-k-way-merge-algorithmic-pattern-for-technical-interviews-6db0e00a049f
.. _binary search: https://en.wikipedia.org/wiki/Binary_search
.. _standard base 64: https://en.wikipedia.org/wiki/Base64
.. _Python: https://www.python.org/
.. _Python's str.strip: https://docs.python.org/3/library/stdtypes.html#str.strip
.. _CRTP: https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
.. _pybind11: https://pybind11.readthedocs.io/en/stable/
.. _tsl::ordered_map: https://github.com/Tessil/ordered-map
.. _tsl::ordered_set: https://github.com/Tessil/ordered-map
.. _XXH3-64: https://github.com/Cyan4973/xxHash
.. _XXH3-128: https://github.com/Cyan4973/xxHash
.. _FNV-1a: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
.. _ASCII: https://en.wikipedia.org/wiki/ASCII
.. _sympy logic query: https://docs.sympy.org/latest/modules/logic.html
.. _Python's str.splitlines: https://docs.python.org/3/library/stdtypes.html#str.splitlines
.. _SLR(1): https://en.wikipedia.org/wiki/Simple_LR_parser
.. _CFG: https://en.wikipedia.org/wiki/Context-free_grammar
.. _CFG (Context Free Grammer): https://en.wikipedia.org/wiki/Context-free_grammar
.. _Nullable Set: https://cs.stackexchange.com/questions/125274/defining-nullable-symbols-and-the-first-set-of-a-grammar
.. _First Set: https://www.geeksforgeeks.org/compiler-design/first-set-in-syntax-analysis/
.. _Follow Set: https://www.geeksforgeeks.org/compiler-design/follow-set-in-syntax-analysis/
.. _LR(0) closure: https://en.wikipedia.org/wiki/LR_parser
.. _UUID: https://en.wikipedia.org/wiki/Universally_unique_identifier
.. _graph: https://en.wikipedia.org/wiki/Graph_theory
.. _Z3: https://github.com/Z3Prover/z3
.. _sympy: https://www.sympy.org/en/index.html
.. _floating point: https://en.wikipedia.org/wiki/C_data_types
.. _half precision floating point: https://en.wikipedia.org/wiki/Half-precision_floating-point_format
.. _unsigned normalized integer: https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
.. _true division: https://peps.python.org/pep-0238/
.. _Compressonator: https://github.com/GPUOpen-Tools/compressonator
.. _Pillow: https://pillow.readthedocs.io/en/stable/index.html
.. _DirectXTex: https://github.com/microsoft/DirectXTex
.. _Gamma Correction: https://www.cambridgeincolour.com/tutorials/gamma-correction.htm
.. _Simple Image Temperature/Tint Adjust Algorithm: https://tannerhelland.com/2014/07/01/simple-algorithms-adjusting-image-temperature-tint.html
.. _Highlight Shadow Approximation Reference: https://stackoverflow.com/questions/51591445/what-is-the-algorithm-behind-photoshops-highlight-or-shadow-alteration
.. _endianness: https://en.wikipedia.org/wiki/Endianness