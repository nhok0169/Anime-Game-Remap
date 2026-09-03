#include "AGRemapCore/view/BaseLogger.h"

#include <cstring>
#include <typeinfo>
#include <utility>

#if defined(__GNUG__)
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif


namespace AGRemapCore {
    const std::string BaseLogger::DefaultHeadingChar = "=";
    const std::string BaseLogger::ErrorHeader = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";


    namespace {
        // The human-readable name of an exception's dynamic type -- the C++ stand-in for the
        // pure-Python original's 'type(exception).__name__'.
        //
        // typeid(...).name() is implementation-defined: MSVC already returns a readable name but
        // decorated with the class-key ("class std::runtime_error"); GCC/Clang return the mangled
        // name ("St13runtime_error") and need the ABI demangler.
        std::string exceptionTypeName(const std::exception& exception) {
            const char* raw = typeid(exception).name();

            #if defined(__GNUG__)
            int status = 0;
            std::unique_ptr<char, void (*)(void*)> demangled(abi::__cxa_demangle(raw, nullptr, nullptr, &status), std::free);
            if (status == 0 && demangled) {
                return std::string(demangled.get());
            }

            return std::string(raw);
            #else
            std::string name = raw;
            for (const char* classKey : {"class ", "struct "}) {
                std::size_t keyLen = std::strlen(classKey);
                if (name.compare(0, keyLen, classKey) == 0) {
                    name.erase(0, keyLen);
                    break;
                }
            }

            return name;
            #endif
        }
    }


    BaseLogger::BaseLogger(std::string prefix, bool logTxt, bool verbose):
        verbose(verbose), logTxt(logTxt), _prefix(std::move(prefix)) {}


    const std::string& BaseLogger::prefix() const {
        return _prefix;
    }

    void BaseLogger::setPrefix(std::string newPrefix) {
        _prefix = std::move(newPrefix);
        _loggedSincePrefixSet = false;
    }

    const std::string& BaseLogger::loggedTxt() const {
        return _loggedTxt;
    }

    const std::deque<Heading>& BaseLogger::headings() const {
        return _headings;
    }

    void BaseLogger::clear() {
        _loggedTxt.clear();
    }


    void BaseLogger::addLogTxt(const std::string& txt) {
        if (logTxt) {
            _loggedTxt += txt;
            _loggedTxt += "\n";
        }
    }


    std::string BaseLogger::getStr(const std::string& message) const {
        return "# " + _prefix + " --> " + message;
    }


    void BaseLogger::log(const std::string& message) {
        std::string rendered = includePrefix ? getStr(message) : message;

        addLogTxt(rendered);
        _loggedSincePrefixSet = true;

        if (verbose) {
            write(rendered);
        }
    }


    void BaseLogger::split() {
        if (_loggedSincePrefixSet) {
            log("\n");
        }
    }


    void BaseLogger::space() {
        log("");
    }


    void BaseLogger::openHeading(const std::string& txt, std::size_t sideLen, const std::string& headingChar) {
        Heading heading(txt, sideLen, headingChar);
        _headings.push_back(heading);
        log(heading.open());
    }


    void BaseLogger::closeHeading() {
        if (_headings.empty()) {
            return;
        }

        Heading heading = _headings.back();
        _headings.pop_back();
        log(heading.close());
    }


    std::string BaseLogger::getBulletStr(const std::string& txt) {
        return "- " + txt;
    }


    std::string BaseLogger::getNumberedStr(const std::string& txt, long long num) {
        return std::to_string(num) + ". " + txt;
    }


    void BaseLogger::bulletPoint(const std::string& txt) {
        log(getBulletStr(txt));
    }


    void BaseLogger::list(const std::vector<std::string>& lst, const Transform& transform) {
        std::size_t lstLen = lst.size();
        for (std::size_t i = 0; i < lstLen; ++i) {
            std::string newTxt = transform ? transform(lst[i]) : lst[i];
            log(getNumberedStr(newTxt, static_cast<long long>(i + 1)));
        }
    }


    void BaseLogger::box(const std::string& message, const std::string& header) {
        log(header);

        // Python's str.split("\n"): every piece is kept, including the empty ones around a
        // leading/trailing/doubled newline, and an empty message is one empty line.
        std::size_t start = 0;
        while (true) {
            std::size_t end = message.find('\n', start);
            if (end == std::string::npos) {
                log(message.substr(start));
                break;
            }

            log(message.substr(start, end - start));
            start = end + 1;
        }

        log(header);
    }


    void BaseLogger::error(const std::string& message) {
        bool prevVerbose = verbose;
        if (!logTxt) {
            verbose = true;
        }

        space();
        box(message, ErrorHeader);
        space();

        verbose = prevVerbose;
    }


    void BaseLogger::handleException(const std::string& exceptionType, const std::string& message, const std::string& traceback) {
        error("\n" + exceptionType + ": " + message + "\n\n" + traceback);
    }


    void BaseLogger::handleException(const std::exception& exception) {
        handleException(exceptionTypeName(exception), exception.what());
    }


    std::string BaseLogger::input(const std::string& desc) {
        std::string rendered = includePrefix ? getStr(desc) : desc;

        addLogTxt(rendered);
        std::string result = read(rendered);
        addLogTxt("Input: " + result);

        return result;
    }


    void BaseLogger::waitExit() {
        bool prevIncludePrefix = includePrefix;
        includePrefix = false;
        input("\n== Press ENTER to exit ==");
        includePrefix = prevIncludePrefix;
    }
}
