#ifndef AGRemapCore_IniFixingContext_H
#define AGRemapCore_IniFixingContext_H


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The per-call knobs a caller hands to :cpp:func:`BaseIniFixer::fix` :raw-html:`<br />`
     :raw-html:`<br />`

     **Not to be confused with** :cpp:class:`IniFixContext`, which is a different thing wearing a
     near-identical name. That one is the ``.ini`` **file** behind an interface -- long-lived, bound
     once, and how a fixer reads and rewrites its file. This one is a plain bag of options describing
     *this particular fix*, is passed by value on every call, and knows nothing about any file
     :raw-html:`<br />` :raw-html:`<br />`

     The fixing counterpart of :cpp:class:`IniRemovalContext`, and it is that pair of names
     (``IniRemoveContext`` / ``IniRemovalContext``) this one follows :raw-html:`<br />`
     :raw-html:`<br />`

     Deliberately a plain struct rather than an interface: nothing here needs to be answered by
     asking someone else :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Not a template, unlike almost everything else in this family, and it does not need to be --
        it holds no `KVP`_ keys, `sections`_ or values. That is what lets the `pybind11`_ layer bind
        it once and hand the same type to every fixer instantiation
     @endrst
     */
    struct IniFixingContext {
        /**
         * @brief
         @rst
         Whether this fixer is running for the **first** :cpp:class:`ModType` of the ``.ini`` file
         :raw-html:`<br />` :raw-html:`<br />`

         The mirror image of #isLastModType, and there for the same reason: several fixers chain over
         one ``.ini`` file, so anything that touches the file itself rather than only the fix has to
         happen exactly once. :cpp:func:`GIMIFixer::fixImpl` uses this for its ``keepBackup`` pass --
         disabling the existing ``.ini`` file as a backup is the whole file's business, and doing it
         again on a later mod type would be backing up a file the first pass already moved aside
         :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            **Default**: ``true``, for the same reason #isLastModType's is -- a fixer driven directly
            is the only one, so it is both the first and the last
         @endrst
         */
        bool isFirstModType = true;

        /**
         * @brief
         @rst
         Whether this fixer is running for the **last** :cpp:class:`ModType` of the ``.ini`` file
         :raw-html:`<br />` :raw-html:`<br />`

         One ``.ini`` file can be fixed by several fixers in turn -- one per mod type it was
         classified as, and one per target mod each of those fixes to (see
         :cpp:func:`IniFile::fix`). They chain over the same file, so anything that rewrites the
         file's own text rather than only adding to the fix has to happen exactly once, at the end.
         :cpp:func:`GIMIFixer::fixImpl` uses this for its ``hideOrig`` pass for that reason: hiding
         the original mod's `sections`_ is the whole file's business, not one mod type's
         :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            **Default**: ``true``, so a fixer driven directly -- one fixer, one call, no chain --
            behaves as though it were the only one, which it is. It is the *chaining* caller that
            has to say otherwise, and :cpp:func:`IniFile::fix` is the one that does
         @endrst
         */
        bool isLastModType = true;

        IniFixingContext() = default;

        /**
         * @brief Constructs a new set of fixing options
         *
         * @param isFirstModType See #isFirstModType
         * @param isLastModType See #isLastModType
         */
        // Neither parameter is defaulted, deliberately. Two adjacent bools with a default on the
        // second would let 'IniFixingContext(x)' compile and silently mean the *first* flag, which
        // is exactly the kind of mistake a caller cannot see. Spell both, or default-construct.
        IniFixingContext(bool isFirstModType, bool isLastModType):
            isFirstModType(isFirstModType), isLastModType(isLastModType) {}
    };
}

#endif
