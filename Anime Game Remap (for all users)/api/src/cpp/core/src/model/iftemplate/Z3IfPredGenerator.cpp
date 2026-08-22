#include "AGRemapCore/model/iftemplate/Z3IfPredGenerator.h"

#include "tools/z3/Z3Internal.h"

#include <stdexcept>
#include <string>
#include <vector>


namespace AGRemapCore {

    namespace {

        // A syntactic category for a rendered fragment of .ini predicate text. Unlike the
        // pure-Python SympyIfPredGenerator's own SympyIfPredData/SympyPrecedence pairing (a single
        // numeric precedence, plus an explicit "higherPrecedenceBracket" exception set for the
        // and/or split), this is shaped directly around the grammar's own fixed nonterminal chain:
        //
        //   pred >= test >= keyexpr(addexpr) >= multexpr >= nterm >= term(atom)
        //     (AndOr)  (Compare)   (AddSub)      (MulDiv)   (Not)    (Atom)
        //
        // Every level reduces from the next-tighter one via a plain "X reduce" production, so a
        // fragment of a given category is always usable, unwrapped, in any slot whose own required
        // category is the same or *looser* -- never tighter. This alone is enough to decide almost
        // every wrap decision (see rank()/reachableWithoutWrap() below); only two extra rules don't
        // fall out of that alone:
        //   1. 'addexpr -> addexpr MINUS multexpr' / 'multexpr -> multexpr SLASH nterm' aren't
        //      right-recursive, so a same-category fragment on the RIGHT of a subtract/divide still
        //      needs an explicit wrap even though it's nominally "reachable" (foldLeftAssoc below).
        //   2. and/or's own children are pre-flattened (flattenAndOr), so the only same-*rank*
        //      fragment that can still show up as a child is the *opposite* AndOr kind, which needs
        //      an explicit wrap of its own (renderAndOr/joinAndOr below).
        //
        // This is a deliberately different (stricter) shape than SympyIfPredGenerator's table: that
        // generator only ever walks a tree *produced by parsing* this same grammar, so it inherits
        // the grammar's own left-associativity for free and never actually has to reason about a
        // same-precedence right-hand child at all. This class walks an arbitrary Z3 expression DAG
        // instead -- Z3 is free to hand back eg. 'subtract(a, add(b, c))' directly, which the naive
        // "wrap only if strictly looser" rule would render as the wrong "a - b + c".
        enum class Category { AndOr, Compare, AddSub, MulDiv, Not, Atom };

        int rank(Category category) {
            switch (category) {
                case Category::AndOr: return 0;
                case Category::Compare: return 1;
                case Category::AddSub: return 2;
                case Category::MulDiv: return 3;
                case Category::Not: return 4;
                case Category::Atom: return 5;
            }

            return 5;
        }

        bool reachableWithoutWrap(Category child, Category atLeast) {
            return rank(child) >= rank(atLeast);
        }

        struct Fragment {
            std::string text;
            Category category;
        };

        std::string wrap(const Fragment& f) {
            return "(" + f.text + ")";
        }

        std::string negateDigits(std::string s) {
            if (!s.empty() && s[0] == '-') {
                return s.substr(1);
            }

            return "-" + s;
        }

        [[noreturn]] void unsupported(const z3::expr& e, const std::string& reason) {
            throw std::invalid_argument("Z3IfPredGenerator::generate: " + reason + " has no .ini predicate equivalent ('" + e.to_string() + "')");
        }

        std::vector<z3::expr> collectArgs(const z3::expr& e) {
            std::vector<z3::expr> args;
            unsigned n = e.num_args();
            args.reserve(n);

            for (unsigned i = 0; i < n; ++i) {
                args.push_back(e.arg(i));
            }

            return args;
        }

        bool isExactValue(const z3::expr& e, int value) {
            return z3::eq(e, e.ctx().real_val(value));
        }

        Fragment render(const z3::expr& e);

