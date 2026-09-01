#ifndef AGRemapCore_IniBoilerPlate_H
#define AGRemapCore_IniBoilerPlate_H

#include <cstddef>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Boilerplate constants used when fixing a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A **partial** port of the pure-Python ``IniBoilerPlate`` enum (``constants/IniConsts.py``),
        which is still the one the `Python`_ ``IniFile`` uses -- only the members
        :cpp:class:`RemapIniFixContext` needs are here, and ``OldHeading`` (which belongs to the
        classifier's already-fixed check, not to writing a fix) is deliberately not among them.
        Same rule as :cpp:class:`IniKeywords`: add members as later-ported subsystems need them

     .. note::
        ``DefaultHeading``'s *title* (``".*Remap"``) has no member here on purpose. Despite living
        on the heading, it is never written into a ``.ini`` file -- ``IniFile._setType`` clears the
        title on every construction and every reclassification, so the title a fix is actually
        wrapped in is always the derived :cpp:func:`RemapIniFixContext::headingName`. The literal
        is a *regular expression* used by the still-pure-Python ``IniRemover`` to find a fix again
        later, and belongs with that class rather than with this one
     @endrst
     */
    class IniBoilerPlate {
        public:

            /**
             * @brief Placeholder for the shortened name of the mod to fix
             */
            static inline const std::string ShortModTypeNameReplaceStr = "{{shortModTypeName}}";

            /**
             * @brief Placeholder for the name of the mod to fix
             */
            static inline const std::string ModTypeNameReplaceStr = "{{modTypeName}}";

            /**
             * @brief
             @rst
             The credit text written into the ``.ini`` file, still holding both placeholders above
             @endrst
             */
            static inline const std::string Credit =
                "\n; " + ModTypeNameReplaceStr + "remapped by Albert Gold#2696 and NK#1321. If you used it to remap your "
                + ShortModTypeNameReplaceStr + "mods pls give credit for \"Albert Gold#2696\" and \"Nhok0169\""
                + "\n; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support";

            /**
             * @brief How many characters one side of the heading's border has
             */
            static inline const std::size_t DefaultHeadingSideLen = 15;

            /**
             * @brief The character the heading's border is drawn with
             */
            static inline const std::string DefaultHeadingSideChar = "-";

            /**
             * @brief
             @rst
             The name a heading falls back to when the ``.ini`` file was never classified as any
             mod type (``ini.getFixModTypeHeadingname``'s own fallback)
             @endrst
             */
            static inline const std::string DefaultModTypeHeadingName = "GI";

            /**
             * @brief The name the credit falls back to when the ``.ini`` file was never classified
             */
            static inline const std::string DefaultCreditModTypeName = "Mod";
    };
}

#endif
