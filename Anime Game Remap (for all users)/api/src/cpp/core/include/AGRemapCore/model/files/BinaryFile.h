#ifndef AGRemapCore_BinaryFile_H
#define AGRemapCore_BinaryFile_H

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

#include <string>
#include <variant>

#include "AGRemapCore/model/buffers/BufValue.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Either a file path (``std::string``) or a raw, already-in-memory sequence of bytes -- the C++
     analogue of the pure-Python original's ``Union[str, bytes]`` :raw-html:`<br />` :raw-html:`<br />`
     @endrst
     */
    using BinarySrc = std::variant<std::string, ByteVec>;

    /**
     * @brief
     @rst
     A class to handle binary files :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original splits this into two classes -- an abstract ``File`` base (just a
        ``read()`` marker) and ``BinaryFile`` itself. Nothing else in this codebase's C++ port
        needs a file model that *isn't* binary, so this port folds the two together rather than
        carrying an unused abstract layer
     @endrst
     */
    class BinaryFile {
        public:

            /**
             * @brief Constructs a new binary file
             *
             * @param src The source file or bytes for the file
             */
            explicit BinaryFile(BinarySrc src);

            virtual ~BinaryFile() = default;

            /**
             * @brief The source file or bytes for the file
             */
            const BinarySrc& getSrc() const;

            /**
             * @brief Sets the source file or bytes for the file
             *
             * @param src The new source
             */
            void setSrc(BinarySrc src);

            /**
             * @brief The bytes read in from the source
             */
            const ByteVec& getData() const;

            /**
             * @brief
             @rst
             Reads the bytes from #getSrc -- if it holds a file path, the file is read from disk;
             if it already holds raw bytes, those bytes are used directly
             @endrst
             *
             * @return The read bytes
             *
             * @throws std::runtime_error if #getSrc holds a file path that cannot be opened
             */
            virtual ByteVec read();

        protected:
            BinarySrc src_;
            ByteVec data_;
    };
}

#endif
