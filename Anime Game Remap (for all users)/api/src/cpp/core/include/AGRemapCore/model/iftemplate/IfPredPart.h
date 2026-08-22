#ifndef AGRemapCore_IfPredPart_H
#define AGRemapCore_IfPredPart_H

#include <optional>
#include <string>

#include "AGRemapCore/constants/IfPredPartType.h"
#include "AGRemapCore/model/iftemplate/IfTemplatePart.h"
#include "AGRemapCore/tools/parsing/ParseContext.h"
#include "AGRemapCore/tools/z3/Z3Context.h"
#include "AGRemapCore/tools/z3/Z3Predicate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`IfTemplatePart`

     Class for defining the predicate part of an `IfTemplate` :raw-html:`<br />` :raw-html:`<br />`

     The full replacement for the pure-Python ``IfPredPart`` -- the old pure-Python class (which
     used a `sympy`_ query, not this class's own :cpp:class:`Z3Predicate`) now lives on as
     ``IfPredPartOld`` (``model/iftemplate/IfPredPartOld.py``), kept only as a deprecated fallback.
     Registered under the bare ``IfPredPart`` Python name (no ``Cpp`` prefix) -- see this class's
     own git history/PR description for the fuller reasoning. The `Z3`_-based dataflow-analysis
     "Ini Graph Editing" subsystem (``IniSectionGraph.py``/``ResGroupCollect.py``) builds its own
     query combination (``&``/``|``/``~``/``simplify``/``isSatisfiable``, see
     :cpp:class:`Z3Predicate`) directly on top of this class's #query
     @endrst
     */
    class IfPredPart : public IfTemplatePart {
        public:

            /**
             * @brief Constructs a new predicate part
             *
             * @param src The original string within the `IfTemplate`
             * @param type The type of predicate encountered
             * @param z3Ctx
             @rst
             The `Z3`_ context #query will belong to -- shared across every :cpp:class:`IfPredPart`
             constructed against the same `Z3`_ context, so the same-named variable across several
             predicates interns to the same `Z3`_ constant (see :cpp:class:`Z3Context`)
             @endrst
             * @param ctx
             @rst
             The context for parsing the predicate, if #type is
             :cpp:enumerator:`IfPredPartType::If`/:cpp:enumerator:`IfPredPartType::Elif` and
             'query' isn't already given :raw-html:`<br />` :raw-html:`<br />`

             If given, this is mutated in place (its #ParseContext::lines replaced with
             #getTestStr's result) so it reflects exactly what was parsed -- matching the
             pure-Python original's own ``ctx.lines = testStr.splitlines()``. If ``nullptr``, a
             fresh, throwaway :cpp:class:`ParseContext` is constructed internally instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``nullptr``
             @endrst
             * @param query
             @rst
             The associated `Z3`_ predicate :raw-html:`<br />` :raw-html:`<br />`

             If this value is ``std::nullopt`` and #type is
             :cpp:enumerator:`IfPredPartType::If`/:cpp:enumerator:`IfPredPartType::Elif`, will parse
             the predicate from 'src' instead (see #getLogicQuery) :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             * @param id
             @rst
             The id for the part. If this parameter is ``std::nullopt``, will generate a new id for
             the part :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             */
            explicit IfPredPart(std::string src, IfPredPartType type, Z3Context& z3Ctx, ParseContext* ctx = nullptr,
                                 std::optional<Z3Predicate> query = std::nullopt, const std::optional<size_t>& id = std::nullopt);

            /**
             * @brief The original string within the `IfTemplate`
             */
            std::string src;

            /**
             * @brief The type of predicate encountered
             */
            IfPredPartType type;

            /**
             * @brief
             @rst
             The associated `Z3`_ predicate for this part :raw-html:`<br />` :raw-html:`<br />`

             ``std::nullopt`` for :cpp:enumerator:`IfPredPartType::EndIf`, or when parsing 'src'
             failed
             @endrst
             */
            std::optional<Z3Predicate> query;

            /**
             * @brief
             @rst
             Creates a copy of this part -- a plain value copy (#query's own
             :cpp:func:`Z3Predicate::Z3Predicate(const Z3Predicate&)` copy constructor), not a
             re-parse of #src :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param newId
             @rst
             Whether to generate a new id for the part :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``false``
             @endrst
             *
             * @return The cloned part
             */
            IfPredPart clone(bool newId = false) const;

            /**
             * @brief
             @rst
             Retrieves #src with #type's own leading keyword (and, for
             :cpp:enumerator:`IfPredPartType::If`/:cpp:enumerator:`IfPredPartType::Elif`, a trailing
             ``then`` keyword) stripped -- the actual predicate text to parse
             @endrst
             *
             * @return The stripped predicate text
             */
            std::string getTestStr() const;

            /**
             * @brief
             @rst
             Generates the corresponding `Z3`_ predicate from a conditional predicate's source text
             @endrst
             *
             * @param ctx The parsing context for reading the conditional predicate
             * @param z3Ctx The `Z3`_ context the generated predicate will belong to
             *
             * @return The generated `Z3`_ predicate, or ``std::nullopt`` if 'ctx' could not be
             *      tokenized/parsed/converted
             */
            static std::optional<Z3Predicate> getLogicQuery(const ParseContext& ctx, Z3Context& z3Ctx);

            /**
             * @brief Generates the .ini predicate text used in the if/else-if/else parts of a .ini
             *      file for some already-built `Z3`_ predicate
             *
             * @param predicate The predicate to render
             *
             * @return The generated predicate text, or ``std::nullopt`` if 'predicate' contains a
             *      construct with no .ini predicate equivalent (see
             *      :cpp:func:`Z3IfPredGenerator::generate`)
             */
            static std::optional<std::string> getIfPredStr(const Z3Predicate& predicate);

            /**
             * @brief
             @rst
             Rebuilds 'predicate' as an equivalent :cpp:class:`Z3Predicate` belonging to a
             *different* :cpp:class:`Z3Context` -- a plain "render to .ini text, then re-parse
             against the target context" round trip (#getIfPredStr followed by #getLogicQuery),
             the only way to move a predicate across `Z3`_ contexts at all (a ``z3::expr`` has no
             cross-context conversion of its own -- see :cpp:class:`Z3Context`'s own ``.. warning::``
             for the constraint this exists to work around: two predicates can only ever be
             combined -- eg. via :cpp:func:`Z3Predicate::operator&` -- when they share the same
             underlying context) :raw-html:`<br />` :raw-html:`<br />`
             @endrst
             *
             * @param predicate The predicate to reparent
             * @param target The `Z3`_ context the returned predicate will belong to
             *
             * @return The reparented predicate, or ``std::nullopt`` if 'predicate' contains a
             *      construct with no .ini predicate equivalent (see #getIfPredStr) or otherwise
             *      fails to re-parse against 'target'
             */
            static std::optional<Z3Predicate> reparent(const Z3Predicate& predicate, Z3Context& target);

            /**
             * @brief Retrieves the part as a string
             *
             * @param linePrefix
             @rst
             The string that will prefix #src :raw-html:`<br />` :raw-html:`<br />`

             If ``std::nullopt``, #src is used as-is. Otherwise, any left spacing from #src is
             stripped and 'linePrefix' is prepended instead :raw-html:`<br />` :raw-html:`<br />`

             **Default**: ``std::nullopt``
             @endrst
             *
             * @return The string representation of the part
             */
            std::string toStr(const std::optional<std::string>& linePrefix = std::nullopt) const;
    };
}

#endif
