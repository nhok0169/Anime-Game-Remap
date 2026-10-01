.. role:: raw-html(raw)
    :format: html

=====================
Python API Reference
=====================

Every class and function below is importable straight from the top-level package, e.g. ``FixRaidenBoss2.IniFile``
(or ``AnimeGameRemap.IniFile`` -- see :doc:`API Setup <apiSetup>` for how the two package names relate). Most of
the library is implemented in C++ (see the :doc:`C++ Core API Reference <coreAPI>`) and reached from Python
through these classes.

:raw-html:`<br />`

Where to start
**************

You rarely need more than a handful of the classes on this page. Depending on what you want to do:

- **Fix (or undo the fix of) a whole folder of mods**, the way the command-line program does:
  :class:`RemapServiceCLI` (its :meth:`~RemapServiceCLI.fix` walks the folder), or :class:`RemapService` if you want
  the same work without the command-line conveniences (log file, tips, string options).
- **Fix a single .ini file**: :class:`IniFile` -- :meth:`~IniFile.parse` it, then :meth:`~IniFile.fix` it, or
  :meth:`~IniFile.removeFix` to undo a previous fix. A ``.ini`` file can also be given as a string instead of a path.
- **Fix a single Blend.buf file**: :class:`BlendFile`.
- **Choose which characters are fixed, and onto what**: the supported mod types are :class:`ModTypes`
  (one :class:`ModType` per character), named by :class:`ModTypeId`.
- **Change how a character is remapped without rebuilding the library**: build a fixer with
  :func:`makeGIMICharFixer` from a :class:`GIMICharFixerConfig` (or one of the other ``make...Fixer`` /
  ``make...Parser`` functions) and register it with :class:`CppStrategyOverrides`.
- **Edit textures**: :class:`TextureFile` together with :class:`TexEditor` / :class:`TexCreator` and the texture
  filters (:class:`GammaFilter`, :class:`HueAdjust`, ...).

:doc:`API Examples <apiExamples>` shows each of these end to end.

.. note::
    Some classes come in two forms, ``CppXxx`` and ``Xxx`` (e.g. :class:`CppTexEditor` and :class:`TexEditor`).
    ``CppXxx`` is the class implemented in C++; ``Xxx`` is the Python class built on top of it, which adds the
    Python-friendly parts of the interface. Use ``Xxx`` unless you have a reason not to.

How a fix works
***************

A ``.ini`` file is read into an :class:`IfTemplate` per section, and the sections are linked into
:class:`IniSectionGraph`\ s by the ``run =`` calls between them. For each mod type the file belongs to, a
**parser** (:class:`BaseIniParser`) reads what the mod contains, a **fixer** (:class:`BaseIniFixer`) writes the
remapped sections by applying **edits** to those graphs -- register edits (:class:`BaseRegEdit`), graph edits
(:class:`BaseIniGraphEdit`), group edits (:class:`BaseIniGraphGroupEdit`) and resource edits
(:class:`BaseResEdit`) -- and a **remover** (:class:`BaseIniRemover`) undoes a previous fix. Which parser, fixer
and remover a mod type uses is decided by its :class:`IniParseBuilder`, :class:`IniFixBuilder` and
:class:`IniRemoveBuilder`. The files a fix needs -- ``.buf`` buffers and ``.dds`` textures -- are handled by the
classes under `Model`_ such as :class:`BlendFile` and :class:`TextureFile`.

:raw-html:`<br />`
:raw-html:`<br />`

Model
*****

Classes that represent the actual mod-fixing domain -- the structure of a ``.ini`` file, mod
content, and the remap graph. Contrast with `Tools`_ below, which has no notion of what a "mod"
or a ``.ini`` file even is.

:raw-html:`<br />`

BaseBufEditor
=============

.. attributetable:: FixRaidenBoss2.BaseBufEditor

.. autoclass:: FixRaidenBoss2.BaseBufEditor
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniClassifier
=================

.. attributetable:: FixRaidenBoss2.BaseIniClassifier

.. autoclass:: FixRaidenBoss2.BaseIniClassifier
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniFixer
============

.. attributetable:: FixRaidenBoss2.BaseIniFixer

.. autoclass:: FixRaidenBoss2.BaseIniFixer
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniGraphEdit
================

.. attributetable:: FixRaidenBoss2.BaseIniGraphEdit

.. autoclass:: FixRaidenBoss2.BaseIniGraphEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniGraphGroupEdit
=====================

.. attributetable:: FixRaidenBoss2.BaseIniGraphGroupEdit

.. autoclass:: FixRaidenBoss2.BaseIniGraphGroupEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniGraphPartEdit
====================

.. attributetable:: FixRaidenBoss2.BaseIniGraphPartEdit

.. autoclass:: FixRaidenBoss2.BaseIniGraphPartEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniParser
=============

.. attributetable:: FixRaidenBoss2.BaseIniParser

.. autoclass:: FixRaidenBoss2.BaseIniParser
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniPartEdit
===============

.. attributetable:: FixRaidenBoss2.BaseIniPartEdit

.. autoclass:: FixRaidenBoss2.BaseIniPartEdit
    :members:
    :private-members:

:raw-html:`<br />`

BaseIniRemover
==============

.. attributetable:: FixRaidenBoss2.BaseIniRemover

.. autoclass:: FixRaidenBoss2.BaseIniRemover
    :members:
    :private-members:

:raw-html:`<br />`

BaseModTypeBuilder
==================

.. attributetable:: FixRaidenBoss2.BaseModTypeBuilder

.. autoclass:: FixRaidenBoss2.BaseModTypeBuilder
    :members:
    :private-members:

:raw-html:`<br />`

BasePixelTransform
==================

.. attributetable:: FixRaidenBoss2.BasePixelTransform

.. autoclass:: FixRaidenBoss2.BasePixelTransform
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseRegEdit
===========

.. attributetable:: FixRaidenBoss2.BaseRegEdit

.. autoclass:: FixRaidenBoss2.BaseRegEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseResEdit
===========

.. attributetable:: FixRaidenBoss2.BaseResEdit

.. autoclass:: FixRaidenBoss2.BaseResEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseTexEditor
=============

.. attributetable:: FixRaidenBoss2.BaseTexEditor

.. autoclass:: FixRaidenBoss2.BaseTexEditor
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BaseTexFilter
=============

.. attributetable:: FixRaidenBoss2.BaseTexFilter

.. autoclass:: FixRaidenBoss2.BaseTexFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BinaryFile
==========

.. attributetable:: FixRaidenBoss2.BinaryFile

.. autoclass:: FixRaidenBoss2.BinaryFile
    :members:
    :private-members:

:raw-html:`<br />`

BlendDownloadData
=================

.. attributetable:: FixRaidenBoss2.BlendDownloadData

.. autoclass:: FixRaidenBoss2.BlendDownloadData
    :members:
    :private-members:

:raw-html:`<br />`

BlendFile
=========

.. attributetable:: FixRaidenBoss2.BlendFile

.. autoclass:: FixRaidenBoss2.BlendFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufBaseFloat
============

.. attributetable:: FixRaidenBoss2.BufBaseFloat

.. autoclass:: FixRaidenBoss2.BufBaseFloat
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufBaseInt
==========

.. attributetable:: FixRaidenBoss2.BufBaseInt

.. autoclass:: FixRaidenBoss2.BufBaseInt
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufDataType
===========

.. attributetable:: FixRaidenBoss2.BufDataType

.. autoclass:: FixRaidenBoss2.BufDataType
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufEditor
=========

.. attributetable:: FixRaidenBoss2.BufEditor

.. autoclass:: FixRaidenBoss2.BufEditor
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufElementType
==============

.. attributetable:: FixRaidenBoss2.BufElementType

.. autoclass:: FixRaidenBoss2.BufElementType
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufFile
=======

.. attributetable:: FixRaidenBoss2.BufFile

.. autoclass:: FixRaidenBoss2.BufFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufFloat
========

.. attributetable:: FixRaidenBoss2.BufFloat

.. autoclass:: FixRaidenBoss2.BufFloat
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufFloat16
==========

.. attributetable:: FixRaidenBoss2.BufFloat16

.. autoclass:: FixRaidenBoss2.BufFloat16
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufReplace
==========

.. attributetable:: FixRaidenBoss2.BufReplace

.. autoclass:: FixRaidenBoss2.BufReplace
    :members:
    :private-members:

:raw-html:`<br />`

BufSignedInt
============

.. attributetable:: FixRaidenBoss2.BufSignedInt

.. autoclass:: FixRaidenBoss2.BufSignedInt
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufTools
========

.. attributetable:: FixRaidenBoss2.BufTools

.. autoclass:: FixRaidenBoss2.BufTools
    :members:
    :private-members:

:raw-html:`<br />`

BufType
=======

.. attributetable:: FixRaidenBoss2.BufType

.. autoclass:: FixRaidenBoss2.BufType
    :members:
    :private-members:

:raw-html:`<br />`

BufUnorm
========

.. attributetable:: FixRaidenBoss2.BufUnorm

.. autoclass:: FixRaidenBoss2.BufUnorm
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

BufUnSignedInt
==============

.. attributetable:: FixRaidenBoss2.BufUnSignedInt

.. autoclass:: FixRaidenBoss2.BufUnSignedInt
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CachedFileStats
===============

.. attributetable:: FixRaidenBoss2.CachedFileStats

.. autoclass:: FixRaidenBoss2.CachedFileStats
    :members:
    :private-members:

:raw-html:`<br />`

CallGraph
=========

.. attributetable:: FixRaidenBoss2.CallGraph

.. autoclass:: FixRaidenBoss2.CallGraph
    :members:
    :private-members:

:raw-html:`<br />`

Colour
======

.. attributetable:: FixRaidenBoss2.Colour

