#ifndef AGRemapPyBind_PyBaseIniParser_H
#define AGRemapPyBind_PyBaseIniParser_H

#include <pybind11/pybind11.h>

#include "../../iftemplate/PyIfContentPart.h"  // reuses PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/strategies/iniParsers/BaseIniParser.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The core :cpp:class:`AGRemapCore::BaseIniParser` specialization every `pybind11`_-facing parser is
 built on
 @endrst
 */
using PyBaseIniParserCore = AGRC::BaseIniParser<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``BaseIniParser`` -- :cpp:class:`AGRemapCore::BaseIniParser` plus the two
 pieces of `Python`_ state the (now deleted) pure-Python original carried :raw-html:`<br />`
 :raw-html:`<br />`

 A real subclass rather than a plain alias, because those two pieces have no core equivalent and
 could not:

 * ``_iniFile`` is the *`Python`_* ``IniFile`` (``model/files/IniFile.py``), an unrelated class to
   :cpp:class:`AGRemapCore::IniFile` -- see :cpp:class:`AGRemapCore::IniParseContext`'s own note.
   Real callers read it straight off a parser (``BaseIniFixer.__init__``'s
   ``self._iniFile = parser._iniFile``, ``GIMIFixer.getFix``, ``IniFixBuilder.build``), so it has to
   stay a plain attribute rather than becoming something they'd have to reach through a context
 * ``_modsToFix`` is a ``Set[str]`` the *fixers* own the lifecycle of, not the parser
   (``MultiModFixer`` assigns into it per mod being fixed and restores it afterwards;
   ``GIMIFixerOld``/``GIMIObjReplaceFixerOld`` read it). The C++ core deliberately dropped it --
   see :cpp:class:`AGRemapCore::BaseIniParser`'s own note -- so it lives here instead

 :raw-html:`<br />`

 .. note::
    Because it is a real subclass, it is also what :cpp:class:`AGRemapCore::GIMIParser`'s
    ``ParserBase`` template parameter is instantiated with, so ``GIMIParser`` inherits both of these
    and ``py::class_<PyGIMIParser, PyBaseIniParser>`` is genuine C++ inheritance. See that
    parameter's own documentation
 @endrst
 */
class PyBaseIniParser: public PyBaseIniParserCore {
    public:
        /**
         * @brief Constructs a new parser
         *
         * @param iniFile The Python ``IniFile`` to parse, or ``None``
         */
        explicit PyBaseIniParser(py::object iniFile = py::none());

        /**
         * @brief The Python ``IniFile`` that will be parsed -- Python-visible as ``_iniFile``
         */
        py::object iniFileObj;

        /**
         * @brief The names of the mods that will be fixed to -- Python-visible as ``_modsToFix``
         */
        py::object modsToFix;

        /**
         * @brief
         @rst
         Clears any saved data -- empties #modsToFix, matching the pure-Python original's own
         ``clear``
         @endrst
         */
        void clear() override;

        /**
         * @brief
         @rst
         Parses the ``.ini`` file and hands the result back as `Python`_ objects -- what the
         ``parse`` binding actually calls :raw-html:`<br />` :raw-html:`<br />`

         Separate from :cpp:func:`AGRemapCore::BaseIniParser::parse` because the two return
         genuinely different things: the core returns
         ``std::vector<IniGraphGroup<py::object, ...>>``, whose groups **own** their graphs, while
         a `Python`_ ``IniGraphGroup`` holds a real ``dict`` of the caller's own
         ``IniSectionGraph`` objects. Converting one into the other would mean deep-copying every
         graph on every parse and losing the `pybind11`_ keep-alive bookkeeping each one carries
         (see ``PyIniSectionGraph``'s own note), so each side builds its own result instead
         :raw-html:`<br />` :raw-html:`<br />`

         Returns an empty list here, matching the base parser's own empty result
         @endrst
         *
         * @return A ``List[IniGraphGroup]``
         */
        virtual py::object parseToPy();
};


/**
 * @brief
 @rst
 Binds the ``_iniFile``/``_modsToFix``/``clear``/``parse`` surface every parser shares onto an
 already-constructed ``py::class_`` :raw-html:`<br />` :raw-html:`<br />`

 ``clear``/``parse`` are re-bound per class rather than inherited from the ``BaseIniParser``
 registration so that each one dispatches to *that* class's C++ override -- the same reason
 ``bindBaseSLR1ParserCommonMethods`` exists for the parser family
 @endrst
 *
 * @tparam T The parser class being bound
 * @tparam PyClass The ``py::class_`` to chain onto
 *
 * @param cls The class to chain onto
 * @param parseDoc
 @rst
 The docstring for ``parse``. Passed in rather than written once here because what a parser
 actually hands back is the one thing that genuinely differs between them
 @endrst
 */
template <typename T, typename PyClass>
void bindBaseIniParserCommonMethods(PyClass &cls, const char *parseDoc) {
    cls.def_readwrite("_iniFile", &T::iniFileObj,
        py::doc(R"doc(:class:`IniFile`: The .ini file that will be parsed)doc"))

       .def_readwrite("_modsToFix", &T::modsToFix,
        py::doc(R"doc(Set[:class:`str`]: The names of the mods that will be fixed to)doc"))

       .def("clear", [](T &self) {
           self.clear();
       }, py::doc(R"doc(Clears any saved data)doc"))

       .def("parse", [](T &self) {
           return self.parseToPy();
       }, py::doc(parseDoc));
}


void initCppBaseIniParser(pybind11::module_ &m);

#endif
