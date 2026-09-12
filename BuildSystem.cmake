
add_library(NLUtils SHARED
    src/Logger.cpp
    src/JSONUtils.cpp
    src/XMLUtils.cpp
    src/Timer.cpp
    src/DebugUtils.cpp
    src/TextUtils.cpp
)


add_subdirectory(libs/pugixml ${CMAKE_CURRENT_BINARY_DIR}/pugixml_build EXCLUDE_FROM_ALL)
add_subdirectory(libs/replxx-0.0.4 ${CMAKE_CURRENT_BINARY_DIR}/replxx_build EXCLUDE_FROM_ALL)
target_include_directories(NLUtils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/pugixml>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/replxx-0.0.4/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/replxx>
)

target_link_libraries(NLUtils PUBLIC
    pugixml
    replxx
)


###########################################################################################


add_library(NLUtils-Extra SHARED
    src/Extra/AssetUtils/DAELoader/Animations.cpp
    src/Extra/AssetUtils/DAELoader/Geometry.cpp
    src/Extra/AssetUtils/DAELoader/Materials.cpp
    src/Extra/AssetUtils/DAELoader/Utils.cpp
    src/Extra/AssetUtils/DAELoader/Base.cpp
    src/Extra/AssetUtils/Base.cpp
    src/Extra/AssetUtils/Mesh.cpp
    src/Extra/AssetUtils/Shader.cpp
    src/Extra/AssetUtils/Texture.cpp
)

add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/glad.c)

add_subdirectory(libs/glm-1.0.3/glm ${CMAKE_CURRENT_BINARY_DIR}/glm_build EXCLUDE_FROM_ALL)

add_library(Nuklear ${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear/Nuklear.c)

target_include_directories(NLUtils-Extra PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/pugixml>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/stb>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/stb>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/nuklear>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glm-1.0.3/glm>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glm>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glad>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glad>
) 

target_link_libraries(NLUtils-Extra PRIVATE
    glm::glm
    NLUtils
    Nuklear
    glad
)

########################################################################################################

add_library(NLUtils-NLGToolkit SHARED
    src/NLGToolkit/NKGLImplementation.cpp
    src/NLGToolkit/NLGToolkit.cpp
)

set(GLFW_BUILD_WAYLAND ON CACHE INTERNAL "")
set(GLFW_BUILD_X11 ON CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_INSTALL OFF CACHE INTERNAL "")

set(CMAKE_SKIP_INSTALL_RULES TRUE)
add_subdirectory(libs/glfw-nor ${CMAKE_CURRENT_BINARY_DIR}/glfw_build EXCLUDE_FROM_ALL)
set(CMAKE_SKIP_INSTALL_RULES FALSE)


target_include_directories(NLUtils-NLGToolkit PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/pugixml>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glad>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glad>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/nuklear>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glm-1.0.3/glm>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glm>

    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw-nor/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glfw>
) 

target_link_libraries(NLUtils-NLGToolkit PRIVATE
    glm::glm
    NLUtils
    NLUtils-Extra
    Nuklear
    glad
    glfw
)

###########################################################################################################

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)


foreach(tgt NLUtils NLUtils-Extra NLUtils-NLGToolkit)
    set_target_properties(${tgt} PROPERTIES
        VERSION   ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH FALSE
        INSTALL_RPATH_USE_LINK_PATH FALSE
    )
endforeach()

install(TARGETS NLUtils NLUtils-Extra NLUtils-NLGToolkit
    EXPORT      NLUtilsTargets
    LIBRARY     DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE     DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME     DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        
)


install(FILES
        libs/pugixml/src/pugixml.hpp
        libs/pugixml/src/pugiconfig.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/pugixml
)

install(DIRECTORY libs/replxx-0.0.4/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/replxx
)

install(DIRECTORY libs/glm-1.0.3/glm/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glm
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h" PATTERN "*.inl"
)

install(DIRECTORY libs/glad/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glad
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY libs/stb/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/stb
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY libs/nuklear/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/nuklear
    FILES_MATCHING PATTERN "*.h" PATTERN "*.c"
)

install(DIRECTORY libs/glfw-nor/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NLUtils/glfw
)


install(EXPORT NLUtilsTargets
    FILE      NLUtilsTargets.cmake
    NAMESPACE NLUtils::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NLUtils
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/NLUtilsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NLUtilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NLUtils
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/NLUtilsConfigVersion.cmake
    VERSION       ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/NLUtilsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/NLUtilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NLUtils
)
