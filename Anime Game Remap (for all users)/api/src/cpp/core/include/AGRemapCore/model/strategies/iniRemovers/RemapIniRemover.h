#ifndef AGRemapCore_RemapIniRemover_H
#define AGRemapCore_RemapIniRemover_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AGRemapCore/constants/FileExt.h"
#include "AGRemapCore/constants/IniBoilerPlate.h"
#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/IniSectionGraph.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iniresources/IniResource.h"
#include "AGRemapCore/model/strategies/iniRemovers/BaseIniRemover.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemovalContext.h"
#include "AGRemapCore/model/strategies/iniRemovers/IniRemoveContext.h"


namespace AGRemapCore {

    class IniFile;

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`BaseIniRemover` :raw-html:`<br />` :raw-html:`<br />`

     Removes this software's fix from a ``.ini`` file, by *reachability* rather than by name
     :raw-html:`<br />` :raw-html:`<br />`

     **This is deliberately not a port of** ``IniRemover.py``, the pure-Python class it replaced --
     note the two do not even share a name any more. That original finds the fix with four
     regular expressions over the raw file text (a ``.*RemapBlend``/``.*RemapPosition``/``.*RemapFix``
     /``.*RemapTex`` `section`_-name pattern, plus the boilerplate heading pattern) and deletes
     whatever matches. This one instead *derives* what the fix is:

     #. Every `section`_ the fix boilerplate surrounds is a candidate, whatever it is called
     #. Every `section`_ **outside** the boilerplate whose name contains ``Remap`` is a candidate
        too -- the leftovers an interrupted or partly-undone fix leaves behind
     #. Those candidates are combined into one :cpp:class:`IniSectionGraph`, whose
        :cpp:func:`IniSectionGraph::targetSectionNames` are the candidates this software is taken to
        have written itself. A candidate is a target when **either**

        * it sits inside the boilerplate -- the boilerplate *is* this software's own marker, so
          everything it surrounds qualifies on that alone, **or**
        * some :cpp:class:`IfContentPartColouring` state reachable in the graph gives its ``hash``
          `KVP`_ a value belonging to one of the ``.ini`` file's :cpp:class:`ModType`\\s (see
          #hashBelongsToModType) -- which is what decides for the leftovers *outside* the
          boilerplate, where there is no marker to go on

        A caller that cannot answer that second question, or does not want it asked, sets
        :cpp:member:`IniRemovalContext::ignoreModType` and gets the whole candidate pool as targets
        instead -- which is exactly what ``IniRemover.py`` always did. See that member for when that
        is the right call

     #. What gets deleted is every target, everything a target references, **and** everything that
        references one of those from within the candidate pool -- so no surviving `section`_ is left
        pointing at a deleted one, in either direction. "References" here is deliberately wider than
        the ``run =`` calls :cpp:class:`IniSectionGraph` itself walks -- see #buildReferences
     #. The boilerplate itself is kept when at least one `section`_ it originally surrounded is
        still there afterwards, and removed outright when none is

     :raw-html:`<br />`

     .. note::
        Under the target rule above that last step nearly always lands on "removed": every
        `section`_ inside a boilerplate region is a target, so nothing inside one normally survives
        to keep it alive. That is the intended outcome -- it is what the pure-Python original does
        too, whose ``_fixRemovalPattern`` deletes the whole region outright. The keep half stays
        because it is still reachable through customization (a narrowed #headings, a caller-supplied
        target rule) and because a region holding no `sections`_ at all has to be handled either way

     .. note::
        The ``.ini`` file is reached through an :cpp:class:`IniRemoveContext`, never through
        :cpp:func:`BaseIniRemover::getIniFile` -- see that interface's own note on why. A remover
        bound to a plain :cpp:class:`AGRemapCore::IniFile` (which is what
        :cpp:func:`IniRemoveBuilder::build`, and so :cpp:func:`IniFile::removeFix`, hands it) builds
        and owns an :cpp:class:`IniFileRemoveContext` for itself, and rebuilds it on every
        #setIniFile

