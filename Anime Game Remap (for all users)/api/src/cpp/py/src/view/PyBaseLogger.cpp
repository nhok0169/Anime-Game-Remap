#include "PyBaseLogger.h"

#include <tuple>

namespace py = pybind11;
namespace AGRC = AGRemapCore;


// Python-visible shape of BaseLogger::headings(): AGRemapCore::Heading has no binding of its own
//   (the pure-Python 'Heading' in tools/Heading.py is still what the package's constants use), so
//   each open heading is handed back as a plain (title, sideLen, sideChar) tuple.
using PyHeadingEntry = std::tuple<std::string, std::size_t, std::string>;


// Builds the exact message the pure-Python Logger.handleException built --
//   f"\n{type(exception).__name__}: {exception}\n\n{traceback.format_exc()}" -- from a real Python
//   exception object. 'traceback' is looked up per call (not cached at init time) so the unit
//   tester's mock.patch("traceback.format_exc") is honoured.
static void handlePyException(AGRC::BaseLogger &self, const py::object &exception) {
    std::string exceptionType = py::str(py::type::of(exception).attr("__name__"));
    std::string message = py::str(exception);
    std::string traceback = py::str(py::module_::import("traceback").attr("format_exc")());

    self.handleException(exceptionType, message, traceback);
}


