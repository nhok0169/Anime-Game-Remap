// -----------------------------------------------------------------------------
// Standalone test for the grapheme-aware text helpers in AGRemapCore::StringTools
// (tools/StringTools.h) and the code that was moved onto them:
//
//   - StringTools::countGrapheme / isSpace / strip / lstrip / rstrip / toLower /
//     firstGraphemes / lastGraphemes / startsWith / endsWith / equalsIgnoreCase /
//     endsWithIgnoreCase, on emojis (ZWJ sequences, skin-tone modifiers), combining
//     marks, Unicode whitespace (NBSP, ideographic space, U+2028) and non-ASCII letters.
//   - GraphemeIterator on malformed UTF-8: a stray byte is its own 1-byte grapheme, and
//     iteration terminates (it used to add utf8proc's negative error length to the cursor).
//   - BaseAhoCorasickDFA::findMaximal(txt, count): the returned indices are grapheme
//     indices even after a multi-byte character, and the search resumes from the right
//     byte (it used to mix byte offsets with grapheme counts, so only ASCII text worked).
//   - IfPredPartTypeTools::getType: Unicode whitespace around / inside "else if".
//   - IniNamingTools::getObjRemapFixName: case-insensitive match of a non-ASCII object
//     name (the old byte-wise ASCII compare could not match "É" against "é").
//
// Non-ASCII text is written as explicit UTF-8 byte escapes so the file compiles the same
// under any source-charset setting (no /utf-8 flag needed).
//
// This file is not built by anything (see AI Agent Help/Testing). Compile it by hand,
// after vcvarsall.bat x64 -- no z3/ordered-map/xxHash needed, only utf8proc:
//
//   cl /std:c++latest /EHsc /nologo /DUTF8PROC_STATIC ^
//      /I <core>/include /I <utf8proc-src> ^
//      StringTools_grapheme_test.cpp ^
//      <utf8proc-src>/utf8proc.c ^
//      <core>/src/tools/StringTools.cpp <core>/src/tools/StringHash.cpp ^
//      <core>/src/tools/TextTools.cpp <core>/src/model/IniNamingTools.cpp ^
//      <core>/src/constants/IfPredPartType.cpp ^
//      <core>/src/tools/grapheme/GraphemeIterator.cpp <core>/src/tools/grapheme/GraphemeRange.cpp ^
//      /Fe:StringTools_grapheme_test.exe
//
// (g++/clang++: swap /std:c++latest /EHsc /I /Fe: for -std=c++23 -I ... -o, and
//  /DUTF8PROC_STATIC for -DUTF8PROC_STATIC)
// -----------------------------------------------------------------------------

#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/grapheme/GraphemeRange.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"
#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/IniNamingTools.h"

#include <cstdio>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace AGRemapCore;

static int failures = 0;
static int checks = 0;

static void expect(bool cond, const char* what) {
    ++checks;
    if (!cond) {
        ++failures;
        std::printf("FAIL: %s\n", what);
    }
}

// UTF-8 spellings of the characters used below.
static const std::string NBSP = "\xC2\xA0";                      // U+00A0 NO-BREAK SPACE
static const std::string IDEOGRAPHIC_SPACE = "\xE3\x80\x80";     // U+3000
static const std::string LINE_SEP = "\xE2\x80\xA8";              // U+2028 LINE SEPARATOR
static const std::string COMBINING_ACUTE = "\xCC\x81";           // U+0301
static const std::string E_ACUTE = "\xC3\xA9";                   // U+00E9 "é" (precomposed)
static const std::string E_ACUTE_UPPER = "\xC3\x89";             // U+00C9 "É"
static const std::string E_PLUS_ACUTE = "e" + COMBINING_ACUTE;   // "é" as 2 codepoints, 1 grapheme
static const std::string ZHE_UPPER = "\xD0\x96";                 // U+0416 Cyrillic "Ж"
static const std::string ZHE_LOWER = "\xD0\xB6";                 // U+0436 Cyrillic "ж"
static const std::string FAMILY = "\xF0\x9F\x91\xA8\xE2\x80\x8D\xF0\x9F\x91\xA9\xE2\x80\x8D\xF0\x9F\x91\xA7"; // 👨‍👩‍👧 (ZWJ sequence, 5 codepoints)
static const std::string THUMBS = "\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD";  // 👍🏽 (base + skin tone modifier)


