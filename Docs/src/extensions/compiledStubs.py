"""
Lets the docs build with no compiled API present, off what the repo already tracks.

``FixRaidenBoss2/__init__.py`` does an unconditional ``from .core import ...``, so ``autodoc`` and
``attributetable`` --- both of which import the real module --- cannot document a single class unless the
pybind11 extension and the four Cython extensions have been built. That is fine on a maintainer's machine
and impossible on Read the Docs, where building them would mean cloning every submodule and compiling
``z3`` (~45 minutes, against a build time limit).

Nothing has to be compiled, though, because a full description of every compiled module is tracked:

  * ``FixRaidenBoss2/core.pyi``, from ``pybind11-stubgen`` --- 210 classes carrying the complete numpydoc
    docstring of every method and property
  * ``api/src/cy/src/tools/*.pyx``, the Cython sources, which carry theirs in the same form
  * ``api/src/cpp/core/xml``, from Doxygen, which ``breathe`` reads for ``coreAPI.rst`` and which needs
    nothing from this module

So each missing extension module is served here by a *documentation-only* module built from whichever of
those describes it: real classes, real signatures, real docstrings, and bodies that return a stand-in
value. What is lost is only what neither a stub nor a source can carry --- the runtime *value* of a class
attribute, which is printed as whatever the stub recorded it to be.

Used from ``conf.py``::

    import compiledStubs
    compiledStubs.install(packageFolder, cythonFolder)

and it steps aside for any module that IS built, so a maintainer's build is unchanged. Setting
``AGREMAP_DOCS_STUBS=force`` in the environment makes it stand in even for the modules that are built,
which is how the Read the Docs path is tested locally without deleting anything.
"""

from __future__ import annotations

import ast
import importlib.abc
import importlib.util
import os
import pathlib
import re
import sys
from typing import Callable, Dict, List, Optional, Tuple

CompiledSuffixes = (".pyd", ".so", ".dll")

ForceEnvVar = "AGREMAP_DOCS_STUBS"

# The pure-Python layer does not only reference the compiled classes, it CALLS them while it is being
#   imported -- 'HashData = _CppHashes().repo.toNestedDict()' and the other data tables -- so a stub whose
#   methods return None gets as far as the first of those and no further. Every stubbed method returns this
#   instead: a value that answers to anything asked of it, and is empty or false wherever it has to be
#   something definite. None of it reaches the docs, which read a method's docstring and its signature.
Preamble = '''

class _StubValue:
    """
    Stands in for a value only the compiled module could produce.
    """

    def __init__(self, shown = "<a value that needs the compiled module>"):
        self._shown = shown

    def __getattr__(self, name):
        return _StubValue()

    def __call__(self, *args, **kwargs):
        return _StubValue()

    def __getitem__(self, key):
        return _StubValue()

    def __iter__(self):
        return iter(())

    def __len__(self):
        return 0

    def __bool__(self):
        return False

    def __contains__(self, item):
        return False

    def __eq__(self, other):
        # every unknown value is the SAME unknown value, so that a table the API builds with one as a key
        #   can still be looked up with another
        return isinstance(other, (_StubValue, _StubConstant))

    def __hash__(self):
        return 0

    def __str__(self):
        return ""

    def __repr__(self):
        return self._shown


class _StubConstant:
    """
    Stands in for a class attribute the stub gives no value for, printing what the stub says its value
    was.

    Deliberately NOT a _StubValue: autodoc asks an attribute for things like '__qualname__' with a
    default, and a catch-all '__getattr__' answers those with something that is not a string. Napoleon
    then unpacks it and the member is dropped with a warning.
    """

    def __init__(self, shown = "<a value that needs the compiled module>"):
        self._shown = shown

    def __eq__(self, other):
        return isinstance(other, (_StubConstant, _StubValue))

    def __hash__(self):
        return 0

    def __str__(self):
        return self._shown

    def __repr__(self):
        return self._shown

'''

# a method that must keep returning None, because python requires it or because a stand-in value would
#   change what the statement around it does
NoReturn = frozenset(["__init__", "__del__", "__setattr__", "__delattr__", "__setitem__", "__delitem__",
                      "__exit__", "__init_subclass__", "__set_name__"])


# ===== The text handling both routes need ====================================================


