#ifndef AGRemapCore_VbFile_H
#define AGRemapCore_VbFile_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufFile` :raw-html:`<br />` :raw-html:`<br />`

     Used for handling ``.vb`` (vertex buffer) files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        A GI character's ``.vb`` data does not live in one file -- it is split across a
        ``Position.buf``, a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex. Use
        :cpp:func:`BufFile::merge` to stitch such a set back together, which fills in both the bytes
        and the elements
     @endrst
     */
    class VbFile: public BufFile {
        public:

            /**
             * @brief Constructs a new ``.vb`` file and immediately reads it
             *
             * @param src The source file or bytes for the ``.vb`` file
             * @param elements The sequence of elements within a vertex line, in byte order.
             *      Required rather than defaulted: unlike a :cpp:class:`BlendFile` or a
             *      :cpp:class:`PositionFile`, a ``.vb`` file has no single fixed layout -- how many
             *      texture coordinates it carries varies by mod
             */
            VbFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements);

            /**
             * @brief The number of vertices making up the mod's mesh
             */
            std::size_t getVertexCount() const;

            /**
             * @brief
             @rst
             Makes the header for a dumped *vb.txt* file :raw-html:`<br />` :raw-html:`<br />`

             An element's ``SemanticIndex`` is its occurrence among the elements sharing its
             :cpp:func:`BufType::getName` -- so the first ``TEXCOORD`` is index 0, the next is 1,
             which is how 3dmigoto tells several same-named elements apart
             @endrst
             *
             * @return The header text, ending with the ``vertex-data:`` marker the data section follows
             */
            std::string makeDumpHeader() const;

            /**
             * @brief
             @rst
             The full text for converting this ``.vb`` file into a dumped *vb.txt* file --
             #makeDumpHeader followed by :cpp:func:`BufFile::getDumpStr`'s data section
             :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                This deliberately **hides** :cpp:func:`BufFile::getDumpStr` rather than overriding
                it -- that one is documented as returning only the data section, and code holding a
                plain :cpp:class:`BufFile` should keep getting exactly that
             @endrst
             *
             * @param prefix The buffer name each entry is prefixed with
             *
             * @return The text for the dumped *vb.txt* file
             */
            std::string getDumpStr(const std::string& prefix = "vb0") const;

            /**
             * @brief
             @rst
             Reads a dumped *vb.txt* file's text back into this file's bytes -- the inverse of
             #getDumpStr :raw-html:`<br />` :raw-html:`<br />`

             Unlike :cpp:func:`BufFile::readDumpStr`, which encodes against whatever #getElements the
             file already has, this first rebuilds those elements from the dump's own header (see
             #parseDumpHeader) when there is one -- so a dump can be read straight back without
             being told its layout. A header-less text falls through to the current elements
             @endrst
             *
             * @param text The text of the dumped *vb.txt* file
             *
             * @throws BadBufData if the parsed bytes do not divide evenly into vertex lines
             */
            void readDumpStr(const std::string& text);

            /**
             * @brief
             @rst
             Builds the data types making up an element from the `DXGI format`_ name a dump's header
             gives for it :raw-html:`<br />` :raw-html:`<br />`

             The channels decide *how many* data types there are and how wide each one is, and the
             suffix decides what kind they are -- so ``R32G32B32_FLOAT`` is 3 four-byte floats,
             ``R8G8B8A8_UNORM`` is 4 one-byte `unsigned normalized integers`_, and
             ``R32G32B32A32_SINT`` is 4 four-byte signed integers
             @endrst
             *
             * @param formatName The format name to parse, eg. ``"R32G32B32_FLOAT"``
             *
             * @return The data types making up the element. Empty if the format is not one this
             *      understands
             */
            static std::vector<std::unique_ptr<BufDataType>> parseFormatName(const std::string& formatName);

            /**
             * @brief
             @rst
             Builds the elements of a vertex line out of a dumped *vb.txt* file's header
             :raw-html:`<br />` :raw-html:`<br />`

             The header names each element and gives its `DXGI format`_, which together are
             everything a ``.vb`` file needs -- so a dump can be read back without being told what
             its layout was
             @endrst
             *
             * @param text The text of the dumped *vb.txt* file
             *
             * @return The elements the header declares, in byte order. Empty when the text has no
             *      header to read them from, or when one of its formats could not be parsed
             */
            static std::vector<std::unique_ptr<BufElementType>> parseDumpHeader(const std::string& text);
    };
}

#endif
