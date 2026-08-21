#ifndef AGRemapCore_BaseDFA_H
#define AGRemapCore_BaseDFA_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <utility>
#include <optional>
#include <memory>
#include <vector>

#include "AGRemapCore/tools/BiMap.h"
#include "AGRemapCore/tools/idGenerator/BaseIdGenerator.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A template for a `DFA`_
     @endrst
     * 
     * @tparam State
     *      The type for a state
     * 
     * @tparam Transition
     *      The type for a transition
     * 
     * @tparam StateEq
     *      The eqaulity function for states
     * 
     * @tparam StateHash
     *      The hash function for states
     * 
     * @tparam TransEq
     *      The equality function for transitions
     * 
     * @tparam  TransHash
     *      The hash function for transitions
     */
    template <typename State, typename Transition, typename StateEq = std::equal_to<State>, typename StateHash = std::hash<State>, typename TransEq = std::equal_to<Transition>, typename TransHash = std::hash<Transition>>
    class BaseDFA {
        public:

            /**
             * @brief
             @rst
             Constructs a new `DFA`_
             @endrst
             */
            BaseDFA();

            /**
             * @brief Getter for the starting state
             * 
             * @return The starting state
             */
            const State& getStartId();

            /**
             * @brief Getter for the pointer to the starting state
             * 
             * @return The pointer to the starting state
             */
            const State* getStartIdPtr();

            /**
             * @brief Setter for the starting state
             * 
             * @param newStartId The state to be set as the new starting state
             * 
             * @return Whether the starting state has been successfully set
             */
            bool setStartId(const State& newStartId);

            /**
             * @brief Getter for the current state
             * 
             * @return The current state
             */
            const State& getCurrentStateId();

            /**
             * @brief Getter for the pointer to the current states
             * 
             * @return The pointer to the current state
             */
            const State* getCurrentStateIdPtr();

            /**
             * @brief Setter for the current state
             * 
             * @param newCurrentStateId The state to be set as the new current state
             * 
             * @return Whether the current state has been successfully set
             */
            bool setCurrentStateId(const State& newCurrentStateId);

            /**
             * @brief
             @rst
             Clears the `DFA`_
             @endrst
             */
            virtual void clear();

            /**
             * @brief
             @rst
             Resets the `DFA_` back to its starting state
             @endrst
             */
            virtual void reset();

            /**
             * @brief Checks whether the state is an accepting state
             * 
             * @param id The state to check
             * 
             * @return Whether the state is an accepting state
             */
            virtual bool isAccept(const State& id);

            /**
             * @brief Checks whether the state is a starting state
             * 
             * @param id The state to check
             * 
             * @return Whether the state is a starting state
             */
            virtual bool isStart(const State& id);

            /**
             * @brief
             @rst
             Checks whether the state exists inside of the `DFA`_
             @endrst
             * 
             * @param id The state to check
             * 
             * @return Whether the state exists
             */
            virtual bool stateExists(const State& id);
            
            /**
             * @brief Retrieves the number of states
             * 
             * @return The number of states
             */
            virtual size_t stateSize() const;

            /**
             * @brief Retrieves the number of accepting states
             * 
             * @return The number of accepting states
             */
            virtual size_t acceptSize() const;

            /**
             * @brief Adds a state
             * 
             * @param id The state to add
             * @param isAccept Whether the state is an accepting state
             * @param isStart Whether the state is a starting state
             * 
             * @note
             @rst
             There can only be 1 starting state in the `DFA`_
             @endrst
             * 
             * @return Whether the state has been added
             */
            virtual bool addState(const State& id, std::optional<bool> isAccept = std::nullopt, bool isStart = false);

            /**
             * @brief Adds a keyword transition
             * 
             * @param srcId The source state for the transition
             * @param keyword The keyword used to trigger the transition
             * @param destId The destionation state for the transition
             * 
             * @return Whether the transition has been added
             */
            bool addKeywordTransition(const State& srcId, const Transition& keyword, const State& destId);

            /**
             * @brief Checks whether a keyword transition exists from a particular state
             *
             * @param srcId The source state to check
             * @param keyword The keyword for the transition to check
             *
             * @return Whether the transition exists from 'srcId'
             */
            virtual bool hasKeywordTransition(const State& srcId, const Transition& keyword);

            /**
             * @brief Retrieves the destination state of a keyword transition from a particular state
             *
             * @param srcId The source state for the transition
             * @param keyword The keyword for the transition
             *
             * @return
             @rst
             The destination state of the transition, or ``std::nullopt`` if no such transition
             exists from ``srcId``
             @endrst
             */
            virtual std::optional<State> getKeywordToState(const State& srcId, const Transition& keyword);

            /**
             * @brief Adds a transition that is triggered based on a predicate
             *
             * @param srcId The source state for the transition
             * @param func The predicate to trigger the transition -- called with the keyword passed to \ref transition
             * @param destId The destionation state for the transition
             *
             * @warning When checking whether a transition is available for some state, given a keyword, using \ref transition ,
             *      keyword transitions are typically faster to check compared to predicate based transitions since keyword transitions check
             *      using their hash. You can add keyword transitions using \ref addKeywordTransition
             *
             * @return Whether the transition has been added
             */
            bool addFuncTransition(const State& srcId, const std::function<bool(const Transition&)>& func, const State& destId);

            /**
             * @brief transitions to a new state
             *
             * @param keyword The keyword used to trigger the transition
             * @param newState The resultant new state that got transitioned to
             * @param isAccept Whether the new state is an accepting state
             * @param transitionTaken Whether a transition was made
             */
            void transition(const Transition& keyword, State* newState, bool* isAccept, bool* transitionTaken);

            /**
             * @brief Retrieves all the keyword transitions connected to a particular state
             *
             * @param id The state to retrieve the keyword transitions for
             * @param stateFound A resultant pointer that indicates whether 'id' exists in the `DFA`_
             *
             * @return
             @rst
             The keyword transitions connected to ``id`` :raw-html:`<br />` :raw-html:`<br />`

             If ``id`` does not exist in the `DFA`_, an empty list is returned and ``stateFound``
             is set to ``false``
             @endrst
             */
            virtual std::vector<Transition> getKeywordTransitions(const State& id, bool* stateFound);

        protected:
            /**
             * @brief The generator used to generate internal ids for the states
             */
            std::unique_ptr<BaseIdGenerator<std::uint64_t>> stateIdGenerator;

            /**
             * @brief The internal id for the current state
             * 
             * @note An id of 0 indicates that the id is not set
             */
            uint64_t currentStateId = 0;

            /**
             * @brief The internal id for the starting state
             * 
             * @note An id of 0 indicates that the id is not set
             */
            uint64_t startId = 0;

            /**
             * @brief
             @rst
             A `one-to-one`_ map for states to their internal ids
             @endrst
             */
            BiMap<State, std::uint64_t, StateHash, StateEq, std::hash<std::uint64_t>, std::equal_to<std::uint64_t>> states;

            /**
             * @brief The set of internal state ids that are accepting states
             */
            std::unordered_set<std::uint64_t> accept;

            /**
             * @brief The neighbours to the states based on keywords
             */
            std::unordered_map<std::uint64_t, std::unordered_map<Transition, std::uint64_t, TransHash, TransEq>> neighbours;

            /**
             * @brief The neighbour to the states based on predicates
             */
            std::unordered_map<std::uint64_t, std::vector<std::pair<std::function<bool(const Transition&)>, std::uint64_t>>> funcNeighbours;

            /**
             * @brief Initializes the generator for generating internal ids for the states
             */
            virtual void initStateIdGenerator();

            /**
             * @brief Internal function for adding a state
             * 
             * @param id The state to add
             * @param resStateId The resultant internal id of the state
             * @param isAccept Whether the state is an accepting state
             * @param isStart Whether the state is a starting state
             * 
             * @note See the note at \ref addState
             * 
             * @return Whether the state has been added
             */
            bool addState(const State& id, std::uint64_t *resStateId, std::optional<bool> isAccept = std::nullopt, bool isStart = false);

            /**
             * @brief Retrieves the internal id of the starting state
             * 
             * @param id The target state
             * 
             * @return The internal id of the state
             */
            const std::uint64_t* getStateId(const State& id);
    };
}


#include "BaseDFA.tpp"

#endif