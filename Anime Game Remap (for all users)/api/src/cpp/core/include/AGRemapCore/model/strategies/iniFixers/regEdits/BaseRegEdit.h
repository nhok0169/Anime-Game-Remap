#ifndef AGRemapCore_BaseRegEdit_H
#define AGRemapCore_BaseRegEdit_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/strategies/iniFixers/BaseIniGraphPartEdit.h"
#include "AGRemapCore/tools/Ranges.h"


namespace AGRemapCore {

    class IniFile;
    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniGraphPartEdit`

     Base class for a filter that edits some registers within an :cpp:class:`IfContentPart`
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a class **template**, parameterized by exactly the same ``K``/``V``/``KeyHash``/
        ``KeyEqual`` as the :cpp:class:`IfContentPart` it edits. It has to be: the `Python`_
        binding layer edits ``IfContentPart<py::object, py::object, ...>`` (every key/value that
        crosses the `pybind11`_ boundary is a ``py::object``), while a plain C++ caller normally
        wants ``IfContentPart<std::string, std::string>`` -- which is what ``K``/``V`` default to
        here, so ``BaseRegEdit<>`` reads exactly like the non-templated version this replaced
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class BaseRegEdit: public BaseIniGraphPartEdit {
        public:

            /**
             * @brief The type of part this edits the registers of
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The ranges of valid `KVP`_ order indices -- ``long long`` because that is the type
             :cpp:class:`IfContentPart` keys its order indices by
             @endrst
             */
            using OrderRanges = Ranges<long long>;

            /**
             * @brief
             @rst
             The shape :cpp:class:`IfContentPart`'s own range-restricted methods
             (:cpp:func:`IfContentPart::removeKeys`, :cpp:func:`IfContentPart::remapKeys`,
             :cpp:func:`IfContentPart::replaceVals`) take -- see \ref toRangeSpec
             @endrst
             */
            using RangeSpec = typename ContentPart::RangeSpec;

            /**
             * @brief
             @rst
             Edits the registers for the current :cpp:class:`IfContentPart` with state info from
             'ini' :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The base implementation forwards straight to \ref edit and **ignores 'ini'
                entirely**, exactly as the pure-Python original does -- see
                :cpp:func:`BaseIniGraphEdit::editFromIni`'s own note
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited
             * @param ini
             @rst
             The associated ``.ini`` file -- a **non-owning, nullable** pointer, where
             ``nullptr`` stands in for "no ``.ini`` file is available" :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param modType
             @rst
             The type of mod to fix -- also a **non-owning, nullable** pointer, for the same
             reason ``ini`` is one: none of this class's own subclasses read it, and the
             `pybind11`_ layer has no C++ :cpp:class:`ModType` to hand over (the `Python`_ API's
             ``ModType`` is still a pure-`Python`_ class of its own) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partRanges
             @rst
             The ranges that indicate the valid order indices to process for 'part' -- a
             **non-owning, nullable** pointer, where ``nullptr`` stands in for the pure-Python
             original's ``partRanges = None`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             *
             * @return The same part that was passed in, after editing
             */
            virtual ContentPart& editFromIni(ContentPart& part, const std::string& sectionName, IniFile* ini = nullptr,
                                             const ModType* modType = nullptr, const std::string& modName = "",
                                             const OrderRanges* partRanges = nullptr);

            /**
             * @brief
             @rst
             Edits the registers for the current :cpp:class:`IfContentPart`. No-op by default
             (returns 'part' untouched), matching the pure-Python original's ``pass``
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited
             * @param modType The type of mod to fix, or ``nullptr``. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            virtual ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                                      const std::string& modName = "", const OrderRanges* partRanges = nullptr);

        protected:

            /**
             * @brief
             @rst
             Converts the nullable :cpp:class:`Ranges` pointer every ``edit`` takes into the
             ``std::optional``-wrapped list :cpp:class:`IfContentPart`'s own range-restricted
             methods expect, with ``nullptr`` becoming ``std::nullopt`` (i.e. no restriction)
             @endrst
             *
             * @param partRanges The ranges to convert, or ``nullptr``
             *
             * @return The equivalent range restriction
             */
            static std::optional<RangeSpec> toRangeSpec(const OrderRanges* partRanges);
    };
}

#include "BaseRegEdit.tpp"

#endif
