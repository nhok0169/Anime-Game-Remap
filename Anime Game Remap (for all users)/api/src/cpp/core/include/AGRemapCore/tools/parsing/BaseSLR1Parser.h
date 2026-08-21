#ifndef AGRemapCore_BaseSLR1Parser_H
#define AGRemapCore_BaseSLR1Parser_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/tools/StringHash.h"
#include "AGRemapCore/tools/dfa/BaseDFA.h"
#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"
#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"
#include "AGRemapCore/tools/idGenerator/UuidIdGenerator.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/parsing/ParseTree.h"
#include "AGRemapCore/tools/parsing/SyntaxErr.h"
#include "AGRemapCore/tools/parsing/Token.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The base class used for bottom-up `SLR(1)`_ parsing :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A production rule's id, and the internally-generated state/production-bookkeeping ids, can
        be any ``Hashable`` (the `pybind11`_ binding of this class defaults them to
        ``str(uuid.uuid4())``, matching the id-generation behavior real callers rely on -- see
        :class:`~FixRaidenBoss2.BaseSLR1Parser`). This class models that single "some hashable id" role with one
        template parameter, #Id, reused both as the key type for #Productions and as the state
        type for the internal `DFA`_ -- every real subclass and use of this class only ever gives
        both roles the same concrete type at once, so splitting them into two independent template
        parameters here would add genericity nothing actually uses
     @endrst
     *
     * @tparam Id The type used both for a production rule's id and for a state id of the internal `DFA`_
     * @tparam IdHash The hash function for ``Id``
     * @tparam IdEq The equality function for ``Id``
     */
    template <typename Id, typename IdHash = std::hash<Id>, typename IdEq = std::equal_to<Id>>
    class BaseSLR1Parser {
        public:

            /**
             * @brief
             @rst
             A single production rule, ``A --> B_1, ..., B_m`` :raw-html:`<br />` :raw-html:`<br />`

             Contains:

             #. The non-terminal at the LHS of the production rule (A)
             #. The symbols that the rule produces (B_1, ..., B_m)
             @endrst
             */
            using Production = std::pair<std::string, std::vector<std::string>>;

            /**
             * @brief
             @rst
             The production rules of the `CFG (Context Free Grammer)`_, keyed by the id of each
             production rule :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Deliberately a `tsl::ordered_map`_, not a plain ``std::unordered_map`` -- the
                pure-Python original's ``self._productions`` is a real Python ``dict``, which
                iterates in insertion order, and :cpp:func:`constructDFA` (and its
                :cpp:func:`addImpliedProductions` helper) iterate over every production while
                generating fresh ids as they go. An unordered iteration order there means *which*
                production/state gets discovered (and thus assigned) which generated id becomes a
                traversal-order accident -- still a behaviorally-correct `DFA`_ either way, but not
                a reproducible one across runs/implementations the way the pure-Python original's
                is. Preserving insertion order here removes that specific source of nondeterminism
             @endrst
             */
            using Productions = tsl::ordered_map<Id, Production, IdHash, IdEq>;

            /**
             * @brief
             @rst
             The items of some state of the `DFA`_ used by #constructDFA -- a list of
             ``(itemId, prodInd)`` pairs, where ``prodInd`` is the id of a production rule (a key
             of #Productions) and ``itemId`` is a *separate*, freshly-generated id identifying this
             specific dotted-item occurrence (used to look up that occurrence's own dot/bookmark
             position, independently of any other occurrence of the same production rule)
             @endrst
             */
            using Items = std::vector<std::pair<Id, Id>>;

            /**
             * @brief The states of the `DFA`_ used by #constructDFA, keyed by the id of each state
             */
            using States = std::unordered_map<Id, Items, IdHash, IdEq>;

            /**
             * @brief
             @rst
             The reduce action(s) for some state -- either unconditional (a single production rule
             id, when the state has exactly one item) or dependent on the lookahead token's type (a
             map from lookahead token type to production rule id, when the state has more than one
             item) :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The map alternative is listed first (rather than #Id) purely so this variant's own
                default constructor doesn't require #Id to be default-constructible -- nothing about
                the *meaning* of the two alternatives depends on their order
             @endrst
             */
            using Reduction = std::variant<std::unordered_map<std::string, Id>, Id>;

            /**
             * @brief The reduce actions of the `DFA`_ used by #constructDFA, keyed by the id of each state
             */
            using Reductions = std::unordered_map<Id, Reduction, IdHash, IdEq>;

            /**
             * @brief The parse tree produced by #parse
             */
            using Tree = ParseTree<Id, IdHash, IdEq>;

            /**
             * @brief Constructs a new `SLR(1)`_ parser
             *
             * @param productions The production rules of the `CFG`_, keyed by the id of each production rule
             * @param startSymbol The starting non-terminal symbol
             * @param startToken The name of the starting token for an input string
             * @param endToken The name of the ending token for an input string
             * @param nullToken The name for the empty token
             * @param setup Whether to initialize all the setup for the parser automatically by calling #setup
             * @param stateIdGenerator
             @rst
             The generator used by #constructDFA to generate fresh ids for the states of the
             internal `DFA`_ (the pure-Python original's overridable ``_generateStateId``)
             :raw-html:`<br />` :raw-html:`<br />`

             If ``nullptr``, defaults (lazily, the first time #constructDFA actually needs one) to
             an :cpp:class:`IncIdGenerator` starting at ``Id(1)`` when #Id is an integral type, or a
             :cpp:class:`UuidIdGenerator` (matching the pure-Python original's own default,
             ``str(uuid.uuid4())``) when #Id is ``std::string``. Supply one explicitly for any other
             #Id

             **Default**: ``nullptr``
             @endrst
             * @param itemIdGenerator
             @rst
             The generator used by #constructDFA to generate fresh ids for dotted-item occurrences
             (the pure-Python original's overridable ``_generateProductionId`` -- despite the name,
             it does *not* generate production rule ids; see #Items) :raw-html:`<br />` :raw-html:`<br />`

             Same lazy default behavior as 'stateIdGenerator'

             **Default**: ``nullptr``
             @endrst
             * @param nodeIdGenerator
             @rst
             The generator used by #parse to generate fresh ids for the nodes of the returned
             #Tree (the pure-Python original's overridable ``_generateParserNodeId``)
             :raw-html:`<br />` :raw-html:`<br />`

             Same lazy default behavior as 'stateIdGenerator'

             **Default**: ``nullptr``
             @endrst
             *
             * @throws std::invalid_argument If a terminal symbol (#startToken, #endToken, or #nullToken)
             *      appears on the LHS of a production in 'productions', or if 'startSymbol' is not a
             *      valid non-terminal symbol of 'productions'
             */
            explicit BaseSLR1Parser(Productions productions, std::string startSymbol,
                                     std::string startToken = "STARTTOKEN", std::string endToken = "ENDTOKEN", std::string nullToken = "EPSILON",
                                     bool setup = true,
                                     std::unique_ptr<BaseIdGenerator<Id>> stateIdGenerator = nullptr,
                                     std::unique_ptr<BaseIdGenerator<Id>> itemIdGenerator = nullptr,
                                     std::unique_ptr<BaseIdGenerator<Id>> nodeIdGenerator = nullptr);

            /**
             * @brief
             @rst
             Constructs a new `SLR(1)`_ parser, assuming the id of each production rule is its index
             position within 'productions' :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param productions The production rules of the `CFG`_, in index order
             * @param startSymbol The starting non-terminal symbol
             * @param startToken The name of the starting token for an input string
             * @param endToken The name of the ending token for an input string
             * @param nullToken The name for the empty token
             * @param setup Whether to initialize all the setup for the parser automatically by calling #setup
             * @param stateIdGenerator See the other constructor overload
             * @param itemIdGenerator See the other constructor overload
             * @param nodeIdGenerator See the other constructor overload
             *
             * @throws std::invalid_argument If a terminal symbol (#startToken, #endToken, or #nullToken)
             *      appears on the LHS of a production in 'productions', or if 'startSymbol' is not a
             *      valid non-terminal symbol of 'productions'
             */
            explicit BaseSLR1Parser(std::vector<Production> productions, std::string startSymbol,
                                     std::string startToken = "STARTTOKEN", std::string endToken = "ENDTOKEN", std::string nullToken = "EPSILON",
                                     bool setup = true,
                                     std::unique_ptr<BaseIdGenerator<Id>> stateIdGenerator = nullptr,
                                     std::unique_ptr<BaseIdGenerator<Id>> itemIdGenerator = nullptr,
                                     std::unique_ptr<BaseIdGenerator<Id>> nodeIdGenerator = nullptr);

            virtual ~BaseSLR1Parser() = default;

            /**
             * @brief The name of the starting token for an input string
             */
            std::string startToken;

            /**
             * @brief The name of the ending token for an input string
             */
            std::string endToken;

            /**
             * @brief The name for the empty token
             */
            std::string nullToken;

            /**
             * @brief
             @rst
             The `Nullable Set`_ :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are whether each symbol is nullable
             @endrst
             */
            std::unordered_map<std::string, bool> nullable;

            /**
             * @brief
             @rst
             The `First Set`_ for only each single non-terminal symbol :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are the possible terminal symbols that could
             appear in front of the particular non-terminal symbol
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<std::string>> first;

            /**
             * @brief
             @rst
             The `Follow Set`_ :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are the possible terminal symbols that could
             appear after the particular non-terminal symbol
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<std::string>> follow;

            /**
             * @brief
             @rst
             The production rules of the `CFG (Context Free Grammer)`_
             @endrst
             *
             * @return The production rules, keyed by the id of each production rule
             */
            const Productions& productions() const;

            /**
             * @brief Sets the new production rules
             *
             * @param newProductions The new production rules, keyed by the id of each production rule
             *
             * @throws std::invalid_argument If a terminal symbol (#startToken, #endToken, or #nullToken)
             *      appears on the LHS of a production in 'newProductions'
             */
            void setProductions(Productions newProductions);

            /**
             * @brief
             @rst
             Sets the new production rules, assuming the id of each production rule is its index
             position within 'newProductions'
             @endrst
             *
             * @param newProductions The new production rules, in index order
             *
             * @throws std::invalid_argument If a terminal symbol (#startToken, #endToken, or #nullToken)
             *      appears on the LHS of a production in 'newProductions'
             */
            void setProductions(std::vector<Production> newProductions);

            /**
             * @brief The set of non-terminal symbols of the `CFG`_, as of the last time #productions was set
             */
            const std::unordered_set<std::string>& nonTermSymbols() const;

            /**
             * @brief The starting non-terminal symbol
             */
            const std::string& startSymbol() const;

            /**
             * @brief Sets the new starting non-terminal symbol
             *
             * @param newStartSymbol The new starting non-terminal symbol
             *
             * @throws std::invalid_argument If 'newStartSymbol' is not a valid non-terminal symbol of #productions
             */
            void setStartSymbol(std::string newStartSymbol);

            /**
             * @brief Clears all the setup from the parser
             */
            void clear();

            /**
             * @brief Retrieves the set of non-terminal symbols of the `CFG`_
             *
             * @return The set of non-terminal symbols
             */
            std::unordered_set<std::string> getNonTermSymbols() const;

            /**
             * @brief
             @rst
             Computes the `Nullable Set`_
             @endrst
             *
             * @return
             @rst
             Whether each non-terminal symbol is nullable :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are whether each symbol is nullable
             @endrst
             */
            std::unordered_map<std::string, bool> getNullableSet() const;

            /**
             * @brief
             @rst
             Computes the `First Set`_ for only each single non-terminal symbol
             @endrst
             *
             * @param updateNullable Whether to update #nullable using #getNullableSet
             *
             * @return
             @rst
             The first terminal symbols to appear for a non-terminal symbol :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are the possible terminal symbols that could
             appear first for the particular non-terminal symbol
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<std::string>> getFirstSet(bool updateNullable = true);

            /**
             * @brief Retrieves the first terminal symbols to appear given a list of symbols
             *
             * @param symbols The symbols to read
             * @param nullableSet
             @rst
             The `Nullable Set`_ :raw-html:`<br />` :raw-html:`<br />`

             The keys are the non-terminal symbols and the values are whether each symbol is nullable
             @endrst
             * @param firstSet
             @rst
             The `First Set`_ for only each single non-terminal symbol, possibly still under
             construction (e.g. the in-progress accumulator passed in by #getFirstSet) --
             a non-terminal symbol with no entry yet is treated the same as one mapped to an
             empty set :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @return The first terminal symbols to appear given 'symbols'
             */
            std::unordered_set<std::string> getFirst(const std::vector<std::string>& symbols,
                                                      const std::unordered_map<std::string, bool>& nullableSet,
                                                      const std::unordered_map<std::string, std::unordered_set<std::string>>& firstSet) const;

            /**
             * @brief
             @rst
             Computes the `Follow Set`_
             @endrst
             *
             * @param updateNullable Whether to update #nullable using #getNullableSet
             * @param updateFirst Whether to update #first using #getFirstSet
             *
             * @return
             @rst
             The keys are the non-terminal symbols and the values are the possible terminal symbols that could
             appear after the particular non-terminal symbol
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<std::string>> getFollowSet(bool updateNullable = true, bool updateFirst = true);

            /**
             * @brief
             @rst
             Constructs the `DFA`_ to determine whether to shift/reduce when reading the input
             @endrst
             *
             * @param updateNullable Whether to update #nullable using #getNullableSet
             * @param updateFirst Whether to update #first using #getFirstSet
             * @param updateFollow Whether to update #follow using #getFollowSet
             *
             * @return The states of the constructed `DFA`_, keyed by the id of each state (also
             *      available afterward, alongside the reduce actions, via #dfa / #reductions)
             */
            States constructDFA(bool updateNullable = true, bool updateFirst = true, bool updateFollow = true);

            /**
             * @brief Initializes all the necessary setup for the parser (#nullable, #first, #follow,
             *      and the internal `DFA`_ / #reductions, in that order)
             */
            void setup();

            /**
             * @brief The internal `DFA`_ used to determine whether to shift/reduce when reading the input
             */
            const BaseDFA<Id, std::string, IdEq, IdHash, std::equal_to<std::string>, StringViewHash>& dfa() const;

            /**
             * @brief
             @rst
             The reduce actions of the `DFA`_, keyed by the id of each state -- populated by
             #constructDFA :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike #nullable/#first/#follow, #clear does **not** reset this -- matching the
                pure-Python original's own ``clear``, which never touches ``_reductions`` either.
                Harmless in practice since #constructDFA always overwrites it wholesale
             @endrst
             */
            const Reductions& reductions() const;

            /**
             * @brief
             @rst
             Parses a sequence of already-tokenized input tokens :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The pure-Python original also accepts a plain ``str`` directly, tokenizing it by
                treating each individual character as its own token (via ``str.splitlines(keepends
                = True)`` for correct per-character line/char numbering). Not ported here: every
                real call site in this codebase always already has a real ``List[Token]`` in hand
                (from a tokenizer's own ``simplifiedMaximalMunch``) before calling ``parse``, so the
                raw-``str`` path is pure convenience with zero real callers -- and faithfully
                replicating it would mean reimplementing Python's ``str.splitlines(keepends=True)``
                line-boundary rules (which recognize several characters beyond ``\\n``/``\\r\\n``),
                not something :cpp:func:`AGRemapCore::StringTools::splitlines` (keepends=False only)
                already covers. Add it if/when a real caller needs it
             @endrst
             *
             * @param tokens The tokenized tokens of the input text
             * @param ctx
             @rst
             The context for parsing :raw-html:`<br />` :raw-html:`<br />`

             If ``nullptr``, a context is constructed from the concatenation of every token's value
             @endrst
             *
             * @throws AGRemapCore::SyntaxErr If the parse tree cannot be constructed
             *
             * @return The constructed parse tree
             */
            Tree parse(std::vector<Token> tokens, const ParseContext* ctx = nullptr);

        private:

            /**
             * @brief The internal `DFA`_ used to determine whether to shift/reduce when reading the input
             */
            BaseDFA<Id, std::string, IdEq, IdHash, std::equal_to<std::string>, StringViewHash> dfa_;

            Productions productions_;
            std::unordered_set<std::string> nonTermSymbols_;
            std::string startSymbol_;
            Reductions reductions_;

            /**
             * @brief See the constructor's 'stateIdGenerator' parameter
             */
            std::unique_ptr<BaseIdGenerator<Id>> stateIdGenerator_;

            /**
             * @brief See the constructor's 'itemIdGenerator' parameter
             */
            std::unique_ptr<BaseIdGenerator<Id>> itemIdGenerator_;

            /**
             * @brief See the constructor's 'nodeIdGenerator' parameter
             */
            std::unique_ptr<BaseIdGenerator<Id>> nodeIdGenerator_;

            /**
             * @brief A ``(production rule id, dot/bookmark position)`` pair -- identifies a specific
             *      dotted item shape, independently of which occurrence(s) of it exist
             */
            struct ProdBookmarkKey {
                Id prodInd;
                std::size_t bookmark;

                // Needed directly (not just via ProdBookmarkEq) -- MSVC's STL implementation of
                // std::unordered_set::operator==(a, b), used below to compare two whole item-sets
                // for equality, falls back to the key type's own operator== rather than the
                // container's Keyeq functor.
                bool operator==(const ProdBookmarkKey& other) const;
            };

            struct ProdBookmarkHash {
                std::size_t operator()(const ProdBookmarkKey& key) const;
            };

            struct ProdBookmarkEq {
                bool operator()(const ProdBookmarkKey& a, const ProdBookmarkKey& b) const;
            };

            using StatesByProds = std::unordered_map<ProdBookmarkKey, std::unordered_set<Id, IdHash, IdEq>, ProdBookmarkHash, ProdBookmarkEq>;

            /**
             * @brief Shared validation for #setProductions -- throws if a terminal symbol appears on
             *      the LHS of some production
             */
            void validateProductions(const Productions& newProductions) const;

            /**
             * @brief Lazily constructs #stateIdGenerator_/#itemIdGenerator_ if either was never supplied
             *
             * @throws std::logic_error If a generator is missing and ``Id`` isn't an integral type eligible
             *      for the built-in default (see the constructor's 'stateIdGenerator' parameter)
             */
            void ensureIdGenerators();

            static std::unique_ptr<BaseIdGenerator<Id>> makeDefaultIdGenerator();

            Id generateStateId();
            Id generateItemId();

            /**
             * @brief
             @rst
             Computes the `LR(0) closure`_ of a set of dotted items in place -- for every item whose
             symbol-after-the-dot is a non-terminal, adds a fresh dot-at-0 item for every production
             rule of that non-terminal not already present in 'items' -- the C++ counterpart to the
             pure-Python original's own ``_addImpliedProductions`` :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param items The dotted items to compute the closure of, grown in place
             * @param bookmarks The dot/bookmark position of every item occurrence seen so far, keyed by itemId
             * @param statesByProds Reverse index from a dotted-item shape to the states already known to
             *      contain it -- only updated for the newly-added closure items when 'stateId' is given
             * @param stateId
             @rst
             The id of the state 'items' belongs to, if already known (only true for the very first,
             starting state -- see the class-level note on ``_addImpliedProductions`` in the pure-Python
             original for why every other call site leaves this ``std::nullopt`` and updates
             'statesByProds' separately, once the state's own final id is actually decided)
             @endrst
             */
            void addImpliedProductions(Items& items, std::unordered_map<Id, std::size_t, IdHash, IdEq>& bookmarks,
                                        StatesByProds& statesByProds, const Id* stateId = nullptr);

            Id generateNodeId();

            /**
             * @brief
             @rst
             Determines whether some state has a reduce action available for a given lookahead
             token type -- the C++ counterpart to the pure-Python original's own classmethod
             ``_hasReduction``
             @endrst
             *
             * @param currentIsAccept Whether the current state is an accepting state
             * @param reduction The current state's reduce action, or ``nullptr`` if it has none
             * @param tokenType The type of the lookahead token
             */
            static bool hasReduction(bool currentIsAccept, const Reduction* reduction, const std::optional<std::string>& tokenType);

            /**
             * @brief Resolves the production rule id a reduce action reduces to for a given lookahead
             *      token type -- only valid to call once #hasReduction has confirmed one exists
             */
            static const Id& resolveProdInd(const Reduction& reduction, const std::optional<std::string>& tokenType);

            /**
             * @brief Raises a syntax error for some unexpected token
             *
             * @throws AGRemapCore::SyntaxErr Always
             */
            [[noreturn]] void raiseSyntaxErr(const ParseContext& ctx, const Token& token) const;
    };
}

#include "BaseSLR1Parser.tpp"

#endif