        // Renders 'e' for a slot that requires at least 'atLeast' -- wraps in the "( pred )"
        // bracket-loop bridge only when actually necessary (see reachableWithoutWrap above).
        Fragment asOperand(const z3::expr& e, Category atLeast) {
            Fragment f = render(e);
            if (reachableWithoutWrap(f.category, atLeast)) {
                return f;
            }

            return Fragment{wrap(f), Category::Atom};
        }

        // Renders an exact Real/Int-sorted numeral, optionally negated -- always exact, never
        // lossy. An exact terminating decimal is preferred for readability (matching what a human
        // -- or IfPredZ3Generator's own INT/FLOAT leaf handling -- would have written); a genuinely
        // repeating rational (eg. 1/3) falls back to the always-exact '(numerator / denominator)'
        // division expression, which the grammar's own 'divide' production already covers exactly.
        Fragment renderNumeral(const z3::expr& numeral, bool negate) {
            if (numeral.get_sort().is_int()) {
                std::string text;
                numeral.is_numeral(text);
                if (negate) {
                    text = negateDigits(text);
                }

                return Fragment{text, Category::Atom};
            }

            std::string numStr;
            std::string denStr;
            numeral.numerator().is_numeral(numStr);
            numeral.denominator().is_numeral(denStr);
            if (negate) {
                numStr = negateDigits(numStr);
            }

            if (denStr == "1") {
                return Fragment{numStr, Category::Atom};
            }

            z3::context& ctx = numeral.ctx();
            z3::expr signedValue = ctx.real_val((numStr + " / " + denStr).c_str());

            // Z3 appends a trailing '?' to get_decimal_string's output when it had to truncate a
            // non-terminating decimal -- check for that first, since re-parsing a '?'-suffixed
            // string isn't valid numeral syntax at all. Otherwise, still verify the candidate
            // round-trips back to the exact same value before trusting it (belt-and-suspenders:
            // confirmed no case in this codebase's own tests actually needs the '?' check to fire
            // on its own, but it's a cheap guard against ever trying to re-parse it).
            std::string candidate = signedValue.get_decimal_string(32);
            bool exact = candidate.find('?') == std::string::npos && z3::eq(ctx.real_val(candidate.c_str()), signedValue);

            if (exact) {
                return Fragment{candidate, Category::Atom};
            }

            return Fragment{"(" + numStr + " / " + denStr + ")", Category::Atom};
        }

        // Rewrites a unary minus -- the grammar has no production for a general "-expr" (only a
        // literal negative numeral token), so a non-numeral operand becomes the binary
        // 'addexpr -> addexpr MINUS multexpr' subtraction '0 - operand' instead, which is always
        // available and exact.
        Fragment renderUminus(const z3::expr& operand) {
            if (operand.is_numeral()) {
                return renderNumeral(operand, true);
            }

            Fragment inner = asOperand(operand, Category::MulDiv);
            return Fragment{"0 - " + inner.text, Category::AddSub};
        }

        Fragment renderNot(const z3::expr& operand) {
            Fragment inner = asOperand(operand, Category::Not);
            return Fragment{"!" + inner.text, Category::Not};
        }

        // Shared by eq/ne (is_eq()/a 2-arg is_distinct()), the pairwise expansion of an n-ary
        // distinct, xor (desugared to '!='), and every arithmetic comparison -- all of them are
        // just "two keyexpr-reachable operands joined by a comparison operator" (per the grammar,
        // 'test -> keyexpr COMPAREOP keyexpr' -- both sides need only reach AddSub, not Compare
        // itself, since comparisons don't chain).
        Fragment renderCompare(const z3::expr& lhs, const z3::expr& rhs, const std::string& opText) {
            Fragment left = asOperand(lhs, Category::AddSub);
            Fragment right = asOperand(rhs, Category::AddSub);
            return Fragment{left.text + opText + right.text, Category::Compare};
        }

