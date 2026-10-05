#ifndef AGRemapCore_TexRegLayout_H
#define AGRemapCore_TexRegLayout_H

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
#include <utility>
#include <vector>

#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemap.h"
#include "AGRemapCore/model/strategies/iniFixers/regEdits/RegRemove.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Builds the register edits that put each bound texture on the register its **name** says
     :raw-html:`<br />` :raw-html:`<br />`

     ``NNFix`` and ``ORFix`` do not re-slot a REGISTER, they read a ROLE out of a fixed one
     (``CommandListReference``: the normal map from ``ps-t0``, the diffuse from ``ps-t1``, the light
     map from ``ps-t2``), so a `section`_ handed to one has to have its textures there. **A mod
     dumped straight from the game does not**: the game's own draw of a GI 6.x character binds the
     light map, normal map and diffuse in that order, and a CitlaliWhisperofStars mod is written
     exactly that way while Citlali's own mods are written in ``ORFix``'s order. Carried across
     unchanged and handed to ``ORFix``, every role comes out of the wrong slot -- a wrong-textured
     model whose every downloaded slot looks right (2026-09-22) :raw-html:`<br />`
     :raw-html:`<br />`

     Which texture is which comes from the resource NAME, through :cpp:class:`RegValChecks` --
     whose header records why the pixels are deliberately not consulted. A binding naming no role
     is left where it is, so a mod already written in the fix's layout is untouched and this
     subsumes a positional shift rather than running beside it
     @endrst
     */
    class TexRegLayout {
        public:

            TexRegLayout() = delete;

            /**
             * @brief
             @rst
             A register per role -- empty for a role this layout has no slot for
             @endrst
             */
            struct Roles {

                /**
                 * @brief The register the normal map belongs on, empty when the layout has none
                 */
                std::string normalMap;

                /**
                 * @brief The register the diffuse belongs on
                 */
                std::string diffuse;

                /**
                 * @brief The register the light map belongs on
                 */
                std::string lightMap;
            };

            /**
             * @brief
             @rst
             The ``ps-t`` registers a GI character's shader reads its textures out of, as
             ``ORFix`` (with a normal map) or ``NNFix`` (without) expects them
             @endrst
             *
             * @param normalMap Whether the layout carries a normal map -- ``ORFix``'s, rather than ``NNFix``'s
             */
            static Roles fixLibraryRoles(bool normalMap);

            /**
             * @brief
             @rst
             The :cpp:class:`RegRemap` rules that send every occurence of 'fromRegs' to the register
             'roles' gives its NAME's role, leaving a value that names none exactly where it is
             :raw-html:`<br />` :raw-html:`<br />`

             The checks are made mutually exclusive rather than taken raw: a
             :cpp:class:`RegRemap` sends a `KVP`_ to EVERY destination whose check passes, and a
             fix's own generated names can compose two roles (a diffuse edited as a light map is
             ``<obj>Diffuse<target>LightMapRemapTex``), which would otherwise land on two registers.
             Precedence: normal map, then light map, then diffuse
             @endrst
             *
             * @param roles Which register each role belongs on -- see \ref fixLibraryRoles
             * @param fromRegs The registers a mod may have bound, all of which are considered
             */
            static std::vector<std::pair<std::string, RegRemap<>::KeyRemapValue>> byName(
                const Roles& roles, const std::vector<std::string>& fromRegs);

            /**
             * @brief
             @rst
             The :cpp:class:`RegRemove` keys that drop a NORMAL MAP wherever it is bound -- for a
             target layout that has no slot for one :raw-html:`<br />` :raw-html:`<br />`

             By name rather than by position, for the same reason as \\ref byName: a mod written in
             the game's own order does not keep its normal map at ``ps-t0``
             @endrst
             *
             * @param fromRegs The registers a mod may have bound
             */
            static std::vector<std::pair<std::string, std::optional<RegRemove<>::RemoveKeyCheck>>> removeNormalMap(
                const std::vector<std::string>& fromRegs);
    };
}

#endif
