#ifndef AGRemapCore_RegNewVals_H
#define AGRemapCore_RegNewVals_H

#include <functional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Assigns new values to specific registers for some :cpp:class:`IfContentPart`
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegNewVals: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief The base class this edit derives from
             */
            using Base = BaseRegEdit<K, V, KeyHash, KeyEqual>;

            /**
             * @copydoc BaseRegEdit::ContentPart
             */
            using ContentPart = typename Base::ContentPart;

            /**
             * @copydoc BaseRegEdit::OrderRanges
             */
            using OrderRanges = typename Base::OrderRanges;

            #ifdef AGREMAPCORE_DOCS_PARSE
            #define ModTypePredicate std::function<bool(const V&, const ModType*)>
            #define ValProducer std::function<V(const ModType*)>
            #define NewVal std::variant<V, std::function<V(const ModType*)>>
            #define NewValSpec std::variant<NewVal, std::vector<NewVal>, std::pair<NewVal, std::function<bool(const V&, const ModType*)>>>
            #else
            /**
             * @brief
             @rst
             The conditional-replacement predicate :raw-html:`<br />` :raw-html:`<br />`

             **Deliberately one argument wider than**
             :cpp:type:`IfContentPart::Predicate`, which takes only the old value: a register
             edit always knows which :cpp:class:`ModType` it is running for, and deciding whether
             to replace a register based on that is the whole point of this class living in the
             fixer layer rather than being a plain
             :cpp:func:`IfContentPart::replaceVals` call. ``modType`` is whatever was handed to
             \ref edit, and may be ``nullptr``
             @endrst
             */
            using ModTypePredicate = std::function<bool(const V&, const ModType*)>;

            /**
             * @brief
             @rst
             A new register value that is computed rather than fixed -- takes the
             :cpp:class:`ModType` being fixed (whatever was handed to \ref edit, and may be
             ``nullptr``) and returns the value to write :raw-html:`<br />` :raw-html:`<br />`

             This is the "which value" counterpart to \ref ModTypePredicate's "whether to
             replace": both exist because a register edit always knows the :cpp:class:`ModType`
             it is running for, which is the whole point of this class living in the fixer layer
             rather than being a plain :cpp:func:`IfContentPart::replaceVals` call
             @endrst
             */
            using ValProducer = std::function<V(const ModType*)>;

            /**
             * @brief
             @rst
             A single new value -- either the value itself, or a \ref ValProducer that computes it
             from the :cpp:class:`ModType` being fixed :raw-html:`<br />` :raw-html:`<br />`

             Every place :cpp:func:`IfContentPart::replaceVals` accepts a bare ``V``, \ref
             NewValSpec accepts one of these instead
             @endrst
             */
            using NewVal = std::variant<V, ValProducer>;

            /**
             * @brief
             @rst
             The per-key replacement rule -- the same three alternatives
             :cpp:func:`IfContentPart::replaceVals` takes (a bare replacement value, a positional
             list of values, or a conditional (value, predicate) pair), except that

             * every value slot is a \ref NewVal rather than a bare ``V``, so any of them may be
               computed from the :cpp:class:`ModType` at \ref edit time, and
             * the conditional form uses \ref ModTypePredicate rather than
               :cpp:type:`IfContentPart::Predicate`
             @endrst
             */
            using NewValSpec = std::variant<NewVal, std::vector<NewVal>, std::pair<NewVal, ModTypePredicate>>;
            #endif

            /**
             * @brief
             @rst
             Defines which registers will have their values changed, as (register name, new
             value) pairs :raw-html:`<br />` :raw-html:`<br />`

             See :cpp:func:`IfContentPart::replaceVals` for the full semantics of each spec,
             bearing in mind this class's own wider predicate (\ref ModTypePredicate) and that
             every value slot is a \ref NewVal, which may be a \ref ValProducer instead of a
             plain value
             @endrst
             */
            std::vector<std::pair<K, NewValSpec>> vals;

            /**
             * @brief
             @rst
             Whether to add new `KVPs`_ if the register keys do not exist in the
             :cpp:class:`IfContentPart`
             @endrst
             */
            bool addNewKVPs;

            /**
             * @brief Constructs a new value-assigning register edit
             *
             * @param vals The registers to change, paired with their new values
             * @param addNewKVPs
             @rst
             Whether to add new `KVPs`_ if the register keys do not exist in the
             :cpp:class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             */
            explicit RegNewVals(std::vector<std::pair<K, NewValSpec>> vals = {}, bool addNewKVPs = false);

            /**
             * @brief
             @rst
             Assigns the new values in \ref vals to 'part', by forwarding to
             :cpp:func:`IfContentPart::replaceVals` :raw-html:`<br />` :raw-html:`<br />`

             Every \ref ValProducer is called with 'modType' to get the value to write, and every
             conditional spec's \ref ModTypePredicate is bound against 'modType' to produce the
             plain single-argument :cpp:type:`IfContentPart::Predicate`
             :cpp:func:`IfContentPart::replaceVals` expects -- which is why this class keeps its
             own spec type instead of reusing :cpp:type:`IfContentPart::ReplaceSpec` directly
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited. Unused by this edit
             * @param modType The type of mod to fix, or ``nullptr``. Passed to every \ref ValProducer and \ref ModTypePredicate in \ref vals. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                              const std::string& modName = "", const OrderRanges* partRanges = nullptr) override;
    };
}

#include "RegNewVals.tpp"

#endif
