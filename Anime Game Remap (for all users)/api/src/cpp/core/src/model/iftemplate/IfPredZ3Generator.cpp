#include "AGRemapCore/model/iftemplate/IfPredZ3Generator.h"

#include "tools/z3/Z3Internal.h"

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>


namespace AGRemapCore {

    namespace {

        // 'None' children (eg. an operator token like PLUS/AND/LPAREN that carries no value of its
        // own) are represented the same way the pure-Python IfPredLogicGenerator represents them --
        // as an empty slot -- except here that's std::optional's empty state rather than Python's
        // 'None', since every present value is a real z3::expr (there's no untyped raw int/float
        // stand-in the way sympy's version keeps until the very end; INT/FLOAT literals are already
        // built into real-sorted z3::expr values by the time they leave their own leaf token).
        using NodeValue = std::optional<z3::expr>;
        using ChildrenValues = std::vector<NodeValue>;
        using GenFunc = std::function<NodeValue(const ParseNode<std::string>&, ChildrenValues&, z3::context&)>;
        using GenFuncs = std::unordered_map<std::string, GenFunc>;

        // Coerces a value to Z3's Bool sort -- a no-op if it already is one, otherwise '!= 0'
        // (matching how generateLogicBinOp/the final top-level result in the pure-Python original
        // fold a bare numeric value into a sympy boolean via 'Ne(value, 0)').
        z3::expr coerceToBool(const z3::expr& value, z3::context& ctx) {
            if (value.is_bool()) {
                return value;
            }

            return value != ctx.real_val(0);
        }

        // Coerces a value to Z3's Real sort -- a no-op if it already is one, otherwise the
        // 'ite(value, 1, 0)' arithmetic equivalent of a boolean (matching how generateCompareBinOp
        // in the pure-Python original folds a bool child into 'int(childLogic)' before combining it
        // with an operator that expects a number).
        z3::expr coerceToArith(const z3::expr& value, z3::context& ctx) {
            if (!value.is_bool()) {
                return value;
            }

            return z3::ite(value, ctx.real_val(1), ctx.real_val(0));
        }

        using Coerce = std::function<z3::expr(const z3::expr&, z3::context&)>;
        using BinOp = std::function<z3::expr(const z3::expr&, const z3::expr&)>;

        // Shared fold used for every left-associative n-ary production in the grammar (and/or,
        // eq/ne/gt/ge/lt/le, add/subtract, multiply/divide) -- ports the shape common to the
        // pure-Python original's generateLogicBinOp/generateBinOp/generateCompareBinOp: skip 'None'
        // (here, empty-optional) children -- these are the operator tokens themselves (AND/PLUS/
        // EQ/...) -- take the first real child as-is, then fold the rest in through 'op', coercing
        // every real child to a consistent sort first via 'coerce'.
        NodeValue foldChildren(ChildrenValues& childrenValues, z3::context& ctx, const Coerce& coerce, const BinOp& op) {
            NodeValue result;

            for (NodeValue& child : childrenValues) {
                if (!child.has_value()) {
                    continue;
                }

                z3::expr value = coerce(*child, ctx);
                if (!result.has_value()) {
                    result = std::move(value);
                    continue;
                }

                result = op(*result, value);
            }

            return result;
        }