def splitTopLevel(text: str) -> List[str]:
    """
    Splits an argument list on its top level commas, so a default of ``{1: 2}`` or ``f(a, b)`` stays in
    one piece.
    """

    result = []
    depth = 0
    quote = ""
    current = ""

    for char in text:
        if (quote):
            if (char == quote):
                quote = ""
        elif (char in "\"'"):
            quote = char
        elif (char in "([{"):
            depth += 1
        elif (char in ")]}"):
            depth -= 1
        elif (char == "," and depth == 0):
            result.append(current)
            current = ""
            continue

        current += char

    if (current.strip()):
        result.append(current)

    return result


def readDocString(lines: List[str], index: int) -> Tuple[List[str], int]:
    """
    Reads the docstring that starts at or after ``lines[index]``, and returns it with the index just past
    it. Returns no lines at all if what follows is not a docstring.
    """

    start = index
    while (start < len(lines) and not lines[start].strip()):
        start += 1

    if (start >= len(lines)):
        return [], index

    opener = lines[start].strip()[:3]
    if (opener not in ('"""', "'''")):
        return [], index

    # a docstring written on one line closes on that same line
    if (len(lines[start].strip()) > 3 and lines[start].strip().endswith(opener)):
        return [lines[start]], start + 1

    end = start + 1
    while (end < len(lines) and opener not in lines[end]):
        end += 1

    return lines[start : end + 1], min(end + 1, len(lines))


# ===== Route 1: a .pyi from pybind11-stubgen =================================================


# a 'def' as pybind11-stubgen writes it: all on one line, always annotated, always returning
StubDefLine = re.compile(r"^(?P<head>\s*def\s+\w+\s*\()(?P<params>.*)(?P<tail>\)\s*->\s*.*:\s*)$")


def repairLine(line: str) -> Optional[str]:
    """
    Gives a default to every parameter that needs one to make ``line`` parse, and returns ``None`` if it
    is not a ``def`` line of the shape a stub uses.

    A binding whose ``py::arg`` list puts a *required* argument after a defaulted one is legal pybind11
    and illegal Python, so the stub generated from it is a file python refuses to parse. That is a real
    defect in the binding rather than anything to do with the docs --- this only keeps it from taking the
    whole docs build down with it, and :func:`install` reports every line it touched.
    """

    match = StubDefLine.match(line)
    if (match is None):
        return None

    seenDefault = False
    params = splitTopLevel(match.group("params"))
    changed = False

    for i, param in enumerate(params):
        # after a bare '*' or a '*args', a parameter without a default is keyword only and legal
        if (param.strip().startswith("*")):
            return line if (changed) else None

        if ("=" in param):
            seenDefault = True
        elif (seenDefault):
            params[i] = param + " = None"
            changed = True

    if (not changed):
        return None

    return match.group("head") + ",".join(params) + match.group("tail")


def repairStub(source: str, name: str) -> Tuple[str, List[str]]:
    """
    Returns ``source`` made parseable, along with a note for each line that had to be changed.
    """

    notes = []
    lines = source.splitlines(keepends = True)

    # bounded: every pass either fixes one line or gives up
    for _ in range(64):
        try:
            ast.parse("".join(lines))
            return "".join(lines), notes
        except SyntaxError as error:
            if (error.lineno is None or error.lineno > len(lines)):
                raise

            index = error.lineno - 1
            repaired = repairLine(lines[index])
            if (repaired is None):
                raise

            notes.append(f"{name}:{error.lineno}: {error.msg} -- gave the trailing parameter(s) a default "
                         f"so the stub can be imported")
            lines[index] = repaired

    raise SyntaxError(f"gave up repairing {name} after 64 passes")


def nodeSpan(node) -> Tuple[int, int]:
    """
    The 1-based line range a top level statement occupies, decorators included.
    """

    start = node.lineno
    for decorator in getattr(node, "decorator_list", []):
        start = min(start, decorator.lineno)

    return start, node.end_lineno


