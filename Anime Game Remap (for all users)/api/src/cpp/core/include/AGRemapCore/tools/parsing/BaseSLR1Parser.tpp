namespace AGRemapCore {

    template <typename Id, typename IdHash, typename IdEq>
    BaseSLR1Parser<Id, IdHash, IdEq>::BaseSLR1Parser(Productions productions, std::string startSymbol,
                                                      std::string startToken, std::string endToken, std::string nullToken,
                                                      bool setup,
                                                      std::unique_ptr<BaseIdGenerator<Id>> stateIdGenerator,
                                                      std::unique_ptr<BaseIdGenerator<Id>> itemIdGenerator,
                                                      std::unique_ptr<BaseIdGenerator<Id>> nodeIdGenerator):
        startToken(std::move(startToken)), endToken(std::move(endToken)), nullToken(std::move(nullToken)),
        stateIdGenerator_(std::move(stateIdGenerator)), itemIdGenerator_(std::move(itemIdGenerator)), nodeIdGenerator_(std::move(nodeIdGenerator))
    {
        setProductions(std::move(productions));
        setStartSymbol(std::move(startSymbol));

        if (setup) {
            this->setup();
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    BaseSLR1Parser<Id, IdHash, IdEq>::BaseSLR1Parser(std::vector<Production> productions, std::string startSymbol,
                                                      std::string startToken, std::string endToken, std::string nullToken,
                                                      bool setup,
                                                      std::unique_ptr<BaseIdGenerator<Id>> stateIdGenerator,
                                                      std::unique_ptr<BaseIdGenerator<Id>> itemIdGenerator,
                                                      std::unique_ptr<BaseIdGenerator<Id>> nodeIdGenerator):
        startToken(std::move(startToken)), endToken(std::move(endToken)), nullToken(std::move(nullToken)),
        stateIdGenerator_(std::move(stateIdGenerator)), itemIdGenerator_(std::move(itemIdGenerator)), nodeIdGenerator_(std::move(nodeIdGenerator))
    {
        setProductions(std::move(productions));
        setStartSymbol(std::move(startSymbol));

        if (setup) {
            this->setup();
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::validateProductions(const Productions& newProductions) const {
        std::unordered_set<std::string> terminalSymbols = {startToken, endToken, nullToken};

        for (const auto& [prodId, production] : newProductions) {
            const std::string& prodKey = production.first;
            if (terminalSymbols.find(prodKey) != terminalSymbols.end()) {
                throw std::invalid_argument("BaseSLR1Parser: " + prodKey + " cannot appear on the LHS of a production since it is a terminal symbol");
            }
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::setProductions(Productions newProductions) {
        validateProductions(newProductions);

        productions_ = std::move(newProductions);
        nonTermSymbols_ = getNonTermSymbols();
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::setProductions(std::vector<Production> newProductions) {
        Productions indexedProductions;
        indexedProductions.reserve(newProductions.size());

        for (std::size_t i = 0; i < newProductions.size(); ++i) {
            indexedProductions.emplace(Id(i), std::move(newProductions[i]));
        }

        setProductions(std::move(indexedProductions));
    }

    template <typename Id, typename IdHash, typename IdEq>
    const typename BaseSLR1Parser<Id, IdHash, IdEq>::Productions& BaseSLR1Parser<Id, IdHash, IdEq>::productions() const {
        return productions_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    const std::unordered_set<std::string>& BaseSLR1Parser<Id, IdHash, IdEq>::nonTermSymbols() const {
        return nonTermSymbols_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    const std::string& BaseSLR1Parser<Id, IdHash, IdEq>::startSymbol() const {
        return startSymbol_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::setStartSymbol(std::string newStartSymbol) {
        if (nonTermSymbols_.find(newStartSymbol) == nonTermSymbols_.end()) {
            throw std::invalid_argument("BaseSLR1Parser: the start symbol, " + newStartSymbol + ", is not a valid non-terminal symbol");
        }

        startSymbol_ = std::move(newStartSymbol);
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::clear() {
        nullable.clear();
        first.clear();
        follow.clear();
        dfa_.clear();
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unordered_set<std::string> BaseSLR1Parser<Id, IdHash, IdEq>::getNonTermSymbols() const {
        std::unordered_set<std::string> result;

        for (const auto& [prodId, production] : productions_) {
            result.insert(production.first);
        }

        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unordered_map<std::string, bool> BaseSLR1Parser<Id, IdHash, IdEq>::getNullableSet() const {
        std::unordered_map<std::string, bool> result;
        for (const std::string& symbol : nonTermSymbols_) {
            result[symbol] = false;
        }

        bool hasChange = true;
        while (hasChange) {
            hasChange = false;

            for (const auto& [prodId, production] : productions_) {
                const std::string& prodKey = production.first;
                const std::vector<std::string>& prodVals = production.second;

                if (result[prodKey]) {
                    continue;
                }

                bool allNullable = true;
                for (const std::string& val : prodVals) {
                    bool valIsNullableNonTerm = (nonTermSymbols_.find(val) != nonTermSymbols_.end()) && result[val];
                    if (val != nullToken && !valIsNullableNonTerm) {
                        allNullable = false;
                        break;
                    }
                }

                if (!allNullable) {
                    continue;
                }

                bool prevNullable = result[prodKey];
                result[prodKey] = true;

                if (!hasChange && !prevNullable) {
                    hasChange = true;
                }
            }
        }

        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unordered_set<std::string> BaseSLR1Parser<Id, IdHash, IdEq>::getFirst(const std::vector<std::string>& symbols,
                                                                                const std::unordered_map<std::string, bool>& nullableSet,
                                                                                const std::unordered_map<std::string, std::unordered_set<std::string>>& firstSet) const {
        std::unordered_set<std::string> result;

        for (const std::string& symbol : symbols) {
            if (symbol == nullToken) {
                continue;
            }

            if (nonTermSymbols_.find(symbol) == nonTermSymbols_.end()) {
                result.insert(symbol);
                break;
            }

            auto firstIt = firstSet.find(symbol);
            if (firstIt != firstSet.end()) {
                result.insert(firstIt->second.begin(), firstIt->second.end());
            }

            auto nullableIt = nullableSet.find(symbol);
            bool symbolIsNullable = (nullableIt != nullableSet.end()) && nullableIt->second;
            if (!symbolIsNullable) {
                break;
            }
        }

        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unordered_map<std::string, std::unordered_set<std::string>> BaseSLR1Parser<Id, IdHash, IdEq>::getFirstSet(bool updateNullable) {
        if (updateNullable) {
            nullable = getNullableSet();
        }

        std::unordered_map<std::string, std::unordered_set<std::string>> result;
        bool hasChange = true;

        while (hasChange) {
            hasChange = false;

            for (const auto& [prodId, production] : productions_) {
                const std::string& prodKey = production.first;
                const std::vector<std::string>& prodVals = production.second;

                std::unordered_set<std::string>& prodKeyFirst = result[prodKey];
                std::size_t prevFirstLen = prodKeyFirst.size();

                std::unordered_set<std::string> newFirst = getFirst(prodVals, nullable, result);
                prodKeyFirst.insert(newFirst.begin(), newFirst.end());

                if (!hasChange && prevFirstLen != prodKeyFirst.size()) {
                    hasChange = true;
                }
            }
        }

        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unordered_map<std::string, std::unordered_set<std::string>> BaseSLR1Parser<Id, IdHash, IdEq>::getFollowSet(bool updateNullable, bool updateFirst) {
        if (updateNullable) {
            nullable = getNullableSet();
        }

        if (updateFirst) {
            first = getFirstSet(false);
        }

        // Combining hash for a RHS suffix (the memoization key below) -- same combine idiom as
        // ModDictAssets::KeyVecHash. Local to this function since nothing outside it needs the type.
        struct SuffixHash {
            std::size_t operator()(const std::vector<std::string>& suffix) const {
                std::size_t seed = 0;
                std::hash<std::string> hasher;
                for (const std::string& s : suffix) {
                    seed ^= hasher(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                }
                return seed;
            }
        };

        // Memoizes getFirst(suffix, ...) per distinct RHS suffix seen -- safe to cache across the
        // whole fixpoint loop below since 'nullable'/'first' are already fixed by this point.
        std::unordered_map<std::vector<std::string>, std::unordered_set<std::string>, SuffixHash> firstVals;

        std::unordered_map<std::string, std::unordered_set<std::string>> result;
        bool hasChange = true;

        while (hasChange) {
            hasChange = false;

            for (const auto& [prodId, production] : productions_) {
                const std::string& prodKey = production.first;
                const std::vector<std::string>& prodVals = production.second;
                std::size_t prodValsLen = prodVals.size();

                for (std::size_t i = 0; i < prodValsLen; ++i) {
                    const std::string& val = prodVals[i];
                    if (nonTermSymbols_.find(val) == nonTermSymbols_.end()) {
                        continue;
                    }

                    std::unordered_set<std::string>& valFollow = result[val];
                    std::size_t prevFollowLen = valFollow.size();

                    std::vector<std::string> suffix(prodVals.begin() + i + 1, prodVals.end());
                    auto firstValsIt = firstVals.find(suffix);
                    if (firstValsIt == firstVals.end()) {
                        firstValsIt = firstVals.emplace(suffix, getFirst(suffix, nullable, first)).first;
                    }

                    valFollow.insert(firstValsIt->second.begin(), firstValsIt->second.end());

                    bool allNullable = true;
                    for (std::size_t j = i + 1; j < prodValsLen; ++j) {
                        const std::string& currentVal = prodVals[j];
                        bool currentValIsNullableNonTerm = (nonTermSymbols_.find(currentVal) != nonTermSymbols_.end())
                                                            && nullable[currentVal];
                        if (currentVal != nullToken && !currentValIsNullableNonTerm) {
                            allNullable = false;
                            break;
                        }
                    }

                    if (allNullable) {
                        // 'val' and 'prodKey' can be the exact same symbol (e.g. left/right-recursive
                        // productions like "S -> S R S"), which would make this the same map entry as
                        // 'valFollow' -- copy out first rather than inserting a range that may alias
                        // its own destination (unordered_set::insert(first, last) is undefined behavior
                        // if [first, last) refers into the container being inserted into).
                        std::unordered_set<std::string> prodKeyFollowCopy = result[prodKey];
                        valFollow.insert(prodKeyFollowCopy.begin(), prodKeyFollowCopy.end());
                    }

                    if (!hasChange && valFollow.size() != prevFollowLen) {
                        hasChange = true;
                    }
                }
            }
        }

        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    bool BaseSLR1Parser<Id, IdHash, IdEq>::ProdBookmarkKey::operator==(const ProdBookmarkKey& other) const {
        IdEq idEq;
        return bookmark == other.bookmark && idEq(prodInd, other.prodInd);
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::size_t BaseSLR1Parser<Id, IdHash, IdEq>::ProdBookmarkHash::operator()(const ProdBookmarkKey& key) const {
        IdHash idHasher;
        std::hash<std::size_t> szHasher;
        std::size_t seed = idHasher(key.prodInd);
        seed ^= szHasher(key.bookmark) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    template <typename Id, typename IdHash, typename IdEq>
    bool BaseSLR1Parser<Id, IdHash, IdEq>::ProdBookmarkEq::operator()(const ProdBookmarkKey& a, const ProdBookmarkKey& b) const {
        IdEq idEq;
        return a.bookmark == b.bookmark && idEq(a.prodInd, b.prodInd);
    }

    template <typename Id, typename IdHash, typename IdEq>
    std::unique_ptr<BaseIdGenerator<Id>> BaseSLR1Parser<Id, IdHash, IdEq>::makeDefaultIdGenerator() {
        if constexpr (std::is_integral_v<Id>) {
            return std::make_unique<IncIdGenerator<Id>>(Id(1));
        } else if constexpr (std::is_same_v<Id, std::string>) {
            // matches the pure-Python original's own default (str(uuid.uuid4()))
            return std::make_unique<UuidIdGenerator>();
        } else {
            throw std::logic_error("BaseSLR1Parser: no id generator was supplied, and Id is neither an integral type "
                                    "(eligible for the built-in IncIdGenerator default) nor std::string (eligible for the "
                                    "built-in UuidIdGenerator default) -- supply stateIdGenerator/itemIdGenerator/"
                                    "nodeIdGenerator explicitly to the constructor");
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::ensureIdGenerators() {
        if (!stateIdGenerator_) {
            stateIdGenerator_ = makeDefaultIdGenerator();
        }
        if (!itemIdGenerator_) {
            itemIdGenerator_ = makeDefaultIdGenerator();
        }
        if (!nodeIdGenerator_) {
            nodeIdGenerator_ = makeDefaultIdGenerator();
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    Id BaseSLR1Parser<Id, IdHash, IdEq>::generateStateId() {
        Id result{};
        stateIdGenerator_->getId(result);
        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    Id BaseSLR1Parser<Id, IdHash, IdEq>::generateItemId() {
        Id result{};
        itemIdGenerator_->getId(result);
        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    Id BaseSLR1Parser<Id, IdHash, IdEq>::generateNodeId() {
        Id result{};
        nodeIdGenerator_->getId(result);
        return result;
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::addImpliedProductions(Items& items, std::unordered_map<Id, std::size_t, IdHash, IdEq>& bookmarks,
                                                                  StatesByProds& statesByProds, const Id* stateId) {
        IdEq idEq;
        std::size_t currentItemsLen = items.size();

        // Seeds from 'items' own current (prodInd, bookmark) pairs -- NOT necessarily bookmark 0,
        // since some of these may already be partway through their RHS (e.g. items carried over
        // from a shift, not just freshly-closed ones).
        std::unordered_set<ProdBookmarkKey, ProdBookmarkHash, ProdBookmarkEq> uniqueItems;
        for (const auto& [itemId, prodInd] : items) {
            uniqueItems.insert(ProdBookmarkKey{prodInd, bookmarks[itemId]});
        }

        std::size_t i = 0;
        while (i < currentItemsLen) {
            // Copies, not references: 'items' can grow via emplace_back below (a new item
            // discovered by this same closure pass), which may reallocate its backing storage and
            // invalidate any reference taken into it beforehand -- itemId/prodInd need to survive
            // across that emplace_back, since the inner for loop keeps reading them afterward (for
            // every remaining production once the first new item is appended). This mirrors Python's
            // own semantics for free -- `prodId, prodInd = prodInds[i]` there always copies out a
            // reference-counted value, unaffected by a later `prodInds.append(...)` -- but needs to
            // be explicit in C++. Confirmed as a real bug, not just a theoretical one: reading a
            // dangling py::object reference this way reliably crashed the pybind11-bound
            // instantiation (BaseSLR1Parser<py::object, ...>) with a CPython-internal SystemError,
            // for any grammar whose closure needed more than one round of expansion.
            const Id itemId = items[i].first;
            const Id prodInd = items[i].second;
            const Production& production = productions_.at(prodInd);
            const std::vector<std::string>& prodVals = production.second;
            std::size_t bookmark = bookmarks[itemId];

            if (bookmark >= prodVals.size()) {
                ++i;
                continue;
            }

            const std::string& currentChar = prodVals[bookmark];
            if (nonTermSymbols_.find(currentChar) == nonTermSymbols_.end()) {
                ++i;
                continue;
            }

            for (const auto& [newProdInd, newProduction] : productions_) {
                const std::string& newProdKey = newProduction.first;

                // Ported verbatim from the pure-Python original's own
                // "(newProdInd != prodInd or bookmark != 0)" guard -- kept as a literal, separately
                // evaluated condition (not folded away as provably redundant with the uniqueItems
                // check below) since this item-closure bookkeeping is easy to get subtly wrong;
                // safer to preserve the original's exact logic than to trust an on-paper simplification.
                bool notSelfAtBookmarkZero = !idEq(newProdInd, prodInd) || bookmark != 0;

                if (newProdKey == currentChar && notSelfAtBookmarkZero && uniqueItems.find(ProdBookmarkKey{newProdInd, 0}) == uniqueItems.end()) {
                    Id newItemId = generateItemId();
                    items.emplace_back(newItemId, newProdInd);
                    uniqueItems.insert(ProdBookmarkKey{newProdInd, 0});

                    std::size_t newBookmark = bookmarks[newItemId];

                    if (stateId != nullptr) {
                        statesByProds[ProdBookmarkKey{newProdInd, newBookmark}].insert(*stateId);
                    }

                    ++currentItemsLen;
                }
            }

            ++i;
        }
    }

    template <typename Id, typename IdHash, typename IdEq>
    typename BaseSLR1Parser<Id, IdHash, IdEq>::States BaseSLR1Parser<Id, IdHash, IdEq>::constructDFA(bool updateNullable, bool updateFirst, bool updateFollow) {
        if (updateNullable) {
            nullable = getNullableSet();
        }
        if (updateFirst) {
            first = getFirstSet(false);
        }
        if (updateFollow) {
            follow = getFollowSet(false, false);
        }

        ensureIdGenerators();

        dfa_.clear();
        Id currentStateId = generateStateId();

        StatesByProds statesByProds;
        std::unordered_map<Id, std::size_t, IdHash, IdEq> bookmarks;
        Reductions reductions;
        States states;

        // get the starting items: dot-0 items for every production of the start symbol
        Items startItems;
        for (const auto& [prodInd, production] : productions_) {
            if (production.first == startSymbol_) {
                startItems.emplace_back(generateItemId(), prodInd);
            }
        }

        states.emplace(currentStateId, std::move(startItems));
        addImpliedProductions(states.at(currentStateId), bookmarks, statesByProds, &currentStateId);
        dfa_.addState(currentStateId, std::nullopt, true);

        std::vector<std::pair<Id, Items>> stack;
        stack.emplace_back(currentStateId, states.at(currentStateId));

        while (!stack.empty()) {
            auto [stateId, items] = std::move(stack.back());
            stack.pop_back();

            std::size_t itemsLen = items.size();

            // tsl::ordered_map, not std::unordered_map, for the same reason #Productions is one --
            // see its class-level note. Iterating 'neighbours' in the order its transition symbols
            // were first discovered (matching the pure-Python original's dict-based
            // defaultdict(list)) removes another traversal-order source of nondeterminism in which
            // neighbour state gets which generated id.
            tsl::ordered_map<std::string, Items> neighbours;

            // shift (or mark for reduction) every item in this state
            for (const auto& [itemId, prodInd] : items) {
                const Production& production = productions_.at(prodInd);
                const std::string& prodKey = production.first;
                const std::vector<std::string>& prodVals = production.second;
                std::size_t bookmark = bookmarks[itemId];
                std::size_t prodValsLen = prodVals.size();

                bool toReduce = true;
                while (bookmark < prodValsLen) {
                    const std::string& currentChar = prodVals[bookmark];

                    if (currentChar != nullToken) {
                        Id neighbourItemId = generateItemId();
                        neighbours[currentChar].emplace_back(neighbourItemId, prodInd);
                        bookmarks[neighbourItemId] = bookmark + 1;
                        toReduce = false;
                        break;
                    }

                    ++bookmark;
                }

                if (!toReduce) {
                    continue;
                }

                // reduction
                if (itemsLen == 1) {
                    reductions[stateId] = Reduction(prodInd);
                } else {
                    auto followIt = follow.find(prodKey);
                    if (followIt != follow.end() && !followIt->second.empty()) {
                        Reduction& red = reductions[stateId];
                        if (!std::holds_alternative<std::unordered_map<std::string, Id>>(red)) {
                            red = std::unordered_map<std::string, Id>{};
                        }

                        auto& reduceMap = std::get<std::unordered_map<std::string, Id>>(red);
                        for (const std::string& followSymbol : followIt->second) {
                            reduceMap[followSymbol] = prodInd;
                        }
                    }
                }

                dfa_.addState(stateId, true, false);
            }

            // add the neighbours
            // (tsl::ordered_map's iterator always dereferences to a const pair -- even for a
            // non-const iterator, both key AND value -- so a genuinely mutable Items& has to come
            // from at(), not from the range-for binding itself)
            for (const auto& [transitionSymbol, unusedNeighbourItems] : neighbours) {
                (void)unusedNeighbourItems;
                Items& neighbourItems = neighbours.at(transitionSymbol);
                addImpliedProductions(neighbourItems, bookmarks, statesByProds);

                // check if a neighbour with this exact item set already exists
                std::optional<std::unordered_set<Id, IdHash, IdEq>> existingNodeIds;
                for (const auto& [itemId, prodInd] : neighbourItems) {
                    ProdBookmarkKey key{prodInd, bookmarks[itemId]};
                    auto statesByProdsIt = statesByProds.find(key);
                    if (statesByProdsIt == statesByProds.end()) {
                        continue;
                    }

                    if (!existingNodeIds.has_value()) {
                        existingNodeIds = statesByProdsIt->second;
                    } else {
                        std::unordered_set<Id, IdHash, IdEq> intersection;
                        for (const Id& candidateId : *existingNodeIds) {
                            if (statesByProdsIt->second.find(candidateId) != statesByProdsIt->second.end()) {
                                intersection.insert(candidateId);
                            }
                        }
                        existingNodeIds = std::move(intersection);
                    }
                }

                bool neighbourExists = false;
                if (existingNodeIds.has_value() && !existingNodeIds->empty()) {
                    for (const Id& neighbourId : *existingNodeIds) {
                        const Items& existingItems = states.at(neighbourId);
                        if (existingItems.size() != neighbourItems.size()) {
                            continue;
                        }

                        std::unordered_set<ProdBookmarkKey, ProdBookmarkHash, ProdBookmarkEq> existingBookmarkIds;
                        for (const auto& [existingItemId, existingProdInd] : existingItems) {
                            existingBookmarkIds.insert(ProdBookmarkKey{existingProdInd, bookmarks[existingItemId]});
                        }

                        std::unordered_set<ProdBookmarkKey, ProdBookmarkHash, ProdBookmarkEq> neighbourBookmarkIds;
                        for (const auto& [neighbourItemId, neighbourProdInd] : neighbourItems) {
                            neighbourBookmarkIds.insert(ProdBookmarkKey{neighbourProdInd, bookmarks[neighbourItemId]});
                        }

                        if (existingBookmarkIds != neighbourBookmarkIds) {
                            continue;
                        }

                        dfa_.addKeywordTransition(stateId, transitionSymbol, neighbourId);
                        neighbourExists = true;
                        break;
                    }
                }

                if (neighbourExists) {
                    continue;
                }

                // add the neighbour
                Id neighbourId = generateStateId();
                dfa_.addKeywordTransition(stateId, transitionSymbol, neighbourId);
                states.emplace(neighbourId, neighbourItems);
                stack.emplace_back(neighbourId, states.at(neighbourId));

                for (const auto& [itemId, prodInd] : neighbourItems) {
                    statesByProds[ProdBookmarkKey{prodInd, bookmarks[itemId]}].insert(neighbourId);
                }
            }
        }

        reductions_ = std::move(reductions);
        return states;
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::setup() {
        clear();
        nullable = getNullableSet();
        first = getFirstSet(false);
        follow = getFollowSet(false, false);
        constructDFA(false, false, false);
    }

    template <typename Id, typename IdHash, typename IdEq>
    const BaseDFA<Id, std::string, IdEq, IdHash, std::equal_to<std::string>, StringViewHash>& BaseSLR1Parser<Id, IdHash, IdEq>::dfa() const {
        return dfa_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    const typename BaseSLR1Parser<Id, IdHash, IdEq>::Reductions& BaseSLR1Parser<Id, IdHash, IdEq>::reductions() const {
        return reductions_;
    }

    template <typename Id, typename IdHash, typename IdEq>
    bool BaseSLR1Parser<Id, IdHash, IdEq>::hasReduction(bool currentIsAccept, const Reduction* reduction, const std::optional<std::string>& tokenType) {
        if (!currentIsAccept || reduction == nullptr) {
            return false;
        }

        const auto* reduceMap = std::get_if<std::unordered_map<std::string, Id>>(reduction);
        if (reduceMap == nullptr) {
            // unconditional (the Id alternative) -- a reduce action always available here
            return true;
        }

        return tokenType.has_value() && reduceMap->find(*tokenType) != reduceMap->end();
    }

    template <typename Id, typename IdHash, typename IdEq>
    const Id& BaseSLR1Parser<Id, IdHash, IdEq>::resolveProdInd(const Reduction& reduction, const std::optional<std::string>& tokenType) {
        const auto* reduceMap = std::get_if<std::unordered_map<std::string, Id>>(&reduction);
        if (reduceMap == nullptr) {
            return std::get<Id>(reduction);
        }

        // only ever called after hasReduction confirmed 'tokenType' has a value and is a key here
        return reduceMap->at(*tokenType);
    }

    template <typename Id, typename IdHash, typename IdEq>
    void BaseSLR1Parser<Id, IdHash, IdEq>::raiseSyntaxErr(const ParseContext& ctx, const Token& token) const {
        throw SyntaxErr(ctx, token, "parsing");
    }

    template <typename Id, typename IdHash, typename IdEq>
    typename BaseSLR1Parser<Id, IdHash, IdEq>::Tree BaseSLR1Parser<Id, IdHash, IdEq>::parse(std::vector<Token> tokens, const ParseContext* ctx) {
        // create a default context if none was given, from the concatenation of every token's value
        // (matches the pure-Python original's own default -- see #parse's note on why there's no
        // separate raw-str overload needing its own default-context construction here)
        ParseContext defaultCtx;
        if (ctx == nullptr) {
            std::string src;
            for (const Token& token : tokens) {
                src += token.val;
            }
            defaultCtx = ParseContext(src);
            ctx = &defaultCtx;
        }

        // Ensures a start/end token wraps the input. Ported as a straight type-based check
        // ("does the first/last token's type already match?") rather than the pure-Python
        // original's literal "tokens[0] != self.startToken" -- that compares a bound Token object
        // against a plain str, which the pybind Token binding never defines __eq__ for, so it's
        // always True (falls back to identity comparison) regardless of the token's actual type.
        // Every real call site's tokens come from a tokenizer that never emits synthetic start/end
        // tokens itself, so this is observably identical to the original in every real/tested case
        // -- it just also does the (evidently unintended) right thing if a caller ever did pass
        // tokens that already carried a start/end token.
        if (tokens.empty() || tokens.front().type != startToken) {
            tokens.insert(tokens.begin(), Token(startToken, startToken, ctx->startLineNo, 0));
        }
        if (tokens.empty() || tokens.back().type != endToken) {
            tokens.push_back(Token(endToken, endToken, ctx->getEndLineNo(), 0));
        }

        ensureIdGenerators();

        dfa_.reset();
        Id currentState = dfa_.getCurrentStateId();
        bool currentIsAccept = dfa_.isAccept(currentState);

        std::vector<Id> stateStack;
        stateStack.push_back(currentState);

        // Only ever pushed to/popped from, never inspected -- see the pure-Python original's own
        // comment on 'symbolStack' for why a synthetic placeholder Token suffices for a reduced
        // non-terminal.
        std::vector<Token> symbolStack;

        typename Tree::Nodes treeNodes;
        typename Tree::Children treeChildren;
        std::vector<Id> treeNodeStack;
        std::optional<Token> currentToken;

        for (const Token& token : tokens) {
            currentToken = token;
            auto reductionsIt = reductions_.find(currentState);
            const Reduction* reduction = (reductionsIt != reductions_.end()) ? &reductionsIt->second : nullptr;
            bool hasReductionNow = hasReduction(currentIsAccept, reduction, token.type);

            while (hasReductionNow) {
                const Id& prodInd = resolveProdInd(*reduction, token.type);
                const Production& production = productions_.at(prodInd);
                const std::string& prodKey = production.first;
                const std::vector<std::string>& prodVals = production.second;

                std::vector<Id> currentChildren;
                for (std::size_t i = 0; i < prodVals.size(); ++i) {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                    currentChildren.push_back(treeNodeStack.back());
                    treeNodeStack.pop_back();
                }
                std::reverse(currentChildren.begin(), currentChildren.end());

                dfa_.setCurrentStateId(stateStack.back());

                Id newState;
                bool newIsAccept;
                bool transitionTaken;
                dfa_.transition(prodKey, &newState, &newIsAccept, &transitionTaken);

                if (!transitionTaken) {
                    raiseSyntaxErr(*ctx, token);
                }

                currentState = newState;
                currentIsAccept = newIsAccept;

                symbolStack.emplace_back(prodKey, "", 0, 0);
                stateStack.push_back(currentState);

                Id parentId = generateNodeId();
                treeNodeStack.push_back(parentId);
                treeNodes.emplace(parentId, ParseNode<Id>(parentId, prodInd));
                treeChildren.emplace(parentId, std::move(currentChildren));

                reductionsIt = reductions_.find(currentState);
                reduction = (reductionsIt != reductions_.end()) ? &reductionsIt->second : nullptr;
                hasReductionNow = hasReduction(currentIsAccept, reduction, token.type);
            }

            // token.type is Optional[str]-shaped (std::optional<std::string>) -- a token with no
            // type can never have a real keyword transition (no transition is ever added keyed by
            // "no type"), so skip straight to the same syntax error the pure-Python original's own
            // "transition on None" would eventually hit anyway (DFA::transition takes 'const
            // std::string&', so there's no direct optional-in equivalent to fall through to here).
            Id newState;
            bool newIsAccept;
            bool transitionTaken = false;
            if (token.type.has_value()) {
                dfa_.transition(*token.type, &newState, &newIsAccept, &transitionTaken);
            }

            if (!transitionTaken) {
                raiseSyntaxErr(*ctx, token);
            }

            currentState = newState;
            currentIsAccept = newIsAccept;

            symbolStack.push_back(token);
            stateStack.push_back(currentState);

            Id nodeId = generateNodeId();
            treeNodeStack.push_back(nodeId);
            treeNodes.emplace(nodeId, ParseNode<Id>(nodeId, std::nullopt, token));
        }

        // last reduction to get the root node
        std::optional<std::string> tokenType = currentToken.has_value() ? currentToken->type : std::nullopt;
        auto reductionsIt = reductions_.find(currentState);
        const Reduction* reduction = (reductionsIt != reductions_.end()) ? &reductionsIt->second : nullptr;
        bool hasReductionNow = hasReduction(currentIsAccept, reduction, tokenType);

        if (!hasReductionNow) {
            Token errToken = currentToken.has_value() ? *currentToken : Token(endToken, endToken, ctx->getEndLineNo(), 0);
            raiseSyntaxErr(*ctx, errToken);
        }

        const Id& prodInd = resolveProdInd(*reduction, tokenType);
        const Production& production = productions_.at(prodInd);
        const std::vector<std::string>& prodVals = production.second;

        std::vector<Id> currentChildren;
        for (std::size_t i = 0; i < prodVals.size(); ++i) {
            symbolStack.pop_back();
            currentState = stateStack.back();
            stateStack.pop_back();
            currentChildren.push_back(treeNodeStack.back());
            treeNodeStack.pop_back();
        }

        dfa_.setCurrentStateId(stateStack.back());

        std::reverse(currentChildren.begin(), currentChildren.end());
        Id rootId = generateNodeId();
        treeNodes.emplace(rootId, ParseNode<Id>(rootId, prodInd));
        treeChildren.emplace(rootId, std::move(currentChildren));

        return Tree(std::move(treeNodes), std::move(treeChildren), rootId);
    }
}
