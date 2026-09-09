#ifndef AGRemapCore_RegDelimitedAdd_H
#define AGRemapCore_RegDelimitedAdd_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/graphEdits/BaseIniGraphEdit.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphEdit`

     Adds a `KVP`_ into some caller/callee graph of :cpp:class:`IniSectionGraph` **exactly once
     per delimiter-free stretch of every execution path** :raw-html:`<br />` :raw-html:`<br />`

     Cut every execution path through the graph at each accepted occurence of a register in
     \ref delimiterRegs. That leaves `segments`: from the start of the path to its first
     delimiter, from each delimiter to the next, and from the last delimiter to the end of the
     path. This edit places \ref additions so that **every segment of every path contains it
     exactly once, as late as possible**, which it does with two placement rules:

     1. immediately before every accepted delimiter occurence, in every part, and
     2. at the end of every `path-terminal` part -- a part after which nothing more executes on
        its path (its :cpp:func:`CallGraph::exitNodeOf` node has no successor: the end of a
        `section`_ that no other section ``run``\\s)

     Both are exact by construction. A pre-delimiter insertion is followed on every path by its
     own delimiter, so it can only ever sit in a segment that ends at that delimiter; a path-end
     insertion has nothing after it, so a path crosses at most one, and every path crosses exactly
     one. No `fixpoint iteration`_, no once-per-window claiming: the whole edit is one walk
     :raw-html:`<br />` :raw-html:`<br />`

     The motivating case is a texture fix (``NNFix``/``ORFix``) that must be issued right before
     every ``drawindexed`` it applies to, and issued once more -- but never twice in a row -- at the
     end of a path that renders nothing:

     .. code-block:: ini

        a = 1
        if $x == 2
           insertion = NNFix      ; before the draw
           drawindexed = a
        else
          foo = 2                 ; nothing here -- the shared end below covers this path
        endif
        b = 3
        insertion = NNFix         ; once, at the end of both paths

     .. note::
        Do not reach for :cpp:class:`RegSurroundedAdd` for this rule -- that edit inserts once per
        `surrounded` **window** across the graph, so two delimiters in sequence get a single
        insertion before the last of them, and no combination of its options reproduces the
        per-segment invariant above

     .. note::
        ``run =`` is modeled as `call-with-return`_: the end of a called `section`_ is not a path
        end, its caller continues afterwards, so the trailing insertion lands in the caller. The
        one case this cannot get exactly right is a `section`_ that is **both** a root and a
        ``run`` target: its end is a path end when reached directly but not when called, and a
        single physical position cannot serve both. Such a section gets **no** trailing insertion
        -- by choice, this edit never doubles the addition, at the cost of missing it on that
        section's direct path. Real mods do not ``run`` their hash-triggered sections, so this does
        not arise in practice

     A \ref BaseIniGraphEdit::PartFilter given to \ref edit gates each candidate position on its
     own (a position the filter's ranges do not contain is simply skipped, never relocated), so a
     filter that excludes the only valid position of a segment breaks the invariant for that
     segment -- the filter is the caller's statement that the part must not be touched there
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegDelimitedAdd: public BaseIniGraphEdit<K, V, KeyHash, KeyEqual> {
        public:

            using Base = BaseIniGraphEdit<K, V, KeyHash, KeyEqual>;

            using Graph = typename Base::Graph;

            using PartFilter = typename Base::PartFilter;

            using IterData = typename Base::IterData;

            using OrderRanges = typename Base::OrderRanges;

            using KeySet = typename Base::KeySet;

            using ContentPart = typename Graph::ContentPart;

            using CallGraphType = typename Graph::CallGraphType;

            using Node = typename CallGraphType::Node;

            /**
             * @brief
             @rst
             The predicate for which particular occurence of a delimiter register to accept, taking
             in the value of the occurence -- an **empty** function accepts any value (the same
             convention as :cpp:type:`RegSurroundedAdd::Predicate`)
             @endrst
             */
            using Predicate = std::function<bool(const V&)>;

            /**
             * @brief
             @rst
             The registers that delimit the segments -- the keys are register names, the values
             are their \ref Predicate (empty accepts any value); same format as
             :cpp:type:`RegSurroundedAdd::RegMap`
             @endrst
             */
            using RegMap = std::unordered_map<K, Predicate, KeyHash, KeyEqual>;

            /**
             * @brief The list of `KVP`_ entries an edit adds -- same shape as :cpp:type:`RegSurroundedAdd::Additions`
             */
            using Additions = std::vector<std::pair<K, V>>;

            /**
             * @brief
             @rst
             The `KVP`_ entries to add. All of them land together at each chosen position, as
             consecutive lines in this order -- an empty list makes the edit a no-op
             @endrst
             */
            Additions additions;

            /**
             * @brief
             @rst
             The registers whose accepted occurences cut every execution path into the segments
             \ref additions is added exactly once into (eg. ``drawindexed`` and
             ``drawindexedinstanced``, each with an empty \ref Predicate) :raw-html:`<br />`
             :raw-html:`<br />`

             Empty is allowed and means every path is a single segment: \ref additions is added
             once at the end of every path and nowhere else
             @endrst
             */
            RegMap delimiterRegs;

            /**
             * @brief
             @rst
             Whether the end-of-path addition is made only when the path never delimits at all
             :raw-html:`<br />` :raw-html:`<br />`

             ``false`` (the default) is the plain segment rule: every delimiter-free segment holds
             the addition, the last one -- from the final delimiter to the end of the path --
             included.

             ``true`` drops that final segment, giving "before every delimiter, **or** once at the
             end if there is no delimiter anywhere on the path". That is what the mandatory fix
             libraries want: NNFix and ORFix belong immediately before each draw, and a call after
             the last draw is a surplus one that ORFix would use to swap the diffuse and lightmap
             registers back. A graph with no draw call at all still gets its single addition, which
             is what places NNFix on a character whose fix does not move the draw call

             **Default**: ``false``
             @endrst
             */
            bool pathEndOnlyWhenUndelimited = false;

            /**
             * @brief Constructs a new per-segment adding edit
             *
             * @param additions The `KVP`_ entries to add, in order
             * @param delimiterRegs The registers that delimit the segments
             * @param pathEndOnlyWhenUndelimited
             @rst
             Whether to make the end-of-path addition only on a path that never delimits -- see
             \ref pathEndOnlyWhenUndelimited :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            explicit RegDelimitedAdd(Additions additions = {}, RegMap delimiterRegs = {},
                                      bool pathEndOnlyWhenUndelimited = false);

            /**
             * @brief
             @rst
             Retrieves the insertion order indices of 'part' that sit **immediately before** each
             accepted occurence of a register in \ref delimiterRegs, ascending and without
             duplicates :raw-html:`<br />` :raw-html:`<br />`

             An insertion index ``i`` means "insert before the `KVP`_ currently at order index
             ``i``", so this is simply the order index of each accepted occurence
             @endrst
             *
             * @param part The part to inspect
             */
            std::vector<long long> getDelimiterInds(const ContentPart& part) const;

            /**
             * @brief
             @rst
             Checks whether 'part' is `path-terminal`: nothing more executes after it on its path
             -- its :cpp:func:`CallGraph::exitNodeOf` node has no successor in 'callGraph'
             @endrst
             *
             * @param callGraph The call graph of the graph being edited
             * @param part The part to check
             */
            static bool isPathEnd(const CallGraphType& callGraph, ContentPart* part);

            // Inserts every entry of 'additions' at 'index' of 'part', keeping their order -- the
            // whole list lands together, as consecutive lines
            void addAdditionsAt(ContentPart& part, long long index) const;

            /**
             * @brief
             @rst
             Adds \ref additions exactly once into every delimiter-free segment of every execution
             path through 'graph', as late as possible -- see the class description for the two
             placement rules :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                'modName'/'trackKeys'/'keysToTrack' are the caller's defaults, handed down by
                :cpp:class:`BaseIniGraphEdit`'s contract (:cpp:class:`GraphGroupEdit` passes its
                own). This edit reads its delimiters straight off each part and never colours the
                graph, so it has no use for them -- they are accepted only so the shared call
                convention keeps working
             @endrst
             */
            Graph& edit(Graph& graph, const ModType* modType,
                         const std::string& modName = "", const PartFilter& partFilter = {},
                         bool trackKeys = false, const std::optional<KeySet>& keysToTrack = std::nullopt) override;
    };
}

#include "RegDelimitedAdd.tpp"

#endif
