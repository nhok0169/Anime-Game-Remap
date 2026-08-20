#include "AGRemapCore/tools/dfa/BaseDFA.h"
#include "AGRemapCore/tools/idGenerator/IncIdGenerator.h"


namespace AGRemapCore {
    template class IncIdGenerator<std::uint64_t>;


    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::BaseDFA() {
        initStateIdGenerator();
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    const State& BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getStartId() {
        return states.getKey(startId);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    const State* BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getStartIdPtr() {
        if (startId == 0) return nullptr;
        return states.findKeyPtr(startId);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::setStartId(const State& newStartId) {
        const std::uint64_t *newStartIdPtr = states.findValuePtr(newStartId);
        if (newStartIdPtr == nullptr) return false;

        startId = *newStartIdPtr;
        return true;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    const State& BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getCurrentStateId() {
        return states.getKey(currentStateId);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    const State* BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getCurrentStateIdPtr() {
        if (currentStateId == 0) return nullptr;
        return states.findKeyPtr(currentStateId);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::setCurrentStateId(const State& newCurrentStateId) {
        const std::uint64_t *newCurrentStateIdPtr = states.findValuePtr(newCurrentStateId);
        if (newCurrentStateIdPtr == nullptr) return false;

        currentStateId = *newCurrentStateIdPtr;
        return true;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    void BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::clear() {
        states.clear();
        neighbours.clear();
        funcNeighbours.clear();
        accept.clear();
        stateIdGenerator->reset();

        currentStateId = 0;
        startId = 0;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    void BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::initStateIdGenerator() {
        stateIdGenerator = std::make_unique<IncIdGenerator<std::uint64_t>>(1);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::addState(const State& id, std::uint64_t *resStateId, std::optional<bool> isAccept, bool isStart) {
        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        std::uint64_t stateId;
        bool isNewlyAdded = false;

        bool isEmpty = states.empty();
        if (isEmpty) {
            isStart = true;
        }

        if (stateIdPtr != nullptr) {
            stateId = *stateIdPtr;
        } else {
            isNewlyAdded = true;
            stateIdGenerator->getId(stateId);
            states.insert(id, stateId);
        }

        bool isAcceptHasValue = isAccept.has_value();
        if (isAcceptHasValue && !(*isAccept) && accept.find(stateId) != accept.end()) {
            accept.erase(stateId);
        } else if (isAcceptHasValue && *isAccept) {
            accept.emplace(stateId);
        }

        if (isStart) {
            startId = stateId;
        }

        if (isEmpty) {
            currentStateId = stateId;
        }

        *resStateId = stateId;
        return isNewlyAdded; 
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::addState(const State& id, std::optional<bool> isAccept, bool isStart) {
        std::uint64_t resStateId;
        return addState(id, &resStateId, isAccept, isStart);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::isAccept(const State& id) {
        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        if (stateIdPtr == nullptr) return false;

        return (accept.find(*stateIdPtr) != accept.end());
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::stateExists(const State& id) {
        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        return stateIdPtr != nullptr;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::isStart(const State& id) {
        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        if (stateIdPtr == nullptr) return false;

        return *stateIdPtr == startId;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    void BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::reset() {
        currentStateId = startId;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    size_t BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::stateSize() const {
        return states.size();
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    size_t BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::acceptSize() const {
        return accept.size();
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    const std::uint64_t* BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getStateId(const State& id) {
        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        if (stateIdPtr != nullptr) return stateIdPtr;

        std::uint64_t resStateId;
        addState(id, &resStateId, false, false);
        return states.findValuePtr(id);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::addKeywordTransition(const State& srcId, const Transition& keyword, const State& destId) {
        const std::uint64_t *srcStateIdPtr =  states.findValuePtr(srcId);
        if (srcStateIdPtr == nullptr) return false;

        const std::uint64_t *dstStateIdPtr = getStateId(destId);
        auto& srcNeighbours = neighbours[*srcStateIdPtr];
        srcNeighbours[keyword] = *dstStateIdPtr;
        return true;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::hasKeywordTransition(const State& srcId, const Transition& keyword) {
        const std::uint64_t *srcStateIdPtr = states.findValuePtr(srcId);
        if (srcStateIdPtr == nullptr) return false;

        auto neighboursKVP = neighbours.find(*srcStateIdPtr);
        if (neighboursKVP == neighbours.end()) return false;

        return neighboursKVP->second.find(keyword) != neighboursKVP->second.end();
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    std::optional<State> BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getKeywordToState(const State& srcId, const Transition& keyword) {
        const std::uint64_t *srcStateIdPtr = states.findValuePtr(srcId);
        if (srcStateIdPtr == nullptr) return std::nullopt;

        auto neighboursKVP = neighbours.find(*srcStateIdPtr);
        if (neighboursKVP == neighbours.end()) return std::nullopt;

        auto keywordKVP = neighboursKVP->second.find(keyword);
        if (keywordKVP == neighboursKVP->second.end()) return std::nullopt;

        return states.getKey(keywordKVP->second);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    bool BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::addFuncTransition(const State& srcId, const std::function<bool(const State&)>& func, const State& destId) {
        const std::uint64_t *srcStateIdPtr = states.findValuePtr(srcId);
        if (srcStateIdPtr == nullptr) return false;

        const std::uint64_t *dstStateIdPtr = getStateId(destId);
        funcNeighbours[*srcStateIdPtr].emplace_back(func, *dstStateIdPtr);
        return true;
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    void BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::transition(const Transition& keyword, State* newState, bool* isAccept, bool* transitionTaken) {
        auto currentNeighbourKVP = neighbours.find(currentStateId);

        if (currentNeighbourKVP != neighbours.end()) {
            auto& currentNeighbours = currentNeighbourKVP->second;
            auto keywordTransKVP = currentNeighbours.find(keyword);

            if (keywordTransKVP != currentNeighbours.end()) {
                currentStateId = keywordTransKVP->second;
                *isAccept = (accept.find(currentStateId) != accept.end());
                *transitionTaken = true;
                *newState = states.getKey(currentStateId);
                return;
            }
        }

        auto currentFuncNeighboursKVP = funcNeighbours.find(currentStateId);

        if (currentFuncNeighboursKVP != funcNeighbours.end()) {
            auto& currentFuncNeighbours = currentFuncNeighboursKVP->second;

            for (const auto& [func, newStateId] : currentFuncNeighbours) {
                if (func(keyword)) {
                    currentStateId = newStateId;
                    *isAccept = (accept.find(newStateId) != accept.end());
                    *transitionTaken = true;
                    *newState = states.getKey(newStateId);
                    return;
                }
            }
        }

        *isAccept = (accept.find(currentStateId) != accept.end());
        *transitionTaken = false;
        *newState = states.getKey(currentStateId);
    }

    template <typename State, typename Transition, typename StateEq, typename StateHash, typename TransEq, typename TransHash>
    std::vector<Transition> BaseDFA<State, Transition, StateEq, StateHash, TransEq, TransHash>::getKeywordTransitions(const State& id, bool* stateFound) {
        std::vector<Transition> result;

        const std::uint64_t *stateIdPtr = states.findValuePtr(id);
        if (stateIdPtr == nullptr) {
            *stateFound = false;
            return result;
        }

        *stateFound = true;

        auto neighboursKVP = neighbours.find(*stateIdPtr);
        if (neighboursKVP == neighbours.end()) {
            return result;
        }

        result.reserve(neighboursKVP->second.size());
        for (const auto& [keyword, destStateId] : neighboursKVP->second) {
            result.push_back(keyword);
        }

        return result;
    }
}