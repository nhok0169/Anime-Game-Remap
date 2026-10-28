#ifndef AGRemapCore_RabbitFX_H
#define AGRemapCore_RabbitFX_H

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

#include <array>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     What a fix needs to know about RabbitFX 8.3.2, the Wuthering Waves effects library a mod calls
     through :cpp:member:`IniKeywords::RabbitFXSetTexturesPath` :raw-html:`<br />` :raw-html:`<br />`

     RabbitFX's ``SetTextures`` decides how to route the maps a draw has bound at ``ps-t0`` ..
     ``ps-t3`` by a TAG its texture overrides give each texture (a ``filter_index`` of ``1718.3`` ..
     ``1718.6``), and writes the result as four routing flags, ``x187`` .. ``w187``. A remapped draw
     binds the TARGET's textures there, so the flags the source's draw would have had are worked out
     from the source's textures (:cpp:func:`textureTag`, :cpp:func:`routingFlags`) and pinned after
     the call (:cpp:member:`FlagKeys`)
     @endrst
     */
    class RabbitFX {
        public:
            /**
             * @brief The four routing flags ``SetTextures`` writes, in register order
             */
            static inline const std::array<std::string, 4> FlagKeys = {"x187", "y187", "z187", "w187"};

            /**
             * @brief
             @rst
             The tag RabbitFX's texture overrides give the texture at 'path', from its ``.dds``
             header: ``3`` for ``filter_index = 1718.3`` .. ``6`` for ``1718.6``, ``0`` for none
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A square ``BC7`` of at most 512 pixels with ONE mip matches both ``1718.4`` and
                ``1718.6``; it is taken as ``6``
             @endrst
             *
             * @param path The ``.dds`` file
             *
             * @return The tag
             */
            static int textureTag(const std::string& path);

            /**
             * @brief
             @rst
             The routing flags ``SetTextures`` writes for a draw whose ``ps-t0`` .. ``ps-t3`` carry
             'tags'. A flag it leaves unwritten keeps whatever the previous draw left, which no fix
             can know; it is ``0`` here
             @endrst
             *
             * @param tags Each register's :cpp:func:`textureTag`
             *
             * @return The flags, in :cpp:member:`FlagKeys`' order
             */
            static std::array<int, 4> routingFlags(const std::array<int, 4>& tags);

            /**
             * @brief Whether 'text' names RabbitFX, ignoring case
             *
             * @param text An ``.ini`` line, key or value
             */
            static bool names(const std::string& text);

            /**
             * @brief
             @rst
             Whether a `KVP`_ is one of RabbitFX's own: a resource under
             :cpp:member:`IniKeywords::RabbitFXResourceFolder`, or a ``run =`` of one of its command lists
             @endrst
             *
             * @param key The key
             * @param value The value
             */
            static bool isOwnKVP(const std::string& key, const std::string& value);
    };
}

#endif
