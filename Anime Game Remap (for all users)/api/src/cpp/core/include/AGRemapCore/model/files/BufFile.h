#ifndef AGRemapCore_BufFile_H
#define AGRemapCore_BufFile_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/buffers/BufValue.h"
#include "AGRemapCore/model/files/BinaryFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     A class to handle ``.buf`` files :raw-html:`<br />` :raw-html:`<br />`

     A ``.buf`` file is a binary file made up of a sequence of same-sized "lines" (one line per
     vertex), each one composed of the same sequence of :cpp:class:`BufElementType`\\s -- there is
     no header or footer, just the lines themselves back-to-back
     @endrst
     */
    class BufFile: public BinaryFile {
        public:

            /**
             * @brief
             @rst
             A function used to process the decoded data for one line of a ``.buf`` file, used by
             :cpp:func:`fix` :raw-html:`<br />` :raw-html:`<br />`

             Takes in, in order:

             #. The decoded data for the line
             #. The starting byte index of the line within the file
             #. The line index being processed, as ``i / bytesPerLine`` -- kept as a
                `floating point`_ value rather than truncated to an integer, matching the pure-Python
                original's own (`true division`_-based) computation of this argument exactly
             #. The size of each line, in bytes

             and returns the (possibly modified) decoded data for the line
             @endrst
             */
            using Filter = std::function<BufLineData(const BufLineData&, long long, double, long long)>;

            /**
             * @brief
             @rst
             The result of :cpp:func:`fix` -- either the raw bytes for the fixed file (when no
             output file path was given) or the output file path that was written to (echoed back,
             matching the pure-Python original's own return convention)
             @endrst
             */
            using FixResult = std::variant<std::string, ByteVec>;

            /**
             * @brief
             @rst
             Every line's value for one single :cpp:class:`BufDataType` within one element, laid out
             contiguously -- one entry per line, all of the same type :raw-html:`<br />`
             :raw-html:`<br />`

             Which of the three alternatives a column uses is decided by the
             :cpp:class:`BufDataType` it belongs to, never by the bytes: a signed integer type
             always yields ``long long``, an unsigned one ``unsigned long long`` and a
             `floating point`_ one ``double``, exactly matching the alternative that type's
             :cpp:func:`BufDataType::decode` returns
             @endrst
             */
            using BufColumn = std::variant<std::vector<long long>, std::vector<unsigned long long>, std::vector<double>>;

            /**
             * @brief
             @rst
             One decoded column of a ``.buf`` file (see :cpp:type:`BufColumn`), tagged with which
             element it belongs to and its position within that element
             @endrst
             */
            struct BufColumnData {
                /**
                 * @brief The key of the element this column belongs to (see #getElements)
                 */
                std::string elementKey;

                /**
                 * @brief The position of this column's data type within that element
                 */
                std::size_t valueInd = 0;

                /**
                 * @brief This column's value for every line, in line order
                 */
                BufColumn values;
            };

            /**
             * @brief Constructs a new ``.buf`` file and immediately reads it (see #read)
             *
             * @param src The source file or bytes for the ``.buf`` file
             * @param elements The sequence of elements within the ``.buf`` file. Ownership of each
             *      element is transferred into this file
             * @param fileType The name for the type of ``.buf`` file
             *
             * @throws BufFileNotRecognized if 'src' holds a file path that cannot be read as a
             *      valid ``.buf`` file of this format
             * @throws BadBufData if 'src' holds raw bytes that are not valid for this format
             */
            BufFile(BinarySrc src, std::vector<std::unique_ptr<BufElementType>> elements, std::string fileType = "Buffer");

            virtual ~BufFile() = default;

            // Owns its BufElementTypes via unique_ptr -- see BufElementType's identical note for
            // why these are explicit rather than left implicit.
            BufFile(const BufFile&) = delete;
            BufFile& operator=(const BufFile&) = delete;
            BufFile(BufFile&&) = default;
            BufFile& operator=(BufFile&&) = default;

            /**
             * @brief The name for the type of ``.buf`` file
             */
            const std::string& getFileType() const;

            /**
             * @brief Sets the name for the type of ``.buf`` file
             *
             * @param fileType The new file type name
             */
            void setFileType(std::string fileType);

            /**
             * @brief The sequence of elements within the ``.buf`` file
             */
            const std::vector<std::unique_ptr<BufElementType>>& getElements() const;

            /**
             * @brief Sets the sequence of elements within the ``.buf`` file (recomputes #getBytesPerLine)
             *
             * @param elements The new elements. Ownership of each element is transferred into this
             *      file
             */
            void setElements(std::vector<std::unique_ptr<BufElementType>> elements);

            /**
             * @brief The number of bytes per line in the ``.buf`` file
             */
            std::size_t getBytesPerLine() const;

            /**
             * @brief Whether the size of #getData is divisible by #getBytesPerLine
             */
            bool isValid() const;

            /**
             * @brief
             @rst
             Reads the bytes in the ``.buf`` file
             @endrst
             *
             * @return The read bytes
             *
             * @throws BufFileNotRecognized if #getSrc holds a file path that cannot be read as a
             *      valid ``.buf`` file of this format
             * @throws BadBufData if #getSrc holds raw bytes that are not valid for this format
             */
            ByteVec read() override;

            /**
             * @brief Decodes a line (a vertex) within the ``.buf`` file
             *
             * @param src The source bytes to decode
             *
             * @return The decoded values for the line, keyed by each element's key (see #getElements)
             */
            BufLineData decodeLine(const ByteVec& src) const;

            /**
             * @brief Encodes the data about a vertex to their corresponding bytes for the line
             *
             * @param src The corresponding data for the vertex, keyed by each element's key (see #getElements)
             *
             * @return The encoded bytes for the line
             */
            ByteVec encodeLine(const BufLineData& src) const;

            /**
             * @brief Fixes the ``.buf`` file
             *
             * @param fixedFile The file path for the fixed ``.buf`` file. If this is ``std::nullopt``,
             *      the fixed bytes are returned directly instead of being written to a file
             * @param filters The filters to process each line, applied in order
             *
             * @return If 'fixedFile' is ``std::nullopt``, the fixed bytes. Otherwise, 'fixedFile' itself
             */
            FixResult fix(const std::optional<std::string>& fixedFile = std::nullopt, const std::vector<Filter>& filters = {});

            /**
             * @brief
             @rst
             Decodes the whole file at once, **column by column** rather than line by line -- the
             bulk counterpart to :cpp:func:`decodeLine` :raw-html:`<br />` :raw-html:`<br />`

             Where :cpp:func:`decodeLine` builds a fresh keyed map per line, this walks the elements
             in declaration order and writes straight into one contiguous, uniformly typed buffer
             per column, so nothing is hashed, allocated or boxed per line. That is what makes it
             the right shape to hand to a columnar consumer (a ``numpy`` array, a dataframe, a
             column store) -- see the ``BufTools`` Python-side tools built on it :raw-html:`<br />`
             :raw-html:`<br />`

             The columns come back in **declaration order** (element by element, then data type
             within each element), not sorted by element key -- a caller wanting some other order
             sorts what it gets back
             @endrst
             *
             * @return One entry per column, each holding one value per line. Empty when the file
             *      has no elements
             */
            std::vector<BufColumnData> decodeAll() const;

            /**
             * @brief
             @rst
             Merges several other ``.buf`` files into this one, line by line :raw-html:`<br />`
             :raw-html:`<br />`

             A GI character's vertex buffer does not live in one file -- it is split across a
             ``Position.buf``, a ``Blend.buf`` and a ``Texcoord.buf``, one line each per vertex.
             This stitches such a set back together: line *i* of the result is line *i* of every
             source, concatenated in the order given, and this file's #getElements becomes every
             source's elements in that same order (deep-copied, so the sources stay usable)
             :raw-html:`<br />` :raw-html:`<br />`

             The number of lines produced is the **smallest** line count among the sources, so a
             ragged set truncates rather than reading past the end of the shortest file. A null
             source is skipped
             @endrst
             *
             * @param bufFiles The ``.buf`` files to merge, in the byte order their elements should
             *      appear in a line
             *
             * @throws BadBufData if the merged bytes do not divide evenly into lines (only
             *      reachable when a source's own data was not a whole number of lines)
             */
            void merge(const std::vector<const BufFile*>& bufFiles);

            /**
             * @brief
             @rst
             The **data** section of the dump text for this ``.buf`` file -- the text a 3dmigoto
             frame analysis writes, which `Blender`_ can then import :raw-html:`<br />`
             :raw-html:`<br />`

             One line per element per line (vertex), in the elements' declared order, shaped as
             ``prefix[lineInd]+byteOffset elementKey: value, value, ...``, with a blank line between
             lines. Deliberately only the data: a complete dump also needs a header, and that header
             differs by the kind of buffer being dumped :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Every number is formatted exactly the way `Python`_'s ``str`` would format it --
                shortest round-trip for a `floating point`_ value, and `Python`_'s own choice
                between fixed and scientific notation. That is not cosmetic: it keeps this byte
                identical to what the pure-`Python`_ implementation this replaced produced, and to
                what a real frame analysis writes
             @endrst
             *
             * @param prefix The buffer name each entry is prefixed with -- the vertex buffer slot
             *      a real dump was taken from
             *
             * @return The data section of the dump text. Empty when the file has no lines
             */
            std::string getDumpStr(const std::string& prefix = "vb0") const;

            /**
             * @brief
             @rst
             The **data** section of the dump text in an *index buffer*'s flat form -- every one of
             a line's values on one line, separated by 'valueSep', with no element name or byte
             offset :raw-html:`<br />` :raw-html:`<br />`

             This is what a ``.ib`` file's dump looks like (``0 1 2`` per triangular face), as
             opposed to the per-element form :cpp:func:`getDumpStr` produces for a vertex buffer
             @endrst
             *
             * @param valueSep What to put between two values on the same line
             *
             * @return The data section of the dump text. Empty when the file has no lines
             */
            std::string getFlatDumpStr(const std::string& valueSep = " ") const;

            /**
             * @brief
             @rst
             Reads dump text back into this file's bytes -- the inverse of :cpp:func:`getDumpStr`
             :raw-html:`<br />` :raw-html:`<br />`

             The values are encoded with this file's **current** #getElements, one text line per
             element and one blank-line-separated block per line (vertex), so a round trip through
             :cpp:func:`getDumpStr` and back returns the bytes it started with :raw-html:`<br />`
             :raw-html:`<br />`

             A complete dump file works too, not just the data section :cpp:func:`getDumpStr`
             returns: anything up to and including a ``vertex-data:`` marker is skipped. Each line's
             values are taken from after its last ``:``, so the ``prefix[i]+offset elementKey:``
             part is ignored rather than having to match :raw-html:`<br />` :raw-html:`<br />`

             A block with fewer values than the elements need is zero-filled and extra values are
             dropped, so every block always contributes exactly #getBytesPerLine bytes
             @endrst
             *
             * @param text The dump text to read
             *
             * @throws BadBufData if the encoded bytes do not divide evenly into lines
             */
            void readDumpStr(const std::string& text);

            /**
             * @brief
             @rst
             Reads *index buffer* dump text back into this file's bytes -- the inverse of
             :cpp:func:`getFlatDumpStr` :raw-html:`<br />` :raw-html:`<br />`

             One text line per line of the file, its values separated by 'valueSep'. A complete dump
             file works too: a header line is recognised by containing a ``:`` (every line of a
             ``.ib`` dump's header does, and none of its data lines do) and skipped
             @endrst
             *
             * @param text The dump text to read
             * @param valueSep What separates two values on the same line
             *
             * @throws BadBufData if the encoded bytes do not divide evenly into lines
             */
            void readFlatDumpStr(const std::string& text, const std::string& valueSep = " ");

            /**
             * @brief
             @rst
             Encodes whole columns back into the file's bytes -- the inverse of
             :cpp:func:`decodeAll`, and the bulk counterpart to :cpp:func:`encodeLine`
             :raw-html:`<br />` :raw-html:`<br />`

             The columns are matched to the file's current #getElements by their
             :cpp:member:`BufColumnData::elementKey`/:cpp:member:`BufColumnData::valueInd`, so their
             order in 'columns' does not matter; a column the file has no data type for is ignored,
             and a data type with no matching column encodes as 0. The number of lines produced is
             the longest column's length :raw-html:`<br />` :raw-html:`<br />`

             Like the pure-Python tools built on it, this sets #getSrc to the newly encoded bytes
             and re-reads from it, so the usual :cpp:func:`read` validation applies
             @endrst
             *
             * @param columns The columns to encode, as produced by \ref decodeAll
             *
             * @throws BadBufData if the encoded bytes do not divide evenly into lines for the
             *      file's current #getElements
             */
            void encodeAll(const std::vector<BufColumnData>& columns);

        private:
            std::string fileType_;
            std::vector<std::unique_ptr<BufElementType>> elements_;
            std::vector<std::string> elementKeys_;
            std::size_t bytesPerLine_ = 0;
    };
}

#endif
