pybind11_add_module(core
    src/bindings.cpp
    src/tools/PyAlgo.cpp
    src/tools/PyBiMap.cpp
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
    src/data/HashData.cpp
    src/data/IndexData.cpp
    src/model/strategies/PyModTypeIdData.cpp
    src/model/strategies/iniClassifiers/PyIniClassifyStats.cpp
    src/model/strategies/iniClassifiers/PyBaseIniClassifier.cpp
    src/constants/PyGameTypeId.cpp
    src/constants/PyModTypeId.cpp
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
    src/model/files/PyBufFile.cpp
    src/model/PyVGRemap.cpp
    src/model/files/PyBlendFile.cpp
    src/model/files/PyPositionFile.cpp
)

# Forces the linker to use response files (.rsp) to bypass Windows command limits
set_target_properties(core PROPERTIES MSVC_USE_RESPONSE_FILES ON)

target_link_libraries(core PRIVATE AGRemapCore::AGRemapCore)

set_target_properties(core PROPERTIES PREFIX "")

install(TARGETS core
    LIBRARY DESTINATION FixRaidenBoss2
    RUNTIME DESTINATION FixRaidenBoss2
)

if (WIN32)
    install(FILES
        $<TARGET_FILE:z3::libz3>
        DESTINATION FixRaidenBoss2
    )
endif()