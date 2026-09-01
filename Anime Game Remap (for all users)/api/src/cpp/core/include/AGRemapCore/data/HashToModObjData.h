#ifndef AGRemapCore_HashToModObjData_H
#define AGRemapCore_HashToModObjData_H

#include <map>
#include <string>
#include <unordered_map>
#include <utility>


namespace AGRemapCore {
    namespace Data {

        /**
         * @brief
         @rst
         A mod object -- the ``(component name, object name)`` pair every mod object in this
         codebase is identified by
         @endrst
         */
        using ModObjKey = std::pair<std::string, std::string>;

        /**
         * @brief
         @rst
         The mod objects one ``ib``-suffixed hash type can resolve to, keyed by the last two index
         columns of the :cpp:class:`Indices` row a ``match_first_index`` value matched
         @endrst
         */
        using IndexKeyToModObj = std::map<ModObjKey, ModObjKey>;

        /**
         * @brief
         @rst
         Splits one of :cpp:func:`getHashDataRows`'s innermost keys into the mod object it names
         :raw-html:`<br />` :raw-html:`<br />`

         A key is either ``objName`` or ``compName;objName``. The first form -- currently the only
         one any row uses -- names an object with no component, so it becomes ``("", objName)``.
         The second exists for mods whose model is split across several components (Yelan's skin,
         the WuWa mods), and becomes ``(compName, objName)``
         @endrst
         *
         * @param key One of the innermost keys of the hash data table
         *
         * @return The mod object that key names
         */
        ModObjKey parseModObjKey(const std::string& key);

        /**
         * @brief
         @rst
         The mod objects a ``hash`` `KVP`_ identifies on its own -- every hash type
         :cpp:func:`getHashDataRows` ships **except** the ``ib``-suffixed ones
         :raw-html:`<br />` :raw-html:`<br />`

         Keys are the innermost keys of the hash data table (``blend_vb``, ``tex_head_diffuse``,
         ...); values are what :cpp:func:`parseModObjKey` makes of them. This is the default
         :cpp:member:`GIMISectionClassifier::hashKeyOnlyToModObj`

         :raw-html:`<br />`

         .. note::
            Derived from :cpp:func:`getHashDataRows` on first use, not transcribed -- so a hash
            type added to ``HashData.cpp`` shows up here with no second edit. That is the opposite
            of how ``HashData.cpp``/``IndexData.cpp`` themselves are maintained, and deliberately
            so: this table holds no information of its own, only a naming convention applied to
            theirs
         @endrst
         */
        const std::unordered_map<std::string, ModObjKey>& getHashKeyOnlyToModObj();

        /**
         * @brief
         @rst
         The mod objects a ``hash`` `KVP`_ can only narrow down, needing a ``match_first_index``
         `KVP`_ to tell them apart -- the ``ib``-suffixed hash types, and only those
         :raw-html:`<br />` :raw-html:`<br />`

         Outer keys are the innermost keys of the hash data table whose object name ends in ``ib``;
         the inner maps are keyed by the ``(component, object)`` pair the matched
         :cpp:class:`Indices` row ends with -- which is already a mod object, so it maps to itself.
         This is the default :cpp:member:`GIMISectionClassifier::indexKeyToModObj`

         :raw-html:`<br />`

         .. note::
            Derived from :cpp:func:`getHashDataRows` and :cpp:func:`getIndexDataRows` on first use
            -- see :cpp:func:`getHashKeyOnlyToModObj`'s own note
         @endrst
         */
        const std::unordered_map<std::string, IndexKeyToModObj>& getIndexKeyToModObj();

    }
}

#endif
