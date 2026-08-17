####
# Svc/Ccsds/KeyMgmtApidRouter/CMakeLists.txt:
#
# SOURCES: list of source files (to be compiled)
# AUTOCODER_INPUTS: list of files to be passed to the autocoders
# DEPENDS: list of libraries that this module depends on
#
# More information in the F´ CMake API documentation:
# https://fprime.jpl.nasa.gov/latest/docs/reference/api/cmake/API/
#
####

add_fprime_subdirectory("${CMAKE_CURRENT_LIST_DIR}/config/KeyMgmtApidRouterConfig")

register_fprime_module(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/KeyMgmtApidRouter.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/KeyMgmtApidRouter.cpp"
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/KeyMgmtApidRouter.hpp"
    DEPENDS
        Svc_Ccsds_KeyMgmtApidRouter_config_KeyMgmtApidRouterConfig
)

### Unit Tests ###
register_fprime_ut(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/KeyMgmtApidRouter.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/KeyMgmtApidRouterTestMain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/test/ut/KeyMgmtApidRouterTester.cpp"
    DEPENDS
        Svc_Ccsds_KeyMgmtApidRouter_config_KeyMgmtApidRouterConfig
        STest
    UT_AUTO_HELPERS
)
