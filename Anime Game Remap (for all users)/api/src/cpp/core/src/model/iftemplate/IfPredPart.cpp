#include "AGRemapCore/model/iftemplate/IfPredPart.h"

#include <cctype>
#include <memory>

#include "AGRemapCore/model/iftemplate/IfPredParser.h"
#include "AGRemapCore/model/iftemplate/IfPredZ3Generator.h"
#include "AGRemapCore/model/iftemplate/Z3IfPredGenerator.h"
#include "AGRemapCore/tools/StringTools.h"
#include "AGRemapCore/tools/parsing/IfPredTokenizer.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"

// Only IfPredPartType::Else needs this directly (to build a literal 'true' Z3Predicate with no
// parse tree involved) -- everything else here goes through IfPredZ3Generator/Z3IfPredGenerator,
// which are the ones that actually own the Z3-flavored logic.
#include "tools/z3/Z3Internal.h"


namespace AGRemapCore {

    namespace {
        // Same plain-ASCII lowercasing convention as IfPredPartTypeTools.cpp (see its own
        // comment) -- duplicated locally rather than shared, since it's a handful of lines and
        // this codebase has no existing "private shared helper" header for this kind of thing
        // outside of the Z3-specific pimpl internals.
        bool startsWithAscii(std::string_view txt, std::string_view prefix) {
            return txt.size() >= prefix.size() && txt.substr(0, prefix.size()) == prefix;
        }

        std::string toLowerAscii(std::string_view txt) {
            std::string result(txt);
            for (char& c : result) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            return result;
        }

        std::string_view lstripAscii(std::string_view txt) {
            size_t start = 0;
            while (start < txt.size() && std::isspace(static_cast<unsigned char>(txt[start]))) {
                ++start;
            }

            return txt.substr(start);
        }

        // Removes 'keyword' from 'src', but only if 'keyword' (case-insensitive) is the first
        // non-whitespace text in 'src' -- mirrors the pure-Python original's own
        // 'regex.sub(r"(?<=(^\s*))" + keyword, "", src, flags=re.IGNORECASE, count=1)'. Leading
        // whitespace before the keyword, and everything after it, is left untouched. No
        // word-boundary check after the keyword (matches IfPredPartTypeTools::getType's own
        // equally permissive prefix check -- not a bug, a deliberate consistency between
        // classification and stripping).
        std::string stripLeadingKeyword(const std::string& src, std::string_view keyword) {
            size_t i = 0;
            while (i < src.size() && std::isspace(static_cast<unsigned char>(src[i]))) {
                ++i;
            }

            if (!startsWithAscii(toLowerAscii(std::string_view(src).substr(i)), keyword)) {
                return src;
            }

            return src.substr(0, i) + src.substr(i + keyword.size());
        }

        // Removes 'keyword' from 'src', but only if 'keyword' (case-insensitive) is immediately
        // followed by nothing but trailing whitespace to the end of 'src' -- mirrors the
        // pure-Python original's own 'keyword + r"(?=(\s*$))"'. Only one position in 'src' can
        // ever satisfy that lookahead (the one right before the final run of trailing whitespace),
        // so this is an exact, not approximate, port.
        std::string stripTrailingKeyword(const std::string& src, std::string_view keyword) {
            size_t end = src.size();
            while (end > 0 && std::isspace(static_cast<unsigned char>(src[end - 1]))) {
                --end;
            }

            if (end < keyword.size()) {
                return src;
            }

            size_t start = end - keyword.size();
            if (toLowerAscii(std::string_view(src).substr(start, keyword.size())) != keyword) {
                return src;
            }

            return src.substr(0, start) + src.substr(start + keyword.size());
        }

        IfPredTokenizer& sharedTokenizer() {
            // A function-local static, same lazy-shared-singleton shape as IfTemplatePart.cpp's
            // own idGenerator() -- avoids rebuilding IfPredTokenizer's DFA on every single
            // IfPredPart construction (matching the pure-Python original's own GlobalCompilerParts
            // caching intent) without needing a separate registry class. simplifiedMaximalMunch/
            // parse below only ever *read* through the shared instances, never mutate DFA state,
            // so concurrent use across threads is assumed safe on that basis -- not independently
            // stress-tested for it, since nothing else in this codebase is written for concurrent
            // use either.
            static IfPredTokenizer tokenizer;
            return tokenizer;
        }

        IfPredParser& sharedParser() {
            static IfPredParser parser;
            return parser;
        }
    }

