#ifndef AGRemapCore_PositionFile_H
#define AGRemapCore_PositionFile_H

#include <memory>
#include <vector>

#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BinaryFile.h"
#include "AGRemapCore/model/files/BufFile.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BufFile` :raw-html:`<br />` :raw-html:`<br />`

     Used for handling ``Position.buf`` files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        We observe that a ``Position.buf`` file is a binary file defined as:

        * a line corresponds to the data for a particular vertex in the mod
        * each line contains 40 bytes (320 bits)
        * each line uses little-endian mode (MSB is to the right while LSB is to the left)
        * the first 12 bytes of a line are the coordinate position of a vertex in an R3 vector space, each scalar value in the coordinate is 4 bytes or 32 bits (3 scalar values/line)
        * the next 12 bytes of a line corresponds to the normal vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (3 scalar values/line)
        * the last 16 bytes of a line corresponds to the tangent vector of a vertex, each scalar value in the vector is 4 bytes or 32 bits (4 scalar values/line)
        * all scalar values in the file are `floating point`_ values
     @endrst
     */
    class PositionFile: public BufFile {
        public:

            /**
             * @brief Constructs a new position file and immediately reads it
             *
             * @param src The source file or bytes for the ``.buf`` file
             */
            explicit PositionFile(BinarySrc src);

        private:
            static std::vector<std::unique_ptr<BufElementType>> defaultElements();
    };
}

#endif
