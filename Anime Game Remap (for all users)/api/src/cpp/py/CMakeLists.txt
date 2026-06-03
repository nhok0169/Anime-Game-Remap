pybind11_add_module(core
    src/bindings.cpp
    src/tools/dfa/PyDFA.cpp
    src/tools/PyBiMap.cpp
    src/tools/PyIntTools.cpp
    src/tools/PyListTools.cpp
)

target_link_libraries(core PRIVATE AGRemapCore::AGRemapCore)

set_target_properties(core PROPERTIES PREFIX "")

install(TARGETS core
    LIBRARY DESTINATION FixRaidenBoss2
    RUNTIME DESTINATION FixRaidenBoss2
)