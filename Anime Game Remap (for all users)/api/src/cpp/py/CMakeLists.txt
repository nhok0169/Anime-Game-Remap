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