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

#ifndef AGRemapCore_InnerLayerOutline_H
#define AGRemapCore_InnerLayerOutline_H

#include <array>
#include <cstddef>
#include <vector>

#include "AGRemapCore/model/buffers/BufValue.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     Which vertices of a mesh's INNER layers should draw no outline -- the vertex colour's alpha, the outline's
     width, set to 0 :raw-html:`<br />` :raw-html:`<br />`

     GI's outline pass redraws a mesh as a shell pushed out along each vertex's outline normal, with its front
     faces culled, so what shows is the shell's back faces around the silhouette. Hair built of two-sided sheets
     whose layers sit close together has a problem with that on a skin whose outline sits further out than the
     character's own: the INNER face's shell -- the face turned towards the body, or covered by another layer --
     comes out in front of the outer face, as small dark shards all over the hair (Yaoyao5 on YaoyaoBamboo,
     2026-09-28). The outer faces draw the silhouette on their own, so the inner ones can lose their outline and
     nothing visible goes with it :raw-html:`<br />` :raw-html:`<br />`

     A target triangle is INNER when at least two of its corners are :cpp:func:`covered` or, with
     :cpp:member:`facingAxis`, when its face points in towards the vertical axis through the target triangles'
     centre (hair hangs round the head). The decision is per TRIANGLE and takes all three corners: decided per
     vertex, a triangle with one corner at the mod's width and two at 0 stretches its shell into a wedge, and an
     inner triangle's wedge is back-facing, so it draws -- one such wedge showed as a dark rectangle on a front
     lock
     @endrst
     */
    struct InnerLayerOutline {
        using Vec3 = std::array<float, 3>;
        using Triangles = std::vector<std::array<unsigned long long, 3>>;

        /**
         * @brief How far along its normal a vertex looks for a layer covering it, in model units. **Default**: ``0.1``
         */
        float reach = 0.1f;

        /**
         * @brief
         @rst
         Whether a triangle facing in towards the vertical axis through the targets' centre is inner too.
         **Default**: ``true`` -- a lock hanging in front of the shoulder has nothing of the mesh along the normal
         of its body side, and only this finds it
         @endrst
         */
        bool facingAxis = true;

        /**
         * @brief
         @rst
         How far in a triangle's face has to point to count as facing the axis: the cosine between its normal and
         the horizontal direction out from the axis must be below ``-facingCos``. **Default**: ``0.2``
         @endrst
         */
        float facingCos = 0.2f;

        /**
         * @brief
         @rst
         Whether each target vertex's NORMAL runs into an occluder triangle (one it is not a corner of) within
         :cpp:member:`reach`
         @endrst
         *
         * @param positions Per vertex, its position
         * @param normals Per vertex, its normal (need not be unit length)
         * @param occluders Every triangle that can cover a layer -- the whole mesh
         * @param targets The triangles whose corners are asked about
         *
         * @return Per vertex of 'positions'; ``false`` for a vertex no target triangle uses
         */
        std::vector<bool> covered(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                                  const std::vector<const Triangles*>& occluders, const std::vector<const Triangles*>& targets) const;

        /**
         * @brief The vertices whose outline goes -- every corner of every inner target triangle
         *
         * @param positions Per vertex, its position
         * @param normals Per vertex, its normal
         * @param occluders Every triangle that can cover a layer -- the whole mesh
         * @param targets The triangles that may lose their outline -- the hair
         *
         * @return Per vertex of 'positions'
         */
        std::vector<bool> find(const std::vector<Vec3>& positions, const std::vector<Vec3>& normals,
                               const std::vector<const Triangles*>& occluders, const std::vector<const Triangles*>& targets) const;

        /**
         * @brief
         @rst
         Reads positions and normals out of a GIMI ``Position.buf`` (``POSITION`` float3, ``NORMAL`` float3 at
         byte 12, any stride of at least 24)
         @endrst
         *
         * @throw std::invalid_argument If the stride is under 24 or the buffer is not a whole number of lines
         */
        static void readPositions(const ByteVec& buffer, std::size_t stride, std::vector<Vec3>& positions, std::vector<Vec3>& normals);
    };
}

#endif
