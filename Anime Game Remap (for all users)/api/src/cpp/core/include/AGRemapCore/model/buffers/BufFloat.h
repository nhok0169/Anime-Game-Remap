#ifndef AGRemapCore_BufFloat_H
#define AGRemapCore_BufFloat_H

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

     The type definition for a generic 32-bit IEEE 754 `floating point`_ number within a ``.buf``
     file
     @endrst
     */
    class BufBaseFloat: public BufDataType {
        public:

            /**
             * @brief Constructs a new floating-point type
             *
             * @param name The name of the type
             * @param size The byte size for the data type
             * @param isBigEndian Whether the type is in big endian mode
             */
            BufBaseFloat(std::string name, std::size_t size, bool isBigEndian = false);

            /**
             * @brief
             @rst
             Decode the raw bytes to a 32-bit `floating point`_ number

             .. warning::
                Please make sure the number of bytes passed into 'src' matches :cpp:func:`BufDataType::getSize`
             @endrst
             *
             * @param src The raw bytes to decode
             *
             * @return The decoded floating-point value, widened to ``double``
             */
            BufValue decode(const ByteVec& src) const override;

            /**
             * @brief
             @rst
             Encodes the `floating point`_ back to raw bytes

             .. warning::
                Please make sure 'src' is within the acceptable range for the type
             @endrst
             *
             * @param src The floating-point value to encode
             *
             * @return The encoded raw bytes
             */
            ByteVec encode(const BufValue& src) const override;

            std::unique_ptr<BufDataType> clone() const override;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufBaseFloat` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for a 32-bit `floating point`_ number within a ``.buf`` file
     @endrst
     */
    class BufFloat: public BufBaseFloat {
        public:

            /**
             * @brief Constructs a new 32-bit floating-point type
             *
             * @param isBigEndian Whether the type is in big endian mode
             */
            explicit BufFloat(bool isBigEndian = false);

            std::unique_ptr<BufDataType> clone() const override;
    };

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufBaseFloat` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for a 16-bit `half precision floating point`_ number within a ``.buf`` file
     @endrst
     */
    class BufFloat16: public BufBaseFloat {
        public:

            /**
             * @brief Constructs a new 16-bit half-precision floating-point type
             *
             * @param isBigEndian Whether the type is in big endian mode
             */
            explicit BufFloat16(bool isBigEndian = false);

            /**
             * @brief
             @rst
             Decode the raw bytes to a 16-bit `half precision floating point`_ number

             .. warning::
                Please make sure the number of bytes passed into 'src' matches :cpp:func:`BufDataType::getSize`
             @endrst
             *
             * @param src The raw bytes to decode
             *
             * @return The decoded floating-point value, widened to ``double``
             */
            BufValue decode(const ByteVec& src) const override;

            /**
             * @brief
             @rst
             Encodes the `floating point`_ back to raw bytes

             .. warning::
                Please make sure 'src' is within the acceptable range for the type
             @endrst
             *
             * @param src The floating-point value to encode
             *
             * @return The encoded raw bytes
             */
            ByteVec encode(const BufValue& src) const override;

            std::unique_ptr<BufDataType> clone() const override;
    };
}

#endif