     .. note::
        This is what :cpp:func:`IniRemoveBuilder::defaultFactory` hands out, through #factory --
        and therefore what every row of :cpp:class:`IniRemoveBuilderData`, every
        :cpp:member:`ModType::iniRemoveBuilder`, :cpp:func:`GlobalIniRemoveBuilders::removeBuilder`
        and :cpp:func:`IniFile::removeFix` all end up using. It is the only concrete remover
        :cpp:class:`AGRemapCore` has
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     * @tparam RemoverBase
     @rst
     The :cpp:class:`BaseIniRemover` specialization to inherit from :raw-html:`<br />`
     :raw-html:`<br />`

     Spliced in as a template parameter for the same reason :cpp:class:`GIMIParser`'s own
     ``ParserBase`` is: the `pybind11`_ layer's base (``PyBaseIniRemover``) carries `Python`_ state
     that has no core equivalent, and passing it here makes
     ``py::class_<PyRemapIniRemover, PyBaseIniRemover>`` genuine C++ inheritance rather than something
     the binding has to fake :raw-html:`<br />` :raw-html:`<br />`

     **Default**: ``BaseIniRemover<K, V, KeyHash, KeyEqual>``
     @endrst
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>,
              typename RemoverBase = BaseIniRemover<K, V, KeyHash, KeyEqual>>
    class RemapIniRemover: public RemoverBase {
        public:

            /**
             * @brief The type of `section`_ this removes
             */
            using Section = IfTemplate<K, V, KeyHash, KeyEqual>;

            /**
             * @brief
             @rst
             The `KVP`_-holding kind of :cpp:class:`IfTemplatePart` -- what a ``hash`` and a
             ``filename`` are read out of
             @endrst
             */
            using ContentPart = IfContentPart<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The type of caller/callee graph this builds over its candidate `sections`_
             */
            using Graph = IniSectionGraph<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The ``.ini`` file this reads and rewrites, behind an interface
             */
            using Context = IniRemoveContext<K, V, KeyHash, KeyEqual>;

            /**
             * @brief The kind of asset table a :cpp:class:`ModType`'s hashes are, as reached from here
             */
            using Assets = typename Context::Assets;

            /**
             * @brief
             @rst
             The ``.ini``-domain customization points this class needs, for the same reason
             :cpp:class:`IfTemplateRunConfig` and ``GIMISectionClassifier::ClassifierConfig`` exist:
             ``K`` is not ``std::string`` for every instantiation, so the `KVP`_ keys this looks up
             cannot be spelled as literals here
             @endrst
             */
            struct RemoverConfig {
                /**
                 * @brief The `KVP`_ key whose value decides whether a candidate is a target -- :cpp:member:`IniKeywords::Hash` for a plain C++ caller
                 */
                K hashKey;

                /**
                 * @brief The `KVP`_ key that makes a removed `section`_ a resource -- :cpp:member:`IniKeywords::Filename` for a plain C++ caller
                 */
                K filenameKey;

                /**
                 * @brief
                 @rst
                 The domain customization points the :cpp:class:`IniSectionGraph` uses
                 :raw-html:`<br />` :raw-html:`<br />`

                 Its :cpp:member:`IfTemplateRunConfig::sectionNameOf` doubles as this class's
                 ``V``-to-``std::string`` conversion -- for reading a ``filename`` value as a path,
                 and for testing whether a `KVP`_ value names a `section`_ in #buildReferences.
                 That is exactly what it already means ("the `section`_ name this value refers to"),
                 and it saves carrying a second, identical converter
                 @endrst
                 */
                IfTemplateRunConfig<K, V> runConfig;
            };

            /**
             * @brief
             @rst
             The default #RemoverConfig for a plain, ``std::string``-keyed C++ caller
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Only an instantiation whose ``K``/``V`` can be built straight from a ``std::string``
                gets real values here. Any other (notably the `pybind11`_ layer's ``py::object``)
                gets a default-constructed one and supplies its own -- the same constraint
                ``GIMISectionClassifier::defaultConfig`` has
             @endrst
             */
            static RemoverConfig defaultConfig();

