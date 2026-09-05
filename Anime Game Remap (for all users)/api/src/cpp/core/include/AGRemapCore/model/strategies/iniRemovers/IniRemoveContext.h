#ifndef AGRemapCore_IniRemoveContext_H
#define AGRemapCore_IniRemoveContext_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "AGRemapCore/model/Version.h"
#include "AGRemapCore/model/assets/ModMappedAssets.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     The ``.ini`` file a remover is removing a fix from, behind an interface :raw-html:`<br />`
     :raw-html:`<br />`

     The removing counterpart of :cpp:class:`IniParseContext`/:cpp:class:`IniFixContext`, and it
     exists for exactly the same reason -- see :cpp:class:`IniParseContext`'s own note. The ``.ini``
     file every real caller of :cpp:class:`RemapIniRemover` passes is the *`Python`_* ``IniFile``
     (``model/files/IniFile.py``), an unrelated class to :cpp:class:`AGRemapCore::IniFile` with
     nothing castable between them, so a plain ``IniFile*`` parameter would always be ``nullptr``
     there and the remover would be inert.

     :raw-html:`<br />`

     .. note::
        This one is deliberately **narrower** than the parse and fix contexts. A remover neither
        builds `sections`_ nor writes new files: it reads the raw lines, reads the parsed
        `sections`_, asks which hashes the ``.ini`` file's mod type(s) own, and hands one rewritten
        text back. Everything else a removal eventually needs -- the resources it turned up, what to
        do with them -- comes back as :cpp:func:`RemapIniRemover::getRemovedResources` for the caller to
        act on, rather than being pushed through here

     .. note::
        There is no ``classify()`` on this interface, on purpose. Classification is the caller's
        job and its cost is the caller's to decide: :cpp:class:`IniFileRemoveContext` does it lazily
        on first use, while a `Python`_ implementation simply reads the ``availableType`` its own
        caller already resolved. #modTypeHashes returning nothing is a perfectly ordinary answer,
        and only affects which `sections`_ *outside* the fix boilerplate are recognized
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class IniRemoveContext {
        public:

            /**
             * @brief The type of `section`_ a ``.ini`` file is made of
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The kind of asset table :cpp:member:`ModType::hashes` is, as reached from here -- the
             same shape :cpp:type:`IniParseContext::Assets` uses, for the same reason
             @endrst
             */
            using Assets = ModMappedAssets<K, V, KeyHash, KeyEqual, KeyHash, KeyEqual>;

            virtual ~IniRemoveContext() = default;

            /**
             * @brief
             @rst
             Whether there is a real ``.ini`` file behind this context :raw-html:`<br />`
             :raw-html:`<br />`

             ``false`` stands in for the pure-Python original's ``ini = None``, and makes
             :cpp:func:`RemapIniRemover::remove` a no-op
             @endrst
             */
            virtual bool hasIni() const = 0;

            /**
             * @brief
             @rst
             The folder the ``.ini`` file lives in (the equivalent of the pure-Python original's
             ``ini.folder``), or an empty string when it has none :raw-html:`<br />`
             :raw-html:`<br />`

             Every path :cpp:func:`RemapIniRemover::getRemovedResources` hands back is resolved against
             this -- see that method's own note on what an empty answer means
             @endrst
             */
            virtual std::string iniFolder() const = 0;

            /**
             * @brief
             @rst
             The game version the ``.ini`` file was written for (``ini.version``), or
             ``std::nullopt`` for "the latest" -- what #modTypeHashes is searched at
             @endrst
             */
            virtual std::optional<Version> version() const = 0;

            /**
             * @brief
             @rst
             The ``hash`` assets of every :cpp:class:`ModType` the ``.ini`` file was classified as.
             Borrowed, not owned; may be empty :raw-html:`<br />` :raw-html:`<br />`

             A **vector**, unlike :cpp:func:`IniParseContext::modTypeHashes`, because an
             :cpp:class:`AGRemapCore::IniFile` can carry several mod types at once and
             :cpp:func:`IniFile::removeFix` hands the same file to each one's remover in turn
             without telling any of them which one it is acting for. The pure-Python ``IniFile``,
             whose ``availableType`` is singular, simply returns a one-element (or empty) vector
             @endrst
             */
            virtual std::vector<Assets*> modTypeHashes() const = 0;

            /**
             * @brief
             @rst
             The text lines of the ``.ini`` file, reading it from disk first if that has not
             happened yet -- the equivalent of the pure-Python original's ``_readLines`` decorator
             :raw-html:`<br />` :raw-html:`<br />`

             Each line keeps its own trailing newline, except possibly the last
             @endrst
             */
            virtual std::vector<std::string> readFileLines() = 0;

            /**
             * @brief
             @rst
             Every `section`_ parsed out of the ``.ini`` file, keyed by name -- the equivalent of the
             pure-Python original's ``ini.sectionIfTemplates``. Borrowed, not owned
             @endrst
             */
            virtual std::unordered_map<std::string, Section*> sectionIfTemplates() const = 0;

            /**
             * @brief The text content of the ``.ini`` file (``ini.fileTxt``)
             */
            virtual std::string fileTxt() const = 0;

            /**
             * @copydoc fileTxt() const
             *
             * @param txt The new text content
             */
            virtual void setFileTxt(std::string txt) = 0;

            /**
             * @brief
             @rst
             Writes the ``.ini`` file's current text content out (``ini.write()``) and returns what
             was written
             @endrst
             */
            virtual std::string write() = 0;

            /**
             * @brief
             @rst
             Drops the read text/lines so the next read comes from disk again
             (``ini.clearRead()``)
             @endrst
             */
            virtual void clearRead() = 0;

            /**
             * @brief
             @rst
             Records that the ``.ini`` file no longer holds a fix (``ini._isFixed = False``)
             :raw-html:`<br />` :raw-html:`<br />`

             The removing counterpart of :cpp:func:`IniFixContext::setIsFixed`, and the same note
             applies: :cpp:class:`AGRemapCore::IniFile` owns that flag through
             :cpp:func:`IniFile::classify` and does not let a remover write it, so a plain C++
             implementation of this is free to do nothing. The `Python`_ ``IniFile`` uses the flag
             the pure-Python original's way, and its implementation does clear it :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                The pure-Python original also sets ``ini._hideOriginalReplaced = True`` here. That
                is deliberately **not** on this interface: the only thing that flag does is let
                ``IniFile.getSectionOptions`` skip stripping the hide-original comment from lines
                that might still carry it -- and by the time this is called
                :cpp:func:`RemapIniRemover::remove` has already stripped every one of them out of the
                text, so the strip it would skip is a no-op either way
             @endrst
             *
             * @param isFixed Whether the .ini file still holds a fix
             */
            virtual void setIsFixed(bool isFixed) = 0;

            /**
             * @brief
             @rst
             Deletes the backup this fix left beside the ``.ini`` file, if one is there
             :raw-html:`<br />` :raw-html:`<br />`

             The counterpart to :cpp:func:`IniFile::disableIni`, which is what created it. A
             backup that is not there is not an error -- a ``.ini`` file fixed without
             ``keepBackup``, or one already cleaned up, simply has nothing to delete

             .. note::
                Only called when :cpp:member:`IniRemovalContext::keepBackups` is ``false``, so an
                implementation does not decide *whether* to delete -- only how
             @endrst
             */
            virtual void removeBackup() = 0;
    };
}

#endif
