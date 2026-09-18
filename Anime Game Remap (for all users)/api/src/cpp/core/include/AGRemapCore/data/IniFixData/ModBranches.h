#ifndef AGRemapCore_ModBranches_H
#define AGRemapCore_ModBranches_H

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

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <tsl/ordered_map.h>

#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/strategies/iniFixers/graphEdits/RegBranchAdd.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     One value a register takes in a mod, with the condition it is taken under
     :raw-html:`<br />` :raw-html:`<br />`

     The condition is the whole reason this is not a plain string: which branch of one component's
     ``CommandList`` goes with which branch of another's is decided by whether the two conditions can
     hold at the same time, so a value that arrives without its condition cannot be paired at all
     @endrst
     */
    struct BranchVal {
        /**
         * @brief The value -- a register's raw value, or whatever a caller resolved it to (eg. a file path)
         */
        std::string val;

        /**
         * @brief The condition the value is taken under, or ``std::nullopt`` for no condition at all
         */
        std::optional<Z3Predicate> query;
    };


    /**
     * @brief
     @rst
     What a fixer needs to read a mod whose values differ PER BRANCH, shared by every fixer
     template that has to cope with a merged master :raw-html:`<br />` :raw-html:`<br />`

     A merged master is several mods behind one ``.ini``: its ``TextureOverride`` carries nothing
     but ``hash``, ``match_first_index`` and ``run =``, and every buffer, register and draw sits in a
     ``$swapvar`` branch of the ``CommandList`` it runs. So every number measured from the mod's
     files -- buffers, index counts, vertex counts, whether the mod draws for itself -- is per
     branch, and this is the one place that reads a value together with its branch and answers
     which branch a later query belongs to :raw-html:`<br />` :raw-html:`<br />`

     Component-agnostic on purpose. It knows nothing of blends, slots or which direction a fix goes:
     the split (one component onto several) and the merge (several onto one) both read through it,
     and so will a fix of several components onto several

     .. warning::
        **Owns the** :cpp:class:`Z3Context` **every** :cpp:class:`BranchVal` **it hands out belongs
        to**, and a :cpp:class:`Z3Predicate` is usable only while its context is alive. Declare an
        instance BEFORE every member holding one of its values, so it is destroyed after them. Not
        copyable or movable for the same reason
     @endrst
     */
    class ModBranches {
        public:
            using Template = IfTemplate<std::string, std::string>;
            using Templates = tsl::ordered_map<std::string, std::unique_ptr<Template>>;

            ModBranches() = default;
            ModBranches(const ModBranches&) = delete;
            ModBranches& operator=(const ModBranches&) = delete;
            ModBranches(ModBranches&&) = delete;
            ModBranches& operator=(ModBranches&&) = delete;

            /**
             * @brief The context every query this reads or reparents belongs to
             */
            Z3Context& context();

            /**
             * @brief
             @rst
             Every value of ``key`` in ``rootSection`` AND in everything it ``run =``\\s, each with
             the condition it sits under :raw-html:`<br />` :raw-html:`<br />`

             Root section first, then every section it reaches in the call graph's own order, so the
             first value is the root's own binding where it has one. Built over the RAW parsed
             sections, because a fixer is constructed before the parser runs
             @endrst

             * @param templates The ``.ini`` file's parsed sections -- see :cpp:func:`IniFile::getIfTemplates`
             * @param rootSection The section to start from
             * @param key The register to read
             */
            std::vector<BranchVal> valsThroughRun(const Templates& templates, const std::string& rootSection,
                                                  const std::string& key);

            /**
             * @brief The first value #valsThroughRun finds, or ``std::nullopt``
             */
            std::optional<std::string> firstValThroughRun(const Templates& templates, const std::string& rootSection,
                                                          const std::string& key);

            /**
             * @brief
             @rst
             A query some graph of the library built, reparented into #context :raw-html:`<br />`
             :raw-html:`<br />`

             Testing it against a :cpp:class:`BranchVal` reparents one side on every call otherwise
             -- a full render / re-parse round trip -- so a query tested against several candidates
             is reparented once here first
             @endrst

             * @param query The query, or ``nullptr`` for none
             */
            std::optional<Z3Predicate> localQuery(const Z3Predicate* query);

            /**
             * @brief Whether two conditions can hold at the same time
             */
            bool compatible(const Z3Predicate& a, const Z3Predicate& b);

            /**
             * @brief
             @rst
             WHICH of ``branches`` a query belongs to, or -1 for "cannot say" :raw-html:`<br />`
             :raw-html:`<br />`

             Exactly one candidate satisfiable with it is the whole test: a part inside
             ``$swapvar == 3`` rules out every other branch, while a section's unconditional preamble
             is satisfiable with all of them and is correctly declined. A value that does not branch
             has one candidate, and every query belongs to it
             @endrst
             */
            long long branchIndexOf(const std::vector<BranchVal>& branches, const std::optional<Z3Predicate>& query);

            /**
             * @brief
             @rst
             Which of a register's values a query takes :raw-html:`<br />` :raw-html:`<br />`

             SATISFIABILITY, not position: a candidate is taken when its condition can hold at the
             same time as the query's, which is indifferent to how the conditions are shaped -- an
             ``if`` / ``else if`` chain, independent toggles, a value that does not branch at all.
             The first satisfiable candidate wins: several are satisfiable at once only where the
             query does not constrain this register, and any of them is then a correct reading
             @endrst

             * @param candidates The register's values
             * @param fallback What to take when there are no candidates at all
             * @param query The query, or ``std::nullopt`` to take the first candidate
             */
            std::string pick(const std::vector<BranchVal>& candidates, const std::string& fallback,
                             const std::optional<Z3Predicate>& query);

            /**
             * @brief
             @rst
             Whether any of ``vals`` can be taken under ``query`` -- also true where either side has
             no condition, since an unconditional value is taken everywhere
             @endrst
             */
            bool anyCompatible(const std::vector<BranchVal>& vals, const std::optional<Z3Predicate>& query);

            /**
             * @brief
             @rst
             Every state of the mod that several per-branch values describe TOGETHER
             :raw-html:`<br />` :raw-html:`<br />`

             No single register's branches are the mod's states in general: an animated master binds
             ONE blend for its whole frame range and a different index buffer per frame, so the
             blend's branches say there is one state and the index buffers say there are eleven.
             Each list refines the states so far -- a state splits into one state per value
             satisfiable with it, and is kept as it is where none is -- so the result is every
             combination that can actually be selected. A list of one value refines nothing
             @endrst

             * @param lists The per-branch values that decide what differs between states
             * @return The states, each a condition in #context -- or a single ``std::nullopt`` when nothing branches
             */
            std::vector<std::optional<Z3Predicate>> states(const std::vector<const std::vector<BranchVal>*>& lists);

            /**
             * @brief What one branch's replacements are, given the branch's index and its query in #context
             */
            using BranchReplacements = std::function<RegBranchAdd<>::Additions(std::size_t, const std::optional<Z3Predicate>&)>;

            /**
             * @brief
             @rst
             An edit SETTING keys to a different value in each branch of ``branches`` :raw-html:`<br />`
             :raw-html:`<br />`

             For the numbers a branch already carries and carries differently from its neighbours --
             a blend's ``draw``, whose count is that variant's own vertex count. A part that cannot
             be attributed to exactly one branch (see #branchIndexOf) is left alone, and so is a
             branch whose replacements come back empty
             @endrst

             * @param branches The values that define the branches, eg. every blend a ``CommandList`` binds
             * @param keyPrefix Tells this edit's branches apart from another edit's over the same graph
             * @param replacementsOf The replacements for one branch
             */
            std::unique_ptr<RegBranchAdd<>> replacePerBranch(std::vector<BranchVal> branches, std::string keyPrefix,
                                                             BranchReplacements replacementsOf);

            /**
             * @brief The first value of ``key`` in one section's own parts, ignoring ``run =``
             */
            static std::optional<std::string> firstVal(const Template& tpl, const std::string& key);

            /**
             * @brief
             @rst
             A register value naming a resource, as that resource's name -- or ``""`` for no value,
             an empty one, or ``null``
             @endrst
             */
            static std::string resourceOf(const std::optional<std::string>& value);

            /**
             * @brief
             @rst
             The absolute path of the file a resource section names, or ``""`` when the resource has
             no section or its section no ``filename``
             @endrst

             * @param templates The ``.ini`` file's parsed sections
             * @param resource The resource's section name
             * @param folder The folder the ``.ini`` file sits in
             */
            static std::string fileOf(const Templates& templates, const std::string& resource, const std::string& folder);

            /**
             * @brief
             @rst
             How many bytes one index takes in the buffer a resource names, from the resource
             section's own ``format`` -- see :cpp:func:`IbFile::bytesPerIndexOf`. 4 when it declares
             none
             @endrst
             */
            static std::size_t ibBytesPerIndexOf(const Templates& templates, const std::string& resource);

        private:
            Z3Context ctx_;
    };
}

#endif
