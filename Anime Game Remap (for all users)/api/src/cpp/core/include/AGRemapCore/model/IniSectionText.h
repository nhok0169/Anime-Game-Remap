#ifndef AGRemapCore_IniSectionText_H
#define AGRemapCore_IniSectionText_H
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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AGRemapCore/constants/IniKeywords.h"
#include "AGRemapCore/model/iftemplate/IfContentPart.h"
#include "AGRemapCore/model/iftemplate/IfPredPart.h"
#include "AGRemapCore/model/iftemplate/IfTemplate.h"
#include "AGRemapCore/model/iftemplate/IfTemplateRender.h"
#include "AGRemapCore/tools/z3/Z3Context.h"

namespace AGRemapCore {
    /**
     * @brief
     @rst
     One ``.ini`` `section`_ a strategy writes itself, BUILT rather than concatenated
     :raw-html:`<br />` :raw-html:`<br />`

     :cpp:class:`IfTemplate` is a `section`_, :cpp:class:`IfContentPart` a run of ``key = value``
     lines, :cpp:class:`IfPredPart` an ``if`` / ``endif``, and :cpp:func:`renderIfTemplate` turns the
     three back into text -- including the ``[name]`` header and the indentation. A caller that
     assembles all of that as strings writes the same structure twice: once as the model every other
     part of a fix is edited through, and once as text of its own :raw-html:`<br />`
     :raw-html:`<br />`

     ``WWMIFixer.cpp`` held this class, and twenty section headers' worth of string assembly before
     it (2026-09-29). Two callers still build a `section`_ by hand and are candidates for it --
     ``GIMIComponentFixer.cpp``'s translucency block and ``SideMeshes.cpp`` -- but converting either
     moves GI output, since the renderer indents with a TAB where hand-written text usually has
     nothing, so each needs a GI corpus sweep of its own rather than a tidy-up
     @endrst
     */
    class SectionText {
        public:
            SectionText(Z3Context& z3Ctx, std::string name): z3_(z3Ctx), name_(std::move(name)) {}

            // A run of `key = value` lines. A value of "" is a line with no `=` at all --
            // 3dmigoto's `local $var` -- which is how IfContentPart already reads one.
            SectionText& keys(const std::vector<std::pair<std::string, std::string>>& kvps) {
                parts_.push_back(std::make_unique<IfContentPart<std::string, std::string>>(kvps, depth_));
                return *this;
            }

            SectionText& key(const std::string& k, const std::string& v = "") {
                return keys({{k, v}});
            }

            SectionText& open(const std::string& predicate) {
                parts_.push_back(std::make_unique<IfPredPart>("if " + predicate, IfPredPartType::If, z3_));
                ++depth_;
                return *this;
            }

            // A comment above the `[name]` line. IfTemplate carries it, so even this is
            // the model's rather than text glued on the front.
            SectionText& prefix(std::string text) {
                prefix_ = std::move(text);
                return *this;
            }

            SectionText& close() {
                if (depth_ > 0) {
                    --depth_;
                }

                parts_.push_back(std::make_unique<IfPredPart>("endif", IfPredPartType::EndIf, z3_));
                return *this;
            }

            // The blank line after a section is this file's own convention, not the renderer's.
            std::string str() {
                IfTemplate<std::string, std::string> section{
                    std::move(parts_),
                    IfTemplateRunConfig<std::string, std::string>{
                        IniKeywords::Run,
                        [](const std::string& val) { return val; },
                        [](const std::string& name) { return name; }},
                    name_,
                    IfTemplate<std::string, std::string>::TreeKind::NonEmptyNode,
                    prefix_};

                return renderIfTemplate(section) + "\n\n";
            }

        private:
            Z3Context& z3_;
            std::string name_;
            std::string prefix_;
            std::vector<std::unique_ptr<IfTemplatePart>> parts_;
            int depth_ = 0;
    };
}

#endif
