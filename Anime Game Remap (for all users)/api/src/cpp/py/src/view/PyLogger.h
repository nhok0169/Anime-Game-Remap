#ifndef AGRemapPyBind_PyLogger_H
#define AGRemapPyBind_PyLogger_H

#include <string>

#include <pybind11/pybind11.h>

#include "PyBaseLogger.h"
#include "AGRemapCore/view/BaseLogger.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


/**
 * @brief
 @rst
 The `pybind11`_-facing ``Logger`` -- the console view, as a :cpp:class:`AGRemapCore::BaseLogger`
 whose sink is `Python`_'s own ``print``/``input`` :raw-html:`<br />` :raw-html:`<br />`

 Deliberately **not** a binding of :cpp:class:`AGRemapCore::Logger` (the ``std::cout``-backed
 C++ console view), for the same reason every ``Py*`` strategy context forwards to `Python`_
 rather than reimplementing (see Architecture's "context seam" section):

 * ``std::cout`` and `Python`_'s ``sys.stdout`` are two separately-buffered streams, so lines
   written through one interleave out of order with lines written through the other -- and the
   package's `Python`_ callers still ``print`` directly around their logger calls
 * the unit tester patches ``builtins.print``/``builtins.input`` to capture what a logger says;
   a ``std::cout`` write is invisible to those patches

 Both lookups happen per call, through ``builtins``, precisely so that a ``mock.patch`` of either
 name is honoured. (``py::print`` would not do: it writes to ``sys.stdout`` directly and never
 goes through ``builtins.print`` at all.)
 @endrst
 */
class PyLogger: public AGRC::BaseLogger {
    public:
        /**
         * @brief Constructs a new logger
         *
         * @param prefix The line that is printed before any message is printed out. **Default**: ``""``
         * @param logTxt Whether to log all the printed messages into a ``.txt`` file once the fix is done. **Default**: ``false``
         * @param verbose Whether to print out output. **Default**: ``true``
         */
        explicit PyLogger(std::string prefix = "", bool logTxt = false, bool verbose = true);

        /**
         * @brief Calls `Python`_'s ``builtins.print(message)``
         *
         * @param message The rendered message to display
         */
        void write(const std::string& message) override;

        /**
         * @brief Calls `Python`_'s ``builtins.input(desc)``
         *
         * @param desc The question/description being asked to the user for input
         *
         * @return The line the user entered
         */
        std::string read(const std::string& desc) override;
};


/**
 * @brief
 @rst
 The trampoline for ``Logger``, so a `Python`_ subclass of the console view can redirect just
 ``write``/``read`` (or any other virtual) and still be reached from C++
 @endrst
 */
using PyBindLogger = PyBindLoggerT<PyLogger>;


void initCppLogger(pybind11::module_ &m);

#endif