.. autoclass:: FixRaidenBoss2.Colour
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ColourRange
===========

.. attributetable:: FixRaidenBoss2.ColourRange

.. autoclass:: FixRaidenBoss2.ColourRange
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ColourReplace
=============

.. attributetable:: FixRaidenBoss2.ColourReplace

.. autoclass:: FixRaidenBoss2.ColourReplace
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ColourReplaceFilter
===================

.. attributetable:: FixRaidenBoss2.ColourReplaceFilter

.. autoclass:: FixRaidenBoss2.ColourReplaceFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CorrectGamma
============

.. attributetable:: FixRaidenBoss2.CorrectGamma

.. autoclass:: FixRaidenBoss2.CorrectGamma
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppBaseIniFixer
===============

.. attributetable:: FixRaidenBoss2.CppBaseIniFixer

.. autoclass:: FixRaidenBoss2.CppBaseIniFixer
    :members:
    :private-members:

:raw-html:`<br />`

CppBaseIniParser
================

.. attributetable:: FixRaidenBoss2.CppBaseIniParser

.. autoclass:: FixRaidenBoss2.CppBaseIniParser
    :members:
    :private-members:

:raw-html:`<br />`

CppBaseIniRemover
=================

.. attributetable:: FixRaidenBoss2.CppBaseIniRemover

.. autoclass:: FixRaidenBoss2.CppBaseIniRemover
    :members:
    :private-members:

:raw-html:`<br />`

CppBasePixelTransform
=====================

.. attributetable:: FixRaidenBoss2.CppBasePixelTransform

.. autoclass:: FixRaidenBoss2.CppBasePixelTransform
    :members:
    :private-members:

:raw-html:`<br />`

CppBaseTexEditor
================

.. attributetable:: FixRaidenBoss2.CppBaseTexEditor

.. autoclass:: FixRaidenBoss2.CppBaseTexEditor
    :members:
    :private-members:

:raw-html:`<br />`

CppBaseTexFilter
================

.. attributetable:: FixRaidenBoss2.CppBaseTexFilter

.. autoclass:: FixRaidenBoss2.CppBaseTexFilter
    :members:
    :private-members:

:raw-html:`<br />`

CppBufFile
==========

.. attributetable:: FixRaidenBoss2.CppBufFile

.. autoclass:: FixRaidenBoss2.CppBufFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppColour
=========

.. attributetable:: FixRaidenBoss2.CppColour

.. autoclass:: FixRaidenBoss2.CppColour
    :members:
    :private-members:

:raw-html:`<br />`

CppColourRange
==============

.. attributetable:: FixRaidenBoss2.CppColourRange

.. autoclass:: FixRaidenBoss2.CppColourRange
    :members:
    :private-members:

:raw-html:`<br />`

CppColourReplace
================

.. attributetable:: FixRaidenBoss2.CppColourReplace

.. autoclass:: FixRaidenBoss2.CppColourReplace
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppColourReplaceFilter
======================

.. attributetable:: FixRaidenBoss2.CppColourReplaceFilter

.. autoclass:: FixRaidenBoss2.CppColourReplaceFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppCorrectGamma
===============

.. attributetable:: FixRaidenBoss2.CppCorrectGamma

.. autoclass:: FixRaidenBoss2.CppCorrectGamma
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppGammaFilter
==============

.. attributetable:: FixRaidenBoss2.CppGammaFilter

.. autoclass:: FixRaidenBoss2.CppGammaFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppGlobalModTypes
=================

.. attributetable:: FixRaidenBoss2.CppGlobalModTypes

.. autoclass:: FixRaidenBoss2.CppGlobalModTypes
    :members:
    :private-members:

:raw-html:`<br />`

CppHighlightShadow
==================

.. attributetable:: FixRaidenBoss2.CppHighlightShadow

.. autoclass:: FixRaidenBoss2.CppHighlightShadow
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppHueAdjust
============

.. attributetable:: FixRaidenBoss2.CppHueAdjust

.. autoclass:: FixRaidenBoss2.CppHueAdjust
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppIniFixBuilderArgs
====================

.. attributetable:: FixRaidenBoss2.CppIniFixBuilderArgs

.. autoclass:: FixRaidenBoss2.CppIniFixBuilderArgs
    :members:
    :private-members:

:raw-html:`<br />`

CppIniParseBuilderArgs
======================

.. attributetable:: FixRaidenBoss2.CppIniParseBuilderArgs

.. autoclass:: FixRaidenBoss2.CppIniParseBuilderArgs
    :members:
    :private-members:

:raw-html:`<br />`

CppIniRemoveBuilderArgs
=======================

.. attributetable:: FixRaidenBoss2.CppIniRemoveBuilderArgs

.. autoclass:: FixRaidenBoss2.CppIniRemoveBuilderArgs
    :members:
    :private-members:

:raw-html:`<br />`

CppInvertAlpha
==============

.. attributetable:: FixRaidenBoss2.CppInvertAlpha

.. autoclass:: FixRaidenBoss2.CppInvertAlpha
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppInvertAlphaFilter
====================

.. attributetable:: FixRaidenBoss2.CppInvertAlphaFilter

.. autoclass:: FixRaidenBoss2.CppInvertAlphaFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppMaterialBandRemapFilter
==========================

.. attributetable:: FixRaidenBoss2.CppMaterialBandRemapFilter

.. autoclass:: FixRaidenBoss2.CppMaterialBandRemapFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppPixelFilter
==============

.. attributetable:: FixRaidenBoss2.CppPixelFilter

.. autoclass:: FixRaidenBoss2.CppPixelFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppRemapServiceCLI
==================

.. attributetable:: FixRaidenBoss2.CppRemapServiceCLI

.. autoclass:: FixRaidenBoss2.CppRemapServiceCLI
    :members:
    :private-members:

:raw-html:`<br />`

CppStrategyOverrides
====================

.. attributetable:: FixRaidenBoss2.CppStrategyOverrides

.. autoclass:: FixRaidenBoss2.CppStrategyOverrides
    :members:
    :private-members:

:raw-html:`<br />`

CppTempControl
==============

.. attributetable:: FixRaidenBoss2.CppTempControl

.. autoclass:: FixRaidenBoss2.CppTempControl
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppTexCreator
=============

.. attributetable:: FixRaidenBoss2.CppTexCreator

.. autoclass:: FixRaidenBoss2.CppTexCreator
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppTexEditor
============

.. attributetable:: FixRaidenBoss2.CppTexEditor

.. autoclass:: FixRaidenBoss2.CppTexEditor
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppTextureFile
==============

.. attributetable:: FixRaidenBoss2.CppTextureFile

.. autoclass:: FixRaidenBoss2.CppTextureFile
    :members:
    :private-members:

:raw-html:`<br />`

CppTintTransform
================

.. attributetable:: FixRaidenBoss2.CppTintTransform

.. autoclass:: FixRaidenBoss2.CppTintTransform
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppTransparency
===============

.. attributetable:: FixRaidenBoss2.CppTransparency

.. autoclass:: FixRaidenBoss2.CppTransparency
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

CppTransparencyAdjustFilter
===========================

.. attributetable:: FixRaidenBoss2.CppTransparencyAdjustFilter

.. autoclass:: FixRaidenBoss2.CppTransparencyAdjustFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

DownloadData
============

.. attributetable:: FixRaidenBoss2.DownloadData

.. autoclass:: FixRaidenBoss2.DownloadData
    :members:
    :private-members:

:raw-html:`<br />`

File
====

.. attributetable:: FixRaidenBoss2.File

.. autoclass:: FixRaidenBoss2.File
    :members:
    :private-members:

:raw-html:`<br />`

FileDownload
============

.. attributetable:: FixRaidenBoss2.FileDownload

.. autoclass:: FixRaidenBoss2.FileDownload
    :members:
    :private-members:

:raw-html:`<br />`

FileStats
=========

.. attributetable:: FixRaidenBoss2.FileStats

.. autoclass:: FixRaidenBoss2.FileStats
    :members:
    :private-members:

:raw-html:`<br />`

FromOldVal
==========

.. attributetable:: FixRaidenBoss2.FromOldVal

.. autoclass:: FixRaidenBoss2.FromOldVal
    :members:
    :private-members:

:raw-html:`<br />`

GameTypeId
==========

.. autoclass:: FixRaidenBoss2.GameTypeId
    :members:

:raw-html:`<br />`

GameTypeIdTools
===============

.. attributetable:: FixRaidenBoss2.GameTypeIdTools

.. autoclass:: FixRaidenBoss2.GameTypeIdTools
    :members:
    :private-members:

:raw-html:`<br />`

GammaFilter
===========

.. attributetable:: FixRaidenBoss2.GammaFilter

.. autoclass:: FixRaidenBoss2.GammaFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GIBuilder
=========

.. attributetable:: FixRaidenBoss2.GIBuilder

.. autoclass:: FixRaidenBoss2.GIBuilder
    :members:
    :private-members:

:raw-html:`<br />`

GIMICharFixerConfig
===================

.. attributetable:: FixRaidenBoss2.GIMICharFixerConfig

.. autoclass:: FixRaidenBoss2.GIMICharFixerConfig
    :members:
    :private-members:

:raw-html:`<br />`

GIMICharParserConfig
====================

.. attributetable:: FixRaidenBoss2.GIMICharParserConfig

.. autoclass:: FixRaidenBoss2.GIMICharParserConfig
    :members:
    :private-members:

:raw-html:`<br />`

GIMIComponentFixerConfig
========================

.. attributetable:: FixRaidenBoss2.GIMIComponentFixerConfig

.. autoclass:: FixRaidenBoss2.GIMIComponentFixerConfig
    :members:
    :private-members:

:raw-html:`<br />`

