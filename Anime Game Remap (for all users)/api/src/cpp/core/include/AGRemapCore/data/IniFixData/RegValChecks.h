#ifndef AGRemapCore_RegValChecks_H
#define AGRemapCore_RegValChecks_H

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

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The ready-made tests a :cpp:class:`GIMICharFixerConfig`'s ``RegRef`` can be conditional on --
     "is the thing bound to this register a lightmap?", and its four siblings
     :raw-html:`<br />` :raw-html:`<br />`

     **WHAT THEY ARE ACTUALLY FOR: a mod that has ALREADY been fixed by hand.** GI swapped which
     registers a character's shader reads its diffuse and its lightmap out of (the same swap behind
     the white cheek spots -- see the face diffuse section in
     :doc:`Creating Remaps </AI Agent Help/CreatingRemaps/CLAUDE>`). There are two ways a mod author
     can answer that :raw-html:`<br />` :raw-html:`<br />`

     1. **Re-issue NNFix / ORFix.** The better answer, and the one this software prefers: those
        libraries carry the correction, so whatever the game changes NEXT is handled by updating
        them rather than every mod.
     2. **Swap the registers by hand**, which is the same thing this software's own ``regEdits``
        do.

     A mod that took route 2 arrives with its registers *already* where the fix was going to move
     them. Applying the shift again would undo the author's work. So the shift is made conditional:
     move ``ps-t2`` **only if what is sitting there still looks like a lightmap**, and otherwise
     (``RegRemapRule::keepIfNoneMatch``) leave it exactly where it is
     :raw-html:`<br />` :raw-html:`<br />`

     **This is a heuristic, and it is one on purpose -- there is no reliable way to tell a lightmap
     from a diffuse.** Both are just textures; nothing in a ``.dds`` says which slot it belongs in.
     What is left is the name the mod author gave the resource section, following the GIMI
     convention (``ResourceKiraraBodyLightMap``), so these are substring tests on that name,
     mirroring the pure-Python ``IniFixBuilderData._isLightMap`` family exactly
     (``val.lower().find("lightmap") != -1``). Anything stricter stops matching real mods
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        **Reading the texture's CONTENT instead was considered and deliberately rejected.** Judging
        a lightmap by its colours would misread any mod that recolours a character on purpose --
        paint someone head to toe in green goo and their diffuse starts looking like a lightmap. A
        name is a weaker signal than the pixels, but it is the author's own statement of intent,
        and it does not punish them for making an unusual mod :raw-html:`<br />` :raw-html:`<br />`

        The failure mode this leaves is someone naming a lightmap "diffuse". That is not a case to
        defend against: a person doing it is deliberately breaking their own fix, and no heuristic
        survives an author working against it :raw-html:`<br />` :raw-html:`<br />`

        Case-insensitive by lowercasing ASCII only, like the original. These match section names,
        not user-facing text, so the grapheme-aware :cpp:class:`StringTools` path buys nothing here
     @endrst
     */
    class RegValChecks {
        public:

            RegValChecks() = delete;

            /**
             * @brief Whether 'val' names a diffuse texture
             *
             * @param val The register's value -- a resource section name
             */
            static bool isDiffuse(const std::string& val);

            /**
             * @brief Whether 'val' names a lightmap
             *
             * @param val The register's value -- a resource section name
             */
            static bool isLightMap(const std::string& val);

            /**
             * @brief Whether 'val' names a normal map
             *
             * @param val The register's value -- a resource section name
             */
            static bool isNormalMap(const std::string& val);

            /**
             * @brief Whether 'val' names a metal map
             *
             * @param val The register's value -- a resource section name
             */
            static bool isMetalMap(const std::string& val);

            /**
             * @brief Whether 'val' names a shadow ramp
             *
             * @param val The register's value -- a resource section name
             */
            static bool isShadow(const std::string& val);
    };
}

#endif
