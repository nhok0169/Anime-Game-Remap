#ifndef AGRemapCore_BufFloat_H
#define AGRemapCore_BufFloat_H

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
             * @brief
             @rst
             How :cpp:func:`BufFloat16::encode` turns a wider value into a 16-bit half
             :raw-html:`<br />` :raw-html:`<br />`

             The two are not interchangeable and the difference is measurable: over one WuWa mod's
             texcoord buffer they disagree on **104 halves of 1,508,336**, every one of which is a
             moved UV.
             @endrst
             */
            enum class Rounding {
                /**
                 * @brief Drop the low mantissa bits (``mantissa >> 13``) and flush a subnormal to zero
                 *
                 * The behaviour this type has always had, and the default, so that no buffer the
                 * library already writes moves. Cheap, and NOT an exact round trip -- decoding a half
                 * and encoding it straight back can change it.
                 */
                Truncate,

                /**
                 * @brief
                 @rst
                 Round the mantissa half to even, keep subnormals, and keep a ``NaN`` a ``NaN``
                 :raw-html:`<br />` :raw-html:`<br />`

                 What numpy's ``float16`` cast does, so a fix whose oracle is a Python prototype wants
                 this one :raw-html:`<br />` :raw-html:`<br />`

                 .. note::
                    Under this mode :cpp:func:`BufFloat16::decode` followed by
                    :cpp:func:`BufFloat16::encode` is an **exact identity** for every one of the
                    65536 half bit patterns. That is what makes :cpp:func:`BufFile::fix` -- which
                    re-encodes every line, including the ones no filter touched -- safe to run over
                    a buffer of halves.
                 @endrst
                 */
                NearestEven
            };

            /**
             * @brief Constructs a new 16-bit half-precision floating-point type
             *
             * @param isBigEndian Whether the type is in big endian mode
             * @param rounding How #encode narrows a value to a half (see #Rounding)
             */
            explicit BufFloat16(bool isBigEndian = false, Rounding rounding = Rounding::Truncate);

            /**
             * @brief Retrieves how #encode narrows a value to a half
             *
             * @return The rounding mode
             */
            Rounding getRounding() const;

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

        private:
            Rounding rounding_;
    };
}

#endif
