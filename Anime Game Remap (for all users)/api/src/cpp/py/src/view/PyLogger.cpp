#include "PyLogger.h"

#include <utility>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


PyLogger::PyLogger(std::string prefix, bool logTxt, bool verbose):
    AGRC::BaseLogger(std::move(prefix), logTxt, verbose) {}


void PyLogger::write(const std::string& message) {
    // builtins.print, looked up per call -- see the class comment for why not py::print, and why
    //   not a cached handle.
    py::module_::import("builtins").attr("print")(message);
}


std::string PyLogger::read(const std::string& desc) {
    return py::module_::import("builtins").attr("input")(desc).cast<std::string>();
}


void initCppLogger(pybind11::module_ &m) {
    py::class_<PyLogger, AGRC::BaseLogger, PyBindLogger, py::smart_holder>(m, "Logger", R"doc(
This class inherits from :class:`BaseLogger`

The console view -- pretty prints output to display on the console (through ``print``), and reads the user's
answers back with ``input``

.. note::
    This is the view every existing part of the package (:class:`RemapService`, :class:`Mod`, ...) is written
    against. To send the same messages somewhere else -- a GUI, a socket to a frontend app -- subclass
    :class:`BaseLogger` (or this class) and implement :meth:`write`/:meth:`read`

Parameters
----------
prefix: :class:`str`
    line that is printed before any message is printed out :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ""

logTxt: :class:`bool`
    Whether to log all the printed messages into a .txt file once the fix is done :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

verbose: :class:`bool`
    Whether to print out output :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``True``
    )doc")

        .def(py::init<std::string, bool, bool>(), py::arg("prefix") = "", py::arg("logTxt") = false, py::arg("verbose") = true)

        .def("write", &PyLogger::write, py::arg("message"), py::doc(R"doc(
Prints the message onto the console, with ``print``

Parameters
----------
message: :class:`str`
    The rendered message to display
        )doc"))

        .def("read", &PyLogger::read, py::arg("desc"), py::doc(R"doc(
Asks the user for a line of input on the console, with ``input``

Parameters
----------
desc: :class:`str`
    The question/description being asked to the user for input

Returns
-------
:class:`str`
    The resultant input the user entered
        )doc"));
}
