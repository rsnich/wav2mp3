cmake_minimum_required(VERSION 3.10)

project(wav2mp3 
    VERSION 1.0
    DESCRIPTION "WAV to MP3 convertor."
    LANGUAGES CXX)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(TARGET_ARCH ${CMAKE_SYSTEM_PROCESSOR})

if ("${ARCH}" MATCHES "x86_64")
    set(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-m64 ${CMAKE_C_FLAGS}")
    set(TARGET_ARCH ${ARCH})
elseif ("${ARCH}" MATCHES "x86")
    set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
    set(TARGET_ARCH ${ARCH})
else ()
    message("Host architecture will be specified as the target architecture.")
endif ()

message("Target architecture: ${TARGET_ARCH}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

link_directories("${CMAKE_SOURCE_DIR}/lib/${TARGET_ARCH}/linux")

add_executable(wav2mp3 wav2mp3.cpp 
                        thread_pool.cpp 
                        wav2mp3_convertor.cpp)

target_include_directories(wav2mp3 PUBLIC
                           ${CMAKE_SOURCE_DIR}
                           ${CMAKE_SOURCE_DIR}/include
                           ${CMAKE_SOURCE_DIR}/utils/file_utils
                            )

target_link_libraries(wav2mp3 PRIVATE mp3lame pthread)

add_subdirectory(utils)