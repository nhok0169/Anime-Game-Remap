#ifndef AGRemapCore_DownloadMode_H
#define AGRemapCore_DownloadMode_H


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
}

#endif
