#ifndef AGRemapCore_Logger_H
#define AGRemapCore_Logger_H

#include <iosfwd>
#include <string>

#include "AGRemapCore/view/BaseLogger.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseLogger`

     The command-line view: a logger that displays its output on a ``std::ostream`` and takes its
     input from a ``std::istream`` -- the console by default :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is the view for a *plain C++* consumer of the core. The `pybind11`_ ``Logger`` is
        deliberately **not** this class -- it is its own :cpp:class:`BaseLogger` subclass that
        writes through `Python`_'s ``print``/``input`` instead (see ``py/src/view/PyLogger.h``),
        so that its output interleaves correctly with `Python`_'s own buffered ``sys.stdout`` and
        so the unit tester's ``builtins.print``/``builtins.input`` patches still see it
     @endrst
     */
    class Logger: public BaseLogger {
        public:

            /**
             * @brief Constructs a new logger
             *
             * @param prefix The line that is printed before any message is printed out. **Default**: ``""``
             * @param logTxt Whether to log all the printed messages into a ``.txt`` file once the fix is done. **Default**: ``false``
             * @param verbose Whether to print out output. **Default**: ``true``
             * @param out Where the output is displayed. Not owned -- must outlive the logger. If ``nullptr``, ``std::cout`` is used. **Default**: ``nullptr``
             * @param in Where input is read from. Not owned -- must outlive the logger. If ``nullptr``, ``std::cin`` is used. **Default**: ``nullptr``
             */
            explicit Logger(std::string prefix = "", bool logTxt = false, bool verbose = true, std::ostream* out = nullptr, std::istream* in = nullptr);

            /**
             * @brief Writes the message to #out, followed by a newline, and flushes
             *
             * @param message The rendered message to display
             */
            void write(const std::string& message) override;

            /**
             * @brief Writes 'desc' to #out without a newline, then reads one line from #in
             *
             * @param desc The question/description being asked to the user for input
             *
             * @return The line read, without its line ending. Empty if #in has no more input
             */
            std::string read(const std::string& desc) override;

            /**
             * @brief The stream the output is displayed on
             */
            std::ostream& out() const;

            /**
             * @brief The stream input is read from
             */
            std::istream& in() const;

        private:
            std::ostream* _out;
            std::istream* _in;
    };
}

#endif
