cmake_minimum_required(VERSION 3.18.1)
project(pikmin_nectar_rpc)

set(CMAKE_CXX_STANDARD 20)
set(SUPPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/support")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -O2 -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ffixed-x18 -Wl,--hash-style=both -Wl,--gc-sections -Wl,--strip-all")

include_directories("${SUPPORT_DIR}" "${SUPPORT_DIR}/xdl/include")
file(GLOB XDL_SOURCES "${SUPPORT_DIR}/xdl/*.c")

add_library(pikmin_nectar_rpc SHARED
    "${SUPPORT_DIR}/main.cpp"
    "${SUPPORT_DIR}/And64InlineHook.cpp"
    nectar_rpc.cpp
    gps_copy.cpp
    ${XDL_SOURCES})
target_link_libraries(pikmin_nectar_rpc log dl)
