cmake_minimum_required(VERSION 3.30)
project(NLUtils VERSION 1.2.3 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

include(CheckIPOSupported)
check_ipo_supported(RESULT ltoSupported OUTPUT ltoError)

if(ltoSupported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    message(STATUS "LTO (IPO) ENABLED")
else()
    message(WARNING "LTO DISABLED: ${ltoError}")
endif()

option(BUILD_NLUTILS_EXTRA "Build NLUtils Extra lib" ON)
option(BUILD_NLUTILS_NLGTOOLKIT "Build NLUtils NL Graphics Toolkit lib (requires Extra)" ON)

if(NOT DEFINED NLUTILS_GLAD_INCLUDE_DIR)
    set(NLUTILS_GLAD_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_NUKLEAR_INCLUDE_DIR)
    set(NLUTILS_NUKLEAR_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_PUGIXML_INCLUDE_DIR)
    set(NLUTILS_PUGIXML_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_GLM_INCLUDE_DIR)
    set(NLUTILS_GLM_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_STB_INCLUDE_DIR)
    set(NLUTILS_STB_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_REPLXX_INCLUDE_DIR)
    set(NLUTILS_REPLXX_INCLUDE_DIR "internal")
endif()
if(NOT DEFINED NLUTILS_GLFW_INCLUDE_DIR)
    set(NLUTILS_GLFW_INCLUDE_DIR "internal")
endif()

add_library(NLUtils 
    src/Logger.cpp
    src/JSONUtils.cpp
    src/XMLUtils.cpp
    src/Timer.cpp
    src/DebugUtils.cpp
    src/TextUtils.cpp
)

if(NLUTILS_PUGIXML_INCLUDE_DIR STREQUAL "internal")
    add_subdirectory(libs/pugixml ${CMAKE_CURRENT_BINARY_DIR}/pugixml_build)
    target_include_directories(NLUtils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src
    )
else()
    target_include_directories(NLUtils PUBLIC
        ${NLUTILS_PUGIXML_INCLUDE_DIR}
    )
endif()

if(NLUTILS_REPLXX_INCLUDE_DIR STREQUAL "internal")
    add_subdirectory(libs/replxx-0.0.4 ${CMAKE_CURRENT_BINARY_DIR}/replxx_build)
    target_include_directories(NLUtils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/replxx-0.0.4/include
    )
else()
    target_include_directories(NLUtils PUBLIC
        ${NLUTILS_REPLXX_INCLUDE_DIR}
        message(STATUS "Current replxx include dir: ${NLUTILS_REPLXX_INCLUDE_DIR}")
    )
endif()


target_link_libraries(NLUtils PUBLIC
    pugixml
    replxx
)


if(BUILD_NLUTILS_EXTRA)
    add_library(NLUtils-Extra
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
    if(NLUTILS_GLAD_INCLUDE_DIR STREQUAL "internal")
        add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/glad.c)
        target_include_directories(NLUtils-Extra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad) 
    else()
        target_include_directories(NLUtils-Extra PUBLIC ${NLUTILS_GLAD_INCLUDE_DIR})
        message(STATUS "Current GLAD include dir: ${NLUTILS_GLAD_INCLUDE_DIR}") 
    endif()

    if(NLUTILS_GLM_INCLUDE_DIR STREQUAL "internal")
        add_subdirectory(libs/glm-1.0.3/glm ${CMAKE_CURRENT_BINARY_DIR}/glm_build)
        target_include_directories(NLUtils-Extra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glm-1.0.3/glm) 
    else()
        target_include_directories(NLUtils-Extra PUBLIC ${NLUTILS_GLM_INCLUDE_DIR}) 
        message(STATUS "Current GLM include dir: ${NLUTILS_GLM_INCLUDE_DIR}")
    endif()

    if(NLUTILS_NUKLEAR_INCLUDE_DIR STREQUAL "internal")
        add_library(Nuklear ${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear/Nuklear.c)
        target_include_directories(NLUtils-Extra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear) 
    else()
        target_include_directories(NLUtils-Extra PUBLIC ${NLUTILS_NUKLEAR_INCLUDE_DIR}) 
        message(STATUS "Current Nuklear include dir: ${NLUTILS_NUKLEAR_INCLUDE_DIR}")
    endif()

    if(NLUTILS_STB_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-Extra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb) 
    else()
        target_include_directories(NLUtils-Extra PUBLIC ${NLUTILS_STB_INCLUDE_DIR})
        message(STATUS "Current STB include dir: ${NLUTILS_STB_INCLUDE_DIR}") 
    endif()

    if(NLUTILS_PUGIXML_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-Extra PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src) 
    else()
        target_include_directories(NLUtils-Extra PUBLIC ${NLUTILS_PUGIXML_INCLUDE_DIR})
        message(STATUS "Current PUGIXML include dir: ${NLUTILS_PUGIXML_INCLUDE_DIR}") 
    endif()


    target_link_libraries(NLUtils-Extra PRIVATE
        glm::glm
        NLUtils
        Nuklear
        glad
    )
endif()


if(BUILD_NLUTILS_NLGTOOLKIT)
    if(BUILD_NLUTILS_NLGTOOLKIT)
    add_library(NLUtils-NLGToolkit
        src/NLGToolkit/NKGLImplementation.cpp
        src/NLGToolkit/NLGToolkit.cpp
    )

    if(NLUTILS_GLM_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glm-1.0.3/glm) 
    else()
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${NLUTILS_GLM_INCLUDE_DIR}) 
        message(STATUS "Current GLM include dir: ${NLUTILS_GLM_INCLUDE_DIR}")
    endif()

    if(NLUTILS_GLAD_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad) 
    else()
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${NLUTILS_GLAD_INCLUDE_DIR})
        message(STATUS "Current GLAD include dir: ${NLUTILS_GLAD_INCLUDE_DIR}") 
    endif()

    if(NLUTILS_NUKLEAR_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/nuklear) 
    else()
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${NLUTILS_NUKLEAR_INCLUDE_DIR}) 
        message(STATUS "Current Nuklear include dir: ${NLUTILS_NUKLEAR_INCLUDE_DIR}")
    endif()

    if(NLUTILS_PUGIXML_INCLUDE_DIR STREQUAL "internal")
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/pugixml/src) 
    else()
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${NLUTILS_PUGIXML_INCLUDE_DIR})
        message(STATUS "Current PUGIXML include dir: ${NLUTILS_PUGIXML_INCLUDE_DIR}") 
    endif()

    if(NLUTILS_GLFW_INCLUDE_DIR STREQUAL "internal")
        add_subdirectory(libs/glfw-nor ${CMAKE_CURRENT_BINARY_DIR}/glfw_build)
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw-nor/include) 
    else()
        target_include_directories(NLUtils-NLGToolkit PUBLIC ${NLUTILS_GLFW_INCLUDE_DIR})
        message(STATUS "Current PUGIXML include dir: ${NLUTILS_GLFW_INCLUDE_DIR}") 
    endif()

    target_link_libraries(NLUtils-NLGToolkit PRIVATE
        glm::glm
        NLUtils
        NLUtils-Extra
        Nuklear
        glad
        glfw
    )
    else()
    message(FATAL_ERROR "Invalid configuration") 
    endif()
endif()