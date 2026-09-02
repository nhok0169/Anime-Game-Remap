#ifndef AGRemapCore_RemapIniFixContext_H
#define AGRemapCore_RemapIniFixContext_H

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/strategies/iniFixers/IniFixContext.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     An :cpp:class:`IniFixContext` that already knows how to wrap a fix in this software's own
     header/credit/footer boilerplate, and how to comment the original mod out of the way
     :raw-html:`<br />` :raw-html:`<br />`

     Two of that interface's methods have an answer that doesn't depend on where the ``.ini`` file
     came from. :cpp:func:`addFixBoilerPlate` needs no ``.ini`` file at all -- it is the same text
     for every remap, built from a heading and a credit line. :cpp:func:`hideOriginalSections`
     needs one, but reaches it only through
     :cpp:func:`IniFixContext::fileTxt`/:cpp:func:`IniFixContext::setFileTxt`, which the interface
     already has. Everything else on it (the file's path, its log, where its groups live) genuinely
     does depend on the concrete file, so this class implements only those two and stays abstract:
     it is a base to inherit from, not a context to use on its own

     :raw-html:`<br />`

     The text it builds is the pure-Python ``IniFile.addFixBoilerPlate``'s, exactly:
     :cpp:func:`getFixHeader`, then :cpp:func:`getFixCredit`, then a blank line and the fix, then
     :cpp:func:`getFixFooter` -- so a fix written by a plain C++ caller is byte-for-byte one the
     `Python`_ side would have written, and the still-pure-Python ``RemapIniRemover`` can find it again

     :raw-html:`<br />`

     .. note::
        Both halves of the divider can be replaced wholesale through #header/#footer. What can't
        is the credit between them -- that is the attribution this software asks for, and a
        subclass that wants it gone can override #addFixBoilerPlate itself rather than being
        handed a switch for it

     .. note::
        One deliberate divergence from the pure-Python original: it caches the heading's title on
        the ``IniFile`` the first time a header is built and clears it again on every
        reclassification, so a header built before a ``.ini`` file was classified keeps the
        pre-classification title afterwards. Here the title is derived from #modTypeName at every
        call, so it is always current. Within any single #addFixBoilerPlate call the two are
        identical -- the header and the footer are always built from the same title either way
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class RemapIniFixContext: public IniFixContext<K, V, KeyHash, KeyEqual> {
        public:

            /**
             * @brief Constructs a new context
             *
             * @param header
             @rst
             The whole header line a fix opens with, or ``std::nullopt`` to build the default one
             -- see #getFixHeader :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param footer
             @rst
             The whole footer a fix closes with, or ``std::nullopt`` to build the default one --
             see #getFixFooter :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit RemapIniFixContext(std::optional<std::string> header = std::nullopt,
                                         std::optional<std::string> footer = std::nullopt);

            /**
             * @brief The header this was constructed with, or ``std::nullopt`` for the default one
             */
            std::optional<std::string> header;

            /**
             * @brief The footer this was constructed with, or ``std::nullopt`` for the default one
             */
            std::optional<std::string> footer;

            /**
             * @brief
             @rst
             The name of the mod type the ``.ini`` file was classified as, or ``std::nullopt`` when
             it was never classified (``ini.getFixModTypeName()``) :raw-html:`<br />` :raw-html:`<br />`

             The one thing the boilerplate needs that this class can't know for itself. The default
             is "never classified", which is what an unclassified ``.ini`` file's boilerplate reads
             as -- a subclass with a mod type overrides it

             :raw-html:`<br />`

             .. note::
                Whatever a subclass returns is used as-is apart from having its newlines and tabs
                stripped, the way the pure-Python original strips them: this text is written into a
                ``;``-comment, and one stray newline in it would silently turn the rest of the
                credit into ``.ini`` directives
             @endrst
             */
            virtual std::optional<std::string> modTypeName() const;

            /**
             * @brief
             @rst
             The mod type name the *heading* uses -- #modTypeName, or ``"GI"`` when there is none
             (``ini.getFixModTypeHeadingname()``)
             @endrst
             */
            std::string modTypeHeadingName() const;

            /**
             * @brief The title of the heading a fix is wrapped in (``ini.getHeadingName()``)
             */
            std::string headingName() const;

            /**
             * @brief
             @rst
             The header line a fix opens with -- #header when one was given, otherwise a
             :cpp:class:`Heading` opened around #headingName (``ini.getFixHeader()``)
             @endrst
             */
            std::string getFixHeader() const;

            /**
             * @brief
             @rst
             The footer a fix closes with -- #footer when one was given, otherwise a
             :cpp:class:`Heading` closed to the same width as #getFixHeader
             (``ini.getFixFooter()``)
             @endrst
             */
            std::string getFixFooter() const;

            /**
             * @brief
             @rst
             The credit line written between the header and the fix (``ini.getFixCredit()``)
             @endrst
             */
            std::string getFixCredit() const;

            /**
             * @brief
             @rst
             What every hidden line is prefixed with :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:member:`IniKeywords::HideOriginalComment` (``";RemapFixHideOrig -->"``)

             :raw-html:`<br />`

             .. danger::
                This has to stay in step with :cpp:member:`RemapIniRemover::hideOriginalComment`.
                A remover strips exactly what a fixer wrote when a fix is undone, so two different
                markers leave the ``.ini`` file permanently commented out
             @endrst
             */
            std::string hideOriginalComment = IniKeywords::HideOriginalComment;

            std::string addFixBoilerPlate(const std::string& fix) const override;

            /**
             * @brief
             @rst
             Comments out every line of the named `sections`_, in place on
             :cpp:func:`IniFixContext::fileTxt` :raw-html:`<br />` :raw-html:`<br />`

             A `section`_ owns every line from its own ``[Header]`` down to the line before the
             next one, trailing blank lines included -- the same line ranges the pure-Python
             ``IniFile.commentSectionOptions`` walks

             :raw-html:`<br />`

             .. note::
                Header lines are recognized, and their names read, with
                :cpp:func:`IniFile::isSectionHeaderLine`/:cpp:func:`IniFile::getSectionNameFromLine`
                rather than by a second implementation of those two rules. The coupling is the
                point: the names being matched here came out of `sections`_
                :cpp:func:`IniFile::getIfTemplates` cut on exactly those boundaries, so re-deriving
                them differently is how the two would silently stop agreeing
             @endrst
             */
            void hideOriginalSections(const std::unordered_set<std::string>& sectionNames) override;

        private:
            // modTypeName with the newlines and tabs taken out -- see modTypeName's own note.
            // Static, taking the context it reads, so the virtual call site is spelled out rather
            // than hidden behind an implicit `this`.
            static std::optional<std::string> cleanModTypeName(const RemapIniFixContext<K, V, KeyHash, KeyEqual>& ctx);

            // Every occurrence of 'target' in 'txt' replaced with 'replacement' -- Python's
            // str.replace, which the credit is written in terms of. StringTools has no equivalent
            // yet, and one placeholder substitution is not enough reason to add one there.
            static std::string replaceAll(const std::string& txt, const std::string& target, const std::string& replacement);
    };
}

#include "RemapIniFixContext.tpp"

#endif
