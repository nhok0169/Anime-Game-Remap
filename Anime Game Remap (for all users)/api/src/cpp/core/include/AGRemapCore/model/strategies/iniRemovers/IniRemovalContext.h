#ifndef AGRemapCore_IniRemovalContext_H
#define AGRemapCore_IniRemovalContext_H


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The per-call knobs a caller hands to :cpp:func:`BaseIniRemover::remove` :raw-html:`<br />`
     :raw-html:`<br />`

     **Not to be confused with** :cpp:class:`IniRemoveContext`, which is a different thing wearing a
     near-identical name. That one is the ``.ini`` **file** behind an interface -- long-lived, bound
     once, and how a remover reads and rewrites its file. This one is a plain bag of options
     describing *this particular removal*, is passed by value on every call, and knows nothing about
     any file :raw-html:`<br />` :raw-html:`<br />`

     Deliberately a plain struct rather than an interface: nothing here needs to be answered by
     asking someone else :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        Not a template, unlike almost everything else in this family, and it does not need to be --
        it holds no `KVP`_ keys, `sections`_ or values. That is what lets the `pybind11`_ layer bind
        it once and hand the same type to every remover instantiation
     @endrst
     */
    struct IniRemovalContext {
        /**
         * @brief
         @rst
         Whether to remove the fix without asking which :cpp:class:`ModType` it belongs to
         :raw-html:`<br />` :raw-html:`<br />`

         :cpp:func:`RemapIniRemover::remove` normally makes a candidate `section`_ a *target* only when
         it carries this software's own marker -- either it sits inside the fix boilerplate, or some
         :cpp:class:`IfContentPartColouring` state gives its ``hash`` `KVP`_ a value belonging to one
         of the ``.ini`` file's mod types. That second half is what recognizes the ``Remap``-named
         leftovers *outside* the boilerplate :raw-html:`<br />` :raw-html:`<br />`

         With this set, the hash half is skipped entirely and **every** candidate is a target -- ie.
         every `section`_ inside the boilerplate plus every ``Remap``-named `section`_ outside it,
         whoever they belong to. That is what the pure-Python ``IniRemover.py`` this replaced always
         did, and what :cpp:func:`IniFile::removeFix` asks for on its **last** mod type
         :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            The reason that last-one-sweeps rule exists: the strict rule can only recognize a
            leftover whose ``hash`` it can attribute, and a ``Remap``-named `section`_ outside the
            boilerplate carrying no ``hash`` at all (or one no mod type of this ``.ini`` file owns)
            would otherwise be left behind forever -- the exact debris an interrupted or partly
            undone fix leaves. Running the strict rule for every mod type but the last lets each one
            take only what it can prove is its own, and then lets the final pass clear whatever is
            still standing :raw-html:`<br />` :raw-html:`<br />`

         **Default**: ``false``
         @endrst
         */
        bool ignoreModType = false;

        /**
         * @brief
         @rst
         Whether the backup this fix left beside the ``.ini`` file survives the removal
         :raw-html:`<br />` :raw-html:`<br />`

         Fixing a ``.ini`` file moves the original aside as ``RemapBKUP<name>.txt`` (see
         :cpp:func:`IniFile::disableIni`). Removing that fix puts the ``.ini`` file back, which
         leaves the backup with nothing left to protect -- so with this ``false``, the removal
         deletes it as well :raw-html:`<br />` :raw-html:`<br />`

         .. note::
            Defaults to ``true`` because that is the side to err on: a backup kept when it did not
            need to be is clutter, whereas one deleted when it was still wanted is the user's
            original ``.ini`` file gone for good
         @endrst
         */
        bool keepBackups = true;

        IniRemovalContext() = default;

        /**
         * @brief Constructs a new set of removal options
         *
         * @param ignoreModType See #ignoreModType. **Default**: ``false``
         * @param keepBackups See #keepBackups. **Default**: ``true``
         */
        explicit IniRemovalContext(bool ignoreModType, bool keepBackups = true):
            ignoreModType(ignoreModType), keepBackups(keepBackups) {}
    };
}

#endif
