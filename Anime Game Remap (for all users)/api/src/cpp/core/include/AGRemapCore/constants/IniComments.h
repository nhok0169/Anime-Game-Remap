#ifndef AGRemapCore_IniComments_H
#define AGRemapCore_IniComments_H

#include <string>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Comment blocks this software writes into the ``.ini`` files it generates :raw-html:`<br />`
     :raw-html:`<br />`

     A port of the pure-Python ``IniComments`` enum (``constants/IniConsts.py``)

     .. note::
        The pure-Python constant this comes from was declared and then never referenced by anything
        -- no fixer, no writer, nothing -- and that is still true here: nothing assigns it on its
        own. It is the text :cpp:member:`GIMIFixer::copyPreamble` is *meant* to be set to, kept as a
        named constant so a caller opting in does not have to carry the paragraph itself
     @endrst
     */
    class IniComments {
        public:

            /**
             * @brief
             @rst
             The explanation written at the top of every generated ``.ini`` **copy**
             :raw-html:`<br />` :raw-html:`<br />`

             A remap that merges several of a mod's objects onto one object of the remapped mod
             cannot put every section in one file -- a GIMI-like importer warns when two sections
             map to the same hash and shows only one of them. The workaround is to lean on the
             importer's own overlapping-mod behaviour and emit one extra ``.ini`` file per extra
             object. This comment is there so that whoever opens one of those files finds out why
             it exists rather than assuming the fix duplicated their mod by mistake
             @endrst
             */
            static inline const std::string GIMIObjMergerPreamble =
"; This is really bad!! Don't do this!\n"
"; ************************************\n"
";\n"
"; jk, but joking aside...\n"
";\n"
"; The goal is to display n mod objects from the mod to be remapped to the mod onto a single mod object of the remapped mod.\n"
";   Therefore we will have n sets of resources all mapping onto a single index (and same hash).\n"
";\n"
"; Ideally, we would want all the sections to be within a single .ini file. The naive approach would be to create n sets of sections\n"
";   (not a single section, cuz you need to include the case of sections depending on other sections, which form a section caller/callee graph) \n"
";    where the sections names are all unique. However, this approach will trigger a warning on GIMI (or any GIMI like importer) of multiple sections\n"
";   mapping to the same hash and only 1 of the mod objects will be displayed\n"
";\n"
"; The next attempt would be to take advantage of GIMI's overlapping mod bug/feature from loading multiple mods of the same character\n"
";   Apart from the original .ini file, there would be n-1 newly generated .ini files (total of n .ini files). Each .ini file would uniquely\n"
";   display a single set of sections from the n sets of sections. The overlapping property from the bug/feature would allow for all the objects to be displayed.\n"
";\n"
"; For now, we were lazy and just simply copied the original .ini file onto the generated .ini files, which results in the original mod to have overlapping copies.\n"
";  But since the mod used in all the .ini files are exactly the same, the user would not see the overlap (they may have some performance issues depending on the size of n. But\n"
";   usually remaps only merge 2 mod objects into a single mod object, which should not cause much of an issue)\n"
";   We could optimize the amount of space taken up by the newly generated .ini files, by only putting the necessary sections, but that is for another day...";
    };
}

#endif
