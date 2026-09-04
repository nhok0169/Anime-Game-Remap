#ifndef AGRemapCore_IbFile_H
#define AGRemapCore_IbFile_H

#include <cstddef>
#include <memory>
#include <string>
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
        #VerticesPerTriangle 32-bit unsigned integers
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
            explicit IbFile(BinarySrc src);

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
            static std::vector<std::unique_ptr<BufElementType>> defaultElements();
    };
}

#endif
