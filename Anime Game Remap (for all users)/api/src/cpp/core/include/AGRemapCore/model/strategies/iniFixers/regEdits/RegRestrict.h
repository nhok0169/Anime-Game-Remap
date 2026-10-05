#ifndef AGRemapCore_RegRestrict_H
#define AGRemapCore_RegRestrict_H

// ##### Credits

// ===== Anime Game Remap (AG Remap) =====
// Authors: Albert Gold#2696, NK#1321
//
// if you used it to remap your mods pls give credit for "Albert Gold#2696" and "Nhok0169"
// Special Thanks:
//   nguen#2011 (for support)
//   SilentNightSound#7430 (for internal knowdege so wrote the blendCorrection code)
//   HazrateGolabi#1364 (for being awesome, and improving the code)

// ##### EndCredits

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    class ModType;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Restricts the registers an :cpp:class:`IfContentPart` binds to the ones its TARGET reads, and to
     one binding each :raw-html:`<br />` :raw-html:`<br />`

     A remapped `section`_ inherits its registers from the MOD's `section`_, and a mod binds whatever
     its SOURCE reads. Against a different target that surplus is not harmless: the target's shader
     reads the extra register as something else, where the target's own mod leaves it unbound so the
     GAME's texture serves it. And a register bound twice keeps only the LAST binding, silently -- so a
     fix that moves one texture onto a register the mod already fills loses the texture it moved.

     .. code-block:: ini

        ; restricted to ps-t0, ps-t1 and ps-t2, over the keys that are ps-t registers
        ps-t1 = ResourceDiffuse
        ps-t0 = ResourceNormalMap
        ps-t2 = ResourceLightMap
        ps-t2 = ResourceMetalMap       ; removed: ps-t2 is already bound above
        ps-t3 = ResourceShadowRamp     ; removed: the target does not read ps-t3
        ib = ResourceIb                ; untouched: not a key this edit governs

     Only the keys \\ref keyFilter accepts are governed; every other key is left exactly as it is.

     .. note::
        The unit is ONE part, not a `section`_. A merged mod binds ``ps-t2`` once in each branch of an
        ``if`` / ``else if`` chain, and each branch is its own part: those are separate paths through
        the `section`_, not one register bound twice, and restricting per `section`_ would keep only
        the first branch's
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegRestrict: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
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

            /**
             * @brief Decides whether this edit governs a key at all
             */
            using KeyFilter = std::function<bool(const K&)>;

            /**
             * @brief
             @rst
             The governed keys a part may keep, or ``std::nullopt`` to keep every governed key (and
             only remove repeated bindings) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            std::optional<std::vector<K>> allowedKeys;

            /**
             * @brief
             @rst
             Which keys this edit governs -- empty governs every key :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty
             @endrst
             */
            KeyFilter keyFilter;

            /**
             * @brief
             @rst
             Whether a governed key bound more than once in a part keeps only its FIRST binding
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             */
            bool keepFirstOnly;

            /**
             * @brief Constructs a new register-restricting edit
             *
             * @param allowedKeys The governed keys a part may keep, or ``std::nullopt`` for all of them. **Default**: ``std::nullopt``
             * @param keyFilter Which keys this edit governs, or empty for every key. **Default**: empty
             * @param keepFirstOnly Whether a repeated governed key keeps only its first binding. **Default**: ``true``
             */
            explicit RegRestrict(std::optional<std::vector<K>> allowedKeys = std::nullopt, KeyFilter keyFilter = {},
                                 bool keepFirstOnly = true);

            /**
             * @brief
             @rst
             Removes every governed key of 'part' that is not in \\ref allowedKeys, and -- with
             \\ref keepFirstOnly -- every binding of a governed key after its first
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited. Unused by this edit
             * @param modType The type of mod to fix. Unused by this edit. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. Unused by this edit. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                              const std::string& modName = "", const OrderRanges* partRanges = nullptr) override;
    };
}

#include "RegRestrict.tpp"

#endif