GIMIComponentParserConfig
=========================

.. attributetable:: FixRaidenBoss2.GIMIComponentParserConfig

.. autoclass:: FixRaidenBoss2.GIMIComponentParserConfig
    :members:
    :private-members:

:raw-html:`<br />`

GIMIFixer
=========

.. attributetable:: FixRaidenBoss2.GIMIFixer

.. autoclass:: FixRaidenBoss2.GIMIFixer
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GIMIMergeFixerConfig
====================

.. attributetable:: FixRaidenBoss2.GIMIMergeFixerConfig

.. autoclass:: FixRaidenBoss2.GIMIMergeFixerConfig
    :members:
    :private-members:

:raw-html:`<br />`

GIMIObjPartFilter
=================

.. attributetable:: FixRaidenBoss2.GIMIObjPartFilter

.. autoclass:: FixRaidenBoss2.GIMIObjPartFilter
    :members:
    :private-members:

:raw-html:`<br />`

GIMIParser
==========

.. attributetable:: FixRaidenBoss2.GIMIParser

.. autoclass:: FixRaidenBoss2.GIMIParser
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GIMISectionClassifier
=====================

.. attributetable:: FixRaidenBoss2.GIMISectionClassifier

.. autoclass:: FixRaidenBoss2.GIMISectionClassifier
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GlobalRemapIniRemover
=====================

.. attributetable:: FixRaidenBoss2.GlobalRemapIniRemover

.. autoclass:: FixRaidenBoss2.GlobalRemapIniRemover
    :members:
    :private-members:

:raw-html:`<br />`

GraphCreate
===========

.. attributetable:: FixRaidenBoss2.GraphCreate

.. autoclass:: FixRaidenBoss2.GraphCreate
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphGroupEdit
==============

.. attributetable:: FixRaidenBoss2.GraphGroupEdit

.. autoclass:: FixRaidenBoss2.GraphGroupEdit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphGroupRemap
===============

.. attributetable:: FixRaidenBoss2.GraphGroupRemap

.. autoclass:: FixRaidenBoss2.GraphGroupRemap
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphGroupRemove
================

.. attributetable:: FixRaidenBoss2.GraphGroupRemove

.. autoclass:: FixRaidenBoss2.GraphGroupRemove
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphInherit
============

.. attributetable:: FixRaidenBoss2.GraphInherit

.. autoclass:: FixRaidenBoss2.GraphInherit
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphRemove
===========

.. attributetable:: FixRaidenBoss2.GraphRemove

.. autoclass:: FixRaidenBoss2.GraphRemove
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

GraphRename
===========

.. attributetable:: FixRaidenBoss2.GraphRename

.. autoclass:: FixRaidenBoss2.GraphRename
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

Hashes
======

.. attributetable:: FixRaidenBoss2.Hashes

.. autoclass:: FixRaidenBoss2.Hashes
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

HighlightShadow
===============

.. attributetable:: FixRaidenBoss2.HighlightShadow

.. autoclass:: FixRaidenBoss2.HighlightShadow
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

HueAdjust
=========

.. attributetable:: FixRaidenBoss2.HueAdjust

.. autoclass:: FixRaidenBoss2.HueAdjust
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IbFile
======

.. attributetable:: FixRaidenBoss2.IbFile

.. autoclass:: FixRaidenBoss2.IbFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IfContentPart
=============

.. attributetable:: FixRaidenBoss2.IfContentPart

.. autoclass:: FixRaidenBoss2.IfContentPart
    :members:
    :private-members:

:raw-html:`<br />`

IfContentPartColourChange
=========================

.. attributetable:: FixRaidenBoss2.IfContentPartColourChange

.. autoclass:: FixRaidenBoss2.IfContentPartColourChange
    :members:
    :private-members:

:raw-html:`<br />`

IfContentPartColouring
======================

.. attributetable:: FixRaidenBoss2.IfContentPartColouring

.. autoclass:: FixRaidenBoss2.IfContentPartColouring
    :members:
    :private-members:

:raw-html:`<br />`

IfPredLogicGenerator
====================

.. attributetable:: FixRaidenBoss2.IfPredLogicGenerator

.. autoclass:: FixRaidenBoss2.IfPredLogicGenerator
    :members:
    :private-members:

:raw-html:`<br />`

IfPredParser
============

.. attributetable:: FixRaidenBoss2.IfPredParser

.. autoclass:: FixRaidenBoss2.IfPredParser
    :members:
    :private-members:

:raw-html:`<br />`

IfPredPart
==========

.. attributetable:: FixRaidenBoss2.IfPredPart

.. autoclass:: FixRaidenBoss2.IfPredPart
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IfPredTokenizer
===============

.. attributetable:: FixRaidenBoss2.IfPredTokenizer

.. autoclass:: FixRaidenBoss2.IfPredTokenizer
    :members:
    :private-members:

:raw-html:`<br />`

IfTemplate
==========

.. attributetable:: FixRaidenBoss2.IfTemplate

.. autoclass:: FixRaidenBoss2.IfTemplate
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IfTemplateNode
==============

.. attributetable:: FixRaidenBoss2.IfTemplateNode

.. autoclass:: FixRaidenBoss2.IfTemplateNode
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IfTemplatePart
==============

.. attributetable:: FixRaidenBoss2.IfTemplatePart

.. autoclass:: FixRaidenBoss2.IfTemplatePart
    :members:
    :private-members:

:raw-html:`<br />`

IfTemplateTree
==============

.. attributetable:: FixRaidenBoss2.IfTemplateTree

.. autoclass:: FixRaidenBoss2.IfTemplateTree
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IndexCounts
===========

.. attributetable:: FixRaidenBoss2.IndexCounts

.. autoclass:: FixRaidenBoss2.IndexCounts
    :members:
    :private-members:

:raw-html:`<br />`

Indices
=======

.. attributetable:: FixRaidenBoss2.Indices

.. autoclass:: FixRaidenBoss2.Indices
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IniClassifier
=============

.. attributetable:: FixRaidenBoss2.IniClassifier

.. autoclass:: FixRaidenBoss2.IniClassifier
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IniClassifyStats
================

.. attributetable:: FixRaidenBoss2.IniClassifyStats

.. autoclass:: FixRaidenBoss2.IniClassifyStats
    :members:
    :private-members:

:raw-html:`<br />`

IniDownloadModel
================

.. attributetable:: FixRaidenBoss2.IniDownloadModel

.. autoclass:: FixRaidenBoss2.IniDownloadModel
    :members:
    :private-members:

:raw-html:`<br />`

IniFile
=======

.. attributetable:: FixRaidenBoss2.IniFile

.. autoclass:: FixRaidenBoss2.IniFile
    :members:
    :private-members:

:raw-html:`<br />`

IniFixBuilder
=============

.. attributetable:: FixRaidenBoss2.IniFixBuilder

.. autoclass:: FixRaidenBoss2.IniFixBuilder
    :members:
    :private-members:

:raw-html:`<br />`

IniFixingContext
================

.. attributetable:: FixRaidenBoss2.IniFixingContext

.. autoclass:: FixRaidenBoss2.IniFixingContext
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IniFixResource
==============

.. attributetable:: FixRaidenBoss2.IniFixResource

.. autoclass:: FixRaidenBoss2.IniFixResource
    :members:
    :private-members:

:raw-html:`<br />`

IniFixResourceModel
===================

.. attributetable:: FixRaidenBoss2.IniFixResourceModel

.. autoclass:: FixRaidenBoss2.IniFixResourceModel
    :members:
    :private-members:

:raw-html:`<br />`

IniGraphGroup
=============

.. attributetable:: FixRaidenBoss2.IniGraphGroup

.. autoclass:: FixRaidenBoss2.IniGraphGroup
    :members:
    :private-members:

:raw-html:`<br />`

IniGroupedResBuilder
====================

.. attributetable:: FixRaidenBoss2.IniGroupedResBuilder

.. autoclass:: FixRaidenBoss2.IniGroupedResBuilder
    :members:
    :private-members:

:raw-html:`<br />`

IniGroupedResource
==================

.. attributetable:: FixRaidenBoss2.IniGroupedResource

.. autoclass:: FixRaidenBoss2.IniGroupedResource
    :members:
    :private-members:

:raw-html:`<br />`

IniNamingTools
==============

.. attributetable:: FixRaidenBoss2.IniNamingTools

.. autoclass:: FixRaidenBoss2.IniNamingTools
    :members:
    :private-members:

:raw-html:`<br />`

IniParseBuilder
===============

.. attributetable:: FixRaidenBoss2.IniParseBuilder

.. autoclass:: FixRaidenBoss2.IniParseBuilder
    :members:
    :private-members:

:raw-html:`<br />`

IniRemovalContext
=================

.. attributetable:: FixRaidenBoss2.IniRemovalContext

.. autoclass:: FixRaidenBoss2.IniRemovalContext
    :members:
    :private-members:

:raw-html:`<br />`

IniRemoveBuilder
================

.. attributetable:: FixRaidenBoss2.IniRemoveBuilder

.. autoclass:: FixRaidenBoss2.IniRemoveBuilder
    :members:
    :private-members:

:raw-html:`<br />`

IniResource
===========

.. attributetable:: FixRaidenBoss2.IniResource

.. autoclass:: FixRaidenBoss2.IniResource
    :members:
    :private-members:

:raw-html:`<br />`

IniResourceModel
================

.. attributetable:: FixRaidenBoss2.IniResourceModel

.. autoclass:: FixRaidenBoss2.IniResourceModel
    :members:
    :private-members:

:raw-html:`<br />`

IniSectionGraph
===============

.. attributetable:: FixRaidenBoss2.IniSectionGraph

.. autoclass:: FixRaidenBoss2.IniSectionGraph
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

IniSrcResourceModel
===================

