#ifndef AGRemapCore_GIMIObjPartFilter_H
#define AGRemapCore_GIMIObjPartFilter_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/SectionIterData.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     Works out **which part of a** :cpp:class:`IfContentPart` **belongs to one mod object**, by its
     ``hash`` and ``match_first_index`` `KVP`_ values :raw-html:`<br />` :raw-html:`<br />`

     The fix-side counterpart of :cpp:class:`GIMISectionClassifier`, and the same question asked
     from the other end. That class answers *"which mod objects does this part belong to"* while
     parsing; this one answers *"which order indices of this part belong to **this** mod object"*
     while fixing, and hands back the :cpp:type:`PartFilter` a
     :cpp:class:`GraphGroupEdit`'s ``keyFilters`` takes :raw-html:`<br />` :raw-html:`<br />`

     **Why a fixer needs this at all.** A parser built with ``disjointModObjs = false`` routinely
     classifies one `section`_ into several mod objects at once -- see
     :cpp:member:`GIMIParser::disjointModObjs` -- and only the stretch of that `section`_ governed
     by a given object's own ``hash``/``match_first_index`` pair actually belongs to it. An edit run
     over the whole `section`_ for each of its mod objects would rewrite ``head``'s registers while
     "fixing" ``body``. Restricting each edit to \\ref window is what keeps them apart
     :raw-html:`<br />` :raw-html:`<br />`

     Typical use, one entry per edit in the graph's own edit list:

     .. code-block:: cpp

        GIMIObjPartFilter<> objFilter(ctx.modTypeHashes(), ctx.modTypeIndices(), {"ib"}, ctx.version());

        iniEdits.edits[modObj]       = {someEdit, someOtherEdit};
        iniEdits.keyFilters[modObj]  = {objFilter.filter(modObj), objFilter.filter(modObj)};
        iniEdits.keysToTrack[modObj] = objFilter.keysToTrack();
        iniEdits.trackKeys[modObj]   = true;

     :raw-html:`<br />`

     .. note::
        \\ref keysToTrack exists so a caller never has to restate which keys the filter reads. Get
        that set wrong and the filter silently sees an empty colouring, which reads as "this mod
        object owns nothing" and quietly skips every edit rather than failing
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GIMIObjPartFilter {
        public:

            /**
             * @brief The ``(component name, mod object name)`` pair naming one mod object
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @brief The kind of asset table \\ref hashes / \\ref indices are
             */
            using Assets = ModMappedAssets<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief The `KVP`_ state of the :cpp:class:`IfContentPart` being filtered
             */
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief The per-part iteration data a \\ref PartFilter is handed
             */
            using IterData = SectionIterData<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ranges of `KVP`_ order indices a \\ref PartFilter returns
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief The set of `KVP`_ keys to track while iterating
             */
            using KeySet = std::unordered_set<K, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             What :cpp:member:`GraphGroupEdit::IniEdits::keyFilters` and
             :cpp:func:`BaseIniGraphEdit::edit` both take -- spelled out here rather than borrowed
             from either, so this header does not have to include the whole edit hierarchy to
             describe its own return type
             @endrst
             */
            using PartFilter = std::function<OrderRanges(const IterData&, const ModType*, IniFile*)>;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs, for the same reason
             :cpp:class:`GIMISectionClassifier::ClassifierConfig` exists: ``K`` is not
             ``std::string`` for every instantiation, so the `KVP`_ keys cannot be spelled as
             literals here
             @endrst
             */
            struct FilterConfig {
                /**
                 * @brief The `KVP`_ key holding a model hash -- :cpp:member:`IniKeywords::Hash` for a plain C++ caller
                 */
                K hashKey;

                /**
                 * @brief The `KVP`_ key holding a first-drawn index -- :cpp:member:`IniKeywords::MatchFirstIndex` for a plain C++ caller
                 */
                K matchFirstIndexKey;
            };

            /**
             * @brief The default \\ref FilterConfig for a plain, ``std::string``-keyed C++ caller
             */
            static FilterConfig defaultConfig();

            /**
             * @brief Constructs a new part filter
             *
             * @param hashes The ``hash`` assets to resolve a ``hash`` `KVP`_ value against. **Nullable** -- a null table matches nothing
             * @param indices The ``match_first_index`` assets. **Nullable**, same meaning
             * @param indexHashKeys
             @rst
             The *types* of hash (the last index column of a :cpp:class:`Hashes` row, eg. ``ib``)
             whose mod objects need a ``match_first_index`` to tell them apart -- the counterpart
             of :cpp:member:`GIMISectionClassifier::indexKeyToModObj`'s outer keys
             :raw-html:`<br />` :raw-html:`<br />`

             A ``hash`` resolving to a type **not** listed here is ignored outright: this class
             deliberately has no "hash alone identifies the object" path, because a fixer restricts
             an edit to a *window*, and a hash with no index has no window to speak of
             @endrst
             * @param version The version of the .ini file, or ``std::nullopt`` for "the latest". **Default**: ``std::nullopt``
             * @param config The .ini-domain customization points to use. **Default**: \\ref defaultConfig
             */
            explicit GIMIObjPartFilter(Assets* hashes, Assets* indices, KeySet indexHashKeys = {},
                                        std::optional<Version> version = std::nullopt,
                                        FilterConfig config = defaultConfig());

            /**
             * @brief The types of hash whose mod objects need a ``match_first_index`` -- see the constructor
             */
            KeySet indexHashKeys;

            /**
             * @brief The version of the .ini file, or ``std::nullopt`` for "the latest"
             */
            std::optional<Version> version;

            /**
             * @brief The ``hash`` assets -- borrowed, may be ``nullptr``
             */
            Assets* hashes() const;

            /**
             * @copydoc hashes() const
             */
            void setHashes(Assets* newHashes);

            /**
             * @brief The ``match_first_index`` assets -- borrowed, may be ``nullptr``
             */
            Assets* indices() const;

            /**
             * @copydoc indices() const
             */
            void setIndices(Assets* newIndices);

            /**
             * @brief The .ini-domain customization points this instance uses
             */
            const FilterConfig& config() const;

            /**
             * @brief
             @rst
             The `KVP`_ keys this filter reads -- exactly \\ref FilterConfig's two
             :raw-html:`<br />` :raw-html:`<br />`

             Hand this straight to :cpp:member:`GraphGroupEdit::IniEdits::keysToTrack` rather than
             restating it; see this class's own note on what a wrong set silently does
             @endrst
             */
            KeySet keysToTrack() const;

            /**
             * @brief
             @rst
             The order indices of 'colouring' that belong to 'modObj' :raw-html:`<br />`
             :raw-html:`<br />`

             Walks the ``hash`` values in order. Each one whose type is in \\ref indexHashKeys opens
             a window running to the *next* ``hash``'s index (or to the end of the part, when it is
             the last), and that window is included when a ``match_first_index`` **inside it**
             resolves to 'modObj'\\'s own :cpp:class:`Indices` row -- the same windowing
             :cpp:func:`GIMISectionClassifier::classify` does
             @endrst
             *
             * @param modObj The mod object to find the ranges of
             * @param colouring The current `KVP`_ state of the part
             *
             * @return The ranges belonging to 'modObj' -- empty when none of it does
             */
            OrderRanges window(const ModObj& modObj, const Colouring& colouring) const;

            /**
             * @brief
             @rst
             \\ref window as a ready-made \\ref PartFilter, for a
             :cpp:member:`GraphGroupEdit::IniEdits::keyFilters` entry :raw-html:`<br />`
             :raw-html:`<br />`

             The returned filter **captures this object by pointer**, so it must not outlive it --
             the same borrowing rule the edits themselves follow. A part with no colouring at all
             (key tracking off) yields empty ranges rather than the whole part: this filter cannot
             answer the question without one, and answering "all of it" would be the dangerous
             guess
             @endrst
             *
             * @param modObj The mod object to restrict an edit to
             */
            PartFilter filter(const ModObj& modObj) const;

        private:
            Assets* hashes_;
            Assets* indices_;
            FilterConfig config_;
    };
}

#include "GIMIObjPartFilter.tpp"

#endif