def fillBodies(source: str, name: str) -> Tuple[str, List[str]]:
    """
    Makes every stubbed method return a :class:`_StubValue` rather than ``None``, and returns the source
    with a note saying how many did.

    A stub writes every body as ``...``. That is enough for anything that only READS the class, and not
    enough for the API's own data modules, which call into the compiled core as they are imported.
    """

    tree = ast.parse(source)
    lines = source.splitlines()
    edits = []

    def isInert(statement):
        """
        A statement that says nothing: the docstring, or the ``...`` a stub writes when there is no
        docstring to write.
        """

        return (isinstance(statement, ast.Expr) and isinstance(statement.value, ast.Constant)
                and (isinstance(statement.value.value, str) or statement.value.value is Ellipsis))

    for node in ast.walk(tree):
        if (not isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef)) or node.name in NoReturn):
            continue

        # never touch a body that does something -- only a stub's empty one
        if (not all(isInert(statement) for statement in node.body)):
            continue

        first, last = node.body[0], node.body[-1]
        line = lines[first.lineno - 1]
        indent = line[: len(line) - len(line.lstrip())]

        # a trailing '...' is replaced; a docstring is followed
        if (isinstance(last.value.value, type(Ellipsis))):
            edits.append((last.lineno - 1, last.lineno, f"{indent}return _StubValue()"))
        else:
            edits.append((last.end_lineno, last.end_lineno, f"{indent}return _StubValue()"))

    if (not edits):
        return source, []

    # from the bottom up, so that an insertion does not move the line another edit is aimed at
    for start, end, text in sorted(edits, reverse = True):
        lines[start : end] = [text]

    rebuilt = "\n".join(lines) + "\n"
    ast.parse(rebuilt)

    return rebuilt, [f"{name}: {len(edits)} method(s) return a stand-in value, since the API's data tables "
                     f"call into the core while they are being imported"]


StubShownValue = re.compile(r"#\s*value\s*=\s*(?P<shown>.+?)\s*$")


def fillAttributes(source: str, name: str) -> Tuple[str, List[str]]:
    """
    Gives every class attribute a stand-in value, and returns the source with a note saying how many
    needed one.

    A stub writes an enum's members as ANNOTATIONS with no value --- ``Amber: typing.ClassVar[ModTypeId]``
    --- so the attribute does not exist at all, and the API's own data tables are keyed by
    ``ModTypeId.Amber``. The real value is in the trailing ``# value =`` comment, which is kept as what the
    stand-in prints, so the page reads the same as one built against the compiled module.
    """

    tree = ast.parse(source)
    lines = source.splitlines()
    edits = []

    for node in ast.walk(tree):
        if (not isinstance(node, ast.ClassDef)):
            continue

        for statement in node.body:
            if (not isinstance(statement, ast.AnnAssign) or statement.value is not None
                    or not isinstance(statement.target, ast.Name)):
                continue

            line = lines[statement.lineno - 1]
            indent = line[: len(line) - len(line.lstrip())]

            shown = StubShownValue.search(line)
            argument = repr(shown.group("shown")) if (shown is not None) else ""

            edits.append((statement.end_lineno, f"{indent}{statement.target.id} = _StubConstant({argument})"))

    if (not edits):
        return source, []

    # from the bottom up, so that an insertion does not move the line another edit is aimed at
    for at, text in sorted(edits, reverse = True):
        lines.insert(at, text)

    rebuilt = "\n".join(lines) + "\n"
    ast.parse(rebuilt)

    return rebuilt, [f"{name}: {len(edits)} class attribute(s) were annotations with no value, and were "
                     f"given a stand-in one"]