.. attributetable:: FixRaidenBoss2.IniSrcResourceModel

.. autoclass:: FixRaidenBoss2.IniSrcResourceModel
    :members:
    :private-members:

:raw-html:`<br />`

IniTexModel
===========

.. attributetable:: FixRaidenBoss2.IniTexModel

.. autoclass:: FixRaidenBoss2.IniTexModel
    :members:
    :private-members:

:raw-html:`<br />`

InnerLayerOutline
=================

.. attributetable:: FixRaidenBoss2.InnerLayerOutline

.. autoclass:: FixRaidenBoss2.InnerLayerOutline
    :members:
    :private-members:

:raw-html:`<br />`

InvertAlpha
===========

.. attributetable:: FixRaidenBoss2.InvertAlpha

.. autoclass:: FixRaidenBoss2.InvertAlpha
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

InvertAlphaFilter
=================

.. attributetable:: FixRaidenBoss2.InvertAlphaFilter

.. autoclass:: FixRaidenBoss2.InvertAlphaFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

makeGIMICharFixer
=================

.. autofunction:: FixRaidenBoss2.makeGIMICharFixer

:raw-html:`<br />`

makeGIMICharParser
==================

.. autofunction:: FixRaidenBoss2.makeGIMICharParser

:raw-html:`<br />`

makeGIMIComponentFixer
======================

.. autofunction:: FixRaidenBoss2.makeGIMIComponentFixer

:raw-html:`<br />`

makeGIMIComponentParser
=======================

.. autofunction:: FixRaidenBoss2.makeGIMIComponentParser

:raw-html:`<br />`

makeGIMIMergeFixer
==================

.. autofunction:: FixRaidenBoss2.makeGIMIMergeFixer

:raw-html:`<br />`

makeWWMIFixer
=============

.. autofunction:: FixRaidenBoss2.makeWWMIFixer

:raw-html:`<br />`

makeWWMIParser
==============

.. autofunction:: FixRaidenBoss2.makeWWMIParser

:raw-html:`<br />`

MaterialBandRemapFilter
=======================

.. attributetable:: FixRaidenBoss2.MaterialBandRemapFilter

.. autoclass:: FixRaidenBoss2.MaterialBandRemapFilter
    :inherited-members:
    :members:
    :private-members:
    :exclude-members: Band

:raw-html:`<br />`

ModAssets
=========

.. attributetable:: FixRaidenBoss2.ModAssets

.. autoclass:: FixRaidenBoss2.ModAssets
    :members:
    :private-members:

:raw-html:`<br />`

ModDictAssets
=============

.. attributetable:: FixRaidenBoss2.ModDictAssets

.. autoclass:: FixRaidenBoss2.ModDictAssets
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

Model (class)
=============

.. attributetable:: FixRaidenBoss2.Model

.. autoclass:: FixRaidenBoss2.Model
    :members:
    :private-members:

:raw-html:`<br />`

ModMappedAssets
===============

.. attributetable:: FixRaidenBoss2.ModMappedAssets

.. autoclass:: FixRaidenBoss2.ModMappedAssets
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ModType
=======

.. attributetable:: FixRaidenBoss2.ModType

.. autoclass:: FixRaidenBoss2.ModType
    :members:
    :private-members:

:raw-html:`<br />`

ModTypeBuilder
==============

.. attributetable:: FixRaidenBoss2.ModTypeBuilder

.. autoclass:: FixRaidenBoss2.ModTypeBuilder
    :members:
    :private-members:

:raw-html:`<br />`

ModTypeId
=========

.. autoclass:: FixRaidenBoss2.ModTypeId
    :members:

:raw-html:`<br />`

ModTypeIdData
=============

.. attributetable:: FixRaidenBoss2.ModTypeIdData

.. autoclass:: FixRaidenBoss2.ModTypeIdData
    :members:
    :private-members:

:raw-html:`<br />`

ModTypeIdTools
==============

.. attributetable:: FixRaidenBoss2.ModTypeIdTools

.. autoclass:: FixRaidenBoss2.ModTypeIdTools
    :members:
    :private-members:

:raw-html:`<br />`

MultiModFixer
=============

.. attributetable:: FixRaidenBoss2.MultiModFixer

.. autoclass:: FixRaidenBoss2.MultiModFixer
    :members:
    :private-members:

:raw-html:`<br />`

PixelFilter
===========

.. attributetable:: FixRaidenBoss2.PixelFilter

.. autoclass:: FixRaidenBoss2.PixelFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

PositionFile
============

.. attributetable:: FixRaidenBoss2.PositionFile

.. autoclass:: FixRaidenBoss2.PositionFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegAdd
======

.. attributetable:: FixRaidenBoss2.RegAdd

.. autoclass:: FixRaidenBoss2.RegAdd
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegAssetRemap
=============

.. attributetable:: FixRaidenBoss2.RegAssetRemap

.. autoclass:: FixRaidenBoss2.RegAssetRemap
    :members:
    :private-members:

:raw-html:`<br />`

RegBottomAdd
============

.. attributetable:: FixRaidenBoss2.RegBottomAdd

.. autoclass:: FixRaidenBoss2.RegBottomAdd
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegBranchAdd
============

.. attributetable:: FixRaidenBoss2.RegBranchAdd

.. autoclass:: FixRaidenBoss2.RegBranchAdd
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegDelimitedAdd
===============

.. attributetable:: FixRaidenBoss2.RegDelimitedAdd

.. autoclass:: FixRaidenBoss2.RegDelimitedAdd
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegDelimitedAddMode
===================

.. autoclass:: FixRaidenBoss2.RegDelimitedAddMode
    :members:

:raw-html:`<br />`

RegFillMissing
==============

.. attributetable:: FixRaidenBoss2.RegFillMissing

.. autoclass:: FixRaidenBoss2.RegFillMissing
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegNewVals
==========

.. attributetable:: FixRaidenBoss2.RegNewVals

.. autoclass:: FixRaidenBoss2.RegNewVals
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegRemap
========

.. attributetable:: FixRaidenBoss2.RegRemap

.. autoclass:: FixRaidenBoss2.RegRemap
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegRemove
=========

.. attributetable:: FixRaidenBoss2.RegRemove

.. autoclass:: FixRaidenBoss2.RegRemove
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegRestrict
===========

.. attributetable:: FixRaidenBoss2.RegRestrict

.. autoclass:: FixRaidenBoss2.RegRestrict
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RegSurroundedAdd
================

.. attributetable:: FixRaidenBoss2.RegSurroundedAdd

.. autoclass:: FixRaidenBoss2.RegSurroundedAdd
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RemapBlendReplace
=================

.. attributetable:: FixRaidenBoss2.RemapBlendReplace

.. autoclass:: FixRaidenBoss2.RemapBlendReplace
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RemapBlendResource
==================

.. attributetable:: FixRaidenBoss2.RemapBlendResource

.. autoclass:: FixRaidenBoss2.RemapBlendResource
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniDownload
================

.. attributetable:: FixRaidenBoss2.RemapIniDownload

.. autoclass:: FixRaidenBoss2.RemapIniDownload
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniFixResource
===================

.. attributetable:: FixRaidenBoss2.RemapIniFixResource

.. autoclass:: FixRaidenBoss2.RemapIniFixResource
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniGroupedResource
=======================

.. attributetable:: FixRaidenBoss2.RemapIniGroupedResource

.. autoclass:: FixRaidenBoss2.RemapIniGroupedResource
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniRemover
===============

.. attributetable:: FixRaidenBoss2.RemapIniRemover

.. autoclass:: FixRaidenBoss2.RemapIniRemover
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniResource
================

.. attributetable:: FixRaidenBoss2.RemapIniResource

.. autoclass:: FixRaidenBoss2.RemapIniResource
    :members:
    :private-members:

:raw-html:`<br />`

RemapIniResourceMixin
=====================

.. attributetable:: FixRaidenBoss2.RemapIniResourceMixin

.. autoclass:: FixRaidenBoss2.RemapIniResourceMixin
    :members:
    :private-members:

:raw-html:`<br />`

remapMain
=========

.. autofunction:: FixRaidenBoss2.remapMain

:raw-html:`<br />`

RemapService
============

.. attributetable:: FixRaidenBoss2.RemapService

.. autoclass:: FixRaidenBoss2.RemapService
    :members:
    :private-members:

:raw-html:`<br />`

RemapServiceCLI
===============

.. attributetable:: FixRaidenBoss2.RemapServiceCLI

.. autoclass:: FixRaidenBoss2.RemapServiceCLI
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RemapStats
==========

.. attributetable:: FixRaidenBoss2.RemapStats

.. autoclass:: FixRaidenBoss2.RemapStats
    :members:
    :private-members:

:raw-html:`<br />`

RemapTexAddResource
===================

.. attributetable:: FixRaidenBoss2.RemapTexAddResource

.. autoclass:: FixRaidenBoss2.RemapTexAddResource
    :members:
    :private-members:

:raw-html:`<br />`

RemapTexEditResource
====================

.. attributetable:: FixRaidenBoss2.RemapTexEditResource

.. autoclass:: FixRaidenBoss2.RemapTexEditResource
    :members:
    :private-members:

:raw-html:`<br />`

ResCreate
=========

.. attributetable:: FixRaidenBoss2.ResCreate

.. autoclass:: FixRaidenBoss2.ResCreate
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ResGroupCollect
===============

.. attributetable:: FixRaidenBoss2.ResGroupCollect

.. autoclass:: FixRaidenBoss2.ResGroupCollect
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ResIdentity
===========

.. attributetable:: FixRaidenBoss2.ResIdentity

.. autoclass:: FixRaidenBoss2.ResIdentity
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ResRegCollect
=============

.. attributetable:: FixRaidenBoss2.ResRegCollect

