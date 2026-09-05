#ifndef AGRemapCore_RemapServiceCLIErrors_H
#define AGRemapCore_RemapServiceCLIErrors_H

#include <stdexcept>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Thrown when a string naming a type of mod matches no mod type :raw-html:`<br />`
     :raw-html:`<br />`

     Only :cpp:class:`RemapServiceCLI` can raise this. :cpp:class:`RemapService` takes
     :cpp:enum:`ModTypeId` integers, which cannot be wrong in this way -- it is turning what a user
     *typed* into one of those that can fail, and that is the CLI's job :raw-html:`<br />`
     :raw-html:`<br />`

     A minimal, `Python`_-free data carrier for the string that could not be resolved. The
     `pybind11`_ layer that catches this constructs the real
     ``FixRaidenBoss2.exceptions.InvalidModType.InvalidModType`` instead, so that exception's own
     message lives in one place -- the same pattern :cpp:class:`BadBufData` uses
     @endrst
     */
    class InvalidModType: public std::runtime_error {
        public:

            /**
             * @brief Constructs a new error
             *
             * @param modType The string that matched no mod type
             */
            explicit InvalidModType(std::string modType);

            /**
             * @brief The string that matched no mod type
             */
            const std::string& modType() const;

        private:
            std::string modType_;

            static std::string buildMessage(const std::string& modType);
    };


    /**
     * @brief
     @rst
     Thrown when a string naming a download mode matches no :cpp:enum:`DownloadMode`
     :raw-html:`<br />` :raw-html:`<br />`

     See :cpp:class:`InvalidModType` for why this can only come from :cpp:class:`RemapServiceCLI`,
     and for why the message is rebuilt on the `Python`_ side rather than crossing as text
     @endrst
     */
    class InvalidDownloadMode: public std::runtime_error {
        public:

            /**
             * @brief Constructs a new error
             *
             * @param downloadMode The string that matched no download mode
             */
            explicit InvalidDownloadMode(std::string downloadMode);

            /**
             * @brief The string that matched no download mode
             */
            const std::string& downloadMode() const;

        private:
            std::string downloadMode_;

            static std::string buildMessage(const std::string& downloadMode);
    };


    /**
     * @brief
     @rst
     Thrown when a string meant to be a game version is not one :raw-html:`<br />`
     :raw-html:`<br />`

     The pure-Python original raised a bare ``ValueError`` here rather than one of its own
     ``Error`` subclasses, so this does **not** carry the ``ERROR:`` prefix the other two do -- see
     :cpp:func:`InvalidModType::buildMessage`
     @endrst
     */
    class InvalidVersion: public std::runtime_error {
        public:

            /**
             * @brief Constructs a new error
             *
             * @param version The string that could not be parsed as a version
             */
            explicit InvalidVersion(std::string version);

            /**
             * @brief The string that could not be parsed as a version
             */
            const std::string& version() const;

        private:
            std::string version_;
    };
}

#endif