def reorderClasses(source: str, name: str) -> Tuple[str, List[str]]:
    """
    Puts every class after the classes it inherits from, and returns the source along with a note if
    anything had to move.

    ``pybind11-stubgen`` writes classes in ALPHABETICAL order, and a base class is the one part of a class
    statement python evaluates immediately -- so a stub is not runnable as written the moment a subclass
    sorts before its base, which ``BaseIniFixer(CppBaseIniFixer)`` does on the first letter of the
    alphabet. Nothing else in a stub cares about order: its functions are never called, and its only other
    top level statements are a docstring, imports and ``__all__``.

    The statements move as slices of the original text rather than being unparsed, so every signature and
    docstring is exactly the generated one.
    """

    tree = ast.parse(source)
    lines = source.splitlines()

    classes = [node for node in tree.body if isinstance(node, ast.ClassDef)]
    others = [node for node in tree.body if not isinstance(node, ast.ClassDef)]

    classNames = {node.name for node in classes}
    dependencies = {node.name: {base.id for base in node.bases
                                if isinstance(base, ast.Name) and base.id in classNames}
                    for node in classes}

    position = {node.name: i for i, node in enumerate(classes)}
    forwardBases = sum(1 for node in classes
                       for base in dependencies[node.name] if position[base] > position[node.name])

    ordered = []
    emitted = set()
    remaining = list(classes)

    while (remaining):
        ready = [node for node in remaining if dependencies[node.name] <= emitted]

        # python has no way to write a cycle of base classes, so this can only be a bug here
        if (not ready):
            raise ValueError(f"{name}: cannot order {len(remaining)} classes by their base classes")

        for node in ready:
            ordered.append(node)
            emitted.add(node.name)

        remaining = [node for node in remaining if node.name not in emitted]

    def textOf(node):
        start, end = nodeSpan(node)
        return "\n".join(lines[start - 1 : end])

    # the module docstring and a '__future__' import have to stay at the top of the file, so the preamble
    #   goes after them and before everything else
    head = 0
    while (head < len(others)
           and (isinstance(others[head], ast.Expr) and isinstance(others[head].value, ast.Constant)
                or isinstance(others[head], ast.ImportFrom) and others[head].module == "__future__")):
        head += 1

    rebuilt = "\n\n".join([textOf(node) for node in others[:head]] + [Preamble]
                          + [textOf(node) for node in others[head:]]
                          + [textOf(node) for node in ordered]) + "\n"

    # the rebuild is only correct if it did not lose or invent anything
    ast.parse(rebuilt)

    notes = []
    if (forwardBases):
        notes.append(f"{name}: {forwardBases} class(es) inherit from a class written later in the file, so "
                     f"the {len(classes)} classes were reordered to make the stub importable")

    return rebuilt, notes


def fromStub(path: pathlib.Path) -> Tuple[str, List[str]]:
    """
    A documentation-only module from a ``.pyi``.
    """

    source, notes = repairStub(path.read_text(encoding = "utf-8"), path.name)

    for step in (fillBodies, fillAttributes, reorderClasses):
        source, stepNotes = step(source, path.name)
        notes += stepNotes

    return source, notes


# ===== Route 2: a .pyx Cython source =========================================================


CythonName = re.compile(r"^#\s*distutils:\s*name\s*=\s*(?P<name>[\w.]+)\s*$")

CythonClassLine = re.compile(r"^(?:cdef\s+|cpdef\s+)?class\s+(?P<name>\w+)\s*(?:\([^)]*\))?\s*:\s*$")

# a method of that class: one indent level in, and ending the signature on its own line
CythonDefLine = re.compile(r"^ {4}(?:cpdef|cpdef\s+api|cdef|def)\s+(?P<before>[^()]*?)\((?P<args>.*)\)\s*:\s*$")

Identifier = re.compile(r"[A-Za-z_]\w*")


def cythonArg(arg: str) -> Optional[str]:
    """
    ``'dict nestedDict'`` -> ``'nestedDict'``, ``'bint ordered = True'`` -> ``'ordered = True'``, dropping
    the C type a Cython signature carries in front of the name.
    """

    arg = arg.strip()
    if (not arg):
        return None

    if (arg.startswith("*")):
        return arg

    declaration, _, default = arg.partition("=")
    names = Identifier.findall(declaration)
    if (not names):
        return None

    name = names[-1]

    return f"{name} = {default.strip()}" if (default.strip()) else name


def fromCython(path: pathlib.Path) -> Tuple[str, List[str]]:
    """
    A documentation-only module from a ``.pyx``.

    Only what the docs read is carried across --- the classes, their methods, and the docstrings of both
    --- because a Cython source is not Python and translating the rest of it would be a compiler. Every
    body returns a stand-in value, and a signature keeps its parameter names and defaults without their C
    types.
    """

    lines = path.read_text(encoding = "utf-8").splitlines()
    out = [Preamble]
    classes = 0
    methods = 0

    index = 0
    while (index < len(lines)):
        line = lines[index]

        classMatch = CythonClassLine.match(line)
        if (classMatch is not None):
            out.append(f"class {classMatch.group('name')}:")
            docLines, index = readDocString(lines, index + 1)
            out.extend(docLines if (docLines) else ["    ..."])
            out.append("")
            classes += 1
            continue

        defMatch = CythonDefLine.match(line)
        if (defMatch is not None):
            names = Identifier.findall(defMatch.group("before"))
            args = [cythonArg(arg) for arg in splitTopLevel(defMatch.group("args"))]
            args = [arg for arg in args if arg is not None]

            if (names):
                out.append(f"    def {names[-1]}({', '.join(args)}):")
                docLines, index = readDocString(lines, index + 1)
                out.extend(docLines)
                out.append("        return _StubValue()")
                out.append("")
                methods += 1
                continue

        index += 1

    source = "\n".join(out) + "\n"
    ast.parse(source)

    return source, [f"{path.name}: translated {classes} class(es) and {methods} method(s) of Cython source "
                    f"into a documentation-only module"]