        // Z3's n-ary distinct means "every pair is different", which is NOT the same as chaining
        // consecutive '!=' -- expand into a conjunction of every pairwise '!='.
        Fragment renderDistinct(const z3::expr& e) {
            unsigned n = e.num_args();
            if (n == 2) {
                return renderCompare(e.arg(0), e.arg(1), " != ");
            }

            std::string text;
            bool first = true;
            for (unsigned i = 0; i < n; ++i) {
                for (unsigned j = i + 1; j < n; ++j) {
                    Fragment pair = renderCompare(e.arg(i), e.arg(j), " != ");
                    if (!first) {
                        text += " && ";
                    }

                    text += pair.text;
                    first = false;
                }
            }

            return Fragment{text, Category::AndOr};
        }

        // Folds a strictly left-associative binary chain (add/subtract, multiply/divide) --
        // 'ownCategory' may reuse itself unwrapped only in the leftmost slot (matching the
        // grammar's own left recursion, eg. 'addexpr -> addexpr PLUS multexpr'); every other
        // (right-hand) operand needs the bracket-loop bridge both for a looser category AND for
        // 'ownCategory' itself, since the grammar has no right-recursive equivalent -- 'a - (b - c)'
        // and 'a - b - c' are genuinely different expressions, unlike the and/or case below.
        // 'rightSlotAtLeast' is the category one step tighter than 'ownCategory' (Not for
        // multiply/divide's own '... STAR nterm' right slot, MulDiv for add/subtract's own
        // '... PLUS multexpr' right slot).
        Fragment foldLeftAssoc(const std::vector<z3::expr>& operands, Category ownCategory, Category rightSlotAtLeast, const std::string& opText) {
            Fragment result = asOperand(operands.at(0), ownCategory);

            for (size_t i = 1; i < operands.size(); ++i) {
                Fragment rendered = render(operands[i]);
                bool needsWrap = (rendered.category == ownCategory) || !reachableWithoutWrap(rendered.category, rightSlotAtLeast);
                std::string rhsText = needsWrap ? wrap(rendered) : rendered.text;
                result = Fragment{result.text + opText + rhsText, ownCategory};
            }

            return result;
        }

        // Flattens a chain of the same n-ary, associative+commutative Z3 op (and/or) -- whether Z3
        // handed it to us as one flat n-ary node (eg. after simplify()) or as a left-nested chain
        // of binary applications (the shape IfPredZ3Generator's own foldChildren builds), both
        // collapse to the same clean joined text.
        void flattenAndOr(const z3::expr& e, bool wantAnd, std::vector<z3::expr>& out) {
            bool isSameKind = wantAnd ? e.is_and() : e.is_or();
            if (isSameKind) {
                unsigned n = e.num_args();
                for (unsigned i = 0; i < n; ++i) {
                    flattenAndOr(e.arg(i), wantAnd, out);
                }

                return;
            }

            out.push_back(e);
        }

        // Joins an already-flattened operand list -- the only operand that ever needs wrapping
        // here is the *opposite* AndOr kind (eg. an 'or' operand of an 'and'); everything else
        // (Compare/AddSub/MulDiv/Not/Atom) is already a valid 'test' shape as-is, and can never
        // itself be the *same* AndOr kind post-flattening.
        Fragment joinAndOr(const std::vector<z3::expr>& operands, bool isAnd) {
            std::string opText = isAnd ? " && " : " || ";
            std::string text;

            for (size_t i = 0; i < operands.size(); ++i) {
                Fragment operand = render(operands[i]);
                bool operandIsOppositeKind = (operand.category == Category::AndOr);
                std::string operandText = operandIsOppositeKind ? wrap(operand) : operand.text;

                if (i > 0) {
                    text += opText;
                }

                text += operandText;
            }

            return Fragment{text, Category::AndOr};
        }

        Fragment renderAndOr(const z3::expr& e, bool isAnd) {
            std::vector<z3::expr> operands;
            flattenAndOr(e, isAnd, operands);
            return joinAndOr(operands, isAnd);
        }