            /**
             * @brief
             @rst
             One flavour of fix boilerplate this recognizes -- the C++ stand-in for one arm of the
             pure-Python original's ``_fixRemovalPattern`` alternation :raw-html:`<br />`
             :raw-html:`<br />`

             A boilerplate region opens on a line *starting* ``"; " + side + " " + <title> + " " +
             side`` (where ``side`` is #sideChar repeated #sideLen times) whose ``<title>`` ends
             with #titleSuffix, and closes on the next line reading ``"; "`` followed by nothing but
             #sideChar -- at least ``2 * (sideLen + 1) + title.size() - 2`` of them, which is the
             same lower bound the original's ``close()[:-2] + "(-)*"`` imposes :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                "Starting", not "reading exactly", and the ``<title>`` match is greedy -- both
                because the original's pattern is neither anchored at the end nor applied per line.
                A heading this software really did write,
                ``"; --------------- Raiden Boss Fix -----------------"``, has 15 ``-`` on its left
                border and 17 on its right; a whole-line match would reject it
             @endrst
             */
            struct BoilerPlateHeading {
                /**
                 * @brief
                 @rst
                 What an opening line's title has to end with -- ``"Remap"`` for
                 ``IniBoilerPlate.DefaultHeading``'s ``".*Remap"``, ``"Boss Fix"`` for
                 ``OldHeading``'s ``".*Boss Fix"``
                 @endrst
                 */
                std::string titleSuffix;

                /**
                 * @brief How many characters one side of the heading's border has
                 */
                std::size_t sideLen = IniBoilerPlate::DefaultHeadingSideLen;

                /**
                 * @brief The character the heading's border is drawn with
                 */
                std::string sideChar = IniBoilerPlate::DefaultHeadingSideChar;
            };

            /**
             * @brief
             @rst
             The kinds a removed resource is sorted into by #classifyResource :raw-html:`<br />`
             :raw-html:`<br />`

             Every one of these is also the name of a :cpp:class:`RemapStats` member, and that is the
             point: they are the keys of #getRemovedResources, so a caller can hand each bucket
             straight to the matching :cpp:func:`FileStats::addRemoved` without a lookup table of its
             own. :cpp:member:`RemapStats::ini` is the one member with no kind here -- the ``.ini``
             file is not a resource inside itself
             @endrst
             */
            class ResourceType {
                public:

                    ResourceType() = delete;

                    /**
                     * @brief A file the fix downloaded -- :cpp:member:`RemapStats::download`
                     */
                    static inline const std::string Download = "download";

                    /**
                     * @brief A texture file the fix created from nothing -- :cpp:member:`RemapStats::texAdd`
                     */
                    static inline const std::string TexAdd = "texAdd";

                    /**
                     * @brief A texture file the fix edited a copy of -- :cpp:member:`RemapStats::texEdit`
                     */
                    static inline const std::string TexEdit = "texEdit";

                    /**
                     * @brief A remapped ``Blend.buf`` -- :cpp:member:`RemapStats::blend`
                     */
                    static inline const std::string Blend = "blend";

                    /**
                     * @brief A remapped ``Position.buf`` -- :cpp:member:`RemapStats::position`
                     */
                    static inline const std::string Position = "position";

                    /**
                     * @brief A remapped ``Texcoord.buf`` -- :cpp:member:`RemapStats::texcoord`
                     */
                    static inline const std::string Texcoord = "texcoord";

                    /**
                     * @brief Any other ``.buf`` -- :cpp:member:`RemapStats::buf`
                     */
                    static inline const std::string Buf = "buf";

                    /**
                     * @brief Anything else at all -- :cpp:member:`RemapStats::other`
                     */
                    static inline const std::string Other = "other";
            };

            /**
             * @brief
             @rst
             The boilerplate flavours recognized when a caller names none -- the current
             ``".*Remap"`` heading and the older ``".*Boss Fix"`` one, matching the two arms of the
             pure-Python original's ``_fixRemovalPattern``
             @endrst
             */
            static const std::vector<BoilerPlateHeading>& defaultHeadings();

            /**
             * @brief
             @rst
             An :cpp:type:`IniRemoveBuilder::Factory` that builds one of these over an
             :cpp:class:`IniFileRemoveContext` -- the one line
             :cpp:func:`IniRemoveBuilder::defaultFactory` (and every
             :cpp:class:`IniRemoveBuilderData` row) would need to start handing out real removers
             :raw-html:`<br />` :raw-html:`<br />`

             Only meaningful for the plain ``<std::string, std::string>`` instantiation, since
             :cpp:class:`IniFileRemoveContext` is the only thing an :cpp:class:`IniFile*` can be
             turned into
             @endrst
             */
            static std::function<std::shared_ptr<BaseIniRemover<>>(IniFile*)> factory();

