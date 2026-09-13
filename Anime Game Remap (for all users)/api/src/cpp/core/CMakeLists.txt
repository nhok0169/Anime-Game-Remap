cmake_minimum_required(VERSION 3.21)
project(AGRemapCore VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# -----------------------------------
# Options
# -----------------------------------

option(BUILD_CPP_SDK "Install C++ SDK (headers + cmake config)" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# -----------------------------------
# Library
# -----------------------------------

add_library(AGRemapCore STATIC
    src/tools/IntTools.cpp
    src/tools/ListTools.cpp
    src/constants/DownloadMode.cpp
    src/constants/GameTypeId.cpp
    src/constants/ModTypeId.cpp
    src/constants/GameType.cpp
    src/constants/IfPredPartType.cpp
    src/constants/GIBuilder.cpp
    src/constants/GlobalIniRemoveBuilders.cpp
    src/constants/StrategyOverrides.cpp
    src/data/HashData.cpp
    src/data/IndexData.cpp
    src/data/HashToModObjData.cpp
    src/data/VertexCountData.cpp
    src/data/VGRemapData.cpp
    src/data/ModDataAssets.cpp
    src/data/IniParseBuilderData.cpp
    src/data/IniFixBuilderData.cpp
    src/data/IniRemoveBuilderData.cpp
    src/data/IniParseData/Raiden/RaidenParser.cpp
    src/data/IniParseData/Amber/AmberParser.cpp
    src/data/IniFixData/AmberCN/AmberCNFixer.cpp
    src/data/IniFixData/GIMICharFixer.cpp
    src/data/IniFixData/MonaCN/MonaCNFixer.cpp
    src/data/IniFixData/Mona/MonaFixer.cpp
    src/data/IniFixData/RosariaCN/RosariaCNFixer.cpp
    src/data/IniFixData/Rosaria/RosariaFixer.cpp
    src/data/IniParseData/AmberCN/AmberCNParser.cpp
    src/data/IniParseData/GIMICharParser.cpp
    src/data/IniParseData/MonaCN/MonaCNParser.cpp
    src/data/IniParseData/Mona/MonaParser.cpp
    src/data/IniParseData/RosariaCN/RosariaCNParser.cpp
    src/data/IniParseData/Rosaria/RosariaParser.cpp
    src/data/IniFixData/Jean/JeanFixer.cpp
    src/data/IniFixData/JeanCN/JeanCNFixer.cpp
    src/data/IniFixData/DarkDiffuse.cpp
    src/data/IniFixData/RegValChecks.cpp
    src/data/IniParseData/Barbara/BarbaraParser.cpp
    src/data/IniFixData/Barbara/BarbaraFixer.cpp
    src/data/IniParseData/BarbaraSummertime/BarbaraSummertimeParser.cpp
    src/data/IniFixData/BarbaraSummertime/BarbaraSummertimeFixer.cpp
    src/data/IniParseData/Klee/KleeParser.cpp
    src/data/IniFixData/Klee/KleeFixer.cpp
    src/data/IniParseData/KleeBlossomingStarlight/KleeBlossomingStarlightParser.cpp
    src/data/IniFixData/KleeBlossomingStarlight/KleeBlossomingStarlightFixer.cpp
    src/data/IniParseData/Lisa/LisaParser.cpp
    src/data/IniFixData/Lisa/LisaFixer.cpp
    src/data/IniParseData/LisaStudent/LisaStudentParser.cpp
    src/data/IniFixData/LisaStudent/LisaStudentFixer.cpp
    src/data/IniParseData/Diluc/DilucParser.cpp
    src/data/IniFixData/Diluc/DilucFixer.cpp
    src/data/IniParseData/DilucFlamme/DilucFlammeParser.cpp
    src/data/IniFixData/DilucFlamme/DilucFlammeFixer.cpp
    src/data/IniParseData/Fischl/FischlParser.cpp
    src/data/IniFixData/Fischl/FischlFixer.cpp
    src/data/IniParseData/FischlHighness/FischlHighnessParser.cpp
    src/data/IniFixData/FischlHighness/FischlHighnessFixer.cpp
    src/data/IniParseData/Kaeya/KaeyaParser.cpp
    src/data/IniFixData/Kaeya/KaeyaFixer.cpp
    src/data/IniParseData/KaeyaSailwind/KaeyaSailwindParser.cpp
    src/data/IniFixData/KaeyaSailwind/KaeyaSailwindFixer.cpp
    src/data/IniParseData/Arlecchino/ArlecchinoParser.cpp
    src/data/IniFixData/Arlecchino/ArlecchinoFixer.cpp
    src/data/IniFixData/Nilou/NilouFixer.cpp
    src/data/IniFixData/NilouBreeze/NilouBreezeFixer.cpp
    src/data/IniParseData/Nilou/NilouParser.cpp
    src/data/IniParseData/NilouBreeze/NilouBreezeParser.cpp
    src/data/IniFixData/Kirara/KiraraFixer.cpp
    src/data/IniFixData/KiraraBoots/KiraraBootsFixer.cpp
    src/data/IniParseData/Kirara/KiraraParser.cpp
    src/data/IniParseData/KiraraBoots/KiraraBootsParser.cpp
    src/data/IniFixData/Ayaka/AyakaFixer.cpp
    src/data/IniParseData/Ayaka/AyakaParser.cpp
    src/data/IniFixData/AyakaSpringbloom/AyakaSpringbloomFixer.cpp
    src/data/IniParseData/AyakaSpringbloom/AyakaSpringbloomParser.cpp
    src/data/IniFixData/JeanSea/JeanSeaFixer.cpp
    src/data/IniFixData/Ganyu/GanyuFixer.cpp
    src/data/IniFixData/GanyuTwilight/GanyuTwilightFixer.cpp
    src/data/IniParseData/Ganyu/GanyuParser.cpp
    src/data/IniParseData/GanyuTwilight/GanyuTwilightParser.cpp
    src/data/IniFixData/Ningguang/NingguangFixer.cpp
    src/data/IniFixData/NingguangOrchid/NingguangOrchidFixer.cpp
    src/data/IniParseData/Ningguang/NingguangParser.cpp
    src/data/IniParseData/NingguangOrchid/NingguangOrchidParser.cpp
    src/data/IniFixData/JeanShading.cpp
    src/data/IniParseData/JeanSea/JeanSeaParser.cpp
    src/data/IniParseData/Jean/JeanParser.cpp
    src/data/IniParseData/JeanCN/JeanCNParser.cpp
    src/data/IniFixData/Keqing/KeqingFixer.cpp
    src/data/IniFixData/KeqingOpulent/KeqingOpulentFixer.cpp
    src/data/IniParseData/Keqing/KeqingParser.cpp
    src/data/IniParseData/KeqingOpulent/KeqingOpulentParser.cpp
    src/data/IniFixData/GIMIComponentFixer.cpp
    src/data/IniFixData/Yelan/YelanFixer.cpp
    src/data/IniParseData/Yelan/YelanParser.cpp
    src/model/strategies/iniFixers/graphGroupEdits/VGSplitGroupResBuilder.cpp
    src/data/IniFixData/Shenhe/ShenheFixer.cpp
    src/data/IniFixData/ShenheFrostFlower/ShenheFrostFlowerFixer.cpp
    src/data/IniParseData/Shenhe/ShenheParser.cpp
    src/data/IniParseData/ShenheFrostFlower/ShenheFrostFlowerParser.cpp
    src/model/iniresources/RemapPositionResource.cpp
    src/model/iniresources/VGSplitGroupResource.cpp
    src/data/IniFixData/Xiangling/XianglingFixer.cpp
    src/data/IniParseData/Xiangling/XianglingParser.cpp
    src/data/IniFixData/XianglingCheer/XianglingCheerFixer.cpp
    src/data/IniParseData/XianglingCheer/XianglingCheerParser.cpp
    src/data/IniFixData/HuTao/HuTaoFixer.cpp
    src/data/IniParseData/HuTao/HuTaoParser.cpp
    src/data/IniFixData/CherryHuTao/CherryHuTaoFixer.cpp
    src/data/IniParseData/CherryHuTao/CherryHuTaoParser.cpp
    src/data/IniFixData/Xingqiu/XingqiuFixer.cpp
    src/data/IniParseData/Xingqiu/XingqiuParser.cpp
    src/data/IniFixData/XingqiuBamboo/XingqiuBambooFixer.cpp
    src/data/IniParseData/XingqiuBamboo/XingqiuBambooParser.cpp
    src/data/IniFixData/Raiden/RaidenFixer.cpp
    src/data/IniFixData/Amber/AmberFixer.cpp
    src/constants/GlobalIniClassifiers.cpp
    src/constants/GlobalModTypes.cpp
    src/model/Version.cpp
    src/model/VersionSet.cpp
    src/tools/Heading.cpp
    src/view/BaseLogger.cpp
    src/view/Logger.cpp
    src/tools/StringHash.cpp
    src/tools/StringTools.cpp
    src/tools/DownloadTools.cpp
    src/tools/TextTools.cpp
    src/tools/grapheme/GraphemeIterator.cpp
    src/tools/grapheme/GraphemeRange.cpp
    src/tools/parsing/Token.cpp
    src/tools/parsing/ParseContext.cpp
    src/tools/parsing/SyntaxErr.cpp
    src/tools/parsing/BaseTokenizer.cpp
    src/tools/parsing/FilteredTokenizer.cpp
    src/tools/parsing/IfPredTokenizer.cpp
    src/tools/parsing/SympyTokenizer.cpp
    src/tools/hashing/HashInt.cpp
    src/tools/hashing/Hash128.cpp
    src/tools/hashing/Hash64.cpp
    src/tools/hashing/HashTools.cpp
    src/tools/idGenerator/UuidIdGenerator.cpp
    src/model/iftemplate/IfTemplatePart.cpp
    src/model/iftemplate/IfTemplateRender.cpp
    src/model/iftemplate/SympyParser.cpp
    src/model/iftemplate/IfPredParser.cpp
    src/model/iftemplate/IfPredZ3Generator.cpp
    src/model/iftemplate/Z3IfPredGenerator.cpp
    src/model/iftemplate/IfPredPart.cpp
    src/tools/z3/Z3Context.cpp
    src/tools/z3/Z3Predicate.cpp
    src/model/strategies/ModTypeIdData.cpp
    src/model/assets/Hashes.cpp
    src/model/assets/Indices.cpp
    src/model/assets/VertexCounts.cpp
    src/model/assets/VGRemaps.cpp
    src/model/strategies/ModType.cpp
    src/model/strategies/iniClassifiers/IniClassifyStats.cpp
    src/model/strategies/iniClassifiers/BaseIniClassifier.cpp
    src/model/strategies/iniClassifiers/IniClassifier.cpp
    src/model/buffers/BufType.cpp
    src/model/buffers/BufDataType.cpp
    src/model/buffers/BufInt.cpp
    src/model/buffers/BufFloat.cpp
    src/model/buffers/BufUnorm.cpp
    src/model/buffers/BufElementType.cpp
    src/model/buffers/VGComponentSplit.cpp
    src/model/files/BufFileErrors.cpp
    src/model/files/BinaryFile.cpp
    src/model/files/BufFile.cpp
    src/model/VGRemap.cpp
    src/model/files/BlendFile.cpp
    src/model/files/PositionFile.cpp
    src/model/files/IbFile.cpp
    src/model/files/VbFile.cpp
    src/model/strategies/bufEditors/BaseBufEditor.cpp
    src/model/strategies/bufEditors/BufEditor.cpp
    src/model/strategies/iniParsers/IniParseBuilder.cpp
    src/model/strategies/iniFixers/IniFixBuilder.cpp
    src/model/strategies/iniFixers/IniFileFixContext.cpp
    src/model/strategies/iniParsers/IniFileParseContext.cpp
    src/model/strategies/iniFixers/graphGroupEdits/IniFileResEditContext.cpp
    src/model/strategies/iniRemovers/IniFileRemoveContext.cpp
    src/model/strategies/iniRemovers/IniRemoveBuilder.cpp
    src/model/strategies/iniFixers/BaseIniPartEdit.cpp
    src/model/files/IniFile.cpp
    src/model/IniNamingTools.cpp
    src/model/textures/Colour.cpp
    src/model/textures/ColourRange.cpp
    src/model/files/TextureFile.cpp
    src/model/strategies/texEditors/pixelTransforms/BasePixelTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/CorrectGamma.cpp
    src/model/strategies/texEditors/pixelTransforms/ColourReplace.cpp
    src/model/strategies/texEditors/pixelTransforms/HighlightShadow.cpp
    src/model/strategies/texEditors/pixelTransforms/InvertAlpha.cpp
    src/model/strategies/texEditors/pixelTransforms/TempControl.cpp
    src/model/strategies/texEditors/pixelTransforms/TintTransform.cpp
    src/model/strategies/texEditors/pixelTransforms/Transparency.cpp
    src/model/strategies/texEditors/texFilters/BaseTexFilter.cpp
    src/model/strategies/texEditors/texFilters/GammaFilter.cpp
    src/model/strategies/texEditors/texFilters/ColourReplaceFilter.cpp
    src/model/strategies/texEditors/texFilters/TransparencyAdjustFilter.cpp
    src/model/strategies/texEditors/texFilters/InvertAlphaFilter.cpp
    src/model/strategies/texEditors/texFilters/HueAdjust.cpp
    src/model/strategies/texEditors/texFilters/PixelFilter.cpp
    src/model/strategies/texEditors/BaseTexEditor.cpp
    src/model/strategies/texEditors/TexEditor.cpp
    src/model/strategies/texEditors/TexCreator.cpp
    src/tools/files/FileService.cpp
    src/tools/files/FileDownload.cpp
    src/model/stats/FileStats.cpp
    src/model/stats/CachedFileStats.cpp
    src/model/stats/RemapStats.cpp
    src/model/iniresources/IniSrcResourceModel.cpp
    src/model/iniresources/IniFixResourceModel.cpp
    src/model/iniresources/IniTexModel.cpp
    src/model/iniresources/IniDownloadModel.cpp
    src/model/iniresources/IniResource.cpp
    src/model/iniresources/RemapIniResource.cpp
    src/model/iniresources/RemapBlendResource.cpp
    src/model/iniresources/RemapTexResource.cpp
    src/RemapService.cpp
    src/RemapServiceCLI.cpp
    src/RemapServiceCLIErrors.cpp

    ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash/xxhash.c
)

add_library(AGRemapCore::AGRemapCore ALIAS AGRemapCore)

# -----------------------------------
# Build speed
# -----------------------------------

# Unity ("jumbo") build for AGRemapCore: compile the sources in batches, one translation unit per
#   batch, so the ~300 headers under core/include are parsed once per batch instead of once per
#   .cpp. It is wired up and it works -- but it is OFF by default, because on the dev machine it
#   measured as a net LOSS. All four combinations, timed end to end (see the LTO note in
#   py/CMakeLists.txt for the other axis; every row already has VGRemapData.cpp at /Od):
#
#                              change one .cpp     change one common header
#     no unity, no LTO              8.6s                    129.5s
#     no unity, LTO                57.5s                    119.5s
#     unity,    no LTO             25.5s                    123.6s
#     unity,    LTO                75.2s                    111.4s
#
#   Unity buys about 6s on a header change and costs about 17s on a single-file change, which is
#   the far more common edit. The reason is that this machine has 24 hardware threads and the
#   library only has ~150 translation units, so it was never short of parallelism to begin with:
#   batching 16 files into one TU mostly converts work that was running in parallel into work
#   that runs serially, and makes ninja rebuild all 16 when you touch one of them.
#
# That trade can invert, so the switch is left in place rather than deleted: on a machine with
#   few cores, or for a genuine from-scratch build where total CPU work matters more than the
#   critical path, turn it on with -DAGREMAP_UNITY_BUILD=ON.
#
# Two things had to be fixed before a batch would compile at all, and both will bite again:
#
#   1. NOMINMAX below. Something in the batch reaches <windows.h> transitively (via
#      compressonator.h or curl), and its min/max macros then wreck any file compiled after it
#      in the same TU: tsl/ordered_hash.h's `std::min(hi, std::max(lo, v))` fails with
#      "C2589: '(': illegal token on right side of '::'", which reads like a corrupt header
#      rather than a macro collision. ColourRange.cpp's `min(min), max(max)` member-init list is
#      the other obvious casualty. Nothing in core/ wants those macros (verified by grep), so
#      this is applied unconditionally -- it is a strict improvement, and it trims preprocessing
#      whether or not the unity build is on.
#
#   2. AGREMAP_UNITY_EXCLUDE below. Files sharing an anonymous-namespace helper name with another
#      file become redefinition errors once folded together.
#
# If a newly added .cpp breaks a batch, exclude that one file rather than turning this off:
#   set_source_files_properties(<file> PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
option(AGREMAP_UNITY_BUILD "Compile AGRemapCore's sources in unity (jumbo) batches" OFF)

if(WIN32)
    target_compile_definitions(AGRemapCore PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# These 17 each define at least one anonymous-namespace name that another file also defines --
#   BlendHashKey, IbHashKey, ModObj and prototypeRepo in four files each, buildRows in three,
#   and 15 more in two. They keep their own translation unit; everything else batches. Renaming
#   those helpers (internal linkage, so a purely local edit with no ABI or behavioural
#   consequence) would let these fold in too.
#
# Derived by brace-counting every `namespace { ... }` body in core/src at any nesting depth --
#   note that nearly all of them sit inside `namespace AGRemapCore { ... }`, so a scan that only
#   looks for anonymous namespaces at file scope reports zero clashes and is wrong.
set(AGREMAP_UNITY_EXCLUDE
    "src/data/IniFixBuilderData.cpp"
    "src/data/IniParseBuilderData.cpp"
    "src/data/IniRemoveBuilderData.cpp"
    "src/data/IniFixData/GIMICharFixer.cpp"
    "src/data/IniFixData/Raiden/RaidenFixer.cpp"
    "src/data/IniParseData/GIMICharParser.cpp"
    "src/data/IniParseData/Raiden/RaidenParser.cpp"
    "src/model/assets/Hashes.cpp"
    "src/model/assets/Indices.cpp"
    "src/model/assets/VGRemaps.cpp"
    "src/model/assets/VertexCounts.cpp"
    "src/model/buffers/BufFloat.cpp"
    "src/model/buffers/BufUnorm.cpp"
    "src/model/iftemplate/IfPredParser.cpp"
    "src/model/iftemplate/SympyParser.cpp"
    "src/tools/parsing/IfPredTokenizer.cpp"
    "src/tools/parsing/SympyTokenizer.cpp"
)

# Vendored third-party C, not ours to reason about -- keep it out of the batches too.
set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash/xxhash.c
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
)

if(AGREMAP_UNITY_BUILD)
    set_target_properties(AGRemapCore PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_MODE BATCH
        UNITY_BUILD_BATCH_SIZE 16
    )

    set_source_files_properties(
        ${AGREMAP_UNITY_EXCLUDE}
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )
endif()

# VGRemapData.cpp is a single function holding one static initializer: 52 VGRemap rows built out
#   of 5,229 nested {a, b} pair initializers. MSVC's optimizer is superlinear in the size of that
#   one expression -- measured 38.6s at /O2 and 37.7s at /O1 against 5.5s at /Od standalone, and
#   ~80s inside the parallel build, where it single-handedly gated AGRemapCore.lib and so sat on
#   the critical path of every core header change (dropping it took that cycle from 137.9s to
#   119.5s). There is nothing worth optimizing here: the file builds a lookup table once at
#   startup and is never hot. /O1 is not enough, it needs /Od. HashData.cpp has a similar shape
#   but is NOT affected (9.4s vs 9.0s), so this is scoped to the one file rather than to data/.
#
# SKIP_UNITY_BUILD_INCLUSION is required, not optional, whenever the unity build is on: a file's
#   per-source COMPILE_OPTIONS do not survive being folded into a batch, so without it the /Od
#   below would silently do nothing and this file would be back to costing 80s inside someone
#   else's batch.
set_source_files_properties(src/data/VGRemapData.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

if(MSVC)
    # This appends, giving "/O2 /Ob2 ... /Od" -- CMake can only add to a source's flags, never
    #   remove an inherited one, and MSVC takes the last /O switch. Verified on MSVC 14.50 that
    #   the flag really lands and really takes effect (VGRemapData.cpp.obj went from 80.4s to
    #   14.2s in the parallel build); this toolset emits no D9025 override warning for the pair.
    set_source_files_properties(src/data/VGRemapData.cpp PROPERTIES COMPILE_OPTIONS "/Od")
else()
    set_source_files_properties(src/data/VGRemapData.cpp PROPERTIES COMPILE_OPTIONS "-O0")
endif()

# Precompiled header. Deliberately third-party and standard-library headers ONLY: nothing from
#   core/include goes in here. A PCH is rebuilt, and every consumer of it recompiled, whenever
#   any header inside it changes -- so putting this library's own headers in would turn a
#   one-header edit into a guaranteed full-target rebuild, which is precisely the case this is
#   meant to make cheaper.
#
# The list is the top of `grep -rho "#include <...>" src include | sort | uniq -c | sort -rn`,
#   cut where the counts stop being interesting. <filesystem> and <variant> are next but are
#   heavy relative to how many files want them.
option(AGREMAP_PCH "Precompiled header for the pybind11 module" ON)
option(AGREMAP_PCH_LIB "Precompiled header for AGRemapCore itself" ON)

if(AGREMAP_PCH_LIB AND NOT AGREMAP_SCCACHE)
    target_precompile_headers(AGRemapCore PRIVATE
        <string>
        <string_view>
        <utility>
        <vector>
        <optional>
        <memory>
        <functional>
        <unordered_map>
        <unordered_set>
        <set>
        <algorithm>
        <stdexcept>
        <cstddef>
        <cstdint>
    )

    # xxhash.c is C; a C++ PCH cannot apply to it, and CMake would otherwise try.
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash/xxhash.c
        PROPERTIES SKIP_PRECOMPILE_HEADERS ON
    )

    # VGRemapData.cpp compiles at /Od (above) while the PCH is built at /O2. MSVC will not reject
    #   that outright -- it just stops being able to use the PCH, and the file pays for the whole
    #   force-included header set on its own. Measured: 14.2s with the file excluded here against
    #   144.2s with it included, i.e. worse than it was before /Od was ever applied, and enough
    #   on its own to make the PCH look like a regression (a header change went 132.6s -> 150.5s
    #   until this line was added). Any future source given per-file COMPILE_OPTIONS needs the
    #   same treatment.
    set_source_files_properties(src/data/VGRemapData.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()



# -----------------------------------
# UTF8Proc
# -----------------------------------

set(UTF8PROC_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8PROC_INSTALL OFF CACHE BOOL "" FORCE)

set(UTF8PROC_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../extern/utf8proc)
set(UTF8PROC_BIN ${CMAKE_BINARY_DIR}/utf8proc)

add_subdirectory(${UTF8PROC_SRC} ${UTF8PROC_BIN})

# -----------------------------------
# Compressonator
# -----------------------------------

set(COMPRESSONATOR_SRC
    ${CMAKE_CURRENT_LIST_DIR}/../../../extern/Compressonator
)

# Compressonator's child CMakeLists expect these platform variables.
if(WIN32)
    set(CMP_HOST_WINDOWS ON)
elseif(UNIX)
    set(CMP_HOST_LINUX ON)
endif()

# Disable optional components that would pull in additional dependencies.
set(OPTION_BUILD_ASTC OFF)
set(OPTION_BUILD_BROTLIG OFF)
set(OPTION_CMP_GTC OFF)
set(OPTION_CMP_APC OFF)

message(STATUS "BEFORE Compressonator:")
message(STATUS "  PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
message(STATUS "  CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")

# Compressonator's child CMakeLists use PROJECT_SOURCE_DIR
# assuming it points to the Compressonator source root.
set(AGREMAP_PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_SOURCE_DIR ${COMPRESSONATOR_SRC})

if(MSVC)
    add_compile_options(/Zc:strictStrings-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Compressonator's applications/_plugins/common headers (cmp_fileio.h, utilfuncs.h) refer to
    #   uintmax_t/uint16_t without including <cstdint>. MSVC pulls those in transitively, GCC 13+
    #   does not, so on Linux they fail with "'uintmax_t' does not name a type" / "'uint16_t' was
    #   not declared in this scope" -- plus a cascading, misleading "redefinition of 'float
    #   HalfToFloat'" that is really just the same unparsed declaration seen twice.
    # Force-including the header fixes it without patching the vendored submodule (whose changes
    #   this repo can't commit anyway). stdint.h rather than <cstdint> so it stays valid for
    #   Compressonator's C sources too, and because the code uses these names unqualified.
    add_compile_options(-include stdint.h)
endif()

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_core
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_core
    EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_framework
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_framework
    EXCLUDE_FROM_ALL
)

add_subdirectory(
    ${COMPRESSONATOR_SRC}/cmp_compressonatorlib
    ${CMAKE_BINARY_DIR}/Compressonator/cmp_compressonatorlib
    EXCLUDE_FROM_ALL
)

# Restore our project's PROJECT_SOURCE_DIR.
set(PROJECT_SOURCE_DIR ${AGREMAP_PROJECT_SOURCE_DIR})


# -----------------------------------
# libcurl
# -----------------------------------

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_MISC_DOCS OFF CACHE BOOL "" FORCE)

set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
endif()

set(CURL_SRC ${CMAKE_CURRENT_LIST_DIR}/../../../extern/curl)
set(CURL_BIN ${CMAKE_BINARY_DIR}/curl)

add_subdirectory(
    ${CURL_SRC}
    ${CURL_BIN}
    EXCLUDE_FROM_ALL
)

# -----------------------------------
# Link Libraries
# -----------------------------------

target_include_directories(AGRemapCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # BaseOrderedMultiMap.h (a public AGRemapCore header) includes tsl/ordered_map.h and
        # tsl/ordered_set.h directly, so any consumer building against AGRemapCore -- not just
        # AGRemapCore's own .cpp files -- needs this on its include path too. PUBLIC (not
        # PRIVATE) so it actually propagates; BUILD_INTERFACE-only since this vendored path
        # isn't itself installed as part of the SDK.
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../../../extern/ordered-map/include>
        # TextureFile.h (a public AGRemapCore header) includes compressonator.h directly (it's a
        # lightweight, plain-C-enum-and-struct header, unlike z3++.h -- see Z3Internal.h's own
        # comment for why *that* one stays PRIVATE behind a pimpl instead), so any consumer
        # building against AGRemapCore needs this on its include path too. Same PUBLIC +
        # BUILD_INTERFACE-only treatment as ordered-map's include dir above, for the same reason.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../extern/Compressonator/cmp_compressonatorlib>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>

    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/../../../extern/xxHash
        # Lets any AGRemapCore .cpp (eg. IfPredZ3Generator.cpp) reach the private,
        # never-installed "tools/z3/Z3Internal.h" (the one place z3::context::Impl/
        # z3::expr::Impl are actually defined in terms of z3++.h) via a
        # public-header-shaped path, without that path living under include/ where it'd
        # ship as part of the public SDK.
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AGRemapCore
    PRIVATE
        z3::libz3
        utf8proc
        CMP_Compressonator
        CMP_Framework
        CURL::libcurl
)

set_target_properties(AGRemapCore PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# -----------------------------------
# Export + Config setup (required for find_package)
# -----------------------------------

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/AGRemapCore)

# Generate AGRemapCoreConfig.cmake
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AGRemapCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

# Optional version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# -----------------------------------
# INSTALL (SDK only)
# -----------------------------------

if(BUILD_CPP_SDK)

    # Library
    install(TARGETS AGRemapCore
        EXPORT AGRemapCoreTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Headers
    install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Export targets
    install(EXPORT AGRemapCoreTargets
        FILE AGRemapCoreTargets.cmake
        NAMESPACE AGRemapCore::
        DESTINATION ${CONFIG_INSTALL_DIR}
    )

    # Config + version files
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/AGRemapCoreConfigVersion.cmake
        DESTINATION ${CONFIG_INSTALL_DIR}
    )

endif()