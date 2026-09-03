#include <pybind11/pybind11.h>

#include <pybind11/pybind11.h>

#include "tools/PyListTools.h"
#include "tools/PyIntTools.h"
#include "tools/dfa/PyDFA.h"
#include "tools/PyBiMap.h"
#include "tools/PyAlgo.h"
#include "tools/PyGraphTools.h"
#include "view/PyBaseLogger.h"
#include "view/PyLogger.h"
#include "tools/PyRanges.h"
#include "tools/tries/PyTrie.h"
#include "tools/tries/PyAhoCorasickDFA.h"
#include "tools/orderedMultiMap/PyOrderedMultiMap.h"
#include "tools/orderedMultiMap/PyOrderedMultiMapSqrt.h"
#include "tools/orderedMultiMap/PyIOrderedMultiMap.h"
#include "model/iftemplate/PyIfContentPart.h"
#include "model/iftemplate/PyIfContentPartColour.h"
#include "model/PyVersion.h"
#include "model/assets/PyModDictAssets.h"
#include "model/assets/PyModMappedAssets.h"
#include "model/assets/PyModAssets.h"
#include "model/assets/PyHashes.h"
#include "model/assets/PyIndices.h"
#include "constants/PyGameTypeId.h"
#include "constants/PyModTypeId.h"
#include "constants/PyGlobalModTypes.h"
#include "constants/PyGIBuilder.h"
#include "model/strategies/PyModTypeIdData.h"
#include "model/strategies/PyModType.h"
#include "model/strategies/iniClassifiers/PyIniClassifyStats.h"
#include "model/strategies/iniClassifiers/PyBaseIniClassifier.h"
#include "model/strategies/iniClassifiers/PyIniClassifier.h"
#include "tools/parsing/PyToken.h"
#include "tools/parsing/PyParseContext.h"
#include "tools/parsing/PyBaseTokenizer.h"
#include "tools/parsing/PyFilteredTokenizer.h"
#include "tools/parsing/PyIfPredTokenizer.h"
#include "tools/parsing/PySympyTokenizer.h"
#include "tools/nodes/PyParseNode.h"
#include "tools/parsing/PyParseTree.h"
#include "tools/parsing/PyBaseSLR1Parser.h"
#include "model/iftemplate/PySympyParser.h"
#include "model/iftemplate/PyIfPredParser.h"
#include "tools/z3/PyZ3Context.h"
#include "tools/z3/PyZ3Predicate.h"
#include "model/iftemplate/PyIfPredPart.h"
#include "model/iftemplate/PyIfTemplateNode.h"
#include "model/iftemplate/PyIfTemplateTree.h"
#include "model/iftemplate/PyIfTemplate.h"
#include "model/PyCallGraph.h"
#include "model/PySectionIterData.h"
#include "model/PyIniSectionGraph.h"
#include "model/PyIniGraphGroup.h"
#include "model/strategies/iniFixers/regEdits/PyBaseRegEdit.h"
#include "model/strategies/iniFixers/regEdits/PyRegAdd.h"
#include "model/strategies/iniFixers/regEdits/PyRegNewVals.h"
#include "model/strategies/iniFixers/regEdits/PyRegRemap.h"
#include "model/strategies/iniFixers/regEdits/PyRegRemove.h"
#include "model/strategies/iniFixers/graphEdits/PyBaseIniGraphEdit.h"
#include "model/strategies/iniFixers/graphEdits/PyGraphRename.h"
#include "model/strategies/iniFixers/graphEdits/PyRegFillMissing.h"
#include "model/strategies/iniFixers/graphEdits/PyRegSurroundedAdd.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyBaseIniGraphGroupEdit.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyGraphRemove.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyGraphInherit.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyGraphGroupRemap.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyGraphGroupEdit.h"
#include "model/strategies/iniFixers/graphGroupEdits/resEdits/PyResEdit.h"
#include "model/strategies/iniFixers/graphGroupEdits/resEdits/PyBlendEdit.h"
#include "model/strategies/iniFixers/graphGroupEdits/resEdits/PyTexEdit.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyResRegCollect.h"
#include "model/strategies/iniFixers/graphGroupEdits/PyResGroupCollect.h"
#include "model/strategies/iniParsers/PyBaseIniParser.h"
#include "model/strategies/iniParsers/PyGIMISectionClassifier.h"
#include "model/strategies/iniParsers/PyGIMIParser.h"
#include "model/strategies/iniParsers/PyIniParseBuilder.h"
#include "model/strategies/iniFixers/PyBaseIniFixer.h"
#include "model/strategies/iniFixers/PyGIMIFixer.h"
#include "model/strategies/iniFixers/PyMultiModFixer.h"
#include "model/strategies/iniFixers/PyIniFixBuilder.h"
#include "model/strategies/iniRemovers/PyBaseIniRemover.h"
#include "model/strategies/iniFixers/PyIniFixingContext.h"
#include "model/strategies/iniRemovers/PyIniRemovalContext.h"
#include "model/strategies/iniRemovers/PyRemapIniRemover.h"
#include "model/strategies/iniRemovers/PyIniRemoveBuilder.h"
#include "tools/hashing/PyHash64.h"
#include "tools/hashing/PyHash128.h"
#include "tools/hashing/PyHashTools.h"
#include "model/buffers/PyBufType.h"
#include "model/buffers/PyBufDataType.h"
#include "model/buffers/PyBufInt.h"
#include "model/buffers/PyBufFloat.h"
#include "model/buffers/PyBufUnorm.h"
#include "model/buffers/PyBufElementType.h"
#include "model/files/PyBinaryFile.h"
#include "model/files/PyIniFile.h"
#include "model/files/PyBufFile.h"
#include "model/PyVGRemap.h"
#include "model/files/PyBlendFile.h"
#include "model/files/PyPositionFile.h"
#include "model/strategies/bufEditors/PyBaseBufEditor.h"
#include "model/strategies/bufEditors/PyBufEditor.h"
#include "model/textures/PyColour.h"
#include "model/textures/PyColourRange.h"
#include "model/files/PyTextureFile.h"
#include "model/strategies/texEditors/pixelTransforms/PyBasePixelTransform.h"
#include "model/strategies/texEditors/pixelTransforms/PyCorrectGamma.h"
#include "model/strategies/texEditors/pixelTransforms/PyColourReplace.h"
#include "model/strategies/texEditors/pixelTransforms/PyHighlightShadow.h"
#include "model/strategies/texEditors/pixelTransforms/PyInvertAlpha.h"
#include "model/strategies/texEditors/pixelTransforms/PyTempControl.h"
#include "model/strategies/texEditors/pixelTransforms/PyTintTransform.h"
#include "model/strategies/texEditors/pixelTransforms/PyTransparency.h"
#include "model/strategies/texEditors/texFilters/PyBaseTexFilter.h"
#include "model/strategies/texEditors/texFilters/PyGammaFilter.h"
#include "model/strategies/texEditors/texFilters/PyColourReplaceFilter.h"
#include "model/strategies/texEditors/texFilters/PyTransparencyAdjustFilter.h"
#include "model/strategies/texEditors/texFilters/PyInvertAlphaFilter.h"
#include "model/strategies/texEditors/texFilters/PyHueAdjust.h"
#include "model/strategies/texEditors/texFilters/PyPixelFilter.h"
#include "model/strategies/texEditors/PyBaseTexEditor.h"
#include "model/strategies/texEditors/PyTexEditor.h"
#include "model/strategies/texEditors/PyTexCreator.h"
#include "model/stats/PyFileStats.h"
#include "model/stats/PyCachedFileStats.h"
#include "model/stats/PyRemapStats.h"
#include "tools/files/PyFileDownload.h"
#include "model/iniresources/PyIniResourceModel.h"
#include "model/iniresources/PyIniSrcResourceModel.h"
#include "model/iniresources/PyIniFixResourceModel.h"
#include "model/iniresources/PyIniTexModel.h"
#include "model/iniresources/PyIniDownloadModel.h"
#include "model/iniresources/PyIniResource.h"
#include "model/iniresources/PyIniGroupedResource.h"
#include "model/iniresources/PyRemapIniResource.h"
#include "model/iniresources/PyRemapIniGroupedResource.h"
#include "model/iniresources/PyRemapBlendResource.h"
#include "model/iniresources/PyRemapTexResource.h"