            /**
             * @brief Constructs a new remover
             *
             * @param ctx
             @rst
             The ``.ini`` file to remove the fix from, behind its interface -- non-owning, and it
             must outlive this remover :raw-html:`<br />` :raw-html:`<br />`

             ``nullptr`` leaves the remover unbound; #setIniFile can still bind it to a plain
             :cpp:class:`AGRemapCore::IniFile` afterwards, which builds a context of its own
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param config The .ini-domain customization points to use. **Default**: #defaultConfig
             */
            explicit RemapIniRemover(Context* ctx = nullptr, RemoverConfig config = defaultConfig());

            /**
             * @brief The boilerplate flavours this instance recognizes. **Default**: #defaultHeadings
             */
            std::vector<BoilerPlateHeading> headings = defaultHeadings();

            /**
             * @brief
             @rst
             The substring that makes an *outside-the-boilerplate* `section`_ a candidate
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:member:`IniKeywords::Remap`
             @endrst
             */
            std::string remapKeyword = IniKeywords::Remap;

            /**
             * @brief
             @rst
             The per-position filter #hashBelongsToModType searches a :cpp:class:`ModType`'s
             :cpp:member:`ModType::hashes` with -- see :cpp:func:`ModMappedAssets::getKey`'s own
             ``fromNonVersionVals`` :raw-html:`<br />` :raw-html:`<br />`

             **Default**: empty, ie. no filtering -- a ``hash`` counts as the mod type's when the
             mod type's hash table knows it at all :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Empty is the right default here even though it reads permissive, and the reason is
                worth spelling out. :cpp:member:`ModType::hashes` is a **fully-populated**
                :cpp:class:`Hashes` (see that class's own note) -- every mod type ships the whole
                table, so narrowing this to ``{modType.name, std::nullopt}`` would not mean "this
                mod type's hashes" so much as "the hashes on the *source* side of this remap". A
                fix `section`_ carries the hash of the mod it was remapped **to**
                (``TextureOverrideRaidenShogunRaidenBossRemapBlend`` gets ``RaidenBoss``'s hash, not
                ``Raiden``'s), so that filter would reject exactly the `sections`_ this class exists
                to find. Set it if a caller wants the narrower question asked
             @endrst
             */
            std::vector<std::optional<K>> hashNonVersionVals;

            /**
             * @brief
             @rst
             The `section`_-name substring marking a downloaded resource -- checked before anything
             else, and without looking at the file path at all :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:member:`IniKeywords::RemapDL` (``"RemapDL"``)
             @endrst
             */
            std::string downloadKeyword = IniKeywords::RemapDL;

            /**
             * @brief
             @rst
             The `section`_-name substring that tells a *created* texture from an *edited* one
             :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``"RemapTexAdd"`` :raw-html:`<br />` :raw-html:`<br />`

             .. warning::
                Nothing this software currently writes produces that substring. Texture `sections`_
                are named through :cpp:func:`IniNamingTools::getRemapTexName`, which yields
                ``RemapTex``; ``RemapTexAdd`` exists only as the *resource type* string
                ``"resourceRemapTexAdd"`` (:cpp:class:`RemapTexAddResource`, :cpp:class:`TexEdit`),
                never in a `section`_ name. So every ``.dds`` currently classifies as
                #ResourceType::TexEdit. Kept as specified (the maintainer's explicit call) rather
                than quietly widened to ``RemapTex`` -- change this member if the naming changes
             @endrst
             */
            std::string texAddKeyword = IniKeywords::RemapTex + "Add";

            /**
             * @brief
             @rst
             The `section`_-name substring marking a remapped ``Blend.buf`` -- ``"RemapBlend"``, the
             same string :cpp:func:`IniNamingTools::getRemapBlendName` builds
             @endrst
             */
            std::string blendKeyword = IniKeywords::Remap + IniKeywords::Blend;

