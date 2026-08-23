#ifndef AGRemapCore_SectionIterData_H
#define AGRemapCore_SectionIterData_H

#include <functional>
#include <optional>
#include <string>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A class that contains the needed data for each iteration after calling
     `IniSectionGraph::iterSectsByContentPart` -- the C++ port of ``SectionIterData.py``'s
     ``SectionIterData`` class :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Owns nothing -- every member is a non-owning reference into data owned elsewhere (the
        `IniSectionGraph`/`IfTemplate` this iteration data came from), matching every other class
        in this port's own "who owns what" convention. Simplifies
        `IfContentPartColouring`\\<K, V, KeyHash, KeyEqual, ValueHash, ValueEqual\\>'s independent
        ``ValueHash``/``ValueEqual`` template parameters down to reusing ``KeyHash``/``KeyEqual``
        for both -- matching how the `Python`_ binding instantiates it anyway (``PyObjectHash``/
        ``PyObjectEqual`` for both K and V), since nothing in this subsystem needs them to differ.
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class SectionIterData {
        public:
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief Constructs a new instance
             *
             * @param sectionName The name of the `section`_
             * @param section The corresponding `section`_ the part resides in
             * @param part The corresponding part
             * @param state The current state of the `section`_
             * @param colouring The current `KVP`_ states of the part's :cpp:class:`IfContentPart`. **Default**: ``nullptr``
             */
            explicit SectionIterData(std::string sectionName, Section* section, ContentPart* part, int state, Colouring* colouring = nullptr);

            /**
             * @brief The name of the `section`_
             */
            std::string sectionName;

            /**
             * @brief The corresponding `section`_ the part resides in
             */
            Section* section;

            /**
             * @brief The corresponding part
             */
            ContentPart* part;

            /**
             * @brief The current state of the `section`_
             */
            int state;

            /**
             * @brief The current `KVP`_ states of the part's :cpp:class:`IfContentPart`
             */
            Colouring* colouring;
    };

    /**
     * @brief
     @rst
     A class that contains the needed data for each iteration after calling
     `IniSectionGraph::iterByQuery` -- the C++ port of ``SectionIterData.py``'s
     ``SectionIterQueryData`` class :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Owns nothing -- see #SectionIterData's own top-level note, which applies identically here.
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyHash``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``, same meaning as :cpp:class:`IfContentPart`'s own ``KeyEqual``. Defaults to ``std::equal_to<K>``
     */
    template <typename K, typename V, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class SectionIterQueryData {
        public:
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief Constructs a new instance
             *
             * @param part The part retrieved
             * @param query
             @rst
             The corresponding logical query that the part resides in -- always a real
             :cpp:class:`Z3Predicate`, including for a part with no enclosing ``if``/``elif``/
             ``else`` predicate at all, which reports the literal ``true`` predicate
             @endrst
             * @param sectionName The name of the `section`_ the part resides in
             * @param section The corresponding `section`_ the part resides in
             * @param rootSectionName The name of the root `section`_ the part resides in
             * @param rootSection The corresponding root `section`_ the part resides in
             * @param state The current state the `section`_ is in
             * @param colouring The current `KVP`_ states of the part's :cpp:class:`IfContentPart`. **Default**: ``nullptr``
             */
            explicit SectionIterQueryData(ContentPart* part, Z3Predicate query, std::string sectionName, Section* section,
                                           std::string rootSectionName, Section* rootSection, int state, Colouring* colouring = nullptr);

            /**
             * @brief The part retrieved
             */
            ContentPart* part;

            /**
             * @brief The corresponding logical query that the part resides in
             */
            Z3Predicate query;

            /**
             * @brief The name of the `section`_ the part resides in
             */
            std::string sectionName;

            /**
             * @brief The corresponding `section`_ the part resides in
             */
            Section* section;

            /**
             * @brief The name of the root `section`_ the part resides in
             */
            std::string rootSectionName;

            /**
             * @brief The corresponding root `section`_ the part resides in
             */
            Section* rootSection;

            /**
             * @brief The current state the `section`_ is in
             */
            int state;

            /**
             * @brief The current `KVP`_ states of the part's :cpp:class:`IfContentPart`
             */
            Colouring* colouring;
    };

}

#include "SectionIterData.tpp"

#endif
