#ifndef AGRemapCore_MultiModFixer_H
#define AGRemapCore_MultiModFixer_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/strategies/iniFixers/BaseIniFixer.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniFixer`

     A fixer that owns no fixing logic of its own and instead **delegates to one child fixer per
     mod type**, keyed by the ``int`` value of that mod type's :cpp:enum:`ModTypeId`
     :raw-html:`<br />` :raw-html:`<br />`

     Which children run is decided by the ``.ini`` file's
     :cpp:member:`IniFile::filteredToModTypeIds`: a child whose id is absent from that filter is
     skipped. ``std::nullopt`` there means no filter, so every child runs

     .. note::
        A child is a plain :cpp:class:`BaseIniFixer`, and this class *is* one -- so a
        ``MultiModFixer`` can be a child of another, and the nesting composes. #fixImpl narrows
        the :cpp:class:`IniFixingContext` it was handed rather than replacing it, which is what
        makes that work: a nested fixer's first child is "the file's first" only if its parent was
        told the same

     .. note::
        Children run in the order they were **inserted**, which #Children being a
        ``tsl::ordered_map`` is what preserves. Only one child may take the ``.ini`` file's backup
        and only one may hide the original mod, so which one is "first" and which is "last" has to
        be answerable the same way twice -- and it is the caller who decides, by ordering the map
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>,
              typename KeyEqual = std::equal_to<K>, typename FixerBase = BaseIniFixer<K, V, KeyHash, KeyEqual>>
    class MultiModFixer: public FixerBase {
        public:

            /**
             * @brief The base this fixer splices itself on top of
             */
            using Base = FixerBase;

            /**
             * @brief The plain fixer interface a child is held by
             */
            using Child = BaseIniFixer<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The children, keyed by the ``int`` value of the :cpp:enum:`ModTypeId` each one fixes
             @endrst
             */
            using Children = tsl::ordered_map<int, std::shared_ptr<Child>>;

            /**
             * @copydoc BaseIniFixer::FixResult
             */
            using FixResult = typename Child::FixResult;

            /**
             * @copydoc BaseIniFixer::ParseData
             */
            using ParseData = typename Child::ParseData;

            /**
             * @brief Constructs a new fixer that delegates to 'children'
             *
             * @param children
             @rst
             The child fixers, keyed by the ``int`` value of the :cpp:enum:`ModTypeId` each fixes.
             A ``nullptr`` child is skipped rather than treated as an error :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: no children, which makes #fixImpl a no-op returning an empty result
             @endrst
             * @param parser The parser to retrieve the data to fix from. **Default**: ``nullptr``
             */
            explicit MultiModFixer(Children children = {}, typename Child::Parser* parser = nullptr);

            /**
             * @brief The child fixers, keyed by mod type id
             *
             * @return The children
             */
            const Children& getChildren() const;

            /**
             * @brief Replaces the child fixers
             *
             * @param children The new children -- see the constructor's own parameter
             */
            void setChildren(Children children);

            /**
             * @brief
             @rst
             Clears this fixer and every child, so a :cpp:func:`BaseIniFixer::clear` on the parent
             reaches all the way down a nest of them
             @endrst
             */
            void clear() override;

        protected:

            /**
             * @brief
             @rst
             Runs each selected child in turn and merges what they produce :raw-html:`<br />`
             :raw-html:`<br />`

             The merge is the same one :cpp:func:`IniFile::fix` performs over its own per-mod-type
             fixers: a later child writing the same file path simply overwrites an earlier one

             .. note::
                Children are invoked through the public :cpp:func:`BaseIniFixer::fix`, which fixes
                *with* boilerplate and *with* the source content. 'withBoilerPlate' and 'withSrc'
                are therefore **not** propagated to them -- ``fixImpl`` is protected, so a child's
                cannot be reached from here. This matches how :cpp:func:`IniFile::fix` invokes its
                own fixers, which is the behaviour this method mirrors
             @endrst
             *
             * @copydetails BaseIniFixer::fixImpl
             */
            FixResult fixImpl(ParseData& parseData, bool keepBackup, bool fixOnly, bool hideOrig,
                               bool withBoilerPlate, bool withSrc, IniFixingContext fixingCtx) override;

            /**
             * @brief
             @rst
             The ids of the children to run, in insertion order, after applying the ``.ini`` file's
             :cpp:member:`IniFile::filteredToModTypeIds` :raw-html:`<br />` :raw-html:`<br />`

             A child with no fixer behind it is left out, so the ids returned here are exactly the
             ones that will contribute -- which is what lets #fixImpl work out which of them is the
             file's first and last without a second pass
             @endrst
             *
             * @return The ids of the children that will run, in insertion order
             */
            virtual std::vector<int> selectedChildIds() const;

            /**
             * @brief The child fixers, keyed by mod type id
             */
            Children children_;
    };
}

#include "MultiModFixer.tpp"

#endif