            /**
             * @brief
             @rst
             The `section`_-name substring marking a remapped ``Position.buf`` --
             ``"RemapPosition"``, the same string :cpp:func:`IniNamingTools::getRemapPositionName`
             builds
             @endrst
             */
            std::string positionKeyword = IniKeywords::Remap + IniKeywords::Position;

            /**
             * @brief
             @rst
             The `section`_-name substring marking a remapped ``Texcoord.buf`` :raw-html:`<br />`
             :raw-html:`<br />`

             **Default**: ``"RemapTexcoord"`` -- note the lowercase ``c``, which is what
             :cpp:func:`IniNamingTools::getRemapTexcoordName` really builds (it is
             :cpp:member:`IniKeywords::Texcoord`, not ``TexCoord``). This one is spelled to match the
             real output rather than literally as specified, per the maintainer's call
             @endrst
             */
            std::string texcoordKeyword = IniKeywords::Remap + IniKeywords::Texcoord;

            /**
             * @brief
             @rst
             The comment prefix a fix uses to hide the original mod's `sections`_, stripped out of
             every surviving line :raw-html:`<br />` :raw-html:`<br />`

             **Default**: :cpp:member:`IniKeywords::HideOriginalComment` (``";RemapFixHideOrig -->"``)
             :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                This is the counterpart of the ``.ini`` file's own ``hideOriginalSections()``, and
                the equivalent of the pure-Python original's ``_removeFixComment``. Without it,
                undoing a fix that was applied with ``hideOrig`` would leave the *original* mod
                commented out -- the fix's own `sections`_ gone and nothing left switched on. Set it
                empty to skip the strip
             @endrst
             */
            std::string hideOriginalComment = IniKeywords::HideOriginalComment;

            /**
             * @brief
             @rst
             The file extension that sends a resource down the texture branch. Compared
             case-insensitively. **Default**: :cpp:member:`FileExt::DDS`
             @endrst
             */
            std::string texExt = FileExt::DDS;

            /**
             * @brief
             @rst
             The file extension that sends a resource down the buffer branch. Compared
             case-insensitively. **Default**: :cpp:member:`FileExt::Buf`
             @endrst
             */
            std::string bufExt = FileExt::Buf;

            /**
             * @brief The .ini-domain customization points this instance uses
             */
            const RemoverConfig& config() const;

            /**
             * @brief
             @rst
             The ``.ini`` file this removes fixes from, behind its interface, or ``nullptr`` if
             unbound
             @endrst
             */
            Context* getContext() const;

            /**
             * @copydoc getContext() const
             *
             * @param ctx The context to bind to, or ``nullptr`` to unbind
             */
            void setContext(Context* ctx);

            /**
             * @brief
             @rst
             Binds this remover to a plain :cpp:class:`AGRemapCore::IniFile`, building an
             :cpp:class:`IniFileRemoveContext` over it :raw-html:`<br />` :raw-html:`<br />`

             Only the plain ``<std::string, std::string>`` instantiation can do this -- every other
             one leaves #getContext alone, since there is no context an :cpp:class:`IniFile*` can be
             turned into for it. A context handed to the constructor (or to #setContext) is never
             replaced by this
             @endrst
             *
             * @param iniFile The .ini file to remove the fix from, or ``nullptr`` to unbind
             */
            void setIniFile(IniFile* iniFile) override;

            /**
             * @brief
             @rst
             The `sections`_ #remove last chose as :cpp:func:`IniSectionGraph::targetSectionNames`,
             in the order the ``.ini`` file declared them. Empty before the first #remove
             @endrst
             */
            const std::vector<std::string>& getTargetSectionNames() const;

            /**
             * @brief
             @rst
             Every `section`_ name #remove last deleted -- the targets, what they reference and what
             references them -- in the order the ``.ini`` file declared them. Empty before the first
             #remove
             @endrst
             */
            const std::vector<std::string>& getRemovedSectionNames() const;

