#ifndef AGRemapCore_ShaderFilterTags_H
#define AGRemapCore_ShaderFilterTags_H
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

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {
    /**
     * @brief
     @rst
     The shaders a fix tells apart by a ``filter_index``, and what it writes for them :raw-html:`<br />`
     :raw-html:`<br />`

     3DMigoto sets a shader's ``filter_index`` from a ``[ShaderOverride...]`` `section`_ on its hash, and a section
     of the mod then asks ``if ps == <index>`` (or ``vs``) to act on one pass of a draw only. This holds the shaders a
     fix tags, in the order they were tagged, with their values; writes the ``[ShaderOverride...]`` sections
     (:cpp:func:`section`); and writes the condition true on a draw by any of some shaders, or by none of them
     (:cpp:func:`condition`) :raw-html:`<br />` :raw-html:`<br />`

     Which value each shader gets is the fixer's choice. 3DMigoto keys a ``[ShaderOverride]`` by its shader hash
     across EVERY loaded ``.ini``, so two fixes that tag the same shader must give it the same value
     @endrst
     */
    class ShaderFilterTags {
        public:
            /**
             * @brief One tagged shader
             */
            struct Tag {
                /**
                 * @brief The shader's hash
                 */
                std::string shaderHash;

                /**
                 * @brief The ``filter_index`` it is given
                 */
                std::string filterIndex;
            };

            /**
             * @brief
             @rst
             Tags a shader, unless it is tagged already (its first value stays)
             @endrst
             *
             * @param shaderHash The shader's hash
             * @param filterIndex The ``filter_index`` it is given
             *
             * @return Whether the shader was tagged by this call
             */
            bool add(const std::string& shaderHash, const std::string& filterIndex);

            /**
             * @brief
             @rst
             Tags each of 'shaderHashes' not tagged yet, in order: with its value in 'given' when it has one, else with
             the next of a numbered run -- 'base', 'base' + 'step', 'base' + 2 'step', ... -- which only the shaders
             without a given value advance
             @endrst
             *
             * @param shaderHashes The shaders, in the order they are tagged
             * @param given The values some of them must have -- one another fix also tags, which has to agree
             * @param base The run's first value
             * @param step The run's step
             */
            void addNumbered(const std::vector<std::string>& shaderHashes, const std::map<std::string, std::string>& given,
                             double base, double step);

            /**
             * @brief The ``filter_index`` of a shader, or ``std::nullopt`` when it is not tagged
             *
             * @param shaderHash The shader's hash
             *
             * @return The value
             */
            std::optional<std::string> filterIndexOf(const std::string& shaderHash) const;

            /**
             * @brief The tagged shaders, in the order they were tagged
             */
            const std::vector<Tag>& tags() const;

            /**
             * @brief
             @rst
             The ``.ini`` condition true on a draw by any of 'shaderHashes' -- ``<reg> == <index> || ...`` -- or, with
             'none', by none of them -- ``<reg> != <index> && ...``. Each shader is named once, in the order given; a
             shader that is not tagged is left out
             @endrst
             *
             * @param reg The register a shader's ``filter_index`` is read from: ``ps`` or ``vs``
             * @param shaderHashes The shaders' hashes
             * @param none Whether the condition is true on a draw by none of the shaders. **Default**: ``false``
             *
             * @return The condition, or empty when no shader of 'shaderHashes' is tagged
             */
            std::string condition(const std::string& reg, const std::vector<std::string>& shaderHashes, bool none = false) const;

            /**
             * @brief
             @rst
             The ``[ShaderOverride...]`` `section`_ that tags shader number 'index' (:cpp:func:`tags`): ``hash`` and
             ``filter_index``
             @endrst
             *
             * @param index The shader's position in :cpp:func:`tags`
             * @param name The section's name
             *
             * @return The section
             */
            std::unique_ptr<IfTemplate<std::string, std::string>> section(std::size_t index, const std::string& name) const;

        private:
            std::vector<Tag> tags_;
    };
}

#endif
