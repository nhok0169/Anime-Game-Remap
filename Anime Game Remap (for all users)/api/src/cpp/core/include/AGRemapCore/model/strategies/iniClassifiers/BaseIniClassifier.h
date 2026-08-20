#ifndef AGRemapCore_BaseIniClassifier_H
#define AGRemapCore_BaseIniClassifier_H

#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/constants/GameTypeId.h"
#include "AGRemapCore/model/strategies/iniClassifiers/IniClassifyStats.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Base class to help classify the type of mod given the mod's .ini files
     @endrst
     */
    class BaseIniClassifier {
        public:

            /**
             * @brief Destroys the classifier
             */
            virtual ~BaseIniClassifier() = default;

            /**
             * @brief
             @rst
             Determines the type of mod given the full text from the mod's .ini file
             @endrst
             *
             * @param iniTxt The full text of the .ini file to read from
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             *
             * @return The stats about the classification of the .ini file
             */
            virtual IniClassifyStats classify(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Determines the type of mod given the text from the mod's .ini file, assuming the lines of the text are already given
             @endrst
             *
             * @param iniTxt The lines of text of the .ini file to read from, with each line ending with a newline character
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             *
             * @return The stats about the classification of the .ini file
             */
            virtual IniClassifyStats classify(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief Clears the state of the classifier
             */
            virtual void clear();
    };
}

#endif
