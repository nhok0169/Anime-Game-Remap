#ifndef AGRemapCore_GIMISectionClassifier_H
#define AGRemapCore_GIMISectionClassifier_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/data/HashToModObjData.h"
#include "AGRemapCore/model/IniGraphGroup.h"
#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/iftemplate/IfContentPartColour.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniParsers/IniParseContext.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Classifies a `section`_ into the mod object(s) it belongs to, by its ``hash`` and
     ``match_first_index`` `KVP`_ values :raw-html:`<br />` :raw-html:`<br />`

     This is the default :cpp:type:`GIMIParser::ObjTargetFunc` whenever a ``.ini`` file was
     actually classified as some mod type and the parser is tracking `KVPs`_ -- ie. the "look at
     what the `section`_ *does*" half of the two ways :cpp:class:`GIMIParser` finds its mod
     objects. The other half (:cpp:func:`GIMIParser::classifyByTextureOverrideName`, "look at what
     the `section`_ is *called*") is the fallback used when there's nothing to match against.

     :raw-html:`<br />`

     .. note::
        A ``hash`` alone identifies most mod objects. The ones it can't (several objects of one
        model sharing a hash, told apart only by which vertex range they draw) are resolved by
        additionally matching the ``match_first_index`` `KVPs`_ that follow that ``hash`` within
        the same :cpp:class:`IfContentPart` -- which is why #classify walks the ``hash`` values in
        index order and windows each one's ``match_first_index`` lookup to the span before the
        *next* ``hash``

     .. note::
        Two deliberate divergences from the pure-Python original:

        * ``hashNonVersionVals``/``indexNonVersionVals`` are stored here already normalized into
          the positional ``std::vector<std::optional<K>>`` shape
          :cpp:func:`ModMappedAssets::getKey` takes. The original stores the caller's raw
          bare-value/list/dict form and normalizes it in its ``hashes``/``indices`` setters only --
          so re-assigning ``hashNonVersionVals`` *after* construction leaves the un-normalized
          value in ``_currentHashNonVersionVals``. That's harmless there (the `Python`_
          ``getKey`` re-normalizes whatever it's handed), but there is no reason to reproduce it
        * ``hashes``/``indices`` are **nullable borrowed pointers**, not owned values -- they
          belong to the :cpp:class:`ModType` the ``.ini`` file was classified as, and #classify
          simply finds nothing when either is missing, rather than raising
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class GIMISectionClassifier {
        public:

            /**
             * @copydoc IniGraphGroup::ModObj
             */
            using ModObj = std::pair<std::string, std::string>;

            /**
             * @copydoc IniGraphGroup::ModObjHash
             */
            using ModObjHash = typename IniGraphGroup<K, V, KeyHash, KeyEqual>::ModObjHash;

            /**
             * @brief The type of `section`_ this classifies
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The `KVP`_ state of the :cpp:class:`IfContentPart` being classified
             */
            using Colouring = IfContentPartColouring<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief The kind of asset table #hashes/#indices are
             */
            using Assets = ModMappedAssets<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The ``(component, mod object)`` half of an ``match_first_index`` lookup key -- the last
             two index columns of an :cpp:class:`Indices` row, as a pair
             @endrst
             */
            using IndexKey = std::pair<K, K>;

            /**
             * @brief Hashes an #IndexKey, so it can key an ``std::unordered_map``
             */
            struct IndexKeyHash {
                std::size_t operator()(const IndexKey& key) const {
                    std::size_t firstHash = KeyHash{}(key.first);
                    std::size_t secondHash = KeyHash{}(key.second);
                    return firstHash ^ (secondHash + 0x9e3779b9 + (firstHash << 6) + (firstHash >> 2));
                }
            };

            /**
             * @brief Compares two #IndexKey\\s, so they can key an ``std::unordered_map``
             */
            struct IndexKeyEqual {
                bool operator()(const IndexKey& left, const IndexKey& right) const {
                    return KeyEqual{}(left.first, right.first) && KeyEqual{}(left.second, right.second);
                }
            };

            /**
             * @brief
             @rst
             The mod objects reachable from one ``hash``, keyed by the ``match_first_index`` value
             that picks between them
             @endrst
             */
            using IndexModObjs = std::unordered_map<IndexKey, ModObj, IndexKeyHash, IndexKeyEqual>;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs, for the same reason
             :cpp:class:`IfTemplateRunConfig` exists: ``K`` is not ``std::string`` for every
             instantiation, so the ``hash``/``match_first_index`` `KVP`_ keys can't be spelled as
             literals here
             @endrst
             */
            struct ClassifierConfig {
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
             * @brief The default #ClassifierConfig for a plain, ``std::string``-keyed C++ caller
             */
            static ClassifierConfig defaultConfig();

            /**
             * @brief Constructs a new classifier
             *
             * @param hashKeyOnlyToModObj
             @rst
             The mod objects identified by a ``hash`` value alone. The keys are the *last* index
             column of the matched :cpp:class:`Hashes` row (the type of hash, eg. ``blend_vb``)
             @endrst
             * @param hashes The ``hash`` assets to resolve a ``hash`` `KVP`_ value against. **Nullable** -- see this class's own note
             * @param indexKeyToModObj
             @rst
             The mod objects that need a ``match_first_index`` value too. The outer keys are the
             same "type of hash" as ``hashKeyOnlyToModObj``'s; the inner keys are the last two index
             columns of the matched :cpp:class:`Indices` row. **Default**: empty
             @endrst
             * @param indices The ``match_first_index`` assets. **Nullable**, same as 'hashes'. **Default**: ``nullptr``
             * @param version The version of the .ini file, or ``std::nullopt`` for "the latest". **Default**: ``std::nullopt``
             * @param config The .ini-domain customization points to use. **Default**: #defaultConfig
             */
            explicit GIMISectionClassifier(std::unordered_map<K, ModObj, KeyHash, KeyEqual> hashKeyOnlyToModObj,
                                            Assets* hashes,
                                            std::unordered_map<K, IndexModObjs, KeyHash, KeyEqual> indexKeyToModObj = {},
                                            Assets* indices = nullptr,
                                            std::optional<Version> version = std::nullopt,
                                            ClassifierConfig config = defaultConfig());

            /**
             * @brief The mod objects identified by a ``hash`` value alone -- see the constructor
             */
            std::unordered_map<K, ModObj, KeyHash, KeyEqual> hashKeyOnlyToModObj;

            /**
             * @brief The mod objects that need a ``match_first_index`` value too -- see the constructor
             */
            std::unordered_map<K, IndexModObjs, KeyHash, KeyEqual> indexKeyToModObj;

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
             * @brief
             @rst
             The already-normalized non-version filter used when searching #hashes -- see this
             class's own note on how this differs from the pure-Python original
             @endrst
             */
            const std::vector<std::optional<K>>& hashNonVersionVals() const;

            /**
             * @copydoc hashNonVersionVals() const
             */
            void setHashNonVersionVals(std::vector<std::optional<K>> newVals);

            /**
             * @brief The already-normalized non-version filter used when searching #indices
             */
            const std::vector<std::optional<K>>& indexNonVersionVals() const;

            /**
             * @copydoc indexNonVersionVals() const
             */
            void setIndexNonVersionVals(std::vector<std::optional<K>> newVals);

            /**
             * @brief The .ini-domain customization points this instance uses
             */
            const ClassifierConfig& config() const;

            /**
             * @brief
             @rst
             Classifies which mod objects a particular :cpp:class:`IfContentPart` belongs to
             @endrst
             *
             * @param sectionName The name of the `section`_ the part belongs to. Unused -- kept because the pure-Python original's signature has it
             * @param section The `section`_ the part belongs to. Unused, same reason
             * @param partKeys The current state of the `KVPs`_ for the part
             *
             * @return The classified mod objects, in first-found order
             */
            std::vector<ModObj> classify(const std::string& sectionName, Section* section, const Colouring& partKeys) const;

            /**
             * @brief
             @rst
             Makes a ``K`` out of one of the hash data table's own ``std::string`` keys -- the one
             thing #buildDefaultHashKeyOnlyToModObj can't do for itself, since ``K`` is not
             ``std::string`` for every instantiation (the same reason #ClassifierConfig exists)
             @endrst
             */
            using KeyMaker = std::function<K(const std::string&)>;

            /**
             * @brief
             @rst
             The default #hashKeyOnlyToModObj -- :cpp:func:`Data::getHashKeyOnlyToModObj`, with its
             keys run through 'makeKey'
             @endrst
             *
             * @param makeKey How to turn a hash data key into a ``K``
             */
            static std::unordered_map<K, ModObj, KeyHash, KeyEqual> buildDefaultHashKeyOnlyToModObj(const KeyMaker& makeKey);

            /**
             * @brief
             @rst
             The default #indexKeyToModObj -- :cpp:func:`Data::getIndexKeyToModObj`, with its outer
             keys and its inner #IndexKey\\s run through 'makeKey'
             @endrst
             *
             * @param makeKey How to turn a hash/index data key into a ``K``
             */
            static std::unordered_map<K, IndexModObjs, KeyHash, KeyEqual> buildDefaultIndexKeyToModObj(const KeyMaker& makeKey);

            /**
             * @brief
             @rst
             Builds the default classifier -- one that knows every asset the mod type ships with,
             mapped by the defaults every mod type shares
             :raw-html:`<br />` :raw-html:`<br />`

             Those mappings cover every hash type the *whole* hash data table ships, not just the
             ones this mod type owns. That is deliberate and harmless: #classify only ever looks up
             a key #hashes itself just returned, so a row this mod type doesn't have is never
             reached

             :raw-html:`<br />`

             .. note::
                Only an instantiation whose ``K`` can be built straight from a ``std::string`` gets
                those defaults. Any other (notably the `pybind11`_ layer's ``py::object``) starts
                empty here and fills them in itself -- exactly as it already has to for
                #defaultConfig
             @endrst
             *
             * @param hashes The mod type's ``hash`` assets
             * @param indices The mod type's ``match_first_index`` assets
             * @param version The version of the .ini file. **Default**: ``std::nullopt``
             * @param config The .ini-domain customization points to use. **Default**: #defaultConfig
             */
            static std::unique_ptr<GIMISectionClassifier<K, V, KeyHash, KeyEqual>> buildDefaultClassifier(
                Assets* hashes, Assets* indices, std::optional<Version> version = std::nullopt,
                ClassifierConfig config = defaultConfig());

            /**
             * @brief #buildDefaultClassifier, reading the mod type and version out of a .ini file
             *
             * @param ctx The .ini file to build the classifier for
             * @param config The .ini-domain customization points to use. **Default**: #defaultConfig
             */
            static std::unique_ptr<GIMISectionClassifier<K, V, KeyHash, KeyEqual>> buildDefaultClassifierFromIni(
                IniParseContext<K, V, KeyHash, KeyEqual>& ctx, ClassifierConfig config = defaultConfig());

        private:
            Assets* hashes_;
            Assets* indices_;
            std::vector<std::optional<K>> hashNonVersionVals_;
            std::vector<std::optional<K>> indexNonVersionVals_;
            ClassifierConfig config_;

            // 'startInd'/'endInd' are std::nullopt for "unbounded on that side". The pure-Python
            // original's own _indFilter compares against a raw None instead, which raises a
            // TypeError rather than meaning anything -- reachable there only for a KVP whose value
            // was carried over from an earlier IfContentPart and so has no index of its own.
            static bool indFilter(const std::optional<long long>& ind, const std::optional<long long>& startInd,
                                   const std::optional<long long>& endInd);
    };
}

#include "GIMISectionClassifier.tpp"

#endif
