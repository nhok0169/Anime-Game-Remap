# -----------------------------------
# Link-time optimization
# -----------------------------------

# pybind11_add_module() links pybind11::lto unless NO_EXTRAS is passed, which on MSVC means /GL
#   on all ~149 binding translation units and -LTCG on the link. That is the right trade for a
#   shipped wheel and the wrong one for the edit-compile-test loop, because the LTCG link is
#   single-threaded, takes ~56s, and runs on EVERY build no matter how little changed:
#
#     change one .cpp in core/src  :   57.2s with LTO  ->    9.1s without
#     rebuild all 149 binding TUs  :  109.8s with LTO  ->  178.5s without
#
#   Both directions are real, and the second one is why this is an option rather than a deletion.
#   /GL makes compiling cheaper by deferring codegen to link time, so LTO wins on a full rebuild
#   and loses badly on a small one; the crossover sits around 110 of the 149 files. Day-to-day
#   iteration is nowhere near that, hence: off for python_dev, on for the cibuildwheel/core_sdk
#   builds that actually ship binaries.
#
# There is also less on the table here than it looks: AGRemapCore is a plain static lib built
#   without /GL, so LTCG only ever optimizes across the thin binding glue, never into the library.
#
# pybind11 adds its LTO flags only `if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)`, so
#   merely DEFINING that variable -- to OFF -- is what suppresses them. It must therefore be set
#   before pybind11_add_module() below, not after.
if ("${BUILD_MODE}" STREQUAL "${PYTHON_DEV_MODE}")
    set(AGREMAP_LTO_DEFAULT OFF)
else()
    set(AGREMAP_LTO_DEFAULT ON)
endif()

option(AGREMAP_ENABLE_LTO "Link-time optimization for the 'core' extension module"
       ${AGREMAP_LTO_DEFAULT})