            /**
             * @brief
             @rst
             The graph #remove last built over its candidate `sections`_, or ``nullptr`` before the
             first #remove :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Its :cpp:func:`IniSectionGraph::targetSectionNames` are #getTargetSectionNames, but
                its :cpp:func:`IniSectionGraph::sections` are only a *subset* of what was removed:
                that class walks ``run =`` calls alone, while the removal set closes over the wider
                reference relation #buildReferences returns. #getRemovedSectionNames is the
                authoritative list

             .. warning::
                Its `sections`_ are **borrowed** from the context's own
                :cpp:func:`IniRemoveContext::sectionIfTemplates`, so this is only good until that
                ``.ini`` file is re-read or re-parsed
             @endrst
             */
            Graph* getRemovalGraph() const;

            /**
             * @brief
             @rst
             Every resource #remove last took out with the `sections`_ that declared it, keyed by
             #ResourceType and, within a kind, in the order the ``.ini`` file declared them
             :raw-html:`<br />` :raw-html:`<br />`

             One entry per ``filename`` `KVP`_ of every removed `section`_ -- a `section`_ naming
             several files in several ``if`` branches contributes one resource per value, and a
             removed `section`_ with no ``filename`` at all (a ``TextureOverride``, a
             ``CommandList``) contributes none. A kind nothing was found for is simply absent rather
             than mapped to an empty vector :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Every one is a plain :cpp:class:`IniResource`, deliberately -- not a
                :cpp:class:`RemapBlendResource`/:cpp:class:`RemapTexAddResource`/
                :cpp:class:`RemapIniDownload`. Those carry the machinery for *producing* a fixed
                file (a ``VGRemap``, a :cpp:class:`TexCreator`, a :cpp:class:`FileDownload`), none
                of which a removal has or needs: what is wanted here is the resolved path of a file
                that is now unreferenced. #ResourceType, kept on :cpp:member:`IniResource::type`, is
                what says which kind it was

             .. note::
                Paths are resolved against :cpp:func:`IniRemoveContext::iniFolder`, the way
                :cpp:class:`IniResource`'s constructor always does. When that is empty -- a ``.ini``
                file with no path, or one whose path is a bare relative file name -- the working
                directory is used instead. Note this is *not* what handing
                :cpp:func:`FileService::absPathOfRelPath` an empty folder does: that throws on
                `MSVC`_, so #remove substitutes ``"."`` deliberately
             @endrst
             */
            const std::unordered_map<std::string, std::vector<std::unique_ptr<IniResource>>>& getRemovedResources() const;

            /**
             * @brief
             @rst
             Whether some :cpp:class:`ModType` of the bound ``.ini`` file knows 'hashVal' -- see
             #hashNonVersionVals
             @endrst
             *
             * @param hashVal The value of a ``hash`` `KVP`_
             */
            bool hashBelongsToModType(const V& hashVal) const;

            /**
             * @brief
             @rst
             Which #ResourceType a resource belongs to, from the name of the `section`_ that
             declared it and the path it points at :raw-html:`<br />` :raw-html:`<br />`

             The decision, in order -- the first match wins:

             #. #downloadKeyword in the `section`_ name -> #ResourceType::Download
             #. 'filePath' ends with #texExt -> #ResourceType::TexAdd when #texAddKeyword is in the
                `section`_ name, otherwise #ResourceType::TexEdit
             #. 'filePath' ends with #bufExt -> #ResourceType::Blend / #ResourceType::Position /
                #ResourceType::Texcoord for the matching keyword, otherwise #ResourceType::Buf
             #. anything else -> #ResourceType::Other

             :raw-html:`<br />`

             .. note::
                The `section`_-name checks are case-**sensitive** (``RemapTexcoord`` and
                ``RemapTexCoord`` are not the same thing, and only the former is ever written); the
                two extension checks are case-**insensitive**, since ``Foo.DDS`` is as much a
                ``.dds`` file as ``foo.dds``
             @endrst
             *
             * @param sectionName The name of the `section`_ the ``filename`` `KVP`_ was found in
             * @param filePath The ``filename`` `KVP`_'s own value, as written in the ``.ini`` file
             *
             * @return One of #ResourceType's members
             */
            std::string classifyResource(const std::string& sectionName, const std::string& filePath) const;

