#ifndef AGRemapCore_IniFile_H
#define AGRemapCore_IniFile_H

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/model/strategies/ModType.h"
#include "AGRemapCore/model/strategies/iniClassifiers/BaseIniClassifier.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Class to handle ``.ini`` files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a from-scratch C++ port, not a 1-1 translation of the pure-Python
        ``FixRaidenBoss2.model.files.IniFile`` original -- capability is being ported over
        incrementally, so this class is expected to grow additional methods/state over time rather
        than arrive fully-formed in one pass
     @endrst
     */
    class IniFile {
        public:

            /**
             * @brief
             @rst
             Constructs a new .ini file. Does not read anything from disk yet -- see #readFileLines
             @endrst
             *
             * @param file The file path to the .ini file. If this is ``std::nullopt``, 'txt' is
             *      used as the content of the .ini file instead
             * @param txt Used as the text content of the .ini file if 'file' is ``std::nullopt``
             * @param gameTypeId
             @rst
             The specific game to filter classification on, if known -- passed through to
             :cpp:func:`BaseIniClassifier::classify`/:cpp:func:`BaseIniClassifier::checkIsFixedMod`
             as the corresponding :cpp:enum:`GameTypeId`, when 'gameTypeId' maps to one (see
             :cpp:func:`GameTypeIdTools::getEnum`) :raw-html:`<br />` :raw-html:`<br />`

             Stored as a plain ``int`` (not :cpp:enum:`GameTypeId` itself), matching this codebase's
             usual convention of keeping id-shaped config data as plain ints so a custom game id
             not registered in :cpp:enum:`GameTypeId` can still be stored, even though it won't
             actually narrow classification (the classifier's own API only accepts a real
             :cpp:enum:`GameTypeId`) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param filteredModTypeIds
             @rst
             The specific :cpp:enum:`ModTypeId`\\s (by id) to accept when classifying #modTypes. If
             this is ``std::nullopt``, every :cpp:enum:`ModTypeId` the classifier reports is
             accepted :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param forcedModTypeIds
             @rst
             The specific :cpp:enum:`ModTypeId`\\s (by id) to forcibly use for #modTypes, overriding
             whatever the classifier itself would have determined -- see #classify :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param overrideModTypes
             @rst
             :cpp:class:`ModType`\\s, keyed by their :cpp:enum:`ModTypeId`, that take precedence
             over the global registry (:cpp:func:`ModTypeIdTools::getModType`) when #classify
             resolves a classified/forced :cpp:enum:`ModTypeId` into an actual
             :cpp:class:`ModType` :raw-html:`<br />` :raw-html:`<br />`

             If this is ``std::nullopt``, treated the same as an empty map (nothing overridden)
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param iniClassifier
             @rst
             The classifier used to identify what mod(s) belong to this .ini file :raw-html:`<br />`
             :raw-html:`<br />`

             This is a non-owning pointer to a classifier owned elsewhere -- if this is ``nullptr``,
             uses the shared default classifier from :cpp:func:`GlobalIniClassifiers::classifier`
             instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit IniFile(std::optional<std::string> file = std::nullopt, std::string txt = "",
                              std::optional<int> gameTypeId = std::nullopt,
                              std::optional<std::unordered_set<int>> filteredModTypeIds = std::nullopt,
                              std::optional<std::unordered_set<int>> forcedModTypeIds = std::nullopt,
                              std::optional<std::unordered_map<int, ModType>> overrideModTypes = std::nullopt,
                              BaseIniClassifier* iniClassifier = nullptr);

            virtual ~IniFile() = default;

            /**
             * @brief The file path to the .ini file, or ``std::nullopt`` if this .ini file has no
             *      backing file and is only ever driven by its in-memory text content
             */
            const std::optional<std::string>& getFile() const;

            /**
             * @brief
             @rst
             The text content of the .ini file, as of the last call to #readFileLines (or the
             constructor's ``txt`` argument, if #getFile is ``std::nullopt``)
             @endrst
             */
            const std::string& getFileTxt() const;

            /**
             * @brief
             @rst
             The text lines of the .ini file, as of the last call to #readFileLines (or the
             constructor's ``txt`` argument, if #getFile is ``std::nullopt``) :raw-html:`<br />`
             :raw-html:`<br />`

             Each line ends with a newline character (``\\n``), except possibly the last line
             @endrst
             */
            const std::vector<std::string>& getFileLines() const;

            /**
             * @brief Whether the lines of the .ini file have been read at least once, either via
             *      #readFileLines or, for a file-less .ini file, the constructor's ``txt`` argument
             */
            bool fileLinesRead() const;

            /**
             * @brief
             @rst
             Reads the lines of the .ini file, similar to Python's `readlines`_ :raw-html:`<br />`
             :raw-html:`<br />`

             If #getFile is ``std::nullopt``, this does not read anything from disk and simply
             returns the existing value of #getFileLines instead :raw-html:`<br />` :raw-html:`<br />`

             .. _readlines: https://docs.python.org/3/tutorial/inputoutput.html#methods-of-file-objects
             @endrst
             *
             * @return The text lines read from the .ini file
             *
             * @throws std::runtime_error if #getFile holds a file path that cannot be opened
             */
            const std::vector<std::string>& readFileLines();

            /**
             * @brief
             @rst
             Classifies the .ini file, determining #isMod, #isFixed, and #modTypes -- reads the
             .ini file first via #readFileLines if it hasn't been read yet (same "read on first use"
             behavior as the pure-Python original's own ``classify``) :raw-html:`<br />`
             :raw-html:`<br />`

             * If the constructor's ``forcedModTypeIds`` argument was ``std::nullopt``, calls the
               classifier's own :cpp:func:`BaseIniClassifier::classify` normally: #isMod/#isFixed are
               set from its result, and #modTypes is built from its ``modType`` map (filtered down
               to the constructor's ``filteredModTypeIds`` argument, when given), resolving each id
               to an actual :cpp:class:`ModType` via the constructor's ``overrideModTypes``/the
               global registry
             * If the constructor's ``forcedModTypeIds`` argument had a value, the classifier is
               never asked to classify a :cpp:enum:`ModTypeId` at all -- only
               :cpp:func:`BaseIniClassifier::checkIsFixedMod` is called, to set #isMod/#isFixed
               independently of any specific mod type, and #modTypes is instead built directly from
               ``forcedModTypeIds`` (each id resolved to a :cpp:class:`ModType` the same way),
               skipping the classifier's own classification entirely
             @endrst
             */
            void classify();

        protected:

            /**
             * @brief Whether the .ini file belongs to a mod, as of the last call to #classify
             */
            bool isMod = false;

            /**
             * @brief Whether the .ini file has already been fixed, as of the last call to #classify
             */
            bool isFixed = false;

            /**
             * @brief
             @rst
             The types of mod the .ini file was classified (or forced) as, as of the last call to
             #classify, keyed by their :cpp:enum:`ModTypeId` :raw-html:`<br />` :raw-html:`<br />`

             Unlike the deprecated pure-Python original (where a .ini file could only ever have a
             single :cpp:class:`ModType`), this can hold more than one entry
             @endrst
             */
            std::unordered_map<int, ModType> modTypes;

        private:
            std::optional<std::string> file_;
            std::string fileTxt_;
            std::vector<std::string> fileLines_;
            bool fileLinesRead_ = false;

            std::optional<int> gameTypeId_;
            std::optional<std::unordered_set<int>> filteredModTypeIds_;
            std::optional<std::unordered_set<int>> forcedModTypeIds_;
            std::unordered_map<int, ModType> overrideModTypes_;
            BaseIniClassifier* iniClassifier_;

            // Reads the raw bytes at 'path' as text, normalizing "\r\n"/"\r" line endings down to
            // "\n" (matching Python text-mode's universal newline translation), and stores the
            // result into fileTxt_/fileLines_/fileLinesRead_.
            void readFromDisk(const std::string& path);

            // Splits 'txt' into lines the same way Python's str.splitlines(keepends = True) does --
            // each returned line retains its own trailing "\n", except possibly the last line if
            // 'txt' doesn't end with one. Stores the result into fileTxt_/fileLines_/fileLinesRead_.
            void setFileTxt(std::string txt);

            // Resolves 'modTypeId' to an actual ModType -- overrideModTypes_ takes precedence,
            // falling back to the global registry (ModTypeIdTools::getModType) otherwise.
            // std::nullopt if 'modTypeId' isn't in overrideModTypes_ and isn't registered globally
            // either.
            std::optional<ModType> getModType(int modTypeId) const;
    };
}

#endif
