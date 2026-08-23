#ifndef AGRemapCore_BufUnorm_H
#define AGRemapCore_BufUnorm_H

#include <cstddef>
#include <memory>
#include <string>

#include "AGRemapCore/model/buffers/BufInt.h"
#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufBaseInt` :raw-html:`<br />` :raw-html:`<br />`

     The type definition for an `unsigned normalized integer`_ number within a ``.buf`` file
     @endrst
     */
    class BufUnorm: public BufBaseInt {
        public:

            /**
             * @brief Constructs a new unsigned-normalized-integer type
             *
             * @param name The name of the type
             * @param size The byte size for the data type
             * @param isBigEndian Whether the type is in big endian mode
             */
            BufUnorm(std::string name, std::size_t size, bool isBigEndian = false);

            /**
             * @brief
             @rst
             Decode the raw bytes to the `floating point`_ value for the `unsigned normalized integer`_

             .. warning::
                Please make sure the number of bytes passed into 'src' matches :cpp:func:`BufDataType::getSize`
             @endrst
             *
             * @param src The raw bytes to decode
             *
             * @return The decoded floating-point value, in the range ``[0, 1]``
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

        private:
            unsigned long long maxValue_;
    };
}

#endif