            /**
             * @brief
             @rst
             Removes this software's fix from the ``.ini`` file -- see this class's own note for
             what "the fix" is taken to mean :raw-html:`<br />` :raw-html:`<br />`

             In order: the file's lines are scanned once for boilerplate regions and `section`_
             spans, the candidate pool is collected out of that scan, the targets are found, the
             removal set is closed over both directions of the reference relation, the surviving
             lines are re-joined (and, like the pure-Python original, stripped), and the result is
             handed back through :cpp:func:`IniRemoveContext::setFileTxt` :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                'parse' is accepted and **ignored**. In the pure-Python original it also builds the
                :cpp:class:`IniResource` models for the ``.buf``/``.dds``/downloaded files that go
                with the deleted `sections`_; this class collects those unconditionally instead, and
                hands them back through #getRemovedResources rather than pushing them onto the
                ``.ini`` file

             .. note::
                Two things happen to the surviving text beyond the `section`_ deletion, both
                matching the pure-Python original: #hideOriginalComment is stripped out of it (the
                original's ``_removeFixComment``), and it is stripped of leading/trailing whitespace
                (the original's ``_removeScriptFix``). The ``.ini`` file is then told it no longer
                holds a fix, through :cpp:func:`IniRemoveContext::setIsFixed`
             @endrst
             *
             * @param parse Ignored -- see the note above. **Default**: ``false``
             * @param writeBack
             @rst
             Whether to write the new content out (:cpp:func:`IniRemoveContext::write`) and then
             :cpp:func:`IniRemoveContext::clearRead` the file, exactly as the pure-Python original
             does :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``true``
             @endrst
             * @param context
             @rst
             The per-call options for this removal -- see :cpp:class:`IniRemovalContext`, whose
             :cpp:member:`IniRemovalContext::ignoreModType` is the one thing here that changes which
             `sections`_ come out :raw-html:`<br />` :raw-html:`<br />`

             **Default**: a default-constructed one, ie. the strict target rule described above
             @endrst
             *
             * @return The new text content of the .ini file
             */
            std::string remove(bool parse = false, bool writeBack = true, IniRemovalContext context = IniRemovalContext()) override;

        protected:

            // One occurrence of one section in the file, as line indices into the context's own
            // readFileLines().
            //
            // "Occurrence", not "section": a name can appear more than once (the second copy of a
            // half-fixed file's Resource sections is the standard case), and while
            // IniRemoveContext::sectionIfTemplates keeps only the first, removing a name has to take
            // every span that carries it -- which is what the pure-Python original's
            // removeSectionOptions does too.
            struct SectionSpan {
                std::string name;

                // [startInd, endInd) -- from this section's own "[Name]" header line up to the next
                // boundary, which is the next section header, a boilerplate opening/closing line, or
                // the end of the file. Anything in between (blank lines, comments) belongs to the
                // section it follows, matching IniFile::getSectionOptions' own spans.
                std::size_t startInd = 0;
                std::size_t endInd = 0;

                // Index into scan.boilerPlates, or npos when this occurrence is outside every
                // boilerplate region.
                std::size_t boilerPlateInd = std::string::npos;
            };

            // One fix-boilerplate region, as line indices. Always terminated: see scanFile.
            struct BoilerPlateSpan {
                std::size_t openInd = 0;
                std::size_t closeInd = 0;
            };

            struct FileScan {
                std::vector<SectionSpan> sections;
                std::vector<BoilerPlateSpan> boilerPlates;
            };

            // What an opening line, once matched, tells the scan about the closing line to expect.
            struct OpenMatch {
                // The border character that heading is drawn with -- the closing line has to be
                // drawn with the same one.
                std::string sideChar;

                // The fewest sideChars the closing line may carry.
                std::size_t minSideChars = 0;
            };

            // Walks 'lines' once, finding every boilerplate region and every section occurrence.
            FileScan scanFile(const std::vector<std::string>& lines) const;

            // Whether 'line' opens a boilerplate region, and what would close it.
            std::optional<OpenMatch> matchBoilerPlateOpen(const std::string& line) const;

            // Whether 'line' closes the boilerplate region 'open' described.
            static bool isBoilerPlateClose(const std::string& line, const OpenMatch& open);

            // The names of the candidate sections found by 'scan', in file order and de-duplicated:
            // everything inside a boilerplate region, plus everything outside one whose name
            // contains remapKeyword.
            std::vector<std::string> collectCandidates(const FileScan& scan) const;

