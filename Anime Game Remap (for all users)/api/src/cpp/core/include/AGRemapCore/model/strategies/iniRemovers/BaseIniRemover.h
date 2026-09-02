#ifndef AGRemapCore_BaseIniRemover_H
#define AGRemapCore_BaseIniRemover_H

#include <functional>
#include <string>

#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"


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
        :cpp:func:`IniFile::removeFix` before it calls any remover, and by
        :cpp:func:`IniRemoveContext::readFileLines` for every other caller

     .. note::
        The pure-Python original exposes its ``.ini`` file as a plain public ``iniFile`` attribute.
        This follows :cpp:class:`BaseIniParser`'s convention instead (protected member plus
        #getIniFile/#setIniFile). :cpp:func:`IniRemoveBuilder::build` is what calls #setIniFile,
        so a remover reached that way arrives already bound

     .. note::
        This is a class template over the same ``K``/``V``/``KeyHash``/``KeyEqual`` as the
        :cpp:class:`IfContentPart`\\s the `sections`_ it removes are made of, defaulting to
        ``<std::string, std::string>``. It has to be, for exactly the reason
        :cpp:class:`BaseIniParser` gives: the `pybind11`_ layer's `sections`_ are
        ``IfTemplate<py::object, py::object, ...>``, so a remover pinned to
        ``<std::string, std::string>`` would be unreachable from any binding -- and
        :cpp:class:`RemapIniRemover`, the one concrete remover this base has, is reached from exactly
        there

     .. note::
        :cpp:class:`IniRemoveBuilder` (and, through it, :cpp:member:`ModType::iniRemoveBuilder` and
        :cpp:func:`IniFile::removeFix`) deliberately stays pinned to ``BaseIniRemover<>`` rather
        than becoming a template of its own -- the same call the parser family made, and for the
        same reason: nothing on the `Python`_ side builds removers through it (the `Python`_ API has
        its own pure-Python ``IniRemoveBuilder``), so the only instantiation it ever needs is the
        plain-``std::string`` one
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
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
                file, freshly built for it

             .. note::
                :cpp:class:`RemapIniRemover` -- the one concrete remover in this hierarchy -- does **not**
                read the ``.ini`` file through this pointer at all; it goes through an
                :cpp:class:`IniRemoveContext` instead, because the ``.ini`` file its real callers
                hand it is the *`Python`_* ``IniFile``, an unrelated class to
                :cpp:class:`AGRemapCore::IniFile`. See that class's own note

             :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             */
            explicit BaseIniRemover(IniFile* iniFile = nullptr): iniFile_(iniFile) {}

            virtual ~BaseIniRemover() = default;

            /**
             * @brief The .ini file this remover removes fixes from, or ``nullptr`` if unbound
             */
            IniFile* getIniFile() const {
                return iniFile_;
            }

            /**
             * @brief
             @rst
             Binds this remover to the ``.ini`` file it should remove fixes from -- non-owning, see
             the constructor :raw-html:`<br />` :raw-html:`<br />`

             ``virtual`` so a subclass reading the file through a context can rebuild that context
             when the binding changes -- which is exactly what :cpp:func:`RemapIniRemover::setIniFile`
             does, and what lets :cpp:func:`IniRemoveBuilder::build` bind a remover a caller-supplied
             factory handed back unbound
             @endrst
             *
             * @param iniFile The .ini file to remove the fix from, or ``nullptr`` to unbind
             */
            virtual void setIniFile(IniFile* iniFile) {
                iniFile_ = iniFile;
            }

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
             * @param context
             @rst
             The per-call options for this removal -- see :cpp:class:`IniRemovalContext`, and note
             that it is **not** the :cpp:class:`IniRemoveContext` a remover reads its file through
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: a default-constructed one, ie. every option off
             @endrst
             *
             * @return The new content of the .ini file
             */
            virtual std::string remove(bool parse = false, bool writeBack = true, IniRemovalContext context = IniRemovalContext()) {
                (void)parse;
                (void)writeBack;
                (void)context;
                return "";
            }

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
