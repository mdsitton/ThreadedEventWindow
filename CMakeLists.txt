cmake_minimum_required(VERSION 3.8)

project(swaptest)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#SET(CMAKE_CXX_FLAGS "-O2")
add_compile_options("/std:c++latest")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(ENV{GLM_ROOT}       "${PROJECT_SOURCE_DIR}/extern/glm")
set(ENV{SPDLOG_ROOT}    "${PROJECT_SOURCE_DIR}/extern/spdlog")

find_package(OpenGL REQUIRED)
find_package(GLM    REQUIRED)

set(SHARED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/timer.cpp)
set(SOURCE_EXAMPLE ${CMAKE_CURRENT_SOURCE_DIR}/src/example.cpp)
set(SOURCE_QUEUE ${CMAKE_CURRENT_SOURCE_DIR}/src/queuetest.cpp)

source_group("src" FILES ${SOURCE})

include_directories(${PROJECT_SOURCE_DIR}/src ${SPDLOG_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${STB_INCLUDE_DIR})

add_executable(example ${SOURCE_EXAMPLE} ${SHARED_SOURCE})
target_link_libraries(example ${OPENGL_LIBRARIES} Hid)


add_executable(queuetest ${SOURCE_QUEUE} ${SHARED_SOURCE})

# add_executable(particletest ${SOURCE_PARTICLE} ${SHARED_SOURCE})
