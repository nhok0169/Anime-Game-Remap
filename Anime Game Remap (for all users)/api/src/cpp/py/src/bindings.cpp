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

#include <pybind11/pybind11.h>

// note: only the init functions are declared here, deliberately, rather than including each binding's
//   Py*.h. bindings.cpp calls nothing else from those headers, and including all of them (429 files
//   once expanded) cost ~45s of pure header parsing for a 3MB object -- and made this file rebuild
//   whenever ANY binding header changed. A new binding adds its one-line declaration here, next to
//   its call below; a mismatched signature is a link error, not a silent problem.
void initCppListTools(pybind11::module_ &m);
void initCppIntTools(pybind11::module_ &m);
void initCppBiMap(pybind11::module_ &m);
void initCppAlgo(pybind11::module_ &m);
void initCppGraphTools(pybind11::module_ &m);
void initCppBaseLogger(pybind11::module_ &m);
void initCppLogger(pybind11::module_ &m);
void initCppRanges(pybind11::module_ &m);
void initCppDFA(pybind11::module_ &m);
void initCppTrie(pybind11::module_ &m);
void initCppAhoCorasickDFA(pybind11::module_ &m);
void initCppOrderedMultiMap(pybind11::module_ &m);
void initCppOrderedMultiMapSqrt(pybind11::module_ &m);
void initCppIOrderedMultiMap(pybind11::module_ &m);
void initCppIfContentPart(pybind11::module_ &m);
void initCppIfContentPartColour(pybind11::module_ &m);
void initCppIniNamingTools(pybind11::module_ &m);
void initCppVersion(pybind11::module_ &m);
void initCppModDictAssets(pybind11::module_ &m);
void initCppModMappedAssets(pybind11::module_ &m);
void initCppModAssets(pybind11::module_ &m);
void initCppHashes(pybind11::module_ &m);
void initCppIndices(pybind11::module_ &m);
void initCppVertexCounts(pybind11::module_ &m);
void initCppIndexCounts(pybind11::module_ &m);
void initCppVGOffsets(pybind11::module_ &m);
void initCppVGCounts(pybind11::module_ &m);
void initCppShapeKeyChecksums(pybind11::module_ &m);
void initCppGameTypeId(pybind11::module_ &m);
void initCppModTypeId(pybind11::module_ &m);
void initCppModTypeIdData(pybind11::module_ &m);
void initCppVGRemap(pybind11::module_ &m);
void initCppVGRemaps(pybind11::module_ &m);
void initCppVGComponentSplit(pybind11::module_ &m);
void initCppVGComponentMerge(pybind11::module_ &m);
void initCppModType(pybind11::module_ &m);
void initCppGlobalModTypes(pybind11::module_ &m);
void initCppGIMICharBuilders(pybind11::module_ &m);
void initCppGIMIComponentBuilders(pybind11::module_ &m);
void initCppWWMIBuilders(pybind11::module_ &m);
void initCppStrategyOverrides(pybind11::module_ &m);
void initCppGIBuilder(pybind11::module_ &m);
void initCppWWMIBuilder(pybind11::module_ &m);
void initCppIniClassifyStats(pybind11::module_ &m);
void initCppBaseIniClassifier(pybind11::module_ &m);
void initCppIniClassifier(pybind11::module_ &m);
void initCppToken(pybind11::module_ &m);
void initCppParseContext(pybind11::module_ &m);
void initCppBaseTokenizer(pybind11::module_ &m);
void initCppFilteredTokenizer(pybind11::module_ &m);
void initCppIfPredTokenizer(pybind11::module_ &m);
void initCppSympyTokenizer(pybind11::module_ &m);
void initCppParseNode(pybind11::module_ &m);
void initCppParseTree(pybind11::module_ &m);
void initCppBaseSLR1Parser(pybind11::module_ &m);
void initCppSympyParser(pybind11::module_ &m);
void initCppIfPredParser(pybind11::module_ &m);
void initCppZ3Context(pybind11::module_ &m);
void initCppZ3Predicate(pybind11::module_ &m);
void initCppIfPredPart(pybind11::module_ &m);
void initCppIfTemplateNode(pybind11::module_ &m);
void initCppIfTemplateTree(pybind11::module_ &m);
void initCppIfTemplate(pybind11::module_ &m);
void initCppCallGraph(pybind11::module_ &m);
void initCppSectionIterData(pybind11::module_ &m);
void initCppIniSectionGraph(pybind11::module_ &m);
void initCppIniGraphGroup(pybind11::module_ &m);
void initCppBaseRegEdit(pybind11::module_ &m);
void initCppRegAdd(pybind11::module_ &m);
void initCppRegAssetRemap(pybind11::module_ &m);
void initCppRegNewVals(pybind11::module_ &m);
void initCppRegRemap(pybind11::module_ &m);
void initCppRegRemove(pybind11::module_ &m);
void initCppRegRestrict(pybind11::module_ &m);
void initCppBaseIniGraphEdit(pybind11::module_ &m);
void initCppGraphRename(pybind11::module_ &m);
void initCppRegFillMissing(pybind11::module_ &m);
void initCppRegSurroundedAdd(pybind11::module_ &m);
void initCppRegBottomAdd(pybind11::module_ &m);
void initCppRegDelimitedAdd(pybind11::module_ &m);
void initCppRegBranchAdd(pybind11::module_ &m);
void initCppGIMIObjPartFilter(pybind11::module_ &m);
void initCppBaseIniGraphGroupEdit(pybind11::module_ &m);
void initCppGraphRemove(pybind11::module_ &m);
void initCppGraphGroupRemove(pybind11::module_ &m);
void initCppGraphInherit(pybind11::module_ &m);
void initCppGraphGroupRemap(pybind11::module_ &m);
void initCppGraphGroupEdit(pybind11::module_ &m);
void initCppResEdit(pybind11::module_ &m);
void initCppRemapBlendReplace(pybind11::module_ &m);
void initCppBufReplace(pybind11::module_ &m);
void initCppTexCreate(pybind11::module_ &m);
void initCppTexReplace(pybind11::module_ &m);
void initCppResRegCollect(pybind11::module_ &m);
void initCppResGroupCollect(pybind11::module_ &m);
void initCppHash64(pybind11::module_ &m);
void initCppHash128(pybind11::module_ &m);
void initCppHashTools(pybind11::module_ &m);
void initCppBufType(pybind11::module_ &m);
void initCppBufDataType(pybind11::module_ &m);
void initCppBufInt(pybind11::module_ &m);
void initCppBufFloat(pybind11::module_ &m);
void initCppBufUnorm(pybind11::module_ &m);
void initCppBufElementType(pybind11::module_ &m);
void initCppBinaryFile(pybind11::module_ &m);
void initCppBufFile(pybind11::module_ &m);
void initCppBlendFile(pybind11::module_ &m);
void initCppPositionFile(pybind11::module_ &m);
void initCppIbFile(pybind11::module_ &m);
void initCppVbFile(pybind11::module_ &m);
void initCppBaseBufEditor(pybind11::module_ &m);
void initCppBufEditor(pybind11::module_ &m);
void initCppColour(pybind11::module_ &m);
void initCppColourRange(pybind11::module_ &m);
void initCppTextureFile(pybind11::module_ &m);
void initCppBasePixelTransform(pybind11::module_ &m);
void initCppCorrectGamma(pybind11::module_ &m);
void initCppColourReplace(pybind11::module_ &m);
void initCppHighlightShadow(pybind11::module_ &m);
void initCppInvertAlpha(pybind11::module_ &m);
void initCppTempControl(pybind11::module_ &m);
void initCppTintTransform(pybind11::module_ &m);
void initCppTransparency(pybind11::module_ &m);
void initCppBaseTexFilter(pybind11::module_ &m);
void initCppGammaFilter(pybind11::module_ &m);
void initCppColourReplaceFilter(pybind11::module_ &m);
void initCppTransparencyAdjustFilter(pybind11::module_ &m);
void initCppInvertAlphaFilter(pybind11::module_ &m);
void initCppHueAdjust(pybind11::module_ &m);
void initCppPixelFilter(pybind11::module_ &m);
void initCppBaseTexEditor(pybind11::module_ &m);
void initCppTexEditor(pybind11::module_ &m);
void initCppTexCreator(pybind11::module_ &m);
void initCppFileStats(pybind11::module_ &m);
void initCppCachedFileStats(pybind11::module_ &m);
void initCppRemapStats(pybind11::module_ &m);
void initCppRemapService(pybind11::module_ &m);
void initCppRemapServiceCLI(pybind11::module_ &m);
void initCppFileDownload(pybind11::module_ &m);
void initCppIniResourceModel(pybind11::module_ &m);
void initCppIniSrcResourceModel(pybind11::module_ &m);
void initCppIniFixResourceModel(pybind11::module_ &m);
void initCppIniTexModel(pybind11::module_ &m);
void initCppIniDownloadModel(pybind11::module_ &m);
void initCppIniResource(pybind11::module_ &m);
void initCppIniFixResource(pybind11::module_ &m);
void initCppIniGroupedResource(pybind11::module_ &m);
void initCppRemapIniResourceMixin(pybind11::module_ &m);
void initCppRemapIniResource(pybind11::module_ &m);
void initCppRemapIniFixResource(pybind11::module_ &m);
void initCppRemapIniGroupedResource(pybind11::module_ &m);
void initCppVGSplitGroupResource(pybind11::module_ &m);
void initCppVGMergeGroupResource(pybind11::module_ &m);
void initCppRemapIniDownload(pybind11::module_ &m);
void initCppRemapBlendResource(pybind11::module_ &m);
void initCppRemapTexAddResource(pybind11::module_ &m);
void initCppRemapTexEditResource(pybind11::module_ &m);
void initCppBaseIniParser(pybind11::module_ &m);
void initCppGIMISectionClassifier(pybind11::module_ &m);
void initCppGIMIParser(pybind11::module_ &m);
void initCppBaseIniFixer(pybind11::module_ &m);
void initCppGIMIFixer(pybind11::module_ &m);
void initCppMultiModFixer(pybind11::module_ &m);
void initCppIniFixingContext(pybind11::module_ &m);
void initCppIniRemovalContext(pybind11::module_ &m);
void initCppBaseIniRemover(pybind11::module_ &m);
void initCppRemapIniRemover(pybind11::module_ &m);
void initCppGlobalRemapIniRemover(pybind11::module_ &m);
void initCppIniFile(pybind11::module_ &m);
void initCppIniParseBuilder(pybind11::module_ &m);
void initCppIniFixBuilder(pybind11::module_ &m);
void initCppIniRemoveBuilder(pybind11::module_ &m);
void initCppModTypeLateBindings(pybind11::module_ &m);

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
    initCppIniNamingTools(m); // no ordering constraints -- every method is static and takes only strings
    initCppVersion(m);
    initCppModDictAssets(m);
    initCppModMappedAssets(m);
    initCppModAssets(m);
    initCppHashes(m);
    initCppIndices(m);
    initCppVertexCounts(m);
    initCppIndexCounts(m); // the four WWMI tables: ModMappedAssets subclasses, so after initCppModMappedAssets like Indices
    initCppVGOffsets(m);
    initCppVGCounts(m);
    initCppShapeKeyChecksums(m);
    initCppGameTypeId(m);
    initCppModTypeId(m);
    initCppModTypeIdData(m);
    // Ahead of ModType, whose getVGRemap returns one: pybind11 bakes a def()'s signature
    // string at registration time, so an unregistered return type renders as a raw C++ name.
    // PyVGRemap.cpp depends on nothing else here, so this is just an ordering choice.
    initCppVGRemap(m);
    initCppVGRemaps(m); // reads best after initCppVGRemap (its get() returns one); not order-critical
    initCppVGComponentSplit(m); // must come after initCppVGRemap (a VGComponentSpec holds one)
    initCppVGComponentMerge(m); // must come after initCppVGRemap (a VGMergeComponentSpec holds one)

    initCppModType(m);
    initCppGlobalModTypes(m); // must come after initCppModType (its all() returns CppModTypes)
    initCppGIMICharBuilders(m); // must come after initCppTexEditor (a TexEdit filter) and before initCppStrategyOverrides (which recognises its factories)
    initCppGIMIComponentBuilders(m); // must come after initCppGIMICharBuilders (shares its PyIniParseFactory/PyIniFixFactory wrappers)
    initCppWWMIBuilders(m); // same wrappers, same ordering; must come after initCppModTypeId and initCppColour (its configs hold a ModTypeId and a Colour)
    initCppStrategyOverrides(m); // takes Python factories; no ordering constraint of its own
    initCppGIBuilder(m); // must come after initCppModType (its methods return ModType) and initCppModTypeId (uses the ModTypeId enum)
    initCppWWMIBuilder(m); // same constraints as initCppGIBuilder
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
    initCppRegAssetRemap(m); // must come after initCppBaseRegEdit (registers its base) and initCppModMappedAssets (the tables it borrows)
    initCppRegNewVals(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegRemap(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegRemove(m); // must come after initCppBaseRegEdit (registers its base)
    initCppRegRestrict(m); // must come after initCppBaseRegEdit (registers its base)

    // ----- iniFixers/graphEdits (full replacement of the pure-Python graphEdits package -- see
    // Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppBaseIniGraphEdit(m); // must come after initCppBaseRegEdit (registers BaseIniGraphPartEdit, its base) and initCppIniSectionGraph (the type it edits)
    initCppGraphRename(m); // must come after initCppBaseIniGraphEdit (registers its base)
    initCppRegFillMissing(m); // must come after initCppBaseIniGraphEdit (registers its base) and initCppIfContentPart (the parts it fills)
    initCppRegSurroundedAdd(m); // must come after initCppBaseIniGraphEdit (registers its base)
    initCppRegBottomAdd(m); // must come after initCppBaseIniGraphEdit (registers its base) and initCppRegSurroundedAdd (shares its parseAdditions/additionsToPy)
    initCppRegDelimitedAdd(m); // must come after initCppBaseIniGraphEdit (registers its base) and initCppRegSurroundedAdd (shares its parsers)
    initCppRegBranchAdd(m); // must come after initCppBaseIniGraphEdit (registers its base), initCppRegSurroundedAdd (shares its parseAdditions), initCppZ3Predicate and initCppSectionIterData (what branchOf is handed)

    // ----- iniFixers/graphGroupEdits (full replacement of the pure-Python graphGroupEdits
    // package -- see Architecture/CLAUDE.md's "Two different outcomes for porting a class") -----
    initCppGIMIObjPartFilter(m); // must come after initCppModMappedAssets (the tables it borrows) and initCppSectionIterData (what its window function takes)
    initCppBaseIniGraphGroupEdit(m); // must come after initCppBaseRegEdit (registers BaseIniPartEdit, its base) and initCppIniSectionGraph/initCppIniGraphGroup (the types it edits)
    initCppGraphRemove(m); // must come after initCppBaseIniGraphGroupEdit (registers its base)
    initCppGraphGroupRemove(m); // must come after initCppBaseIniGraphGroupEdit (registers its base)
    initCppGraphInherit(m); // must come after initCppBaseIniGraphGroupEdit (registers its base) and initCppRanges (its partFilter returns one)
    initCppGraphGroupRemap(m); // must come after initCppBaseIniGraphGroupEdit (registers its base)
    initCppGraphGroupEdit(m); // must come after initCppBaseIniGraphGroupEdit (registers its base) and initCppBaseRegEdit (its isinstance target for register edits)
    initCppResEdit(m); // must come after initCppIniResource/initCppIniFixResource (the models it builds) and initCppIniSectionGraph/initCppIfTemplate
    initCppRemapBlendReplace(m); // must come after initCppResEdit (registers its base) and initCppRemapBlendResource (the model it builds)
    initCppBufReplace(m); // must come after initCppResEdit (registers its base) and initCppRemapIniFixResource (the model it builds)
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
    initCppIbFile(m); // must come after initCppBufFile (registers its base)
    initCppVbFile(m); // must come after initCppBufFile/initCppBufElementType (registers its base / constructor arg type)
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
    initCppRemapService(m); // must come after initCppIniFile/initCppBaseLogger (its members/arguments are those types)
    initCppRemapServiceCLI(m); // must come after initCppRemapService (its constructor takes one) and initCppLogger
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
    initCppVGSplitGroupResource(m); // must come after initCppIniGroupedResource/initCppRemapIniResourceMixin (registers its bases) and initCppVGComponentSplit (its specs)
    initCppVGMergeGroupResource(m); // must come after initCppIniGroupedResource/initCppRemapIniResourceMixin (registers its bases) and initCppVGComponentMerge (its specs)
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
    initCppGlobalRemapIniRemover(m); // must come after initCppRemapIniRemover (registers its base)

    // Registered after ModType, BaseIniClassifier, CppVersion, IfTemplate and IniResource:
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

    // ModType::fixIni takes a IniFile, so it can only be bound now that one exists.
    initCppModTypeLateBindings(m);
}