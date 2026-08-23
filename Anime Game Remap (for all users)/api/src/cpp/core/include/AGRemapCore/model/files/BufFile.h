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

        private:
            std::string fileType_;
            std::vector<std::unique_ptr<BufElementType>> elements_;
            std::vector<std::string> elementKeys_;
            std::size_t bytesPerLine_ = 0;
    };
}

#endif