if(NOT AGREMAP_ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
endif()

# NOTE: this target is deliberately NOT unity-built, unlike AGRemapCore. It would be the bigger
#   win of the two (measured 11x on a 19-file subset, 21x with a precompiled header), but 21
#   anonymous-namespace helpers in py/src share names across files -- translateBufFileErrors in
#   5, intVectorMapFromDict in 4, ModObj / factoryFromPy / makeRunConfig / parseDownloadMode in
#   3 each -- and a unity batch turns each of those into a redefinition error. Renaming them, or
#   hoisting them into a shared header, unblocks it.

pybind11_add_module(core
    src/bindings.cpp
    src/tools/PyAlgo.cpp
    src/tools/PyBiMap.cpp
    src/tools/PyGraphTools.cpp
    src/view/PyBaseLogger.cpp
    src/view/PyLogger.cpp
    src/tools/PyIntTools.cpp
    src/tools/PyListTools.cpp
    src/tools/PyRanges.cpp
    src/tools/dfa/PyDFA.cpp
    src/tools/tries/PyTrie.cpp
    src/tools/tries/PyAhoCorasickDFA.cpp
    src/tools/orderedMultiMap/PyOrderedMultiMap.cpp
    src/tools/orderedMultiMap/PyOrderedMultiMapSqrt.cpp
    src/tools/orderedMultiMap/PyIOrderedMultiMap.cpp
    src/model/iftemplate/PyIfContentPart.cpp
    src/model/iftemplate/PyIfContentPartColour.cpp
    src/model/PyVersion.cpp
    src/model/assets/PyModDictAssets.cpp
    src/model/assets/PyModMappedAssets.cpp
    src/model/assets/PyModAssets.cpp
    src/model/assets/PyHashes.cpp
    src/model/assets/PyIndices.cpp
    src/model/assets/PyVertexCounts.cpp
    src/model/assets/PyVGRemaps.cpp
    src/model/strategies/PyModTypeIdData.cpp
    src/model/strategies/PyModType.cpp
    src/model/strategies/iniClassifiers/PyIniClassifyStats.cpp
    src/model/strategies/iniClassifiers/PyBaseIniClassifier.cpp
    src/model/strategies/iniClassifiers/PyIniClassifier.cpp
    src/constants/PyGameTypeId.cpp
    src/constants/PyModTypeId.cpp
    src/constants/PyGlobalModTypes.cpp
    src/constants/PyGIBuilder.cpp
    src/constants/PyStrategyOverrides.cpp
    src/tools/parsing/PyToken.cpp
    src/tools/parsing/PyParseContext.cpp
    src/tools/parsing/PyBaseTokenizer.cpp
    src/tools/parsing/PyFilteredTokenizer.cpp
    src/tools/parsing/PyIfPredTokenizer.cpp
    src/tools/parsing/PySympyTokenizer.cpp
    src/tools/nodes/PyParseNode.cpp
    src/tools/parsing/PyParseTree.cpp
    src/tools/parsing/PyBaseSLR1Parser.cpp
    src/model/iftemplate/PySympyParser.cpp
    src/model/iftemplate/PyIfPredParser.cpp
    src/tools/z3/PyZ3Context.cpp
    src/tools/z3/PyZ3Predicate.cpp
    src/model/iftemplate/PyIfPredPart.cpp
    src/model/iftemplate/PyIfTemplateNode.cpp
    src/model/iftemplate/PyIfTemplateTree.cpp
    src/model/iftemplate/PyIfTemplate.cpp
    src/model/PyCallGraph.cpp
    src/model/PySectionIterData.cpp
    src/model/PyIniSectionGraph.cpp
    src/model/PyIniGraphGroup.cpp
    src/model/strategies/iniFixers/regEdits/PyBaseRegEdit.cpp
    src/model/strategies/iniFixers/regEdits/PyRegAdd.cpp
    src/model/strategies/iniFixers/regEdits/PyRegNewVals.cpp
    src/model/strategies/iniFixers/regEdits/PyRegRemap.cpp
    src/model/strategies/iniFixers/regEdits/PyRegRemove.cpp
    src/model/strategies/iniFixers/graphEdits/PyBaseIniGraphEdit.cpp
    src/model/strategies/iniFixers/graphEdits/PyGraphRename.cpp
    src/model/strategies/iniFixers/graphEdits/PyRegFillMissing.cpp
    src/model/strategies/iniFixers/graphEdits/PyRegSurroundedAdd.cpp
    src/model/strategies/iniFixers/graphEdits/PyRegDelimitedAdd.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyIniGraphGroups.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyBaseIniGraphGroupEdit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyGraphRemove.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyGraphInherit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyGraphGroupRemap.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyGraphGroupEdit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/resEdits/PyResEdit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/resEdits/PyBlendEdit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/resEdits/PyTexEdit.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyResRegCollect.cpp
    src/model/strategies/iniFixers/graphGroupEdits/PyResGroupCollect.cpp
    src/model/strategies/iniParsers/PyBaseIniParser.cpp
    src/model/strategies/iniParsers/PyGIMISectionClassifier.cpp
    src/model/strategies/iniParsers/PyGIMIParser.cpp
    src/model/strategies/iniParsers/PyIniParseBuilder.cpp
    src/model/strategies/iniFixers/PyBaseIniFixer.cpp
    src/model/strategies/iniFixers/PyGIMIFixer.cpp
    src/model/strategies/iniFixers/PyMultiModFixer.cpp
    src/model/strategies/iniFixers/PyIniFixBuilder.cpp
    src/model/strategies/iniRemovers/PyBaseIniRemover.cpp
    src/model/strategies/iniFixers/PyIniFixingContext.cpp
    src/model/strategies/iniRemovers/PyIniRemovalContext.cpp
    src/model/strategies/iniRemovers/PyRemapIniRemover.cpp
    src/model/strategies/iniRemovers/PyGlobalRemapIniRemover.cpp
    src/model/strategies/iniRemovers/PyIniRemoveBuilder.cpp
    src/tools/hashing/PyHash64.cpp
    src/tools/hashing/PyHash128.cpp
    src/tools/hashing/PyHashTools.cpp
    src/model/buffers/PyBufBytes.cpp
    src/model/buffers/PyBufType.cpp
    src/model/buffers/PyBufDataType.cpp
    src/model/buffers/PyBufInt.cpp
    src/model/buffers/PyBufFloat.cpp
    src/model/buffers/PyBufUnorm.cpp
    src/model/buffers/PyBufElementType.cpp
    src/model/files/PyBinaryFile.cpp
    src/model/files/PyIniFile.cpp
    src/PyRemapService.cpp
    src/PyRemapServiceCLI.cpp
    src/model/files/PyBufFile.cpp
    src/model/PyVGRemap.cpp
    src/model/files/PyBlendFile.cpp
    src/model/files/PyPositionFile.cpp
    src/model/files/PyIbFile.cpp
    src/model/files/PyVbFile.cpp
    src/model/strategies/bufEditors/PyBaseBufEditor.cpp
    src/model/strategies/bufEditors/PyBufEditor.cpp
    src/model/textures/PyColour.cpp
    src/model/textures/PyColourRange.cpp
    src/model/files/PyTextureFile.cpp
    src/model/strategies/texEditors/pixelTransforms/PyBasePixelTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/PyCorrectGamma.cpp
    src/model/strategies/texEditors/pixelTransforms/PyColourReplace.cpp
    src/model/strategies/texEditors/pixelTransforms/PyHighlightShadow.cpp
    src/model/strategies/texEditors/pixelTransforms/PyInvertAlpha.cpp
    src/model/strategies/texEditors/pixelTransforms/PyTempControl.cpp
    src/model/strategies/texEditors/pixelTransforms/PyTintTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/PyTransparency.cpp
    src/model/strategies/texEditors/texFilters/PyBaseTexFilter.cpp
    src/model/strategies/texEditors/texFilters/PyGammaFilter.cpp
    src/model/strategies/texEditors/texFilters/PyTexFilterCommon.cpp
    src/model/strategies/texEditors/texFilters/PyColourReplaceFilter.cpp
    src/model/strategies/texEditors/texFilters/PyTransparencyAdjustFilter.cpp
    src/model/strategies/texEditors/texFilters/PyInvertAlphaFilter.cpp
    src/model/strategies/texEditors/texFilters/PyHueAdjust.cpp
    src/model/strategies/texEditors/texFilters/PyPixelFilter.cpp
    src/model/strategies/texEditors/PyBaseTexEditor.cpp
    src/model/strategies/texEditors/PyTexEditor.cpp
    src/model/strategies/texEditors/PyTexCreator.cpp
    src/model/stats/PyFileStats.cpp
    src/model/stats/PyCachedFileStats.cpp
    src/model/stats/PyRemapStats.cpp
    src/tools/files/PyFileDownload.cpp
    src/model/iniresources/PyIniResourceModel.cpp
    src/model/iniresources/PyIniSrcResourceModel.cpp
    src/model/iniresources/PyIniFixResourceModel.cpp
    src/model/iniresources/PyIniTexModel.cpp
    src/model/iniresources/PyIniDownloadModel.cpp
    src/model/iniresources/PyIniResource.cpp
    src/model/iniresources/PyIniGroupedResource.cpp
    src/model/iniresources/PyRemapIniResource.cpp
    src/model/iniresources/PyRemapIniGroupedResource.cpp
    src/model/iniresources/PyRemapBlendResource.cpp
    src/model/iniresources/PyRemapTexResource.cpp
)

# Forces the linker to use response files (.rsp) to bypass Windows command limits
set_target_properties(core PROPERTIES MSVC_USE_RESPONSE_FILES ON)

target_link_libraries(core PRIVATE AGRemapCore::AGRemapCore)

# -----------------------------------
# Precompiled header
# -----------------------------------

# With LTO off, the critical path of a core-header change is now made up entirely of these 149
#   binding translation units -- and most of what they spend their time on is not this project's
#   code. pybind11/pybind11.h alone is the single largest thing every one of them parses.
#
# Measured on this target, one file at a time, at the flags the dev build actually uses:
#     a small binding (PyInvertAlpha.cpp, 17 lines)  : 5.4s -> 2.7s
#     a large one     (PyGIMIFixer.cpp)              : 10.9s -> 7.9s
#
# As with AGRemapCore's PCH: third-party and standard-library headers only, nothing from
#   core/include, or a single header edit would invalidate the PCH and force all 149 to rebuild.
#
# pybind11/stl.h is in here even though only 79 of the sources include it today, which makes its
#   container type casters visible to the other 70. That is safe *in this codebase specifically*,
#   and it was checked rather than assumed: making stl.h's inclusion inconsistent across
#   translation units is an ODR hazard when a container is declared opaque or bound as a class,
#   and this project does neither -- no PYBIND11_MAKE_OPAQUE, no stl_bind.h/bind_vector/bind_map,
#   and no py::class_ over a std:: or tsl:: type anywhere under py/src. If any of those are
#   introduced later, take stl.h back out of this list.
#
# pybind11/numpy.h is deliberately absent: it is heavy and exactly one source wants it.
if(AGREMAP_PCH AND NOT AGREMAP_SCCACHE)
    target_precompile_headers(core PRIVATE
        <pybind11/pybind11.h>
        <pybind11/stl.h>
        <pybind11/functional.h>
        <string>
        <utility>
        <optional>
        <vector>
        <memory>
        <unordered_map>
        <unordered_set>
        <tuple>
        <functional>
        <cstddef>
        <cstdint>
    )
endif()


set_target_properties(core PROPERTIES PREFIX "")

install(TARGETS core
    LIBRARY DESTINATION FixRaidenBoss2
    RUNTIME DESTINATION FixRaidenBoss2
)

if (WIN32)
    install(FILES
        $<TARGET_FILE:z3::libz3>
        $<TARGET_FILE:utf8proc>
        $<TARGET_FILE:CURL::libcurl>
        DESTINATION FixRaidenBoss2
    )
else()
    # The ELF/Mach-O counterpart of the Windows branch above.
    #
    # On Windows the module locates its sibling DLLs because FixRaidenBoss2/__init__.py calls
    #   os.add_dll_directory() on the package folder. There is no equivalent hook on Linux/macOS,
    #   so without an RPATH the loader never looks beside the module and `import FixRaidenBoss2`
    #   dies with "libz3.so.<ver>: cannot open shared object file". CMake also strips the build-tree
    #   RPATH on install by default ('Set non-toolchain portion of runtime path to ""'), so this
    #   has to be set explicitly rather than inherited.
    #
    # TARGET_SONAME_FILE (not TARGET_FILE) is what gets installed here because the soname is the
    #   name the loader actually requests -- libz3.so.4.17 rather than the unversioned libz3.so
    #   symlink or the fully-versioned libz3.so.4.17.0.0 real file.
    #
    # libcurl is included deliberately: without it the module still imports, but only because it
    #   silently resolves against the *system* /lib/x86_64-linux-gnu/libcurl.so.4 instead of the
    #   vendored copy it was linked against, which defeats vendoring curl at all.
    if (APPLE)
        set(AGREMAP_MODULE_ORIGIN "@loader_path")
    else()
        set(AGREMAP_MODULE_ORIGIN "$ORIGIN")
    endif()

    set_target_properties(core PROPERTIES
        INSTALL_RPATH "${AGREMAP_MODULE_ORIGIN}"
        INSTALL_RPATH_USE_LINK_PATH FALSE
    )

    # Only *shared* deps need copying next to the module; a static one is already linked into
    #   core itself. This has to be decided per-target rather than hardcoded, because these
    #   dependencies do not build as a consistent library type: utf8proc in particular has been
    #   observed building STATIC on one configure of a tree and SHARED on the next (the Windows
    #   install alternates between utf8proc.dll and utf8proc_static.lib for the same reason).
    #   Naming TARGET_SONAME_FILE unconditionally makes the *generate* step fail outright with
    #   "TARGET_SONAME_FILE is allowed only for SHARED libraries" whenever it lands on static.
    set(AGREMAP_RUNTIME_DEPS "")
    foreach(agremapDep z3::libz3 utf8proc CURL::libcurl)
        if (TARGET ${agremapDep})
            get_target_property(agremapDepType ${agremapDep} TYPE)
            if (agremapDepType STREQUAL "SHARED_LIBRARY")
                list(APPEND AGREMAP_RUNTIME_DEPS "$<TARGET_SONAME_FILE:${agremapDep}>")
            else()
                message(STATUS "AGRemap: ${agremapDep} is ${agremapDepType}; not colocating it")
            endif()
        endif()
    endforeach()

    # Install each dependency as a REAL FILE named after its soname -- deliberately not as a
    #   symlink chain.
    #
    # A versioned shared library is normally a chain (libz3.so.4.17 -> libz3.so.4.17.0.0). Two
    #   tempting approaches both fail here:
    #     - install(FILES $<TARGET_SONAME_FILE:...>) copies the *symlink* without its target,
    #       leaving a dangling link whose import error is identical to installing nothing at all.
    #     - file(INSTALL ... FOLLOW_SYMLINK_CHAIN) fixes that, but recreates the symlink in the
    #       package directory. This tree is shared with a Windows checkout, and a symlink written
    #       from WSL lands on the Windows drive as an LX_SYMLINK reparse point that Windows cannot
    #       stat at all -- APIBuilder's own cleanInstalls() then dies walking it with
    #       "OSError: [WinError 1920] The file cannot be accessed by the system", breaking the
    #       *Windows* build as a side effect of having run the Linux one.
    #
    # Dereferencing and renaming sidesteps both: the loader only cares that a file with the soname
    #   exists next to the module, not that it is a link (this is what auditwheel/delvewheel do when
    #   vendoring libraries into wheels). It also installs less -- one real file per dependency
    #   instead of a real file plus a link.
    if (AGREMAP_RUNTIME_DEPS)
        string(JOIN "\" \"" AGREMAP_RUNTIME_DEPS_STR ${AGREMAP_RUNTIME_DEPS})
        install(CODE "
            foreach(agremapLib \"${AGREMAP_RUNTIME_DEPS_STR}\")
                get_filename_component(agremapLibName \"\${agremapLib}\" NAME)
                file(REAL_PATH \"\${agremapLib}\" agremapLibReal)
                # TYPE FILE, not TYPE SHARED_LIBRARY: CMake rejects the combination of RENAME with
                #   the SHARED_LIBRARY type ('file INSTALL option RENAME may be used only with
                #   FILES or PROGRAMS'). Permissions are therefore set explicitly to the 755 a
                #   shared library would normally get.
                file(INSTALL
                    DESTINATION \"\${CMAKE_INSTALL_PREFIX}/FixRaidenBoss2\"
                    TYPE FILE
                    FILES \"\${agremapLibReal}\"
                    RENAME \"\${agremapLibName}\"
                    FILE_PERMISSIONS
                        OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE
                )
            endforeach()
        ")
    endif()
endif()