#ifndef AGRemapCore_BufInt_H
#define AGRemapCore_BufInt_H

#include <cstddef>
#include <memory>
#include <string>

#include "AGRemapCore/model/buffers/BufDataType.h"
#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufDataType` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for some generic integer type within a ``.buf`` file, at most 8 bytes wide
     (see :cpp:class:`BufDataType`'s class-level warning)
     @endrst
     */
    class BufBaseInt: public BufDataType {
        public:

            /**
             * @brief Constructs a new integer type
             *
             * @param name The name of the type
             * @param size The byte size for the data type
             * @param isBigEndian Whether the type is in big endian mode
             * @param isSigned Whether the type is signed
             */
            BufBaseInt(std::string name, std::size_t size, bool isBigEndian = false, bool isSigned = true);

            /**
             * @brief Whether the data type is signed
             */
            bool getIsSigned() const;

            /**
             * @brief
             @rst
             Decode the raw bytes to an integer

             .. warning::
                Please make sure the number of bytes passed into 'src' matches :cpp:func:`BufDataType::getSize`
             @endrst
             *
             * @param src The raw bytes to decode
             *
             * @return The decoded integer, held as ``long long`` if :cpp:func:`getIsSigned` is
             *      ``true`` or ``unsigned long long`` otherwise
             */
            BufValue decode(const ByteVec& src) const override;

            /**
             * @brief
             @rst
             Encodes an integer back to raw bytes

             .. warning::
                Please make sure 'src' is within the acceptable range for the type
             @endrst
             *
             * @param src The integer to encode (either alternative of :cpp:type:`BufValue` is
             *      accepted regardless of :cpp:func:`getIsSigned` -- a floating-point alternative
             *      is truncated towards zero)
             *
             * @return The encoded raw bytes
             */
            ByteVec encode(const BufValue& src) const override;

            std::unique_ptr<BufDataType> clone() const override;

        private:
            bool isSigned_;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufBaseInt` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for some signed integer type within a ``.buf`` file
     @endrst
     */
    class BufSignedInt: public BufBaseInt {
        public:

            /**
             * @brief Constructs a new signed integer type
             *
             * @param name The name of the type
             * @param size The byte size for the data type
             * @param isBigEndian Whether the type is in big endian mode
             */
            explicit BufSignedInt(std::string name = "SignedInt32", std::size_t size = 4, bool isBigEndian = false);

            std::unique_ptr<BufDataType> clone() const override;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufBaseInt` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for some unsigned integer type within a ``.buf`` file
     @endrst
     */
    class BufUnSignedInt: public BufBaseInt {
        public:

            /**
             * @brief Constructs a new unsigned integer type
             *
             * @param name The name of the type
             * @param size The byte size for the data type
             * @param isBigEndian Whether the type is in big endian mode
             */
            explicit BufUnSignedInt(std::string name = "UnsignedInt32", std::size_t size = 4, bool isBigEndian = false);

            std::unique_ptr<BufDataType> clone() const override;
    };
}

#endif
