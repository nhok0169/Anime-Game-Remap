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

# Force autosectionlabel to prepend the filename to all section headings
#
# Note: If you want to reference some heading, do something like this:
#   :ref:\coreAPI:Tools
autosectionlabel_prefix_document = True


# add the edit on github link
html_context = {
    "display_github": True,
    "github_user": "nhok0169",
    "github_repo": "Anime-Game-Remap",
    "github_version": "nhok0169",
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

breathe_projects = {
    "AGRemapCore": os.path.abspath('../../Anime Game Remap (for all users)/api/src/cpp/core/xml')
}

breathe_default_project = "AGRemapCore"
doxygen_xml_dir = breathe_projects["AGRemapCore"]