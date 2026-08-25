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
             * @brief
             @rst
             Determines whether the mod's .ini file belongs to a mod, given the full text from the
             .ini file :raw-html:`<br />` :raw-html:`<br />`

             Cheaper than :cpp:func:`classify` when only this yes/no answer is needed -- see
             :cpp:func:`classify`'s own doc comment for what "belongs to a mod" means
             @endrst
             *
             * @param iniTxt The full text of the .ini file to read from
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             *
             * @return Whether the .ini file belongs to a mod
             */
            virtual bool checkIsMod(const std::string& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Determines whether the mod's .ini file belongs to a mod, given the text from the .ini
             file, assuming the lines of the text are already given :raw-html:`<br />` :raw-html:`<br />`

             Cheaper than :cpp:func:`classify` when only this yes/no answer is needed -- see
             :cpp:func:`classify`'s own doc comment for what "belongs to a mod" means
             @endrst
             *
             * @param iniTxt The lines of text of the .ini file to read from, with each line ending with a newline character
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             *
             * @return Whether the .ini file belongs to a mod
             */
            virtual bool checkIsMod(const std::vector<std::string>& iniTxt, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Determines whether the mod's .ini file is fixed and/or belongs to a mod, given the
             full text from the .ini file :raw-html:`<br />` :raw-html:`<br />`

             Cheaper than :cpp:func:`classify` when only these yes/no answers are needed -- see
             :cpp:func:`classify`'s own doc comment for what "belongs to a mod"/"is fixed" mean
             @endrst
             *
             * @param iniTxt The full text of the .ini file to read from
             * @param isFixed Set to whether the .ini file is fixed
             * @param isMod Set to whether the .ini file belongs to a mod
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             */
            virtual void checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief
             @rst
             Determines whether the mod's .ini file is fixed and/or belongs to a mod, given the
             text from the .ini file, assuming the lines of the text are already given
             :raw-html:`<br />` :raw-html:`<br />`

             Cheaper than :cpp:func:`classify` when only these yes/no answers are needed -- see
             :cpp:func:`classify`'s own doc comment for what "belongs to a mod"/"is fixed" mean
             @endrst
             *
             * @param iniTxt The lines of text of the .ini file to read from, with each line ending with a newline character
             * @param isFixed Set to whether the .ini file is fixed
             * @param isMod Set to whether the .ini file belongs to a mod
             * @param gameTypeId The game the .ini file is expected to belong to, if known
             */
            virtual void checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, std::optional<GameTypeId> gameTypeId = std::nullopt);

            /**
             * @brief Clears the state of the classifier
             */
            virtual void clear();
    };
}

#endif