static void testCountGrapheme() {
    std::printf("-- countGrapheme --\n");
    expect(StringTools::countGrapheme("") == 0, "countGrapheme: empty");
    expect(StringTools::countGrapheme("abc") == 3, "countGrapheme: ascii");
    expect(StringTools::countGrapheme("a" + FAMILY + THUMBS + E_PLUS_ACUTE) == 4, "countGrapheme: emoji ZWJ, modifier, combining mark are one each");
    expect(StringTools::countGrapheme("\r\n") == 1, "countGrapheme: CRLF is one grapheme");
}


static void testIsSpace() {
    std::printf("-- isSpace --\n");
    expect(StringTools::isSpace(" "), "isSpace: space");
    expect(StringTools::isSpace("\t"), "isSpace: tab");
    expect(StringTools::isSpace("\r\n"), "isSpace: CRLF");
    expect(StringTools::isSpace(NBSP), "isSpace: NBSP");
    expect(StringTools::isSpace(IDEOGRAPHIC_SPACE), "isSpace: ideographic space");
    expect(StringTools::isSpace(LINE_SEP), "isSpace: U+2028");
    expect(StringTools::isSpace("  \t"), "isSpace: run of whitespace");
    expect(!StringTools::isSpace(""), "isSpace: empty is not whitespace");
    expect(!StringTools::isSpace("a"), "isSpace: letter");
    expect(!StringTools::isSpace(" a"), "isSpace: mixed");
    expect(!StringTools::isSpace(" " + COMBINING_ACUTE), "isSpace: space carrying a combining mark is not whitespace");
    expect(!StringTools::isSpace("\xFF"), "isSpace: malformed byte is not whitespace");
}


static void testStrip() {
    std::printf("-- strip / lstrip / rstrip --\n");
    expect(StringTools::strip("  abc \t") == "abc", "strip: ascii whitespace");
    expect(StringTools::strip(NBSP + "abc" + IDEOGRAPHIC_SPACE) == "abc", "strip: Unicode whitespace on both ends");
    expect(StringTools::strip(" " + NBSP + "\t") == "", "strip: all whitespace -> empty");
    expect(StringTools::strip("") == "", "strip: empty");
    expect(StringTools::strip("\t" + FAMILY + "\n") == FAMILY, "strip: keeps an emoji ZWJ sequence intact");
    expect(StringTools::lstrip(" " + COMBINING_ACUTE + "x ") == " " + COMBINING_ACUTE + "x ", "lstrip: leading space with a combining mark is one non-whitespace grapheme");
    expect(StringTools::lstrip("  x  ") == "x  ", "lstrip: leading only");
    expect(StringTools::rstrip("  x  ") == "  x", "rstrip: trailing only");
    expect(StringTools::rstrip("x" + LINE_SEP + NBSP) == "x", "rstrip: Unicode whitespace");
    expect(StringTools::rstrip("x \xFF ") == "x \xFF", "rstrip: stops at a malformed byte");

    // The returned view must alias the input, not a copy.
    std::string src = " hello ";
    std::string_view stripped = StringTools::strip(src);
    expect(stripped.data() == src.data() + 1 && stripped.size() == 5, "strip: result is a view into the input");
}