void initCppBaseLogger(pybind11::module_ &m) {
    py::class_<AGRC::BaseLogger, PyBindBaseLogger, py::smart_holder>(m, "BaseLogger", R"doc(
Base class for the *view* of the project's MVC architecture -- everything the remap reports back to the user
(progress messages, headings, lists, errors, prompts) goes through one of these :raw-html:`<br />` :raw-html:`<br />`

This class owns all of the *formatting* and *bookkeeping* (the prefix, the heading stack, the ``.txt`` log
transcript, the verbosity flags) and funnels every rendered line through exactly two abstract methods,
:meth:`write` and :meth:`read`, which a subclass implements for wherever the output actually needs to go:

* :class:`Logger` writes through ``print``/``input`` -- the console (CLI) view
* a GUI, or a backend server that needs to forward the messages to a frontend app, subclasses this and
  implements :meth:`write`/:meth:`read` for its own transport instead

Every higher-level method (:meth:`log`, :meth:`openHeading`, :meth:`error`, ...) can also be overridden, so a
view that wants *structured* events rather than pre-rendered text (e.g. a backend telling a frontend
"a heading opened", not "here is a line of ``=`` characters") can override at that level instead and never see
the text rendering at all. The defaults render text and route it through :meth:`log`, so overriding just
:meth:`write` is enough for any plain text sink :raw-html:`<br />` :raw-html:`<br />`

.. note::
    A subclass written in Python is fully supported -- its overrides are reached both from Python callers
    and from C++ code holding the logger through this base class

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

        .def_readonly_static("DefaultHeadingSideLen", &AGRC::BaseLogger::DefaultHeadingSideLen, py::doc(R"doc(
:class:`int`: The default number of characters for one side of a heading's border
        )doc"))

        .def_readonly_static("DefaultHeadingChar", &AGRC::BaseLogger::DefaultHeadingChar, py::doc(R"doc(
:class:`str`: The default character a heading's border is drawn with
        )doc"))

        .def_readonly_static("ErrorHeader", &AGRC::BaseLogger::ErrorHeader, py::doc(R"doc(
:class:`str`: The line printed above and below an error message (see :meth:`error`)
        )doc"))

        .def_readwrite("includePrefix", &AGRC::BaseLogger::includePrefix, py::doc(R"doc(
:class:`bool`: Whether to include the prefix string when printing out a message
        )doc"))

        .def_readwrite("verbose", &AGRC::BaseLogger::verbose, py::doc(R"doc(
:class:`bool`: Whether to print out output
        )doc"))

        .def_readwrite("logTxt", &AGRC::BaseLogger::logTxt, py::doc(R"doc(
:class:`bool`: Whether to log all the printed messages into a .txt file once the fix is done
        )doc"))

        .def_property("prefix", &AGRC::BaseLogger::prefix, &AGRC::BaseLogger::setPrefix, py::doc(R"doc(
:class:`str`: The line of text that is printed before any message is printed out
        )doc"))

        .def_property_readonly("loggedTxt", &AGRC::BaseLogger::loggedTxt, py::doc(R"doc(
:class:`str`: The text to be logged into a .txt file
        )doc"))

        .def_property_readonly("headings", [](const AGRC::BaseLogger &self) {
            std::vector<PyHeadingEntry> result;
            for (const AGRC::Heading &heading : self.headings()) {
                result.emplace_back(heading.title, heading.sideLen, heading.sideChar);
            }

            return result;
        }, py::doc(R"doc(
List[Tuple[:class:`str`, :class:`int`, :class:`str`]]: The stack of headings that have been opened (by calling :meth:`openHeading`), but have not been closed yet (have not called :meth:`closeHeading` yet) :raw-html:`<br />` :raw-html:`<br />`

Each heading is a ``(title, sideLen, headingChar)`` tuple, innermost (most recently opened) last. A fresh copy on every access
        )doc"))

        .def("clear", &AGRC::BaseLogger::clear, py::doc(R"doc(
Clears out any saved text from the logger
        )doc"))

        .def("write", &AGRC::BaseLogger::write, py::arg("message"), py::doc(R"doc(
Writes one already-rendered message to wherever this view displays output :raw-html:`<br />` :raw-html:`<br />`

The raw sink every printing method ends up in. Only called when :attr:`verbose` is ``True``. A ``message`` is a single
line without its trailing newline -- the sink adds whatever line ending it needs (matching ``print``) :raw-html:`<br />` :raw-html:`<br />`

**Abstract** -- a subclass must implement this

Parameters
----------
message: :class:`str`
    The rendered message to display
        )doc"))

        .def("read", &AGRC::BaseLogger::read, py::arg("desc"), py::doc(R"doc(
Asks the user for a line of input :raw-html:`<br />` :raw-html:`<br />`

The raw source :meth:`input` ends up in, matching the builtin ``input(desc)``: ``desc`` is displayed *without* a
trailing newline, then one line is read back :raw-html:`<br />` :raw-html:`<br />`

**Abstract** -- a subclass must implement this

Parameters
----------
desc: :class:`str`
    The question/description being asked to the user for input

Returns
-------
:class:`str`
    The resultant input the user entered
        )doc"))

        .def("getStr", &AGRC::BaseLogger::getStr, py::arg("message"), py::doc(R"doc(
Retrieves the string to be printed out by the logger

Parameters
----------
message: :class:`str`
    The message we want to print out

Returns
-------
:class:`str`
    The transformed text that the logger prints out
        )doc"))

        .def("log", &AGRC::BaseLogger::log, py::arg("message"), py::doc(R"doc(
Regularly prints text onto the console

Parameters
----------
message: :class:`str`
    The message we want to print out
        )doc"))

        .def("split", &AGRC::BaseLogger::split, py::doc(R"doc(
Prints out a new line, if anything has been logged since :attr:`prefix` was last set
        )doc"))

        .def("space", &AGRC::BaseLogger::space, py::doc(R"doc(
Prints out a space
        )doc"))

        .def("openHeading", &AGRC::BaseLogger::openHeading, py::arg("txt"),
             py::arg("sideLen") = AGRC::BaseLogger::DefaultHeadingSideLen, py::arg("headingChar") = AGRC::BaseLogger::DefaultHeadingChar, py::doc(R"doc(
Prints out an opening heading

Parameters
----------
txt: :class:`str`
    The message we want to print out

sideLen: :class:`int`
    How many characters we want for the side border of the heading :raw-html:`<br />`
    (see line 1 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: 2

headingChar: :class:`str`
    The type of character used to print the side border of the heading :raw-html:`<br />`
    (see line 3 of the example at :class:`Heading`) :raw-html:`<br />` :raw-html:`<br />`

    **Default**: "="
        )doc"))

        .def("closeHeading", &AGRC::BaseLogger::closeHeading, py::doc(R"doc(
Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at :class:`Heading`)
        )doc"))

        .def_static("getBulletStr", &AGRC::BaseLogger::getBulletStr, py::arg("txt"), py::doc(R"doc(
Creates the string for an item in an unordered list

Parameters
----------
txt: :class:`str`
    The message we want to print out

Returns
-------
:class:`str`
    The text formatted as an item in an unordered list
        )doc"))

        .def_static("getNumberedStr", &AGRC::BaseLogger::getNumberedStr, py::arg("txt"), py::arg("num"), py::doc(R"doc(
Creates the string for an ordered list

Parameters
----------
txt: :class:`str`
    The message we want to print out

num: :class:`int`
    The number we want to print out before the text for the ordered list

Returns
-------
:class:`str`
    The text formatted as an item in an ordered list
        )doc"))

        .def("bulletPoint", &AGRC::BaseLogger::bulletPoint, py::arg("txt"), py::doc(R"doc(
Prints out an item in an unordered list

Parameters
----------
txt: :class:`str`
    The message we want to print out
        )doc"))

        // 'transform' defaults to None and is converted here rather than declared as a
        //   std::function parameter with a default: the functional caster turns None into an empty
        //   std::function anyway, but a py::none() default keeps the signature honest in core.pyi
        //   (Optional[Callable]) instead of a baked cpp_function object.
        .def("list", [](AGRC::BaseLogger &self, const std::vector<std::string> &lst, const py::object &transform) {
            AGRC::BaseLogger::Transform transformFunc = nullptr;
            if (!transform.is_none()) {
                // The functional caster's own failure surfaces as a cast_error (-> RuntimeError);
                //   a bad argument type is a TypeError, the way the pure-Python original failed.
                if (!PyCallable_Check(transform.ptr())) {
                    throw py::type_error("'transform' must be callable or None");
                }

                transformFunc = transform.cast<AGRC::BaseLogger::Transform>();
            }

            self.list(lst, transformFunc);
        }, py::arg("lst"), py::arg("transform") = py::none(), py::doc(R"doc(
Prints out an ordered list

Parameters
----------
lst: List[:class:`str`]
    The list of messages we want to print out

transform: Optional[Callable[[:class:`str`], :class:`str`]]
    A function used to do any processing on each message in the list of messages

    If this parameter is ``None``, then the list of message will not go through any type of processing :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
        )doc"))

        .def("box", &AGRC::BaseLogger::box, py::arg("message"), py::arg("header"), py::doc(R"doc(
Prints the message to be sandwiched by the text defined in the argument, ``header``

Parameters
----------
message: :class:`str`
    The message we want to print out. Printed one line at a time

header: :class:`str`
    The string that we want to sandwich our message against
        )doc"))

        .def("error", &AGRC::BaseLogger::error, py::arg("message"), py::doc(R"doc(
Prints an error message :raw-html:`<br />` :raw-html:`<br />`

An error is always displayed, even when :attr:`verbose` is ``False`` -- unless the messages are being logged to a
.txt file (:attr:`logTxt`), in which case the current verbosity is respected and the error only ends up in
:attr:`loggedTxt`

Parameters
----------
message: :class:`str`
    The message we want to print out
        )doc"))

        .def("handleException", &handlePyException, py::arg("exception"), py::doc(R"doc(
Prints the message for an error

Parameters
----------
exception: :class:`BaseException`
    The error we want to handle
        )doc"))

        .def("handleException", py::overload_cast<const std::string&, const std::string&, const std::string&>(&AGRC::BaseLogger::handleException),
             py::arg("exceptionType"), py::arg("message"), py::arg("traceback") = "", py::doc(R"doc(
Prints the message for an error, from its already-separated parts -- for an error that did not come from a live
exception object (e.g. one reported by another process, or by the C++ core) :raw-html:`<br />` :raw-html:`<br />`

Renders the same text as the overload above and hands it to :meth:`error`

Parameters
----------
exceptionType: :class:`str`
    The name of the type of the error

message: :class:`str`
    The error's own message

traceback: :class:`str`
    Where the error came from, if known :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ""
        )doc"))

        .def("input", &AGRC::BaseLogger::input, py::arg("desc"), py::doc(R"doc(
Handles user input from the console

Parameters
----------
desc: :class:`str`
    The question/description being asked to the user for input

Returns
-------
:class:`str`
    The resultant input the user entered
        )doc"))

        .def("waitExit", &AGRC::BaseLogger::waitExit, py::doc(R"doc(
Prints the message used when the script finishes running, and waits for the user to press ENTER
        )doc"));
}
