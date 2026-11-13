#ifndef AGRemapCore_DrawUVCoverage_H
#define AGRemapCore_DrawUVCoverage_H

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
#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Which texels of a texture the vertices of one ``drawindexed`` range land on :raw-html:`<br />`
     :raw-html:`<br />`

     Built once from a mesh's index list and the U and V of each vertex, then asked per range: of
     the range's distinct vertices, how many sit, at their UV, on a texel a test accepts. That is how
     a fix tells which of a mod's draws paint a given part of an atlas -- the ones whose vertices
     mostly land on texels a material map marks :raw-html:`<br />` :raw-html:`<br />`

     A UV is wrapped into ``[0, 1)`` the way a sampler in wrap mode reads it, and a UV that is not a
     finite number is read as ``0``
     @endrst
     */
    class DrawUVCoverage {
        public:
            /**
             * @brief Whether the texel at (x, y) counts
             */
            using TexelTest = std::function<bool(long long x, long long y)>;

            /**
             * @brief A triangle, as its three vertices
             */
            using Triangle = std::array<long long, 3>;

            /**
             * @brief What \ref count found for one range
             */
            struct Count {
                /**
                 * @brief How many distinct vertices the range draws
                 */
                std::size_t vertices = 0;

                /**
                 * @brief How many of them land on a texel the test accepts
                 */
                std::size_t matched = 0;
            };

            /**
             * @brief Constructs the coverage of a mesh
             *
             * @param indices The mesh's index list -- see :cpp:func:`IbFile::getIndices`
             * @param us Each vertex's U
             * @param vs Each vertex's V
             * @param vertices How many vertices the mesh has. An index at or past it is skipped
             */
            DrawUVCoverage(std::vector<long long> indices, std::vector<double> us, std::vector<double> vs, std::size_t vertices);

            /**
             * @brief
             @rst
             The coverage of a mesh read off its files: the index list of 'indexFile' (:cpp:class:`IbFile`), and the
             first U and V of each vertex of 'texcoordFile', whose stride is its size over 'vertices'
             (:cpp:func:`TexcoordFile::halfElements`)
             @endrst
             *
             * @param indexFile The mesh's index buffer
             * @param texcoordFile The mesh's texcoord buffer, the UVs as 16-bit halves
             * @param vertices How many vertices the mesh has
             *
             * @throws std::runtime_error If 'texcoordFile''s size is not a whole number of vertices of at least
             *      one UV each
             *
             * @return The coverage
             */
            static DrawUVCoverage ofMesh(const std::string& indexFile, const std::string& texcoordFile, std::size_t vertices);

            /**
             * @brief The texel a UV lands on in a texture of 'width' x 'height', wrapped into the texture
             *
             * @param u The U
             * @param v The V
             * @param width The texture's width
             * @param height The texture's height
             *
             * @return (x, y)
             */
            static std::pair<long long, long long> texelOf(double u, double v, long long width, long long height);

            /**
             * @brief Counts one ``drawindexed`` range's distinct vertices, and those landing on texels 'test' accepts
             *
             * @param firstIndex The range's first index
             * @param indexCount How many indices it draws. The part past the end of the index list is not read
             * @param width The width of the texture the texels are in
             * @param height Its height
             * @param test Which texels count
             *
             * @return The counts
             */
            Count count(long long firstIndex, long long indexCount, long long width, long long height, const TexelTest& test) const;

            /**
             * @brief
             @rst
             The ``drawindexed`` ranges more than 'share' of whose distinct vertices land on texels 'test' accepts
             (ef count), in the order given
             @endrst
             *
             * @param ranges The ranges, each ``(index count, first index)``
             * @param width The width of the texture the texels are in
             * @param height Its height
             * @param test Which texels count
             * @param share The share of a range's vertices that must land on them, from 0 to 1
             *
             * @return The ranges
             */
            std::vector<std::pair<long long, long long>> coveredRanges(const std::vector<std::pair<long long, long long>>& ranges,
                                                                       long long width, long long height, const TexelTest& test,
                                                                       double share) const;

            /**
             * @brief
             @rst
             The triangles of 'ranges', in order: every three indices of each range, skipping a triangle with an index
             past the end of the index list or a vertex at or past the mesh's vertex count
             @endrst
             *
             * @param ranges The ranges, each ``(index count, first index)``
             *
             * @return The triangles
             */
            std::vector<Triangle> triangles(const std::vector<std::pair<long long, long long>>& ranges) const;

            /**
             * @brief
             @rst
             The connected pieces of a mesh: triangles that share a vertex are in one piece. A piece is what a
             modeller lays out as one UV island, so it is how a fix names a part of a texture by the geometry
             that paints it
             @endrst
             *
             * @param triangles The triangles
             *
             * @return For each triangle, the number of its piece, from 0, in the order the pieces are first met
             */
            static std::vector<std::size_t> islands(const std::vector<Triangle>& triangles);

            /**
             * @brief
             @rst
             The texels of a texture of 'width' x 'height' the accepted triangles cover and no other triangle of
             'triangles' does, grown 'grow' texels into the texels no triangle covers -- the part of an atlas a
             set of triangles paints, and the gutter round it, never a texel a neighbouring part also paints
             :raw-html:`<br />` :raw-html:`<br />`

             A texel is covered when its centre lies in the triangle at the vertices' UVs. A triangle is
             wrapped into the texture the way a sampler in wrap mode reads it, and a UV that is not a finite
             number is read as ``0``
             @endrst
             *
             * @param triangles The triangles, eg. from #triangles
             * @param accepted For each triangle, whether its texels are in the region. A triangle past its end is not
             * @param width The texture's width
             * @param height Its height
             * @param grow How many texels to grow the region into the uncovered texels round it. **Default**: ``0``
             *
             * @return For each texel, row by row, whether it is in the region
             */
            std::vector<bool> region(const std::vector<Triangle>& triangles, const std::vector<bool>& accepted,
                                     long long width, long long height, int grow = 0) const;

        private:
            std::vector<long long> indices_;
            std::vector<double> us_;
            std::vector<double> vs_;
            std::size_t vertices_;
    };
}

#endif
