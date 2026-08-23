#ifndef AGRemapCore_BufElementType_H
#define AGRemapCore_BufElementType_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufType.h"
#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufType` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for an element within a ``.buf`` file -- a single named field (eg. a
     vertex's position, or its blend weights) composed of one or more :cpp:class:`BufDataType`\\s
     laid out back-to-back
     @endrst
     */
    class BufElementType: public BufType {
        public:

            /**
             * @brief Constructs a new element type
             *
             * @param name The name of the element
             * @param formatName The name of the type format according to 3dmigoto
             * @param dataTypes The data types composed within the element, in byte order. Ownership
             *      of each data type is transferred into this element
             */
            BufElementType(std::string name, std::string formatName, std::vector<std::unique_ptr<BufDataType>> dataTypes);

            // Owns its BufDataTypes via unique_ptr, so a real copy needs a deep clone (each
            // BufDataType's own BufDataType::clone(), see that method's doc comment for why a
            // clone rather than a shared/moved pointer) -- unlike AGRemapCore::IfTemplate's
            // deliberately move-only vector<unique_ptr<...>> of owned, identity-bearing parts, a
            // BufDataType is a small shareable value, and this codebase's own real usage
            // (BufDataTypes.py's DeferredEnum-cached values, reused across many different
            // BufElementType definitions) requires that the same source BufDataType survive being
            // copied into more than one BufElementType.
            BufElementType(const BufElementType& other);
            BufElementType& operator=(const BufElementType& other);
            BufElementType(BufElementType&&) = default;
            BufElementType& operator=(BufElementType&&) = default;

            /**
             * @brief The name of the type format according to 3dmigoto
             */
            const std::string& getFormatName() const;

            /**
             * @brief Sets the name of the type format according to 3dmigoto
             *
             * @param formatName The new format name
             */
            void setFormatName(std::string formatName);

            /**
             * @brief The data types composed within the element
             */
            const std::vector<std::unique_ptr<BufDataType>>& getDataTypes() const;

            /**
             * @brief Sets the data types composed within the element (recomputes #getSize)
             *
             * @param dataTypes The new data types. Ownership of each data type is transferred into
             *      this element
             */
            void setDataTypes(std::vector<std::unique_ptr<BufDataType>> dataTypes);

            /**
             * @brief The byte size for the element (the sum of every composing data type's size)
             */
            std::size_t getSize() const;

            /**
             * @brief Decodes a raw sequence of bytes into one decoded value per data type
             *      composing this element
             *
             * @param src The raw bytes to decode (its length should match #getSize)
             *
             * @return The decoded values, one per entry of #getDataTypes, in the same order
             */
            std::vector<BufValue> decode(const ByteVec& src) const;

            /**
             * @brief Encodes the decoded values for this element back to raw bytes
             *
             * @param src The decoded values to encode, one per entry of #getDataTypes -- if fewer
             *      values than data types are given, only the leading data types are encoded
             *      (mirroring the pure-Python original's ``min(len(dataTypes), len(src))`` guard)
             *
             * @return The encoded raw bytes
             */
            ByteVec encode(const std::vector<BufValue>& src) const;

        private:
            std::string formatName_;
            std::vector<std::unique_ptr<BufDataType>> dataTypes_;
            std::size_t size_;

            static std::size_t computeSize(const std::vector<std::unique_ptr<BufDataType>>& dataTypes);
            static std::vector<std::unique_ptr<BufDataType>> cloneAll(const std::vector<std::unique_ptr<BufDataType>>& dataTypes);
    };
}

#endif