static void testToLower() {
    std::printf("-- toLower --\n");
    expect(StringTools::toLower("ABC def") == "abc def", "toLower: ascii");
    expect(StringTools::toLower(E_ACUTE_UPPER) == E_ACUTE, "toLower: E-acute");
    expect(StringTools::toLower(ZHE_UPPER + "A") == ZHE_LOWER + "a", "toLower: Cyrillic");
    expect(StringTools::toLower(FAMILY + THUMBS) == FAMILY + THUMBS, "toLower: emoji untouched");
    expect(StringTools::toLower("\xFF" "A\xFE") == "\xFF" "a\xFE", "toLower: malformed bytes passed through, letters around them lowered");
    expect(StringTools::toLower("") == "", "toLower: empty");
}


static void testFirstLastGraphemes() {
    std::printf("-- firstGraphemes / lastGraphemes --\n");
    expect(StringTools::firstGraphemes(FAMILY + "ab", 1) == FAMILY, "firstGraphemes: whole ZWJ sequence");
    expect(StringTools::firstGraphemes(E_PLUS_ACUTE + "x", 1) == E_PLUS_ACUTE, "firstGraphemes: base + combining mark");
    expect(StringTools::firstGraphemes("ab", 5) == "ab", "firstGraphemes: more than available -> all");
    expect(StringTools::firstGraphemes("ab", 0) == "", "firstGraphemes: zero -> empty");
    expect(StringTools::firstGraphemes("", 3) == "", "firstGraphemes: empty input");

    expect(StringTools::lastGraphemes("ab" + THUMBS, 1) == THUMBS, "lastGraphemes: base + modifier");
    expect(StringTools::lastGraphemes("a" + E_PLUS_ACUTE, 1) == E_PLUS_ACUTE, "lastGraphemes: base + combining mark");
    expect(StringTools::lastGraphemes("abc", 2) == "bc", "lastGraphemes: ascii");
    expect(StringTools::lastGraphemes("ab", 0) == "", "lastGraphemes: zero -> empty");
    expect(StringTools::lastGraphemes("ab", 9) == "ab", "lastGraphemes: more than available -> all");

    std::string src = "xyz";
    std::string_view tail = StringTools::lastGraphemes(src, 2);
    expect(tail.data() == src.data() + 1, "lastGraphemes: result is a view into the input");
}


static void testPrefixSuffix() {
    std::printf("-- startsWith / endsWith / *IgnoreCase --\n");
    expect(StringTools::startsWith("abc", "ab"), "startsWith: ascii");
    expect(StringTools::startsWith("abc", ""), "startsWith: empty prefix");
    expect(!StringTools::startsWith("ab", "abc"), "startsWith: prefix longer than text");
    expect(StringTools::startsWith(E_ACUTE + "x", E_ACUTE), "startsWith: precomposed");
    expect(!StringTools::startsWith(E_PLUS_ACUTE + "x", "e"), "startsWith: never matches part of a grapheme");
    expect(std::string_view(E_PLUS_ACUTE + "x").starts_with("e"), "(sanity) the byte-wise check would have matched");

    expect(StringTools::endsWith("x.dds", ".dds"), "endsWith: ascii");
    expect(StringTools::endsWith("x", ""), "endsWith: empty suffix");
    expect(!StringTools::endsWith("x" + E_PLUS_ACUTE, "e"), "endsWith: never matches part of a grapheme");
    expect(!StringTools::endsWith("ds", ".dds"), "endsWith: suffix longer than text");

    expect(StringTools::equalsIgnoreCase(E_ACUTE_UPPER + "A", E_ACUTE + "a"), "equalsIgnoreCase: non-ASCII");
    expect(!StringTools::equalsIgnoreCase("a", "b"), "equalsIgnoreCase: different");
    expect(StringTools::endsWithIgnoreCase("foo.DDS", ".dds"), "endsWithIgnoreCase: ascii");
    expect(StringTools::endsWithIgnoreCase("x" + E_ACUTE_UPPER, E_ACUTE), "endsWithIgnoreCase: non-ASCII");
    expect(!StringTools::endsWithIgnoreCase("ds", ".dds"), "endsWithIgnoreCase: suffix longer than text");
    expect(!StringTools::endsWithIgnoreCase("x" + E_PLUS_ACUTE, "E"), "endsWithIgnoreCase: never matches part of a grapheme");
    expect(StringTools::endsWithIgnoreCase("x", ""), "endsWithIgnoreCase: empty suffix");
}