.. autoclass:: FixRaidenBoss2.ResRegCollect
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ResReplace
==========

.. attributetable:: FixRaidenBoss2.ResReplace

.. autoclass:: FixRaidenBoss2.ResReplace
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

SectionIterData
===============

.. attributetable:: FixRaidenBoss2.SectionIterData

.. autoclass:: FixRaidenBoss2.SectionIterData
    :members:
    :private-members:

:raw-html:`<br />`

SectionIterQueryData
====================

.. attributetable:: FixRaidenBoss2.SectionIterQueryData

.. autoclass:: FixRaidenBoss2.SectionIterQueryData
    :members:
    :private-members:

:raw-html:`<br />`

ShapeKeyChecksums
=================

.. attributetable:: FixRaidenBoss2.ShapeKeyChecksums

.. autoclass:: FixRaidenBoss2.ShapeKeyChecksums
    :members:
    :private-members:

:raw-html:`<br />`

SideMeshes
==========

.. attributetable:: FixRaidenBoss2.SideMeshes

.. autoclass:: FixRaidenBoss2.SideMeshes
    :members:
    :private-members:

:raw-html:`<br />`

SympyIfPredGenerator
====================

.. attributetable:: FixRaidenBoss2.SympyIfPredGenerator

.. autoclass:: FixRaidenBoss2.SympyIfPredGenerator
    :members:
    :private-members:

:raw-html:`<br />`

SympyParser
===========

.. attributetable:: FixRaidenBoss2.SympyParser

.. autoclass:: FixRaidenBoss2.SympyParser
    :members:
    :private-members:

:raw-html:`<br />`

SympyTokenizer
==============

.. attributetable:: FixRaidenBoss2.SympyTokenizer

.. autoclass:: FixRaidenBoss2.SympyTokenizer
    :members:
    :private-members:

:raw-html:`<br />`

TempControl
===========

.. attributetable:: FixRaidenBoss2.TempControl

.. autoclass:: FixRaidenBoss2.TempControl
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TexCache
========

.. attributetable:: FixRaidenBoss2.TexCache

.. autoclass:: FixRaidenBoss2.TexCache
    :members:
    :private-members:

:raw-html:`<br />`

TexCreate
=========

.. attributetable:: FixRaidenBoss2.TexCreate

.. autoclass:: FixRaidenBoss2.TexCreate
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TexCreator
==========

.. attributetable:: FixRaidenBoss2.TexCreator

.. autoclass:: FixRaidenBoss2.TexCreator
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TexEditor
=========

.. attributetable:: FixRaidenBoss2.TexEditor

.. autoclass:: FixRaidenBoss2.TexEditor
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TexMetadataFilter
=================

.. attributetable:: FixRaidenBoss2.TexMetadataFilter

.. autoclass:: FixRaidenBoss2.TexMetadataFilter
    :members:
    :private-members:

:raw-html:`<br />`

TexReplace
==========

.. attributetable:: FixRaidenBoss2.TexReplace

.. autoclass:: FixRaidenBoss2.TexReplace
    :members:
    :private-members:

:raw-html:`<br />`

TextureFile
===========

.. attributetable:: FixRaidenBoss2.TextureFile

.. autoclass:: FixRaidenBoss2.TextureFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TintTransform
=============

.. attributetable:: FixRaidenBoss2.TintTransform

.. autoclass:: FixRaidenBoss2.TintTransform
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

Transparency
============

.. attributetable:: FixRaidenBoss2.Transparency

.. autoclass:: FixRaidenBoss2.Transparency
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

TransparencyAdjustFilter
========================

.. attributetable:: FixRaidenBoss2.TransparencyAdjustFilter

.. autoclass:: FixRaidenBoss2.TransparencyAdjustFilter
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

VbFile
======

.. attributetable:: FixRaidenBoss2.VbFile

.. autoclass:: FixRaidenBoss2.VbFile
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

Version
=======

.. attributetable:: FixRaidenBoss2.Version

.. autoclass:: FixRaidenBoss2.Version
    :members:
    :private-members:

:raw-html:`<br />`

VersionSet
==========

.. attributetable:: FixRaidenBoss2.VersionSet

.. autoclass:: FixRaidenBoss2.VersionSet
    :members:
    :private-members:

:raw-html:`<br />`

VertexCounts
============

.. attributetable:: FixRaidenBoss2.VertexCounts

.. autoclass:: FixRaidenBoss2.VertexCounts
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentBuffers
==================

.. attributetable:: FixRaidenBoss2.VGComponentBuffers

.. autoclass:: FixRaidenBoss2.VGComponentBuffers
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentMerge
================

.. attributetable:: FixRaidenBoss2.VGComponentMerge

.. autoclass:: FixRaidenBoss2.VGComponentMerge
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentMergeStats
=====================

.. attributetable:: FixRaidenBoss2.VGComponentMergeStats

.. autoclass:: FixRaidenBoss2.VGComponentMergeStats
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentSpec
===============

.. attributetable:: FixRaidenBoss2.VGComponentSpec

.. autoclass:: FixRaidenBoss2.VGComponentSpec
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentSplit
================

.. attributetable:: FixRaidenBoss2.VGComponentSplit

.. autoclass:: FixRaidenBoss2.VGComponentSplit
    :members:
    :private-members:

:raw-html:`<br />`

VGComponentSplitStats
=====================

.. attributetable:: FixRaidenBoss2.VGComponentSplitStats

.. autoclass:: FixRaidenBoss2.VGComponentSplitStats
    :members:
    :private-members:

:raw-html:`<br />`

VGCounts
========

.. attributetable:: FixRaidenBoss2.VGCounts

.. autoclass:: FixRaidenBoss2.VGCounts
    :members:
    :private-members:

:raw-html:`<br />`

VGMergeComponent
================

.. attributetable:: FixRaidenBoss2.VGMergeComponent

.. autoclass:: FixRaidenBoss2.VGMergeComponent
    :members:
    :private-members:

:raw-html:`<br />`

VGMergeComponentFiles
=====================

.. attributetable:: FixRaidenBoss2.VGMergeComponentFiles

.. autoclass:: FixRaidenBoss2.VGMergeComponentFiles
    :members:
    :private-members:

:raw-html:`<br />`

VGMergeComponentSpec
====================

.. attributetable:: FixRaidenBoss2.VGMergeComponentSpec

.. autoclass:: FixRaidenBoss2.VGMergeComponentSpec
    :members:
    :private-members:

:raw-html:`<br />`

VGMergeGroupResource
====================

.. attributetable:: FixRaidenBoss2.VGMergeGroupResource

.. autoclass:: FixRaidenBoss2.VGMergeGroupResource
    :members:
    :private-members:

:raw-html:`<br />`

VGMergeObject
=============

.. attributetable:: FixRaidenBoss2.VGMergeObject

.. autoclass:: FixRaidenBoss2.VGMergeObject
    :members:
    :private-members:

:raw-html:`<br />`

VGOffsets
=========

.. attributetable:: FixRaidenBoss2.VGOffsets

.. autoclass:: FixRaidenBoss2.VGOffsets
    :members:
    :private-members:

:raw-html:`<br />`

VGPushAway
==========

.. attributetable:: FixRaidenBoss2.VGPushAway

.. autoclass:: FixRaidenBoss2.VGPushAway
    :members:
    :private-members:

:raw-html:`<br />`

VGRemap
=======

.. attributetable:: FixRaidenBoss2.VGRemap

.. autoclass:: FixRaidenBoss2.VGRemap
    :members:
    :private-members:

:raw-html:`<br />`

VGRemaps
========

.. attributetable:: FixRaidenBoss2.VGRemaps

.. autoclass:: FixRaidenBoss2.VGRemaps
    :members:
    :private-members:

:raw-html:`<br />`

VGSplitGroupResource
====================

.. attributetable:: FixRaidenBoss2.VGSplitGroupResource

.. autoclass:: FixRaidenBoss2.VGSplitGroupResource
    :members:
    :private-members:

:raw-html:`<br />`

WWMIBuilder
===========

.. attributetable:: FixRaidenBoss2.WWMIBuilder

.. autoclass:: FixRaidenBoss2.WWMIBuilder
    :members:
    :private-members:

:raw-html:`<br />`

WWMIFixerConfig
===============

.. attributetable:: FixRaidenBoss2.WWMIFixerConfig

.. autoclass:: FixRaidenBoss2.WWMIFixerConfig
    :members:
    :private-members:

:raw-html:`<br />`

WWMIParserConfig
================

.. attributetable:: FixRaidenBoss2.WWMIParserConfig

.. autoclass:: FixRaidenBoss2.WWMIParserConfig
    :members:
    :private-members:

:raw-html:`<br />`

WWMITextureFacts
================

.. attributetable:: FixRaidenBoss2.WWMITextureFacts

.. autoclass:: FixRaidenBoss2.WWMITextureFacts
    :members:
    :private-members:

:raw-html:`<br />`


:raw-html:`<br />`
:raw-html:`<br />`

View
****

How the remap reports its progress, headings, lists and errors back to the user, and asks for input.
:class:`BaseLogger` owns all of the formatting; a concrete view such as :class:`Logger` only decides where
the rendered lines go. Subclass :class:`BaseLogger` to send the output somewhere else (a GUI, a socket to a
frontend app, ...).

:raw-html:`<br />`

BaseLogger
==========

.. attributetable:: FixRaidenBoss2.BaseLogger

.. autoclass:: FixRaidenBoss2.BaseLogger
    :members:
    :private-members:

:raw-html:`<br />`

Logger
======

.. attributetable:: FixRaidenBoss2.Logger

.. autoclass:: FixRaidenBoss2.Logger
    :members:
    :private-members:

:raw-html:`<br />`

:raw-html:`<br />`
:raw-html:`<br />`

Constants
*********

