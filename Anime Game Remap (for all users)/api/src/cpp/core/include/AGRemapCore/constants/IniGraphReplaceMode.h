#ifndef AGRemapCore_IniGraphReplaceMode_H
#define AGRemapCore_IniGraphReplaceMode_H


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Different replacement modes for when the :cpp:class:`IniSectionGraph` already exists when
     :cpp:class:`BaseResEdit` builds the corresponding graph :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The `Python`_-facing ``IniGraphReplaceMode`` is a separate, still-pure-Python ``Enum``
        (``constants/IniGraphReplaceMode.py``) whose members carry the same three string values.
        The binding layer maps one onto the other by that value, rather than either side being
        replaced -- this enum exists so `AGRemapCore` stays usable with no `Python`_ at all
     @endrst
     */
    enum class IniGraphReplaceMode {
        /**
         * @brief Keeps the already existing graph
         */
        Ignore,

        /**
         * @brief Replaces the already existing graph with a brand-new one
         */
        Replace,

        /**
         * @brief Combines the already existing graph with the newly created graph
         */
        Combine
    };
}

#endif
