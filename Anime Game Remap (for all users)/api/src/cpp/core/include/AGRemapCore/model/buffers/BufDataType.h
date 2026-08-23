#ifndef AGRemapCore_BufDataType_H
#define AGRemapCore_BufDataType_H

#include <cstddef>
#include <memory>

#include "AGRemapCore/model/buffers/BufType.h"
#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufType` :raw-html:`<br />` :raw-html:`<br />`

     The abstract base for an elementary data type within a ``.buf`` file (eg. a single integer or
     `floating point`_ number) -- a real format is one of :cpp:class:`BufBaseInt`'s or
     :cpp:class:`BufBaseFloat`'s concrete subclasses, or :cpp:class:`BufUnorm`

     .. warning::
        Unlike the pure-Python original (where any subclass can be defined in plain Python and used
        immediately -- ``BufDataType.decode``/``encode`` dispatch dynamically on whatever object is
        passed in), a brand-new elementary data type not already covered by one of this class's
        existing C++ subclasses needs a real C++ subclass and a rebuild -- this port does not expose
        a Python-overridable virtual dispatch point for :cpp:func:`decode`/:cpp:func:`encode`
     @endrst
     */
    class BufDataType: public BufType {
        public:

            /**
             * @brief Constructs a new elementary data type
             *
             * @param name The name of the type
             * @param size The byte size for the data type (at most 8 bytes -- see the class-level warning
             *      on this codebase's 64-bit integer cap)
             * @param isBigEndian Whether the type is in big endian mode
             *
             * @throws std::invalid_argument if 'size' is 0 or greater than 8
             */
            BufDataType(std::string name, std::size_t size, bool isBigEndian = false);

            virtual ~BufDataType() = default;

            /**
             * @brief The byte size for the data type
             */
            std::size_t getSize() const;

            /**
             * @brief Sets the byte size for the data type
             *
             * @param size The new byte size
             *
             * @throws std::invalid_argument if 'size' is 0 or greater than 8
             */
            void setSize(std::size_t size);

            /**
             * @brief
             @rst
             The `endianness`_ for the data type
             @endrst
             */
            bool getIsBigEndian() const;

            /**
             * @brief
             @rst
             Sets the `endianness`_ for the data type
             @endrst
             *
             * @param isBigEndian The new endianness (see #getIsBigEndian)
             */
            void setIsBigEndian(bool isBigEndian);

            /**
             * @brief
             @rst
             Decode the raw bytes to the required format for the type

             .. warning::
                Please make sure the number of bytes passed into 'src' matches :cpp:func:`getSize`
             @endrst
             *
             * @param src The raw bytes to decode
             *
             * @return The decoded value for the type
             */
            virtual BufValue decode(const ByteVec& src) const = 0;

            /**
             * @brief
             @rst
             Encodes the format of the type back to raw bytes

             .. warning::
                Please make sure 'src' is within the acceptable range for the type
             @endrst
             *
             * @param src The decoded value to encode
             *
             * @return The encoded raw bytes
             */
            virtual ByteVec encode(const BufValue& src) const = 0;

            /**
             * @brief
             @rst
             Makes an owned copy of this data type :raw-html:`<br />` :raw-html:`<br />`

             A ``BufDataType`` is a small, shareable *value* -- a type descriptor, not a unique
             identity-bearing node -- and this codebase's own real usage relies on that: the
             `pybind11`_ binding for :cpp:class:`BufElementType` clones each incoming data type
             through this method rather than taking ownership of the exact Python object passed
             in, specifically so the same cached ``BufDataType`` instance (eg.
             ``BufDataTypes.Float32.value`` in the Python-side ``constants/BufDataTypes.py``,
             reused across many different ``.buf`` element definitions) can be passed into more
             than one :cpp:class:`BufElementType` without being consumed by the first use
             @endrst
             */
            virtual std::unique_ptr<BufDataType> clone() const = 0;

        private:
            std::size_t size_;
            bool isBigEndian_;

            static void validateSize(std::size_t size);
    };
}

#endif
