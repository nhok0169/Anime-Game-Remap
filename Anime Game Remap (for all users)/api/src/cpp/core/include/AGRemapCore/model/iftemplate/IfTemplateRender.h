#ifndef AGRemapCore_IfTemplateRender_H
#define AGRemapCore_IfTemplateRender_H

#include <string>

#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Renders one :cpp:class:`IfContentPart` as ``.ini`` text -- one ``key = value`` line per KVP,
     in true positional order
     @endrst
     *
     * @param part The part to render
     * @param linePrefix Prepended to every line -- indentation, or a comment marker
     *
     * @return The rendered lines, joined by ``"\n"`` with no trailing newline
     */
    std::string renderIfContentPart(const IfContentPart<std::string, std::string>& part,
                                     const std::string& linePrefix = "");

    /**
     * @brief
     @rst
     Renders one :cpp:class:`IfTemplate` -- a whole ``.ini`` `section`_ -- as text
     :raw-html:`<br />` :raw-html:`<br />`

     **Free functions, not methods.** Neither :cpp:class:`IfTemplate` nor
     :cpp:class:`IfContentPart` has a ``toStr`` of its own, and that is deliberate: what a
     `section`_ looks like is the caller's business, which is why
     :cpp:type:`GIMIFixer::SectionToStr` is a callback rather than a virtual. These are the
     *default* answer a caller can reach for -- :cpp:func:`IniFixBuilder::defaultFactory` passes
     this one -- not an answer forced on every caller :raw-html:`<br />` :raw-html:`<br />`

     Lifted from the pybind11 layer's own ``IfTemplate.toStr`` binding, which was already C++
     working on this exact instantiation, but rendered each content part by calling back into
     `Python`_. That round-trip is why a core-only caller had no renderer at all
     @endrst
     *
     * @param section The `section`_ to render
     * @param linePrefix Prepended to every line -- indentation, or a comment marker
     * @param autoindent
     @rst
     Whether to indent the body of each ``if``/``elif``/``else`` block by one tab per level.
     ``endif`` and ``elif``/``else`` dedent before they are written, so they line up with their
     opening ``if``
     @endrst
     *
     * @return The rendered `section`_, joined by ``"\n"`` with no trailing newline
     */
    std::string renderIfTemplate(IfTemplate<std::string, std::string>& section,
                                  const std::string& linePrefix = "", bool autoindent = true);
}

#endif
