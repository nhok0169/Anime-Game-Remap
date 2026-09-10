#ifndef AGRemapCore_TexEditorEdit_H
#define AGRemapCore_TexEditorEdit_H

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

#include <optional>
#include <string>

#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEdit.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`TexReplace`

     A :cpp:class:`TexReplace` that actually **edits** the texture it names, by building a real
     :cpp:class:`RemapTexEditResource` around a caller-supplied :cpp:class:`TexEditor`
     :raw-html:`<br />` :raw-html:`<br />`

     This exists for the same reason :cpp:class:`VGRemapBlendReplace` does, and the two are worth
     reading together. :cpp:class:`TexReplace` names things correctly but deliberately does not
     override :cpp:func:`BaseResEdit::buildResModel`, because the texture editor that does the work
     reaches it from `Python`_ -- so a fix driven from **plain C++** collects the textures, renames
     them, rewrites the registers that point at them, reports success, and writes no edited file at
     all. The failure is completely silent: the ``.ini`` file looks right and the texture it names
     is simply never created :raw-html:`<br />` :raw-html:`<br />`

     Use it through :cpp:class:`ResRegCollect`, exactly as the blend is:

     .. code-block:: cpp

        faceCollect_.srcRegs  = {{faceGraph, "ps-t0"}};
        faceCollect_.resEdits = {{"face", faceReplace_.get()}};

     .. note::
        The :cpp:class:`TexEditor` is held **by value**. It is small (a vector of
        ``std::function``\\ s), it has to outlive every resource this builds, and copying it is what
        lets one fixer hand the same edit to several resources without owning a separate lifetime
        for it
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class TexEditorReplace: public TexReplace<K, V, KeyHash, KeyEqual> {
        public:
            using Base = TexReplace<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;

            /**
             * @brief The editor run over every texture this resource edit collects
             */
            TexEditor texEditor;

            /**
             * @brief Constructs a new editing texture resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param texEditor The editor to run over each collected texture
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexEdit"``
             * @param resSubType The name of the subtype of the resource, folded into every fixed name. **Default**: none
             */
            TexEditorReplace(GraphId resModObj, TexEditor texEditor, ResEditConfig config,
                              std::string resType = "resourceRemapTexEdit",
                              std::optional<std::string> resSubType = std::nullopt);

            /**
             * @brief
             @rst
             Builds the :cpp:class:`RemapTexEditResource` that reads 'srcPath', runs \ref texEditor
             over it and writes 'fixedPath'
             @endrst
             */
            void buildResModel(const std::string& resType, const std::string& srcPath,
                                const std::string& fixedPath, const std::string& modName,
                                const std::string& fileKey, Context& ctx) override;
    };
}

#include "TexEditorEdit.tpp"

#endif
