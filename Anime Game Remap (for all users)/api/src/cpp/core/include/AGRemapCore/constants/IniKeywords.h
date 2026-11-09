#ifndef AGRemapCore_IniKeywords_H
#define AGRemapCore_IniKeywords_H

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

#include <string>
#include <unordered_set>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Common keywords used in the .ini file :raw-html:`<br />` :raw-html:`<br />`

     Bound to `Python`_ as ``IniKeywords``, each member a plain string. Add a member when a caller on
     either side needs a keyword, rather than spelling the literal out where it is used
     @endrst
     */
    class IniKeywords {
        public:

            /**
             * @brief The starting prefix used for any `sections`_ that reference some file
             */
            static inline const std::string Resource = "Resource";

            /**
             * @brief The starting prefix used for some `section`_ that overrides the resource of a mod
             */
            static inline const std::string TextureOverride = "TextureOverride";

            /**
             * @brief
             @rst
             The `section`_ prefix of a ``3dmigoto`` command list -- a `section`_ that is run by name
             rather than matched against a draw
             @endrst
             */
            static inline const std::string CommandList = "CommandList";

            /**
             * @brief
             @rst
             The `KVP`_ key that overrides the resource the `section`_ itself matched
             :raw-html:`<br />` :raw-html:`<br />`

             A texture-only recolour is written with it -- ``hash`` names one of the game's textures
             and ``this`` names what to draw in its place -- so a `section`_ carrying it binds no
             register of its own
             @endrst
             */
            static inline const std::string This = "this";

            /**
             * @brief The starting prefix used for some `section`_ that overrides a mod's shader
             */
            static inline const std::string ShaderOverride = "ShaderOverride";

            /**
             * @brief The substring used to indicate a `section`_ is edited by this software
             */
            static inline const std::string Remap = "Remap";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Blend.buf`` file
             @endrst
             */
            static inline const std::string Blend = "Blend";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Position.buf`` file
             @endrst
             */
            static inline const std::string Position = "Position";

            /**
             * @brief
             @rst
             The substring that usually occurs in the name of a `section`_ to indicate that the
             `section`_ will call some ``*.Texcoord.buf`` file
             @endrst
             */
            static inline const std::string Texcoord = "Texcoord";

            /**
             * @brief The substring used to indicate that the `section`_ was created by this program
             */
            static inline const std::string RemapFix = Remap + "Fix";

            /**
             * @brief
             @rst
             The substring used to indicate that the `section`_ contains some edited/created texture
             ``*.RemapTex.dds`` file
             @endrst
             */
            static inline const std::string RemapTex = Remap + "Tex";

            /**
             * @brief The substring used to indicate that the `section`_ contains some downloaded file from the internet
             */
            static inline const std::string RemapDL = Remap + "DL";

            /**
             * @brief
             @rst
             The substring used to indicate that a resource `section`_ INSIDE a fix's block names a
             file the fix did **not** write -- one of the mod's own, referenced by the fix
             :raw-html:`<br />` :raw-html:`<br />`
             An undo takes every section of the fix's block out and deletes every file those
             sections name, which is right for a ``RemapBlend`` / ``RemapTex`` / ``RemapDL`` file the
             fix produced and destroys the mod for a texture the fix merely bound (the WWMI fixer
             binds a mod's textures by register, and a file no resource of the mod's own ``.ini``
             names has to be declared somewhere). A section carrying this keyword is removed with
             the block and its file is left alone -- see :cpp:func:`RemapIniRemover::collectRemovedResources`
             @endrst
             */
            static inline const std::string RemapRef = Remap + "Ref";

            /**
             * @brief The `KVP`_ key used to reference/call another `section`_
             */
            static inline const std::string Run = "run";

            /**
             * @brief
             @rst
             The `KVP`_ key holding a `section`_'s model hash -- what
             :cpp:class:`GIMISectionClassifier` looks a mod object up by
             @endrst
             */
            static inline const std::string Hash = "hash";

            /**
             * @brief
             @rst
             The `KVP`_ key holding the first index of the model a `section`_ draws -- the second
             half of what :cpp:class:`GIMISectionClassifier` looks a mod object up by, for the mod
             objects a ``hash`` alone cannot tell apart
             @endrst
             */
            static inline const std::string MatchFirstIndex = "match_first_index";

            /**
             * @brief
             @rst
             The `KVP`_ key holding how many indices of the model a `section`_ draws
             @endrst
             */
            static inline const std::string MatchIndexCount = "match_index_count";

            /**
             * @brief
             @rst
             The `KVP`_ key holding how many vertices the model a `section`_ draws has
             @endrst
             */
            static inline const std::string MatchVertexCount = "match_vertex_count";

            /**
             * @brief
             @rst
             The `KVP`_ key breaking the tie when several `sections`_ match one draw
             @endrst
             */
            static inline const std::string MatchPriority = "match_priority";

            /**
             * @brief The ``filter_index`` `KVP`_ key -- the value a shader's own draw writes into
             *      ``IniConstants``, which a remapped `section`_ matches on to tell one pass from another
             */
            static inline const std::string FilterIndex = "filter_index";

            /**
             * @brief The ``array`` `KVP`_ key -- how many elements a buffer resource holds
             */
            static inline const std::string Array = "array";

            /**
             * @brief
             @rst
             The `KVP`_ key holding which kind of draw call a `section`_ matches
             @endrst
             */
            static inline const std::string MatchType = "match_type";

            /**
             * @brief
             @rst
             Every `KVP`_ key by which a `section`_ selects the draw it overrides
             :raw-html:`<br />` :raw-html:`<br />`

             These are what make a ``TextureOverride`` fire, so they are meaningful only where
             3dmigoto is matching. A ``CommandList`` is *called*, never matched, so a fix that
             copies a section's body into one drops these on the way -- carried across they say
             nothing, and a stale ``hash`` in particular reads as a second claim on a draw the
             fix has already remapped
             @endrst
             */
            static inline const std::unordered_set<std::string> MatchKeys = {
                Hash, MatchPriority, MatchFirstIndex, MatchIndexCount, MatchType, MatchVertexCount};

            /**
             * @brief
             @rst
             The `KVP`_ key naming the file a resource `section`_ points at :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Classes that edit `KVPs`_ generically take this as a ``K``-typed customization point
                instead (eg. :cpp:type:`BaseResEdit::ResEditConfig`'s ``filenameKey``), since ``K``
                is not ``std::string`` in the `pybind11`_ layer. This constant is for the
                plain-``std::string`` callers, and for keeping the one spelling in one place
             @endrst
             */
            static inline const std::string Filename = "filename";

            /**
             * @brief
             @rst
             The prefix a register binding uses to name a resource `section`_ rather than a value --
             ``ps-t0 = ref ResourceFoo``. Strip it with
             :cpp:func:`IniNamingTools::removeRefPrefix` rather than by hand: it was written out at
             seven sites across three files, one of them carrying the literal's LENGTH as a bare 4
             @endrst
             */
            static inline const std::string Ref = "ref";

            /**
             * @brief
             @rst
             A resource `section`_'s kind, eg. ``type = Buffer``
             @endrst
             */
            static inline const std::string Type = "type";

            /**
             * @brief
             @rst
             A resource `section`_'s bytes per element, eg. ``stride = 40``
             @endrst
             */
            static inline const std::string Stride = "stride";

            /**
             * @brief
             @rst
             A resource `section`_'s element format, eg. ``format = R32_UINT``
             @endrst
             */
            static inline const std::string Format = "format";

            /**
             * @brief
             @rst
             The value a `KVP`_ carries to mean "nothing at all" -- a ``filename =`` naming this is
             a placeholder, not a real resource
             @endrst
             */
            static inline const std::string Null = "null";

            /**
             * @brief
             @rst
             The statement that fills a resource with a copy of another: ``ResourceX = copy ResourceY``
             @endrst
             */
            static inline const std::string Copy = "copy";

            /**
             * @brief
             @rst
             The statement that gives a resource another's description only: ``ResourceX = copy_desc ResourceY``
             @endrst
             */
            static inline const std::string CopyDesc = "copy_desc";

            /**
             * @brief
             @rst
             The `KVP`_ key naming a `section`_'s index buffer -- also the last index column of the
             :cpp:class:`Hashes` row holding that buffer's hash
             @endrst
             */
            static inline const std::string Ib = "ib";

            /**
             * @brief The `KVP`_ key naming a `section`_'s position vertex buffer
             */
            static inline const std::string Vb0 = "vb0";

            /**
             * @brief
             @rst
             The `KVP`_ key naming a `section`_'s blend **or** texcoord vertex buffer -- which of
             the two it is depends on the `section`_, not on the key
             @endrst
             */
            static inline const std::string Vb1 = "vb1";

            /**
             * @brief
             @rst
             The `KVP`_ key controlling how a draw call is handled -- ``skip`` is the only value
             this software writes, alongside \ref Draw, after a downloaded ``Blend.buf``
             @endrst
             */
            static inline const std::string Handling = "handling";

            /**
             * @brief
             @rst
             The `KVP`_ key issuing a non-indexed draw, written as ``<vertex count>,0`` -- the pair
             \ref Handling completes after a downloaded ``Blend.buf``
             @endrst
             */
            static inline const std::string Draw = "draw";

            /**
             * @brief
             @rst
             The `KVP`_ key that actually draws a model. What a texture fix has to be issued in
             front of -- see :cpp:class:`RegDelimitedAdd`
             @endrst
             */
            static inline const std::string DrawIndexed = "drawindexed";

            /**
             * @brief
             @rst
             The sub-command call to `ORFix`_ (Outline Reflection Fix) :raw-html:`<br />`
             :raw-html:`<br />`

             One of the two fixes shipped by an external ``3dmigoto`` library that work around
             textures the game breaks each version. A ``run =`` naming this is a call into that
             library, not into the mod's own `sections`_
             @endrst
             */
            static inline const std::string ORFixPath = "CommandList\\global\\ORFix\\ORFix";

            /**
             * @brief
             @rst
             The sub-command call to ``NNFix`` (No Normal Fix) -- the other half of the same
             external library as \ref ORFixPath :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                It lives under the library's ``ORFix`` folder, **not** an ``NNFix`` one. That
                asymmetry is the library's, not a typo here
             @endrst
             */
            static inline const std::string NNFixPath = "CommandList\\global\\ORFix\\NNFix";

            /**
             * @brief
             @rst
             GIMI's own texture helper, ``CommandList\GIMI\SetTextures`` -- ORFix reading
             :cpp:member:`GIMINormalMap` / :cpp:member:`GIMIDiffuse` / :cpp:member:`GIMILightMap`
             instead of ``ps-t0`` / ``ps-t1`` / ``ps-t2``. A mod's use of it is rewritten into the
             traditional API as it is read -- see :cpp:class:`GIMIApiNormalizer`
             @endrst
             */
            static inline const std::string GIMISetTexturesPath = "CommandList\\GIMI\\SetTextures";

            /**
             * @brief The normal map key of GIMI's texture API -- read as ``ps-t0``, see \ref GIMISetTexturesPath
             */
            static inline const std::string GIMINormalMap = "Resource\\GIMI\\NormalMap";

            /**
             * @brief The diffuse key of GIMI's texture API -- read as ``ps-t1``, see \ref GIMISetTexturesPath
             */
            static inline const std::string GIMIDiffuse = "Resource\\GIMI\\Diffuse";

            /**
             * @brief The light map key of GIMI's texture API -- read as ``ps-t2``, see \ref GIMISetTexturesPath
             */
            static inline const std::string GIMILightMap = "Resource\\GIMI\\LightMap";

            /**
             * @brief
             @rst
             The folder holding the `TexFx`_ external library's sub-commands :raw-html:`<br />`
             :raw-html:`<br />`

             A third addon alongside ORFix and NNFix, and the one that gives textures capabilities
             the importer does not have on its own -- most visibly making parts of a model look
             transparent. What each channel of an input texture means is documented on its own
             GitHub rather than here :raw-html:`<br />` :raw-html:`<br />`

             It owns **two dedicated registers**, ``ps-t69`` and ``ps-t70``, which is why a fix that
             shifts a character's registers around leaves those two alone
             @endrst
             */
            static inline const std::string TexFxFolder = "CommandList\\TexFx";

            /**
             * @brief
             @rst
             The first of `TexFx`_'s two dedicated registers :raw-html:`<br />` :raw-html:`<br />`

             A modder opts into TexFx by binding these, which is what makes the library optional
             where NNFix and ORFix are mandatory -- so a fix issues a TexFx sub-command only where
             one of them is actually bound, never simply because the character's row names one
             @endrst
             */
            static inline const std::string PsT69 = "ps-t69";

            /**
             * @brief
             @rst
             `TexFx`_'s variable that, at ``-1``, makes it leave a draw to the game's own shader
             @endrst
             */
            static inline const std::string TexFxUseDefaultShader = "$\\TexFx\\use_default_shader";

            /**
             * @brief The second of `TexFx`_'s two dedicated registers -- see \ref PsT69
             */
            static inline const std::string PsT70 = "ps-t70";

            /**
             * @brief
             @rst
             `TexFx`_'s transparency sub-command for a diffuse living on ``ps-t0``, GI 5.0 and later
             :raw-html:`<br />` :raw-html:`<br />`

             The call names the register the diffuse is on, so which of these two a fix issues
             follows from where its register shift left the diffuse -- not from the character
             @endrst
             */
            static inline const std::string TexFxTransparency0 = TexFxFolder + "\\TN.0";

            /**
             * @brief
             @rst
             `TexFx`_'s transparency sub-command for a diffuse living on ``ps-t1`` -- see
             \ref TexFxTransparency0
             @endrst
             */
            static inline const std::string TexFxTransparency1 = TexFxFolder + "\\TN.1";

            /**
             * @brief
             @rst
             The PRE-5.0 spelling of \ref TexFxTransparency0 -- ``T.0`` rather than ``TN.0``
             :raw-html:`<br />` :raw-html:`<br />`

             The same sub-command, which `TexFx`_ itself renamed at 5.0. A fix transcribed from a
             4.x row has to issue THIS one: the two are not interchangeable, and issuing the 5.0
             name names a sub-command that library does not have -- which surfaces as an effect
             that quietly does nothing rather than as an error
             @endrst
             */
            static inline const std::string TexFxTransparency0Pre5_0 = TexFxFolder + "\\T.0";

            /**
             * @brief
             @rst
             The PRE-5.0 spelling of \ref TexFxTransparency1 -- see \ref TexFxTransparency0Pre5_0
             @endrst
             */
            static inline const std::string TexFxTransparency1Pre5_0 = TexFxFolder + "\\T.1";

            /**
             * @brief
             @rst
             Written in place of a ``hash`` that has no mapping onto the mod being fixed to --
             see :cpp:class:`RegAssetRemap`
             @endrst
             */
            static inline const std::string HashNotFound = "HashNotFound";

            /**
             * @brief
             @rst
             Written in place of a ``match_first_index`` that has no mapping onto the mod being
             fixed to -- the index counterpart of :cpp:member:`HashNotFound`
             @endrst
             */
            static inline const std::string IndexNotFound = "IndexNotFound";

            /**
             * @brief The comment marker used to hide (comment out) a previously-fixed original `section`_
             */
            static inline const std::string HideOriginalComment = ";RemapFixHideOrig -->";

            /**
             * @brief
             @rst
             The statement keyword that declares a variable for the whole ``.ini`` file: ``global $x = 0``
             @endrst
             */
            static inline const std::string Global = "global";

            /**
             * @brief
             @rst
             The statement keyword that declares a variable for one `section`_: ``local $x``
             @endrst
             */
            static inline const std::string Local = "local";

            /**
             * @brief
             @rst
             The keyword that runs a line after the frame's draws rather than before them: ``post $x = 0``
             @endrst
             */
            static inline const std::string Post = "post";

            /**
             * @brief
             @rst
             The :cpp:member:`Handling` value that stops the game drawing what a `section`_ matched
             @endrst
             */
            static inline const std::string Skip = "skip";

            /**
             * @brief
             @rst
             The :cpp:member:`Type` of a resource holding a vertex, index or constant buffer
             @endrst
             */
            static inline const std::string Buffer = "Buffer";

            /**
             * @brief
             @rst
             The `section`_ 3dmigoto runs once a frame, after every draw
             @endrst
             */
            static inline const std::string Present = "Present";

            /**
             * @brief
             @rst
             The `section`_ 3dmigoto runs once, when the ``.ini`` file loads -- where ``global`` variables are declared
             @endrst
             */
            static inline const std::string Constants = "Constants";

            /**
             * @brief
             @rst
             The namespace of WWMI's own variables and command lists, which every WWMI mod's ``.ini`` calls into
             @endrst
             */
            static inline const std::string WWMINamespace = "$\\WWMIv1";

            /**
             * @brief
             @rst
             The first merged-skeleton bone of the component a WWMI draw is for
             @endrst
             */
            static inline const std::string WWMIVgOffset = WWMINamespace + "\\vg_offset";

            /**
             * @brief
             @rst
             How many merged-skeleton bones the component a WWMI draw is for has
             @endrst
             */
            static inline const std::string WWMIVgCount = WWMINamespace + "\\vg_count";

            /**
             * @brief
             @rst
             The checksum WWMI checks a mod's shape keys against before applying them
             @endrst
             */
            static inline const std::string WWMIShapeKeyChecksum = WWMINamespace + "\\shapekey_checksum";

            /**
             * @brief
             @rst
             How many groups WWMI's shape-key compute pass dispatches over the game's own shape keys
             @endrst
             */
            static inline const std::string WWMIShapeKeyDispatchSizeYOriginalBatch = WWMINamespace + "\\shapekey_dispatch_size_y_original_batch";

            /**
             * @brief
             @rst
             The scale WWMI's skeleton shaders apply to a mod's mesh
             @endrst
             */
            static inline const std::string WWMICustomMeshScale = WWMINamespace + "\\custom_mesh_scale";

            /**
             * @brief
             @rst
             Which of a mod's blend remaps WWMI's ``BlendRemapper`` applies
             @endrst
             */
            static inline const std::string WWMIBlendRemapId = WWMINamespace + "\\blend_remap_id";

            /**
             * @brief
             @rst
             How many bone influences a vertex of the mod's ``Blend.buf`` carries, as WWMI's ``BlendRemapper`` reads it
             @endrst
             */
            static inline const std::string WWMIWeightsPerVertexCount = WWMINamespace + "\\weights_per_vertex_count";

            /**
             * @brief
             @rst
             The vertex count WWMI's compute shaders run over
             @endrst
             */
            static inline const std::string WWMICustomVertexCount = WWMINamespace + "\\custom_vertex_count";

            /**
             * @brief
             @rst
             WWMI's shader that merges one component's bones into the mod's merged skeleton
             @endrst
             */
            static inline const std::string WWMISkeletonMerger = "CustomShader\\WWMIv1\\SkeletonMerger";

            /**
             * @brief
             @rst
             WWMI's shader that remaps a merged skeleton through a blend remap
             @endrst
             */
            static inline const std::string WWMISkeletonRemapper = "CustomShader\\WWMIv1\\SkeletonRemapper";

            /**
             * @brief
             @rst
             WWMI's shader that writes a mod's remapped ``Blend.buf`` at load
             @endrst
             */
            static inline const std::string WWMIBlendRemapper = "CustomShader\\WWMIv1\\BlendRemapper";

            /**
             * @brief
             @rst
             The register a draw reads its bones from (``vs-cb4``): what a skeleton merge takes, and a remapped skeleton is
             bound back to
             @endrst
             */
            static inline const std::string WWMIBoneDataCb = "vs-cb4";

            /**
             * @brief
             @rst
             The register a draw reads its previous frame's bones from (``vs-cb3``), the extra merged skeleton's
             @endrst
             */
            static inline const std::string WWMIExtraBoneDataCb = "vs-cb3";

            /**
             * @brief
             @rst
             :cpp:member:`WWMISkeletonMerger`'s input: the bone-data constant buffer it merges (``cs-cb8``)
             @endrst
             */
            static inline const std::string WWMIMergerBonesReg = "cs-cb8";

            /**
             * @brief
             @rst
             :cpp:member:`WWMISkeletonMerger`'s output: the merged skeleton it writes (``cs-u6``)
             @endrst
             */
            static inline const std::string WWMIMergerSkeletonReg = "cs-u6";

            /**
             * @brief
             @rst
             :cpp:member:`WWMISkeletonRemapper`'s input: the blend remap's forward table, local id to merged bone (``cs-t37``)
             @endrst
             */
            static inline const std::string WWMIRemapperForwardReg = "cs-t37";

            /**
             * @brief
             @rst
             :cpp:member:`WWMISkeletonRemapper`'s input: the merged skeleton it remaps (``cs-t38``)
             @endrst
             */
            static inline const std::string WWMIRemapperSkeletonReg = "cs-t38";

            /**
             * @brief
             @rst
             :cpp:member:`WWMISkeletonRemapper`'s output: the remapped skeleton it writes (``cs-u5``)
             @endrst
             */
            static inline const std::string WWMIRemapperOutReg = "cs-u5";

            /**
             * @brief
             @rst
             :cpp:member:`WWMIBlendRemapper`'s input: the blend remap's reverse table, merged bone to local id (``cs-t34``)
             @endrst
             */
            static inline const std::string WWMIBlendRemapperReverseReg = "cs-t34";

            /**
             * @brief
             @rst
             :cpp:member:`WWMIBlendRemapper`'s input: every vertex's merged bone ids (``cs-t35``)
             @endrst
             */
            static inline const std::string WWMIBlendRemapperVertexVGReg = "cs-t35";

            /**
             * @brief
             @rst
             :cpp:member:`WWMIBlendRemapper`'s output: the blend it rewrites into the remap's local ids (``cs-u4``)
             @endrst
             */
            static inline const std::string WWMIBlendRemapperOutReg = "cs-u4";

            /**
             * @brief
             @rst
             A resource's ``type`` for a buffer a compute shader writes
             @endrst
             */
            static inline const std::string RWBuffer = "RWBuffer";

            /**
             * @brief
             @rst
             A resource's ``format`` of four 32-bit floats an element -- eg. a merged skeleton's bone matrices
             @endrst
             */
            static inline const std::string FormatR32G32B32A32Float = "R32G32B32A32_FLOAT";

            /**
             * @brief
             @rst
             A resource's ``format`` of one unsigned byte an element
             @endrst
             */
            static inline const std::string FormatR8UInt = "DXGI_FORMAT_R8_UINT";

            /**
             * @brief
             @rst
             A resource's ``format`` of one unsigned 16-bit integer an element
             @endrst
             */
            static inline const std::string FormatR16UInt = "DXGI_FORMAT_R16_UINT";

            /**
             * @brief
             @rst
             The flag every `section`_ of a WWMI mod sets when its character is drawn, and its ``[Present]`` resets each frame
             @endrst
             */
            static inline const std::string WWMIObjectDetected = "$object_detected";

            /**
             * @brief
             @rst
             The flag a WWMI mod draws under once WWMI has registered it
             @endrst
             */
            static inline const std::string WWMIModEnabled = "$mod_enabled";

            /**
             * @brief
             @rst
             The ``global`` a WWMI mod declares its mesh's vertex count in
             @endrst
             */
            static inline const std::string WWMIMeshVertexCount = "$mesh_vertex_count";

            /**
             * @brief
             @rst
             WWMI's per-frame state counter, which a mod compares to run a merge once a frame
             @endrst
             */
            static inline const std::string WWMIStateId = "$state_id";

            /**
             * @brief
             @rst
             A WWMI Tools export's ``Blend.buf`` resource
             @endrst
             */
            static inline const std::string WWMIBlendBuffer = "ResourceBlendBuffer";

            /**
             * @brief
             @rst
             The resource WWMI's ``BlendRemapper`` fills with a mod's remapped blend
             @endrst
             */
            static inline const std::string WWMIBlendBufferOverride = "ResourceBlendBufferOverride";

            /**
             * @brief
             @rst
             A WWMI Tools export's index buffer resource
             @endrst
             */
            static inline const std::string WWMIIndexBuffer = "ResourceIndexBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's ``Position.buf`` resource
             @endrst
             */
            static inline const std::string WWMIPositionBuffer = "ResourcePositionBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's ``Texcoord.buf`` resource
             @endrst
             */
            static inline const std::string WWMITexcoordBuffer = "ResourceTexcoordBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's ``Vector.buf`` resource: each vertex's normal and tangent
             @endrst
             */
            static inline const std::string WWMIVectorBuffer = "ResourceVectorBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's shape-key offsets: the first entry of each key slot -- see :cpp:class:`WWMIShapeKeys`
             @endrst
             */
            static inline const std::string WWMIShapeKeyOffsetBuffer = "ResourceShapeKeyOffsetBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's shape-key vertex ids -- see :cpp:class:`WWMIShapeKeys`
             @endrst
             */
            static inline const std::string WWMIShapeKeyVertexIdBuffer = "ResourceShapeKeyVertexIdBuffer";

            /**
             * @brief
             @rst
             A WWMI Tools export's shape-key vertex offsets -- see :cpp:class:`WWMIShapeKeys`
             @endrst
             */
            static inline const std::string WWMIShapeKeyVertexOffsetBuffer = "ResourceShapeKeyVertexOffsetBuffer";

            /**
             * @brief
             @rst
             RabbitFX's command list that binds the maps a mod hands it and sets its routing flags -- see :cpp:class:`RabbitFX`
             @endrst
             */
            static inline const std::string RabbitFXSetTexturesPath = "CommandList\\RabbitFX\\SetTextures";

            /**
             * @brief
             @rst
             The folder RabbitFX's resources live under -- a mod hands RabbitFX its maps as resources under it
             @endrst
             */
            static inline const std::string RabbitFXResourceFolder = "Resource\\RabbitFX";
    };
}

#endif
