#ifndef AGRemapCore_TexCreatorEdit_H
#define AGRemapCore_TexCreatorEdit_H

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
#include <vector>

#include "AGRemapCore/model/iniresources/RemapTexResource.h"
#include "AGRemapCore/model/strategies/iniFixers/graphGroupEdits/resEdits/TexEdit.h"
#include "AGRemapCore/model/strategies/texEditors/TexCreator.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`TexCreate`

     A :cpp:class:`TexCreate` that actually **creates** the texture it names, by building a real
     :cpp:class:`RemapTexAddResource` around a caller-supplied :cpp:class:`TexCreator` --- and the
     `section`_ that points at it :raw-html:`<br />` :raw-html:`<br />`

     **The third class of this shape, and the pattern is now worth naming.**
     :cpp:class:`RemapBlendReplace` needed :cpp:class:`VGRemapBlendReplace`,
     :cpp:class:`TexReplace` needed :cpp:class:`TexEditorReplace`, and
     :cpp:class:`TexCreate` needs this. In each case the core class does the naming and the
     `pybind11`_ subclass does the work, so a fix driven from **plain C++** renames everything
     correctly, rewrites the registers, reports success, and writes no file at all
     :raw-html:`<br />` :raw-html:`<br />`

     This one was missing **two** overrides rather than one, which is why
     :cpp:class:`TexCreate` is not merely inert from C++ but abstract:

     ===================== ================================================================
     :cpp:func:`buildResModel`  builds the :cpp:class:`RemapTexAddResource` that holds the
                                :cpp:class:`TexCreator` and writes the ``.dds``
     :cpp:func:`buildSection`   builds the ``[Resource...]`` `section`_ naming that file ---
                                pure virtual on :cpp:class:`ResCreate`, since a created
                                resource has no existing `section`_ to copy
     ===================== ================================================================

     Use it through :cpp:class:`ResRegCollect`, exactly as the `blend`_ and the texture edit are:

     .. code-block:: cpp

        normalMapCollect_.srcRegs  = {{headGraph, "ps-t0"}};
        normalMapCollect_.resEdits = {{"head", normalMapCreate_.get()}};

     .. note::
        The :cpp:class:`TexCreator` is held **by value**, for the same reason
        :cpp:class:`TexEditorReplace` holds its editor that way

     .. note::
        The `sections`_ this builds are owned **here**, in \\ref sections_, because
        :cpp:func:`ResCreate::buildSection` hands back a borrowed pointer and the graph only copies
        it when asked to. \\ref clear drops them along with the texture counter
     @endrst
     *
     * @tparam K The type of the keys stored in a referenced :cpp:class:`IfContentPart`
     * @tparam V The type of the values stored in a referenced :cpp:class:`IfContentPart`
     * @tparam KeyHash A hasher for ``K``. Defaults to ``std::hash<K>``
     * @tparam KeyEqual An equality comparator for ``K``. Defaults to ``std::equal_to<K>``
     */
    template <typename K = std::string, typename V = std::string, typename KeyHash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
    class TexCreatorCreate: public TexCreate<K, V, KeyHash, KeyEqual> {
        public:
            using Base = TexCreate<K, V, KeyHash, KeyEqual>;
            using GraphId = typename Base::GraphId;
            using ResEditConfig = typename Base::ResEditConfig;
            using Context = typename Base::Context;
            using Section = typename Base::Section;

            /**
             * @brief The creator that produces every texture this resource edit adds
             */
            TexCreator texCreator;

            /**
             * @brief Constructs a new texture-creating resource edit
             *
             * @param resModObj The mod object to hold the newly created graph for the resource
             * @param texName The name the created texture is filed under, eg. ``"NormalMap"``
             * @param texCreator The creator that produces each texture
             * @param config The domain customization points this instance uses
             * @param resType The name of the type of resource. **Default**: ``"resourceRemapTexAdd"``
             */
            TexCreatorCreate(GraphId resModObj, std::string texName, TexCreator texCreator,
                              ResEditConfig config, std::string resType = "resourceRemapTexAdd");

            /**
             * @brief
             @rst
             Builds the ``[Resource...]`` `section`_ for a created texture --- a single
             ``filename =`` `KVP`_ naming the ``.dds`` this edit writes
             @endrst
             *
             * @param sectionName The name of the section to build
             * @param modName The name of the mod being fixed to
             *
             * @return The section, owned by this edit
             */
            Section* buildSection(const std::string& sectionName, const std::string& modName) override;

            /**
             * @brief
             @rst
             Builds the :cpp:class:`RemapTexAddResource` that runs \\ref texCreator and writes
             'srcPath'
             @endrst
             */
            void buildResModel(const std::string& resType, const std::string& srcPath,
                                const std::string& fixedPath, const std::string& modName,
                                const std::string& fileKey, Context& ctx) override;

            /**
             * @brief Clears the texture counter and every section this edit built
             */
            void clear() override;

        private:
            std::vector<std::unique_ptr<Section>> sections_;
    };
}

#include "TexCreatorEdit.tpp"

#endif