    IfPredPart::IfPredPart(std::string src, IfPredPartType type, Z3Context& z3Ctx, ParseContext* ctx,
                            std::optional<Z3Predicate> query, const std::optional<size_t>& id):
        IfTemplatePart(id), src(std::move(src)), type(type)
    {
        if (type == IfPredPartType::EndIf) {
            this->query = std::nullopt;
            return;
        }

        if (type == IfPredPartType::Else) {
            // No parse tree to run through IfPredZ3Generator for this trivial case -- build the
            // literal 'true' Z3Predicate directly (needs friend access to both Z3Context's and
            // Z3Predicate's own internals; see their header comments). The shared_ptr passed
            // alongside the expr is what keeps 'z3Ctx' alive for as long as this Z3Predicate is
            // (see Z3Internal.h's own comment on Z3Context::Impl/Z3Predicate::Impl).
            Z3Context::Impl& z3CtxImpl = z3Ctx.impl();
            this->query = Z3Predicate(std::make_unique<Z3Predicate::Impl>(z3CtxImpl.ctx->bool_val(true), z3CtxImpl.ctx));
            return;
        }

        if (query.has_value()) {
            this->query = std::move(query);
            return;
        }

        std::string testStr = getTestStr();

        // Only one of these two branches' work actually happens (mirroring the pure-Python
        // original's own if/else) -- avoid constructing a throwaway ParseContext when 'ctx' was
        // already supplied.
        std::optional<ParseContext> ownedCtx;
        ParseContext* activeCtx = ctx;
        if (activeCtx) {
            std::vector<std::string_view> testStrLines = StringTools::splitlines(testStr);
            activeCtx->lines.assign(testStrLines.begin(), testStrLines.end());
        } else {
            activeCtx = &ownedCtx.emplace(testStr);
        }

        this->query = getLogicQuery(*activeCtx, z3Ctx);
    }

    IfPredPart IfPredPart::clone(bool newId) const {
        IfPredPart copy(*this);
        if (newId) {
            copy.refreshId();
        }

        return copy;
    }

    std::string IfPredPart::getTestStr() const {
        static const std::string thenKeyword = "then";

        if (type != IfPredPartType::Elif) {
            std::string result = stripLeadingKeyword(src, IfPredPartTypeTools::getName(type));
            result = stripTrailingKeyword(result, thenKeyword);
            return result;
        }

        std::string cleanedSrc = toLowerAscii(lstripAscii(src));
        std::string elseKeyword = IfPredPartTypeTools::getName(IfPredPartType::Else);

        if (startsWithAscii(cleanedSrc, elseKeyword)) {
            std::string result = stripLeadingKeyword(src, elseKeyword);
            return stripLeadingKeyword(result, IfPredPartTypeTools::getName(IfPredPartType::If));
        }

        return stripLeadingKeyword(src, IfPredPartTypeTools::getName(IfPredPartType::Elif));
    }

    std::optional<Z3Predicate> IfPredPart::getLogicQuery(const ParseContext& ctx, Z3Context& z3Ctx) {
        std::vector<Token> tokens;
        try {
            tokens = sharedTokenizer().simplifiedMaximalMunch(ctx);
        } catch (const SyntaxErr&) {
            return std::nullopt;
        }

        // ParseTree has no default-constructible "empty" state (its constructor validates
        // 'rootId' against 'nodes' immediately), so there's no useful placeholder to declare
        // ahead of a successful parse() -- combine the parse+generate step into one try instead;
        // a SyntaxErr from parse() and any other exception from generate() both mean the same
        // thing here (return std::nullopt), matching the pure-Python original's own two separate
        // but identically-handled try/except blocks.
        try {
            auto tree = sharedParser().parse(tokens, &ctx);
            return IfPredZ3Generator::generate(tree, z3Ctx);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    std::optional<std::string> IfPredPart::getIfPredStr(const Z3Predicate& predicate) {
        try {
            return Z3IfPredGenerator::generate(predicate);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    std::optional<Z3Predicate> IfPredPart::reparent(const Z3Predicate& predicate, Z3Context& target) {
        std::optional<std::string> text = getIfPredStr(predicate);
        if (!text.has_value()) {
            return std::nullopt;
        }

        ParseContext ctx(*text);
        return getLogicQuery(ctx, target);
    }

    std::string IfPredPart::toStr(const std::optional<std::string>& linePrefix) const {
        std::string result = src;
        if (linePrefix.has_value()) {
            result = *linePrefix + std::string(lstripAscii(result));
        }

        if (result.empty()) {
            return result;
        }

        if (result.back() == '\n') {
            result.pop_back();
        }

        return result;
    }
}
