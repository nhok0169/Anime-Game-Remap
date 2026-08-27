#ifndef AGRemapCore_RemapBlendResource_H
#define AGRemapCore_RemapBlendResource_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "AGRemapCore/model/VGRemap.h"
#include "AGRemapCore/model/buffers/BufElementType.h"
#include "AGRemapCore/model/files/BlendFile.h"
#include "AGRemapCore/model/iniresources/RemapIniResource.h"


namespace AGRemapCore {

    /**
     * @brief
     @rst
     This class inherits from :cpp:class:`RemapIniFixResource`

     Class for fixing some ``Blend.buf`` file used by the overall remap process -- mirrors the
     pure-Python ``RemapBlendResource`` class (``model/iniresources/RemapBlendResource.py``)
     :raw-html:`<br />` :raw-html:`<br />`

     .. note::
        #fixExists isn't re-overridden here -- the Python original re-declares it with an identical
        body to what :cpp:class:`RemapIniFixResource` (its own base) already does
        (``os.path.isfile(fixedPath)``), so this class just inherits :cpp:func:`RemapIniFixResource::fixExists`
        directly rather than duplicating it

        #_fix's return value: same "the Python original has no ``return`` statement here at all, so
        it always implicitly returns ``None``/falsy" gap as :cpp:class:`RemapIniDownload::_fix` --
        see that class's own doc comment. This port returns ``true`` on success instead
     @endrst
     */
    class RemapBlendResource: public RemapIniFixResource {
        public:

            /**
             * @brief Constructs a new blend resource
             *
             * @param iniFolderPath The path to the folder of the .ini file
             * @param srcPath The file path to the resource
             * @param fixedPath The file path to the fixed resource
             * @param vgRemap The vertex group remap for the ``Blend.buf`` file
             * @param type The name for the type of resource
             * @param fixFunc Custom function for fixing the resource, overriding #_fix if given
             * @param blendElements
             @rst
             The sequence of elements for constructing the ``Blend.buf`` file -- if this is empty,
             :cpp:class:`BlendFile` uses the elements for a GIMI character instead (see
             :cpp:class:`BlendFile`'s own constructor) :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                None of 'type'/'fixFunc'/'blendElements' have a default value here, unlike most
                other trailing constructor parameters in this codebase -- 'blendElements' can't (see
                :cpp:class:`IniGroupedResource`'s own constructor doc comment for the MSVC
                move-only-container-default-argument quirk this works around), and since a
                defaulted parameter can't precede a non-defaulted one, 'type'/'fixFunc' lost their
                defaults too rather than reordering this constructor's parameter list. Pass explicit
                values for all three from C++; the pybind11 binding supplies its own independent
                Python-side defaults regardless
             @endrst
             */
            RemapBlendResource(const std::string& iniFolderPath, const std::string& srcPath, const std::string& fixedPath, VGRemap vgRemap,
                                std::string type, std::function<bool(RemapBlendResource&)> fixFunc,
                                std::vector<std::unique_ptr<BufElementType>> blendElements);

            /**
             * @brief
             @rst
             .. note::
                Explicitly declared (rather than left implicit) to work around the same MSVC
                eager-instantiation-of-a-deleted-copy-constructor quirk documented on
                :cpp:class:`IniGroupedResource`'s own explicit special member declarations --
                #blendElements is a ``std::vector`` of ``unique_ptr``, which triggers it identically
             @endrst
             */
            RemapBlendResource(const RemapBlendResource&) = delete;
            RemapBlendResource& operator=(const RemapBlendResource&) = delete;
            RemapBlendResource(RemapBlendResource&&) = default;
            RemapBlendResource& operator=(RemapBlendResource&&) = default;

            /**
             * @brief The vertex group remap for the ``Blend.buf`` file
             */
            VGRemap vgRemap;

            /**
             * @brief The sequence of elements for constructing the ``Blend.buf`` file (see the constructor)
             */
            std::vector<std::unique_ptr<BufElementType>> blendElements;

            /**
             * @brief Custom function for fixing the resource, overriding #_fix if set
             */
            std::function<bool(RemapBlendResource&)> fixFunc;

            bool srcEncounteredError(const RemapStats& stats) const override;
            bool srcIsFixed(const RemapStats& stats) const override;
            bool fixEncounteredError(const RemapStats& stats) const override;
            bool fixIsFixed(const RemapStats& stats) const override;

            /**
             * @brief Creates the blend file -- a fresh copy of #blendElements is cloned into it, leaving this resource's own copy untouched
             *
             * @return The created blend file
             */
            BlendFile createBlend() const;

            /**
             * @brief Fixes the resource -- calls #fixFunc if set, otherwise #_fix
             *
             * @return Whether the resource was fixed
             */
            bool fix();

        protected:

            /**
             * @brief Performs a vertex group remap on the ``Blend.buf`` file
             *
             * @return Whether the resource was fixed
             */
            virtual bool _fix();
    };
}

#endif