Enums and constant tables used throughout the API: the supported games and mod types, the names of
``.ini`` keywords, file names and extensions, buffer element/data types, colours, and the command-line
options.

:raw-html:`<br />`

BufDataTypeNames
================

.. autoclass:: FixRaidenBoss2.BufDataTypeNames
    :members:

:raw-html:`<br />`

BufDataTypes
============

.. autoclass:: FixRaidenBoss2.BufDataTypes

:raw-html:`<br />`

BufElementNames
===============

.. autoclass:: FixRaidenBoss2.BufElementNames
    :members:

:raw-html:`<br />`

BufElementTypes
===============

.. autoclass:: FixRaidenBoss2.BufElementTypes

:raw-html:`<br />`

BufFormatNames
==============

.. autoclass:: FixRaidenBoss2.BufFormatNames
    :members:

:raw-html:`<br />`

ByteSize
========

.. autoclass:: FixRaidenBoss2.ByteSize
    :members:

:raw-html:`<br />`

ColourConsts
============

.. autoclass:: FixRaidenBoss2.ColourConsts
    :members:

:raw-html:`<br />`

CommandOpts
===========

.. autoclass:: FixRaidenBoss2.CommandOpts
    :members:

:raw-html:`<br />`

DownloadMode
============

.. autoclass:: FixRaidenBoss2.DownloadMode
    :members:

:raw-html:`<br />`

DownloadModeTools
=================

.. attributetable:: FixRaidenBoss2.DownloadModeTools

.. autoclass:: FixRaidenBoss2.DownloadModeTools
    :members:
    :private-members:

:raw-html:`<br />`

FileEncodings
=============

.. autoclass:: FixRaidenBoss2.FileEncodings
    :members:

:raw-html:`<br />`

FileExt
=======

.. autoclass:: FixRaidenBoss2.FileExt
    :members:

:raw-html:`<br />`

FilePathConsts
==============

.. attributetable:: FixRaidenBoss2.FilePathConsts

.. autoclass:: FixRaidenBoss2.FilePathConsts
    :members:
    :private-members:

:raw-html:`<br />`

FilePrefixes
============

.. autoclass:: FixRaidenBoss2.FilePrefixes
    :members:

:raw-html:`<br />`

FileTypes
=========

.. autoclass:: FixRaidenBoss2.FileTypes
    :members:

:raw-html:`<br />`

GameTypes
=========

.. attributetable:: FixRaidenBoss2.GameTypes

.. autoclass:: FixRaidenBoss2.GameTypes
    :members:
    :private-members:

:raw-html:`<br />`

GlobalClassifiers
=================

.. autoclass:: FixRaidenBoss2.GlobalClassifiers

:raw-html:`<br />`

GlobalPackageManager
====================

.. autoclass:: FixRaidenBoss2.GlobalPackageManager

:raw-html:`<br />`

IfPredPartType
==============

.. autoclass:: FixRaidenBoss2.IfPredPartType
    :members:

:raw-html:`<br />`

IfPredPartTypeTools
===================

.. attributetable:: FixRaidenBoss2.IfPredPartTypeTools

.. autoclass:: FixRaidenBoss2.IfPredPartTypeTools
    :members:
    :private-members:

:raw-html:`<br />`

ImgFormats
==========

.. autoclass:: FixRaidenBoss2.ImgFormats
    :members:

:raw-html:`<br />`

IniGraphModObjKeywords
======================

.. autoclass:: FixRaidenBoss2.IniGraphModObjKeywords
    :members:

:raw-html:`<br />`

IniGraphReplaceMode
===================

.. autoclass:: FixRaidenBoss2.IniGraphReplaceMode
    :members:

:raw-html:`<br />`

IniKeywords
===========

.. autoclass:: FixRaidenBoss2.IniKeywords
    :members:

:raw-html:`<br />`

ModTypes
========

.. autoclass:: FixRaidenBoss2.ModTypes

:raw-html:`<br />`

RegFillMissingMode
==================

.. autoclass:: FixRaidenBoss2.RegFillMissingMode
    :members:

:raw-html:`<br />`

ShortCommandOpts
================

.. autoclass:: FixRaidenBoss2.ShortCommandOpts
    :members:

:raw-html:`<br />`

TexEngine
=========

.. autoclass:: FixRaidenBoss2.TexEngine
    :members:

:raw-html:`<br />`

TexMetadataNames
================

.. autoclass:: FixRaidenBoss2.TexMetadataNames
    :members:

:raw-html:`<br />`

:raw-html:`<br />`
:raw-html:`<br />`

Data
****

The data tables the library ships with for each supported character (hashes, indices, vertex group
remaps, ...), keyed by mod type and game version.

:raw-html:`<br />`

ModData
=======

.. autoclass:: FixRaidenBoss2.ModData

:raw-html:`<br />`

ModDataAssets
=============

.. autoclass:: FixRaidenBoss2.ModDataAssets

:raw-html:`<br />`

VGRemapDataBuilder
==================

.. attributetable:: FixRaidenBoss2.VGRemapDataBuilder

.. autoclass:: FixRaidenBoss2.VGRemapDataBuilder
    :members:
    :private-members:

:raw-html:`<br />`

:raw-html:`<br />`
:raw-html:`<br />`

Exceptions
**********

The exceptions raised by the API. Every one of them derives from :class:`Error`.

:raw-html:`<br />`

BadBufData
==========

.. attributetable:: FixRaidenBoss2.BadBufData

.. autoclass:: FixRaidenBoss2.BadBufData
    :members:
    :private-members:

:raw-html:`<br />`

BufFileNotRecognized
====================

.. attributetable:: FixRaidenBoss2.BufFileNotRecognized

.. autoclass:: FixRaidenBoss2.BufFileNotRecognized
    :members:
    :private-members:

:raw-html:`<br />`

ConflictingOptions
==================

.. attributetable:: FixRaidenBoss2.ConflictingOptions

.. autoclass:: FixRaidenBoss2.ConflictingOptions
    :members:
    :private-members:

:raw-html:`<br />`

DuplicateFileException
======================

.. attributetable:: FixRaidenBoss2.DuplicateFileException

.. autoclass:: FixRaidenBoss2.DuplicateFileException
    :members:
    :private-members:

:raw-html:`<br />`

Error
=====

.. attributetable:: FixRaidenBoss2.Error

.. autoclass:: FixRaidenBoss2.Error
    :members:
    :private-members:

:raw-html:`<br />`

FileException
=============

.. attributetable:: FixRaidenBoss2.FileException

.. autoclass:: FixRaidenBoss2.FileException
    :members:
    :private-members:

:raw-html:`<br />`

InvalidDownloadMode
===================

.. attributetable:: FixRaidenBoss2.InvalidDownloadMode

.. autoclass:: FixRaidenBoss2.InvalidDownloadMode
    :members:
    :private-members:

:raw-html:`<br />`

InvalidGameType
===============

.. attributetable:: FixRaidenBoss2.InvalidGameType

.. autoclass:: FixRaidenBoss2.InvalidGameType
    :members:
    :private-members:

:raw-html:`<br />`

InvalidModType
==============

.. attributetable:: FixRaidenBoss2.InvalidModType

.. autoclass:: FixRaidenBoss2.InvalidModType
    :members:
    :private-members:

:raw-html:`<br />`

MissingFileException
====================

.. attributetable:: FixRaidenBoss2.MissingFileException

.. autoclass:: FixRaidenBoss2.MissingFileException
    :members:
    :private-members:

:raw-html:`<br />`

NoModType
=========

.. attributetable:: FixRaidenBoss2.NoModType

.. autoclass:: FixRaidenBoss2.NoModType
    :members:
    :private-members:

:raw-html:`<br />`

RemapMissingBlendFile
=====================

.. attributetable:: FixRaidenBoss2.RemapMissingBlendFile

.. autoclass:: FixRaidenBoss2.RemapMissingBlendFile
    :members:
    :private-members:

:raw-html:`<br />`

SyntaxErr
=========

.. attributetable:: FixRaidenBoss2.SyntaxErr

.. autoclass:: FixRaidenBoss2.SyntaxErr
    :members:
    :private-members:

:raw-html:`<br />`

:raw-html:`<br />`
:raw-html:`<br />`

Tools
*****

Generic, reusable-outside-this-project building blocks -- data structures, algorithms, and
string/hash/graph utilities with no notion of what a "mod" or a ``.ini`` file even is. Contrast
with `Model`_ above, which is specifically about the mod-fixing domain.

:raw-html:`<br />`

AhoCorasickBuilder
==================

.. attributetable:: FixRaidenBoss2.AhoCorasickBuilder

.. autoclass:: FixRaidenBoss2.AhoCorasickBuilder
    :members:
    :private-members:

:raw-html:`<br />`

AhoCorasickDFA
==============

.. attributetable:: FixRaidenBoss2.AhoCorasickDFA

.. autoclass:: FixRaidenBoss2.AhoCorasickDFA
    :members:
    :private-members:

:raw-html:`<br />`

AhoCorasickSingleton
====================

.. attributetable:: FixRaidenBoss2.AhoCorasickSingleton

.. autoclass:: FixRaidenBoss2.AhoCorasickSingleton
    :members:
    :private-members:

:raw-html:`<br />`

Algo
====

.. attributetable:: FixRaidenBoss2.Algo

.. autoclass:: FixRaidenBoss2.Algo
    :members:
    :private-members:

:raw-html:`<br />`

appendAllToOrderedMultiMap
==========================

.. autofunction:: FixRaidenBoss2.appendAllToOrderedMultiMap

:raw-html:`<br />`

BaseAhoCorasickDFA
==================

.. attributetable:: FixRaidenBoss2.BaseAhoCorasickDFA

.. autoclass:: FixRaidenBoss2.BaseAhoCorasickDFA
    :members:
    :private-members:

