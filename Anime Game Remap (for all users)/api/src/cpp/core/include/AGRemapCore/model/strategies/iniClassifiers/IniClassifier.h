#ifndef AGRemapCore_IniClassifier_H
#define AGRemapCore_IniClassifier_H

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/model/strategies/ModTypeIdData.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"
#include "AGRemapCore/tools/dfa/BaseDFA.h"
#include "AGRemapCore/tools/tries/BaseAhoCorasickDFA.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniClassifier`

     Class to help classify the type of mod given the mod's .ini files
     @endrst
     */
    class IniClassifier: public BaseIniClassifier {
        public:

            /**
             * @brief
             @rst
             Constructs the classifier, setting #stateDFA to its initial state:

             .. code-block::

                start --- game:<GameTypeId::WuWa> --> isWuwa
             @endrst
             *
             * @param checkHasTextureOverride See #checkHasTextureOverride
             */
            explicit IniClassifier(bool checkHasTextureOverride = true);

            IniClassifyStats classify(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            /**
             * @brief
             @rst
             Resets #modTypeIdDistribution and #savedWuWaModTypeIds (transient, per-classification
             state that must not leak into a later, unrelated call), reads through 'iniTxt' line by
             line via :cpp:func:`readLine`, then adds whichever :cpp:enum:`ModTypeId`\\s ended up
             with the highest count in #modTypeIdDistribution to the result's ``modType`` -- ties
             are all included, sorted in ascending order
             @endrst
             */
            IniClassifyStats classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            bool checkIsMod(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            /**
             * @brief
             @rst
             Same as :cpp:func:`classify`, except it resets #modTypeIdDistribution/
             #savedWuWaModTypeIds and reads through 'iniTxt' line by line via :cpp:func:`readLine`
             the exact same way, but returns ``true`` immediately, without reading any further
             lines, the moment ``stats.isMod`` first becomes ``true`` -- cheaper than
             :cpp:func:`classify` when only this yes/no answer is needed, since it never runs the
             tie-breaking #modTypeIdDistribution pass and can stop reading partway through the file
             @endrst
             */
            bool checkIsMod(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            void checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            /**
             * @brief
             @rst
             Same as :cpp:func:`checkIsMod`, except it also tracks ``stats.isFixed`` and sets
             '*isFixed'/'*isMod' from the resultant ``stats.isFixed``/``stats.isMod`` once it stops
             reading -- it stops (without reading any further lines) the moment ``stats.isMod``
             **and** ``stats.isFixed`` have both become ``true``, rather than as soon as
             ``stats.isMod`` alone becomes ``true``
             @endrst
             *
             * @param isFixed Set to whether the .ini file is fixed
             * @param isMod Set to whether the .ini file belongs to a mod
             */
            void checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId = std::nullopt) override;

            /**
             * @brief
             @rst
             Registers a GI mod type into #stateDFA :raw-html:`<br />` :raw-html:`<br />`

             Fails (returns ``false``) without registering anything if ``modType.modTypeId`` is
             already registered in #modTypes, or if ``modType.gameTypeId`` isn't
             :cpp:enumerator:`GameTypeId::GI`
             @endrst
             *
             * @param modType The mod type to register
             * @param hashes The hashes that identify 'modType'
             * @param sectionKeywords The `section`_ keywords that identify 'modType'
             *
             * @return Whether 'modType' was newly registered
             */
            virtual bool addGIModType(const ModTypeIdData& modType, const std::unordered_set<std::string>& hashes, const std::unordered_set<std::string>& sectionKeywords);

            /**
             * @brief
             @rst
             Registers a WuWa mod type into #stateDFA :raw-html:`<br />` :raw-html:`<br />`

             Fails (returns ``false``) without registering anything if ``modType.modTypeId`` is
             already registered in #modTypes, or if ``modType.gameTypeId`` isn't
             :cpp:enumerator:`GameTypeId::WuWa`
             @endrst
             *
             * @param modType The mod type to register
             * @param hashes The hashes that identify 'modType'
             *
             * @return Whether 'modType' was newly registered
             */
            virtual bool addWuWaModType(const ModTypeIdData& modType, const std::unordered_set<std::string>& hashes);

            /**
             * @brief Retrieves the registered :cpp:class:`ModTypeIdData` for a :cpp:enum:`ModTypeId`
             *
             * @param modTypeId The id for the :cpp:enum:`ModTypeId` to retrieve the :cpp:class:`ModTypeIdData` for
             *
             * @throw std::out_of_range Thrown if 'modTypeId' is not registered in #modTypes
             *
             * @return The corresponding :cpp:class:`ModTypeIdData`
             */
            virtual ModTypeIdData getModType(int modTypeId);

            /**
             * @brief
             @rst
             Clears the state of the classifier, resetting it back to the state it was in when no
             mod types were registered yet -- i.e. #stateDFA back to its initial state (see the
             constructor's doc comment) and #hashGameTypeIds, #modTypes, #sectionKeywordsDFA,
             #keywordGameTypeIds, #modTypeIdDistribution, #savedWuWaModTypeIds, and
             #acceptModTypeIds all emptied
             @endrst
             */
            void clear() override;

        protected:

            /**
             * @brief
             @rst
             Whether :cpp:func:`readSectionName` should require a `section`_ name to start with
             ``TextureOverride`` before doing anything else with it :raw-html:`<br />` :raw-html:`<br />`

             Set once at construction; unaffected by :cpp:func:`clear`
             @endrst
             */
            bool checkHasTextureOverride;

            /**
             * @brief The `DFA`_ used to keep track of the states seen so far while reading a .ini file
             */
            BaseDFA<std::string, std::string> stateDFA;

            /**
             * @brief
             @rst
             Identifies which :cpp:enum:`GameTypeId`\\s a hash belongs to :raw-html:`<br />` :raw-html:`<br />`

             The keys are hash values and the values are the ids for the :cpp:enum:`GameTypeId`\\s
             that hash identifies (normally just a single id, but a set to account for the
             theoretical case where the same hash is registered for more than one game type)
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                The values are plain ``int``\\s rather than :cpp:enum:`GameTypeId` itself, so a
                custom game type using some id not registered in :cpp:enum:`GameTypeId` can still be
                stored here
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<int>> hashGameTypeIds;

            /**
             * @brief
             @rst
             The registered :cpp:class:`ModTypeIdData` for each :cpp:enum:`ModTypeId` seen :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids for the :cpp:enum:`ModTypeId`\\s registered (see #hashGameTypeIds
             for why these are plain ``int``\\s rather than :cpp:enum:`ModTypeId` itself) and the
             values are the corresponding :cpp:class:`ModTypeIdData`\\s
             @endrst
             */
            std::unordered_map<int, ModTypeIdData> modTypes;

            /**
             * @brief
             @rst
             The `DFA`_ (using `Aho-Corasick`_) used to search for keywords within a `section`_ name
             :raw-html:`<br />` :raw-html:`<br />`

             The values are the ids for the :cpp:enum:`GameTypeId`\\s a found keyword identifies --
             see #hashGameTypeIds for why these are plain ``int``\\s rather than :cpp:enum:`GameTypeId`
             itself
             @endrst
             */
            BaseAhoCorasickDFA<std::unordered_set<int>> sectionKeywordsDFA;

            /**
             * @brief
             @rst
             Identifies which :cpp:enum:`GameTypeId`\\s a `section`_ keyword belongs to :raw-html:`<br />` :raw-html:`<br />`

             The keys are `section`_ keywords and the values are the ids for the
             :cpp:enum:`GameTypeId`\\s that keyword identifies (normally just a single id, but a
             set to account for the theoretical case where the same keyword is registered for more
             than one game type) :raw-html:`<br />` :raw-html:`<br />`

             See #hashGameTypeIds for why these are plain ``int``\\s rather than :cpp:enum:`GameTypeId`
             itself
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<int>> keywordGameTypeIds;

            /**
             * @brief
             @rst
             A distribution of the :cpp:enum:`ModTypeId`\\s seen so far :raw-html:`<br />` :raw-html:`<br />`

             The keys are the ids for the :cpp:enum:`ModTypeId`\\s seen (see #hashGameTypeIds for why
             these are plain ``int``\\s rather than :cpp:enum:`ModTypeId` itself) and the values are
             how many times that :cpp:enum:`ModTypeId` has been seen
             @endrst
             */
            std::unordered_map<int, int> modTypeIdDistribution;

            /**
             * @brief
             @rst
             The ids for the WuWa :cpp:enum:`ModTypeId`\\s that are potential candidates to
             classify a .ini file as, for future reference :raw-html:`<br />` :raw-html:`<br />`

             See #hashGameTypeIds for why these are plain ``int``\\s rather than :cpp:enum:`ModTypeId`
             itself
             @endrst
             */
            std::unordered_set<int> savedWuWaModTypeIds;

            /**
             * @brief
             @rst
             Identifies which :cpp:enum:`ModTypeId`\\s an accepting `DFA`_ state (a ``accept<ModTypeId>``
             or ``save<ModTypeId>`` state in #stateDFA) is associated with :raw-html:`<br />` :raw-html:`<br />`

             The keys are the names of the accepting states in #stateDFA and the values are the ids
             for the :cpp:enum:`ModTypeId`\\s associated with that accepting state (normally just a
             single id, but a set to account for the theoretical case where 2 different
             :cpp:class:`ModTypeIdData`\\s share the same hash) :raw-html:`<br />` :raw-html:`<br />`

             See #hashGameTypeIds for why these are plain ``int``\\s rather than :cpp:enum:`ModTypeId`
             itself
             @endrst
             */
            std::unordered_map<std::string, std::unordered_set<int>> acceptModTypeIds;

            /**
             * @brief Sets #stateDFA to its initial state (see the constructor's doc comment)
             */
            void setupStateDFA();

            /**
             * @brief
             @rst
             Reads a single line in a .ini file, mutating 'stats' with whatever the line reveals
             about the classification of the .ini file :raw-html:`<br />` :raw-html:`<br />`

             Does a high-level read of 'line' (already stripped of leading/trailing whitespace by
             the caller), then delegates the actual work to whichever subfunction below applies:

             * If 'line' starts with ``hash``, any amount of whitespace, then ``=``, the value part
               after the ``=`` is passed to :cpp:func:`readHash`
             * If 'line' starts with ``$\\WWMIv1``, :cpp:func:`markWuWa` is called
             * If 'line' starts with a `section`_ header (``[X]``, where ``X`` contains none of
               ``[``, ``]``, or a .ini comment character), ``X`` is passed to
               :cpp:func:`readSectionName`
             @endrst
             *
             * @param line The line in the .ini file to read, already stripped of leading/trailing whitespace
             * @param stats The resultant stats to mutate based off the classification result found from 'line'
             * @param gameTypeId The :cpp:enum:`GameTypeId` of the game the .ini file is meant for, if known
             */
            virtual void readLine(const std::string& line, IniClassifyStats& stats, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Reads the value part of a ``hash = <value>`` `KVP`_ found from a line in the .ini file,
             mutating 'stats' with whatever the value reveals about the classification of the .ini file
             :raw-html:`<br />` :raw-html:`<br />`

             Transitions #stateDFA via the ``hash:<hash>`` keyword; does nothing if that transition
             is invalid :raw-html:`<br />` :raw-html:`<br />`

             If the transition lands on an accepting state and #stateDFA was previously on
             ``isWuwa`` (i.e. ``$\\WWMIv1`` was already seen and 'hash' is registered for a WuWa mod
             type), the match is already confirmed: for each :cpp:enum:`ModTypeId` from
             #acceptModTypeIds, :cpp:func:`incModTypeCountByHash` is called (setting ``stats.isMod``
             to ``true`` if any were called), #stateDFA takes the ``reset`` transition, and this
             method returns :raw-html:`<br />` :raw-html:`<br />`

             Otherwise, for each :cpp:enum:`GameTypeId` 'hash' is registered under (see
             #hashGameTypeIds), #stateDFA attempts the ``game:<GameTypeId>`` transition; if it lands
             on an accepting state, the associated :cpp:enum:`ModTypeId`\\s (see #acceptModTypeIds)
             are added to #savedWuWaModTypeIds (for :cpp:enumerator:`GameTypeId::WuWa`) or passed to
             :cpp:func:`incModTypeCountByHash` (otherwise, setting ``stats.isMod`` to ``true`` if any
             were called). Between each :cpp:enum:`GameTypeId` tried, #stateDFA takes the
             ``prev:hash:<hash>`` back edge to return to the ``foundHash:<hash>`` state, except after
             the last one, where it takes the ``reset`` transition instead
             @endrst
             *
             * @param hash The value part of the ``hash`` `KVP`_
             * @param stats The resultant stats to mutate based off the classification result found from 'hash'
             */
            virtual void readHash(std::string_view hash, IniClassifyStats& stats);

            /**
             * @brief
             @rst
             Marks 'stats' with whatever a ``$\\WWMIv1`` marker line reveals about the classification
             of the .ini file :raw-html:`<br />` :raw-html:`<br />`

             Does nothing unless #stateDFA is currently on the ``start`` state, in which case it
             transitions #stateDFA to ``isWuwa`` (via the ``game:<GameTypeId::WuWa>`` keyword) and, if
             #savedWuWaModTypeIds isn't empty, increases each of those ids' #modTypeIdDistribution
             count via :cpp:func:`incModTypeCountByHash`, sets ``stats.isMod`` to ``true``, and
             clears #savedWuWaModTypeIds
             @endrst
             *
             * @param stats The resultant stats to mutate based off finding a ``$\\WWMIv1`` marker line
             */
            virtual void markWuWa(IniClassifyStats& stats);

            /**
             * @brief
             @rst
             Reads the name of a `section`_ found from a line in the .ini file, mutating 'stats' with
             whatever the section name reveals about the classification of the .ini file
             :raw-html:`<br />` :raw-html:`<br />`

             If 'sectionName' starts with ``TextureOverride`` or ``ShaderOverride``, ``stats.isMod``
             is set to ``true`` -- this check runs unconditionally, before #checkHasTextureOverride
             is considered, so a ``ShaderOverride`` `section`_ is still recognized as belonging to a
             mod even when #checkHasTextureOverride goes on to block it from any further,
             keyword-based classification below :raw-html:`<br />` :raw-html:`<br />`

             If #checkHasTextureOverride is ``true``, does nothing further unless 'sectionName'
             starts with ``TextureOverride`` :raw-html:`<br />` :raw-html:`<br />`

             If 'sectionName' contains the substring ``Remap``, ``stats.isFixed`` is set to
             ``true`` :raw-html:`<br />` :raw-html:`<br />`

             Searches 'sectionName' for the maximal registered keyword via #sectionKeywordsDFA --
             if 'gameTypeId' has a value, only keywords registered under that
             :cpp:enum:`GameTypeId` (see #keywordGameTypeIds) are considered. Does nothing if no
             keyword is found :raw-html:`<br />` :raw-html:`<br />`

             Otherwise, transitions #stateDFA to the matched keyword's ``sectionName:<keyword>``
             state; does nothing if that transition is invalid :raw-html:`<br />` :raw-html:`<br />`

             For each :cpp:enum:`GameTypeId` the matched keyword is registered under, #stateDFA
             attempts the ``game:<GameTypeId>`` transition; if it lands on an accepting state, each
             associated :cpp:enum:`ModTypeId` (see #acceptModTypeIds) is passed to
             :cpp:func:`incModTypeCountBySectionName`. Between each :cpp:enum:`GameTypeId` tried,
             #stateDFA takes the ``prev:sectionName:<keyword>`` back edge to return to the
             ``sectionName:<keyword>`` state, except after the last one, where it takes the
             ``reset`` transition instead
             @endrst
             *
             * @param sectionName The name of the `section`_ (the ``X`` in ``[X]``)
             * @param stats The resultant stats to mutate based off the classification result found from 'sectionName'
             * @param gameTypeId The :cpp:enum:`GameTypeId` of the game the .ini file is meant for, if known
             */
            virtual void readSectionName(std::string_view sectionName, IniClassifyStats& stats, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Increases #modTypeIdDistribution's count for ``modTypeId`` by 1, for a match found by
             `section`_ name
             @endrst
             *
             * @param modTypeId The id for the :cpp:enum:`ModTypeId` to increase the count for
             */
            virtual void incModTypeCountBySectionName(int modTypeId);

            /**
             * @brief
             @rst
             Increases #modTypeIdDistribution's count for ``modTypeId`` by 2, for a match found by hash
             @endrst
             *
             * @param modTypeId The id for the :cpp:enum:`ModTypeId` to increase the count for
             */
            virtual void incModTypeCountByHash(int modTypeId);
    };
}

#endif