namespace py = pybind11;


PYBIND11_MODULE(core, m) {
    py::options options;
    options.enable_user_defined_docstrings();

    m.doc() = "C++ internal core of AGRemap";

    initCppListTools(m);
    initCppIntTools(m);
    initCppBiMap(m);
    initCppAlgo(m);
    initCppGraphTools(m);

    // ----- view (the MVC view -- full replacement of the pure-Python view/Logger.py; no
    //       dependency on any other binding, BaseLogger's heading stack is plain tuples) -----
    initCppBaseLogger(m);
    initCppLogger(m); // must come after initCppBaseLogger (registers its base)
    initCppRanges(m);
    initCppDFA(m);
    initCppTrie(m);
    initCppAhoCorasickDFA(m);
    initCppOrderedMultiMap(m);
    initCppOrderedMultiMapSqrt(m);
    initCppIOrderedMultiMap(m);
    initCppIfContentPart(m);
    initCppIfContentPartColour(m);
    initCppVersion(m);
    initCppModDictAssets(m);
    initCppModMappedAssets(m);
    initCppModAssets(m);
    initCppHashes(m);
    initCppIndices(m);
    initCppGameTypeId(m);
    initCppModTypeId(m);
    initCppModTypeIdData(m);
    // Ahead of CppModType, whose getVGRemap returns one: pybind11 bakes a def()'s signature
    // string at registration time, so an unregistered return type renders as a raw C++ name.
    // PyVGRemap.cpp depends on nothing else here, so this is just an ordering choice.
    initCppVGRemap(m);

    initCppModType(m);
    initCppGlobalModTypes(m); // must come after initCppModType (its all() returns CppModTypes)
    initCppGIBuilder(m); // must come after initCppModType (its methods return CppModType) and initCppModTypeId (uses the ModTypeId enum)
    initCppIniClassifyStats(m);
    initCppBaseIniClassifier(m);
    initCppIniClassifier(m); // must come after initCppBaseIniClassifier (registers its base)
    initCppToken(m);
    initCppParseContext(m);
    initCppBaseTokenizer(m);
    initCppFilteredTokenizer(m);
    initCppIfPredTokenizer(m);
    initCppSympyTokenizer(m);
    initCppParseNode(m);
    initCppParseTree(m);
    initCppBaseSLR1Parser(m);
    initCppSympyParser(m);
    initCppIfPredParser(m);
    initCppZ3Context(m);
    initCppZ3Predicate(m);
    initCppIfPredPart(m); // must come after initCppIfContentPart (registers its base, IfTemplatePart) and initCppZ3Context/initCppZ3Predicate
    initCppIfTemplateNode(m); // reuses PyIfContentPart/AGRC::IfPredPart in its own method signatures, so registered after both
    initCppIfTemplateTree(m); // its 'root' property returns IfTemplateNode, so registered after initCppIfTemplateNode
    initCppIfTemplate(m); // its 'tree' property returns IfTemplateTree, so registered after initCppIfTemplateTree
    initCppCallGraph(m);
    initCppSectionIterData(m);
    initCppIniSectionGraph(m);
    initCppIniGraphGroup(m); // no ordering dependency -- holds py::object graph values generically, no pybind base of its own

    // ----- iniFixers/regEdits (full replacement of the pure-Python regEdits package -- see
    // Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppBaseRegEdit(m); // registers BaseIniPartEdit/BaseIniGraphPartEdit/BaseRegEdit; must come after initCppIfContentPart (its edit signatures take one) and initCppRanges (partRanges)
    initCppRegAdd(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegNewVals(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegRemap(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegRemove(m); // must come after initCppBaseRegEdit (registers its base)

    // ----- iniFixers/graphEdits (full replacement of the pure-Python graphEdits package -- see
    // Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppBaseIniGraphEdit(m); // must come after initCppBaseRegEdit (registers BaseIniGraphPartEdit, its base) and initCppIniSectionGraph (the type it edits)
    initCppGraphRename(m); // must come after initCppBaseIniGraphEdit (registers its base)
    initCppRegFillMissing(m); // must come after initCppBaseIniGraphEdit (registers its base) and initCppIfContentPart (the parts it fills)
    initCppRegSurroundedAdd(m); // must come after initCppBaseIniGraphEdit (registers its base)

    // ----- iniFixers/graphGroupEdits (full replacement of the pure-Python graphGroupEdits
    // package -- see Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppBaseIniGraphGroupEdit(m); // must come after initCppBaseRegEdit (registers BaseIniPartEdit, its base) and initCppIniSectionGraph/initCppIniGraphGroup (the types it edits)
    initCppGraphRemove(m); // must come after initCppBaseIniGraphGroupEdit (registers its base)
    initCppGraphInherit(m); // must come after initCppBaseIniGraphGroupEdit (registers its base) and initCppRanges (its partFilter returns one)
    initCppGraphGroupRemap(m); // must come after initCppBaseIniGraphGroupEdit (registers its base)
    initCppGraphGroupEdit(m); // must come after initCppBaseIniGraphGroupEdit (registers its base) and initCppBaseRegEdit (its isinstance target for register edits)
    initCppResEdit(m); // must come after initCppIniResource/initCppIniFixResource (the models it builds) and initCppIniSectionGraph/initCppIfTemplate
    initCppRemapBlendReplace(m); // must come after initCppResEdit (registers its base) and initCppRemapBlendResource (the model it builds)
    initCppTexCreate(m); // must come after initCppResEdit (registers its base) and initCppRemapTexAddResource/initCppTexCreator (the model it builds)
    initCppTexReplace(m); // same ordering needs as initCppTexCreate above
    initCppResRegCollect(m); // must come after initCppBaseIniGraphGroupEdit (registers its base) and initCppResEdit (its resEdits values)
    initCppResGroupCollect(m); // must come after initCppBaseIniGraphGroupEdit (registers its base), initCppResEdit and initCppIniGroupedResource (the groups it builds)
    initCppHash64(m);
    initCppHash128(m);
    initCppHashTools(m);
    initCppBufType(m);
    initCppBufDataType(m); // must come after initCppBufType (registers its base)
    initCppBufInt(m); // must come after initCppBufDataType (registers its base)
    initCppBufFloat(m); // must come after initCppBufDataType (registers its base)
    initCppBufUnorm(m); // must come after initCppBufInt (registers its base, CppBufBaseInt)
    initCppBufElementType(m); // must come after initCppBufType and initCppBufDataType (constructor takes CppBufDataType instances)
    initCppBinaryFile(m);
    initCppBufFile(m); // must come after initCppBinaryFile/initCppBufElementType (registers its base / constructor arg type)
    initCppBlendFile(m); // must come after initCppBufFile/initCppVGRemap
    initCppPositionFile(m); // must come after initCppBufFile
    initCppBaseBufEditor(m); // must come after initCppBufFile (its 'fix' method signature references it)
    initCppBufEditor(m); // must come after initCppBaseBufEditor (registers its base)
    initCppColour(m);
    initCppColourRange(m); // must come after initCppColour (constructor arg type)
    initCppTextureFile(m);
    initCppBasePixelTransform(m);
    initCppCorrectGamma(m); // must come after initCppBasePixelTransform (registers its base)
    initCppColourReplace(m); // must come after initCppBasePixelTransform/initCppColourRange
    initCppHighlightShadow(m); // must come after initCppBasePixelTransform (registers its base)
    initCppInvertAlpha(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTempControl(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTintTransform(m); // must come after initCppBasePixelTransform (registers its base)
    initCppTransparency(m); // must come after initCppBasePixelTransform (registers its base)
    initCppBaseTexFilter(m); // must come after initCppTextureFile (its 'transform' method signature references it)
    initCppGammaFilter(m); // must come after initCppBaseTexFilter (registers its base)
    initCppColourReplaceFilter(m); // must come after initCppBaseTexFilter/initCppColourRange
    initCppTransparencyAdjustFilter(m); // must come after initCppBaseTexFilter/initCppColourRange
    initCppInvertAlphaFilter(m); // must come after initCppBaseTexFilter (registers its base)
    initCppHueAdjust(m); // must come after initCppBaseTexFilter (registers its base)
    initCppPixelFilter(m); // must come after initCppBaseTexFilter/initCppBasePixelTransform
    initCppBaseTexEditor(m); // must come after initCppTextureFile (its 'fix' method signature references it)
    initCppTexEditor(m); // must come after initCppBaseTexEditor (registers its base)
    initCppTexCreator(m); // must come after initCppBaseTexEditor (registers its base) and initCppColour (constructor arg type)

    // ----- iniresources / stats / FileDownload (Phase 1 of the Cpp-prefix-then-full-replacement
    // playbook -- see Architecture/CLAUDE.md's "Two different outcomes for porting a class"; every
    // class below already exists as a live pure-Python class of the same bare name today) -----
    initCppFileStats(m);
    initCppCachedFileStats(m); // must come after initCppFileStats (registers its base)
    initCppRemapStats(m); // must come after initCppFileStats/initCppCachedFileStats (its members are those types)
    initCppFileDownload(m);
    initCppIniResourceModel(m);
    initCppIniSrcResourceModel(m); // must come after initCppIniResourceModel (registers its base)
    initCppIniFixResourceModel(m); // must come after initCppIniResourceModel (registers its base)
    initCppIniTexModel(m); // must come after initCppIniFixResourceModel (registers its base) and initCppBaseTexEditor (constructor takes ownership of CppBaseTexEditor instances)
    initCppIniDownloadModel(m); // must come after initCppIniSrcResourceModel (registers its base) and initCppFileDownload (constructor takes ownership of FileDownload instances)
    initCppIniResource(m);
    initCppIniFixResource(m); // must come after initCppIniResource (registers its base)
    initCppIniGroupedResource(m); // binds PyIniGroupedResource (base: plain AGRC::IniGroupedResource, not separately registered) -- no ordering dependency on initCppIniResource
    initCppRemapIniResourceMixin(m);
    initCppRemapIniResource(m); // must come after initCppIniResource/initCppRemapIniResourceMixin (registers its bases)
    initCppRemapIniFixResource(m); // must come after initCppIniFixResource/initCppRemapIniResourceMixin (registers its bases)
    initCppRemapIniGroupedResource(m); // must come after initCppIniGroupedResource/initCppRemapIniResourceMixin (registers PyIniGroupedResource/RemapIniResourceMixin, its real bases)
    initCppRemapIniDownload(m); // must come after initCppRemapIniResource (registers its base) and initCppFileDownload (constructor takes ownership of a FileDownload instance)
    initCppRemapBlendResource(m); // must come after initCppRemapIniFixResource (registers its base); VGRemap/BufElementType already registered above
    initCppRemapTexAddResource(m); // must come after initCppRemapIniResource (registers its base); CppTexCreator already registered above
    initCppRemapTexEditResource(m); // must come after initCppRemapIniResource (registers its base); CppTexEditor likewise

    // ----- iniParsers (full replacement of the pure-Python BaseIniParser/GIMIParser pair --
    // see Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppBaseIniParser(m);
    initCppGIMISectionClassifier(m); // must come after initCppHashes/initCppIndices (its assets) and initCppIfContentPartColour (what it classifies from)
    initCppGIMIParser(m); // must come after initCppBaseIniParser (registers its base), initCppIniGraphGroup/initCppIniSectionGraph (the graphs it builds) and initCppRemapIniDownload (the downloads it records)

    // ----- iniFixers (full replacement of the pure-Python BaseIniFixer/GIMIFixer pair) -----
    initCppBaseIniFixer(m);
    initCppGIMIFixer(m); // must come after initCppBaseIniFixer (registers its base), initCppGIMIParser (what it fixes from) and initCppResEdit (its pyCoreModule() is how the package's own constants are reached)
    initCppMultiModFixer(m); // must come after initCppBaseIniFixer (registers its base)

    // ----- iniRemovers (the C++ RemapIniRemover reached through an IniRemoveContext -- see that
    //       interface's own note on why a remover can't just take an AGRemapCore::IniFile*) -----
    initCppIniFixingContext(m);
    initCppIniRemovalContext(m);
    initCppBaseIniRemover(m);
    initCppRemapIniRemover(m); // must come after initCppBaseIniRemover (registers its base), initCppIfTemplate (the sections it reads) and initCppIniResource (the resources it collects)

    // Registered after CppModType, BaseIniClassifier, CppVersion, IfTemplate and IniResource:
    // every one of them appears in one of this class's own def() signatures, and pybind11 bakes
    // those strings at def() time -- see PyIfContentPartColour.cpp's note on what an unregistered
    // type there does to the signature (and to core.pyi).
    initCppIniFile(m);

    // These three must come after BOTH their strategy base (their factory returns one) and
    // initCppIniFile: build() takes a IniFile, and pybind11 bakes a def()'s signature
    // string at registration time -- registering earlier renders it as a raw C++ name and
    // corrupts core.pyi.
    initCppIniParseBuilder(m);
    initCppIniFixBuilder(m);
    initCppIniRemoveBuilder(m);

    // CppModType::fixIni takes a IniFile, so it can only be bound now that one exists.
    initCppModTypeLateBindings(m);
}