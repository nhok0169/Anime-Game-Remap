#ifndef AGRemapCore_BaseIniRemover_H
#define AGRemapCore_BaseIniRemover_H

#include <string>


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     Base class to remove fixes from a ``.ini`` file :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        The pure-Python original also carries a ``_readLines`` **decorator**, used to make sure the
        ``.ini`` file has been read before a method runs. That is not ported: it is a `Python`_
        decorator mechanism with no C++ equivalent, and the one thing it actually does --
        "read the file first if it hasn't been read yet" -- is done by
        :cpp:func:`IniFile::removeFix` before it calls any remover

     .. note::
        The pure-Python original exposes its ``.ini`` file as a plain public ``iniFile`` attribute.
        This follows :cpp:class:`BaseIniParser`'s convention instead (protected member plus
        #getIniFile/#setIniFile). :cpp:func:`IniRemoveBuilder::build` is what calls #setIniFile, on
        every call -- which is how one cached remover serves many ``.ini`` files
     @endrst
     */
    class BaseIniRemover {
        public:

            /**
             * @brief Constructs a new remover
             *
             * @param iniFile
             @rst
             The ``.ini`` file to remove the fix from :raw-html:`<br />` :raw-html:`<br />`

             This is a non-owning pointer to a file owned elsewhere -- it must outlive this remover
             :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` is allowed so a remover can exist before it is bound to any particular file;
             call #setIniFile before #remove in that case :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                A remover reached through a :cpp:class:`ModType` is never in that state --
                :cpp:member:`ModType::iniRemoveBuilder` hands one back already bound to the caller's
                file. Unlike the parser and fixer builders, though, that one is a *flyweight*: the
                instance may be shared with other files and stays correctly bound only until that
                builder's next :cpp:func:`IniRemoveBuilder::build`
             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit BaseIniRemover(IniFile* iniFile = nullptr);

            virtual ~BaseIniRemover() = default;

            /**
             * @brief The .ini file this remover removes fixes from, or ``nullptr`` if unbound
             */
            IniFile* getIniFile() const;

            /**
             * @brief
             @rst
             Binds this remover to the ``.ini`` file it should remove fixes from -- non-owning, see
             the constructor
             @endrst
             *
             * @param iniFile The .ini file to remove the fix from, or ``nullptr`` to unbind
             */
            void setIniFile(IniFile* iniFile);

            /**
             * @brief
             @rst
             Removes the fix from the ``.ini`` file. Returns an empty string by default, matching
             the pure-Python original's ``pass``
             @endrst
             *
             * @param parse
             @rst
             Whether to also parse for the ``*.RemapBlend.buf`` files that need to be removed
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             * @param writeBack Whether to write back the new text content of the .ini file. **Default**: ``true``
             *
             * @return The new content of the .ini file
             */
            virtual std::string remove(bool parse = false, bool writeBack = true);

        protected:

            /**
             * @brief
             @rst
             The ``.ini`` file the fix will be removed from -- non-owning, see the constructor
             @endrst
             */
            IniFile* iniFile_;
    };
}

#endif
