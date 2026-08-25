#ifndef AGRemapCore_IniKeywords_H
#define AGRemapCore_IniKeywords_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Common keywords used in the .ini file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        This is a **partial** port of the pure-Python ``IniKeywords`` enum
        (``constants/IniConsts.py``) -- that enum has ~30 members covering many subsystems that
        haven't been ported to C++ yet; only the members :cpp:class:`IniNamingTools` actually needs
        are included here. Add more members as later-ported subsystems need them, rather than
        porting the whole enum speculatively up front
     @endrst
     */
    class IniKeywords {
        public:

            /**
             * @brief The starting prefix used for any `sections`_ that reference some file
             */
            static inline const std::string Resource = "Resource";

            /**
             * @brief The starting prefix used for some `section`_ that overrides the resource of a mod
             */
            static inline const std::string TextureOverride = "TextureOverride";

            /**
             * @brief The starting prefix used for some `section`_ that overrides a mod's shader
             */
            static inline const std::string ShaderOverride = "ShaderOverride";

            /**
             * @brief The substring used to indicate a `section`_ is edited by this software
             */
            static inline const std::string Remap = "Remap";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Blend.buf`` file
             @endrst
             */
            static inline const std::string Blend = "Blend";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Position.buf`` file
             @endrst
             */
            static inline const std::string Position = "Position";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Texcoord.buf`` file
             @endrst
             */
            static inline const std::string Texcoord = "Texcoord";

            /**
             * @brief The substring used to indicate that the `section`_ was created by this program
             */
            static inline const std::string RemapFix = Remap + "Fix";

            /**
             * @brief
             @rst
             The substring used to indicate that the `section`_ contains some edited/created texture
             ``*.RemapTex.dds`` file
             @endrst
             */
            static inline const std::string RemapTex = Remap + "Tex";

            /**
             * @brief The substring used to indicate that the `section`_ contains some downloaded file from the internet
             */
            static inline const std::string RemapDL = Remap + "DL";
    };
}

#endif