static void testGraphemeIteratorMalformed() {
    std::printf("-- GraphemeIterator on malformed UTF-8 --\n");
    auto collect = [](std::string_view txt) {
        std::vector<std::string> out;
        for (std::string_view g : GraphemeRange(txt)) {
            out.emplace_back(g);
            if (out.size() > 100) break;   // guard against a regression back into an endless loop
        }
        return out;
    };

    std::vector<std::string> g1 = collect("\xFF\xFE" "ab");
    expect(g1.size() == 4 && g1[0] == "\xFF" && g1[1] == "\xFE" && g1[2] == "a" && g1[3] == "b", "malformed lead bytes become 1-byte graphemes");

    std::vector<std::string> g2 = collect("a\xE3\x80");   // truncated 3-byte sequence at the end
    expect(g2.size() == 3 && g2[0] == "a" && g2[1] == "\xE3" && g2[2] == "\x80", "truncated multi-byte sequence at the end terminates");

    std::vector<std::string> g3 = collect("\xFF");
    expect(g3.size() == 1 && g3[0] == "\xFF", "lone malformed byte");

    std::vector<std::string> g4 = collect("a" + COMBINING_ACUTE + "\x80" + COMBINING_ACUTE);
    expect(g4.size() == 3 && g4[0] == "a" + COMBINING_ACUTE && g4[1] == "\x80" && g4[2] == COMBINING_ACUTE, "a malformed byte breaks the cluster on both sides");

    expect(StringTools::countGrapheme("\xFF\xFE\xFD") == 3, "countGrapheme agrees with GraphemeRange on malformed input");
}


static void testAhoCorasickFindMaximalCount() {
    std::printf("-- BaseAhoCorasickDFA::findMaximal(txt, count) on non-ASCII text --\n");
    // build(data) (re)builds the trie from 'data' -- a bare build() after add() would wipe it.
    BaseAhoCorasickDFA<std::unordered_set<int>> dfa;
    dfa.build(std::unordered_map<std::string, std::unordered_set<int>>{{"ab", {1}}, {"cd", {2}}});

    // graphemes: 0:"a" 1:"b" 2:FAMILY 3:"c" 4:"d" 5:E_PLUS_ACUTE 6:"a" 7:"b"
    std::string txt = "ab" + FAMILY + "cd" + E_PLUS_ACUTE + "ab";
    auto [keywords, inds] = dfa.findMaximal(std::string_view(txt), 10);

    expect(keywords.size() == 3, "findMaximal(count): three keywords found");
    expect(keywords.size() == 3 && keywords[0] == "ab" && keywords[1] == "cd" && keywords[2] == "ab", "findMaximal(count): keywords in order");
    expect(inds.size() == 3 && inds[0] == 0 && inds[1] == 3 && inds[2] == 6, "findMaximal(count): indices are grapheme indices past multi-byte characters");

    // Single-keyword overload agrees on the second match once the text starts after the emoji.
    size_t ind = 0;
    const std::string* kw = dfa.findMaximalPtr(std::string_view(txt).substr(2 + FAMILY.size()), &ind);
    expect(kw != nullptr && *kw == "cd" && ind == 0, "findMaximalPtr: baseline on the suffix");

    // 'count' limits how many are returned, and the walk stays on grapheme boundaries.
    auto [keywords2, inds2] = dfa.findMaximal(std::string_view(txt), 2);
    expect(keywords2.size() == 2 && inds2.size() == 2 && inds2[1] == 3, "findMaximal(count = 2): first two matches only");

    // With the empty keyword registered, unmatched graphemes are reported one at a time and the
    // trailing empty match sits at the grapheme length of the text, not its byte length.
    BaseAhoCorasickDFA<std::unordered_set<int>> dfaEmpty;
    dfaEmpty.build(std::unordered_map<std::string, std::unordered_set<int>>{{"", {0}}});
    std::string txt2 = "a" + FAMILY + "b";
    auto [keywords3, inds3] = dfaEmpty.findMaximal(std::string_view(txt2), 30);
    expect(keywords3.size() == 4, "findMaximal(count) with empty keyword: one per grapheme plus the trailing one");
    expect(inds3.size() == 4 && inds3[0] == 0 && inds3[1] == 1 && inds3[2] == 2 && inds3[3] == 3, "findMaximal(count) with empty keyword: grapheme indices 0..3");
}


