#ifndef AGRemapCore_DownloadMode_H
#define AGRemapCore_DownloadMode_H

#include <optional>
#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The download mode of how the software handles file downloads :raw-html:`<br />`
     :raw-html:`<br />`

     .. note::
        The `Python`_-facing ``DownloadMode`` is a separate, still-pure-Python ``Enum``
        (``constants/DownloadMode.py``) whose members carry the same three string values. The
        binding layer maps one onto the other by that value, rather than either side being replaced
        -- this enum exists so `AGRemapCore` stays usable with no `Python`_ at all, matching what
        ``IniGraphReplaceMode`` already does for the same reason
     @endrst
     */
    enum class DownloadMode {
        /**
         * @brief Will not perform any file downloads for any mods
         */
        Disabled,

        /**
         * @brief Only perform file downloads at places in a .ini file where a resource is missing
         */
        Normal,

        /**
         * @brief
         @rst
         Will always perform file downloads for every mod, if possible, using pessimistic
         assumptions
         @endrst
         */
        Always
    };


    /**
     * @brief
     @rst
     Turns a :cpp:enum:`DownloadMode` into the string a user types for it, and back
     :raw-html:`<br />` :raw-html:`<br />`

     The strings are the `Python`_ ``DownloadMode`` enum's own values, which are what the command
     line actually accepts. Only :cpp:class:`RemapServiceCLI` needs them -- the model takes the enum
     @endrst
     */
    class DownloadModeTools {
        public:

            /**
             * @brief The string a user types for a download mode
             *
             * @param value The download mode to name
             *
             * @return Its name
             */
            static std::string getName(DownloadMode value);

            /**
             * @brief
             @rst
             Finds the :cpp:enum:`DownloadMode` a string names, ignoring case and surrounding
             whitespace :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                An **exact** match on the trimmed, lowercased text, where the pure-Python
                ``DownloadMode.search`` did a maximal-substring `Aho-Corasick`_ match. With three
                short values that search accepted things like ``"normally"`` and
                ``"not disabled"`` -- the latter meaning the opposite of what it was matched to.
                Nothing needs the leniency, and a typo reported as an error beats one silently
                resolved to the wrong mode
             @endrst
             *
             * @param name The text to resolve
             *
             * @return The download mode it names, if it names one
             */
            static std::optional<DownloadMode> findByName(const std::string& name);
    };
}

#endif
