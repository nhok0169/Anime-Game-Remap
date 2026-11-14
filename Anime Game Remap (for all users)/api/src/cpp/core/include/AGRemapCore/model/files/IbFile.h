#ifndef AGRemapCore_IbFile_H
#define AGRemapCore_IbFile_H

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
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufFile` :raw-html:`<br />` :raw-html:`<br />`

     Used for handling ``.ib`` (index buffer) files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Where a ``.buf`` file is split into *vertex lines*, a ``.ib`` file is split into *face
        lines* -- each line names the vertices making up one triangular face of the mod's mesh.
        Every face in a 3dmigoto mod is atomically a triangle, so a line is always
        #VerticesPerTriangle unsigned integers -- 32-bit by default, and 16-bit for a buffer its
        ``.ini`` declares ``DXGI_FORMAT_R16_UINT`` (see #bytesPerIndexOf)

     .. warning::
        **The width cannot be inferred from the file, and guessing is worse than failing.** A
        16-bit buffer whose byte count happens to divide by 12 reads "successfully" as 32-bit, at
        half the index count and with values far past the end of the mesh -- measured on one mod:
        5520 indices peaking at 154,011,948 on 10137 vertices, where the declared reading is 11040
        indices that are all valid. Pass the width the ``.ini`` declares
     @endrst
     */
    class IbFile: public BufFile {
        public:

            /**
             * @brief The name given to the single element making up a face line
             */
            static const std::string TriangleBufElementKey;

            /**
             * @brief How many vertices make up a face -- 3, since every face is a triangle
             */
            static const std::size_t VerticesPerTriangle;

            /**
             * @brief Constructs a new ``.ib`` file and immediately reads it
             *
             * @param src The source file or bytes for the ``.ib`` file
             */
            explicit IbFile(BinarySrc src, std::size_t bytesPerIndex = 4);

            /**
             * @brief
             @rst
             How many bytes one index takes in a buffer declared with this ``format`` -- 2 for
             ``DXGI_FORMAT_R16_UINT``, and 4 for ``DXGI_FORMAT_R32_UINT``, an absent format, or
             anything else, which is what every mod not saying otherwise uses
             @endrst
             *
             * @param dxgiFormat The ``format`` `KVP`_'s value from the buffer's `section`_
             */
            static std::size_t bytesPerIndexOf(const std::string& dxgiFormat);

            /**
             * @brief The number of triangular faces making up the mod's mesh
             */
            std::size_t getTriangleCount() const;

            /**
             * @brief
             @rst
             The number of vertex indices in the file -- #VerticesPerTriangle times
             #getTriangleCount
             @endrst
             */
            std::size_t getIndexCount() const;

            /**
             * @brief
             @rst
             Every vertex index in the file, in order -- the list a ``drawindexed = <count>, <first>, 0``
             addresses, with ``first`` counted in indices rather than triangles
             @endrst
             *
             * @return #getIndexCount indices
             */
            std::vector<long long> getIndices() const;

            /**
             * @brief
             @rst
             The bytes of an index buffer holding one span of this file's indices with every
             triangle wound the other way -- the same vertices, presented to the rasterizer as their
             back faces :raw-html:`<br />` :raw-html:`<br />`

             Each triangle ``(a, b, c)`` of the span becomes ``(a, c, b)``. The span is laid out from
             index 0, so a draw of ``count, first`` into this file is ``count, first - firstIndex``
             into the result. Written at this file's own index width
             @endrst
             *
             * @param firstIndex The index the span starts at
             * @param indexCount How many indices the span holds. A trailing part-triangle is left zero
             *
             * @return The bytes, 'indexCount' indices long
             *
             * @throws std::out_of_range if the span runs past the end of the file
             */
            ByteVec windingReversed(std::size_t firstIndex, std::size_t indexCount) const;

            /**
             * @brief
             @rst
             A draw range of an index buffer, moved onto a copy of it that keeps only some of its triangles
             :raw-html:`<br />` :raw-html:`<br />`

             'keptTriangleIds' lists, ascending, the triangle of the ORIGINAL buffer at each place of the copy. The
             range's new first index is the number of kept triangles before it and its new count the number inside
             it, both times 3 -- so a range over removed triangles draws only what is left of it, and every range
             after a removed triangle moves down

             .. code-block:: ini

                ; keptTriangleIds = [0, 1, 2, 5, 6, 7]: triangles 3 and 4 were removed
                9, 0   ; -> 9, 0
                9, 9   ; -> 3, 9   (only triangle 5 of 3..5 is kept)
             @endrst
             *
             * @param indexCount How many indices the range holds
             * @param firstIndex The index the range starts at
             * @param keptTriangleIds The original triangle at each place of the copy, ascending
             *
             * @return The range in the copy, as ``(index count, first index)``
             */
            static std::pair<std::size_t, std::size_t> remapDrawRange(std::size_t indexCount, std::size_t firstIndex,
                                                                      const std::vector<std::size_t>& keptTriangleIds);

            /**
             * @brief Makes the header for a dumped *ib.txt* file
             *
             * @param firstIndex The index this file's first vertex index is numbered from. A mod's
             *      faces are spread over several ``.ib`` files (one per mod object), which a dump
             *      numbers continuously -- so each file after the first starts where the previous
             *      one's #getIndexCount left off
             *
             * @return The header text
             */
            std::string makeDumpHeader(long long firstIndex = 0) const;

            /**
             * @brief
             @rst
             The full text for converting this ``.ib`` file into a dumped *ib.txt* file --
             #makeDumpHeader followed by :cpp:func:`BufFile::getFlatDumpStr` :raw-html:`<br />`
             :raw-html:`<br />`

             Unlike :cpp:func:`BufFile::getDumpStr` this is a *complete* dump, and its data section
             is a ``.ib`` file's own flat, space-separated form rather than the per-element form a
             vertex buffer's data uses
             @endrst
             *
             * @param firstIndex The index this file's first vertex index is numbered from (see #makeDumpHeader)
             *
             * @return The text for the dumped *ib.txt* file
             */
            std::string getDumpStr(long long firstIndex = 0) const;

            /**
             * @brief
             @rst
             Reads a dumped *ib.txt* file's text back into this file's bytes -- the inverse of
             #getDumpStr, and a convenience for :cpp:func:`BufFile::readFlatDumpStr` (an index
             buffer's dump uses the flat form). The header is skipped, so a whole dump file can be
             handed straight in
             @endrst
             *
             * @param text The text of the dumped *ib.txt* file
             *
             * @throws BadBufData if the parsed bytes do not divide evenly into face lines
             */
            void readDumpStr(const std::string& text);

        private:
            static std::vector<std::unique_ptr<BufElementType>> defaultElements(std::size_t bytesPerIndex = 4);
    };
}

#endif
