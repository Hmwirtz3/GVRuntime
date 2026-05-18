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

    src/Modules/MessageRelay/MessageRelay.cpp

    src/Modules/ControlInput/Controller.cpp
    src/Modules/TextureDictionary/TextureDictionary.cpp
    src/Modules/UI/TexturedQuad.cpp
    src/Modules/UI/Button.cpp
    src/Modules/UI/Crosshair.cpp
    src/Modules/UI/ScrollingMap.cpp

    src/Modules/StaticMesh/StaticMesh.cpp

    src/Modules/Camera/Camera.cpp
    src/Modules/Camera/CameraSystem.cpp

    src/Modules/AreaTriggerBox/AreaTriggerBox.cpp
    src/Modules/ZoneTrigger/ZoneTrigger.cpp

    src/Modules/Audio/AudioSource.cpp

    #src/Modules/Networking/Network.cpp
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
    pspnet
    pspnet_inet
    pspnet_apctl
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

# ------------------------------------------------------------
# DEPLOY DIRECTORY
# ------------------------------------------------------------

set(DEPLOY_DIR
    "/mnt/c/Users/Harrison/Desktop/GravitasTest/New folder (2)/NewProject/Data"
)

# ------------------------------------------------------------
# COPY EBOOT AFTER BUILD
# ------------------------------------------------------------

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${DEPLOY_DIR}"

    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/EBOOT.PBP"
        "${DEPLOY_DIR}/EBOOT.PBP"

    VERBATIM
)