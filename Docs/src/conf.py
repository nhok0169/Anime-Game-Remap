import os, sys, re


# Configuration file for the Sphinx documentation builder.

# -- Project information

project = 'AnimeGameRemap'
copyright = '2024, Albert Gold, nhok0169'
author = 'Albert Gold, nhok0169'

# read the version from the pyproject.toml from the project's pypi library
release = ''
with open('../../Anime Game Remap (for all users)/api/pyproject.toml') as f:
    text = f.read()
    releaseSearchResult = re.search(r"version\s*=\s*(" + '"' + r"|').*(" + '"' + r"|')", text, re.MULTILINE)
    releaseIndices = releaseSearchResult.span()
    release = text[releaseIndices[0] : releaseIndices[1]]
    release = release[:-1]
    releaseIndex = re.search('"' + r"|'", release).start()
    release = release[releaseIndex + 1:]

version = release

# -- add extensions from own repository ---

# path to the overall documentation
sys.path.insert(0, os.path.abspath('..'))

# path for some external libaries for the sphinx docs
sys.path.append(os.path.abspath('extensions'))

# path to the overall library
#
# note: insert(0, ...) rather than append(...) -- this has to take precedence over site-packages.
#   AnimeGameRemap/FixRaidenBoss2 is a published PyPI package, so a machine that has it pip-installed
#   (very easy to end up with, e.g. from testing the released build) would otherwise have autodoc and
#   the attributetable extension document *that* copy instead of the local one. The failure is not
#   subtle once the two versions diverge -- the released 4.5.4 lacks classes that exist on
#   development, giving "Extension error (attributetable): module 'FixRaidenBoss2' has no attribute
#   'BaseIniGraphEdit'" -- but it is very easy to misread as a docs bug rather than a shadowing one.
sys.path.insert(0, os.path.abspath('../../Anime Game Remap (for all users)/api/src/py'))

# the API's compiled modules may simply not be built here -- on Read the Docs they never are, since
#   building them would mean cloning every submodule and compiling z3 (~45 minutes, against a build
#   time limit). Nothing needs to be compiled for the docs, because everything that DESCRIBES the
#   compiled half is tracked: 'core/xml' for the C++ reference (read by breathe, see the bottom of this
#   file), and 'core.pyi' plus the Cython sources for everything autodoc and attributetable import
#   here. This steps aside for any module that IS built, so a maintainer's build is unchanged.
import compiledStubs

for message in compiledStubs.install(os.path.abspath('../../Anime Game Remap (for all users)/api/src/py/FixRaidenBoss2'),
                                     os.path.abspath('../../Anime Game Remap (for all users)/api/src/cy/src')):
    print(f"[compiledStubs] {message}")

# -----------------------------------------

# -- General configuration

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.napoleon',
    'sphinx.ext.intersphinx',
    'sphinx.ext.autosectionlabel',
    "sphinx_design",
    'attributetable',
    "cppattributetable",
    "breathe"
]

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'furo'

# -- Options for EPUB output
epub_show_urls = 'footnote'


# don't add the module names
add_module_names = False
toc_object_entries = False

autodoc_typehints = "description"

# Every class page also lists what it inherits, so a reader does not have to go up to the parent class
# to find a method (eg. IfPredTokenizer's methods are all defined on BaseTokenizer). Members that come
# from Python's own built-in types are left out, or a class like StrEnum (a str and an Enum) would list
# every method of str.
autodoc_default_options = {
    "inherited-members": "object,str,int,float,bool,bytes,dict,list,set,frozenset,tuple,"
                         "Enum,IntEnum,Flag,IntFlag,Exception,BaseException,ABC,Generic,"
                         "pybind11_object",
}

# Force autosectionlabel to prepend the filename to all section headings
#
# Note: If you want to reference some heading, do something like this:
#   :ref:\coreAPI:Tools
autosectionlabel_prefix_document = True

# The tutorial's choices each walk through their own STEP 1 / 2 / 3, so those headings share labels.
# Nothing references a STEP heading (a reference goes to the choice's own heading instead)
suppress_warnings = ["autosectionlabel.tutorial"]


# add the edit on github link
html_context = {
    "display_github": True,
    "github_user": "nhok0169",
    "github_repo": "Anime-Game-Remap",
    "github_version": "master",
    "conf_py_path": "/Docs/src/",
    "page_source_suffix": ".rst"
}

# These folders are copied to the documentation's HTML output
html_static_path = ['_static']

# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_css_files = [
    'css/styles.css',
]

# breathe reads a prepared copy of the committed Doxygen XML, in which the members each class inherits
#   (listed because the Doxyfile sets INLINE_INHERITED_MEMB) have ids of their own -- see doxygenInherited
import tempfile
import doxygenInherited

doxygenXmlSrc = os.path.abspath('../../Anime Game Remap (for all users)/api/src/cpp/core/xml')
doxygenXmlDst = os.path.join(tempfile.gettempdir(), "AGRemapDocs", "coreXml")
doxygenInheritedStats = doxygenInherited.prepare(doxygenXmlSrc, doxygenXmlDst)
print(f"[doxygenInherited] inherited members renamed: {doxygenInheritedStats['renamed']}, "
      f"dropped as already overridden: {doxygenInheritedStats['dropped']}")

breathe_projects = {
    "AGRemapCore": doxygenXmlDst
}

breathe_default_project = "AGRemapCore"
doxygen_xml_dir = breathe_projects["AGRemapCore"]