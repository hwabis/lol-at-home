include(${CMAKE_SOURCE_DIR}/cmake/AddTestExecutable.cmake)

add_test_executable(lol-at-home-shared-tests lol-at-home-shared
  GameStateSerializerTest.cc
)
