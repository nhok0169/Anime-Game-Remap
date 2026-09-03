#ifndef AGRemapPyBind_PyIniSectionGraph_H
#define AGRemapPyBind_PyIniSectionGraph_H

#include <pybind11/pybind11.h>

#include "iftemplate/PyIfContentPart.h"  // reuses PyIfContentPart/PyObjectHash/PyObjectEqual
#include "AGRemapCore/model/IniSectionGraph.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


using PyIniSectionGraphCore = AGRC::IniSectionGraph<std::string, std::string>;


/**
 * @brief
 @rst
 The `pybind11`_-facing name for `AGRC::IniSectionGraph`\\<py::object, py::object\\>. Registered
 under the bare ``IniSectionGraph`` name; the deprecated pure-Python original this replaced has
 been removed.

 :raw-html:`<br />`

 .. warning::
    :attr:`sections` are borrowed (not owned) unless constructed with ``copySections = True`` --
    see `AGRC::IniSectionGraph`'s own top-level note. This is a real, load-bearing subclass (not a
    plain alias like every other binding in this port): the core class only holds *raw*
    :class:`IfTemplate` pointers for its sections, matching the pure-Python original's own
    ``self.sections = sections`` -- but the pure-Python original gets a real keep-alive *for free*
    there, since storing a reference to the whole ``dict`` object automatically keeps every value
    inside it alive via the dict's own refcounting, even if the caller never keeps a separate
    reference to an inline-constructed section (eg.
    ``IniSectionGraph({"a": IfTemplate(...)}, ["a"])`` -- extremely common, including in this
    port's own test suite). A raw-pointer-only C++ port has no such automatic protection, so
    #keepAlive_ exists specifically to replicate it: a real `Python`_-level container holding a
    strong reference to every section `pybind11`_ wrapper currently reachable from #sections(),
    refreshed synchronously (via #refreshKeepAlive) at the end of every binding that can change
    which sections are reachable (the constructor, :meth:`build`, :meth:`combine`) -- confirmed via
    a real, reproduced use-after-free crash during this port's own testing that skipping this step
    is not just theoretical.

    :raw-html:`<br />`

    #refreshKeepAlive additionally snapshots every reachable *part* (not just section) into
    #partsKeepAlive_, for the same reason one level down: :cpp:func:`pyIdOfPart`'s ``id(part)``
    correlation contract (see `AGRC::CallGraph`'s own top-level note) needs every
    :class:`IfContentPart` to have a *stable*, already-registered wrapper, but
    ``py::cast(ptr, reference)`` only reuses an existing wrapper -- it doesn't keep one alive by
    itself. Without this, a part nobody's Python-side code has directly touched yet (eg. one
    sitting inside an inline-constructed ``IfTemplate([IfContentPart(...)])``, same as the
    section-level case above) gets a fresh, unprotected wrapper that dies the instant
    :cpp:func:`pyIdOfPart`'s local ``py::object`` goes out of scope -- and the *next* part
    processed in the same loop can land at that just-freed address, silently colliding two
    different parts onto the same ``id()``. Confirmed via a real, reproduced ``id()``-collision
    regression (missing insertions in ``RegSurroundedAdd``) during this port's own testing that
    skipping this step is not just theoretical.

    :raw-html:`<br />`

    #z3CtxKeepAlive_ is the same story a third time, for the optional ``z3Ctx`` constructor
    argument: the core only ever stores a raw, non-owning ``Z3Context*`` (#AGRC::IniSectionGraph's
    own ``z3Ctx_``), set once at construction and never reassigned (not even by :meth:`deepcopy`,
    which deliberately shares the *same* pointer rather than copying it -- see
    `AGRC::IniSectionGraph::deepcopy`'s own note). An inline-constructed
    ``IniSectionGraph(..., z3Ctx = Z3Context())`` (exactly as common as the inline-``IfTemplate``
    case above, and exercised by this port's own test suite) leaves nothing else holding a
    reference to that ``Z3Context`` wrapper once the constructor call returns -- confirmed via a
    real, reproduced access-violation crash (on interpreter shutdown, well after construction
    completed successfully) during this port's own testing that skipping this step is not just
    theoretical. Populated by the constructor and propagated (not recomputed -- there's nothing to
    walk, just the one pointer) by :meth:`deepcopy`/``__copy__``/``__deepcopy__`` to whichever
    fresh instance shares the source's ``z3Ctx_``.
 @endrst
 */
class PyIniSectionGraph : public PyIniSectionGraphCore {
    public:
        using PyIniSectionGraphCore::PyIniSectionGraphCore;

        /**
         * @brief
         @rst
         Move-promotes an already-built base-typed instance into this derived type -- needed
         because `AGRC::IniSectionGraph::deepcopy` constructs a fresh base-typed object directly
         (it's generic, Python-free code with no notion of this Python-only subclass), not a
         `PyIniSectionGraph`. #keepAlive_ starts empty (correct for a fresh #deepcopy result: every
         section it holds is genuinely owned by its own ``ownedSections_``, not borrowed, so it
         needs no keep-alive entries of its own until #refreshKeepAlive is called anyway).
         @endrst
         */
        explicit PyIniSectionGraph(PyIniSectionGraphCore&& base): PyIniSectionGraphCore(std::move(base)) {

        }

        /**
         * @brief
         @rst
         Rebuilds #keepAlive_ from every section pointer currently reachable via #sections(), and
         #partsKeepAlive_ from every part pointer reachable via each of those sections' own
         ``parts()``.

         :raw-html:`<br />`

         Must be called synchronously, before returning to `Python`_, from any binding method that
         may have changed #sections()'s membership -- see this class's own top-level note.
         @endrst
         */
        void refreshKeepAlive();

        /**
         * @brief
         @rst
         Sets #z3CtxKeepAlive_ to a strong reference to 'z3Ctx' (or clears it, for ``py::none()``)
         -- see this class's own top-level note. Must be called synchronously, before returning to
         `Python`_, by the constructor (with its own ``z3Ctx`` argument) and by
         :meth:`deepcopy`/``__copy__``/``__deepcopy__`` (with the *source* instance's own
         #z3CtxKeepAlive via #z3CtxKeepAlive).
         @endrst
         */
        void setZ3CtxKeepAlive(py::object z3Ctx);

        /**
         * @brief The current value of #z3CtxKeepAlive_ -- see #setZ3CtxKeepAlive
         */
        py::object z3CtxKeepAlive() const;

    private:
        py::dict keepAlive_;

        /**
         * @brief
         @rst
         A real `Python`_-level container holding a strong reference to every part `pybind11`_
         wrapper currently reachable from any section in #keepAlive_ -- see this class's own
         top-level note.
         @endrst
         */
        py::list partsKeepAlive_;

        /**
         * @brief A strong reference to this instance's ``z3Ctx`` wrapper (``py::none()`` if none) -- see this class's own top-level note
         */
        py::object z3CtxKeepAlive_ = py::none();
};


void initCppIniSectionGraph(pybind11::module_ &m);

#endif