            // The candidates this software is taken to have written -- see this class's own doc.
            // 'graph' must already be built over the whole candidate pool, and 'boilerPlateSections'
            // holds every candidate at least one of whose occurrences sits inside a boilerplate
            // region. The returned names are in 'candidates' order.
            //
            // 'candidates' is the FULL candidate list, not just the ones sectionIfTemplates could
            // parse: sitting inside the boilerplate makes a section a target on its own, and only
            // the hash half of the rule needs a parsed section to walk.
            //
            // 'ignoreModType' short-circuits the whole thing to "every candidate is a target" --
            // see IniRemovalContext::ignoreModType. 'graph' and 'boilerPlateSections' are then
            // untouched, since neither half of the rule is asked.
            std::vector<std::string> findTargets(const Graph& graph, const std::vector<std::string>& candidates,
                                                  const std::unordered_set<std::string>& boilerPlateSections,
                                                  bool ignoreModType = false) const;

            // Builds removedResources_ out of the sections named by 'removedNames', looking each one
            // up in 'pool'. One IniResource per filename KVP value, classified by classifyResource,
            // resolved against 'iniFolder'.
            void collectRemovedResources(const std::unordered_map<std::string, Section*>& pool,
                                          const std::vector<std::string>& removedNames, const std::string& iniFolder);

            // Whether 'path' ends with 'ext', ignoring case -- see classifyResource's own note.
            static bool hasExt(const std::string& path, const std::string& ext);

            // Who references whom, within 'pool' -- the edge relation the removal set closes over,
            // in both directions.
            //
            // DELIBERATELY WIDER THAN IniSectionGraph'S OWN EDGES, and this is the whole reason this
            // exists. That class builds its adjacency from `run =` KVPs alone (IfTemplateRunConfig::
            // runKey), but a fix points at its Resource sections with `vb1 =`/`vb0 =`/`ib =`/
            // `ps-t0 =` and frequently has no `run =` to them at all -- see the fix in
            // Testing/.../expected_fullFix_modFixed/fullFix/RaidenShogun/Mod/ei.ini, whose
            // TextureOverride reaches its Resources purely through `vb1 =`. Walking `run =` alone
            // leaves every one of those sections (and every .buf file behind them) in place.
            //
            // Rather than enumerate the keys that can name a section -- a list that would silently
            // rot the day a new one is used -- ANY KVP value that names another section in the pool
            // counts as a reference, whatever its key is. That subsumes `run =` (a run value is just
            // such a name) and needs no maintenance. The pool is already narrow enough (boilerplate
            // contents plus Remap-named leftovers) that a value coinciding with one of its section
            // names by accident is not a real concern.
            std::unordered_map<std::string, std::vector<std::string>> buildReferences(
                const std::unordered_map<std::string, Section*>& pool) const;

            // Everything reachable from 'seeds' by following 'edges', 'seeds' included.
            static std::unordered_set<std::string> closeOver(const std::unordered_map<std::string, std::vector<std::string>>& edges,
                                                              const std::unordered_set<std::string>& seeds);

            // 'edges' with every edge turned around.
            static std::unordered_map<std::string, std::vector<std::string>> reverse(
                const std::unordered_map<std::string, std::vector<std::string>>& edges);

            // 'value' as the section name it would refer to, via the run config -- see
            // RemoverConfig::runConfig. An empty string when no converter was supplied.
            std::string valToStr(const V& value) const;

            // Every occurrence of 'target' removed from 'txt'.
            static std::string removeAll(const std::string& txt, const std::string& target);

            RemoverConfig config_;

            // Borrowed. Points at ownedCtx_ when this remover built its own -- see setIniFile.
            Context* ctx_ = nullptr;

            // Non-null only when this remover built its own context out of an IniFile*.
            std::unique_ptr<Context> ownedCtx_;

            std::vector<std::string> targetSectionNames_;
            std::vector<std::string> removedSectionNames_;
            std::unordered_map<std::string, std::vector<std::unique_ptr<IniResource>>> removedResources_;
            std::unique_ptr<Graph> removalGraph_;
    };
}

#include "RemapIniRemover.tpp"

#endif
