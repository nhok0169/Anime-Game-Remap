#ifndef AGRemapCore_RegFillMissingMode_H
#define AGRemapCore_RegFillMissingMode_H


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Different modes for handling :cpp:class:`IfContentPart`\s with missing registers
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The `Python`_-facing ``RegFillMissingMode`` is a separate, still-pure-Python ``Enum``
        (``constants/RegFillMissingMode.py``) whose members carry the same two string values. The
        binding layer maps one onto the other by that value, rather than either side being replaced
        -- this enum exists so `AGRemapCore` stays usable with no `Python`_ at all, matching what
        ``IniGraphReplaceMode`` already does for the same reason
     @endrst
     */
    enum class RegFillMissingMode {
        /**
         * @brief
         @rst
         Finds all :cpp:class:`IfContentPart`\s missing the desired register and fills those parts
         with the register
         @endrst
         */
        FillMissing,

        /**
         * @brief
         @rst
         Determines whether the caller/callee graph (:cpp:class:`IniSectionGraph`) contains an
         :cpp:class:`IfContentPart` missing the desired register, then adds the register to the
         roots of the graph to cover for the missing registers
         @endrst
         */
        TopdownCover
    };
}

#endif
