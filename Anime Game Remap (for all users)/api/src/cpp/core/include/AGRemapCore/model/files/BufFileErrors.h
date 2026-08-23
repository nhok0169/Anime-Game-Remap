#ifndef AGRemapCore_BufFileErrors_H
#define AGRemapCore_BufFileErrors_H

#include <stdexcept>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Thrown when certain bytes do not correspond to the format defined for a ``.buf`` file
     :raw-html:`<br />` :raw-html:`<br />`

     A minimal, Python-free data carrier for the ``fileType`` that caused the error -- the
     `pybind11`_ binding that catches this constructs the real
     ``FixRaidenBoss2.exceptions.BadBufData.BadBufData`` Python object instead, so that exception's
     own message formatting stays living in exactly one place (see
     :cpp:class:`AGRemapCore::SyntaxErr`'s own doc comment for the same pattern applied to a
     different exception)
     @endrst
     */
    class BadBufData: public std::runtime_error {
        public:

            /**
             * @brief Constructs a new error
             *
             * @param fileType The name for the type of ``.buf`` file
             */
            explicit BadBufData(std::string fileType = "Buffer");

            /**
             * @brief The name for the type of ``.buf`` file
             */
            const std::string& fileType() const;

        private:
            std::string fileType_;

            static std::string buildMessage(const std::string& fileType);
    };

    /**
     * @brief
     @rst
     Thrown when a ``.buf`` file cannot be read :raw-html:`<br />` :raw-html:`<br />`

     A minimal, Python-free data carrier for the ``filePath``/``fileType`` that caused the error --
     see :cpp:class:`BadBufData`'s doc comment for why the message formatting itself is not
     reimplemented here
     @endrst
     */
    class BufFileNotRecognized: public std::runtime_error {
        public:

            /**
             * @brief Constructs a new error
             *
             * @param filePath The file path to the ``.buf`` file
             * @param fileType The name for the type of ``.buf`` file
             */
            explicit BufFileNotRecognized(std::string filePath, std::string fileType = "Buffer");

            /**
             * @brief The file path to the ``.buf`` file
             */
            const std::string& filePath() const;

            /**
             * @brief The name for the type of ``.buf`` file
             */
            const std::string& fileType() const;

        private:
            std::string filePath_;
            std::string fileType_;

            static std::string buildMessage(const std::string& filePath, const std::string& fileType);
    };
}

#endif
