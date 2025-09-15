cmake_minimum_required(VERSION 3.31)

project(
  lol-at-home
  LANGUAGES CXX
)

enable_testing()
find_package(GTest CONFIG REQUIRED)

# add find_package stuff as needed

add_subdirectory(src)
add_subdirectory(tests)