static void testIfPredPartType() {
    std::printf("-- IfPredPartTypeTools::getType with Unicode whitespace --\n");
    expect(IfPredPartTypeTools::getType(NBSP + "if $x == 1") == IfPredPartType::If, "getType: NBSP before 'if'");
    expect(IfPredPartTypeTools::getType("ELSE" + IDEOGRAPHIC_SPACE + "IF $x") == IfPredPartType::Elif, "getType: ideographic space inside 'else if', uppercase");
    expect(IfPredPartTypeTools::getType("\tendif" + NBSP) == IfPredPartType::EndIf, "getType: endif");
    expect(IfPredPartTypeTools::getType("else") == IfPredPartType::Else, "getType: else");
    expect(!IfPredPartTypeTools::getType(E_ACUTE + "lse").has_value(), "getType: non-keyword");
}


static void testIniNamingToolsObjRemapFixName() {
    std::printf("-- IniNamingTools::getObjRemapFixName with a non-ASCII object name --\n");
    // objName ("élf", "body") is capitalized to "ÉlfBody" and looked up case-insensitively in
    // 'name', which spells it "ÉLFBODY" (uppercase É: C3 89, vs lowercase é: C3 A9 -- the old
    // byte-wise ASCII compare could never match these).
    std::string name = "Mod" + E_ACUTE_UPPER + "LFBODYThing";
    std::string result = IniNamingTools::getObjRemapFixName(name, "raiden", {E_ACUTE + "lf", "body"}, {"dog", "head"});
    std::string expected = IniNamingTools::getRemapFixName("ModDogHeadThing", "Raiden");
    expect(result == expected, "getObjRemapFixName: non-ASCII object name replaced case-insensitively");

    // The LAST occurrence is the one replaced, and the splice keeps the surrounding bytes exact.
    std::string name2 = E_ACUTE + "lfBody" + FAMILY + E_ACUTE_UPPER + "LFBODY";
    std::string result2 = IniNamingTools::getObjRemapFixName(name2, "raiden", {E_ACUTE + "lf", "body"}, {"dog", "head"});
    std::string expected2 = IniNamingTools::getRemapFixName(E_ACUTE + "lfBody" + FAMILY + "DogHead", "Raiden");
    expect(result2 == expected2, "getObjRemapFixName: last occurrence replaced, emoji in between untouched");

    // No match falls back to the "append" form, as before.
    std::string result3 = IniNamingTools::getObjRemapFixName("Plain", "raiden", {"x", "y"}, {"dog", "head"});
    expect(result3 == IniNamingTools::getRemapFixName("Plain", "RaidenDogHead"), "getObjRemapFixName: no match -> appended form");
}


int main() {
    testCountGrapheme();
    testIsSpace();
    testStrip();
    testToLower();
    testFirstLastGraphemes();
    testPrefixSuffix();
    testGraphemeIteratorMalformed();
    testAhoCorasickFindMaximalCount();
    testIfPredPartType();
    testIniNamingToolsObjRemapFixName();

    if (failures == 0) {
        std::printf("ALL PASSED (%d checks)\n", checks);
        return 0;
    }

    std::printf("%d of %d checks FAILED\n", failures, checks);
    return 1;
}
