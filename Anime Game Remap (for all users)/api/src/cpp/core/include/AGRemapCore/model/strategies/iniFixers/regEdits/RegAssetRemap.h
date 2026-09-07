#ifndef AGRemapCore_RegAssetRemap_H
#define AGRemapCore_RegAssetRemap_H

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/BaseRegEdit.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseRegEdit`

     Rewrites a register's value from **the mod being fixed from** to **the mod being fixed to**, by
     looking the old value up in an asset table :raw-html:`<br />` :raw-html:`<br />`

     The remapped `sections`_ a fix emits are copies of the original mod's, so every asset value in
     them still names the *original* mod's asset. A ``hash`` still naming the source model means the
     remapped `section`_ never triggers on the target at all; a ``match_first_index`` still naming
     the source's vertex range means it triggers on the wrong part of the target's model. Both are
     silent -- the ``.ini`` file is perfectly well-formed either way :raw-html:`<br />`
     :raw-html:`<br />`

     This is the direct equivalent of the pure-Python ``BaseIniFixer._getHashReplacement`` /
     ``_getIndexReplacement`` pair, which are the same call
     (``assetRepo.replace(asset, version, toAssets = modName)``) against two different tables --
     which is exactly why this class takes the table per register rather than knowing about hashes
     or indices itself :raw-html:`<br />` :raw-html:`<br />`

     .. code-block:: cpp

        // hashes AND indices -- the usual case
        RegAssetRemap<> remap({{IniKeywords::Hash, {hashes, IniKeywords::HashNotFound}},
                                {IniKeywords::MatchFirstIndex, {indices, IniKeywords::IndexNotFound}}},
                               toModName, fromVersion, toVersion);

     .. warning::
        **This is the right tool for a** ``hash`` **and the wrong one for a**
        ``match_first_index``. :cpp:func:`ModMappedAssets::replace` is *reverse-then-forward*: it
        looks the old value up to find which row owns it, then forwards that row's key onto the
        target. For a ``hash`` that reverse step is the point -- the value is what tells you which
        *kind* of hash it is (``ib`` / ``blend_vb`` / ``position_vb``), which a mod object cannot
        tell you on its own, and hashes are unique so the lookup is unambiguous
        :raw-html:`<br />` :raw-html:`<br />`

        An index has neither property. ``0`` is **every** character's head index, so the reverse
        lookup is ambiguous and simply fails -- writing \ref AssetSpec::notFoundVal into a field
        that has to be a number. And it is unnecessary: an index's meaning *is* its mod object, and
        the fixer already knows which object's graph it is editing. Use a **forward** lookup
        instead -- :cpp:func:`ModMappedAssets::get`\ ``({toModName, component, object})`` -- and
        write the result with :cpp:class:`RegNewVals` (whose ``addNewKVPs = false`` default leaves
        a `section`_ that has no ``match_first_index`` alone). That makes the index edit **per mod
        object**, where the hash edit can stay shared across all of them

     .. note::
        Not every mod object needs both. Raiden's 6.1 fix remaps only the ``hash`` of her *blend*
        `section`_ -- her drawn objects keep the source model's own ``hash``/``match_first_index``,
        because the boss draws the same geometry and only the blend weights differ. Which registers
        need remapping is a per-mod fact, which is why this class remaps exactly the ones it is
        given and never infers them

     .. note::
        A value with no mapping is written as its register's \\ref AssetSpec::notFoundVal (the
        pure-Python original's ``HashNotFound``/``IndexNotFound`` sentinels), or left untouched when
        that is ``std::nullopt``. Leaving it is the quieter option but the more dangerous one: an
        unmapped ``hash`` left alone is indistinguishable from one that was correctly mapped to
        itself, so prefer the sentinel unless a caller genuinely wants a pass-through
     @endrst
     *
     * @tparam K The type of the keys stored in the parts this edits
     * @tparam V The type of the values stored in the parts this edits
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RegAssetRemap: public BaseRegEdit<K, V, KeyHash, KeyEqual> {
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
             * @brief The kind of asset table a register is looked up in
             */
            using Assets = ModMappedAssets<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief How one register's values are remapped
             */
            struct AssetSpec {
                /**
                 * @brief The table to look the old value up in -- **borrowed**, and a ``nullptr`` remaps nothing
                 */
                Assets* assets = nullptr;

                /**
                 * @brief
                 @rst
                 What to write when the old value has no mapping -- ``std::nullopt`` leaves the
                 value untouched. See this class's own note on why the sentinel is usually right
                 @endrst
                 */
                std::optional<V> notFoundVal;

                AssetSpec() = default;

                /**
                 * @brief Constructs a spec for one register
                 *
                 * @param assets The table to look the old value up in
                 * @param notFoundVal What to write when there is no mapping. **Default**: leave the value alone
                 */
                explicit AssetSpec(Assets* assets, std::optional<V> notFoundVal = std::nullopt):
                    assets(assets), notFoundVal(std::move(notFoundVal)) {}
            };

            /**
             * @brief Which registers to remap, and against which table -- applied in the order given
             */
            std::vector<std::pair<K, AssetSpec>> assets;

            /**
             * @brief The name of the mod being fixed **to** -- what every value is remapped onto
             */
            std::string toModName;

            /**
             * @brief
             @rst
             The name of the mod being fixed **from**, used to constrain the reverse lookup
             :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                **Leaving this empty is a real bug risk, not a convenience.**
                :cpp:func:`ModMappedAssets::replace` starts by reverse-looking-up the value to find
                which row owns it, and when the source and the target **share** that value -- which
                every CN pair does for at least one asset -- the row it lands on is whichever the
                map happens to yield. Land on the *target*'s row and the forward step then asks the
                remap graph for ``target -> target``, which is not an edge, so the whole remap
                returns nothing and the register is written as ``HashNotFound``
                :raw-html:`<br />` :raw-html:`<br />`

                That is exactly what happened to Rosaria's face hash (``2abd61ee``, shared with
                RosariaCN) while Amber's and Mona's identical situations worked purely by map
                ordering -- a silent, character-dependent failure. Filling this in makes the reverse
                lookup deterministic
             @endrst
             */
            std::string fromModName;

            /**
             * @brief The version of the ``.ini`` file being fixed, or ``std::nullopt`` for "the latest"
             */
            std::optional<Version> fromVersion;

            /**
             * @brief The version being fixed to, or ``std::nullopt`` for "the latest"
             */
            std::optional<Version> toVersion;

            /**
             * @brief Constructs a new asset-remapping register edit
             *
             * @param assets Which registers to remap, and against which table
             * @param toModName The name of the mod being fixed to
             * @param fromModName The name of the mod being fixed from -- see \ref fromModName for why leaving it empty is risky. **Default**: ``""``
             * @param fromVersion The version of the .ini file being fixed. **Default**: ``std::nullopt``
             * @param toVersion The version being fixed to. **Default**: ``std::nullopt``
             */
            explicit RegAssetRemap(std::vector<std::pair<K, AssetSpec>> assets = {}, std::string toModName = "",
                                    std::string fromModName = "",
                                    std::optional<Version> fromVersion = std::nullopt,
                                    std::optional<Version> toVersion = std::nullopt);

            /**
             * @brief
             @rst
             Remaps every register named in \\ref assets onto \\ref toModName's equivalent
             @endrst
             *
             * @param part The part of the `IfTemplate` being edited, modified in place
             * @param sectionName The name of the `section`_ being edited. Unused by this edit
             * @param modType The type of mod to fix. Unused -- the target is \\ref toModName. **Default**: ``nullptr``
             * @param modName The name of the mod to fix to. Unused, same reason. **Default**: ``""``
             * @param partRanges The valid order indices to process for 'part', or ``nullptr`` for all of them. **Default**: ``nullptr``
             *
             * @return The same part that was passed in, after editing
             */
            ContentPart& edit(ContentPart& part, const std::string& sectionName, const ModType* modType = nullptr,
                               const std::string& modName = "", const OrderRanges* partRanges = nullptr) override;
    };
}

#include "RegAssetRemap.tpp"

#endif
