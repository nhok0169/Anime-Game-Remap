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

#ifndef AGRemapCore_VGComponentSplit_H
#define AGRemapCore_VGComponentSplit_H

#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/BufValue.h"

namespace AGRemapCore {
    class BlendFile;
    class IbFile;

    /**
     * @brief
     @rst
     One target component of a skin made of several -- YelanTranquil's ``Body``, ``Bang`` and
     ``Eye`` -- and how a mod's vertex groups reach its bones
     @endrst
     */
    struct VGComponentSpec {
        /**
         * @brief The component's name
         */
        std::string name;

        /**
         * @brief The mod's vertex group (source index) to this component's bone
         */
        VGRemap remap;

        /**
         * @brief
         @rst
         Further source groups the component has a bone for, honoured only on a vertex that also
         carries one of \ref remap's groups -- the reverse remap turned around. A hair vertex
         weighted head + bang keeps its head weight on the Bang's head bone; a face vertex weighted
         to the head alone still collapses. Only used by a negative-index component
         @endrst
         */
        std::unordered_map<long long, long long> secondary;

        /**
         * @brief
         @rst
         ``true``: the **negative-index** strategy -- the component draws the whole mod, every bone
         of another component becomes the ``-index-1`` sentinel, and its index buffers are trimmed
         to the triangles all of whose corners are live. ``false``: the **graph cut** strategy --
         the component takes the triangles the negative-index components leave, shared out among
         the cut components by majority, and every buffer is filtered to the vertices it uses
         @endrst
         */
        bool negativeIndex = false;
    };

    /**
     * @brief Counts worth reporting about one component's split
     */
    struct VGComponentSplitStats {
        std::size_t vertexCount = 0;
        std::size_t keptVertices = 0;
        std::vector<std::size_t> trianglesKept;
        std::vector<std::size_t> trianglesDropped;
        std::size_t renormalised = 0;
        std::size_t neighbourSkinned = 0;
        std::size_t sentinels = 0;
    };

    /**
     * @brief
     @rst
     What one component gets out of a split: the vertices it draws and its buffers over them
     @endrst
     */
    struct VGComponentBuffers {
        /**
         * @brief The mod's vertex indices this component draws, ascending (every one for a negative-index component)
         */
        std::vector<std::size_t> vertices;

        /**
         * @brief Per kept vertex, its 4 weights in the component's bones (renormalised for a cut)
         */
        std::vector<std::array<double, 4>> weights;

        /**
         * @brief Per kept vertex, its 4 bone indices in the component's numbering (negative sentinels for negative index)
         */
        std::vector<std::array<long long, 4>> indices;

        /**
         * @brief
         @rst
         Per source index buffer, the triangles this component draws -- renumbered into
         \ref vertices for a cut, in the mod's own numbering for negative index
         @endrst
         */
        std::vector<std::vector<std::array<unsigned long long, 3>>> ibs;

        /**
         * @brief Negative index only: per mod vertex, whether it carries no sentinel
         */
        std::vector<bool> live;

        VGComponentSplitStats stats;
    };

    /**
     * @brief
     @rst
     Splits one mod's geometry across the components of a target skin :raw-html:`<br />`
     :raw-html:`<br />`

     The ``Blend.buf`` decides which component each vertex belongs to, the index buffers decide
     which triangles go where, and every vertex buffer is then filtered to the vertices a component
     keeps -- so the buffers of a mod cannot be split one at a time, which is what makes this a
     grouped resource's job (see :cpp:class:`VGSplitGroupResource`). The strategies mirror
     ``Tools/VGRemapFinder``'s ``ComponentSplit.py``, whose output the Yelan -> YelanTranquil pair
     was confirmed with in game: the negative-index components first draw every triangle all of
     whose corners are live in them, and the cut components share the rest out by majority
     (its ``fill`` mode) :raw-html:`<br />` :raw-html:`<br />`

     Weights are decoded from the file's own 32-bit floats and handled as ``float`` wherever the
     Python original's ``numpy`` did, so a cut component's renormalised blend comes out byte for
     byte the same
     @endrst
     */
    class VGComponentSplit {
        public:
            using Weights = std::vector<std::array<double, 4>>;
            using Indices = std::vector<std::array<long long, 4>>;
            using Triangles = std::vector<std::array<unsigned long long, 3>>;

            /**
             * @brief Constructs a split over decoded buffers
             *
             * @param weights Per vertex, its 4 blend weights
             * @param indices Per vertex, its 4 vertex group indices
             * @param ibs The mod's index buffers, one per drawn object
             * @param specs Every component of the target
             */
            VGComponentSplit(Weights weights, Indices indices, std::vector<Triangles> ibs, std::vector<VGComponentSpec> specs);

            /**
             * @brief Decodes a ``Blend.buf`` into weights and indices
             */
            static std::pair<Weights, Indices> readBlend(BlendFile& blend);

            /**
             * @brief Decodes a ``.ib`` into triangles
             */
            static Triangles readIb(IbFile& ib);

            /**
             * @brief Encodes weights and indices back into ``Blend.buf`` bytes (4 floats then 4 signed ints per line)
             */
            static ByteVec encodeBlend(const Weights& weights, const Indices& indices);

            /**
             * @brief Encodes triangles back into ``.ib`` bytes (3 unsigned ints per triangle)
             */
            static ByteVec encodeIb(const Triangles& triangles);

            /**
             * @brief
             @rst
             Keeps only the given lines of a fixed-stride buffer, in the order given -- how a
             ``Position.buf`` or ``Texcoord.buf`` follows the vertices a cut component kept
             @endrst
             *
             * @throw std::invalid_argument If 'bytesPerLine' is 0 or a line is past the end
             */
            static ByteVec keepLines(const ByteVec& src, std::size_t bytesPerLine, const std::vector<std::size_t>& lines);

            std::size_t vertexCount() const;
            const std::vector<VGComponentSpec>& specs() const;

            /**
             * @brief Splits for one component
             *
             * @throw std::invalid_argument If no component has that name
             */
            VGComponentBuffers split(const std::string& component) const;

        private:
            struct Membership {
                std::vector<std::array<long long, 4>> mapped;
                std::vector<std::array<bool, 4>> inComponent;
            };

            Membership membership(const VGComponentSpec& spec, bool withSecondary) const;
            std::vector<double> forwardShares(const VGComponentSpec& spec) const;
            std::vector<bool> liveVertices(const VGComponentSpec& spec) const;
            VGComponentBuffers splitNegative(const VGComponentSpec& spec) const;
            VGComponentBuffers splitCut(std::size_t column) const;

            Weights weights_;
            Indices indices_;
            std::vector<Triangles> ibs_;
            std::vector<VGComponentSpec> specs_;
            std::vector<double> totals_;
    };
}

#endif