        // 'implies(a, b)' has no direct .ini token -- desugar to '!a || b', reusing the same
        // flatten/join machinery as a plain 'or' (so a 'b' that's itself an 'or' chain still
        // flattens cleanly).
        Fragment renderImplies(const z3::expr& a, const z3::expr& b) {
            z3::expr notA = !a;
            std::vector<z3::expr> operands;
            flattenAndOr(notA, false, operands);
            flattenAndOr(b, false, operands);
            return joinAndOr(operands, false);
        }

        // 'ite(boolExpr, 1, 0)' is exactly the shape IfPredZ3Generator::coerceToArith produces when
        // it needs to use a boolean value where an arithmetic one is expected (eg. '!$flag * 2').
        // Unwrap it back to just 'boolExpr''s own text -- it's already valid wherever this ite()
        // was found, since the .ini reader auto-coerces bool<->0/1 at parse time the same way
        // coerceToArith does going the other direction. Any other 'ite' shape has no .ini
        // equivalent at all (no ternary operator in the grammar).
        Fragment renderIte(const z3::expr& e) {
            z3::expr cond = e.arg(0);
            z3::expr thenBranch = e.arg(1);
            z3::expr elseBranch = e.arg(2);

            if (thenBranch.is_numeral() && elseBranch.is_numeral() && isExactValue(thenBranch, 1) && isExactValue(elseBranch, 0)) {
                return render(cond);
            }

            unsupported(e, "a general 'ite' (only the 'ite(boolExpr, 1, 0)' coercion shape is supported)");
        }

        Fragment renderVariable(const z3::expr& e) {
            std::string name = e.decl().name().str();
            if (name.empty() || name[0] != '$') {
                unsupported(e, "a named constant not shaped like a '$name' .ini variable");
            }

            return Fragment{name, Category::Atom};
        }

        Fragment render(const z3::expr& e) {
            if (e.is_numeral()) {
                return renderNumeral(e, false);
            }
            if (e.is_true()) {
                return Fragment{"1", Category::Atom};
            }
            if (e.is_false()) {
                return Fragment{"0", Category::Atom};
            }
            if (e.is_and()) {
                return renderAndOr(e, true);
            }
            if (e.is_or()) {
                return renderAndOr(e, false);
            }
            if (e.is_not()) {
                return renderNot(e.arg(0));
            }
            if (e.is_eq()) {
                return renderCompare(e.arg(0), e.arg(1), " == ");
            }
            if (e.is_distinct()) {
                return renderDistinct(e);
            }
            if (e.is_ite()) {
                return renderIte(e);
            }

            if (e.is_app()) {
                switch (e.decl().decl_kind()) {
                    case Z3_OP_LE: return renderCompare(e.arg(0), e.arg(1), " <= ");
                    case Z3_OP_GE: return renderCompare(e.arg(0), e.arg(1), " >= ");
                    case Z3_OP_LT: return renderCompare(e.arg(0), e.arg(1), " < ");
                    case Z3_OP_GT: return renderCompare(e.arg(0), e.arg(1), " > ");
                    case Z3_OP_XOR: return renderCompare(e.arg(0), e.arg(1), " != ");
                    case Z3_OP_IMPLIES: return renderImplies(e.arg(0), e.arg(1));
                    case Z3_OP_ADD: return foldLeftAssoc(collectArgs(e), Category::AddSub, Category::MulDiv, " + ");
                    case Z3_OP_SUB: return foldLeftAssoc(collectArgs(e), Category::AddSub, Category::MulDiv, " - ");
                    case Z3_OP_MUL: return foldLeftAssoc(collectArgs(e), Category::MulDiv, Category::Not, " * ");
                    case Z3_OP_DIV: return foldLeftAssoc(collectArgs(e), Category::MulDiv, Category::Not, " / ");
                    case Z3_OP_UMINUS: return renderUminus(e.arg(0));
                    case Z3_OP_UNINTERPRETED: return renderVariable(e);
                    default: break;
                }
            }

            unsupported(e, "this construct");
        }
    }

    std::string Z3IfPredGenerator::generate(const Z3Predicate& predicate) {
        return render(predicate.impl().predicate).text;
    }
}
