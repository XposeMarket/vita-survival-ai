cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(SurvivalAI)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -std=c++11")

# Include directories
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/libs/libzim/include
  ${CMAKE_SOURCE_DIR}/libs/sqlite3
)

# Source files
file(GLOB SOURCES
  "src/*.c"
  "src/*.cpp"
  "src/ui/*.cpp"
  "src/database/*.cpp"
  "src/zim/*.cpp"
  "src/search/*.cpp"
  "src/voice/*.cpp"
  "src/net/*.cpp"
  "src/rss/*.cpp"
  "src/extractor/*.cpp"
  "src/online/*.cpp"
  "src/llm/*.cpp"
)

# SQLite3 library (we'll build it in)
set(SQLITE_SOURCES
  libs/sqlite3/sqlite3.c
  libs/sqlite3/sqlite3_vita_os.c
)

add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${SQLITE_SOURCES}
)

target_compile_definitions(SurvivalAI PRIVATE 
  SQLITE_OMIT_LOAD_EXTENSION=1
  SQLITE_THREADSAFE=0
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
  vita2d
  SceDisplay_stub
  SceCtrl_stub
  SceAudio_stub
  SceSysmodule_stub
  SceGxm_stub
  ScePgf_stub
  SceCommonDialog_stub
  SceIme_stub
  SceNet_stub
  SceNetCtl_stub
  SceHttp_stub
  SceSsl_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceIofilemgr_stub
  freetype
  png
  jpeg
  z
  m
  c
  stdc++
)

# Create VPK
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
