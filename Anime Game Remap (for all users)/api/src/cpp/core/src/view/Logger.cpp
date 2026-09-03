#include "AGRemapCore/view/Logger.h"

#include <iostream>
#include <utility>


namespace AGRemapCore {
    Logger::Logger(std::string prefix, bool logTxt, bool verbose, std::ostream* out, std::istream* in):
        BaseLogger(std::move(prefix), logTxt, verbose),
        _out(out != nullptr ? out : &std::cout),
        _in(in != nullptr ? in : &std::cin) {}


    void Logger::write(const std::string& message) {
        // Flushed per line, the way Python's print behaves on an interactive console -- a
        // progress log that only shows up once its buffer fills is not much of a progress log.
        *_out << message << std::endl;
    }


    std::string Logger::read(const std::string& desc) {
        *_out << desc << std::flush;

        std::string line;
        std::getline(*_in, line);

        // A stream fed CRLF text in binary mode (or a std::istringstream) leaves the '\r' on the
        // line; the console CRT already translates it away. Either way, the user did not type it.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        return line;
    }


    std::ostream& Logger::out() const {
        return *_out;
    }


    std::istream& Logger::in() const {
        return *_in;
    }
}
