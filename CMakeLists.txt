cmake_minimum_required(VERSION 3.5)

project(GVRuntime)

# ------------------------------------------------------------
# PSP TOOLCHAIN
# ------------------------------------------------------------

if(NOT PSP)
    message(FATAL_ERROR "Build with PSP toolchain (psp-cmake)")
endif()

# ------------------------------------------------------------
# BUILD TYPE
# ------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ------------------------------------------------------------
# SOURCE FILES
# ------------------------------------------------------------

set(SOURCES
    
    src/main.cpp
    src/game.cpp

    
    src/Renderer/Backends/PSP/PSPGraphics.cpp

    src/Platform/PSP/PSPSystem.cpp

    src/Framework/SceneHandler/SceneLoader.cpp
    src/Framework/LogicUnitHandler/LogicUnitDispatch.cpp 
    src/Framework/MessageHandler/MessageHandler.cpp 

    src/Framework/Utils/Allocator.cpp
    src/Framework/Utils/RenderCache.cpp
    

    src/Renderer/Renderer.cpp

    src/Font/FontRenderer.cpp 
    src/Font/FontAtlas2.cpp

    src/Modules/ControlInput/Controller.cpp
    src/Modules/TextureDictionary/TextureDictionary.cpp
    src/Modules/UI/TexturedQuad.cpp
    src/Modules/StaticMesh/StaticMesh.cpp
    src/Modules/Camera/Camera.cpp 
    src/Modules/Camera/CameraSystem.cpp 
    src/Modules/AreaTriggerBox/AreaTriggerBox.cpp 
    src/Modules/Audio/AudioSource.cpp 



)

# ------------------------------------------------------------
# EXECUTABLE
# ------------------------------------------------------------

add_executable(${PROJECT_NAME} ${SOURCES})

# ------------------------------------------------------------
# INCLUDE PATHS
# ------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ------------------------------------------------------------
# COMPILER FLAGS
# ------------------------------------------------------------

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-O0>
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -G0
    -Wall
)

# ------------------------------------------------------------
# LINK LIBRARIES
# ------------------------------------------------------------

target_link_libraries(${PROJECT_NAME}
    pspgu
    pspgum
    pspdisplay
    pspctrl
    pspmp3
    pspaudio
    pspaudiolib
    pspdebug
    pspge
    pspvfpu
    c
    m
)

# ------------------------------------------------------------
# PSP EBOOT
# ------------------------------------------------------------

create_pbp_file(
    TARGET ${PROJECT_NAME}
    TITLE "OblivionDemo"
    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL
    SND0_PATH NULL
)