def cythonSources(folder) -> Dict[str, pathlib.Path]:
    """
    Maps each extension module name to the ``.pyx`` that declares it, read off the ``distutils: name``
    line rather than guessed from the file name --- ``DictTools.pyx`` builds ``CyDictTools``.
    """

    result = {}
    if (folder is None):
        return result

    folder = pathlib.Path(folder)
    if (not folder.is_dir()):
        return result

    for source in sorted(folder.rglob("*.pyx")):
        for line in source.read_text(encoding = "utf-8").splitlines():
            match = CythonName.match(line)
            if (match is not None):
                result[match.group("name")] = source
                break

    return result


# ===== Serving them under the real module names ==============================================


SourceCache: Dict[str, Tuple[str, List[str]]] = {}


def buildSource(path: pathlib.Path, make: Callable[[pathlib.Path], Tuple[str, List[str]]]):
    """
    ``make(path)``, remembering the result so the check :func:`install` does is not paid for again when
    the module is imported.
    """

    key = str(path)
    if (key not in SourceCache):
        SourceCache[key] = make(path)

    return SourceCache[key]


class StubLoader(importlib.abc.Loader):
    """
    Executes a description of a module as the module itself.
    """

    def __init__(self, path: pathlib.Path, make):
        self.path = path
        self.make = make

    def create_module(self, spec):
        return None

    def exec_module(self, module):
        source, _ = buildSource(self.path, self.make)

        module.__file__ = str(self.path)
        module.__is_agremap_stub__ = True
        exec(compile(source, str(self.path), "exec"), module.__dict__)


class StubFinder(importlib.abc.MetaPathFinder):
    """
    Serves the stubbed submodules, and nothing else.
    """

    def __init__(self, stubs):
        self.stubs = stubs

    def find_spec(self, fullName, path = None, target = None):
        stub = self.stubs.get(fullName)
        if (stub is None):
            return None

        return importlib.util.spec_from_loader(fullName, StubLoader(*stub))


def isCompiled(packageFolder: pathlib.Path, moduleName: str) -> bool:
    """
    Whether ``moduleName`` has actually been built beside its stub.
    """

    for child in packageFolder.iterdir():
        if (child.suffix.lower() in CompiledSuffixes and child.name.split(".")[0] == moduleName):
            return True

    return False


def install(packageFolder, cythonFolder = None, force: Optional[bool] = None) -> List[str]:
    """
    Stands in for every extension module of ``packageFolder`` that has not been built, and returns the
    lines worth printing: what each one was built from, and every repair or translation it took.

    A module is described by its ``.pyx`` where there is one and by its ``.pyi`` otherwise --- the Cython
    stubs say nothing at all (they re-import the extension they are meant to describe), while the pybind11
    ones are complete.

    Passing ``force`` overrides the ``AGREMAP_DOCS_STUBS`` environment variable.
    """

    packageFolder = pathlib.Path(packageFolder)
    package = packageFolder.name

    if (force is None):
        force = os.environ.get(ForceEnvVar, "").strip().lower() == "force"

    pyx = cythonSources(cythonFolder)

    described = {}
    for stub in sorted(packageFolder.glob("*.pyi")):
        described[stub.name[: -len(".pyi")]] = (stub, fromStub)
    for moduleName, source in pyx.items():
        described[moduleName] = (source, fromCython)

    stubs = {}
    for moduleName, entry in sorted(described.items()):
        if (not force and isCompiled(packageFolder, moduleName)):
            continue

        stubs[f"{package}.{moduleName}"] = entry

    if (not stubs):
        return []

    sys.meta_path.insert(0, StubFinder(stubs))

    reason = "forced" if (force) else "not built here"
    messages = [f"standing in for {len(stubs)} extension module(s) ({reason}):"]

    # build every one NOW rather than when it is first imported, so that anything wrong with a description
    #   is reported by this call instead of surfacing much later as an autodoc import failure
    for fullName, (path, make) in sorted(stubs.items()):
        notes = buildSource(path, make)[1]
        messages.append(f"    {fullName} <- {path.name}")
        messages.extend(f"        {note}" for note in notes)

    return messages
