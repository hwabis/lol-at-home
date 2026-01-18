cmake_minimum_required(VERSION 3.28)

project(
  lol-at-home
  LANGUAGES CXX
)

enable_testing()
find_package(GTest CONFIG REQUIRED)

find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-enet CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
