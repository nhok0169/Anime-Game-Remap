import os, sys, re


# Configuration file for the Sphinx documentation builder.

# -- Project information

project = 'FixRaidenBoss2'
copyright = '2024, nhok0169, Albert Gold'
author = 'nhok0169, Albert Gold'

# read the version from the pyproject.toml from the project's pypi library
release = ''
with open('../../Fix-Raiden-Boss 2.0 (for all user )/api/pyproject.toml') as f:
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
sys.path.append(os.path.abspath('../../Fix-Raiden-Boss 2.0 (for all user )/api/src'))

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
    'attributetable'
]

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'sphinx_rtd_theme'

# -- Options for EPUB output
epub_show_urls = 'footnote'


# don't add the module names
add_module_names = False


# add the edit on github link
html_context = {
    "display_github": True,
    "github_user": "nhok0169",
    "github_repo": "Fix-Raiden-Boss",
    "github_version": "master",
    "conf_py_path": "/Docs/src/"
}

# These folders are copied to the documentation's HTML output
html_static_path = ['_static']

# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_css_files = [
    'css/styles.css',
]