cmake_minimum_required(VERSION 3.16)
project(FieldFest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad PUBLIC third_party/glad/include)

add_library(imgui STATIC
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/backends/imgui_impl_glfw.cpp
  third_party/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
target_link_libraries(imgui PUBLIC glfw)

add_library(miniaudio STATIC third_party/miniaudio/miniaudio.c)
target_include_directories(miniaudio PUBLIC third_party/miniaudio)
if(APPLE)
  target_link_libraries(miniaudio PUBLIC "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox")
elseif(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(miniaudio PUBLIC Threads::Threads ${CMAKE_DL_LIBS} m)
endif()

find_package(OpenGL REQUIRED)

add_executable(FieldFest
  src/main.cpp
  src/App.cpp
  src/Audio.cpp
  src/Music.cpp
  src/Track.cpp
  src/Show.cpp
  src/Stage.cpp
  src/Crowd.cpp
  src/Performers.cpp
  src/Location.cpp
  src/GridMesh.cpp
  src/Props.cpp
  src/Beams.cpp
  src/Shader.cpp
  src/Camera.cpp
  src/Terrain.cpp
  src/Grass.cpp
  src/Sky.cpp
  src/GlowSprites.cpp
  src/Game.cpp
  src/Hud.cpp
  src/GpuTimer.cpp
  src/Screenshot.cpp
  src/FrameUniforms.cpp)

target_include_directories(FieldFest PRIVATE src third_party/glm)
target_link_libraries(FieldFest PRIVATE glfw glad imgui miniaudio OpenGL::GL)
target_compile_definitions(FieldFest PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders" SONGS_DIR="${CMAKE_SOURCE_DIR}/songs")
if(APPLE)
  target_compile_definitions(FieldFest PRIVATE GL_SILENCE_DEPRECATION)
endif()
if(MSVC)
  target_compile_options(FieldFest PRIVATE /W3 /utf-8)
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT FieldFest)
  set_property(TARGET FieldFest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:FieldFest>")
else()
  target_compile_options(FieldFest PRIVATE -Wall -Wextra)
endif()

add_custom_command(TARGET FieldFest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:FieldFest>/shaders")
