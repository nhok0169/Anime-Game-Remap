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
         to the head alone still collapses :raw-html:`<br />` :raw-html:`<br />`

         On a **cut** component these are STAND-INS: a kept vertex's weight on another component's
         group goes to the stand-in bone instead of being dropped and renormalised away. Where one
         surface is cut between two components, each side of the seam otherwise keeps only its own
         half of the weights, and the two copies of every seam point follow different bones --
         Neuvillette5's cape tore open across the back. They never decide which component takes a
         triangle; only \ref remap does. Empty (every cut component until 2026-09-25): foreign
         weight is dropped, as before
         @endrst
         */
        std::unordered_map<long long, long long> secondary;

        /**
         * @brief
         @rst
         For a **cut** component: the least share of a vertex's weight that must sit on
         \ref remap's groups for the component to CLAIM the vertex, from ``0`` to ``1``
         :raw-html:`<br />` :raw-html:`<br />`

         Ownership is otherwise a plain majority, which puts the seam between two components exactly
         where a surface's weight is split half and half -- the one place both sides lose the most. A
         component that should take only what is clearly its own (a coat's hanging tails, not the
         back panel they are blended into) sets a high share: a vertex below it goes to the next
         component that claims it, and the seam moves to where the weights are clean. A vertex no
         component can claim falls back to the plain majority. **Default**: ``0``, the plain majority
         @endrst
         */
        double claimShare = 0.0;

        /**
         * @brief
         @rst
         For a **cut** component: how many rings of its NEIGHBOURS' triangles it draws as well, past
         its own edge :raw-html:`<br />` :raw-html:`<br />`

         Where one surface is cut between two components, the two sides of the seam are skinned by
         different bones -- no two components share a bone -- and when the skin poses they pull
         apart, showing whatever is behind (Neuvillette5's cape, torn across the back). A band of
         overlap is drawn by BOTH components, so a gap narrower than the band is covered by the other
         side's copy. It never changes which component owns a triangle: the band is drawn in
         addition, skinned with this component's bones (and its \ref secondary stand-ins), and a
         ring is one step through a shared vertex. **Default**: ``0``, no overlap
         @endrst
         */
        std::size_t overlapRings = 0;

        /**
         * @brief
         @rst
         For a **cut** component: the source index buffers (by position in the split's list) whose
         triangles get a MIRRORED INNER LAYER :raw-html:`<br />` :raw-html:`<br />`

         Single-layer cloth shows its back faces from inside -- a skirt's inner side -- and whether
         a back face renders as cloth is up to the target's shader: Neuvillette's shades it like the
         outside, NeuvilletteMelusent's lights it like rim light, flat light blue (Neuvillette2's inner
         skirt, 2026-09-26). The layer gives each such triangle a front-facing twin seen from the
         other side: every corner is copied once (same weights, same source vertex -- flagged in
         :cpp:member:`VGComponentBuffers::mirrored`, so the vertex buffers' writer can turn its normal
         round, see :cpp:func:`VGComponentSplit::mirrorPositionLine`), and each triangle is followed
         by its copy wound the other way. The copy carries the SAME source triangle id in
         :cpp:member:`VGComponentBuffers::keptTriangleIds`, so a mod's own draw range takes both.
         Ignored on a negative-index component, whose vertex buffers are not rewritten. **Default**:
         empty, no layer
         @endrst
         */
        std::vector<std::size_t> mirroredIbs;

        /**
         * @brief
         @rst
         For a cut component with \ref mirroredIbs: how far behind a mirrored triangle to look for a layer of the
         mesh facing the other way, in model units -- a triangle so BACKED gets no twin (see
         :cpp:func:`InnerLayerOutline::backed`) :raw-html:`<br />` :raw-html:`<br />`

         Cloth modelled with its own lining needs no inner layer, and a twin moved inward from it pokes through the
         lining a few millimetres behind: Lumine2's coat on LumineHeaven showed flat grey polygons over its flaps,
         and they went with the backed twins (in game, 2026-09-29). Needs the mod's positions, handed over by
         :cpp:func:`VGComponentSplit::setGeometry`; without them every triangle is mirrored. A triangle only PARTLY
         over a lining keeps its twin, moved inward no further than half way to the lining
         (:cpp:member:`VGComponentBuffers::mirrorLimits`). **Default**: ``0``, every triangle of a mirrored buffer
         gets its twin at the full offset
         @endrst
         */
        float mirrorBackedReach = 0.0f;

        /**
         * @brief
         @rst
         For a **cut** component: source groups whose weight is SHARED among several of the component's bones,
         as ``{source group: [(bone, share), ...]}`` -- applied after the remap, over the vertex's final
         weights, the shares summing to 1 :raw-html:`<br />` :raw-html:`<br />`

         A cloth part of the source with no counterpart on the target rides ONE bone of it, and either choice
         can be wrong: Neuvillette3's front coat flap on the skin's pelvis (rigid) went through the leg as it
         stepped, and on its thigh (following) swung its face round and showed the lining (2026-09-26). Shared
         between the two, a link moves part of the way with each. A vertex left with more than 4 influences
         keeps its 4 largest, renormalised. **Default**: empty
         @endrst
         */
        std::unordered_map<long long, std::vector<std::pair<long long, double>>> splitGroups;

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
        std::size_t overlapTriangles = 0;
        std::size_t sentinels = 0;
        std::size_t mirroredVertices = 0;
        std::size_t mirroredTriangles = 0;
        std::size_t mirrorBacked = 0;
        std::size_t splitVertices = 0;
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
         * @brief
         @rst
         Per source index buffer, the SOURCE index of every triangle in \ref ibs, ascending --
         what a mod's own ``drawindexed = <count>, <start>, 0`` ranges have to be remapped through,
         since the split removes triangles from the middle of an object as well as its end
         @endrst
         */
        std::vector<std::vector<std::size_t>> keptTriangleIds;

        /**
         * @brief Negative index only: per mod vertex, whether it carries no sentinel
         */
        std::vector<bool> live;

        /**
         * @brief
         @rst
         Per entry of \ref vertices: whether it is a MIRRORED copy -- the inner layer of
         :cpp:member:`VGComponentSpec::mirroredIbs`, whose position line the writer turns round.
         Empty when the component has no layer
         @endrst
         */
        std::vector<bool> mirrored;

        /**
         * @brief
         @rst
         Per entry of \ref vertices, for a MIRRORED copy: the most it may move inward, half the distance to the
         lining facing the other way behind it (:cpp:func:`InnerLayerOutline::backed`'s partial case), or ``-1`` for
         no limit. Empty when :cpp:member:`VGComponentSpec::mirrorBackedReach` did not apply. A twin kept short of a
         lining stays hidden behind both surfaces, where one moved the full offset came out in front of it
         @endrst
         */
        std::vector<float> mirrorLimits;

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

            /**
             * @brief
             @rst
             A GIMI ``Position.buf`` line (``POSITION`` float3, ``NORMAL`` float3, ``TANGENT`` float4)
             for the mirrored inner layer: the normal turned round, and the position moved ``offset``
             model units against the ORIGINAL normal -- to the inside, so the layer is nearer a viewer
             who sees the back face and behind the surface from the outside, and never ties with it in
             depth. The tangent is kept. A line shorter than the normal is returned as it is
             @endrst
             *
             * @param line The source line
             * @param offset How far inward, in model units
             *
             * @return The mirrored line
             */
            static ByteVec mirrorPositionLine(const ByteVec& line, float offset);

            std::size_t vertexCount() const;
            const std::vector<VGComponentSpec>& specs() const;

            /**
             * @brief
             @rst
             Hands the split the mod's own positions and normals, per source vertex -- what
             :cpp:member:`VGComponentSpec::mirrorBackedReach` asks about. Without them (or with the wrong count)
             every triangle of a mirrored buffer is mirrored
             @endrst
             */
            void setGeometry(std::vector<std::array<float, 3>> positions, std::vector<std::array<float, 3>> normals);

            /**
             * @brief Whether splitting for 'component' asks about the mod's geometry -- see \ref setGeometry
             */
            bool needsGeometry(const std::string& component) const;

            /**
             * @brief
             @rst
             \ref setGeometry from the mod's own ``Position.buf`` bytes, one line per source vertex. EVERY split whose
             output has to agree -- the one an ``.ini`` takes its counts from and the one that writes the buffers --
             reads it this way
             @endrst
             *
             * @return Whether the buffer had a normal on every line; if not, nothing is set
             */
            bool readGeometry(const ByteVec& positionBuffer);

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
            void addMirroredLayer(VGComponentBuffers& result, const VGComponentSpec& spec) const;

            Weights weights_;
            Indices indices_;
            std::vector<Triangles> ibs_;
            std::vector<VGComponentSpec> specs_;
            std::vector<double> totals_;
            std::vector<std::array<float, 3>> positions_;
            std::vector<std::array<float, 3>> normals_;
    };
}

#endif
