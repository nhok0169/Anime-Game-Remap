#ifndef AGRemapCore_BaseLogger_H
#define AGRemapCore_BaseLogger_H

#include <cstddef>
#include <deque>
#include <exception>
#include <functional>
#include <string>
#include <vector>

#include "AGRemapCore/tools/Heading.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class for the *view* of the project's MVC architecture -- everything the remap reports
     back to the user (progress messages, headings, lists, errors, prompts) goes through one of
     these :raw-html:`<br />` :raw-html:`<br />`

     This class owns all of the *formatting* and *bookkeeping* (the prefix, the heading stack,
     the ``.txt`` log transcript, the verbosity flags) and funnels every rendered line through
     exactly two pure virtual methods, ``write`` and ``read``, which a subclass implements for wherever
     the output actually needs to go:

     * :cpp:class:`Logger` writes to a ``std::ostream``/reads from a ``std::istream`` -- the CLI view
     * the `pybind11`_ ``Logger`` writes through `Python`_'s own ``print``/``input``
     * a future GUI / backend-server view would forward ``write`` to a window or a socket instead

     Every higher-level method (``log``, ``openHeading``, ``error``, ...) is also ``virtual``, so a view
     that wants *structured* events rather than pre-rendered text (e.g. a backend telling a
     frontend "a heading opened", not "here is a line of ``=`` characters") can override at that
     level instead and never see the text rendering at all. The defaults render text and route it
     through ``log``, so overriding just ``write`` is enough for any plain text sink.

     :raw-html:`<br />`

     .. note::
        Replaces the pure-Python ``Logger`` (``view/Logger.py``) outright -- the `pybind11`_
        ``Logger`` is a thin subclass of this class, see ``py/src/view/PyLogger.h``
     @endrst
     */
    class BaseLogger {
        public:

            #ifdef AGREMAPCORE_DOCS_PARSE
            #define Transform std::function<std::string(const std::string&)>
            #else
            /**
             * @brief A function used to do any processing on each message in a list of messages (see #list)
             */
            using Transform = std::function<std::string(const std::string&)>;
            #endif

            /**
             * @brief The default number of characters for one side of a heading's border
             */
            static constexpr std::size_t DefaultHeadingSideLen = 2;

            /**
             * @brief The default character a heading's border is drawn with
             */
            static const std::string DefaultHeadingChar;

            /**
             * @brief The line printed above and below an error message (see #error)
             */
            static const std::string ErrorHeader;

            /**
             * @brief Constructs a new logger
             *
             * @param prefix The line that is printed before any message is printed out. **Default**: ``""``
             * @param logTxt Whether to log all the printed messages into a ``.txt`` file once the fix is done. **Default**: ``false``
             * @param verbose Whether to print out output. **Default**: ``true``
             */
            explicit BaseLogger(std::string prefix = "", bool logTxt = false, bool verbose = true);

            virtual ~BaseLogger() = default;

            /**
             * @brief Whether to include the prefix string when printing out a message
             */
            bool includePrefix = true;

            /**
             * @brief Whether to print out output
             */
            bool verbose;

            /**
             * @brief Whether to log all the printed messages into a ``.txt`` file once the fix is done
             */
            bool logTxt;

            /**
             * @brief The line of text that is printed before any message is printed out
             */
            const std::string& prefix() const;

            /**
             * @brief Sets the line of text that is printed before any message is printed out
             *
             * @param newPrefix The new prefix
             */
            void setPrefix(std::string newPrefix);

            /**
             * @brief The text to be logged into a ``.txt`` file
             */
            const std::string& loggedTxt() const;

            /**
             * @brief The stack of headings that have been opened (by calling #openHeading), but have not been closed yet (have not called #closeHeading yet)
             */
            const std::deque<Heading>& headings() const;

            /**
             * @brief Clears out any saved text from the logger
             */
            void clear();

            /**
             * @brief
             @rst
             Writes one already-rendered message to wherever this view displays output :raw-html:`<br />` :raw-html:`<br />`

             The raw sink every printing method ends up in. Only called when ``verbose`` is ``true``.
             A ``message`` is a single line without its trailing newline -- the sink adds whatever
             line ending it needs (matching `Python`_'s ``print``)
             @endrst
             *
             * @param message The rendered message to display
             */
            virtual void write(const std::string& message) = 0;

            /**
             * @brief
             @rst
             Asks the user for a line of input :raw-html:`<br />` :raw-html:`<br />`

             The raw source ``input`` ends up in, matching `Python`_'s builtin ``input(desc)``:
             ``desc`` is displayed *without* a trailing newline, then one line is read back
             @endrst
             *
             * @param desc The question/description being asked to the user for input
             *
             * @return The resultant input the user entered
             */
            virtual std::string read(const std::string& desc) = 0;

            /**
             * @brief Retrieves the string to be printed out by the logger
             *
             * @param message The message we want to print out
             *
             * @return The transformed text that the logger prints out
             */
            virtual std::string getStr(const std::string& message) const;

            /**
             * @brief Regularly prints text onto the console
             *
             * @param message The message we want to print out
             */
            virtual void log(const std::string& message);

            /**
             * @brief Prints out a new line, if anything has been logged since the prefix was last set
             */
            virtual void split();

            /**
             * @brief Prints out a space
             */
            virtual void space();

            /**
             * @brief Prints out an opening heading
             *
             * @param txt The message we want to print out
             * @param sideLen How many characters we want for the side border of the heading (see line 1 of the example at \ref Heading). **Default**: #DefaultHeadingSideLen
             * @param headingChar The type of character used to print the side border of the heading (see line 3 of the example at \ref Heading). **Default**: #DefaultHeadingChar
             */
            virtual void openHeading(const std::string& txt, std::size_t sideLen = DefaultHeadingSideLen, const std::string& headingChar = DefaultHeadingChar);

            /**
             * @brief Prints out a closing heading that corresponds to a previous opening heading printed (see line 3 of the example at \ref Heading)
             */
            virtual void closeHeading();

            /**
             * @brief Creates the string for an item in an unordered list
             *
             * @param txt The message we want to print out
             *
             * @return The text formatted as an item in an unordered list
             */
            static std::string getBulletStr(const std::string& txt);

            /**
             * @brief Creates the string for an ordered list
             *
             * @param txt The message we want to print out
             * @param num The number we want to print out before the text for the ordered list
             *
             * @return The text formatted as an item in an ordered list
             */
            static std::string getNumberedStr(const std::string& txt, long long num);

            /**
             * @brief Prints out an item in an unordered list
             *
             * @param txt The message we want to print out
             */
            virtual void bulletPoint(const std::string& txt);

            /**
             * @brief Prints out an ordered list
             *
             * @param lst The list of messages we want to print out
             * @param transform A function used to do any processing on each message in the list of messages. If empty, the messages are printed as they are. **Default**: empty
             */
            virtual void list(const std::vector<std::string>& lst, const Transform& transform = nullptr);

            /**
             * @brief Prints the message to be sandwiched by the text defined in the argument, 'header'
             *
             * @param message The message we want to print out. Printed one line at a time
             * @param header The string that we want to sandwich our message against
             */
            virtual void box(const std::string& message, const std::string& header);

            /**
             * @brief
             @rst
             Prints an error message :raw-html:`<br />` :raw-html:`<br />`

             An error is always displayed, even when ``verbose`` is ``false`` -- unless the messages
             are being logged to a ``.txt`` file (``logTxt``), in which case the current verbosity is
             respected and the error only ends up in the transcript
             @endrst
             *
             * @param message The message we want to print out
             */
            virtual void error(const std::string& message);

            /**
             * @brief
             @rst
             Prints the message for an error, from its already-separated parts :raw-html:`<br />` :raw-html:`<br />`

             Renders ``"\n{exceptionType}: {message}\n\n{traceback}"`` and hands it to ``error`` --
             the same shape the pure-Python original built from ``type(e).__name__``/``str(e)``/
             ``traceback.format_exc()``. Not ``virtual``: a view that wants to treat errors
             specially overrides ``error``, which every error path funnels through
             @endrst
             *
             * @param exceptionType The name of the type of the error
             * @param message The error's own message
             * @param traceback Where the error came from, if known. **Default**: ``""``
             */
            void handleException(const std::string& exceptionType, const std::string& message, const std::string& traceback = "");

            /**
             * @brief Prints the message for an error
             *
             * @param exception The error we want to handle. Its type name (demangled where the compiler supports it) is used as the error's type
             */
            void handleException(const std::exception& exception);

            /**
             * @brief Handles user input from the console
             *
             * @param desc The question/description being asked to the user for input
             *
             * @return The resultant input the user entered
             */
            virtual std::string input(const std::string& desc);

            /**
             * @brief Prints the message used when the script finishes running, and waits for the user to press ENTER
             */
            virtual void waitExit();

        protected:
            /**
             * @brief Appends the text to the logged output to be printed to a ``.txt`` file, if #logTxt is set
             *
             * @param txt The text to be added onto the logged output
             */
            void addLogTxt(const std::string& txt);

        private:
            std::string _prefix;
            std::string _loggedTxt;
            std::deque<Heading> _headings;

            // The pure-Python original kept a '_currentPrefixTxt' string that every logged
            // message was appended onto, only ever *tested for emptiness* (by split()) and reset
            // by the prefix setter -- so it grew without bound for the whole run. A flag carries
            // the exact same observable behaviour.
            bool _loggedSincePrefixSet = false;
    };
}

#endif
