#ifndef AGRemapPyBind_PyGIMISectionClassifier_H
#define AGRemapPyBind_PyGIMISectionClassifier_H

#include <pybind11/pybind11.h>

#include "../../iftemplate/PyIfContentPart.h"  // reuses PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/strategies/iniParsers/GIMISectionClassifier.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief The core :cpp:class:`AGRemapCore::GIMISectionClassifier` specialization this binds
 */
using PyGIMISectionClassifierCore = AGRC::GIMISectionClassifier<py::object, py::object, PyObjectHash, PyObjectEqual>;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``GIMISectionClassifier`` -- a real subclass rather than a plain alias,
 because it keeps the caller's own `Python`_ objects for every constructor argument and re-derives
 the core members from them at the start of every :meth:`classify` :raw-html:`<br />`
 :raw-html:`<br />`

 That is option 3 of the three ways a binding can hold a `Python`_-supplied container (see this
 project's own architecture notes), and it is forced here rather than chosen: ``GIMIParser``'s own
 test suite builds a default classifier and then *assigns into*
 ``hashKeyOnlyToModObj``/``indexKeyToModObj`` afterwards, so a parse-once-at-construction binding
 would silently ignore everything the caller set up. Keeping the raw dicts also preserves object
 identity and honours in-place mutation, exactly as the pure-Python original did
 @endrst
 */
class PyGIMISectionClassifier: public PyGIMISectionClassifierCore {
    public:
        using Core = PyGIMISectionClassifierCore;
        using ModObj = Core::ModObj;

        /**
         * @brief Constructs a new classifier
         *
         * @param hashKeyOnlyToModObj The Python ``Dict[str, Tuple[str, str]]`` of mod objects identified by a ``hash`` alone
         * @param hashes The Python ``Hashes``
         * @param indexKeyToModObj The Python ``Dict[str, Dict[Tuple[str, str], Tuple[str, str]]]`` of mod objects that need a ``match_first_index`` too, or ``None``
         * @param indices The Python ``Indices``, or ``None``
         * @param version The version of the .ini file, or ``None``
         * @param hashNonVersionVals The filter values used when searching 'hashes'
         * @param indexNonVersionVals The filter values used when searching 'indices'
         */
        PyGIMISectionClassifier(py::object hashKeyOnlyToModObj, py::object hashes, py::object indexKeyToModObj,
                                 py::object indices, py::object version, py::object hashNonVersionVals,
                                 py::object indexNonVersionVals);

        /**
         * @brief The exact Python object given for ``hashKeyOnlyToModObj``
         */
        py::object hashKeyOnlyToModObjObj;

        /**
         * @brief The exact Python object given for ``indexKeyToModObj``
         */
        py::object indexKeyToModObjObj;

        /**
         * @brief The exact Python object given for ``hashes``
         */
        py::object hashesObj;

        /**
         * @brief The exact Python object given for ``indices``
         */
        py::object indicesObj;

        /**
         * @brief The exact Python object given for ``version``
         */
        py::object versionObj;

        /**
         * @brief The exact Python object given for ``hashNonVersionVals``
         */
        py::object hashNonVersionValsObj;

        /**
         * @brief The exact Python object given for ``indexNonVersionVals``
         */
        py::object indexNonVersionValsObj;

        /**
         * @brief
         @rst
         Re-derives every core member from the `Python`_ objects above -- see this class's own note
         @endrst
         */
        void refresh();

        /**
         * @brief #refresh, then classify -- the Python-visible ``classify``
         *
         * @param sectionName The name of the `section`_ the part belongs to
         * @param section The `section`_ the part belongs to, or ``None``
         * @param partKeys The current state of the `KVPs`_ for the part
         *
         * @return The classified mod objects, as a Python ``List[Tuple[str, str]]``
         */
        py::list classifyFromPy(const std::string &sectionName, const py::object &section, const py::object &partKeys);

        /**
         * @brief The .ini-domain customization points every Python-facing classifier uses
         */
        static Core::ClassifierConfig makeConfig();
};


void initCppGIMISectionClassifier(pybind11::module_ &m);

#endif
