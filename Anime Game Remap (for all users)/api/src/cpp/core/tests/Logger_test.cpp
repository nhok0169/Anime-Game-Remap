// -----------------------------------------------------------------------------
// Standalone test for AGRemapCore::BaseLogger / AGRemapCore::Logger (view/)
//
// What this covers that the Python suite cannot
// ---------------------------------------------
// The pybind11 'Logger' is its own BaseLogger subclass (PyLogger, writing through Python's
// print/input), so the std::ostream/std::istream-backed AGRemapCore::Logger -- the console view
// for a plain C++ consumer of the core -- is unreachable from Testing/Unit Tester. This file
// exercises it directly, plus the parts of BaseLogger that only a C++ caller can see:
// handleException(const std::exception&) and its type-name demangling, a C++ subclass sink,
// and the code-point (not byte) width of a non-ASCII closing heading.
//
// Not wired into any build. Compile by hand. Heading::close() counts graphemes, which drags in the
// StringTools/grapheme cone and utf8proc (the only external dependency -- no z3/ordered-map/xxHash):
//
//   cl /std:c++latest /EHsc /nologo /utf-8 /DUTF8PROC_STATIC /I "<core>/include" /I "<extern>/utf8proc" ^
//      Logger_test.cpp ^
//      "<core>/src/view/BaseLogger.cpp" "<core>/src/view/Logger.cpp" "<core>/src/tools/Heading.cpp" ^
//      "<core>/src/tools/StringTools.cpp" "<core>/src/tools/StringHash.cpp" ^
//      "<core>/src/tools/grapheme/GraphemeIterator.cpp" "<core>/src/tools/grapheme/GraphemeRange.cpp" ^
//      "<extern>/utf8proc/utf8proc.c" ^
//      /Fe:test.exe
//
//   g++ -std=c++23 -finput-charset=UTF-8 -DUTF8PROC_STATIC -I <core>/include -I <extern>/utf8proc \
//       Logger_test.cpp <core>/src/view/BaseLogger.cpp <core>/src/view/Logger.cpp <core>/src/tools/Heading.cpp \
//       <core>/src/tools/StringTools.cpp <core>/src/tools/StringHash.cpp \
//       <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp \
//       <extern>/utf8proc/utf8proc.c -o test
//
// Prints "ALL PASSED" and exits 0 on success; the first failing check prints its line and exits 1.
// -----------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "AGRemapCore/view/BaseLogger.h"
#include "AGRemapCore/view/Logger.h"

namespace AGRC = AGRemapCore;


static int g_checks = 0;

