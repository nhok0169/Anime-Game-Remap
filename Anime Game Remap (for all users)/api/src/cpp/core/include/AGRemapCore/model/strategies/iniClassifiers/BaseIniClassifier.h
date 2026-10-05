#ifndef AGRemapCore_BaseIniClassifier_H
#define AGRemapCore_BaseIniClassifier_H

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
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             *
             * @return The stats about the classification of the .ini file
             */
            virtual IniClassifyStats classify(const std::string& iniTxt, GameTypeIdFilter gameTypeIds = std::nullopt);

            /**
             * @brief
             @rst
             Determines the type of mod given the text from the mod's .ini file, assuming the lines of the text are already given
             @endrst
             *
             * @param iniTxt The lines of text of the .ini file to read from, with each line ending with a newline character
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             *
             * @return The stats about the classification of the .ini file
             */
            virtual IniClassifyStats classify(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds = std::nullopt);

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
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             *
             * @return Whether the .ini file belongs to a mod
             */
            virtual bool checkIsMod(const std::string& iniTxt, GameTypeIdFilter gameTypeIds = std::nullopt);

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
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             *
             * @return Whether the .ini file belongs to a mod
             */
            virtual bool checkIsMod(const std::vector<std::string>& iniTxt, GameTypeIdFilter gameTypeIds = std::nullopt);

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
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             */
            virtual void checkIsFixedMod(const std::string& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds = std::nullopt);

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
             * @param gameTypeIds The games the .ini file may belong to, or ``std::nullopt`` for every game
             */
            virtual void checkIsFixedMod(const std::vector<std::string>& iniTxt, bool* isFixed, bool* isMod, GameTypeIdFilter gameTypeIds = std::nullopt);

            /**
             * @brief Clears the state of the classifier
             */
            virtual void clear();
    };
}

#endif
