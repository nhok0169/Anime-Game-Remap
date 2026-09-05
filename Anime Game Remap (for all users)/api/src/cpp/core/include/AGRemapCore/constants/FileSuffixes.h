#ifndef AGRemapCore_FileSuffixes_H
#define AGRemapCore_FileSuffixes_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Suffixes this software puts on the files it creates :raw-html:`<br />` :raw-html:`<br />`

     A complete port of the pure-Python ``FileSuffixes`` enum (``constants/FileSuffixes.py``)

     .. note::
        Despite the class name, these are markers looked for **anywhere** in a file's base name,
        not only at its end -- the pure-Python original searches for them with the same
        substring-matching machinery it uses for :cpp:class:`FilePrefixes`, and a remap copy is
        named ``<something>RemapFix.ini``, so the marker never actually sits at the very end
     @endrst
     */
    class FileSuffixes {
        public:

            /**
             * @brief
             @rst
             What marks a ``.ini`` file as a *copy* this software generated, rather than one a mod
             author wrote :raw-html:`<br />` :raw-html:`<br />`

             :cpp:func:`RemapService::fix` filters these out of its walk -- handing one back to the
             fix would be feeding it its own output
             @endrst
             */
            static inline const std::string RemapFixCopy = "RemapFix";
    };
}

#endif