#define CHECK(cond) \
    do { \
        ++g_checks; \
        if (!(cond)) { \
            std::printf("FAILED (line %d): %s\n", __LINE__, #cond); \
            std::exit(1); \
        } \
    } while (false)

#define CHECK_EQ(a, b) \
    do { \
        ++g_checks; \
        const auto &lhs_ = (a); \
        const auto &rhs_ = (b); \
        if (!(lhs_ == rhs_)) { \
            std::printf("FAILED (line %d): %s == %s\n  got:      [%s]\n  expected: [%s]\n", __LINE__, #a, #b, \
                        std::string(lhs_).c_str(), std::string(rhs_).c_str()); \
            std::exit(1); \
        } \
    } while (false)


static const std::string ErrorHeader = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";


// A C++ view of the "keep everything" shape, mirroring test_BaseLogger.py's ListLogger
class VectorLogger: public AGRC::BaseLogger {
    public:
        using AGRC::BaseLogger::BaseLogger;

        std::vector<std::string> written;
        std::vector<std::string> asked;
        std::vector<std::string> answers;

        void write(const std::string& message) override {
            written.push_back(message);
        }

        std::string read(const std::string& desc) override {
            asked.push_back(desc);
            if (answers.empty()) {
                return "";
            }

            std::string result = answers.front();
            answers.erase(answers.begin());
            return result;
        }
};


namespace Custom {
    struct RemapFailed: public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}


static std::string joined(const std::vector<std::string>& lines) {
    std::string result;
    for (const std::string& line : lines) {
        result += line;
        result += "\n";
    }

    return result;
}


static void testStreamLogger() {
    std::ostringstream out;
    std::istringstream in("first answer\r\nsecond answer\n");
    AGRC::Logger logger("pre", true, true, &out, &in);

    CHECK(&logger.out() == &out);
    CHECK(&logger.in() == &in);
    CHECK_EQ(logger.prefix(), "pre");
    CHECK(logger.logTxt);
    CHECK(logger.verbose);
    CHECK(logger.includePrefix);

    logger.log("hello");
    CHECK_EQ(out.str(), "# pre --> hello\n");
    CHECK_EQ(logger.loggedTxt(), "# pre --> hello\n");

    logger.includePrefix = false;
    logger.verbose = false;
    logger.log("quiet");
    CHECK_EQ(out.str(), "# pre --> hello\n");           // not written
    CHECK_EQ(logger.loggedTxt(), "# pre --> hello\nquiet\n");  // but logged

    logger.verbose = true;
    std::string answer = logger.input("q? ");
    CHECK_EQ(answer, "first answer");                    // '\r' stripped
    CHECK_EQ(out.str(), "# pre --> hello\nq? ");          // desc written without a newline
    CHECK_EQ(logger.loggedTxt(), "# pre --> hello\nquiet\nq? \nInput: first answer\n");

    logger.waitExit();
    CHECK_EQ(out.str(), "# pre --> hello\nq? \n== Press ENTER to exit ==");
    CHECK(!logger.includePrefix);                        // restored to what it was (false)

    // exhausted input reads as empty, no throw
    CHECK_EQ(logger.read(""), "");

    // defaults go to std::cout/std::cin without touching them here
    AGRC::Logger console;
    CHECK(&console.out() != &out);
    CHECK_EQ(console.prefix(), "");
    CHECK(!console.logTxt);
    CHECK(console.verbose);

    logger.clear();
    CHECK_EQ(logger.loggedTxt(), "");
}


static void testFormatting() {
    VectorLogger logger;
    logger.includePrefix = false;

    // split only after something was logged under the current prefix
    logger.split();
    CHECK(logger.written.empty());
    logger.log("a");
    logger.split();
    CHECK_EQ(joined(logger.written), "a\n\n\n");
    logger.setPrefix("new");
    logger.split();
    CHECK_EQ(joined(logger.written), "a\n\n\n");           // prefix reset -> nothing yet
    logger.written.clear();

    logger.space();
    logger.bulletPoint("b");
    logger.list({"x", "y"});
    logger.list({"x"}, [](const std::string& s) { return s + "!"; });
    CHECK_EQ(joined(logger.written), "\n- b\n1. x\n2. y\n1. x!\n");
    logger.written.clear();

    CHECK_EQ(AGRC::BaseLogger::getBulletStr(""), "- ");
    CHECK_EQ(AGRC::BaseLogger::getNumberedStr("m", -123), "-123. m");
    CHECK_EQ(AGRC::BaseLogger::getNumberedStr("", 0), "0. ");

    // box keeps every piece of a Python-style split("\n")
    logger.box("", "#");
    logger.box("\n", "#");
    logger.box("a\n\nb\n", "#");
    CHECK_EQ(joined(logger.written), "#\n\n#\n#\n\n\n#\n#\na\n\nb\n\n#\n");
    logger.written.clear();

    // getStr with the prefix
    logger.includePrefix = true;
    CHECK_EQ(logger.getStr("m"), "# new --> m");
    logger.log("m");
    CHECK_EQ(joined(logger.written), "# new --> m\n");
}


static void testHeadings() {
    VectorLogger logger;
    logger.includePrefix = false;

    CHECK_EQ(AGRC::BaseLogger::DefaultHeadingChar, "=");
    CHECK(AGRC::BaseLogger::DefaultHeadingSideLen == 2);

    logger.closeHeading();                                // nothing open: no-op
    CHECK(logger.written.empty());

    logger.openHeading("default");
    logger.openHeading("wide", 5);
    logger.openHeading("star", 2, "*");
    CHECK(logger.headings().size() == 3);
    CHECK_EQ(logger.headings()[1].title, "wide");
    CHECK(logger.headings()[1].sideLen == 5);
    CHECK_EQ(logger.headings()[2].sideChar, "*");

    logger.closeHeading();
    logger.closeHeading();
    logger.closeHeading();
    logger.closeHeading();
    CHECK(logger.headings().empty());
    CHECK_EQ(joined(logger.written),
             "== default ==\n===== wide =====\n** star **\n**********\n================\n=============\n");
    logger.written.clear();

    // non-ASCII title: the closing line is as wide (in graphemes) as the opening one
    const std::string title = "\xE8\xA6\x8B\xE5\x87\xBA\xE3\x81\x97 \xF0\x9F\x90\xB1";  // "見出し 🐱": 3 + 1 + 1 graphemes, 14 bytes
    logger.openHeading(title, 3, "-");
    logger.closeHeading();
    CHECK_EQ(logger.written[0], "--- " + title + " ---");
    CHECK_EQ(logger.written[1], std::string(2 * (3 + 1) + 5, '-'));
    logger.written.clear();

    // graphemes, not code points: "e" + combining acute (2 code points, 1 grapheme), then a
    // family emoji ZWJ sequence (7 code points, 1 grapheme)
    const std::string combined = "e\xCC\x81 \xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7";
    logger.openHeading(combined, 1, "=");
    logger.closeHeading();
    CHECK_EQ(logger.written[1], std::string(2 * (1 + 1) + 3, '='));
}


static void testErrors() {
    VectorLogger logger;
    logger.includePrefix = false;

    // error() forces verbose on when nothing is being logged to a file, then restores it
    logger.verbose = false;
    logger.error("bad\nthing");
    CHECK_EQ(joined(logger.written), "\n" + ErrorHeader + "\nbad\nthing\n" + ErrorHeader + "\n\n");
    CHECK(!logger.verbose);
    logger.written.clear();

    // ...but respects it when it is
    logger.logTxt = true;
    logger.error("quiet");
    CHECK(logger.written.empty());
    CHECK_EQ(logger.loggedTxt(), "\n" + ErrorHeader + "\nquiet\n" + ErrorHeader + "\n\n");
    logger.logTxt = false;
    logger.clear();

    logger.verbose = true;
    logger.handleException("RemoteError", "msg", "tb");
    CHECK_EQ(joined(logger.written), "\n" + ErrorHeader + "\n\nRemoteError: msg\n\ntb\n" + ErrorHeader + "\n\n");
    logger.written.clear();

    logger.handleException("RemoteError", "msg");
    CHECK_EQ(joined(logger.written), "\n" + ErrorHeader + "\n\nRemoteError: msg\n\n\n" + ErrorHeader + "\n\n");
    logger.written.clear();

    // a live std::exception: dynamic type name, demangled / class-key stripped
    try {
        throw std::runtime_error("boom");
    } catch (const std::exception& e) {
        logger.handleException(e);
    }

    CHECK_EQ(joined(logger.written), "\n" + ErrorHeader + "\n\nstd::runtime_error: boom\n\n\n" + ErrorHeader + "\n\n");
    logger.written.clear();

    try {
        throw Custom::RemapFailed("nope");
    } catch (const std::exception& e) {
        logger.handleException(e);
    }

    CHECK_EQ(logger.written[3], "Custom::RemapFailed: nope");
}


static void testPolymorphicUse() {
    // A consumer written against the base, handed either view
    std::ostringstream out;
    AGRC::Logger console("c", false, true, &out);
    VectorLogger vec("v");

    std::vector<AGRC::BaseLogger*> views = {&console, &vec};
    for (AGRC::BaseLogger* view : views) {
        view->openHeading("Summary", 10);
        view->log("done");
        view->closeHeading();
    }

    const std::string closing(2 * (10 + 1) + 7, '=');
    CHECK_EQ(out.str(), "# c --> ========== Summary ==========\n# c --> done\n# c --> " + closing + "\n");
    CHECK_EQ(joined(vec.written), "# v --> ========== Summary ==========\n# v --> done\n# v --> " + closing + "\n");
}


int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    testStreamLogger();
    testFormatting();
    testHeadings();
    testErrors();
    testPolymorphicUse();

    std::printf("ALL PASSED (%d checks)\n", g_checks);
    return 0;
}