        // Ported verbatim in shape (same keys as the pure-Python IfPredLogicGenerator's own
        // 'generationFuncs' dict literal) from IfPredLogicGenerator.generate -- see that file for
        // the sympy-flavored original this mirrors production-for-production and token-for-token.
        GenFuncs makeGenerationFuncs() {
            GenFuncs funcs;

            funcs["start"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(1);
            };
            funcs["start empty"] = [](const ParseNode<std::string>&, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.bool_val(false);
            };
            funcs["pred reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["and"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToBool, [](const z3::expr& a, const z3::expr& b) { return a && b; });
            };
            funcs["or"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToBool, [](const z3::expr& a, const z3::expr& b) { return a || b; });
            };
            funcs["test reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["eq"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a == b; });
            };
            funcs["ne"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a != b; });
            };
            funcs["gt"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a > b; });
            };
            funcs["ge"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a >= b; });
            };
            funcs["lt"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a < b; });
            };
            funcs["le"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a <= b; });
            };
            funcs["keyexpr reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["null"] = [](const ParseNode<std::string>&, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.real_val(0);
            };
            funcs["addexpr reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["add"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a + b; });
            };
            funcs["subtract"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a - b; });
            };
            funcs["multexpr reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["multiply"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a * b; });
            };
            funcs["divide"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                return foldChildren(c, ctx, coerceToArith, [](const z3::expr& a, const z3::expr& b) { return a / b; });
            };
            funcs["nterm reduce"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["not"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context& ctx) -> NodeValue {
                const NodeValue& operand = c.at(1);
                if (!operand.has_value()) {
                    return std::nullopt;
                }

                return !coerceToBool(*operand, ctx);
            };
            funcs["variable"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["int"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["float"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(0);
            };
            funcs["bracket loop"] = [](const ParseNode<std::string>&, ChildrenValues& c, z3::context&) -> NodeValue {
                return c.at(1);
            };

            // Leaf tokens -- most carry no value of their own (their text is only meaningful to the
            // production above them, eg. 'AND'/'EQ'/'PLUS'), except NULL/ID/INT/FLOAT, which are
            // where an actual z3::expr first comes into existence.
            //
            // (An array, not a plain {"PLUS", "MINUS", ...} braced-init-list, on purpose -- string
            // literals of different lengths are different array types, so a braced-init-list of
            // them can't deduce a single std::initializer_list<const char*> element type on its own;
            // an array declared with element type 'const char*' forces the per-element decay.)
            static const char* const noValueTokens[] = {"PLUS", "MINUS", "STAR", "SLASH", "LPAREN", "RPAREN",
                                                          "EQ", "NE", "LT", "GT", "LE", "GE",
                                                          "AND", "OR", "NOT", "SPACE", "TAB"};
            for (const char* noValueToken : noValueTokens) {
                funcs[noValueToken] = [](const ParseNode<std::string>&, ChildrenValues&, z3::context&) -> NodeValue {
                    return std::nullopt;
                };
            }

            funcs["NULL"] = [](const ParseNode<std::string>&, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.real_val(0);
            };
            funcs["ID"] = [](const ParseNode<std::string>& node, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.real_const(node.token->val.c_str());
            };
            funcs["INT"] = [](const ParseNode<std::string>& node, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.real_val(node.token->val.c_str());
            };
            funcs["FLOAT"] = [](const ParseNode<std::string>& node, ChildrenValues&, z3::context& ctx) -> NodeValue {
                return ctx.real_val(node.token->val.c_str());
            };

            return funcs;
        }

        enum class WalkState { Explore, Collect };
    }

    Z3Predicate IfPredZ3Generator::generate(const ParseTree<std::string>& parseTree, Z3Context& ctxWrapper, bool simplify) {
        Z3Context::Impl& ctxImpl = ctxWrapper.impl();
        z3::context& ctx = *ctxImpl.ctx;
        static const GenFuncs generationFuncs = makeGenerationFuncs();

        // Same iterative two-state (explore/collect) post-order walk as the pure-Python
        // IfPredLogicGenerator.generate -- see that file's own comments for why the push/pop order
        // below ends up handing each production's generation function its children's values in the
        // same left-to-right order they appear in 'parseTree.children', despite everything going
        // through two LIFO stacks.
        std::vector<std::pair<WalkState, std::string>> nodeIdStack;
        std::vector<NodeValue> valueStack;
        nodeIdStack.emplace_back(WalkState::Explore, parseTree.rootId());

        while (!nodeIdStack.empty()) {
            auto [state, nodeId] = nodeIdStack.back();
            nodeIdStack.pop_back();

            auto childrenIt = parseTree.children.find(nodeId);
            bool hasChildren = childrenIt != parseTree.children.end();

            if (state == WalkState::Explore) {
                nodeIdStack.emplace_back(WalkState::Collect, nodeId);

                if (hasChildren) {
                    for (const std::string& childId : childrenIt->second) {
                        nodeIdStack.emplace_back(WalkState::Explore, childId);
                    }
                }

                continue;
            }

            const ParseNode<std::string>* node = parseTree.getNode(nodeId);
            bool isLeaf = parseTree.isChild(nodeId);

            size_t numChildren = hasChildren ? childrenIt->second.size() : 0;
            ChildrenValues childrenValues(numChildren);
            for (size_t i = 0; i < numChildren; ++i) {
                childrenValues[i] = std::move(valueStack.back());
                valueStack.pop_back();
            }

            std::string key;
            if (isLeaf) {
                key = (node->token.has_value() && node->token->type.has_value()) ? *node->token->type : std::string();
            } else {
                key = node->prodId.value_or(std::string());
            }

            auto funcIt = generationFuncs.find(key);
            NodeValue value = (funcIt != generationFuncs.end()) ? funcIt->second(*node, childrenValues, ctx) : std::nullopt;

            valueStack.push_back(std::move(value));
        }

        NodeValue result = std::move(valueStack.back());
        valueStack.pop_back();

        z3::expr resultExpr = coerceToBool(result.has_value() ? *result : ctx.bool_val(false), ctx);

        if (simplify) {
            resultExpr = resultExpr.simplify();
        }

        // Z3Predicate's real constructor is private (only IfPredZ3Generator is friended, and
        // that friendship doesn't extend to a free helper function even from this same file), so
        // this generate() itself has to be the one place that ever calls it. The shared_ptr
        // passed alongside the expr is what keeps 'ctxWrapper' alive for as long as this
        // Z3Predicate is (see Z3Internal.h's own comment on Z3Context::Impl/Z3Predicate::Impl).
        return Z3Predicate(std::make_unique<Z3Predicate::Impl>(std::move(resultExpr), ctxImpl.ctx));
    }
}
