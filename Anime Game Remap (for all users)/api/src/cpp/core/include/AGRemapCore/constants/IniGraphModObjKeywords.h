#ifndef AGRemapCore_IniGraphModObjKeywords_H
#define AGRemapCore_IniGraphModObjKeywords_H

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


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Keywords used as the *component* half of a :cpp:type:`IniGraphGroup::ModObj` for graphs that
     don't belong to a real mod object :raw-html:`<br />` :raw-html:`<br />`

     A :cpp:class:`IniGraphGroup` is keyed by ``(component, mod object)``, which works for the
     graphs that really are one mod object's ``TextureOverride`` command chain. The group a parser
     hands back also carries graphs that aren't -- the synthesized download-resource `sections`_ --
     and those get a reserved component name instead, so they can never collide with a real
     ``(component, object)`` pair from the ``.ini`` file.

     :raw-html:`<br />`

     .. note::
        The `Python`_-facing ``IniGraphModObjKeywords`` is a separate, still-pure-Python ``Enum``
        (``constants/IniConsts.py``) whose member carries the same string value. The binding layer
        uses the same literal rather than either side being replaced -- this class exists so
        `AGRemapCore` stays usable with no `Python`_ at all, matching what
        :cpp:enum:`IniGraphReplaceMode` already does for the same reason
     @endrst
     */
    class IniGraphModObjKeywords {
        public:

            /**
             * @brief Marks a graph whose `sections`_ are download resources rather than a mod object's commands
             */
            static inline const std::string Download = "download";
    };
}

#endif
