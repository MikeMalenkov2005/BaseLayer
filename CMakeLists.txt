cmake_minimum_required(VERSION 3.16)
project(BaseLayer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)

if (TARGET ${PROJECT_NAME})
  message(WARNING "${PROJECT_NAME} is included twice!")
else()
  file(GLOB BASE_LAYER_SOURCES "src/*.c")

  add_library(${PROJECT_NAME} STATIC ${BASE_LAYER_SOURCES})

  if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE wsock32 ws2_32)
  endif()

  target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
endif()
