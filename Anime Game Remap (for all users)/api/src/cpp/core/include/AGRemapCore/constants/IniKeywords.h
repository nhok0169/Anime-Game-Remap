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

            /**
             * @brief The `KVP`_ key used to reference/call another `section`_
             */
            static inline const std::string Run = "run";

            /**
             * @brief
             @rst
             The `KVP`_ key holding a `section`_'s model hash -- what
             :cpp:class:`GIMISectionClassifier` looks a mod object up by
             @endrst
             */
            static inline const std::string Hash = "hash";

            /**
             * @brief
             @rst
             The `KVP`_ key holding the first index of the model a `section`_ draws -- the second
             half of what :cpp:class:`GIMISectionClassifier` looks a mod object up by, for the mod
             objects a ``hash`` alone cannot tell apart
             @endrst
             */
            static inline const std::string MatchFirstIndex = "match_first_index";

            /**
             * @brief
             @rst
             The `KVP`_ key naming the file a resource `section`_ points at :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Classes that edit `KVPs`_ generically take this as a ``K``-typed customization point
                instead (eg. :cpp:type:`BaseResEdit::ResEditConfig`'s ``filenameKey``), since ``K``
                is not ``std::string`` in the `pybind11`_ layer. This constant is for the
                plain-``std::string`` callers, and for keeping the one spelling in one place
             @endrst
             */
            static inline const std::string Filename = "filename";

            /**
             * @brief
             @rst
             The value a `KVP`_ carries to mean "nothing at all" -- a ``filename =`` naming this is
             a placeholder, not a real resource
             @endrst
             */
            static inline const std::string Null = "null";

            /**
             * @brief
             @rst
             The `KVP`_ key naming a `section`_'s index buffer -- also the last index column of the
             :cpp:class:`Hashes` row holding that buffer's hash
             @endrst
             */
            static inline const std::string Ib = "ib";

            /**
             * @brief The `KVP`_ key naming a `section`_'s position vertex buffer
             */
            static inline const std::string Vb0 = "vb0";

            /**
             * @brief
             @rst
             The `KVP`_ key naming a `section`_'s blend **or** texcoord vertex buffer -- which of
             the two it is depends on the `section`_, not on the key
             @endrst
             */
            static inline const std::string Vb1 = "vb1";

            /**
             * @brief
             @rst
             The `KVP`_ key controlling how a draw call is handled -- ``skip`` is the only value
             this software writes, alongside \ref Draw, after a downloaded ``Blend.buf``
             @endrst
             */
            static inline const std::string Handling = "handling";

            /**
             * @brief
             @rst
             The `KVP`_ key issuing a non-indexed draw, written as ``<vertex count>,0`` -- the pair
             \ref Handling completes after a downloaded ``Blend.buf``
             @endrst
             */
            static inline const std::string Draw = "draw";

            /**
             * @brief
             @rst
             The `KVP`_ key that actually draws a model. What a texture fix has to be issued in
             front of -- see :cpp:class:`RegDelimitedAdd`
             @endrst
             */
            static inline const std::string DrawIndexed = "drawindexed";

            /**
             * @brief
             @rst
             The sub-command call to `ORFix`_ (Outline Reflection Fix) :raw-html:`<br />`
             :raw-html:`<br />`

             One of the two fixes shipped by an external ``3dmigoto`` library that work around
             textures the game breaks each version. A ``run =`` naming this is a call into that
             library, not into the mod's own `sections`_
             @endrst
             */
            static inline const std::string ORFixPath = "CommandList\\global\\ORFix\\ORFix";

            /**
             * @brief
             @rst
             The sub-command call to ``NNFix`` (No Normal Fix) -- the other half of the same
             external library as \ref ORFixPath :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                It lives under the library's ``ORFix`` folder, **not** an ``NNFix`` one. That
                asymmetry is the library's, not a typo here
             @endrst
             */
            static inline const std::string NNFixPath = "CommandList\\global\\ORFix\\NNFix";

            /**
             * @brief
             @rst
             Written in place of a ``hash`` that has no mapping onto the mod being fixed to --
             see :cpp:class:`RegAssetRemap`
             @endrst
             */
            static inline const std::string HashNotFound = "HashNotFound";

            /**
             * @brief
             @rst
             Written in place of a ``match_first_index`` that has no mapping onto the mod being
             fixed to -- the index counterpart of ef HashNotFound
             @endrst
             */
            static inline const std::string IndexNotFound = "IndexNotFound";

            /**
             * @brief The comment marker used to hide (comment out) a previously-fixed original `section`_
             */
            static inline const std::string HideOriginalComment = ";RemapFixHideOrig -->";
    };
}

#endif