:raw-html:`<br />`

BaseSLR1Parser
==============

.. attributetable:: FixRaidenBoss2.BaseSLR1Parser

.. autoclass:: FixRaidenBoss2.BaseSLR1Parser
    :members:
    :private-members:

:raw-html:`<br />`

BaseTokenizer
=============

.. attributetable:: FixRaidenBoss2.BaseTokenizer

.. autoclass:: FixRaidenBoss2.BaseTokenizer
    :members:
    :private-members:

:raw-html:`<br />`

Builder
=======

.. attributetable:: FixRaidenBoss2.Builder

.. autoclass:: FixRaidenBoss2.Builder
    :members:
    :private-members:

:raw-html:`<br />`

Cache
=====

.. attributetable:: FixRaidenBoss2.Cache

.. autoclass:: FixRaidenBoss2.Cache
    :members:
    :private-members:

:raw-html:`<br />`

ConcurrentManager
=================

.. attributetable:: FixRaidenBoss2.ConcurrentManager

.. autoclass:: FixRaidenBoss2.ConcurrentManager
    :members:
    :private-members:

:raw-html:`<br />`

CppAhoCorasickDFA
=================

.. attributetable:: FixRaidenBoss2.CppAhoCorasickDFA

.. autoclass:: FixRaidenBoss2.CppAhoCorasickDFA
    :members:
    :private-members:

:raw-html:`<br />`

CppAlgo
=======

.. attributetable:: FixRaidenBoss2.CppAlgo

.. autoclass:: FixRaidenBoss2.CppAlgo
    :members:
    :private-members:

:raw-html:`<br />`

CppHashTools
============

.. attributetable:: FixRaidenBoss2.CppHashTools

.. autoclass:: FixRaidenBoss2.CppHashTools
    :members:
    :private-members:

:raw-html:`<br />`

CppListTools
============

.. attributetable:: FixRaidenBoss2.CppListTools

.. autoclass:: FixRaidenBoss2.CppListTools
    :members:
    :private-members:

:raw-html:`<br />`

CppTrie
=======

.. attributetable:: FixRaidenBoss2.CppTrie

.. autoclass:: FixRaidenBoss2.CppTrie
    :members:
    :private-members:

:raw-html:`<br />`

CyAlgo
======

.. attributetable:: FixRaidenBoss2.CyAlgo

.. autoclass:: FixRaidenBoss2.CyAlgo
    :members:
    :private-members:

:raw-html:`<br />`

CyDictTools
===========

.. attributetable:: FixRaidenBoss2.CyDictTools

.. autoclass:: FixRaidenBoss2.CyDictTools
    :members:
    :private-members:

:raw-html:`<br />`

CyHashTools
===========

.. attributetable:: FixRaidenBoss2.CyHashTools

.. autoclass:: FixRaidenBoss2.CyHashTools
    :members:
    :private-members:

:raw-html:`<br />`

CyListTools
===========

.. attributetable:: FixRaidenBoss2.CyListTools

.. autoclass:: FixRaidenBoss2.CyListTools
    :members:
    :private-members:

:raw-html:`<br />`

DeferredEnum
============

.. autoclass:: FixRaidenBoss2.DeferredEnum
    :members:

:raw-html:`<br />`

DFA
===

.. attributetable:: FixRaidenBoss2.DFA

.. autoclass:: FixRaidenBoss2.DFA
    :members:
    :private-members:

:raw-html:`<br />`

DictTools
=========

.. attributetable:: FixRaidenBoss2.DictTools

.. autoclass:: FixRaidenBoss2.DictTools
    :members:
    :private-members:

:raw-html:`<br />`

FilePath
========

.. attributetable:: FixRaidenBoss2.FilePath

.. autoclass:: FixRaidenBoss2.FilePath
    :members:
    :private-members:

:raw-html:`<br />`

FileService
===========

.. attributetable:: FixRaidenBoss2.FileService

.. autoclass:: FixRaidenBoss2.FileService
    :members:
    :private-members:

:raw-html:`<br />`

FilteredTokenizer
=================

.. attributetable:: FixRaidenBoss2.FilteredTokenizer

.. autoclass:: FixRaidenBoss2.FilteredTokenizer
    :members:
    :private-members:

:raw-html:`<br />`

FlyweightBuilder
================

.. attributetable:: FixRaidenBoss2.FlyweightBuilder

.. autoclass:: FixRaidenBoss2.FlyweightBuilder
    :members:
    :private-members:

:raw-html:`<br />`

GraphTools
==========

.. attributetable:: FixRaidenBoss2.GraphTools

.. autoclass:: FixRaidenBoss2.GraphTools
    :members:
    :private-members:

:raw-html:`<br />`

Hash128
=======

.. attributetable:: FixRaidenBoss2.Hash128

.. autoclass:: FixRaidenBoss2.Hash128
    :members:
    :private-members:

:raw-html:`<br />`

Hash64
======

.. attributetable:: FixRaidenBoss2.Hash64

.. autoclass:: FixRaidenBoss2.Hash64
    :members:
    :private-members:

:raw-html:`<br />`

HashTools
=========

.. attributetable:: FixRaidenBoss2.HashTools

.. autoclass:: FixRaidenBoss2.HashTools
    :members:
    :private-members:

:raw-html:`<br />`

Heading
=======

.. attributetable:: FixRaidenBoss2.Heading

.. autoclass:: FixRaidenBoss2.Heading
    :members:
    :private-members:

:raw-html:`<br />`

HeapNode
========

.. attributetable:: FixRaidenBoss2.HeapNode

.. autoclass:: FixRaidenBoss2.HeapNode
    :members:
    :private-members:

:raw-html:`<br />`

IntTools
========

.. attributetable:: FixRaidenBoss2.IntTools

.. autoclass:: FixRaidenBoss2.IntTools
    :members:
    :private-members:

:raw-html:`<br />`

IOrderedMultiMap
================

.. attributetable:: FixRaidenBoss2.IOrderedMultiMap

.. autoclass:: FixRaidenBoss2.IOrderedMultiMap
    :members:
    :private-members:

:raw-html:`<br />`

KeyRemapData
============

.. attributetable:: FixRaidenBoss2.KeyRemapData

.. autoclass:: FixRaidenBoss2.KeyRemapData
    :members:
    :private-members:

:raw-html:`<br />`

ListTools
=========

.. attributetable:: FixRaidenBoss2.ListTools

.. autoclass:: FixRaidenBoss2.ListTools
    :members:
    :private-members:

:raw-html:`<br />`

LruCache
========

.. attributetable:: FixRaidenBoss2.LruCache

.. autoclass:: FixRaidenBoss2.LruCache
    :members:
    :private-members:

:raw-html:`<br />`

Node
====

.. attributetable:: FixRaidenBoss2.Node

.. autoclass:: FixRaidenBoss2.Node
    :members:
    :private-members:

:raw-html:`<br />`

OrderedMultiMap
===============

.. attributetable:: FixRaidenBoss2.OrderedMultiMap

.. autoclass:: FixRaidenBoss2.OrderedMultiMap
    :members:
    :private-members:

:raw-html:`<br />`

OrderedMultiMapIterator
=======================

.. attributetable:: FixRaidenBoss2.OrderedMultiMapIterator

.. autoclass:: FixRaidenBoss2.OrderedMultiMapIterator
    :members:
    :private-members:

:raw-html:`<br />`

OrderedMultiMapSqrt
===================

.. attributetable:: FixRaidenBoss2.OrderedMultiMapSqrt

.. autoclass:: FixRaidenBoss2.OrderedMultiMapSqrt
    :members:
    :private-members:

:raw-html:`<br />`

OrderedMultiMapSqrtIterator
===========================

.. attributetable:: FixRaidenBoss2.OrderedMultiMapSqrtIterator

.. autoclass:: FixRaidenBoss2.OrderedMultiMapSqrtIterator
    :members:
    :private-members:

:raw-html:`<br />`

PackageData
===========

.. attributetable:: FixRaidenBoss2.PackageData

.. autoclass:: FixRaidenBoss2.PackageData
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

PackageManager
==============

.. attributetable:: FixRaidenBoss2.PackageManager

.. autoclass:: FixRaidenBoss2.PackageManager
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

ParseContext
============

.. attributetable:: FixRaidenBoss2.ParseContext

.. autoclass:: FixRaidenBoss2.ParseContext
    :members:
    :private-members:

:raw-html:`<br />`

ParseNode
=========

.. attributetable:: FixRaidenBoss2.ParseNode

.. autoclass:: FixRaidenBoss2.ParseNode
    :members:
    :private-members:

:raw-html:`<br />`

ParseTree
=========

.. attributetable:: FixRaidenBoss2.ParseTree

.. autoclass:: FixRaidenBoss2.ParseTree
    :members:
    :private-members:

:raw-html:`<br />`

ProcessManager
==============

.. attributetable:: FixRaidenBoss2.ProcessManager

.. autoclass:: FixRaidenBoss2.ProcessManager
    :members:
    :private-members:

:raw-html:`<br />`

PyWrapAhoCorasickDFA
====================

.. attributetable:: FixRaidenBoss2.PyWrapAhoCorasickDFA

.. autoclass:: FixRaidenBoss2.PyWrapAhoCorasickDFA
    :members:
    :private-members:

:raw-html:`<br />`

Ranges
======

.. attributetable:: FixRaidenBoss2.Ranges

.. autoclass:: FixRaidenBoss2.Ranges
    :inherited-members:
    :members:
    :private-members:

:raw-html:`<br />`

RemappedKeyData
===============

.. attributetable:: FixRaidenBoss2.RemappedKeyData

.. autoclass:: FixRaidenBoss2.RemappedKeyData
    :members:
    :private-members:

:raw-html:`<br />`

ReplaceIf
=========

.. attributetable:: FixRaidenBoss2.ReplaceIf

.. autoclass:: FixRaidenBoss2.ReplaceIf
    :members:
    :private-members:

:raw-html:`<br />`

ReplaceList
===========

.. attributetable:: FixRaidenBoss2.ReplaceList

.. autoclass:: FixRaidenBoss2.ReplaceList
    :members:
    :private-members:

:raw-html:`<br />`

StrEnum
=======

.. autoclass:: FixRaidenBoss2.StrEnum
    :members:

:raw-html:`<br />`

TextTools
=========

.. attributetable:: FixRaidenBoss2.TextTools

.. autoclass:: FixRaidenBoss2.TextTools
    :members:
    :private-members:

:raw-html:`<br />`

ThreadManager
=============

.. attributetable:: FixRaidenBoss2.ThreadManager

.. autoclass:: FixRaidenBoss2.ThreadManager
    :members:
    :private-members:

:raw-html:`<br />`

Token
=====

.. attributetable:: FixRaidenBoss2.Token

.. autoclass:: FixRaidenBoss2.Token
    :members:
    :private-members:

:raw-html:`<br />`

Trie
====

.. attributetable:: FixRaidenBoss2.Trie

.. autoclass:: FixRaidenBoss2.Trie
    :members:
    :private-members:

:raw-html:`<br />`

Z3Context
=========

.. attributetable:: FixRaidenBoss2.Z3Context

.. autoclass:: FixRaidenBoss2.Z3Context
    :members:
    :private-members:

:raw-html:`<br />`

Z3Predicate
===========

.. attributetable:: FixRaidenBoss2.Z3Predicate

.. autoclass:: FixRaidenBoss2.Z3Predicate
    :members:
    :private-members:

:raw-html:`<br />`

.. _First Set: https://www.geeksforgeeks.org/compiler-design/first-set-in-syntax-analysis/
.. _Nullable Set: https://cs.stackexchange.com/questions/125274/defining-nullable-symbols-and-the-first-set-of-a-grammar
.. _Follow Set: https://www.geeksforgeeks.org/compiler-design/follow-set-in-syntax-analysis/
.. _Simplified Maximal Munch: https://en.wikipedia.org/wiki/Maximal_munch
.. _sympy: https://www.sympy.org/en/index.html
.. _sympy.Symbol: https://docs.sympy.org/latest/modules/core.html#sympy.core.symbol.Symbol
.. _sympy.Boolean: https://docs.sympy.org/latest/modules/logic.html#sympy.logic.boolalg.Boolean
.. _sympy logic query: https://docs.sympy.org/latest/modules/logic.html
.. _sympy's pretty function: https://docs.sympy.org/latest/tutorials/intro-tutorial/printing.html#unicode-pretty-printer
.. _post-order traversal: https://www.geeksforgeeks.org/dsa/postorder-traversal-of-binary-tree/
.. _python's builtin regex: https://docs.python.org/3/library/re.html
.. _regex library: https://pypi.org/project/regex/
.. _pandas: https://pandas.pydata.org/
.. _pandas DataFrame: https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html
.. _pandas.DataFrame: https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.html
.. _numpy: https://numpy.org/
.. _numpy array: https://numpy.org/doc/2.4/reference/generated/numpy.ndarray.html
.. _numpy.ndarray: https://numpy.org/doc/2.4/reference/generated/numpy.ndarray.html
.. _Blender: https://www.blender.org/
.. _DXGI format: https://learn.microsoft.com/en-us/windows/win32/api/dxgiformat/ne-dxgiformat-dxgi_format
.. _Generator: https://wiki.python.org/moin/Generators
.. _SimpleNamespace: https://docs.python.org/3/library/types.html#types.SimpleNamespace
.. _multipath pruning: https://artint.info/2e/html2e/ArtInt2e.Ch3.S7.SS2.html
.. _cycle pruning: https://artint.info/2e/html2e/ArtInt2e.Ch3.S7.SS1.html
.. _satisfiable (SAT) problems: https://en.wikipedia.org/wiki/Boolean_satisfiability_problem

.. _call graph: https://en.wikipedia.org/wiki/Call_graph
.. _call-with-return: https://en.wikipedia.org/wiki/Subroutine
.. _goto: https://en.wikipedia.org/wiki/Goto
.. _Python: https://www.python.org/
.. _branch: https://en.wikipedia.org/wiki/Branch_(computer_science)
.. _BFS: https://en.wikipedia.org/wiki/Breadth-first_search
.. _dataflow analysis: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _MUST: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _available expressions: https://en.wikipedia.org/wiki/Available_expression_set
.. _very busy expressions: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _fixpoint: https://en.wikipedia.org/wiki/Fixed_point_(mathematics)
.. _fixpoint iteration: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _worklist algorithm: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _Kildall's algorithm: https://en.wikipedia.org/wiki/Data-flow_analysis
.. _Z3: https://github.com/Z3Prover/z3
.. _adjacency list: https://www.geeksforgeeks.org/adjacency-list-meaning-definition-in-dsa
.. _Aho-Corasick: https://www.geeksforgeeks.org/aho-corasick-algorithm-pattern-searching/
.. _ASCII: https://en.wikipedia.org/wiki/ASCII
.. _BCn Encoding Algorithm: https://learn.microsoft.com/en-us/windows/win32/direct3d11/texture-block-compression-in-direct3d-11
.. _binary search: https://en.wikipedia.org/wiki/Binary_search
.. _bipartite graph: https://en.wikipedia.org/wiki/Bipartite_graph
.. _blend: https://en.wikipedia.org/wiki/Skeletal_animation
.. _built-in filter: https://docs.python.org/3/library/functions.html#filter
.. _CFG: https://en.wikipedia.org/wiki/Context-free_grammar
.. _CFG (Context Free Grammer): https://en.wikipedia.org/wiki/Context-free_grammar
.. _compare function: https://www.geeksforgeeks.org/how-compare-method-works-in-java/
.. _Compressonator: https://github.com/GPUOpen-Tools/compressonator
.. _defaultdict: https://docs.python.org/3/library/collections.html#collections.defaultdict
.. _DFA (Deterministic Finite Automaton): https://en.wikipedia.org/wiki/Deterministic_finite_automaton
.. _DFS: https://en.wikipedia.org/wiki/Depth-first_search
.. _Direct Draw Surface: https://en.wikipedia.org/wiki/DirectDraw_Surface
.. _endianness: https://en.wikipedia.org/wiki/Endianness
.. _Enum: https://docs.python.org/3/library/enum.html#enum.Enum
.. _floating point: https://en.wikipedia.org/wiki/C_data_types
.. _Flyweight Design Pattern: https://refactoring.guru/design-patterns/flyweight
.. _Gamma Correction: https://www.cambridgeincolour.com/tutorials/gamma-correction.htm
.. _graph: https://en.wikipedia.org/wiki/Graph_theory
.. _half precision floating point: https://en.wikipedia.org/wiki/Half-precision_floating-point_format
.. _hashable: https://docs.python.org/3/glossary.html#term-hashable
.. _heap: https://en.wikipedia.org/wiki/Heap_(data_structure)
.. _Highlight Shadow Approximation Reference: https://stackoverflow.com/questions/51591445/what-is-the-algorithm-behind-photoshops-highlight-or-shadow-alteration
.. _KVP: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _KVPs: https://en.wikipedia.org/wiki/Name%E2%80%93value_pair
.. _LRU Cache: https://en.wikipedia.org/wiki/Cache_replacement_policies#Least_recently_used_(LRU)
.. _Maximal Munch: https://en.wikipedia.org/wiki/Maximal_munch
.. _module: https://docs.python.org/3/tutorial/modules.html
.. _ORFix: https://github.com/leotorrez/LeoTools/blob/main/releases/ORFix.ini
.. _packaging.version.Version: https://packaging.pypa.io/en/stable/version.html#packaging.version.Version
.. _PEP 440: https://peps.python.org/pep-0440/
.. _PIL.Image: https://pillow.readthedocs.io/en/stable/reference/Image.html
.. _PIL.Image.Image.info: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.info
.. _PIL.ImageEnhance: https://pillow.readthedocs.io/en/stable/reference/ImageEnhance.html
.. _PIL.PixelAccess: https://pillow.readthedocs.io/en/stable/reference/PixelAccess.html
.. _Pillow: https://pillow.readthedocs.io/en/stable/index.html
.. _pip: https://pip.pypa.io/en/stable/
.. _pyahocorasick: https://pyahocorasick.readthedocs.io/en/latest/
.. _pyahocorasick.Automaton: https://pyahocorasick.readthedocs.io/en/latest/#automaton-class
.. _Pypi: https://pypi.org/project/AnimeGameRemap/
.. _readlines: https://docs.python.org/3/library/io.html#io.IOBase.readlines
.. _section: https://en.wikipedia.org/wiki/INI_file#Sections
.. _sections: https://en.wikipedia.org/wiki/INI_file#Sections
.. _signed integer: https://en.wikipedia.org/wiki/Integer_(computer_science)
.. _Simple Image Temperature/Tint Adjust Algorithm: https://tannerhelland.com/2014/07/01/simple-algorithms-adjusting-image-temperature-tint.html
.. _SLR(1): https://en.wikipedia.org/wiki/Simple_LR_parser
.. _standard base 64: https://en.wikipedia.org/wiki/Base64
.. _TexFx: https://github.com/leotorrez/LeoTools/blob/main/releases/TexFx.ini
.. _TextIOWrapper: https://docs.python.org/3/library/io.html#io.TextIOWrapper
.. _unsigned integer: https://en.wikipedia.org/wiki/Integer_(computer_science)
.. _unsigned normalized integer: https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
.. _unsigned normalized integers: https://learn.microsoft.com/en-us/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-data-conversion
